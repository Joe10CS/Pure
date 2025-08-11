/*
 * LP5009.c
 *
 *  Created on: May 21, 2025
 *      Author: yossi
 */

#include "LP5009.h"

//static const uint8_t lp5009_brightness_regs[3] = {
//    LP5009_REG_LED0_BRIGHTNESS,
//    LP5009_REG_LED1_BRIGHTNESS,
//    LP5009_REG_LED2_BRIGHTNESS
//};

static const uint8_t lp5009_color_regs[LP5009_LED_COUNT] = {
    LP5009_REG_OUT0_COLOR, LP5009_REG_OUT1_COLOR, LP5009_REG_OUT2_COLOR,
    LP5009_REG_OUT3_COLOR, LP5009_REG_OUT4_COLOR, LP5009_REG_OUT5_COLOR,
    LP5009_REG_OUT6_COLOR, LP5009_REG_OUT7_COLOR, LP5009_REG_OUT8_COLOR
};

int ggg_init_ok = 0;
// Initialize LP5009: enable chip and set max brightness for all groups
HAL_StatusTypeDef LP5009_Init(I2C_HandleTypeDef *hi2c)
{
    HAL_StatusTypeDef status;

    // Hardware Enable the chip (power up the chip)
    HAL_GPIO_WritePin(LED_EN_GPIO_Port, LED_EN_Pin, GPIO_PIN_SET);

	HAL_Delay(1);

	// Software Enable the chip (Chip_EN = 1)
    status = HAL_I2C_Mem_Write(hi2c, LP5009_I2C_ADDR, LP5009_REG_DEVICE_CONFIG0, 1, (uint8_t[]){0x40}, 1, HAL_MAX_DELAY);
    if (status != HAL_OK) return status;

	HAL_Delay(1);
//    // Enable all config features (LogScale, PowerSave, AutoIncr, Dithering)
//    status = HAL_I2C_Mem_Write(hi2c, LP5009_I2C_ADDR, LP5009_REG_DEVICE_CONFIG1, 1, (uint8_t[]){0x3C}, 1, HAL_MAX_DELAY);
//    if (status != HAL_OK) return status;

#if 0
    // Enable all config features but Disable Power Save (bit 4 = 0)
    HAL_I2C_Mem_Write(hi2c, LP5009_I2C_ADDR, 0x01, 1, (uint8_t[]){0x2C}, 1, HAL_MAX_DELAY);

    // Set all LEDx_BRIGHTNESS to 0xFF (100%)
    for (uint8_t i = 0; i < 3; ++i)
    {
    	HAL_Delay(10);
        status = HAL_I2C_Mem_Write(hi2c, LP5009_I2C_ADDR, lp5009_brightness_regs[i], 1, (uint8_t[]){0xFF}, 1, HAL_MAX_DELAY);
        if (status != HAL_OK)
        	{
        	return status;
        	}
    }
#endif
    ggg_init_ok = 1;
    return HAL_OK;
}

// Turn all LEDs off
HAL_StatusTypeDef LP5009_AllLedsOff(I2C_HandleTypeDef *hi2c)
{
    HAL_StatusTypeDef status;
    uint8_t value = 0xFF;  // 0xFF = OFF

    for (uint8_t i = 0; i < LP5009_LED_COUNT; ++i)
    {
        status = HAL_I2C_Mem_Write(hi2c, LP5009_I2C_ADDR, lp5009_color_regs[i], 1, &value, 1, HAL_MAX_DELAY);
        if (status != HAL_OK) return status;
    }

    return HAL_OK;
}
//HAL_StatusTypeDef LP5009_RGB(I2C_HandleTypeDef *hi2c,
//                             uint8_t r, uint8_t g, uint8_t b)
//{
//    uint8_t regR = 255 - r; // OUT5
//    uint8_t regG = 255 - g; // OUT6
//    uint8_t regB = 255 - b; // OUT7
//
//    if (HAL_I2C_Mem_Write(hi2c, LP5009_I2C_ADDR, LP5009_REG_OUT5_COLOR, 1, &regR, 1, HAL_MAX_DELAY) != HAL_OK)
//        return HAL_ERROR;
//    if (HAL_I2C_Mem_Write(hi2c, LP5009_I2C_ADDR, LP5009_REG_OUT6_COLOR, 1, &regG, 1, HAL_MAX_DELAY) != HAL_OK)
//        return HAL_ERROR;
//    if (HAL_I2C_Mem_Write(hi2c, LP5009_I2C_ADDR, LP5009_REG_OUT7_COLOR, 1, &regB, 1, HAL_MAX_DELAY) != HAL_OK)
//        return HAL_ERROR;
//
//    return HAL_OK;
//}

HAL_StatusTypeDef LP5009_RGB(I2C_HandleTypeDef *hi2c, uint8_t r, uint8_t g, uint8_t b)
{
//    uint8_t wr[3] = { (uint8_t)(255 - r), (uint8_t)(255 - g), (uint8_t)(255 - b) };
    uint8_t wr[3] = { (uint8_t)(r), (uint8_t)(g), (uint8_t)(b) };
    // OUT5_COLOR..OUT7_COLOR are consecutive; auto-increment is enabled by default
    if (HAL_I2C_Mem_Write(hi2c, LP5009_I2C_ADDR, LP5009_REG_OUT5_COLOR, 1, &wr[0], 1, HAL_MAX_DELAY)) return HAL_ERROR;
    if (HAL_I2C_Mem_Write(hi2c, LP5009_I2C_ADDR, LP5009_REG_OUT6_COLOR, 1, &wr[1], 1, HAL_MAX_DELAY)) return HAL_ERROR;
    if (HAL_I2C_Mem_Write(hi2c, LP5009_I2C_ADDR, LP5009_REG_OUT7_COLOR, 1, &wr[2], 1, HAL_MAX_DELAY)) return HAL_ERROR;
    return HAL_OK;
}


// Set brightness (0–100%) for individual OUTx pin
HAL_StatusTypeDef LP5009_SetLed(I2C_HandleTypeDef *hi2c, uint8_t ledNumber, uint8_t brightnessPercent)
{
    if (ledNumber >= LP5009_LED_COUNT) return HAL_ERROR;
    if (brightnessPercent > 100) brightnessPercent = 100;

    // Convert 0–100% to register value (0x00 = full ON, 0xFF = OFF)
    uint8_t regValue = 255 - ((brightnessPercent * 255) / 100);

    return HAL_I2C_Mem_Write(hi2c, LP5009_I2C_ADDR, lp5009_color_regs[ledNumber], 1, &regValue, 1, HAL_MAX_DELAY);
}
HAL_StatusTypeDef LP5009_SetLedOff(I2C_HandleTypeDef *hi2c)
{
	uint8_t zero = 0x00;
	HAL_I2C_Mem_Write(hi2c, LP5009_I2C_ADDR, LP5009_REG_LED1_BRIGHTNESS, 1, &zero, 1, HAL_MAX_DELAY);
	HAL_I2C_Mem_Write(hi2c, LP5009_I2C_ADDR, LP5009_REG_LED2_BRIGHTNESS, 1, &zero, 1, HAL_MAX_DELAY);
    return HAL_OK;
}

HAL_StatusTypeDef LP5009_RGB_Off(I2C_HandleTypeDef *hi2c)
{
    uint8_t ff = 0xFF; // OUTx_COLOR: 0xFF = OFF
    HAL_StatusTypeDef s;
    s = HAL_I2C_Mem_Write(hi2c, LP5009_I2C_ADDR, LP5009_REG_OUT5_COLOR, 1, &ff, 1, HAL_MAX_DELAY); if (s) return s;
    s = HAL_I2C_Mem_Write(hi2c, LP5009_I2C_ADDR, LP5009_REG_OUT6_COLOR, 1, &ff, 1, HAL_MAX_DELAY); if (s) return s;
    s = HAL_I2C_Mem_Write(hi2c, LP5009_I2C_ADDR, LP5009_REG_OUT7_COLOR, 1, &ff, 1, HAL_MAX_DELAY); if (s) return s;
    return HAL_OK;
}

HAL_StatusTypeDef LP5009_RGB_DisableGroups(I2C_HandleTypeDef *hi2c) // "hard off"
{
    uint8_t zero = 0x00;
    HAL_StatusTypeDef s;
    s = HAL_I2C_Mem_Write(hi2c, LP5009_I2C_ADDR, LP5009_REG_LED1_BRIGHTNESS, 1, &zero, 1, HAL_MAX_DELAY); if (s) return s;
    s = HAL_I2C_Mem_Write(hi2c, LP5009_I2C_ADDR, LP5009_REG_LED2_BRIGHTNESS, 1, &zero, 1, HAL_MAX_DELAY); if (s) return s;
    return HAL_OK;
}

HAL_StatusTypeDef LP5009_RGB_EnableGroups(I2C_HandleTypeDef *hi2c)  // call before lighting RGB again
{
    uint8_t full = 0xFF;
    HAL_StatusTypeDef s;
    s = HAL_I2C_Mem_Write(hi2c, LP5009_I2C_ADDR, LP5009_REG_LED1_BRIGHTNESS, 1, &full, 1, HAL_MAX_DELAY); if (s) return s;
    s = HAL_I2C_Mem_Write(hi2c, LP5009_I2C_ADDR, LP5009_REG_LED2_BRIGHTNESS, 1, &full, 1, HAL_MAX_DELAY); if (s) return s;
    return HAL_OK;
}
