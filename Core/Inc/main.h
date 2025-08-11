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
#include "Common.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
/* USER CODE BEGIN EC */
// 6 full ADC scan cycles for 3 channels
#define ADC_DMA_BUFFER_SIZE   ((uint32_t)  18)

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

HAL_StatusTypeDef StartADCConversion();


extern bool gIsGuiControlMode;
extern eCarbonationLevel gCarbonationLevel;
extern bool gButtonsFunction;
extern uint32_t gBottleSizeThresholdmSecs;
extern uint32_t mPumpStartTimeTick;

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define WaterPMP_CURR_ADC1_IN0_Pin GPIO_PIN_0
#define WaterPMP_CURR_ADC1_IN0_GPIO_Port GPIOA
#define ACCEL_CS_Pin GPIO_PIN_4
#define ACCEL_CS_GPIO_Port GPIOA
#define Pump_CMD_Pin GPIO_PIN_5
#define Pump_CMD_GPIO_Port GPIOA
#define Pump_FDBK_Pin GPIO_PIN_0
#define Pump_FDBK_GPIO_Port GPIOB
#define WATER_LVL_ADC1_IN10_Pin GPIO_PIN_2
#define WATER_LVL_ADC1_IN10_GPIO_Port GPIOB
#define WaterPMP_FDBK_Pin GPIO_PIN_10
#define WaterPMP_FDBK_GPIO_Port GPIOB
#define UV_CURR_ADC1_IN15_Pin GPIO_PIN_11
#define UV_CURR_ADC1_IN15_GPIO_Port GPIOB
#define WaterLVL_CMD_Pin GPIO_PIN_12
#define WaterLVL_CMD_GPIO_Port GPIOB
#define BTN1_Pin GPIO_PIN_13
#define BTN1_GPIO_Port GPIOB
#define BTN1_EXTI_IRQn EXTI4_15_IRQn
#define BTN2_Pin GPIO_PIN_14
#define BTN2_GPIO_Port GPIOB
#define BTN2_EXTI_IRQn EXTI4_15_IRQn
#define BTN3_Pin GPIO_PIN_15
#define BTN3_GPIO_Port GPIOB
#define BTN3_EXTI_IRQn EXTI4_15_IRQn
#define Pump_WD_FDBK_Pin GPIO_PIN_8
#define Pump_WD_FDBK_GPIO_Port GPIOA
#define Pump_WD_FDBK_EXTI_IRQn EXTI4_15_IRQn
#define WaterPMP_CMD_Pin GPIO_PIN_6
#define WaterPMP_CMD_GPIO_Port GPIOC
#define Main_SW_Pin GPIO_PIN_7
#define Main_SW_GPIO_Port GPIOC
#define BTN4_Pin GPIO_PIN_3
#define BTN4_GPIO_Port GPIOB
#define BTN4_EXTI_IRQn EXTI2_3_IRQn
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
