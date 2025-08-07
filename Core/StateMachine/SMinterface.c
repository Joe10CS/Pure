/*
 * SMinterface.c
 *
 *  Created on: May 20, 2025
 *      Author: yossi
 */

#include "main.h"
#include "SMinterface.h"
#include "SMSodaStreamPure.h"

eCarbonationLevel carbonationLevel = eCarbLevel_undef;

extern SMSodaStreamPure mStateMachine;

void StartCarbonation(eCarbonationLevel level) {}
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

void StartWaterPump()
{
	// if auto mode - start pump sensor (based on pumpStopsOnSensor)
	// in GUI mode the sensor is enabled all the time
	// to allow reading the sensor value even when the pump is not working
	if ((! IsGuiControlMode()) && mStateMachine.vars.pumpStopsOnSensor)
	{
		WaterPumpSensor(1);
	}
	HAL_GPIO_WritePin(WaterPMP_CMD_GPIO_Port, WaterPMP_CMD_Pin, GPIO_PIN_SET);
}

void StopWaterPump()
{
	// if auto mode - stop pump sensor (based on pumpStopsOnSensor)
	// in GUI mode the sensor is enabled all the time
	// to allow reading the sensor value even when the pump is not working
	if ((! IsGuiControlMode()) && mStateMachine.vars.pumpStopsOnSensor)
	{
		WaterPumpSensor(0);
	}
	HAL_GPIO_WritePin(WaterPMP_CMD_GPIO_Port, WaterPMP_CMD_Pin, GPIO_PIN_RESET);

}

void StartWaterFilterLedSequence() {}
void StartCarbonationLedSequance(eCarbonationLevel level) {}
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
void FadeInLeds(eCarbonationLevel level) {}

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
void SetLedByLastMsg() {}
void SetRGBLedByLastMsg() {}
void SolenoidPumpPower(int isOn)
{
	HAL_GPIO_WritePin(GPIOC, Main_SW_Pin, (isOn == 1) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}
void StartStatusTransmit() {}
void StopStatusTransmit() {}
