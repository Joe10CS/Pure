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
uint32_t gPumpStartTimeTick = 0;
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


uint32_t val = 0;
bool IsOOTBState()
{
    if (HAL_OK == RBMEM_ReadElement(eRBMEM_isFirstTimeSetupRequired, &val)) {
        return (val != 0);
    }
    return false; // in case of error - assume not OOTB
}
bool IsLedsSequencePlaying()
{
    return (IsAnimationActive() || IsPendingAnimation());
}

void ClearFilterOOTBFlag()
{
    RBMEM_WriteElement(eRBMEM_isFilterOOTBResetRequired, 0);
    // check if need to clear the eRBMEM_isFirstTimeSetupRequired
    RBMEM_ReadElement(eRBMEM_isCO2OOTBResetRequired, &val);
    if (val == 0) // Both cleared
    {
        RBMEM_WriteElement(eRBMEM_isFirstTimeSetupRequired,0);
    }
}

void ClearCO2OOTBFlag()
{
    RBMEM_WriteElement(eRBMEM_isCO2OOTBResetRequired, 0);
    // check if need to clear the eRBMEM_isFirstTimeSetupRequired
    RBMEM_ReadElement(eRBMEM_isFilterOOTBResetRequired, &val);
    if (val == 0) // Both cleared
    {
        RBMEM_WriteElement(eRBMEM_isFirstTimeSetupRequired,0);
    }

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
    case LEDS_OOTBCO2Down:
        StartAnimation(eAnimation_OOTBCO2Down, true);
        break;
    case LEDS_OOTBStatus:
        StartAnimation(eAnimation_OOTBStatus, true);
        break;
    case LEDS_OOTBFilterDown:
        StartAnimation(eAnimation_OOTBFilterDown, true);
        break;
    case LEDS_CheckFilterStatus:
        StartAnimation(eAnimation_CheckFilterStatus, true);
        break;
    case LEDS_CO2Warning:
        StartAnimation(eAnimation_CO2Warning, true);
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

void StartCarbonationLedSequance() {}
void StartMalfunctionLedsSequence() {}
void WaterLedOrangeToBlue() {}

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
	HAL_GPIO_WritePin(Pump_CMD_GPIO_Port, Pump_CMD_Pin, (isOn == 1) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}
void SolenoidPumpUVPower(int isOn)
{
	HAL_GPIO_WritePin(GPIOC, Main_SW_Pin, (isOn == 1) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void ResetFilterDaysCounter()
{
    ClearFilterOOTBFlag();
    RestartFilterTimer();
}

void StartFilterToCarbDelay()
{
    gFilterToCarbDelayStartTick = HAL_GetTick();
}

bool FilterToCarbDelayDone()
{
    return (gFilterToCarbDelayStartTick + FILTER_TO_CARBONATION_DELAY_MSECS < HAL_GetTick());
}

