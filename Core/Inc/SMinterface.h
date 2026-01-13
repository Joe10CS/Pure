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
	LEDS_Splash, // Power up or wakup
	LEDS_FilterState, // Show folter sate
	LEDS_StartMakeDring, // Start making drink
	LEDS_DoneMakeDring, // Done making drink
	LEDS_RinsingStart,
    LEDS_RinsingEnd,

    LEDS_Status, // show ring and current status of carb level and filter
	LEDS_StartUpCO2,
	LEDS_CheckFilterStatus,
    LEDS_FilterWarning, // shows the flashing warning before 2nd rinsing stage
	LEDS_CO2Warning,
	LEDS_NoWaterWarning,
    LEDS_CO2WarningWhileMakeingADrink,
	LEDS_allOff,
	LEDS_CO2Level,
	LEDS_CO2WarnOff,
    LEDS_FilterWarnOff,
	LEDS_Malfunction,
	LEDS_HWWatchdog,
	LEDS_SafetyError
}eLedsSequence;
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
void SetMakeADrinkInProgress(bool inProgress);
void StartCarbStageTimer(bool isOnCycle);
bool CarbonationOffCycleExpired(uint16_t carbCycle);
bool CarbonationOnCycleExpired(uint16_t carbCycle);
bool IsCarbonationLastCycle(uint16_t carbCycle);

bool IsOOTBWindowTimeExpired();
bool IsCO2LeveButtonPressed();
bool IsFilterButtonPressed();
void ResetToOOTB();

void ResetRinsingNumber();
void UpdateRinsingNumber();
bool Rinsing2Done();
bool WaterPumpNoWater();
bool FilterExpired();

void ResetFilterLifetimeTimer();
bool IsFirstPowerON();
bool FilterLifeTimeExpired();
void StartReadyTimer();
bool ReadyTimerExpired();
void StartWaterPumpingTimer();

void RestartCO2Counter();

bool IsGuiControlMode();
void SolenoidPump(int itOn);
void SolenoidPumpUVPower(int itOn);


// Added for pure 2
bool WaterPumpTimerExpired();
void LedsSequence(eLedsSequence seq);
bool CarbonationEnabled();
bool IsBottleFull();
bool Tilted();
void CheckUVError(bool isOnWakeup);
bool IsUVLedCheckDone(bool isOnWakeup);
bool LedsPlayDone();

void ResetFilterDaysCounter();
void StartFilterToCarbDelay();
bool FilterToCarbDelayDone();
#endif /* INC_SMINTERFACE_H_ */
