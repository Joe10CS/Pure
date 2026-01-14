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

#define BUTTON_PRESS_MIN_COUNT_FOR_ANYKEY (5)
#define BUTTON_LONG_PRESS_COUNT_FOR_ANYKEY (300)


#define BUTTON_FALSE_DETECT_SUSPENSION_TIME_MSEC (70)

#define IS_MAIN_BUTTON_PRESSED() (HAL_GPIO_ReadPin(BTN1_GPIO_Port, BTN1_Pin) == GPIO_PIN_RESET)


/* Private variables ---------------------------------------------------------*/
uint32_t gLastMainButtonKeyPressTick = 0;

uint32_t gLastFilterKeyPressTick = 0;
bool gIgnoreFilterButtonRelease = true;

//uint32_t gCarbonationLevelPressTick = 0;
//bool gIgnoreCarbonationLevelRelease = true;

bool gIgnoreMainButtonRelease = true;

// These variables are used to track button press counts and states - in polling mode
uint16_t gFilterButtonPressCount = 0;
bool gFilterAnyKeyAlreadySent = false;
bool gFilterLongPressAlreadySent = false;
uint16_t gCO2LevelButtonPressCount = 0;
bool gCO2LevelAnyKeyAlreadySent = false;
bool gCO2LevelLongPressAlreadySent = false;

uint16_t gFalseButMainCounter = 0;
uint16_t gFalseButCarbLevelCounter = 0;
uint16_t gKeyPressButMainMS = 0;
uint16_t gKeyPressButCarbLevelMS = 0;
uint16_t gKeyPressButFilterMS = 0;
extern bool gMakeADrinkInProgress;

extern uint32_t gPumpStartTimeTick; // when this is not 0 the pump is running
//uint32_t gLastLongPressMS = 0;
/* Private function prototypes -----------------------------------------------*/


/* Private user code ---------------------------------------------------------*/
///////////////// Button press handling /////////////////
void HAL_GPIO_EXTI_Falling_Callback(uint16_t GPIO_Pin)
{
	switch(GPIO_Pin) {
	case GPIO_PIN_8: // Pump_WD_FDBK - error on 1
		if (gPumpStartTimeTick > 0) { // refer to this as error only when pump is running
			SMEventQueue_Add(SMSodaStreamPure_EventId_EVENT_HWWATCHDOG); // Handle watchdog event
		}
		break;

    case GPIO_PIN_13: // BTN1 - Main button
        if (gLastMainButtonKeyPressTick + DEBOUNCE_BUTTONS_PERIOD_MSEC < HAL_GetTick()) {
            gLastMainButtonKeyPressTick = HAL_GetTick();
            gIgnoreMainButtonRelease = false;
        }
        break;

    case GPIO_PIN_14: // BTN2 (PB14) - carbonation level single button that toggles the levels
	                  // Short press - cycle through levels
	                  // Long press - restarts the CO2 count-down

        // Handling in interrupt is disabled - handled in periodic check (polling)

//		if (gCarbonationLevelPressTick + DEBOUNCE_BUTTONS_PERIOD_MSEC < HAL_GetTick()) {
//		    gCarbonationLevelPressTick = HAL_GetTick();
//            gIgnoreCarbonationLevelRelease = false;
//			if (gButtonsFunction)
//			{
//                SMEventQueue_Add(SMSodaStreamPure_EventId_EVENT_CARBLEVELDOWN);
//                return;
//			}
//		}
		break;

    case GPIO_PIN_15: // BTN3 - Filter Button
        // Handling in interrupt is disabled - handled in periodic check (polling)
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
        // Handling in interrupt is disabled - handled in periodic check (polling)

//        // Act upon release
//        if (!gIgnoreCarbonationLevelRelease) {
//            gIgnoreCarbonationLevelRelease = true;
//            // Ignore it if too Short press
//            if (gCarbonationLevelPressTick + BUTTON_FALSE_DETECT_SUSPENSION_TIME_MSEC < HAL_GetTick()) {
//                if (gButtonsFunction)
//                {
//                    if (gCarbonationLevelPressTick + LONG_PRESS_PERIOD_MSEC > HAL_GetTick()) { // Short press
//                        gCarbonationLevel++;
//                        if (gCarbonationLevel == eLevel_number_of_levels) {
//                            gCarbonationLevel = eLevel_off;
//                        }
//                        SMEventQueue_Add(SMSodaStreamPure_EventId_EVENT_CARBLEVELSHORTPRESSED);
//                    }
//                } else {
//                    gKeyPressButCarbLevelMS = (uint16_t)(HAL_GetTick() - gCarbonationLevelPressTick);
//                    SMEventQueue_Add(SMSodaStreamPure_EventId_EVENT_ANYKEYPRESS);
//                }
//            } else {
//                gFalseButCarbLevelCounter++;
//            }
//        }
        break;
    case GPIO_PIN_15: // BTN3 - Filter Button
        // Handling in interrupt is disabled - handled in periodic check (polling)
        break;
	}

}

void CheckButtonsPressPeriodic()
{
    // Carbonation level button handling in polling mode
    if (IS_CARB_LEVEL_BUTTON_PRESSED()) {
        gCO2LevelButtonPressCount++;
        if (gCO2LevelButtonPressCount == BUTTON_PRESS_MIN_COUNT_FOR_ANYKEY) {// 50 ms debounce passed
            if (!gButtonsFunction) {
                // Buttons disabled (e.g., carbonation) -> emergency ANYKEY event
                SMEventQueue_Add(SMSodaStreamPure_EventId_EVENT_ANYKEYPRESS);
                gCO2LevelAnyKeyAlreadySent = true; // Prevent sending again on release
            }
        }
        // LONG PRESS reached while button is still pressed (only in functional mode)
         if (gButtonsFunction && !gCO2LevelLongPressAlreadySent && (gCO2LevelButtonPressCount >= BUTTON_LONG_PRESS_COUNT_FOR_ANYKEY)) {
             SMEventQueue_Add(SMSodaStreamPure_EventId_EVENT_CARBLEVELLONGPRESSED);
             gCO2LevelLongPressAlreadySent = true;  // suppress later events
         }

    } else { // button not pressed
        // If there was a press previously and the release is beyond debounce time
        if (gCO2LevelButtonPressCount >= BUTTON_PRESS_MIN_COUNT_FOR_ANYKEY) {
            // If long-press wasn't already sent → it is a short press
            if (!gCO2LevelLongPressAlreadySent) {
                if (gButtonsFunction) {
                    // Normal mode -> classify short vs long press
                    if (gCO2LevelButtonPressCount < BUTTON_LONG_PRESS_COUNT_FOR_ANYKEY) {
                        // Short press the carbonation level toggles the level unless we are in OOB and CO2 has not been reset yet
						gCarbonationLevel++;
						if (gCarbonationLevel == eLevel_number_of_levels) {
							gCarbonationLevel = eLevel_off;
						}
                        SMEventQueue_Add(SMSodaStreamPure_EventId_EVENT_CARBLEVELSHORTPRESSED);
                    } else {
                        // if long press time passed but long press event not sent (rare case)
                        SMEventQueue_Add(SMSodaStreamPure_EventId_EVENT_CARBLEVELLONGPRESSED);
                    }
                } else {
                    // Buttons disabled mode (e.g., carbonation)
                    // -> If ANYKEY already sent on press: DO NOTHING.
                    // -> If debounce passed but no ANYKEY was sent (rare case): send once here.
                    if (!gCO2LevelAnyKeyAlreadySent) {
                        SMEventQueue_Add(SMSodaStreamPure_EventId_EVENT_ANYKEYPRESS);
                    }
                }
            }
        }

        // Reset press counter & state
        gCO2LevelButtonPressCount = 0;
        gCO2LevelAnyKeyAlreadySent = false;
        gCO2LevelLongPressAlreadySent = false;
    }

    // Filter button handling in polling mode
    if (IS_FILTER_BUTTON_PRESSED()) {
        gFilterButtonPressCount++;
        if (gFilterButtonPressCount == BUTTON_PRESS_MIN_COUNT_FOR_ANYKEY) {// 50 ms debounce passed
            if (gLastFilterKeyPressTick == 0) {
                gLastFilterKeyPressTick = HAL_GetTick();
            }
            if (!gButtonsFunction) {
                // Buttons disabled (e.g., carbonation) -> emergency ANYKEY event
                SMEventQueue_Add(SMSodaStreamPure_EventId_EVENT_ANYKEYPRESS);
                gFilterAnyKeyAlreadySent = true; // Prevent sending again on release
            }
        }
        // LONG PRESS reached while button is still pressed (only in functional mode)
         if (gButtonsFunction && !gFilterLongPressAlreadySent && (gFilterButtonPressCount >= BUTTON_LONG_PRESS_COUNT_FOR_ANYKEY)) {
             SMEventQueue_Add(SMSodaStreamPure_EventId_EVENT_FILTERBUTTONLONGPRESSED);
             gFilterLongPressAlreadySent = true;  // suppress later events
         }

    } else { // button not pressed
        // If there was a press previously and the release is beyond debounce time
        if (gFilterButtonPressCount >= BUTTON_PRESS_MIN_COUNT_FOR_ANYKEY) {
            // If long-press wasn't already sent → it is a short press
            if (!gFilterLongPressAlreadySent) {
                if (gButtonsFunction) {
                    // Normal mode -> classify short vs long press
                    if (gFilterButtonPressCount < BUTTON_LONG_PRESS_COUNT_FOR_ANYKEY) {
                        SMEventQueue_Add(SMSodaStreamPure_EventId_EVENT_FILTERBUTTONSHORTPRESSED);
                    } else {
                        // if long press time passed but long press event not sent (rare case)
                        SMEventQueue_Add(SMSodaStreamPure_EventId_EVENT_FILTERBUTTONLONGPRESSED);
                    }
                } else {
                    // Buttons disabled mode (e.g., carbonation)
                    // -> If ANYKEY already sent on press: DO NOTHING.
                    // -> If debounce passed but no ANYKEY was sent (rare case): send once here.
                    if (!gFilterAnyKeyAlreadySent) {
                        SMEventQueue_Add(SMSodaStreamPure_EventId_EVENT_ANYKEYPRESS);
                    }
                }
            }
        }

        // Reset press counter & state
        gFilterButtonPressCount = 0;
        gFilterAnyKeyAlreadySent = false;
        gFilterLongPressAlreadySent = false;
        gLastFilterKeyPressTick = 0;
    }

//    if (IS_CARB_LEVEL_BUTTON_PRESSED() && (gIgnoreCarbonationLevelRelease == false) && (gCarbonationLevelPressTick > 0)) {
//        if (gCarbonationLevelPressTick + LONG_PRESS_PERIOD_MSEC < HAL_GetTick()) { // Long press
//            gIgnoreCarbonationLevelRelease = true;
//            SMEventQueue_Add(SMSodaStreamPure_EventId_EVENT_CARBLEVELLONGPRESSED);
//        }
//    }

    // This also take case of saving the last carbonation level to FRAM
    if (gPrevCarbonationLevel != gCarbonationLevel)
    {
        gPrevCarbonationLevel = gCarbonationLevel;
        RBMEM_WriteElement(eRBMEM_lastCarbonationLevel, gCarbonationLevel);
    }
}

// This method is used by the state machine, to check if any button is currently pressed
// in case where someone presses a button for a very long time and not releasing it
// when time expired and going to sleep mode, the states machine is also checking
// this to prevent from going to sleep mode (although after increasing the sleep timer
//  from 10 sec to 10 min it is not likely to happen)
bool IsAnyKeyPressed()
{
    return IS_FILTER_BUTTON_PRESSED() || IS_CARB_LEVEL_BUTTON_PRESSED()|| IS_MAIN_BUTTON_PRESSED();
}

