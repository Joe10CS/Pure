/*
 * SMinterface.c
 *
 *  Created on: May 20, 2025
 *      Author: yossi
 */

#include "main.h"
#include "SMinterface.h"
#include "SMSodaStreamPure.h"

eCarbonationLevel gCarbonationLevel = eLevel_Low; // stam
uint32_t mCarbCycleTickStart = 0;//   tickstart = HAL_GetTick();
uint32_t mPumpStartTimeTick = 0;
uint32_t mLastPumpTimeMSecs = 0;
eBottleSize mLastDetectedBottleSize = eBottle_1_Litter; // default
bool gButtonsFunction = false;


extern SMSodaStreamPure mStateMachine;
extern uint16_t gCarbTimeTable[eLevel_number_of_levels*2][eCycle_number_of_cycles][MAX_NUMBER_OF_CARBONATION_STEPS];

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

void InitCarbonationOnly()
{
	mLastDetectedBottleSize = eBottle_1_Litter;
	mLastPumpTimeMSecs = 0;
	mPumpStartTimeTick = 0;
}

void StartWaterPump()
{
	// if auto mode - start pump sensor (based on pumpStopsOnSensor)
	// in GUI mode the sensor controlled by command
	// to allow reading the sensor value even when the pump is not working
	if (! gIsGuiControlMode)
	{
		mPumpStartTimeTick = HAL_GetTick();
		mLastPumpTimeMSecs = 0;
		if (mStateMachine.vars.pumpStopsOnSensor)
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
	if ((! IsGuiControlMode()) && mStateMachine.vars.pumpStopsOnSensor) {
		WaterPumpSensor(0);
	}
	mLastPumpTimeMSecs = HAL_GetTick() - mPumpStartTimeTick;
	if (mLastPumpTimeMSecs >= gBottleSizeThresholdmSecs) {
		mLastDetectedBottleSize = eBottle_1_Litter;
	} else {
		mLastDetectedBottleSize = eBottle_0_5_Litter;
	}

	mPumpStartTimeTick = 0;

	HAL_GPIO_WritePin(WaterPMP_CMD_GPIO_Port, WaterPMP_CMD_Pin, GPIO_PIN_RESET);

}

void ButtonsFunction(bool isFunctioning)
{
	gButtonsFunction = isFunctioning;
}

void StartCarbStageTimer()
{
	mCarbCycleTickStart = HAL_GetTick();
}

bool CarbonationOffCycleExpired(uint16_t carbCycle)
{
	int row_index = gCarbonationLevel + ((mLastDetectedBottleSize == eBottle_1_Litter) ? 0 : 3);
	if (mCarbCycleTickStart + gCarbTimeTable[row_index][eCycle_off][carbCycle] < HAL_GetTick())
	{
		return true;
	}
	return false;
}

bool CarbonationOnCycleExpired(uint16_t carbCycle)
{
	int row_index = gCarbonationLevel + ((mLastDetectedBottleSize == eBottle_1_Litter) ? 0 : 3);
	if (mCarbCycleTickStart + gCarbTimeTable[row_index][eCycle_on][carbCycle] < HAL_GetTick())
	{
		return true;
	}
	return false;
}

bool IsCarbonationLastCycle(uint16_t carbCycle)
{
	int row_index = gCarbonationLevel + ((mLastDetectedBottleSize == eBottle_1_Litter) ? 0 : 3);
	if (gCarbTimeTable[row_index][eCycle_on][carbCycle] == 0)
	{
		return true;
	}
	return false;

}

void StartWaterFilterLedSequence() {}
void StartCarbonationLedSequance() {}
void StartMalfunctionLedsSequence() {}
void StartRinsingLedSequence() {}
void StopRinsingLedSequence() {}
void WaterLedOrangeToBlue() {}

void ResetFilterLifetimeTimer() {}
bool IsFirstPowerON() {return false;}
bool FilterLifeTimeExpired() {return false;}
void StartReadyTimer() {}
void StartWaterPumpingTimer() {}

void LedsOff(uint32_t leds) {}
void FadeOutLeds(uint32_t leds) {}
void FadeInLeds() {}

void FadeInAmbiantLight() {}
void FadeOutAmbiantLight() {}
void AmbiantLightOff() {}


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
void StartStatusTransmit() {}
void StopStatusTransmit() {}
