/*! ------------------------------------------------------------------------------------------------------------------
 * @file    dwm_api.h
 * @brief   DWM1001 host API header 
 *
 * @attention
 *
 * Copyright 2017 (c) Decawave Ltd, Dublin, Ireland.
 *
 * All rights reserved.
 *
 */ 
#include "nrf_drv_spi.h"
#include "nrf_drv_gpiote.h"
#include "my_dwm.h"
#include "buckler.h"
#include <string.h>
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "app_util_platform.h"
#include "nordic_common.h"
#include "app_uart.h"
#include "nrf_drv_clock.h"
#include "nrf_power.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#define RESP_ERRNO_LEN           3
#define RESP_DAT_TYPE_OFFSET     RESP_ERRNO_LEN
#define RESP_DAT_LEN_OFFSET      RESP_DAT_TYPE_OFFSET+1
#define RESP_DAT_VALUE_OFFSET    RESP_DAT_LEN_OFFSET+1


ret_code_t dwm_init(nrf_drv_spi_t* spi_instance) {
  // initialize spi
  nrf_drv_spi_config_t spi_config = {
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
  return nrf_drv_spi_init(spi_instance, &spi_config, NULL, NULL);
}

ret_code_t dwm_reboot(nrf_drv_spi_t* spi_instance) {
  ret_code_t error_code;
  uint8_t rx_buf[3] = {0};

  uint8_t tx_buf[2];
  tx_buf[0] = 0x14;
  tx_buf[1] = 0x00;

  while (1) {
    error_code = nrf_drv_spi_transfer(spi_instance, tx_buf, 2, NULL, 0);
    APP_ERROR_CHECK(error_code);

    while (1) {
      error_code = nrf_drv_spi_transfer(spi_instance, NULL, 0, rx_buf, 2);
      APP_ERROR_CHECK(error_code);
      if (error_code != NRF_SUCCESS) {
        printf("spi error code: %d\n", (int) error_code);
      }
      printf("!%x %x\n", rx_buf[0], rx_buf[1]);
      if (rx_buf[0] != 0 || rx_buf[1] != 0) {
        break;
      }
    }

    printf("size/num: %d %d\n", rx_buf[0], rx_buf[1]);
    if (rx_buf[0] != 3 || rx_buf[1] != 1) {
      continue; 
    }

    error_code = nrf_drv_spi_transfer(spi_instance, NULL, 0, rx_buf, 3);
    break;
  }
  return error_code;
}

ret_code_t dwm_reset(nrf_drv_spi_t* spi_instance) {
  uint8_t reset_buf[1];
  reset_buf[0] = 0xff;

  nrf_delay_ms(100);
  ret_code_t error_code = nrf_drv_spi_transfer(spi_instance, reset_buf, 1, NULL, 0);
  APP_ERROR_CHECK(error_code);
  nrf_delay_ms(100);
  error_code = nrf_drv_spi_transfer(spi_instance, reset_buf, 1, NULL, 0);
  APP_ERROR_CHECK(error_code);
  nrf_delay_ms(100);
  error_code = nrf_drv_spi_transfer(spi_instance, reset_buf, 1, NULL, 0);
  APP_ERROR_CHECK(error_code);
  return error_code;
}

void dwm_pos_set(dwm_pos_t* pos)
{
   // uint8_t tx_data[DWM1001_TLV_MAX_SIZE], tx_len = 0;
   // uint8_t rx_data[DWM1001_TLV_MAX_SIZE];
   // uint16_t rx_len;
   // tx_data[tx_len++] = DWM1001_TLV_TYPE_CMD_POS_SET;
   // tx_data[tx_len++] = 13;
   // *(uint32_t*)(tx_data+tx_len) = pos->x;
   // tx_len+=4;
   // *(uint32_t*)(tx_data+tx_len) = pos->y;
   // tx_len+=4;
   // *(uint32_t*)(tx_data+tx_len) = pos->z;
   // tx_len+=4;
   // tx_data[tx_len++] = pos->qf;
   // LMH_Tx(tx_data, &tx_len);
   // return LMH_WaitForRx(rx_data, &rx_len, 3);
}

int dwm_pos_get(nrf_drv_spi_t* spi_instance, dwm_pos_t* pos) {
  ret_code_t error_code;
  uint8_t data_cnt;
  uint8_t rx_buf[100] = {0};

  uint8_t tx_buf[2];
  tx_buf[0] = DWM1001_TLV_TYPE_CMD_POS_GET;
  // tx_buf[0] = 0x0C;
  tx_buf[1] = 0x00;

  // // while (1) {
  //   nrf_delay_ms(10);
    error_code = nrf_drv_spi_transfer(spi_instance, tx_buf, 2, NULL, 0);
    APP_ERROR_CHECK(error_code);

    while (1) {
      error_code = nrf_drv_spi_transfer(spi_instance, NULL, 0, rx_buf, 2);
      APP_ERROR_CHECK(error_code);
      if (error_code != NRF_SUCCESS) {
        printf("spi error code: %d\n", (int) error_code);
      }
      //printf("!%x %x\n", rx_buf[0], rx_buf[1]);
      if (rx_buf[0] != 0 || rx_buf[1] != 0) {
        break;
      }
    }
    printf("SIZE NUM: %d %d\n", rx_buf[0], rx_buf[1]);
    // if (rx_buf[0] != 81 || rx_buf[1] != 1) {
      if (rx_buf[0] != 255) {
        error_code = nrf_drv_spi_transfer(spi_instance, NULL, 0, rx_buf, rx_buf[0]);
      }
      else {
        return 0;
      }
      // return 0; 
    // }

    // error_code = nrf_drv_spi_transfer(spi_instance, NULL, 0, rx_buf, 81);

    if (rx_buf[2] == 0 && rx_buf[3] == 0x41) {
      data_cnt = 5;
      pos -> x = rx_buf[data_cnt]
            + (rx_buf[data_cnt+1]<<8)
            + (rx_buf[data_cnt+2]<<16)
            + (rx_buf[data_cnt+3]<<24);

      data_cnt += 4;
      pos -> y = rx_buf[data_cnt]
              + (rx_buf[data_cnt+1]<<8)
              + (rx_buf[data_cnt+2]<<16)
              + (rx_buf[data_cnt+3]<<24);
      //printf("%d %d\n", pos->x, pos->y);
      return 1;
    }
  // }
  return 0;
}

void dwm_upd_rate_set(uint16_t ur, uint16_t ur_static)
{
   // uint8_t tx_data[DWM1001_TLV_MAX_SIZE], tx_len = 0;
   // uint8_t rx_data[DWM1001_TLV_MAX_SIZE];
   // uint16_t rx_len;
   // tx_data[tx_len++] = DWM1001_TLV_TYPE_CMD_UR_SET;
   // tx_data[tx_len++] = 4;
   // tx_data[tx_len++] = ur & 0xff;
   // tx_data[tx_len++] = (ur>>8) & 0xff;   
   // tx_data[tx_len++] = ur_static & 0xff;
   // tx_data[tx_len++] = (ur_static>>8) & 0xff;   
   // LMH_Tx(tx_data, &tx_len);    
   // return LMH_WaitForRx(rx_data, &rx_len, 3);   
}

void dwm_upd_rate_get(uint16_t *ur, uint16_t *ur_static)
{
   // uint8_t tx_data[DWM1001_TLV_MAX_SIZE], tx_len = 0;
   // uint8_t rx_data[DWM1001_TLV_MAX_SIZE];
   // uint16_t rx_len;
   // uint8_t data_cnt;
   // tx_data[tx_len++] = DWM1001_TLV_TYPE_CMD_UR_GET;
   // tx_data[tx_len++] = 0;   
   // LMH_Tx(tx_data, &tx_len);   
   // if(LMH_WaitForRx(rx_data, &rx_len, 9) == RV_OK)
   // {
   //    data_cnt = RESP_DAT_VALUE_OFFSET;
   //    *ur = rx_data[data_cnt] + (rx_data[data_cnt+1]<<8);
   //    data_cnt += 2;
   //    *ur_static  = rx_data[data_cnt] + (rx_data[data_cnt+1]<<8);
   //    data_cnt += 2;
   //    return RV_OK;
   // }   
   // return RV_ERR;
}

ret_code_t dwm_cfg_tag_set(nrf_drv_spi_t* spi_instance, dwm_cfg_tag_t* cfg) {
  ret_code_t error_code = dwm_reset(spi_instance);
  
  uint8_t rx_buf[3] = {0};
  uint8_t tx_buf[4];
  tx_buf[0] = DWM1001_TLV_TYPE_CMD_CFG_TN_SET;
  tx_buf[1] = 0x02;
  tx_buf[3] = cfg -> stnry_en << 2 | cfg -> meas_mode;
  tx_buf[2] = cfg -> low_power_en << 7 | cfg -> loc_engine_en << 6 |
              cfg -> common.enc_en << 5 | cfg -> common.led_en << 4 | 
              cfg -> common.ble_en << 3 |
              cfg -> common.fw_update_en << 2 | 
              cfg -> common.uwb_mode;
  // tx_buf[2] = 0;
  // tx_buf[3] = 0;

  while (1) {
    nrf_delay_ms(2000);
    printf("tag cfg set\n");

    error_code = nrf_drv_spi_transfer(spi_instance, tx_buf, 4, NULL, 0);
    APP_ERROR_CHECK(error_code);

    while (1) {
      error_code = nrf_drv_spi_transfer(spi_instance, NULL, 0, rx_buf, 2);
      APP_ERROR_CHECK(error_code);
      if (error_code != NRF_SUCCESS) {
        printf("spi error code: %d\n", (int) error_code);
      }
      if (rx_buf[0] != 0 || rx_buf[1] != 0) {
        break;
      }
    }
    printf("SIZE NUM: %d %d\n", rx_buf[0], rx_buf[1]);
    if (rx_buf[0] == 0x40) {
      error_code = dwm_reset(spi_instance);
      APP_ERROR_CHECK(error_code);
    }
    else {
      error_code = nrf_drv_spi_transfer(spi_instance, NULL, 0, rx_buf, 3);
      APP_ERROR_CHECK(error_code);
      if (rx_buf[2] == 0) break;
    }
  }
  return error_code;
}

ret_code_t dwm_cfg_get(nrf_drv_spi_t* spi_instance, dwm_cfg_t* cfg) {
  // ret_code_t error_code = dwm_reset(spi_instance);
  ret_code_t error_code;
  uint8_t rx_buf[10] = {0};
  uint8_t tx_buf[2];
  tx_buf[0] = DWM1001_TLV_TYPE_CMD_CFG_GET;
  tx_buf[1] = 0x00;
  
  while (1) {
    nrf_delay_ms(2000);
    printf("tag cfg get\n");

    error_code = nrf_drv_spi_transfer(spi_instance, tx_buf, 2, NULL, 0);
    APP_ERROR_CHECK(error_code);

    while (1) {
      error_code = nrf_drv_spi_transfer(spi_instance, NULL, 0, rx_buf, 2);
      APP_ERROR_CHECK(error_code);
      if (error_code != NRF_SUCCESS) {
        printf("spi error code: %d\n", (int) error_code);
      }
      if (rx_buf[0] != 0 || rx_buf[1] != 0) {
        break;
      }
    }
    printf("SIZE NUM: %d %d\n", rx_buf[0], rx_buf[1]);
    if (rx_buf[0] == 0x40) {
      error_code = dwm_reset(spi_instance);
      APP_ERROR_CHECK(error_code);
    }
    else {
      error_code = nrf_drv_spi_transfer(spi_instance, NULL, 0, rx_buf, 7);
      APP_ERROR_CHECK(error_code);
      if (rx_buf[2] == 0 && rx_buf[3] == 0x46) {
        cfg -> common.uwb_mode = rx_buf[5] & 0x03;
        cfg -> common.fw_update_en = (rx_buf[5] & 0x04) >> 2;
        cfg -> common.ble_en = (rx_buf[5] & 0x08) >> 3;
        cfg -> common.led_en = (rx_buf[5] & 0x10) >> 4;
        cfg -> common.enc_en = (rx_buf[5] & 0x20) >> 5;
        cfg -> loc_engine_en = (rx_buf[5] & 0x40) >> 6;
        cfg -> low_power_en = (rx_buf[5] & 0x80) >> 7;
        cfg -> meas_mode = rx_buf[6] & 0x03;
        cfg -> stnry_en = (rx_buf[6] & 0x04) >> 2;
        break;        
      }
    }
  }
  return error_code;
}

void my_quick_sort(int arr[], int left, int right) {
  if (left < right) {
    int k = arr[left];
    int i = left, j = right;
    while (i < j) {
      while (arr[j] > k && j > i) {
        j --;
      }
      if (i < j) {
        arr[i ++] = arr[j];
      }
      while (i < j && arr[i] < k) {
        i ++;
      }
      if (i < j) {
        arr[j --] = arr[i];
      }
    }
    arr[i] = k;
    my_quick_sort(arr, left, j - 1);
    my_quick_sort(arr, i + 1, right);
  }
}