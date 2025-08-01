/*
 * LP5009.c
 *
 *  Created on: May 21, 2025
 *      Author: yossi
 */

#include "LP5009.h"

static const uint8_t lp5009_brightness_regs[3] = {
    LP5009_REG_LED0_BRIGHTNESS,
    LP5009_REG_LED1_BRIGHTNESS,
    LP5009_REG_LED2_BRIGHTNESS
};

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

// Set brightness (0–100%) for individual OUTx pin
HAL_StatusTypeDef LP5009_SetLed(I2C_HandleTypeDef *hi2c, uint8_t ledNumber, uint8_t brightnessPercent)
{
    if (ledNumber >= LP5009_LED_COUNT) return HAL_ERROR;
    if (brightnessPercent > 100) brightnessPercent = 100;

    // Convert 0–100% to register value (0x00 = full ON, 0xFF = OFF)
    uint8_t regValue = 255 - ((brightnessPercent * 255) / 100);

    return HAL_I2C_Mem_Write(hi2c, LP5009_I2C_ADDR, lp5009_color_regs[ledNumber], 1, &regValue, 1, HAL_MAX_DELAY);
}


// DEBUG REMOVE
int dd_wr_err = 0;
int dd_rd_err = 0;
int dd_val_err = 0;
int dd_HAL_ERROR = 0;
int dd_HAL_BUSY = 0;
int dd_HAL_TIMEOUT = 0;
void cntErr(HAL_StatusTypeDef sts)
{
	switch (sts)
	{
	case HAL_ERROR:
		dd_HAL_ERROR++;
		break;
	case HAL_BUSY:
		dd_HAL_BUSY++;
		break;
	case HAL_TIMEOUT:
		dd_HAL_TIMEOUT++;
		break;
	default:
		break;
	}
}
HAL_StatusTypeDef I2C_Write_Read_Byte(I2C_HandleTypeDef *hi2c,
                                      uint16_t DevAddress,
                                      uint16_t MemAddress,
                                      uint16_t MemAddSize,
                                      uint8_t value,
                                      uint32_t Timeout)
{
    HAL_StatusTypeDef status;

    // 1. Write one byte
    status = HAL_I2C_Mem_Write(hi2c, DevAddress, MemAddress, MemAddSize, &value, 1, Timeout);
    if (status != HAL_OK)
    {
    	dd_wr_err++;
    	cntErr(status);
    	return status;
    }

    // 2. Read back the same byte
    uint8_t readback = 0;
    status = HAL_I2C_Mem_Read(hi2c, DevAddress, MemAddress, MemAddSize, &readback, 1, Timeout);
    if (status != HAL_OK)
    {
    	dd_rd_err++;
        return status;
    }
    // 3. Compare written and read values
    if (readback == value)
  	{
    	return HAL_OK;
   	}
    else
    {
    	dd_val_err++;
        return HAL_ERROR;
    }
}
int ddd_conf0_err = 0;
int ddd_conf1_err = 0;
void LP5009_check_config(I2C_HandleTypeDef *hi2c,
                                      uint16_t DevAddress,
									  uint8_t expected_config0,
									  uint8_t expected_config1)
{
    HAL_StatusTypeDef status;

    uint8_t readback = 0;
    status = HAL_I2C_Mem_Read(hi2c, DevAddress, LP5009_REG_DEVICE_CONFIG0, 1, &readback, 1, HAL_MAX_DELAY);
    if (status != HAL_OK)
    {
    	dd_rd_err++;
    }
    else if (readback != expected_config0)
    {
    	ddd_conf0_err++;
    }
    status = HAL_I2C_Mem_Read(hi2c, DevAddress, LP5009_REG_DEVICE_CONFIG1, 1, &readback, 1, HAL_MAX_DELAY);
    if (status != HAL_OK)
    {
    	dd_rd_err++;
    }
    else if (readback != expected_config1)
    {
    	ddd_conf1_err++;
    }
}

HAL_StatusTypeDef stat = 0;
int err_count = 0;
int ok_count = 0;
extern I2C_HandleTypeDef hi2c2;

void test_LP5009()
{
	 // Hardware disable pin for reset (?)
	  HAL_GPIO_WritePin(LED_EN_GPIO_Port, LED_EN_Pin, GPIO_PIN_RESET);
		HAL_Delay(20);
		  // Hardware Enable the chip (power up the chip)
	  HAL_GPIO_WritePin(LED_EN_GPIO_Port, LED_EN_Pin, GPIO_PIN_SET);

		HAL_Delay(2);

		stat = I2C_Write_Read_Byte(&hi2c2, 0x28, 0x00, 1, 0x40, HAL_MAX_DELAY);
	  if (stat != HAL_OK)
	  		{
	  			err_count++;
	  		}
	  // 3. Disable power-save mode, keep other features enabled
	  stat = I2C_Write_Read_Byte(&hi2c2, 0x28, 0x01, 1, 0x2C, HAL_MAX_DELAY);
	  if (stat != HAL_OK)
	  		{
	  			err_count++;
	  		}

	  stat = I2C_Write_Read_Byte(&hi2c2, 0x28, 0x02, 1, 0x00, HAL_MAX_DELAY);
	if (stat != HAL_OK)
			{
				err_count++;
			}

	// 4. Set LED0 group brightness to max (controls OUT0–2)
	  stat = I2C_Write_Read_Byte(&hi2c2, 0x28, 0x07, 1, 0xFF, HAL_MAX_DELAY);
	  if (stat != HAL_OK)
	  		{
	  			err_count++;
	  		}
	  stat = I2C_Write_Read_Byte(&hi2c2, 0x28, 0x08, 1, 0xFF, HAL_MAX_DELAY);
	  if (stat != HAL_OK)
	  		{
	  			err_count++;
	  		}
	  // 5. Set OUT0_COLOR to 0x00 (fully ON)
	  stat = I2C_Write_Read_Byte(&hi2c2, 0x28, 0x0B, 1, 0x00, HAL_MAX_DELAY);
	  if (stat != HAL_OK)
	  		{
	  			err_count++;
	  		}
	  stat = I2C_Write_Read_Byte(&hi2c2, 0x28, 0x0C, 1, 0x00, HAL_MAX_DELAY);
	  if (stat != HAL_OK)
	  		{
	  			err_count++;
	  		}
	  stat = I2C_Write_Read_Byte(&hi2c2, 0x28, 0x0D, 1, 0x00, HAL_MAX_DELAY);
	  if (stat != HAL_OK)
	  		{
	  			err_count++;
	  		}
	  stat = I2C_Write_Read_Byte(&hi2c2, 0x28, 0x0E, 1, 0x00, HAL_MAX_DELAY);
	  if (stat != HAL_OK)
	  		{
	  			err_count++;
	  		}

	  LP5009_Init(&hi2c2);
	  LP5009_AllLedsOff(&hi2c2);
	  /* USER CODE END 2 */

	  /* Infinite loop */
	  /* USER CODE BEGIN WHILE */
	  int led = 0;
	  int led_bright = 0;





	  while (1)
	  {
		HAL_Delay(100);
		stat = LP5009_SetLed(&hi2c2,led, led_bright);
			if (stat != HAL_OK)
			{
				err_count++;
			}
			else
			{
				ok_count++;
				led_bright += 10;
				if (led_bright > 100)
				{
					led_bright = 0;
					led++;
					if (led >= LP5009_LED_COUNT)
					{
						led = 0;
					}
				}
			}
	  }

}
