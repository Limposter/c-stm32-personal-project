/*
 * sht31.h
 *
 *  Created on: Jun 14, 2025
 *      Author: charl
 */

#ifndef LIB_INC_SHT31_H_
#define LIB_INC_SHT31_H_

#include "stm32f1xx_hal.h"

#define SHT31_ADDRESS         (0x44 << 1)  // 7-bit address shifted for HAL

HAL_StatusTypeDef SHT31_ReadTempHum(I2C_HandleTypeDef *hi2c, float *temp, float *humi);

#endif /* LIB_INC_SHT31_H_ */
