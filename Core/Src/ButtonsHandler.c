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
#include "RtcBackupMemory.h"
/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/
#define DEBOUNCE_BUTTONS_PERIOD_MSEC (300)
#define LONG_PRESS_PERIOD_MSEC (3000)

#define RESET_TO_OOTB_MSEC (5000)

#define BUTTON_FALSE_DETECT_SUSPENSION_TIME_MSEC (70)

#define IS_MAIN_BUTTON_PRESSED() (HAL_GPIO_ReadPin(BTN1_GPIO_Port, BTN1_Pin) == GPIO_PIN_RESET)

#define IS_FILTER_BUTTON_PRESSED() (HAL_GPIO_ReadPin(BTN3_GPIO_Port, BTN3_Pin) == GPIO_PIN_RESET)
#define IS_CARB_LEVEL_BUTTON_PRESSED() (HAL_GPIO_ReadPin(BTN2_GPIO_Port, BTN2_Pin) == GPIO_PIN_RESET)

/* Private variables ---------------------------------------------------------*/
uint32_t gLastMainButtonKeyPressTick = 0;

uint32_t gLastFilterKeyPressTick = 0;
bool gIgnoreFilterButtonRelease = true;

uint32_t gCarbonationLevelPressTick = 0;
bool gIgnoreCarbonationLevelRelease = true;

bool gIgnoreMainButtonRelease = true;

uint16_t gFalseButMainCounter = 0;
uint16_t gFalseButCarbLevelCounter = 0;
uint16_t gFalseButFilterCounter = 0;
uint16_t gKeyPressButMainMS = 0;
uint16_t gKeyPressButCarbLevelMS = 0;
uint16_t gKeyPressButFilterMS = 0;
extern bool gMakeADrinkInProgress;
//uint32_t gLastLongPressMS = 0;
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
        if (gLastMainButtonKeyPressTick + DEBOUNCE_BUTTONS_PERIOD_MSEC < HAL_GetTick()) {
            gLastMainButtonKeyPressTick = HAL_GetTick();
            gIgnoreMainButtonRelease = false;
//            //COMM_UART_QueueTxMessage((uint8_t *)"$BTN4 High falling\r\n", 20);
//            if (gButtonsFunction)
//            {
//                gIgnoreMainButtonRelease = false;
//                return;
//            }
//            gIgnoreMainButtonRelease = true;
//            //SMEventQueue_Add(SMSodaStreamPure_EventId_EVENT_ANYKEYPRESS);
        }
        break;

    case GPIO_PIN_14: // BTN2 (PB14) - carbonation level single button that toggles the levels
	                  // Short press - cycle through levels
	                  // Long press - restarts the CO2 count-down
		if (gCarbonationLevelPressTick + DEBOUNCE_BUTTONS_PERIOD_MSEC < HAL_GetTick()) {
		    gCarbonationLevelPressTick = HAL_GetTick();
            gIgnoreCarbonationLevelRelease = false;
			if (gButtonsFunction)
			{
                SMEventQueue_Add(SMSodaStreamPure_EventId_EVENT_CARBLEVELDOWN);
                return;
			}
//			gIgnoreCarbonationLevelRelease = true;
			//SMEventQueue_Add(SMSodaStreamPure_EventId_EVENT_ANYKEYPRESS);
		}
		break;

    case GPIO_PIN_15: // BTN3 - Filter Button
        if (gLastFilterKeyPressTick + DEBOUNCE_BUTTONS_PERIOD_MSEC < HAL_GetTick()) {
            gLastFilterKeyPressTick = HAL_GetTick();
            gIgnoreFilterButtonRelease = false;
            if (gButtonsFunction)
            {
                SMEventQueue_Add(SMSodaStreamPure_EventId_EVENT_FILTERDOWN);
                return;
            }
            //SMEventQueue_Add(SMSodaStreamPure_EventId_EVENT_ANYKEYPRESS);
        } else { // if debounced the press, debounce the release too            TODO probably not needed since the ignore is true all the time unless it's a valid press.
            gIgnoreFilterButtonRelease = true;
        }
        break;
	}
}
///////////////// BUtton release handling /////////////////
void HAL_GPIO_EXTI_Rising_Callback(uint16_t GPIO_Pin)
{
	switch(GPIO_Pin) {
    case GPIO_PIN_13: // BTN1 - CMain button
        // Act upon release
        if (!gIgnoreMainButtonRelease) {
            gIgnoreMainButtonRelease = true;
            // Ignore it if too Short press
            if (gLastMainButtonKeyPressTick + BUTTON_FALSE_DETECT_SUSPENSION_TIME_MSEC < HAL_GetTick()) {
                if (gButtonsFunction) {
                    SMEventQueue_Add(SMSodaStreamPure_EventId_EVENT_PRIMARYBUTTONPRESSED);
                } else {
                    gKeyPressButMainMS = (uint16_t)(HAL_GetTick() - gLastMainButtonKeyPressTick);
                    SMEventQueue_Add(SMSodaStreamPure_EventId_EVENT_ANYKEYPRESS);
                }
            } else {
                gFalseButMainCounter++;
            }
        }
        break;
    case GPIO_PIN_14: // BTN2 - Carbonation level button
        // Act upon release
        if (!gIgnoreCarbonationLevelRelease) {
            gIgnoreCarbonationLevelRelease = true;
            // Ignore it if too Short press
            if (gCarbonationLevelPressTick + BUTTON_FALSE_DETECT_SUSPENSION_TIME_MSEC < HAL_GetTick()) {
                if (gButtonsFunction)
                {
                    if (gCarbonationLevelPressTick + LONG_PRESS_PERIOD_MSEC > HAL_GetTick()) { // Short press
                        gCarbonationLevel++;
                        if (gCarbonationLevel == eLevel_number_of_levels) {
                            gCarbonationLevel = eLevel_off;
                        }
                        SMEventQueue_Add(SMSodaStreamPure_EventId_EVENT_CARBLEVELSHORTPRESSED);
                    }
                } else {
                    gKeyPressButCarbLevelMS = (uint16_t)(HAL_GetTick() - gCarbonationLevelPressTick);
                    SMEventQueue_Add(SMSodaStreamPure_EventId_EVENT_ANYKEYPRESS);
                }
            } else {
                gFalseButCarbLevelCounter++;
            }
        }
        break;
    case GPIO_PIN_15: // BTN3 - Filter Button
        if (!gIgnoreFilterButtonRelease) {
            gIgnoreFilterButtonRelease = true;
            // Ignore it if too Short press
            if (gLastFilterKeyPressTick + BUTTON_FALSE_DETECT_SUSPENSION_TIME_MSEC < HAL_GetTick()) {
                if (gButtonsFunction)
                {
                    // Handle filter short press
                    if (gLastFilterKeyPressTick + LONG_PRESS_PERIOD_MSEC > HAL_GetTick()) { // Short press
                            // Short press - has no specific action
                        SMEventQueue_Add(SMSodaStreamPure_EventId_EVENT_FILTERBUTTONSHORTPRESSED);
                    }
                } else {
                    gKeyPressButFilterMS = (uint16_t)(HAL_GetTick() - gLastFilterKeyPressTick);
                    // TODO Temporary test of not using the filter buttton for cancelling carbonation
                    if (! gMakeADrinkInProgress) {
                        SMEventQueue_Add(SMSodaStreamPure_EventId_EVENT_ANYKEYPRESS);
                    }
                }
            } else {
                gFalseButFilterCounter++;
            }
        }
        break;
	}

}

void CheckLongPressButtonsPeriodic()
{
    uint32_t ootb = 0;
    RBMEM_ReadElement(eRBMEM_isFirstTimeSetupRequired, &ootb);

    if (gButtonsFunction && IS_FILTER_BUTTON_PRESSED() && IS_CARB_LEVEL_BUTTON_PRESSED() && (ootb == 0))
    {
        if (gLastFilterKeyPressTick + RESET_TO_OOTB_MSEC  < HAL_GetTick()) { // Long press on filter and carb level
            RBMEM_ReadElement(eRBMEM_RTC_Memory_magicNumber, &ootb);
            if (ootb != 0) {
                RBMEM_WriteElement(eRBMEM_RTC_Time_Start_magicNumber, 0);
                RBMEM_WriteElement(eRBMEM_RTC_Memory_magicNumber, 0);
                NVIC_SystemReset();
            }
        }
        return;
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
        RBMEM_WriteElement(eRBMEM_lastCarbonationLevel, gCarbonationLevel);
    }
}

bool IsAnyKeyPressed()
{
    return IS_FILTER_BUTTON_PRESSED() || IS_CARB_LEVEL_BUTTON_PRESSED()|| IS_MAIN_BUTTON_PRESSED();
}

