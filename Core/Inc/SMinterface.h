/*
 * SMinterface.h
 *
 *  Created on: May 20, 2025
 *      Author: yossi
 */

#ifndef INC_SMINTERFACE_H_
#define INC_SMINTERFACE_H_

#include "main.h"

#define LEDS_all (eLEDS_BlueWater|eLEDS_OrangeWater|eLEDS_CardLevel1|eLEDS_CardLevel2|eLEDS_CardLevel3)
#define LEDS_AllCarbonation (eLEDS_CardLevel1|eLEDS_CardLevel2|eLEDS_CardLevel3)
#define LEDS_OrangeWater (eLEDS_OrangeWater)

typedef enum
{
	eLEDS_undefine = 0,
	eLEDS_BlueWater = 0x1,
	eLEDS_OrangeWater = 0x2,
	eLEDS_CardLevel1 = 0x4,
	eLEDS_CardLevel2 = 0x8,
	eLEDS_CardLevel3 = 0x10,
	eLEDS_AmbiandLight = 0x20,
}eLEDs;

typedef enum
{
	LEDS_Splash,
	LEDS_FilterState,
	LEDS_Rinsing
}eLedsSequence;
void StartMalfunctionLedsSequence();
void FadeOutAmbiantLight();


void StartCarbonation();
void StopCarbonation();


void StartUVLEd();
void StopUVLed();

void WaterPumpSensor(int isOn);
void StartWaterPump();
void StopWaterPump();
void SendDonePumpOK();
void ButtonsFunction(bool isFunctioning);
void StartCarbStageTimer();
bool CarbonationOffCycleExpired(uint16_t carbCycle);
bool CarbonationOnCycleExpired(uint16_t carbCycle);
bool IsCarbonationLastCycle(uint16_t carbCycle);

void StartWaterFilterLedSequence();
void StartCarbonationLedSequance();
void StartMalfunctionLedsSequence();
void StartRinsingLedSequence();
void StopRinsingLedSequence();
void WaterLedOrangeToBlue();

void ResetFilterLifetimeTimer();
bool IsFirstPowerON();
bool FilterLifeTimeExpired();
void StartReadyTimer();
bool ReadyTimerExpired();
void StartWaterPumpingTimer();

void LedsOff(uint32_t leds);
void FadeOutLeds(uint32_t leds);
void FadeInLeds();
void SetLevelLed();

void FadeInAmbiantLight();
void AmbiantLightOff();

bool IsGuiControlMode();
void SolenoidPump(int itOn);
void SolenoidPumpUVPower(int itOn);
void StartStatusTransmit();
void StopStatusTransmit();

// Added for pure 2
bool WaterPumpTimerExpired();
void LedsSequence(eLedsSequence seq);
bool CarbonationEnabled();
bool IsBottleFull();
bool Tilted();
#endif /* INC_SMINTERFACE_H_ */
