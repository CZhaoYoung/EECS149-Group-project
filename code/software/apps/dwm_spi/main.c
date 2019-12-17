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

  error_code = dwm_init(&spi_instance);
  APP_ERROR_CHECK(error_code);
  printf("spi initialized!\n");


  // send three 0xff to reset
  // error_code = dwm_hard_reset(&spi_instance);
  // APP_ERROR_CHECK(error_code);

  error_code = dwm_reset(&spi_instance);
  APP_ERROR_CHECK(error_code);

  uint8_t rx_buf[4];
  rx_buf[0] = 0;
  rx_buf[1] = 0;
  rx_buf[2] = 0;
  rx_buf[3] = 0;

  // error_code = nrf_drv_spi_transfer(&spi_instance, NULL, 0, rx_buf, 2);
  // APP_ERROR_CHECK(error_code);
  // while (rx_buf[0] != 0xff) {
  //   error_code = nrf_drv_spi_transfer(&spi_instance, NULL, 0, rx_buf, 2);
  //   APP_ERROR_CHECK(error_code);
  //   if (error_code != NRF_SUCCESS) {
  //     printf("continuing spi error code: %d\n", (int) error_code);
  //   }
  //   printf("%x %x\n", rx_buf[0], rx_buf[1]);
//  }

//  error_code = dwm_cfg_set(&spi_instance);

  // dwm_cfg_tag_t cfg_tag;
  // dwm_cfg_t cfg_node;

  // cfg_tag.low_power_en = 0;
  // cfg_tag.meas_mode = DWM_MEAS_MODE_TWR;
  // cfg_tag.loc_engine_en = 1;
  // cfg_tag.common.led_en = 1;
  // cfg_tag.common.ble_en = 1;
  // cfg_tag.common.uwb_mode = DWM_UWB_MODE_ACTIVE;
  // cfg_tag.common.fw_update_en = 0;

  // error_code = dwm_cfg_tag_set(&spi_instance, &cfg_tag);
  // APP_ERROR_CHECK(error_code);
  // printf("set cfg\n");

  // nrf_delay_ms(1000);

  // error_code = dwm_cfg_get(&spi_instance, &cfg_node);
  // APP_ERROR_CHECK(error_code);

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

  dwm_pos_t pos;
  while (1) {
    nrf_delay_ms(1000);
    dwm_pos_get(&spi_instance, &pos);
    printf("x y: [%ld, %ld]\n", pos.x , pos.y);
  }
}