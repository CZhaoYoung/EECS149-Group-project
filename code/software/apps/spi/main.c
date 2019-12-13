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
  nrf_power_dcdcen_set(1);

  // Turn on power gate
  // nrf_gpio_cfg_output(MAX44009_EN);
  // nrf_gpio_cfg_output(ISL29125_EN);
  // nrf_gpio_cfg_output(MS5637_EN);
  // nrf_gpio_cfg_output(SI7021_EN);
  // nrf_gpio_pin_set(MAX44009_EN);
  // nrf_gpio_pin_set(ISL29125_EN);
  // nrf_gpio_pin_set(MS5637_EN);
  // nrf_gpio_pin_set(SI7021_EN);

  int wait_period = 1000;
  dwm_cfg_tag_t cfg_tag;
  dwm_cfg_t cfg_node;

  printf("dwn init\n");
  dwm_init(&spi_instance);
  printf("init success\n");

  cfg_tag.low_power_en = 0;
  cfg_tag.meas_mode = DWM_MEAS_MODE_TWR;
  cfg_tag.loc_engine_en = 0;
  cfg_tag.common.led_en = 0;
  cfg_tag.common.ble_en = 0;
  cfg_tag.common.uwb_mode = DWM_UWB_MODE_ACTIVE;
  cfg_tag.common.fw_update_en = 0;

  dwm_cfg_tag_set(&cfg_tag);
  nrf_delay_ms(7000);
  dwm_cfg_get(&cfg_node);

  if((cfg_tag.low_power_en        != cfg_node.low_power_en)
   || (cfg_tag.meas_mode           != cfg_node.meas_mode)
   || (cfg_tag.loc_engine_en       != cfg_node.loc_engine_en)
   || (cfg_tag.common.led_en       != cfg_node.common.led_en)
   || (cfg_tag.common.ble_en       != cfg_node.common.ble_en)
   || (cfg_tag.common.uwb_mode     != cfg_node.common.uwb_mode)
   || (cfg_tag.common.fw_update_en != cfg_node.common.fw_update_en))
   {
    printf("low_power_en        cfg_tag=%d : cfg_node=%d\n", cfg_tag.low_power_en,     cfg_node.low_power_en);
    printf("meas_mode           cfg_tag=%d : cfg_node=%d\n", cfg_tag.meas_mode,        cfg_node.meas_mode);
    printf("loc_engine_en       cfg_tag=%d : cfg_node=%d\n", cfg_tag.loc_engine_en,    cfg_node.loc_engine_en);
    printf("common.led_en       cfg_tag=%d : cfg_node=%d\n", cfg_tag.common.led_en,    cfg_node.common.led_en);
    printf("common.ble_en       cfg_tag=%d : cfg_node=%d\n", cfg_tag.common.ble_en,    cfg_node.common.ble_en);
    printf("common.uwb_mode     cfg_tag=%d : cfg_node=%d\n", cfg_tag.common.uwb_mode,  cfg_node.common.uwb_mode);
    printf("common.fw_update_en cfg_tag=%d : cfg_node=%d\n", cfg_tag.common.fw_update_en, cfg_node.common.fw_update_en);
    printf("Configuration failed\n");
  }
  else {
    printf("Configuration success\n");
  }

  dwm_pos_t pos;

  while (1) {
    nrf_delay_ms(2000);
    dwm_pos_get(&pos);
  }
}