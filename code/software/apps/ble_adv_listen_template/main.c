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

#include "my_dwm.h"
#include "states.h"

NRF_TWI_MNGR_DEF(twi_mngr_instance, 5, 0);
KobukiSensors_t sensors = {0};

#define PI 3.14159265

static nrf_drv_spi_t my_spi_instance = NRF_DRV_SPI_INSTANCE(1);
nrf_drv_spi_config_t my_spi_config = {
  .sck_pin = SD_CARD_SPI_SCLK,
  .mosi_pin = SD_CARD_SPI_MOSI,
  .miso_pin = SD_CARD_SPI_MISO,
  .ss_pin = SD_CARD_SPI_CS,
  .irq_priority = NRFX_SPI_DEFAULT_CONFIG_IRQ_PRIORITY,
  .orc = 0,
  .frequency = NRF_DRV_SPI_FREQ_4M,
  .mode = NRF_DRV_SPI_MODE_0,
  .bit_order = NRF_DRV_SPI_BIT_ORDER_MSB_FIRST
};
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
bool stop_auto = false;
bool get_pos_error = false;
int chase = 0;
int len_target = 51;
dwm_pos_t my_pos[51];
dwm_pos_t target_pos[51];
int pos_x[51] = {0};
int pos_y[51] = {0};
int index_target = 0;
int index_my = 0;
int speed = 150;
uint16_t start_encoder;

static float mesure_distance(uint16_t current_encoder, uint16_t previous_encoder) {
  if (current_encoder < previous_encoder) {
    current_encoder += 2^16;
  }
  
  const float CONVERSION = 0.00008529;
  float distance = CONVERSION * (current_encoder - previous_encoder);
  if (abs(distance) > 1) distance = 0;
  return distance;
}


void ble_evt_adv_report(ble_evt_t const* p_ble_evt) {

  ble_gap_evt_adv_report_t const* adv_report = &(p_ble_evt->evt.gap_evt.params.adv_report);
  // TODO: extract the fields we care about (Peer address and data)
  ble_gap_addr_t peer = adv_report -> peer_addr;
  ble_data_t data = adv_report -> data;
  uint8_t* p_data = data.p_data;

  // TODO: filter on Peer address
  if (peer.addr[5] == 0xC0 && peer.addr[4] == 0x98 && peer.addr[3] == 0xE5 &&
      peer.addr[2] == 0x49 && peer.addr[1] == 0x20 && peer.addr[0] == 0x43) {
    
    uint16_t index = 0;
    while (1) {
      uint8_t len = p_data[index];
      uint8_t type = p_data[index];
      if (type == 0x1b) {
        // for (int i = 0; i < len - 1; i ++) {
        //   printf("%x ", p_data[index + 2 + i]);
        // }
        // printf("\n");
        int robot_state = p_data[index + 4];
        if (robot_state == 0) {
          drive = false;
          back = false;
          left = false;
          right = false;
        }
        if (robot_state == 8) {
          drive = true;
          back = false;
          left = false;
          right = false;
        }
        if (robot_state == 4) {
          drive = false;
          back = true;
          left = false;
          right = false;
        }
        if (robot_state == 1) {
          drive = false;
          back = false;
          left = true;
          right = false;
        }
        if (robot_state == 2) {
          drive = false;
          back = false;
          left = false;
          right = true;
        }
        target_pos[index_target].x = p_data[index + 5] << 24 | p_data[index + 6] << 16 | 
                        p_data[index + 7] << 8 | p_data[index + 8];
        target_pos[index_target].y = p_data[index + 9] << 24 | p_data[index + 10] << 16 | 
                        p_data[index + 11] << 8 | p_data[index + 12];
        index_target = (index_target + 1) % len_target;
        
        if (chase == 0 && p_data[index + 13]) {
          stop_auto = false;
        }
        chase = p_data[index + 13];
        if (p_data[index + 14] == 2) {
          speed -= 25;
        }
        if (p_data[index + 14] == 1) {
          speed += 25;
        } 
        break;
      }
      index += len + 1;
      if (index >= data.len) break;
      // TODO: get length of field
      // TODO: get type of field: if type is 0xFF, we found it!
      // Print the data as a string. i.e. printf("%s\n", data + offset)
      // Otherwise, skip ahead by the length of the current field
    //}
    }
  }
}

void print_state(states current_state){
  nrf_drv_spi_uninit(&my_spi_instance);
  nrf_drv_spi_init(&spi_instance, &spi_config, NULL, NULL);
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
    case C_ROTATE: {
      display_write("CHASE", DISPLAY_LINE_0);
      break;
    }
    case C_DRIVE: {
      display_write("CHASE", DISPLAY_LINE_0);
      break;
    }
    }
  }
  if (get_pos_error) {
    display_write("POS_ERROR", DISPLAY_LINE_1);
  }
  else {
    display_write(" ", DISPLAY_LINE_1);
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

  error_code = nrf_drv_spi_init(&my_spi_instance, &my_spi_config, NULL, NULL);
  APP_ERROR_CHECK(error_code);
  printf("spi initialized!\n");

  error_code = dwm_reset(&my_spi_instance);
  APP_ERROR_CHECK(error_code);

  nrf_delay_ms(3000);

  // uint8_t rx_buf[4] = {0};
  // error_code = nrf_drv_spi_transfer(&my_spi_instance, NULL, 0, rx_buf, 2);
  // APP_ERROR_CHECK(error_code);
  // while (rx_buf[0] != 0xff) {
  //   error_code = nrf_drv_spi_transfer(&my_spi_instance, NULL, 0, rx_buf, 2);
  //   APP_ERROR_CHECK(error_code);
  //   if (error_code != NRF_SUCCESS) {
  //     printf("continuing spi error code: %d\n", (int) error_code);
  //   }
  // }

  // dwm_cfg_tag_t cfg_tag;
  // dwm_cfg_t cfg_node;

  // cfg_tag.low_power_en = 0;
  // cfg_tag.meas_mode = DWM_MEAS_MODE_TWR;
  // cfg_tag.loc_engine_en = 1;
  // cfg_tag.common.led_en = 1;
  // cfg_tag.common.ble_en = 1;
  // cfg_tag.common.uwb_mode = DWM_UWB_MODE_ACTIVE;
  // cfg_tag.common.fw_update_en = 1;

  // nrf_delay_ms(1000);

  // error_code = dwm_cfg_tag_set(&my_spi_instance, &cfg_tag);
  // APP_ERROR_CHECK(error_code);
  // printf("set cfg\n");

  // error_code = dwm_reboot(&my_spi_instance);
  // APP_ERROR_CHECK(error_code);

  // nrf_delay_ms(1000);

  // error_code = dwm_cfg_get(&my_spi_instance, &cfg_node);
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
  
  printf("!!!!\n");

  nrf_drv_spi_uninit(&my_spi_instance);
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

  states state = WAIT;  
  dwm_pos_t current_pos;
  double rotate_angle;
  double current_rotate_angle;
  float distance;
  float target_distance;
  dwm_pos_t previous_pos;
  dwm_pos_t target_pos_1;
  int left_right;

  kobukiInit();  
  printf("Kobuki initialized!\n");
  chase = false;
  while (1) {
    nrf_drv_spi_uninit(&spi_instance);
    error_code = nrf_drv_spi_init(&my_spi_instance, &my_spi_config, NULL, NULL);
    int k = dwm_pos_get(&my_spi_instance, &my_pos[index_my]);
    index_my = (index_my + k) % len_target;
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
            get_pos_error = false;
            kobukiDriveDirect(speed, speed);
          }
          else if (chase && !stop_auto) {
            state = C_DRIVE;
            get_pos_error = false;
            for (int i = 0; i < len_target; i ++) {
              pos_x[i] = my_pos[i].x;
              pos_y[i] = my_pos[i].y;
            }
            my_quick_sort(pos_x, 0, len_target - 1);
            my_quick_sort(pos_y, 0, len_target - 1);
            previous_pos.x = pos_x[(len_target + 1) / 2];
            previous_pos.y = pos_y[(len_target + 1) / 2];
            target_distance = 0.5;
            kobukiDriveDirect(0, 0);
            mpu9250_stop_gyro_integration();
            start_encoder = sensors.leftWheelEncoder;
            distance = 0;
          }
          else if (!drive && back && !right && !left) {
            state = BACK;
            get_pos_error = false;
            kobukiDriveDirect(-speed, -speed);
          }
          else if (!drive && !back && right && !left) {
            state = RIGHT;
            get_pos_error = false;
            kobukiDriveDirect(speed, -speed);
          }
          else if (!drive && !back && !right && left) {
            state = LEFT;
            get_pos_error = false;
            kobukiDriveDirect(-speed, speed);
          }
          else {
            state = WAIT;
            kobukiDriveDirect(0, 0);
          }
        }
        break; // each case needs to end with break!
      }
      case C_ROTATE: {
        print_state(state);
        if (is_button_pressed(&sensors) || !chase) {
          state = WAIT;
          stop_auto = true;
          kobukiDriveDirect(0, 0);
          mpu9250_stop_gyro_integration();
          break;
        }
        current_rotate_angle = mpu9250_read_gyro_integration().z_axis;
        if (abs(current_rotate_angle) >= rotate_angle) {
          state = C_DRIVE;
          kobukiDriveDirect(0, 0);
          target_distance = 0.5;
          mpu9250_stop_gyro_integration();
          start_encoder = sensors.leftWheelEncoder;
          distance = 0;
          state = C_DRIVE;
          for (int i = 0; i < len_target; i ++) {
            pos_x[i] = my_pos[i].x;
            pos_y[i] = my_pos[i].y;
          }
          my_quick_sort(pos_x, 0, len_target - 1);
          my_quick_sort(pos_y, 0, len_target - 1);
          previous_pos.x = pos_x[(len_target + 1) / 2];
          previous_pos.y = pos_y[(len_target + 1) / 2];
        }
        else {
          if (left_right == 1) {
            kobukiDriveDirect(-250, 250);
          }
          else {
            kobukiDriveDirect(250, -250);
          }
        }
        break;
      }
      case C_DRIVE: {
        print_state(state);
        if (is_button_pressed(&sensors) || !chase) {
          stop_auto = true;
          state = WAIT;
          kobukiDriveDirect(0, 0);
          break;
        }
        if (abs(distance) >= target_distance) {
          state = C_ROTATE;

          for (int i = 0; i < len_target; i ++) {
            pos_x[i] = my_pos[i].x;
            pos_y[i] = my_pos[i].y;
          }
          my_quick_sort(pos_x, 0, len_target - 1);
          my_quick_sort(pos_y, 0, len_target - 1);
          current_pos.x = pos_x[(len_target + 1) / 2];
          current_pos.y = pos_y[(len_target + 1) / 2];

          if (current_pos.x == previous_pos.x && current_pos.y == previous_pos.y) {
            stop_auto = true;
            get_pos_error = true;
            state = WAIT;
            kobukiDriveDirect(0, 0);
            break; 
          }

          if (chase == 1) {
            for (int i = 0; i < len_target; i ++) {
              pos_x[i] = target_pos[i].x;
              pos_y[i] = target_pos[i].y;
            }
            my_quick_sort(pos_x, 0, len_target - 1);
            my_quick_sort(pos_y, 0, len_target - 1);
            target_pos_1.x = pos_x[(len_target + 1) / 2];
            target_pos_1.y = pos_y[(len_target + 1) / 2];
          }
          else {
            target_pos_1.x = 0;
            target_pos_1.y = 0;
          }

          double a = sqrt((current_pos.x - previous_pos.x) * (current_pos.x - previous_pos.x) + 
                    (current_pos.y - previous_pos.y) * (current_pos.y - previous_pos.y));
          double b = sqrt((current_pos.x - target_pos_1.x) * (current_pos.x - target_pos_1.x) + 
                    (current_pos.y - target_pos_1.y) * (current_pos.y - target_pos_1.y));
          double c = sqrt((previous_pos.x - target_pos_1.x) * (previous_pos.x - target_pos_1.x) + 
                    (previous_pos.y - target_pos_1.y) * (previous_pos.y - target_pos_1.y));
          rotate_angle = acos((a * a + b * b - c * c) / (2*a*b));
          double vec1_x = current_pos.x - previous_pos.x;
          double vec1_y = current_pos.y - previous_pos.y;
          double vec2_x = target_pos_1.x - previous_pos.x;
          double vec2_y = target_pos_1.y - previous_pos.y;
          if (vec1_x * vec2_y - vec1_y * vec2_x > 0) {
            left_right = 1;
          }
          else {
            left_right = 0;
          }
          rotate_angle = PI - rotate_angle;
          rotate_angle = rotate_angle * 180.0 / PI;
          kobukiDriveDirect(0, 0);
          current_rotate_angle = 0;
          mpu9250_start_gyro_integration();
          // printf("!!%d %d %d %d %d %d\n", previous_pos.x, previous_pos.y, current_pos.x, current_pos.y, target_pos_1.x, target_pos_1.y);
          // printf("%d %f\n", left_right, rotate_angle);
        }
        else {
          distance = mesure_distance(sensors.leftWheelEncoder, start_encoder);
          kobukiDriveDirect(300, 300);
        }
        break;
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
            kobukiDriveDirect(speed, speed);
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
            kobukiDriveDirect(-speed, -speed);
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
            kobukiDriveDirect(-speed, speed);
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
            kobukiDriveDirect(speed, -speed);
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



