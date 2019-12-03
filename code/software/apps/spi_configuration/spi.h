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








void ab1815_init(const nrf_drv_spi_t* instance)
void ab1815_set_config(ab1815_control_t config)
void ab1815_get_config(ab1815_control_t* config)
void ab1815_interrupt_config(ab1815_int_config_t config)
static void interrupt_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)

void  ab1815_read_reg(uint8_t reg, uint8_t* read_buf, size_t len)
void ab1815_write_reg(uint8_t reg, uint8_t* write_buf, size_t len)


void dwm_get_loc()