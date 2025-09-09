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
#define DEBOUNCE_BUTTONS_PERIOD_MSEC (300)
#define LONG_PRESS_PERIOD_MSEC (1000)
/* Private variables ---------------------------------------------------------*/
uint32_t gLastKeyPressTick = 0;
uint32_t gFilterWaterPressTick = 0;
bool gIgnoreFilterRelease = false;
/* Private function prototypes -----------------------------------------------*/

/* Private user code ---------------------------------------------------------*/
void HAL_GPIO_EXTI_Falling_Callback(uint16_t GPIO_Pin)
{
	switch(GPIO_Pin) {
	case GPIO_PIN_8: // Pump_WD_FDBK - error on 1
//		SMEventQueue_Add(SMSodaStreamPure_EventId_EVENT_HWWATCHDOG);
		break;
	case GPIO_PIN_13: // BTN1 - Filter Water
		if (gLastKeyPressTick + DEBOUNCE_BUTTONS_PERIOD_MSEC < HAL_GetTick()) {
			gLastKeyPressTick = HAL_GetTick();
			//COMM_UART_QueueTxMessage((uint8_t *)"$Filter pressed\r\n", 17);
			if (gButtonsFunction)
			{
				gIgnoreFilterRelease = false;
				gFilterWaterPressTick = HAL_GetTick();
				return;
			}
			gIgnoreFilterRelease = true;
			SMEventQueue_Add(SMSodaStreamPure_EventId_EVENT_ANYKEYPRESS);
		} else { // if debounced the press, debounce the release too
			gIgnoreFilterRelease = true;
		}

		break;
	case GPIO_PIN_14: // TODO STAM  -  Assuming that this is the carbonation level single button that toggles the levels
		if (gLastKeyPressTick + DEBOUNCE_BUTTONS_PERIOD_MSEC < HAL_GetTick()) {
			gLastKeyPressTick = HAL_GetTick();
			//COMM_UART_QueueTxMessage((uint8_t *)"$BTN2Low falling\r\n", 18);
			if (gButtonsFunction)
			{
				gCarbonationLevel++;
				if (gCarbonationLevel == eLevel_number_of_levels) {
					gCarbonationLevel = eLevel_off;
				}
				SMEventQueue_Add(SMSodaStreamPure_EventId_EVENT_CARBLEVELPRESSED);
				return;
			}
			SMEventQueue_Add(SMSodaStreamPure_EventId_EVENT_ANYKEYPRESS);
		}
		break;
	case GPIO_PIN_15: // BTN3 - Level Medium
		if (gLastKeyPressTick + DEBOUNCE_BUTTONS_PERIOD_MSEC < HAL_GetTick()) {
			gLastKeyPressTick = HAL_GetTick();
			//COMM_UART_QueueTxMessage((uint8_t *)"$BTN3 Med falling\r\n", 19);
			if (gButtonsFunction)
			{
				gCarbonationLevel = eLevel_medium;
				SMEventQueue_Add(SMSodaStreamPure_EventId_EVENT_CARBLEVELPRESSED);
				return;
			}
			SMEventQueue_Add(SMSodaStreamPure_EventId_EVENT_ANYKEYPRESS);
		}
		break;
	case GPIO_PIN_3: // BTN4 - Level High
		if (gLastKeyPressTick + DEBOUNCE_BUTTONS_PERIOD_MSEC < HAL_GetTick()) {
			gLastKeyPressTick = HAL_GetTick();
			//COMM_UART_QueueTxMessage((uint8_t *)"$BTN4 High falling\r\n", 20);
			if (gButtonsFunction)
			{
				gCarbonationLevel = eLevel_high;
				SMEventQueue_Add(SMSodaStreamPure_EventId_EVENT_CARBLEVELPRESSED);
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
		//COMM_UART_QueueTxMessage((uint8_t *)"$Filter released\r\n", 18);
		// Act upon release
		if (gButtonsFunction && !gIgnoreFilterRelease) {
			SMEventQueue_Add(SMSodaStreamPure_EventId_EVENT_FILTERBUTTONLONGPRESSED);
			// todo - handle filter short/long press
//			if (gFilterWaterPressTick + LONG_PRESS_PERIOD_MSEC < HAL_GetTick()) { // Long press
//				SMEventQueue_Add(SMSodaStreamPure_EventId_EVENT_LONGPRESSWATERFILTER);
//			} else {
//				SMEventQueue_Add(SMSodaStreamPure_EventId_EVENT_SHORTPRESSWATERFILTER);
//			}
		}
		break;
	}
}


