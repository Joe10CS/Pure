/*
 * SMinterface.c
 *
 *  Created on: May 20, 2025
 *      Author: yossi
 */

#include "SMinterface.h"

eCarbonationLevel carbonationLevel = eCarbLevel_undef;



void StartCarbonation(eCarbonationLevel level) {}
void StopCarbonation() {}


void StartUVLEd() {}
void StopUVLed() {}

void StartWaterPump() {}
void StopWaterPump() {}

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

