/**
  ******************************************************************************
  * @file           : ButtonsHandler.c
  * @brief          : Buttons handler
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "EventQueue.h"
/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/
#define DEBOUNCE_BUTTONS_PERIOD_MSEC (150)
#define LONG_PRESS_PERIOD_MSEC (2000)
/* Private variables ---------------------------------------------------------*/
uint32_t gLastKeyPressTick = 0;
uint32_t gFilterWaterPressTick = 0;
/* Private function prototypes -----------------------------------------------*/

/* Private user code ---------------------------------------------------------*/
void HAL_GPIO_EXTI_Falling_Callback(uint16_t GPIO_Pin)
{
	switch(GPIO_Pin) {
	case GPIO_PIN_13: // BTN1 - Filter Water
		if (gLastKeyPressTick + DEBOUNCE_BUTTONS_PERIOD_MSEC < HAL_GetTick()) {
			gLastKeyPressTick = HAL_GetTick();
			if (gButtonsFunction)
			{
				gFilterWaterPressTick = gLastKeyPressTick;
				return;
			}
			SMEventQueue_Add(SMSodaStreamPure_EventId_EVENT_ANYKEYPRESS);
		}
		break;
	case GPIO_PIN_14: // BTN2 - Level Low
		if (gLastKeyPressTick + DEBOUNCE_BUTTONS_PERIOD_MSEC < HAL_GetTick()) {
			gLastKeyPressTick = HAL_GetTick();
			if (gButtonsFunction)
			{
				gCarbonationLevel = eLevel_Low;
				SMEventQueue_Add(SMSodaStreamPure_EventId_EVENT_CARBONATIONLEVELPRESSED);
				return;
			}
			SMEventQueue_Add(SMSodaStreamPure_EventId_EVENT_ANYKEYPRESS);
		}
		break;
	case GPIO_PIN_15: // BTN3 - Level Medium
		if (gLastKeyPressTick + DEBOUNCE_BUTTONS_PERIOD_MSEC < HAL_GetTick()) {
			gLastKeyPressTick = HAL_GetTick();
			if (gButtonsFunction)
			{
				gCarbonationLevel = eLevel_medium;
				SMEventQueue_Add(SMSodaStreamPure_EventId_EVENT_CARBONATIONLEVELPRESSED);
				return;
			}
			SMEventQueue_Add(SMSodaStreamPure_EventId_EVENT_ANYKEYPRESS);
		}
		break;
	case GPIO_PIN_3: // BTN4 - Level High
		if (gLastKeyPressTick + DEBOUNCE_BUTTONS_PERIOD_MSEC < HAL_GetTick()) {
			gLastKeyPressTick = HAL_GetTick();
			if (gButtonsFunction)
			{
				gCarbonationLevel = eLevel_high;
				SMEventQueue_Add(SMSodaStreamPure_EventId_EVENT_CARBONATIONLEVELPRESSED);
				return;
			}
			SMEventQueue_Add(SMSodaStreamPure_EventId_EVENT_ANYKEYPRESS);
		}
		break;
	}
}

void HAL_GPIO_EXTI_Rising_Callback(uint16_t GPIO_Pin)
{
	switch(GPIO_Pin) {
	case GPIO_PIN_13: // BTN1 - Filter Water
		// Act upon release
		if (gFilterWaterPressTick + LONG_PRESS_PERIOD_MSEC < HAL_GetTick()) { // Long press
			SMEventQueue_Add(SMSodaStreamPure_EventId_EVENT_LONGPRESSWATERFILTER);
		}
		else
		{
			SMEventQueue_Add(SMSodaStreamPure_EventId_EVENT_SHORTPRESSWATERFILTER);
		}
		break;
	}
}


