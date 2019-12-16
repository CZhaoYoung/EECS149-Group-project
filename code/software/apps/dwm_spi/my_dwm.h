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

#ifndef _DWM_API_H_
#define _DWM_API_H_

#include "dwm1001_tlv.h"

/**
 * @brief DWM Error codes, returned from DWM1001 module
 */
#define DWM_OK				   (0)
#define DWM_ERR_INTERNAL	(-1)
#define DWM_ERR_BUSY		   (-2)
#define DWM_ERR_INVAL_ADDR	(-3)
#define DWM_ERR_INVAL_PARAM   (-4)
#define DWM_ERR_OVERRUN		(-5)
#define DWM_ERR_I2C_ANACK	(-10)
#define DWM_ERR_I2C_DNACK	(-11)

/**
 * @brief Return value Error codes, returned from DWM API functions
 */
#define RV_OK           (0)      /*  ret value OK */
#define RV_ERR          (1)      /*  ret value ERROR: unknown command or broken tlv frame */
#define RV_ERR_INTERNAL (2)      /*  ret value ERROR: internal error */
#define RV_ERR_PARAM    (3)      /*  ret value ERROR: invalid parameter */
#define RV_ERR_BUSY     (4)      /*  ret value ERROR: busy  */
#define RV_ERR_PERMIT   (5)      /*  ret value ERROR: operation not permitted */


#define DWM_API_USR_DATA_LEN_MAX 		34
#define DWM_API_LABEL_LEN_MAX 			16
#define DWM_API_UWB_SCAN_RESULT_CNT_MAX 16
#define DWM_API_BH_ORIGIN_CNT_MAX 		9
/********************************************************************************************************************/
/*                                                     API LIST                                                     */
/********************************************************************************************************************/

/**
 * @brief Initializes the required components for DWM1001 module, 
 *       especially the Low-level Module Handshake (LMH) devides. 
 *
 * @param[in] none
 *
 * @return none
 */
ret_code_t dwm_init(nrf_drv_spi_t* spi_instance);

ret_code_t dwm_reset(nrf_drv_spi_t* spi_instance);

/**
 * @brief Position coordinates in millimeters + quality factor
 */
typedef struct {
	int32_t x;
	int32_t y;
	int32_t z;
	uint8_t qf;
} dwm_pos_t;
/**
 * @brief Sets position of anchor node
 *
 * @param[in] pos, pointer to position coordinates
 *
 * @return Error code
 */
void dwm_pos_set(dwm_pos_t* pos);

/**
 * @brief Gets position of the node
 *
 * @param[out] pos, pointer to position
 *
 * @return Error code
 */
ret_code_t dwm_pos_get(nrf_drv_spi_t* spi_instance, dwm_pos_t* pos);

/* maximum and minimum update rate in multiple of 100 ms */
enum dwm_upd_rate{
	DWM_UPD_RATE_MAX = 600,	/* 1 minute */
	DWM_UPD_RATE_MIN = 1	/* 100 ms */
};
/**
 * @brief Sets update rate
 *
 * @param[in] ur, Update rate in multiply of 100 ms, [min,max] = [DWM_UPD_RATE_MIN, DWM_UPD_RATE_MAX]
 * @param[in] urs, Stationary update rate in multiply of 100 ms, [min,max] = [DWM_UPD_RATE_MIN, DWM_UPD_RATE_MAX]
 *
 * @return Error code
 */
void dwm_upd_rate_set(uint16_t ur, uint16_t urs);

/**
 * @brief Gets update rate
 *
 * @param[out] ur, Pointer to update rate, update rate is multiply of 100 ms
 * [min,max] = [DWM_UPD_RATE_MIN, DWM_UPD_RATE_MAX]
 * @param[out] urs, Pointer to stationary update rate, update rate is multiply of 100 ms
 * [min,max] = [DWM_UPD_RATE_MIN, DWM_UPD_RATE_MAX]
 *
 * @return Error code
 */
void dwm_upd_rate_get(uint16_t *ur, uint16_t *urs);


/**
 * @brief Position measurement modes
 */
typedef enum {
	DWM_MEAS_MODE_TWR = 0,//!< DWM_MEAS_MODE_TWR
	DWM_MEAS_MODE_TDOA = 1//!< DWM_MEAS_MODE_TDOA
} dwm_meas_mode_t;

/**
 * @brief Device modes
 */
typedef enum {
	DWM_MODE_TAG = 0,  //!< DWM_MODE_TAG
	DWM_MODE_ANCHOR = 1//!< DWM_MODE_ANCHOR
} dwm_mode_t;

typedef enum {
	DWM_UWB_MODE_OFF = 0,
	DWM_UWB_MODE_PASSIVE = 1,
	DWM_UWB_MODE_ACTIVE = 2
}dwm_uwb_mode_t;

typedef enum {
	DWM_UWB_BH_ROUTING_OFF = 0,
	DWM_UWB_BH_ROUTING_ON = 1,
	DWM_UWB_BH_ROUTING_AUTO = 2,
} dwm_uwb_bh_routing_t;

typedef struct dwm_cfg_common {
	dwm_uwb_mode_t uwb_mode;
	bool fw_update_en;
	bool ble_en;
	bool led_en;
	bool enc_en;
} dwm_cfg_common_t;

typedef struct dwm_cfg_anchor {
	dwm_cfg_common_t common;
	bool bridge;
	bool initiator;
	dwm_uwb_bh_routing_t uwb_bh_routing;
} dwm_cfg_anchor_t;

typedef struct dwm_cfg_tag {
	dwm_cfg_common_t common;
	bool loc_engine_en;
	bool low_power_en;
	bool stnry_en;
	dwm_meas_mode_t meas_mode;
} dwm_cfg_tag_t;

typedef struct dwm_cfg {
	dwm_cfg_common_t common;
	bool loc_engine_en;
	bool low_power_en;
	bool stnry_en;
	dwm_meas_mode_t meas_mode;
	dwm_uwb_bh_routing_t uwb_bh_routing;
	bool bridge;
	bool initiator;
	dwm_mode_t mode;
} dwm_cfg_t;

/**
 * @brief Configures node to tag mode with given options
 *
 * @param[in] cfg, Tag configuration options
 *
 * @return Error code
 */
ret_code_t dwm_cfg_tag_set(nrf_drv_spi_t* spi_instance, dwm_cfg_tag_t* cfg);

/**
 * @brief Configures node to anchor mode with given options
 *
 * @param[in] cfg, Anchor configuration options
 *
 * @return Error code
 */

/**
 * @brief Reads configuration of the node
 *
 * @param[out] cfg, Node configuration
 *
 * @return Error code
 */
ret_code_t dwm_cfg_get(nrf_drv_spi_t* spi_instance, dwm_cfg_t* cfg);

#endif //_DWM_API_H_

