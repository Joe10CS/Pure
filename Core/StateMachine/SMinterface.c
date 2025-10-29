/*
 * SMinterface.c
 *
 *  Created on: May 20, 2025
 *      Author: yossi
 */

#include "main.h"
#include "SMinterface.h"
#include "SMSodaStreamPure.h"
#include "FRAM.h"
#include "LedsPlayer.h"
#include "RTC.h"
// TODO replace this with WS2811 as needed #include "LP5009.h"// TODO remove this on new Pure board

eCarbonationLevel gCarbonationLevel = eLevel_Low; // stam
eCarbonationLevel gPrevCarbonationLevel = eLevel_Low; // stam
uint32_t gCarbCycleTickStart = 0;//   tickstart = HAL_GetTick();
uint32_t gPumpStartTimeTick = 0;
uint32_t gLastPumpTimeMSecs = 0;
eBottleSize gLastDetectedBottleSize = eBottle_1_Litter; // default
bool gButtonsFunction = false;

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
char dbgMsg[40];
extern void DBGSendMessage(char *msg);

void StartCarbonation() {}
void StopCarbonation() {}


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
	// TODO uncomment!!
	//return gIsTilted
	return false;
}


uint32_t val = 0;
bool IsOOTBState()
{
    if (HAL_OK == FRAM_ReadElement(eFRAM_isFirstTimeSetupRequired, &val)) {
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
    FRAM_WriteElement(eFRAM_isFilterOOTBResetRequired, 0);
    // check if need to clear the eFRAM_isFirstTimeSetupRequired
    FRAM_ReadElement(eFRAM_isCO2OOTBResetRequired, &val);
    if (val == 0) // Both cleared
    {
        FRAM_WriteElement(eFRAM_isFirstTimeSetupRequired,0);
    }
}

void ClearCO2OOTBFlag()
{
    FRAM_WriteElement(eFRAM_isCO2OOTBResetRequired, 0);
    // check if need to clear the eFRAM_isFirstTimeSetupRequired
    FRAM_ReadElement(eFRAM_isFilterOOTBResetRequired, &val);
    if (val == 0) // Both cleared
    {
        FRAM_WriteElement(eFRAM_isFirstTimeSetupRequired,0);
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
			//// TODO replace this with WS2811 as needed LP5009_SetLed(&hi2c1, (uint8_t)(3), 0);
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
        StartAnimation(eAnimation_MakeADrinkSuccess, true);
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
    case LEDS_FIlterWarning:
        StartAnimation(eAnimation_FilterWarning, true);
        break;
    case LEDS_CO2Warning:
        StartAnimation(eAnimation_CO2Warning, true);
        break;
    case LEDS_allOff:
        StartAnimation(eAnimation_ClearLedsFromLastValue, true);
        break;
    case LEDS_CO2Level:
        StartAnimation(eAnimation_CO2Level, true);
        break;
    case LEDS_Malfunction:
        // TODO turning all leds off is cirrect for tilt mode - need to check what about HW or Security faults
        StartAnimation(eAnimation_ClearLedsFromLastValue, true);
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

void StartCarbStageTimer()
{
	gCarbCycleTickStart = HAL_GetTick();
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

void RestartCO2Counter(){/* TODO implement */}

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

