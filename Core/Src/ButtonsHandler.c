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
#define LONG_PRESS_PERIOD_MSEC (3000)

// This is used only for the first ever reset (OOTB)
// in the future: if CO2 level is detected - can be used to reset the CO2 countdown
#define CARBONATION_LEVEL_BUTTON_SUPPORT_LONG_PRESS

/* Private variables ---------------------------------------------------------*/
uint32_t gLastKeyPressTick = 0;

uint32_t gFilterButtonPressTick = 0;
bool gIgnoreFilterButtonRelease = false;

#ifdef CARBONATION_LEVEL_BUTTON_SUPPORT_LONG_PRESS
uint32_t gCarbonationLevelPressTick = 0;
bool gIgnoreCarbonationLevelRelease = true;
#endif

/* Private function prototypes -----------------------------------------------*/

/* Private user code ---------------------------------------------------------*/
void HAL_GPIO_EXTI_Falling_Callback(uint16_t GPIO_Pin)
{
	switch(GPIO_Pin) {
	case GPIO_PIN_8: // Pump_WD_FDBK - error on 1
//		SMEventQueue_Add(SMSodaStreamPure_EventId_EVENT_HWWATCHDOG);
		break;

    case GPIO_PIN_13: // BTN1 - Main button
        if (gLastKeyPressTick + DEBOUNCE_BUTTONS_PERIOD_MSEC < HAL_GetTick()) {
            gLastKeyPressTick = HAL_GetTick();
            //COMM_UART_QueueTxMessage((uint8_t *)"$BTN4 High falling\r\n", 20);
            if (gButtonsFunction)
            {
                SMEventQueue_Add(SMSodaStreamPure_EventId_EVENT_PRIMARYBUTTONPRESSED);
                return;
            }
            SMEventQueue_Add(SMSodaStreamPure_EventId_EVENT_ANYKEYPRESS);
        }
        break;

    case GPIO_PIN_14: // BTN2 (PB14) - carbonation level single button that toggles the levels
	                  // Short press - cycle through levels
	                  // Long press - restarts the CO2 count-down
		if (gLastKeyPressTick + DEBOUNCE_BUTTONS_PERIOD_MSEC < HAL_GetTick()) {
			gLastKeyPressTick = HAL_GetTick();
			if (gButtonsFunction)
			{
#ifdef CARBONATION_LEVEL_BUTTON_SUPPORT_LONG_PRESS
			    gIgnoreCarbonationLevelRelease = false;
			    gCarbonationLevelPressTick = HAL_GetTick();
                SMEventQueue_Add(SMSodaStreamPure_EventId_EVENT_CARBLEVELDOWN);
#else
	              gCarbonationLevel++;
	              if (gCarbonationLevel == eLevel_number_of_levels) {
	                  gCarbonationLevel = eLevel_off;
	              }
	              SMEventQueue_Add(SMSodaStreamPure_EventId_EVENT_CARBLEVELSHORTPRESSED);
#endif
                return;
			}
#ifdef CARBONATION_LEVEL_BUTTON_SUPPORT_LONG_PRESS
			gIgnoreCarbonationLevelRelease = true;
#endif
			SMEventQueue_Add(SMSodaStreamPure_EventId_EVENT_ANYKEYPRESS);
		}
		break;

    case GPIO_PIN_15: // BTN3 - Filter Button
        if (gLastKeyPressTick + DEBOUNCE_BUTTONS_PERIOD_MSEC < HAL_GetTick()) {
            gLastKeyPressTick = HAL_GetTick();
            if (gButtonsFunction)
            {
                gIgnoreFilterButtonRelease = false;
                gFilterButtonPressTick = HAL_GetTick();
                SMEventQueue_Add(SMSodaStreamPure_EventId_EVENT_FILTERDOWN);
                return;
            }
            gIgnoreFilterButtonRelease = true;
            SMEventQueue_Add(SMSodaStreamPure_EventId_EVENT_ANYKEYPRESS);
        } else { // if debounced the press, debounce the release too
            gIgnoreFilterButtonRelease = true;
        }
        break;
	}
}

void HAL_GPIO_EXTI_Rising_Callback(uint16_t GPIO_Pin)
{
	switch(GPIO_Pin) {
	case GPIO_PIN_15: // BTN3 - Filter Button
		//COMM_UART_QueueTxMessage((uint8_t *)"$Filter released\r\n", 18);
		// Act upon release
		if (gButtonsFunction && !gIgnoreFilterButtonRelease) {
			// Handle filter short/long press
			if (gFilterButtonPressTick + LONG_PRESS_PERIOD_MSEC < HAL_GetTick()) { // Long press
				SMEventQueue_Add(SMSodaStreamPure_EventId_EVENT_FILTERBUTTONLONGPRESSED);
			} else {
			    // Short press - has no specific action
			    SMEventQueue_Add(SMSodaStreamPure_EventId_EVENT_ANYKEYPRESS);
			}
		}
		break;

	case GPIO_PIN_14: // BTN2 - Carbonation level button
        // Act upon release
#ifdef CARBONATION_LEVEL_BUTTON_SUPPORT_LONG_PRESS
        if (gButtonsFunction && !gIgnoreCarbonationLevelRelease) {
          gIgnoreCarbonationLevelRelease = true;
          if (gIgnoreCarbonationLevelRelease + LONG_PRESS_PERIOD_MSEC < HAL_GetTick()) { // Long press
              SMEventQueue_Add(SMSodaStreamPure_EventId_EVENT_CARBLEVELLONGPRESSED);
          } else {
              gCarbonationLevel++;
              if (gCarbonationLevel == eLevel_number_of_levels) {
                  gCarbonationLevel = eLevel_off;
              }
              SMEventQueue_Add(SMSodaStreamPure_EventId_EVENT_CARBLEVELSHORTPRESSED);
              return;
          }
        }
#endif
        break;
	}
}

