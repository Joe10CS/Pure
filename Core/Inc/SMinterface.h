/*
 * SMinterface.h
 *
 *  Created on: May 20, 2025
 *      Author: yossi
 */

#ifndef INC_SMINTERFACE_H_
#define INC_SMINTERFACE_H_

#include "main.h"


typedef enum {
	eCarbLevel_undef,
	eCarbLevel_1,
	eCarbLevel_2,
	eCarbLevel_3,
}eCarbonationLevel;


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

extern eCarbonationLevel carbonationLevel;

void StartMalfunctionLedsSequence();
void FadeOutAmbiantLight();


void StartCarbonation(eCarbonationLevel carbonationLevel);
void StopCarbonation();


void StartUVLEd();
void StopUVLed();

void StartWaterPump();
void StopWaterPump();

void StartWaterFilterLedSequence();
void StartCarbonationLedSequance(eCarbonationLevel level);
void StartMalfunctionLedsSequence();
void StartRinsingLedSequence();
void StopRinsingLedSequence();
void WaterLedOrangeToBlue();

void ResetFilterLifetimeTimer();
bool IsFirstPowerON();
bool FilterLifeTimeExpired();
void StartReadyTimer();
void StartWaterPumpingTimer();

void LedsOff(uint32_t leds);
void FadeOutLeds(uint32_t leds);
void FadeInLeds(eCarbonationLevel level);

void FadeInAmbiantLight();
void AmbiantLightOff();

bool IsGuiControlMode();
void SolenoidPump(int itOn);
void SetLedByLastMsg();
void SetRGBLedByLastMsg();
void SolenoidPumpPower(int itOn);
void StartStatusTransmit();
void StopStatusTransmit();

extern eCarbonationLevel carbonationLevel;
extern SMSodaStreamPure mStateMachine;
#endif /* INC_SMINTERFACE_H_ */
