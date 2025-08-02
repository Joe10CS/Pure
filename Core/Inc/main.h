/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32g0xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdbool.h>
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
/* USER CODE BEGIN EC */
#define ADC_DMA_BUFFER_SIZE   ((uint32_t)  16)

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
void MainLogicInit(void);
void MainLogicPeriodic(void);

// Accelerometer
bool IsSlanted();
void AccelerometerInit(void);
bool AccelerometerIsPresent(void);



extern bool gIsGuiControlMode;
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define ACCEL_CS_Pin GPIO_PIN_4
#define ACCEL_CS_GPIO_Port GPIOA
#define WATER_LVL_ADC1_IN10_Pin GPIO_PIN_2
#define WATER_LVL_ADC1_IN10_GPIO_Port GPIOB
#define WaterPMP_CMD_Pin GPIO_PIN_6
#define WaterPMP_CMD_GPIO_Port GPIOC
#define UV_LED_EN_Pin GPIO_PIN_5
#define UV_LED_EN_GPIO_Port GPIOB
#define LED_EN_Pin GPIO_PIN_9
#define LED_EN_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
