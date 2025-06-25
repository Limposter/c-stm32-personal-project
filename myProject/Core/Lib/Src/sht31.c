/*
 * sht31.c
 *
 *  Created on: Jun 14, 2025
 *      Author: charl
 */

#include "sht31.h"
#include <math.h>

static uint8_t cmd_measure[] = {0x24, 0x00};  // High repeatability, clock stretching disabled

static uint16_t SHT31_CalcCRC(uint8_t *data)
{
    uint8_t crc = 0xFF;
    for (uint8_t i = 0; i < 2; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x80)
                crc = (crc << 1) ^ 0x31;
            else
                crc <<= 1;
        }
    }
    return crc;
}

HAL_StatusTypeDef SHT31_ReadTempHum(I2C_HandleTypeDef *hi2c, float *temp, float *humi)
{
    HAL_StatusTypeDef ret;
    uint8_t data[6];

    ret = HAL_I2C_Master_Transmit(hi2c, SHT31_ADDRESS, cmd_measure, 2, HAL_MAX_DELAY);
    if (ret != HAL_OK) return ret;

    HAL_Delay(20);  // Wait for measurement to complete

    ret = HAL_I2C_Master_Receive(hi2c, SHT31_ADDRESS, data, 6, HAL_MAX_DELAY);
    if (ret != HAL_OK) return ret;

    // CRC check
    if (SHT31_CalcCRC(data) != data[2] || SHT31_CalcCRC(&data[3]) != data[5])
        return HAL_ERROR;

    uint16_t rawTemp = (data[0] << 8) | data[1];
    uint16_t rawHumi = (data[3] << 8) | data[4];

    *temp = -45.0f + 175.0f * ((float)rawTemp / 65535.0f);
    *humi = 100.0f * ((float)rawHumi / 65535.0f);

    return HAL_OK;
}
