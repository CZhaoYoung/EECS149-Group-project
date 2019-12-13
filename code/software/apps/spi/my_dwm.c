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

#define RESP_ERRNO_LEN           3
#define RESP_DAT_TYPE_OFFSET     RESP_ERRNO_LEN
#define RESP_DAT_LEN_OFFSET      RESP_DAT_TYPE_OFFSET+1
#define RESP_DAT_VALUE_OFFSET    RESP_DAT_LEN_OFFSET+1

static const nrf_drv_spi_t* spi_instance;
static nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;

void dwm_init(const nrf_drv_spi_t* instance) {
   spi_instance = instance;

   spi_config.sck_pin    = SD_CARD_SPI_SCLK;
   spi_config.miso_pin   = SD_CARD_SPI_MISO;
   spi_config.mosi_pin   = SD_CARD_SPI_MOSI;
   spi_config.ss_pin     = SD_CARD_SPI_CS;
   spi_config.frequency  = NRF_DRV_SPI_FREQ_2M;
   spi_config.mode       = NRF_DRV_SPI_MODE_0;
   spi_config.bit_order  = NRF_DRV_SPI_BIT_ORDER_MSB_FIRST;

   // nrf_gpio_cfg_output(RTC_WDI);
   // nrf_gpio_pin_set(RTC_WDI);
}

void read_reg(uint8_t reg, uint8_t* read_buf, size_t len){
  if (len > 256) return;
  uint8_t readreg = reg;
  uint8_t buf[257];

  nrf_drv_spi_init(spi_instance, &spi_config, NULL, NULL);
  nrf_drv_spi_transfer(spi_instance, &readreg, 1, buf, len+1);
  nrf_drv_spi_uninit(spi_instance);

  memcpy(read_buf, buf+1, len);
}

void write_reg(uint8_t reg, uint8_t* write_buf, size_t len){
  if (len > 256) return;
  uint8_t buf[257];
  buf[0] = 0x80 | reg;
  memcpy(buf+1, write_buf, len);

  nrf_drv_spi_init(spi_instance, &spi_config, NULL, NULL);
  nrf_drv_spi_transfer(spi_instance, buf, len+1, NULL, 0);
  nrf_drv_spi_uninit(spi_instance);
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

void dwm_pos_get(dwm_pos_t* p_pos)
{
   // uint8_t tx_data[DWM1001_TLV_MAX_SIZE], tx_len = 0;
   // uint8_t rx_data[DWM1001_TLV_MAX_SIZE];
   // uint16_t rx_len;
   // uint8_t data_cnt;
   // tx_data[tx_len++] = DWM1001_TLV_TYPE_CMD_POS_GET;
   // tx_data[tx_len++] = 0;   
   // LMH_Tx(tx_data, &tx_len);   
   // if(LMH_WaitForRx(rx_data, &rx_len, 18) == RV_OK)
   // {
   //    data_cnt = RESP_DAT_VALUE_OFFSET;
   //    p_pos->x = rx_data[data_cnt] 
   //           + (rx_data[data_cnt+1]<<8) 
   //           + (rx_data[data_cnt+2]<<16) 
   //           + (rx_data[data_cnt+3]<<24); 
   //    data_cnt += 4;
   //    p_pos->y = rx_data[data_cnt] 
   //           + (rx_data[data_cnt+1]<<8) 
   //           + (rx_data[data_cnt+2]<<16) 
   //           + (rx_data[data_cnt+3]<<24); 
   //    data_cnt += 4;
   //    p_pos->z = rx_data[data_cnt] 
   //           + (rx_data[data_cnt+1]<<8) 
   //           + (rx_data[data_cnt+2]<<16) 
   //           + (rx_data[data_cnt+3]<<24); 
   //    data_cnt += 4;
   //    p_pos->qf = rx_data[data_cnt];
   //    return RV_OK;
   // }   
   // return RV_ERR;
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

void dwm_cfg_tag_set(dwm_cfg_tag_t* cfg) 
{
   uint8_t write[2];

   write[1] = cfg -> stnry_en << 2 | cfg -> meas_mode;
   write[0] = cfg -> low_power_en << 7 | cfg -> loc_engine_en << 6 |
      cfg -> common.enc_en << 5 | cfg -> common.led_en << 4 | 
      cfg -> common.ble_en << 3 |
      cfg -> common.fw_update_en << 2 | 
      cfg -> common.uwb_mode;
   write_reg(DWM1001_TLV_TYPE_CFG, write, 2);  
}

void dwm_cfg_get(dwm_cfg_t* cfg)
{
   uint8_t read[2];

   read_reg(DWM1001_TLV_TYPE_CMD_CFG_GET, read, 2);

   cfg->stnry_en = (read[1] & 0x04) >> 2;
   cfg->meas_mode = (read[1] & 0x03);
   cfg->low_power_en = (read[0] & 0x80) >> 7;
   cfg->loc_engine_en = (read[0] & 0x40) >> 6;
   cfg->common.enc_en = (read[0] & 0x20) >> 5;
   cfg->common.led_en = (read[0] & 0x10) >> 4;
   cfg->common.ble_en = (read[0] & 0x08) >> 3;
   cfg->common.fw_update_en = (read[0] & 0x04) >> 2;
   cfg->common.uwb_mode = read[0] & 0x03;
}