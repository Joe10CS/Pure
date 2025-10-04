/*
 * LedsPlayer.h
 *
 */

#ifndef INC_LEDSPLAYER_H_
#define INC_LEDSPLAYER_H_

#include "main.h"

#define LEDS_EASE_VECTOR_SIZE (256)


// DEBUG REMOVE
// THIS DEFINITION IS FOR THE TEST HARDWARE WITH A CIRCLE OF 8 RGB LEDs

typedef enum {
	eLEd_Circle1R = 0x01,
	eLEd_Circle1G = 0x02,
	eLEd_Circle1B = 0x04,
	eLEd_Circle2R = 0x08,
	eLEd_Circle2G = 0x10,
	eLEd_Circle2B = 0x20,
	eLEd_Circle3R = 0x40,
	eLEd_Circle3G = 0x80,
	eLEd_Circle3B = 0x100,
	eLEd_Circle4R = 0x200,
	eLEd_Circle4G = 0x400,
	eLEd_Circle4B = 0x800,
	eLEd_Circle5R = 0x1000,
	eLEd_Circle5G = 0x2000,
	eLEd_Circle5B = 0x4000,
	eLEd_Circle6R = 0x8000,
	eLEd_Circle6G = 0x10000,
	eLEd_Circle6B = 0x20000,
	eLEd_Circle7R = 0x40000,
	eLEd_Circle7G = 0x80000,
	eLEd_Circle7B = 0x100000,
	eLEd_Circle8R = 0x200000,
	eLEd_Circle8G = 0x400000,
	eLEd_Circle8B = 0x800000,
}eLedIds;

// DEBUG REMOVE


typedef enum {
	eLedEase_InOutQuad,
	eLedEase_OutExpo,

	eLedEase_num_of_ease
}eLedEaseFuncs;


typedef struct {
	int32_t ledIdMask;
	uint16_t delayMS;       // MSecs to wait before starting
	uint8_t startPercent;   // starting intensity 0-255
	uint8_t endPercent;     // ending intensity 0-255
	uint8_t totalSteps10ms; // total playing steps
	uint16_t totalMs;      // actual length in ms for precision
	eLedEaseFuncs easeFunc; // ease function
}sOneLedSeq;


uint8_t EaseLUT_PlaySegment(
    eLedEaseFuncs easeFunc,   // which LUT to use
    uint16_t step,            // current step, from 0..totalSteps-1
    uint16_t totalSteps,      // number of steps in the animation
    uint8_t startPct,         // start percent of LUT (0..255)
    uint8_t endPct);           // end percent of LUT (0..255)

#endif /* INC_LEDSPLAYER_H_ */
