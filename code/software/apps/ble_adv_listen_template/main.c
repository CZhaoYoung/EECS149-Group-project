// BLE RX app
//
// Receives BLE advertisements with data

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "app_error.h"
#include "app_timer.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "nrf.h"
#include "app_util.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_drv_spi.h"

#include "buckler.h"
#include "display.h"
#include "kobukiActuator.h"
#include "kobukiSensorPoll.h"
#include "kobukiSensorTypes.h"
#include "kobukiUtilities.h"
#include "mpu9250.h"
#include "simple_ble.h"

#include "states.h"

NRF_TWI_MNGR_DEF(twi_mngr_instance, 5, 0);
KobukiSensors_t sensors = {0};

// BLE configuration
// This is mostly irrelevant since we are scanning only
static simple_ble_config_t ble_config = {
        // BLE address is c0:98:e5:49:00:00
        .platform_id       = 0x49,    // used as 4th octet in device BLE address
        .device_id         = 0x2048,  // Last two octets of device address
        .adv_name          = "EE149", // irrelevant in this example
        .adv_interval      = MSEC_TO_UNITS(1000, UNIT_0_625_MS), // send a packet once per second (minimum is 20 ms)
        .min_conn_interval = MSEC_TO_UNITS(500, UNIT_1_25_MS), // irrelevant if advertising only
        .max_conn_interval = MSEC_TO_UNITS(1000, UNIT_1_25_MS), // irrelevant if advertising only
};
simple_ble_app_t* simple_ble_app;

bool drive = false;
bool back = false;
bool left = false;
bool right = false;

void ble_evt_adv_report(ble_evt_t const* p_ble_evt) {

  ble_gap_evt_adv_report_t const* adv_report = &(p_ble_evt->evt.gap_evt.params.adv_report);
  // TODO: extract the fields we care about (Peer address and data)
  ble_gap_addr_t peer = adv_report -> peer_addr;
  ble_data_t data = adv_report -> data;

  // TODO: filter on Peer address
  if (peer.addr[5] == 0xC0 && peer.addr[4] == 0x98 && peer.addr[3] == 0xE5 &&
      peer.addr[2] == 0x49 && peer.addr[1] == 0x20 && peer.addr[0] == 0x43) {
    
    drive = (data.p_data[7] == 21 && data.p_data[8] == 7 && data.p_data[9] == 0 && data.p_data[10] == 0);
    back = (data.p_data[7] == 129 && data.p_data[8] == 10 && data.p_data[9] == 0 && data.p_data[10] == 0);
    left = (data.p_data[7] == 4 && data.p_data[8] == 0 && data.p_data[9] == 32 && data.p_data[10] == 129);
    right = (data.p_data[7] == 0 && data.p_data[8] == 32 && data.p_data[9] == 129 && data.p_data[10] == 10);
    
    //while (1) {

      // TODO: get length of field
      // TODO: get type of field: if type is 0xFF, we found it!
      // Print the data as a string. i.e. printf("%s\n", data + offset)
      // Otherwise, skip ahead by the length of the current field
    //}
  }
}

void print_state(states current_state){
  switch(current_state){
  case OFF: {
    display_write("OFF", DISPLAY_LINE_0);
    break;
    }
    case DRIVE: {
    display_write("DRIVE", DISPLAY_LINE_0);
    break;
    }
    case BACK: {
    display_write("BACK", DISPLAY_LINE_0);
    break;
    }
    case LEFT: {
    display_write("LEFT", DISPLAY_LINE_0);
    break;
    }
    case RIGHT: {
    display_write("RIGHT", DISPLAY_LINE_0);
    break;
    }
    case WAIT: {
    display_write("WAIT", DISPLAY_LINE_0);
    break;
    }
  }
}

int main(void) {
  ret_code_t error_code = NRF_SUCCESS;

  // Initialize

  // initialize RTT library
  error_code = NRF_LOG_INIT(NULL);
  APP_ERROR_CHECK(error_code);
  NRF_LOG_DEFAULT_BACKENDS_INIT();
  printf("Log initialized\n");

  printf("!!!!!\n");
  // Setup BLE
  // Note: simple BLE is our own library. You can find it in `nrf5x-base/lib/simple_ble/`
  simple_ble_app = simple_ble_init(&ble_config);
  advertising_stop();

  // TODO: Start scanning
  scanning_start();

  // initialize LEDs
  nrf_gpio_pin_dir_set(23, NRF_GPIO_PIN_DIR_OUTPUT);
  nrf_gpio_pin_dir_set(24, NRF_GPIO_PIN_DIR_OUTPUT);
  nrf_gpio_pin_dir_set(25, NRF_GPIO_PIN_DIR_OUTPUT);

  // initialize display
  nrf_drv_spi_t spi_instance = NRF_DRV_SPI_INSTANCE(1);
  nrf_drv_spi_config_t spi_config = {
    .sck_pin = BUCKLER_LCD_SCLK,
    .mosi_pin = BUCKLER_LCD_MOSI,
    .miso_pin = BUCKLER_LCD_MISO,
    .ss_pin = BUCKLER_LCD_CS,
    .irq_priority = NRFX_SPI_DEFAULT_CONFIG_IRQ_PRIORITY,
    .orc = 0,
    .frequency = NRF_DRV_SPI_FREQ_4M,
    .mode = NRF_DRV_SPI_MODE_2,
    .bit_order = NRF_DRV_SPI_BIT_ORDER_MSB_FIRST
  };
  error_code = nrf_drv_spi_init(&spi_instance, &spi_config, NULL, NULL);
  APP_ERROR_CHECK(error_code);
  display_init(&spi_instance);
  printf("Display initialized!\n");

  // initialize i2c master (two wire interface)
  nrf_drv_twi_config_t i2c_config = NRF_DRV_TWI_DEFAULT_CONFIG;
  i2c_config.scl = BUCKLER_SENSORS_SCL;
  i2c_config.sda = BUCKLER_SENSORS_SDA;
  i2c_config.frequency = NRF_TWIM_FREQ_100K;
  error_code = nrf_twi_mngr_init(&twi_mngr_instance, &i2c_config);
  APP_ERROR_CHECK(error_code);
  mpu9250_init(&twi_mngr_instance);
  printf("IMU initialized!\n");

  // initialize Kobuki
  kobukiInit();
  printf("Kobuki initialized!\n");

  

  states state = OFF;  

  while (1) {
    // Sleep while SoftDevice handles BLE
    kobukiSensorPoll(&sensors);

    switch(state) {
      case OFF: {
        print_state(state);
        // transition logic
        if (is_button_pressed(&sensors)) {
            state = WAIT;
        } else {
          state = OFF;
          // perform state-specific actions here
          kobukiDriveDirect(0, 0);
        }
        break; // each case needs to end with break!
      }
      case WAIT: {
        print_state(state);
        // transition logic
        if (is_button_pressed(&sensors)) {
          state = OFF;
          kobukiDriveDirect(0, 0);
        } else {
          if (drive && !back && !right && !left) {
            state = DRIVE;
            kobukiDriveDirect(100, 100);
          }
          else if (!drive && back && !right && !left) {
            state = BACK;
            kobukiDriveDirect(-100, -100);
          }
          else if (!drive && !back && right && !left) {
            state = RIGHT;
            kobukiDriveDirect(100, -100);
          }
          else if (!drive && !back && !right && left) {
            state = LEFT;
            kobukiDriveDirect(-100, 100);
          }
          else {
            state = WAIT;
            kobukiDriveDirect(0, 0);
          }
        }
        break; // each case needs to end with break!
      }
      case DRIVE: {
        print_state(state);
        // transition logic
        if (is_button_pressed(&sensors)) {
          state = OFF;
          kobukiDriveDirect(0, 0);
        } else {
          if (drive && !back && !right && !left) {
            state = DRIVE;
            kobukiDriveDirect(100, 100);
          }
          else {
            state = WAIT;
            kobukiDriveDirect(0, 0);
          }
        }
        break; // each case needs to end with break!
      }
      case BACK: {
        print_state(state);
        // transition logic
        if (is_button_pressed(&sensors)) {
          state = OFF;
          kobukiDriveDirect(0, 0);
        } else {
          if (!drive && back && !right && !left) {
            state = BACK;
            kobukiDriveDirect(-100, -100);
          }
          else {
            state = WAIT;
            kobukiDriveDirect(0, 0);
          }
        }
        break; // each case needs to end with break!
      }
      case LEFT: {
        print_state(state);
        // transition logic
        if (is_button_pressed(&sensors)) {
          state = OFF;
          kobukiDriveDirect(0, 0);
        } else {
          if (!drive && !back && !right && left) {
            state = LEFT;
            kobukiDriveDirect(-100, 100);
          }
          else {
            state = WAIT;
            kobukiDriveDirect(0, 0);
          }
        }
        break; // each case needs to end with break!
      }
      case RIGHT: {
        print_state(state);
        // transition logic
        if (is_button_pressed(&sensors)) {
          state = OFF;
          kobukiDriveDirect(0, 0);
        } else {
          if (!drive && !back && right && !left) {
            state = RIGHT;
            kobukiDriveDirect(100, -100);
          }
          else {
            state = WAIT;
            kobukiDriveDirect(0, 0);
          }
        }
        break; // each case needs to end with break!
      }
    }

  }
}



