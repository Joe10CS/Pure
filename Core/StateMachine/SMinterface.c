/*
 * SMinterface.c
 *
 *  Created on: May 20, 2025
 *      Author: yossi
 */

#include "main.h"
#include "SMinterface.h"
#include "SMSodaStreamPure.h"
#include "LP5009.h"// TODO remove this on new Pure board

eCarbonationLevel gCarbonationLevel = eLevel_Low; // stam
uint32_t mCarbCycleTickStart = 0;//   tickstart = HAL_GetTick();
uint32_t mPumpStartTimeTick = 0;
uint32_t mLastPumpTimeMSecs = 0;
eBottleSize mLastDetectedBottleSize = eBottle_1_Litter; // default
bool gButtonsFunction = false;


extern SMSodaStreamPure mStateMachine;
extern uint16_t gCarbTimeTable[eLevel_number_of_levels*2][eCycle_number_of_cycles][MAX_NUMBER_OF_CARBONATION_STEPS];
char dbgMsg[40];
extern void DBGSendMessage(char *msg);

extern volatile uint16_t mReadWaterLevelADC;
extern uint16_t mWaterLevelSensorThreahsold;

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
	return (mReadWaterLevelADC >= mWaterLevelSensorThreahsold);
}

void InitCarbonationOnly()
{
	mLastDetectedBottleSize = eBottle_1_Litter;
	mLastPumpTimeMSecs = 0;
	mPumpStartTimeTick = 0;
}

void StartWaterPump()
{
	mPumpStartTimeTick = HAL_GetTick();
	mLastPumpTimeMSecs = 0;
	mLastDetectedBottleSize = eBottle_1_Litter; // set to default
	// if auto mode - start pump sensor (based on pumpStopsOnSensor)
	// in GUI mode the sensor controlled by command
	// to allow reading the sensor value even when the pump is not working
	if (! gIsGuiControlMode)
	{
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
	if (mPumpStartTimeTick > 0)
	{
		mLastPumpTimeMSecs = HAL_GetTick() - mPumpStartTimeTick;

		if (mLastPumpTimeMSecs >= gBottleSizeThresholdmSecs) {
			mLastDetectedBottleSize = eBottle_1_Litter;
		} else {
			mLastDetectedBottleSize = eBottle_0_5_Litter;
			LP5009_SetLed(&hi2c1, (uint8_t)(3), 0);
		}
		mPumpStartTimeTick = 0;
	}

	HAL_GPIO_WritePin(WaterPMP_CMD_GPIO_Port, WaterPMP_CMD_Pin, GPIO_PIN_RESET);
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
void SetLevelLed()
{
	LP5009_SetLed(&hi2c1, (uint8_t)(gCarbonationLevel), 0);
}
void StartWaterFilterLedSequence()
{
	// B G R
	LP5009_RGB(&hi2c1,(uint8_t)(250),(uint8_t)(0),(uint8_t)(0));
}
void StartCarbonationLedSequance() {}
void StartMalfunctionLedsSequence() {}
void StartRinsingLedSequence()
{
	// B G R
	LP5009_RGB(&hi2c1,(uint8_t)(0),(uint8_t)(255),(uint8_t)(0));
}
void StopRinsingLedSequence()
{
	// B G R
	LP5009_RGB(&hi2c1,(uint8_t)(0),(uint8_t)(0),(uint8_t)(0));
}
void WaterLedOrangeToBlue() {}

void ResetFilterLifetimeTimer()
{
	FilterRTC_Replaced_StartTimer();
}
bool IsFirstPowerON() {return false;}
bool FilterLifeTimeExpired()
{
	return FilterRTC_IsDue();
}
void StartReadyTimer() {}
void StartWaterPumpingTimer() {}

void LedsOff(uint32_t leds)
{
	LP5009_SetLed(&hi2c1, (uint8_t)(0), 100);
	LP5009_SetLed(&hi2c1, (uint8_t)(1), 100);
	LP5009_SetLed(&hi2c1, (uint8_t)(2), 100);
	LP5009_SetLed(&hi2c1, (uint8_t)(3), 100);
	LP5009_RGB(&hi2c1,(uint8_t)(0),(uint8_t)(0),(uint8_t)(0));
}
void FadeOutLeds(uint32_t leds) {}
void FadeInLeds()
{
	LedsOff(0);
}

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
bool dddP = false;  // TODO debug remove
void SolenoidPumpUVPower(int isOn)
{
	dddP = isOn;
	HAL_GPIO_WritePin(GPIOC, Main_SW_Pin, (isOn == 1) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}
void StartStatusTransmit() {}
void StopStatusTransmit() {}
