//
#include "dwm_api.h"
#include "nrf_drv_spi.h"
#include "nrf_drv_gpiote.h"
#include "app_error.h"

#include "spi.h"

static const nrf_drv_spi_t* spi_instance;
static ab1815_control_t ctrl_config;
static nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;
static ab1815_alarm_callback* interrupt_callback;

void ab1815_init(const nrf_drv_spi_t* instance) {
  spi_instance = instance;

  spi_config.sck_pin    = SPI_SCLK;
  spi_config.miso_pin   = SPI_MISO;
  spi_config.mosi_pin   = SPI_MOSI;
  spi_config.ss_pin     = RTC_CS;
  spi_config.frequency  = NRF_DRV_SPI_FREQ_2M;
  spi_config.mode       = NRF_DRV_SPI_MODE_0;
  spi_config.bit_order  = NRF_DRV_SPI_BIT_ORDER_MSB_FIRST;

  nrf_gpio_cfg_output(RTC_WDI);
  nrf_gpio_pin_set(RTC_WDI);
}

void  ab1815_read_reg(uint8_t reg, uint8_t* read_buf, size_t len){
  if (len > 256) return;
  uint8_t readreg = reg;
  uint8_t buf[257];

  nrf_drv_spi_init(spi_instance, &spi_config, NULL, NULL);
  nrf_drv_spi_transfer(spi_instance, &readreg, 1, buf, len+1);
  nrf_drv_spi_uninit(spi_instance);

  memcpy(read_buf, buf+1, len);
}

void ab1815_write_reg(uint8_t reg, uint8_t* write_buf, size_t len){
  if (len > 256) return;
  uint8_t buf[257];
  buf[0] = 0x80 | reg;
  memcpy(buf+1, write_buf, len);

  nrf_drv_spi_init(spi_instance, &spi_config, NULL, NULL);
  nrf_drv_spi_transfer(spi_instance, buf, len+1, NULL, 0);
  nrf_drv_spi_uninit(spi_instance);
}

static void interrupt_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action) {
  // read and clear interrupts
  uint8_t status = 0;
  ab1815_read_reg(AB1815_STATUS, &status, 1);

  if (status & 0x4 && interrupt_callback) {
    // call user callback
    interrupt_callback();
  }
}


void ab1815_set_config(ab1815_control_t config) {
  uint8_t write;
  write =  config.stop << 7 | config.hour_12 << 6 | config.OUTB << 5 |
            config.OUT << 4 | config.rst_pol << 3 | config.auto_rst << 2 |
            0x2 | config.write_rtc;
  ctrl_config = config;

  //nrf_spi_mngr_transfer_t const config_transfer[] = {
  //  NRF_SPI_MNGR_TRANSFER(write, 2, NULL, 0),
  //};

  //int error = nrf_spi_mngr_perform(spi_instance, &spi_config, config_transfer, 1, NULL);
  //APP_ERROR_CHECK(error);
  ab1815_write_reg(AB1815_CONTROL1, &write, 1);
}

void ab1815_get_config(ab1815_control_t* config) {
  uint8_t read;

  //nrf_spi_mngr_transfer_t const config_transfer[] = {
  //  NRF_SPI_MNGR_TRANSFER(write, 1, read, 2),
  //};

  //int error = nrf_spi_mngr_perform(spi_instance, &spi_config, config_transfer, 1, NULL);
  //APP_ERROR_CHECK(error);

  //printf("%x\n", read[1]);

  ab1815_read_reg(AB1815_CONTROL1, &read, 1);

  config->stop      = (read & 0x80) >> 7;
  config->hour_12   = (read & 0x40) >> 6;
  config->OUTB      = (read & 0x20) >> 5;
  config->OUT       = (read & 0x10) >> 4;
  config->rst_pol   = (read & 0x08) >> 3;
  config->auto_rst  = (read & 0x04) >> 2;
  config->write_rtc = read & 0x01;
}

void ab1815_interrupt_config(ab1815_int_config_t config) {
  return;
}


 void dwm_get_loc(){

 }