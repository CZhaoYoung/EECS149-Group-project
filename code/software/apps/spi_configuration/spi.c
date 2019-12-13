#include "nrf_drv_spi.h"
#include "nrf_drv_gpiote.h"
#include "app_error.h"

#include "ab1815.h"
#include "dwm_api.h"
#include "buckler.h"

static const nrf_drv_spi_t* spi_instance;
static ab1815_control_t ctrl_config;
static nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;
static ab1815_alarm_callback* interrupt_callback;

void ab1815_init(const nrf_drv_spi_t* instance) {
  spi_instance = instance;

  spi_config.sck_pin    = SD_CARD_SPI_SCLK;
  spi_config.miso_pin   = SD_CARD_SPI_MISO;
  spi_config.mosi_pin   = SD_CARD_SPI_MOSI;
  spi_config.ss_pin     = SD_CARD_SPI_CS;
  spi_config.frequency  = NRF_DRV_SPI_FREQ_2M;
  spi_config.mode       = NRF_DRV_SPI_MODE_0;
  spi_config.bit_order  = NRF_DRV_SPI_BIT_ORDER_MSB_FIRST;

  nrf_gpio_cfg_output(RTC_WDI);
  nrf_gpio_pin_set(RTC_WDI);
}

void  ab1815_read_reg(uint32_t reg, uint32_t* read_buf, size_t len){
  if (len > 256) return;
  uint32_t readreg = reg;
  uint32_t buf[257];

  nrf_drv_spi_init(spi_instance, &spi_config, NULL, NULL);
  nrf_drv_spi_transfer(spi_instance, &readreg, 1, buf, len+1);
  nrf_drv_spi_uninit(spi_instance);

  memcpy(read_buf, buf+1, len);
}

void ab1815_write_reg(uint32_t reg, uint32_t* write_buf, size_t len){
  if (len > 256) return;
  uint32_t buf[257];
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

void uwb_set_config(uwb_control_t config) {
  uint16_t write;

  write = config.stnry_en << 10| config.meas_mode << 9 |
      config.low_power_en << 7 | config.loc_engine_en << 6 |
      config.enc_en << 5 | config.led_en << 4 | config.ble_en << 3 |
      config.fw_update_en << 2 | config.uwb_mode;
  ctrl_config = config; 
  ab1815_write_reg(UWB_CONTROL1, &write, 1)
}



void uwb_get_config(uwb_control_t* config) {
  uint16_t read;

  ab1815_read_reg(UWB_CONTROL1, &read, 1);
  config->stnry_en = (read & 0x0200) >> 10;
  config->meas_mode = (read & 0x0100) >> 9;
  config->low_power_en = (read & 0x0080) >> 7;
  config->loc_engine_en = (read & 0x0040) >> 6;
  config->enc_en = (read & 0x0020) >> 5;
  config->led_en = (read & 0x0010) >> 4;
  config->ble_en = (read & 0x0008) >> 3;
  config->fw_update_en = (read & 0x0004) >> 2;
  config->uwb_mode = read & 0x0003;

}

void ab1815_interrupt_config(ab1815_int_config_t config) {
  return;
}

inline uint8_t get_tens(uint8_t x) {
  return (x / 10) % 10;
}

inline uint8_t get_ones(uint8_t x) {
  return x % 10;
}

void ab1815_form_time_buffer(uwb_position position, uint32_t* buf) {

  // check the x, y is valid
  // we don't need to check 


  // To do
  buf[0] = (position.x & 0xFFFFFFFF) << 32 //?
  buf[1] = (position.y & 0xFFFFFFFF) << 32 //?



}

void uwb_get_position(uwb_position* position) { 
  uint32_t read[4];
  ab1815_read_reg(UWB_Y, read, 32);

  position -> x = (read[0] & 0xFFFFFFFF)
  position -> y = (read[0] & 0xFFFFFFFF)
}







