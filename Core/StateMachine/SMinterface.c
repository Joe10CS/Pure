/*
 * SMinterface.c
 *
 *  Created on: May 20, 2025
 *      Author: yossi
 */

#include "main.h"
#include "SMinterface.h"
#include "SMSodaStreamPure.h"
#include "RtcBackupMemory.h"
#include "LedsPlayer.h"
#include "RTC.h"
#ifdef DEBUG_STATE_MACHINE
#include "RxTxMsgs.h"
#endif

eCarbonationLevel gCarbonationLevel = eLevel_Low; // stam
eCarbonationLevel gPrevCarbonationLevel = eLevel_Low; // stam
uint32_t gCarbCycleTickStart = 0;//   tickstart = HAL_GetTick();
uint32_t gPumpStartTimeTick = 0; // this is used also by HAL_GPIO_EXTI_Falling_Callback - non zero means pump is on
uint32_t gLastPumpTimeMSecs = 0;
eBottleSize gLastDetectedBottleSize = eBottle_1_Litter; // default
bool gButtonsFunction = false;
bool gMakeADrinkInProgress = false;
uint32_t gCurrentCarbonationOnTimeMSecs = 0;
bool gCurrentCarbonationOnTimeMSecsUpdated = false;
uint32_t gCO2CounterBeforeCurrnetCarbonationMSecs = 0;
bool gIsAlreadyInCO2CouterWarningState = false;
uint32_t gCO2MaxCounter = 0;

uint32_t gReadyTimerStartTick = 0;
uint32_t gFilterToCarbDelayStartTick = 0;

uint8_t gRinsingCyclesDone = 0;
uint32_t gUVLedTestStart = 0;
bool gUVLedTestFailed = false;

uint32_t gSolenoidPumpStartTick = 0;
extern uint16_t gSolenoidPumpWDCounter;

extern uint32_t gPumpTimoutMsecs;
extern volatile uint16_t gReadWaterLevelADC; // Hold the last read (A2D) value of the water level sensor
extern volatile uint16_t gReadWaterPumpCurrentADC;
extern volatile uint16_t gReadUVCurrentADC;
extern uint16_t gWaterLevelSensorThreahsold;
extern bool gIsTilted;

extern SMSodaStreamPure gStateMachine;
extern uint16_t gCarbTimeTable[eLevel_number_of_levels*2][eCycle_number_of_cycles][MAX_NUMBER_OF_CARBONATION_STEPS];

#ifdef DEBUG_STATE_MACHINE
extern uint8_t msg_len;
extern uint8_t gRawMsgForEcho[MAX_RX_BUFFER_LEN];
#endif
extern void DBGSendMessage(char *msg);

void StartCarbonation()
{
    gCurrentCarbonationOnTimeMSecs = 0;

    gIsAlreadyInCO2CouterWarningState = RBMEM_IsCO2CounterExpired();
    if (! gIsAlreadyInCO2CouterWarningState)
    {
        RBMEM_ReadElement(eRBMEM_total_CO2_msecs_used, &gCO2CounterBeforeCurrnetCarbonationMSecs);
        RBMEM_ReadElement(eRBMEM_total_CO2_msecs_max, &gCO2MaxCounter);
    }

}
void StopCarbonation()
{
    // if carbonation was interrupted - the last cycle was not recorded
    if (!gCurrentCarbonationOnTimeMSecsUpdated)
    {
        gCurrentCarbonationOnTimeMSecs += (HAL_GetTick() - gCarbCycleTickStart); // add last cycle partial time
        gCurrentCarbonationOnTimeMSecsUpdated = true;
    }
    RBMEM_AddMSecsToCO2Counter(gCurrentCarbonationOnTimeMSecs); // add milliseconds to CO2 counter
#ifdef DEBUG_STATE_MACHINE
 //   msg_len = (uint8_t)BuildReply((char*)gRawMsgForEcho, eUARTCommand_dbug, (uint32_t[]){gCurrentCarbonationOnTimeMSecs}, 1, false);
 //   COMM_UART_QueueTxMessage(gRawMsgForEcho, msg_len);
#endif
    gCurrentCarbonationOnTimeMSecs = 0;
}


void StartUVLEd()
{
	HAL_GPIO_WritePin(UV_LED_EN_GPIO_Port, UV_LED_EN_Pin, GPIO_PIN_SET);
}
void StopUVLed()
{
	HAL_GPIO_WritePin(UV_LED_EN_GPIO_Port, UV_LED_EN_Pin, GPIO_PIN_RESET);
}

void WaterPumpSensor(int isOn)
{
	HAL_GPIO_WritePin(WaterLVL_CMD_GPIO_Port, WaterLVL_CMD_Pin, (isOn == 1) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

bool IsBottleFull()
{
	return (gReadWaterLevelADC >= gWaterLevelSensorThreahsold);
}
bool Tilted()
{
	return gIsTilted;
}



uint16_t gDebugLastFailedUVADC = 0; // DEBUG REMOVE
uint16_t gDebugUVADCFailureDelay = 300; // DEBUG REMOVE

// Check UV for error:
// First, turn it on and on the next cycle
// make sure the ADC current is above the threshold
void CheckUVError()
{
	if (gUVLedTestStart == 0) // first time - just turn it on
	{
		// assume test will pass
		gUVLedTestFailed = false;
		// turn on UV led for test
		StartUVLEd();
		gUVLedTestStart = HAL_GetTick();

		gDebugLastFailedUVADC = 0;  // DEBUG REMOVE
	}
}
bool IsUVLedCheckDone(bool isOnWakeup)
{
	if (gUVLedTestStart > 0) // test in progress
	{
		//if ((HAL_GetTick() - gUVLedTestStart) >= 250) // wait at least 200 msecs for ADC to stabilize
    	if ((HAL_GetTick() - gUVLedTestStart) >= gDebugUVADCFailureDelay) // DEBUG REMOVE
		{
			gUVLedTestStart = 0;
#ifndef DEBUG_NO_UV_CHECK
			// check ADC value
			if (gReadUVCurrentADC < UV_MIN_ADC_THRESHOLD)
			{
				gDebugLastFailedUVADC = gReadUVCurrentADC;  // DEBUG REMOVE
				if (isOnWakeup) {
					StartAnimation(eAnimation_UVError, true);
				} else {
					// On start of a process (filtering and carbonation) this flag signals
					// the LEDs player that an error occurred and when playing the
					// progress animation it should switch on also the UV error animation
					gUVLedTestFailed = true;
				}
			}
#endif
			// turn off UV led after test
			StopUVLed();
			return true; // check is done
		} else {
			return false; // still waiting
		}
	}
	return true; // no test in progress
}

uint32_t val = 0;

bool LedsPlayDone()
{
    return (!IsAnimationActive() && !IsPendingAnimation());
}

void StartWaterPump()
{
	gPumpStartTimeTick = HAL_GetTick();
	gLastPumpTimeMSecs = 0;
	gLastDetectedBottleSize = eBottle_1_Litter; // set to default
	// if auto mode - start pump sensor (based on pumpStopsOnSensor)
	// in GUI mode the sensor controlled by command
	// to allow reading the sensor value even when the pump is not working
	if (! gIsGuiControlMode)
	{
		if (gStateMachine.vars.pumpStopsOnSensor)
		{
			WaterPumpSensor(1);
		}

	}
	HAL_GPIO_WritePin(WaterPMP_CMD_GPIO_Port, WaterPMP_CMD_Pin, GPIO_PIN_SET);
}

void StopWaterPump()
{
	// if auto mode - stop pump sensor (based on pumpStopsOnSensor)
	// in GUI mode the sensor is enabled all the time
	// to allow reading the sensor value even when the pump is not working
	if ((! IsGuiControlMode()) && gStateMachine.vars.pumpStopsOnSensor) {
		WaterPumpSensor(0);
	}
	if (gPumpStartTimeTick > 0)
	{
		gLastPumpTimeMSecs = HAL_GetTick() - gPumpStartTimeTick;

		if (gLastPumpTimeMSecs >= gBottleSizeThresholdmSecs) {
			gLastDetectedBottleSize = eBottle_1_Litter;
		} else {
			gLastDetectedBottleSize = eBottle_0_5_Litter;
		}
		gPumpStartTimeTick = 0;
	}

	HAL_GPIO_WritePin(WaterPMP_CMD_GPIO_Port, WaterPMP_CMD_Pin, GPIO_PIN_RESET);
}

bool WaterPumpTimerExpired()
{
	// Check for water pump timout
	if (gPumpStartTimeTick > 0) { // need to monitor water pump time
		if (gPumpStartTimeTick + gPumpTimoutMsecs < HAL_GetTick()){
			return true;
		}
	}
	return false;
}

void LedsSequence(eLedsSequence seq)
{
    switch (seq)
    {
    case LEDS_Splash:
        StartAnimation(eAnimation_StartUp, true);
        break;
    case LEDS_FilterState:
        // TODO Set the sequence based on time left to filter change
        //StartAnimation(eAnimation_StartUp, true);
        break;
    case LEDS_StartMakeDring:
        StartAnimation(eAnimation_MakeADrinkProgress, true);
        break;
    case LEDS_DoneMakeDring:
        // This flow is starting over the continuous playing of
        // MakeADrink, the "false" is used for adding
        // as a pending sequence
        StartAnimation(eAnimation_MakeADrinkSuccess, false);
        break;
    case LEDS_RinsingStart:
        StartAnimation(eAnimation_RingLoaderStart, true);
        break;
    case LEDS_RinsingEnd:
        StartAnimation(eAnimation_RingLoaderEnd, true);
        break;
    case LEDS_Status:
        StartAnimation(eAnimation_Status, true);
        break;
    case LEDS_StartUpCO2:
        StartAnimation(eAnimation_StartUpCO2, true);
        break;
    case LEDS_CheckFilterStatus:
        StartAnimation(eAnimation_CheckFilterStatus, true);
        break;
    case LEDS_FilterWarning:
        StartAnimation(eAnimation_Filter2ndRinsingWarning, true);
        break;
    case LEDS_CO2Warning:
        StartAnimation(eAnimation_CO2Warning, true);
        break;
    case LEDS_NoWaterWarning:
        StartAnimation(eAnimation_NoWaterWarning, true);
        break;
    case LEDS_CO2WarningWhileMakeingADrink:
        StartAnimation(eAnimation_CO2WarningWhileMakeingADrink, true);
        break;
    case LEDS_allOff:
        StartAnimation(eAnimation_ClearLedsFromLastValue, true);
        break;
    case LEDS_CO2Level:
        StartAnimation(eAnimation_CO2Level, true);
        break;
    case LEDS_CO2WarnOff:
        StartAnimation(eAnimation_ClearCO2Warning, true);
        break;
    case LEDS_FilterWarnOff:
        StartAnimation(eAnimation_ClearFilterWarning, true);
        break;
    case LEDS_Malfunction:
        StartAnimation(eAnimation_ClearLedsFromLastValue, true);
        break;
    case LEDS_HWWatchdog:
        // Turn Off Leds
        StartAnimation(eAnimation_ClearLedsFromLastValue, true);
        // Add a pending Error Sequance
        StartAnimation(eAnimation_DeviceError, false); // false - add as pending
        break;
    case LEDS_SafetyError:
        // Turn Off Leds
        StartAnimation(eAnimation_ClearLedsFromLastValue, true);
        // Add a pending Error Sequance
        StartAnimation(eAnimation_DeviceError, false); // false - add as pending
        break;
     default:
         break;
    }
}

bool CarbonationEnabled()
{
	return (gCarbonationLevel != eLevel_off);
}
void SendDonePumpOK()
{
	SendDoneMessage(eDone_OK);
}
void ButtonsFunction(bool isFunctioning)
{
	gButtonsFunction = isFunctioning;
}

void SetMakeADrinkInProgress(bool inProgress)
{
    gMakeADrinkInProgress = inProgress;
}
void StartCarbStageTimer(bool isOnCycle)
{
	gCarbCycleTickStart = HAL_GetTick();
	if (isOnCycle)
    {
	    // Set the flag to indicate that the current carbonation on time was not updated yet
	    // (will be updated when the on cycle expires)
	    // If the carbonation is stopped before the on cycle expires - the time will be added then
        gCurrentCarbonationOnTimeMSecsUpdated = false;
    }
}

bool CarbonationOffCycleExpired(uint16_t carbCycle)
{
	int row_index = (gCarbonationLevel - 1) + ((gLastDetectedBottleSize == eBottle_1_Litter) ? 0 : 3);
	if (gCarbCycleTickStart + gCarbTimeTable[row_index][eCycle_off][carbCycle] < HAL_GetTick())
	{
		return true;
	}
	return false;
}

bool CarbonationOnCycleExpired(uint16_t carbCycle)
{
	int row_index = (gCarbonationLevel - 1) + ((gLastDetectedBottleSize == eBottle_1_Litter) ? 0 : 3);
	if (gCarbCycleTickStart + gCarbTimeTable[row_index][eCycle_on][carbCycle] < HAL_GetTick())
	{
	    gCurrentCarbonationOnTimeMSecsUpdated = true;
	    gCurrentCarbonationOnTimeMSecs += gCarbTimeTable[row_index][eCycle_on][carbCycle];
	    if (!gIsAlreadyInCO2CouterWarningState)
	    {
	        uint32_t totalCO2MsecsUsed = gCO2CounterBeforeCurrnetCarbonationMSecs + gCurrentCarbonationOnTimeMSecs;
            if (totalCO2MsecsUsed >= gCO2MaxCounter)
            {
                // entered expired state
                gIsAlreadyInCO2CouterWarningState = true;
                // Start CO2 expired leds sequence
                LedsSequence(LEDS_CO2WarningWhileMakeingADrink);
            }
	    }
		return true;
	}
	return false;
}

bool IsCarbonationLastCycle(uint16_t carbCycle)
{
	int row_index = (gCarbonationLevel - 1) + ((gLastDetectedBottleSize == eBottle_1_Litter) ? 0 : 3);
	if (gCarbTimeTable[row_index][eCycle_on][carbCycle] == 0)
	{
		return true;
	}
	return false;

}
bool IsCO2LeveButtonPressed()
{
	return IS_CARB_LEVEL_BUTTON_PRESSED();
}
bool IsFilterButtonPressed()
{
	return IS_FILTER_BUTTON_PRESSED();
}

// Return true if OOTB window closed
bool IsOOTBWindowTimeExpired()
{
	return HAL_GetTick() > RESET_TO_OOTB_MSEC;
}

void ResetToOOTB()
{
	RBMEM_ResetDataToDefaults();
	ForceFilterExpired();
	RBMEM_WriteElement(eRBMEM_total_CO2_msecs_used, CO2_LIFETIME_MSECS + 1);
}

void ResetRinsingNumber()
{
	gRinsingCyclesDone = 0;
}
void UpdateRinsingNumber()
{
	gRinsingCyclesDone++;
	RBMEM_WriteElement(eRBMEM_Rinsing2ndWaiting, (gRinsingCyclesDone == 1) ? 1 : 0);
}

bool Rinsing2Done()
{
	return gRinsingCyclesDone >= 2;
}

bool IsRinsing2ndStagePending()
{
	uint32_t val = 0;
	RBMEM_ReadElement(eRBMEM_Rinsing2ndWaiting, &val);
	if (val != 0) {
		gRinsingCyclesDone = 1;
	}
	return (val != 0);
}
uint32_t gNoWaterInPumpStartTick = 0;

// This function is called periodically while the pump is running
// it detects no water in pump condition
// It is done automatically by the SM mechanism while in State_Rinsing or in State_Filtering
// since it's an exit condition for both states
bool WaterPumpNoWater()
{
	if (gReadWaterPumpCurrentADC < NO_WATER_IN_PUMP_DETECTION_ADC_THRESHOLD)
	{
		// current is low - possible no water in pump
		if (gNoWaterInPumpStartTick == 0)
		{
			// start counting time
			gNoWaterInPumpStartTick = HAL_GetTick();
		}
		else
		{
			// check time
			if (gNoWaterInPumpStartTick + MINIMUM_NO_WATER_IN_PUMP_DETECTION_TIME_MSEC < HAL_GetTick())
			{
				// reset time tracker
				gNoWaterInPumpStartTick = 0;
				// detected no water in pump condition
				return true;
			}
		}
	}
	else
	{
		// current is ok - reset counter
		gNoWaterInPumpStartTick = 0;
	}
	return false;
}

bool FilterExpired()
{
	return (GetFilterStatus() == eFilterStatus_Expired);
}



void StartReadyTimer()
{
    gReadyTimerStartTick = HAL_GetTick();
}
bool ReadyTimerExpired()
{
    return (gReadyTimerStartTick + READY_STATE_TIMEOUT_MSECS < HAL_GetTick());
}
void StartWaterPumpingTimer() {}

void RestartCO2Counter()
{
    RBMEM_WriteElement(eRBMEM_total_CO2_msecs_used, 0);
}

bool IsGuiControlMode()
{
	return gIsGuiControlMode;
}

void SolenoidPump(int isOn)
{
	gSolenoidPumpStartTick = (isOn == 1) ? HAL_GetTick() : 0;
	gSolenoidPumpWDCounter = 0; // reset counter always on state change
	HAL_GPIO_WritePin(Pump_CMD_GPIO_Port, Pump_CMD_Pin, (isOn == 1) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}
void SolenoidPumpUVPower(int isOn)
{
	HAL_GPIO_WritePin(GPIOC, Main_SW_Pin, (isOn == 1) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void IncreaseFilteringCounter()
{
	RBMEM_IncreaseFilteringCounter();
}

void ResetFilterCounters()
{
	RBMEM_WriteElement(eRBMEM_FilteringCounter, 0); // reset the number of filtering done
    RestartFilterTimer(); // reset the filtering time counter
}

void StartFilterToCarbDelay()
{
    gFilterToCarbDelayStartTick = HAL_GetTick();
}

bool FilterToCarbDelayDone()
{
    return (gFilterToCarbDelayStartTick + FILTER_TO_CARBONATION_DELAY_MSECS < HAL_GetTick());
}

