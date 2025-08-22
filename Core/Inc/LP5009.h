/*
 * LP5009.h
 *
 *  Created on: May 21, 2025
 *      Author: yossi
 */

#ifndef INC_LP5009_H_
#define INC_LP5009_H_

#include "main.h"

// ====== I2C DEVICE ADDRESS
#define LP5009_I2C_ADDR         (0x14 << 1)  // = 0x28
// #define LP5009_I2C_ADDR         (0x14)

// ====== REGISTER ADDRESSES
#define LP5009_REG_DEVICE_CONFIG0        0x00
#define LP5009_REG_DEVICE_CONFIG1        0x01
#define LP5009_REG_LED0_BRIGHTNESS       0x07
#define LP5009_REG_LED1_BRIGHTNESS       0x08
#define LP5009_REG_LED2_BRIGHTNESS       0x09
#define LP5009_REG_OUT0_COLOR            0x0B
#define LP5009_REG_OUT1_COLOR            0x0C
#define LP5009_REG_OUT2_COLOR            0x0D
#define LP5009_REG_OUT3_COLOR            0x0E
#define LP5009_REG_OUT4_COLOR            0x0F
#define LP5009_REG_OUT5_COLOR            0x10
#define LP5009_REG_OUT6_COLOR            0x11
#define LP5009_REG_OUT7_COLOR            0x12
#define LP5009_REG_OUT8_COLOR            0x13

// LED count
#define LP5009_LED_COUNT 9

// Public function prototypes
HAL_StatusTypeDef LP5009_Init(I2C_HandleTypeDef *hi2c);
HAL_StatusTypeDef LP5009_AllLedsOff(I2C_HandleTypeDef *hi2c);
HAL_StatusTypeDef LP5009_SetLed(I2C_HandleTypeDef *hi2c, uint8_t ledNumber, uint8_t brightnessPercent);

HAL_StatusTypeDef I2C_Write_Read_Byte(I2C_HandleTypeDef *hi2c,
                                      uint16_t DevAddress,
                                      uint16_t MemAddress,
                                      uint16_t MemAddSize,
                                      uint8_t value,
                                      uint32_t Timeout);
HAL_StatusTypeDef LP5009_RGB(I2C_HandleTypeDef *hi2c,uint8_t r, uint8_t g, uint8_t b);
HAL_StatusTypeDef LP5009_SetLedOff(I2C_HandleTypeDef *hi2c);
HAL_StatusTypeDef LP5009_RGB_EnableGroups(I2C_HandleTypeDef *hi2c);
HAL_StatusTypeDef LP5009_RGB_Off(I2C_HandleTypeDef *hi2c);

extern I2C_HandleTypeDef hi2c1;
#endif /* INC_LP5009_H_ */
