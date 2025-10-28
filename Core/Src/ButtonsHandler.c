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
#include "FRAM.h"
/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/
#define DEBOUNCE_BUTTONS_PERIOD_MSEC (300)
#define LONG_PRESS_PERIOD_MSEC (3000)

#define RESET_TO_OOTB_MSEC (5000)

#define IS_FILTER_BUTTON_PRESSED() (HAL_GPIO_ReadPin(BTN3_GPIO_Port, BTN3_Pin) == GPIO_PIN_RESET)
#define IS_CARB_LEVEL_BUTTON_PRESSED() (HAL_GPIO_ReadPin(BTN2_GPIO_Port, BTN2_Pin) == GPIO_PIN_RESET)

/* Private variables ---------------------------------------------------------*/
uint32_t gLastKeyPressTick = 0;

uint32_t gLastFilterKeyPressTick = 0;
bool gIgnoreFilterButtonRelease = false;

uint32_t gCarbonationLevelPressTick = 0;
bool gIgnoreCarbonationLevelRelease = true;

/* Private function prototypes -----------------------------------------------*/

/* Private user code ---------------------------------------------------------*/
///////////////// Button press handling /////////////////
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
		if (gCarbonationLevelPressTick + DEBOUNCE_BUTTONS_PERIOD_MSEC < HAL_GetTick()) {
		    gCarbonationLevelPressTick = HAL_GetTick();
			if (gButtonsFunction)
			{
			    gIgnoreCarbonationLevelRelease = false;
                SMEventQueue_Add(SMSodaStreamPure_EventId_EVENT_CARBLEVELDOWN);
                return;
			}
			gIgnoreCarbonationLevelRelease = true;
			SMEventQueue_Add(SMSodaStreamPure_EventId_EVENT_ANYKEYPRESS);
		}
		break;

    case GPIO_PIN_15: // BTN3 - Filter Button
        if (gLastFilterKeyPressTick + DEBOUNCE_BUTTONS_PERIOD_MSEC < HAL_GetTick()) {
            gLastFilterKeyPressTick = HAL_GetTick();
            if (gButtonsFunction)
            {
                gIgnoreFilterButtonRelease = false;
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
///////////////// BUtton release handling /////////////////
void HAL_GPIO_EXTI_Rising_Callback(uint16_t GPIO_Pin)
{
	switch(GPIO_Pin) {
	case GPIO_PIN_14: // BTN2 - Carbonation level button
        // Act upon release
        if (gButtonsFunction && !gIgnoreCarbonationLevelRelease) {
          gIgnoreCarbonationLevelRelease = true;
          if (gCarbonationLevelPressTick + LONG_PRESS_PERIOD_MSEC > HAL_GetTick()) { // Short press
              gCarbonationLevel++;
              if (gCarbonationLevel == eLevel_number_of_levels) {
                  gCarbonationLevel = eLevel_off;
              }
              SMEventQueue_Add(SMSodaStreamPure_EventId_EVENT_CARBLEVELSHORTPRESSED);
              return;
          }
        }
        break;
    case GPIO_PIN_15: // BTN3 - Filter Button
        if (gButtonsFunction && !gIgnoreFilterButtonRelease) {
            gIgnoreFilterButtonRelease = true;
            // Handle filter short press
            if (gLastFilterKeyPressTick + LONG_PRESS_PERIOD_MSEC > HAL_GetTick()) { // Short press
                    // Short press - has no specific action
                SMEventQueue_Add(
                        SMSodaStreamPure_EventId_EVENT_FILTERBUTTONSHORTPRESSED);
            }
        }
        break;

	}
}

void CheckLongPressButtonsPeriodic()
{
    uint32_t ootb = 0;
    FRAM_ReadElement(eFRAM_isFirstTimeSetupRequired, &ootb);

    if (IS_FILTER_BUTTON_PRESSED() && IS_CARB_LEVEL_BUTTON_PRESSED() && (ootb == 0))
    {
        if (gLastFilterKeyPressTick + RESET_TO_OOTB_MSEC  < HAL_GetTick()) { // Long press on filter and carb level
            FRAM_WriteElement(eFRAM_magicNumber, 0);
        } else {
            return;
        }
    }
    if (IS_FILTER_BUTTON_PRESSED() && (gIgnoreFilterButtonRelease == false) && (gLastFilterKeyPressTick > 0)) {
        if (gLastFilterKeyPressTick + LONG_PRESS_PERIOD_MSEC < HAL_GetTick()) { // Long press
            gIgnoreFilterButtonRelease = true;
            SMEventQueue_Add(SMSodaStreamPure_EventId_EVENT_FILTERBUTTONLONGPRESSED);
        }
    }
    if (IS_CARB_LEVEL_BUTTON_PRESSED() && (gIgnoreCarbonationLevelRelease == false) && (gCarbonationLevelPressTick > 0)) {
        if (gCarbonationLevelPressTick + LONG_PRESS_PERIOD_MSEC < HAL_GetTick()) { // Long press
            gIgnoreCarbonationLevelRelease = true;
            SMEventQueue_Add(SMSodaStreamPure_EventId_EVENT_CARBLEVELLONGPRESSED);
        }
    }

    // This also take case of saving the last carbonation level to FRAM
    if (gPrevCarbonationLevel != gCarbonationLevel)
    {
        gPrevCarbonationLevel = gCarbonationLevel;
        FRAM_WriteElement(eFRAM_lastCarbonationLevel, gCarbonationLevel);
    }
}
