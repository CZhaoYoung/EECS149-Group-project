// Robot Template app
//
// Framework for creating applications that control the Kobuki robot

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "app_error.h"
#include "app_timer.h"
#include "nrf.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
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

#define PI 3.14159265

// I2C manager
NRF_TWI_MNGR_DEF(twi_mngr_instance, 5, 0);

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

// global variables
KobukiSensors_t sensors = {0};

// Intervals for advertising and connections
static simple_ble_config_t ble_config = {
        // c0:98:e5:49:xx:xx
        .platform_id       = 0x49,    // used as 4th octect in device BLE address
        .device_id         = 0x2043, // TODO: replace with your lab bench number
        .adv_name          = "FFJMMM", // used in advertisements if there is room
        .adv_interval      = MSEC_TO_UNITS(1000, UNIT_0_625_MS),
        .min_conn_interval = MSEC_TO_UNITS(100, UNIT_1_25_MS),
        .max_conn_interval = MSEC_TO_UNITS(200, UNIT_1_25_MS),
};

//4607eda0-f65e-4d59-a9ff-84420d87a4ca
static simple_ble_service_t robot_service = {{
    .uuid128 = {0xca,0xa4,0x87,0x0d,0x42,0x84,0xff,0xA9,
                0x59,0x4D,0x5e,0xf6,0xa0,0xed,0x07,0x46}
}};

// TODO: Declare characteristics and variables for your service
static simple_ble_char_t robot_state_char = {.uuid16 = 0xeda1};
static int robot_state;
bool drive = false;
bool back = false;
bool left = false;
bool right = false;
bool get_dir = false;
bool stop = false;
bool alone = false;
bool chase = false;
bool speed_up = false;
bool speed_down = false;
bool home = false;

simple_ble_app_t* simple_ble_app;

void ble_evt_write(ble_evt_t const* p_ble_evt) {
    // TODO: logic for each characteristic and related state changes

	 printf("%d\n", robot_state);
    if (robot_state == 11) {
      home = !home;
    }
    if (robot_state == 33) {
      speed_down = true;
    }
    else {
      speed_down = false;
    }
    if (robot_state == 22) {
      speed_up = true;
    }
    else {
      speed_up = false;
    }
    if (robot_state == 128) {
      chase = !chase;
    }
    if (robot_state == 64) {
      alone = !alone;
    }
    if (robot_state == 32) {
      stop = !stop;
    }
    if (robot_state == 16) {
      drive = false;
      back = false;
      left = false;
      right = false;
      get_dir = true;
    }
    if (robot_state == 8) {
    	drive = true;
    	back = false;
    	left = false;
    	right = false;
      get_dir = false;
    }
    if (robot_state == 4) {
    	drive = false;
    	back = true;
    	left = false;
    	right = false;
      get_dir = false;
    }
    if (robot_state == 1) {
    	drive = false;
    	back = false;
    	left = true;
    	right = false;
      get_dir = false;
    }
    if (robot_state == 2) {
    	drive = false;
    	back = false;
    	left = false;
    	right = true;
      get_dir = false;
    }
    if (robot_state == 0) {
    	drive = false;
    	back = false;
    	left = false;
    	right = false;
      get_dir = false;
    }
}

void print_state(states current_state){
	// nrf_drv_spi_uninit(&my_spi_instance);
 //  nrf_drv_spi_init(&spi_instance, &spi_config, NULL, NULL);
 //  switch(current_state){
 //    case OFF: {
	// 	  display_write("OFF", DISPLAY_LINE_0);
 //      break;
 //    }
 //    case DRIVE: {
 //  		display_write("DRIVE", DISPLAY_LINE_0);
 //  		break;
 //    }
 //    case BACK: {
 //  		display_write("BACK", DISPLAY_LINE_0);
 //  		break;
 //    }
 //    case LEFT: {
 //  		display_write("LEFT", DISPLAY_LINE_0);
 //  		break;
 //    }
 //    case RIGHT: {
 //  		display_write("RIGHT", DISPLAY_LINE_0);
 //      break;
 //    }
 //    case WAIT: {
 //  		display_write("WAIT", DISPLAY_LINE_0);
 //  		break;
 //    }
 //    case GET_DIR: {
 //      display_write("GET_DIR", DISPLAY_LINE_0);
 //      break;
 //    }
 //    case ROTATE: {
 //      display_write("ROTATE", DISPLAY_LINE_0);
 //      break;
 //    }
	// }
  
}

int main(void) {
  ret_code_t error_code = NRF_SUCCESS;

  // initialize RTT library
  error_code = NRF_LOG_INIT(NULL);
  APP_ERROR_CHECK(error_code);
  NRF_LOG_DEFAULT_BACKENDS_INIT();
  printf("Log initialized!\n");
  
  error_code = nrf_drv_spi_init(&my_spi_instance, &my_spi_config, NULL, NULL);
  APP_ERROR_CHECK(error_code);
  printf("spi initialized!\n");

  nrf_delay_ms(5000);

  // error_code = dwm_hard_reset(&my_spi_instance);
  // APP_ERROR_CHECK(error_code);

  error_code = dwm_reset(&my_spi_instance);
  APP_ERROR_CHECK(error_code);

  uint8_t rx_buf[4] = {0};
  error_code = nrf_drv_spi_transfer(&my_spi_instance, NULL, 0, rx_buf, 2);
  APP_ERROR_CHECK(error_code);
  while (rx_buf[0] != 0xff) {
    error_code = nrf_drv_spi_transfer(&my_spi_instance, NULL, 0, rx_buf, 2);
    APP_ERROR_CHECK(error_code);
    if (error_code != NRF_SUCCESS) {
      printf("continuing spi error code: %d\n", (int) error_code);
    }
  }

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
  simple_ble_app = simple_ble_init(&ble_config);

  simple_ble_add_service(&robot_service);

  // TODO: Register your characteristics
  simple_ble_add_characteristic(1, 1, 0, 0, 
  	sizeof(robot_state), (uint8_t*)&robot_state,
  	&robot_service, &robot_state_char);

  // Start Advertising
  //simple_ble_adv_only_name();
  uint8_t x[15] = {0};
  simple_ble_adv_manuf_data(x, sizeof(x));

  // initialize LEDs
  nrf_gpio_pin_dir_set(23, NRF_GPIO_PIN_DIR_OUTPUT);
  nrf_gpio_pin_dir_set(24, NRF_GPIO_PIN_DIR_OUTPUT);
  nrf_gpio_pin_dir_set(25, NRF_GPIO_PIN_DIR_OUTPUT);

  //initialize display
  // nrf_drv_spi_uninit(&my_spi_instance);
  // error_code = nrf_drv_spi_init(&spi_instance, &spi_config, NULL, NULL);
  // APP_ERROR_CHECK(error_code);
  // display_init(&spi_instance);
  // printf("Display initialized!\n");

  // initialize i2c master (two wire interface)
  nrf_drv_twi_config_t i2c_config = NRF_DRV_TWI_DEFAULT_CONFIG;
  i2c_config.scl = BUCKLER_SENSORS_SCL;
  i2c_config.sda = BUCKLER_SENSORS_SDA;
  i2c_config.frequency = NRF_TWIM_FREQ_100K;
  error_code = nrf_twi_mngr_init(&twi_mngr_instance, &i2c_config);
  APP_ERROR_CHECK(error_code);
  mpu9250_init(&twi_mngr_instance);
  printf("IMU initialized!\n");


  states state = WAIT;
  dwm_pos_t start_pos;
  dwm_pos_t current_pos;
  int len_list = 11;
  dwm_pos_t pos_list[len_list];
  int pos_list_sort_x[len_list];
  int pos_list_sort_y[len_list];
  int index_pos_list = 0;
  double rotate_angle;
  double current_rotate_angle;
  
  // nrf_drv_spi_uninit(&spi_instance);
  // error_code = nrf_drv_spi_init(&my_spi_instance, &my_spi_config, NULL, NULL);
  // while (index_pos_list < len_list) {
  //    int k = dwm_pos_get(&my_spi_instance, &pos_list[index_pos_list]);
  //    index_pos_list += k;
  // }
  //dwm_pos_get(&my_spi_instance, &current_pos);
  index_pos_list = 0;
  kobukiInit();
  printf("Kobuki initialized!\n");
  printf("start state machine\n");

  while (1) {
  	kobukiSensorPoll(&sensors);
    dwm_pos_get(&my_spi_instance, &current_pos);
    // printf("%ld %ld\n", current_pos.x, current_pos.y);
    //int k = dwm_pos_get(&my_spi_instance, &pos_list[index_pos_list]);
    //current_pos = pos_list[index_pos_list];
    //index_pos_list = (index_pos_list + k) % len_list;
    if (!alone) {
      if (speed_up) x[10] = 1;
      else if (speed_down) x[10] = 2;
      else x[10] = 0;
      if (chase) x[9] = 1;
      else if (home) x[9] = 2;
      else x[9] = 0;
      if (state == WAIT) x[0] = 0;
      else if (state == DRIVE) x[0] = 8;
      else if (state == BACK) x[0] = 4;
      else if (state == RIGHT) x[0] = 2;
      else if (state == LEFT) x[0] = 1;
      //dwm_pos_get(&my_spi_instance, &current_pos);
      x[1] = (current_pos.x >> 24);
      x[2] = (current_pos.x >> 16) & 0xff;
      x[3] = (current_pos.x >> 8) & 0xff;
      x[4] = current_pos.x & 0xff;
      x[5] = (current_pos.y >> 24);
      x[6] = (current_pos.y >> 16) & 0xff;
      x[7] = (current_pos.y >> 8) & 0xff;
      x[8] = current_pos.y & 0xff;
      simple_ble_adv_manuf_data((uint8_t*)x, sizeof(x));
    }
    
   //  nrf_drv_spi_uninit(&spi_instance);
   //  error_code = nrf_drv_spi_init(&my_spi_instance, &my_spi_config, NULL, NULL);
    // int k = dwm_pos_get(&my_spi_instance, &pos_list[index_pos_list]);
    // index_pos_list = (index_pos_list + k) % len_list;
    // nrf_delay_ms(100);
    // dwm_pos_get(&my_spi_instance, &current_pos);

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
          else if (get_dir) {
            state = GET_DIR;
            kobukiDriveDirect(0, 0);
            while (index_pos_list < len_list) {
              int k = dwm_pos_get(&my_spi_instance, &pos_list[index_pos_list]);
              index_pos_list += k;
            }
            index_pos_list = 0;
            for (int i = 0; i < len_list; i ++) {
              pos_list_sort_x[i] = pos_list[i].x;
              pos_list_sort_y[i] = pos_list[i].y;
            }
            my_quick_sort(pos_list_sort_x, 0, len_list - 1);
            my_quick_sort(pos_list_sort_y, 0, len_list - 1);
            start_pos.x = pos_list_sort_x[(len_list + 1) / 2];
            start_pos.y = pos_list_sort_y[(len_list + 1) / 2];
            //dwm_pos_get(&my_spi_instance, &start_pos);
          }
          else {
          	state = WAIT;
          	kobukiDriveDirect(0, 0);
          }
        }
        break; // each case needs to end with break!
      }
      case GET_DIR: {
        print_state(state);
        if (is_button_pressed(&sensors)) {
          state = OFF;
          kobukiDriveDirect(0, 0);
          break;
        }
        int k = dwm_pos_get(&my_spi_instance, &pos_list[index_pos_list]);
        index_pos_list += k;
        //dwm_pos_get(&my_spi_instance, &current_pos);
        for (int i = 0; i < len_list; i ++) {
          pos_list_sort_x[i] = pos_list[i].x;
          pos_list_sort_y[i] = pos_list[i].y;
        }
        my_quick_sort(pos_list_sort_x, 0, len_list - 1);
        my_quick_sort(pos_list_sort_y, 0, len_list - 1);
        current_pos.x = pos_list_sort_x[(len_list + 1) / 2];
        current_pos.y = pos_list_sort_y[(len_list + 1) / 2];
        // for (int i = 0; i < len_list; i ++) {
        //   printf("%d %d\n", pos_list[i].x, pos_list[i].y);
        // }
        int distance = (current_pos.x - start_pos.x) * (current_pos.x - start_pos.x) +
                        (current_pos.y - start_pos.y) * (current_pos.y - start_pos.y);
        printf("[%ld %ld], [%ld %ld]\n", current_pos.x, current_pos.y,
                                         start_pos.x, start_pos.y);
        printf("%d\n", distance);
        if (distance < 700 * 700) { 
          kobukiDriveDirect(50, 50);
        }
        else {
          kobukiDriveDirect(0, 0);
          double d_x = current_pos.x - start_pos.x;
          double d_y = current_pos.y - start_pos.y;
          rotate_angle = atan(d_y / d_x);
          if (d_x < 0) rotate_angle += PI;
          if (rotate_angle < 0) rotate_angle = 2*PI + rotate_angle;
          rotate_angle = rotate_angle * 180.0 / PI;
          state = ROTATE;
          current_rotate_angle = 0;
          mpu9250_start_gyro_integration();
        }
        break;
      }
      case ROTATE: {
        print_state(state);
        if (is_button_pressed(&sensors)) {
          state = OFF;
          kobukiDriveDirect(0, 0);
          mpu9250_stop_gyro_integration();
          break;
        }
        current_rotate_angle = mpu9250_read_gyro_integration().z_axis;
        // char buf[10];
        // gcvt(current_rotate_angle, 6, buf);
        // display_write(buf, DISPLAY_LINE_1);
 
        printf("!!!%f %f\n", current_rotate_angle, rotate_angle);
        if (abs(current_rotate_angle) >= rotate_angle) {
          state = WAIT;
          kobukiDriveDirect(0, 0);
          mpu9250_stop_gyro_integration();
        }
        else {
          kobukiDriveDirect(50, -50);
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
            if (!stop)
          	 kobukiDriveDirect(100, 100);
            else
              kobukiDriveDirect(0, 0);
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
            if (!stop)
          	 kobukiDriveDirect(-100, -100);
            else
              kobukiDriveDirect(0, 0);
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
            if (!stop)
          	 kobukiDriveDirect(-100, 100);
            else
              kobukiDriveDirect(0, 0);
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
            if (!stop)
          	 kobukiDriveDirect(100, -100);
            else
              kobukiDriveDirect(0, 0);
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