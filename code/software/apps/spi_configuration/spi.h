// Datasheet: https://www.decawave.com/sites/default/files/dwm1001_datasheet.pdf
#pragma once
#include "time.h"

#include "nrf_spi_mngr.h"
#include "buckler.h"

// Time and date registers
#define AB1815_HUND           0x00
#define UWB_X                 0x01
#define UWB_Y                 0x02
#define UWB_Z                 0x03
#define UWB_DQF               0x04

// Config registers
#define UWB_STATUS             0x0F
#define UWB_CONTROL1           0x10
#define UWB_CONTROL2           0x11
#define UWB_INT_MASK           0x12
#define UWB_SQW                0x13

// Calibration registers
#define AB1815_CALIB_XT           0x14
#define AB1815_CALIB_RC_UP        0x15
#define AB1815_CALIB_RC_LO        0x16
// Sleep control register
#define AB1815_SLEEP              0x17
// Timer registers
#define AB1815_COUNTDOWN_CTRL     0x18
#define AB1815_COUNTDOWN_TIMER    0x19
#define AB1815_TIMER_INIT_VAL     0x1A
#define AB1815_WATCHDOG_TIMER     0x1B
// Oscillator registers
#define AB1815_OSCILLATOR_CTRL    0x1C
#define AB1815_OSCILLATOR_STATUS  0x1D
#define AB1815_OSCILLATOR_KEY     0x1F

typedef void ab1815_alarm_callback(void);

typedef enum {
  _16HZ = 0,
  _4HZ,
  _1HZ,
  _1_4HZ,
} ab1815_watch_clock_freq;

typedef enum {
  HUNDREDTH_MATCH = 0x7,
  ONCE_PER_MINUTE = 0x6,
  ONCE_PER_HOUR   = 0x5,
  ONCE_PER_DAY    = 0x4,
  ONCE_PER_WEEK   = 0x3,
  ONCE_PER_MONTH  = 0x2,
  ONCE_PER_YEAR   = 0x1,
  DISABLED        = 0x0,
} ab1815_alarm_repeat;

typedef struct {
  bool stnry_en;
  bool low_power_en;
  bool loc_engine_en;
  bool enc_en;
  bool led_en;
  bool ble_en;
  bool fw_updata_en;
  uint8_t uwb_mode; // ??
  uint8_t meas_mode;  // ??
} uwb_control_t;

typedef struct 
{
  uint32_t x;
  uint32_t y;
  uint32_t z;
  uint8_t pqf;  // position quality factor in percent
} uwb_position; // posotion is 13 byte position array 

// typedef struct {
//   bool stop;
//   bool hour_12;
//   bool OUTB;
//   bool OUT;
//   bool rst_pol;
//   bool auto_rst;
//   bool write_rtc;
//   uint8_t psw_nirq2_function;
//   uint8_t fout_nirq_function;
// } ab1815_control_t;


typedef struct {
  bool century_en;
  uint8_t int_mode;
  bool bat_low_en;
  bool timer_en;
  bool alarm_en;
  bool xt2_en;
  bool xt1_en;
} ab1815_int_config_t;

typedef struct {
  uint8_t hundredths;
  uint8_t seconds;
  uint8_t minutes;
  uint8_t hours;
  uint8_t date;
  uint8_t months;
  uint8_t years;
  uint8_t weekday;
} ab1815_time_t;

void ab1815_init(const nrf_drv_spi_t* instance);
void ab1815_get_config(ab1815_control_t* config);
void ab1815_set_config(ab1815_control_t config);
void ab1815_interrupt_config(ab1815_int_config_t config);
ab1815_time_t unix_to_ab1815(struct timeval tv);
struct timeval ab1815_to_unix(ab1815_time_t time);
void ab1815_set_time(ab1815_time_t time);
void ab1815_get_time(ab1815_time_t* time);
struct timeval ab1815_get_time_unix(void);
void ab1815_set_alarm(ab1815_time_t time, ab1815_alarm_repeat repeat, ab1815_alarm_callback* cb);
void ab1815_set_watchdog(bool reset, uint8_t clock_cycles, uint8_t clock_frequency);
void ab1815_tickle_watchdog(void);
void ab1815_clear_watchdog(void);

