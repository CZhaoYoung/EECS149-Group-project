//
#pragma once
#include "time.h"

#include "nrf_spi_mngr.h"
#include "dwm_api.h"

// position registers
#define UWB_x                0x00
#define UWB_y                0x01
#define UWB_z                0x02

// sleep registers
#define AB1815_SLEEP              0x03

// Config registers
#define AB1815_STATUS             0x0F
#define AB1815_CONTROL1           0x10
#define AB1815_CONTROL2           0x11
#define AB1815_INT_MASK           0x12
#define AB1815_SQW                0x13

typedef struct {
  bool stop;
  bool hour_12;
  bool OUTB;
  bool OUT;
  bool rst_pol;
  bool auto_rst;
  bool write_rtc;
  uint8_t psw_nirq2_function;
  uint8_t fout_nirq_function;
} UWB_control_t;

typedef struct {
  bool century_en;
  uint8_t int_mode;
  bool bat_low_en;
  bool timer_en;
  bool alarm_en;
  bool xt2_en;
  bool xt1_en;
} UWB_int_config_t;


typedef struct {
  uint8_t x;
  uint8_t y;
  uint8_t z;
} UWB_position_t;






void UWB_init(const nrf_drv_spi_t* instance)
void UWB_set_config(UWB_control_t config)
void UWB_get_config(UWB_control_t* config)
void UWB_interrupt_config(UWB_int_config_t config)
static void interrupt_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)

void  UWB_read_reg(uint8_t reg, uint8_t* read_buf, size_t len)
void UWB_write_reg(uint8_t reg, uint8_t* write_buf, size_t len)


void dwm_get_loc()