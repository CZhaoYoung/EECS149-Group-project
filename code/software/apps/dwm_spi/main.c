#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "app_util_platform.h"
#include "nordic_common.h"
#include "app_uart.h"
#include "nrf_drv_clock.h"
#include "nrf_power.h"
#include "nrf_drv_spi.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
//
#include "buckler.h"
#include "my_dwm.h"

static nrf_drv_spi_t spi_instance = NRF_DRV_SPI_INSTANCE(1);

int main(void) {
  ret_code_t error_code = NRF_SUCCESS;

  // initialize RTT library
  error_code = NRF_LOG_INIT(NULL);
  APP_ERROR_CHECK(error_code);
  NRF_LOG_DEFAULT_BACKENDS_INIT();
  printf("Log initialized!\n");

  // initialize spi
  nrf_drv_spi_config_t spi_config = {
    .sck_pin = SD_CARD_SPI_SCLK,
    .mosi_pin = SD_CARD_SPI_MOSI,
    .miso_pin = SD_CARD_SPI_MISO,
    .ss_pin = SD_CARD_SPI_CS,
    .irq_priority = NRFX_SPI_DEFAULT_CONFIG_IRQ_PRIORITY,
    .orc = 0,
    .frequency = NRF_DRV_SPI_FREQ_4M,
    .mode = NRF_DRV_SPI_MODE_1,
    .bit_order = NRF_DRV_SPI_BIT_ORDER_MSB_FIRST
  };
  error_code = nrf_drv_spi_init(&spi_instance, &spi_config, NULL, NULL);
  APP_ERROR_CHECK(error_code);
  printf("SPI initialized!\n");

  // printf("dwn init\n");
  // dwm_init(&spi_instance);
  // printf("init success\n");

  uint8_t reset_buf[1];
  reset_buf[0] = 0xff;

  uint8_t tx_buf[2];
  tx_buf[0] = 0x02;
  tx_buf[1] = 0x00;
  // tx_buf[2] = 0x00;
  // tx_buf[3] = 0x00;

  uint8_t rx_buf[18];
  rx_buf[0] = 0;
  rx_buf[1] = 0;
  rx_buf[2] = 0;
  rx_buf[3] = 0;
  rx_buf[4] = 0;
  rx_buf[5] = 0;
  rx_buf[6] = 0;
  rx_buf[7] = 0;
  rx_buf[8] = 0;
  rx_buf[9] = 0;
  rx_buf[10] = 0;
  rx_buf[11] = 0;
  rx_buf[12] = 0;
  rx_buf[13] = 0;
  rx_buf[14] = 0;
  rx_buf[15] = 0;
  rx_buf[16] = 0;
  rx_buf[17] = 0;

  error_code = nrf_drv_spi_transfer(&spi_instance, reset_buf, 1, NULL, 0);
  APP_ERROR_CHECK(error_code);

  printf("reset\n");
  while (rx_buf[0] == 0 && rx_buf[1] == 0) {
    error_code = nrf_drv_spi_transfer(&spi_instance, NULL, 0, rx_buf, 2);
    APP_ERROR_CHECK(error_code);
    if (error_code != NRF_SUCCESS) {
      printf("continuing spi error code: %d\n", (int) error_code);
    }
    else {
      printf("rx_buf: %x %x %x\n", rx_buf[0], rx_buf[1], rx_buf[2]);
    }
    nrf_delay_ms(100);
  }

  nrf_delay_ms(1000);

  printf("Trying to send command\n");
  error_code = nrf_drv_spi_transfer(&spi_instance, tx_buf, 4, NULL, 0);
  APP_ERROR_CHECK(error_code);
  error_code = nrf_drv_spi_transfer(&spi_instance, NULL, 0, rx_buf, 2);
  APP_ERROR_CHECK(error_code);
  if (error_code != NRF_SUCCESS) {
    printf("spi error code: %d\n", (int) error_code);
  }

  while (rx_buf[0] == 0 && rx_buf[1] == 0) {
    error_code = nrf_drv_spi_transfer(&spi_instance, NULL, 0, rx_buf, 2);
    APP_ERROR_CHECK(error_code);
    if (error_code != NRF_SUCCESS) {
      printf("continuing spi error code: %d\n", (int) error_code);
    }
    else {
      printf("rx_buf: %x %x %x\n", rx_buf[0], rx_buf[1], rx_buf[2]);
    }
  }

  printf("received: %x %x\n", rx_buf[0], rx_buf[1]);

  while (rx_buf[0] == 0 || rx_buf[0] == 0xff) {
    error_code = nrf_drv_spi_transfer(&spi_instance, NULL, 0, rx_buf, 3);
    APP_ERROR_CHECK(error_code);
    if (error_code != NRF_SUCCESS) {
      printf("continuing spi error code: %d\n", (int) error_code);
    }
    else {
      printf("rx_buf: %x %x %x\n", rx_buf[0], rx_buf[1], rx_buf[2]);
    }
  }

  printf("received: %x %x\n", rx_buf[0], rx_buf[1]);
  printf("x: %x %x %x %x \n", rx_buf[5], rx_buf[6], rx_buf[7], rx_buf[8]);
  printf("y: %x %x %x %x \n", rx_buf[9], rx_buf[10], rx_buf[11], rx_buf[12]);





  // dwm_cfg_tag_t cfg_tag;
  // dwm_cfg_t cfg_node;

  // cfg_tag.low_power_en = 0;
  // cfg_tag.meas_mode = DWM_MEAS_MODE_TWR;
  // cfg_tag.loc_engine_en = 0;
  // cfg_tag.common.led_en = 0;
  // cfg_tag.common.ble_en = 0;
  // cfg_tag.common.uwb_mode = DWM_UWB_MODE_ACTIVE;
  // cfg_tag.common.fw_update_en = 0;

  // dwm_cfg_tag_set(&cfg_tag);
  // nrf_delay_ms(5000);
  // dwm_cfg_get(&cfg_node);

  // if((cfg_tag.low_power_en        != cfg_node.low_power_en)
  //  || (cfg_tag.meas_mode           != cfg_node.meas_mode)
  //  || (cfg_tag.loc_engine_en       != cfg_node.loc_engine_en)
  //  || (cfg_tag.common.led_en       != cfg_node.common.led_en)
  //  || (cfg_tag.common.ble_en       != cfg_node.common.ble_en)
  //  || (cfg_tag.common.uwb_mode     != cfg_node.common.uwb_mode)
  //  || (cfg_tag.common.fw_update_en != cfg_node.common.fw_update_en))
  //  {
  //   printf("low_power_en        cfg_tag=%d : cfg_node=%d\n", cfg_tag.low_power_en,     cfg_node.low_power_en);
  //   printf("meas_mode           cfg_tag=%d : cfg_node=%d\n", cfg_tag.meas_mode,        cfg_node.meas_mode);
  //   printf("loc_engine_en       cfg_tag=%d : cfg_node=%d\n", cfg_tag.loc_engine_en,    cfg_node.loc_engine_en);
  //   printf("common.led_en       cfg_tag=%d : cfg_node=%d\n", cfg_tag.common.led_en,    cfg_node.common.led_en);
  //   printf("common.ble_en       cfg_tag=%d : cfg_node=%d\n", cfg_tag.common.ble_en,    cfg_node.common.ble_en);
  //   printf("common.uwb_mode     cfg_tag=%d : cfg_node=%d\n", cfg_tag.common.uwb_mode,  cfg_node.common.uwb_mode);
  //   printf("common.fw_update_en cfg_tag=%d : cfg_node=%d\n", cfg_tag.common.fw_update_en, cfg_node.common.fw_update_en);
  //   printf("Configuration failed\n");
  // }
  // else {
  //   printf("Configuration success\n");
  // }

  // dwm_pos_t pos;

  while (1) {
    nrf_delay_ms(2000);
  //   dwm_pos_get(&pos);
  }

}