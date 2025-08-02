/*
 * SMinterface.c
 *
 *  Created on: May 20, 2025
 *      Author: yossi
 */

#include "main.h"
#include "SMinterface.h"

eCarbonationLevel carbonationLevel = eCarbLevel_undef;



void StartCarbonation(eCarbonationLevel level) {}
void StopCarbonation() {}


void StartUVLEd() {}
void StopUVLed() {}

void StartWaterPump()
{
	// TODO consider moving enable ADC from here to amore general place or even do it continuously all the time
	//      if there are more channels on the ADC1
	// Enable ADC
	if (mStateMachine.vars.pumpStopsOnSensor)
	{
		StartADCConversion();
	}


	HAL_GPIO_WritePin(WaterPMP_CMD_GPIO_Port, WaterPMP_CMD_Pin, GPIO_PIN_RESET);
}

void StopWaterPump()
{

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

void SolenoidPump(int itOn) {}
void SetLedByLastMsg() {}
void SetRGBLedByLastMsg() {}
void SolenoidPumpPower(int itOn) {}
void StartStatusTransmit() {}
void StopStatusTransmit() {}
