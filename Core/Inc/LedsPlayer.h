/*
 * LedsPlayer.h
 *
 */

#ifndef INC_LEDSPLAYER_H_
#define INC_LEDSPLAYER_H_

#include "main.h"

#define LEDS_EASE_VECTOR_SIZE (256)


// DEBUG REMOVE / CHANGE
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

// DEBUG REMOVE / CHANGE

typedef enum {
	eAnimation_none,
	eAnimation_InitalSetup,
	eAnimation_MakeADrinkProgress,
	eAnimation_MakeADrinkSuccess,
	eAnimation_StartUp,
	eAnimation_MakeADrink,
}eAnimations;


typedef enum {
	eLEdEase_constant, // when there is no transition
	eLedEase_InOutQuad,
	eLedEase_OutExpo,

	eLedEase_num_of_ease
}eLedEaseFuncs;

// Step is a single led step fade in or out or staying constant
// when it is played in an array > 1, the delay sets the starting point
// therefore steps can overlap or even played parallel
typedef struct {
	int32_t ledIdMask;
	uint16_t delayMS;       // MSecs to wait before starting the step when it is part of array of steps (otherwise 0)
	uint8_t startPercent;   // starting intensity 0-255
	uint8_t endPercent;     // ending intensity 0-255
	uint8_t totalSteps10ms; // total playing steps
	uint16_t totalMs;       // actual length in ms for precision
	eLedEaseFuncs easeFunc; // ease function
}sLedsStep;

// Sequence is a list of steps (1 or N) played according the the delay starting time of each
// and therefore can be overlapping
#define ENDLESS_LOOP (0xff)
#define MAX_NUMBER_OF_SUBSEQ (10)
typedef struct {
	uint8_t sequenceLen;     // number of steps in subSeq[]
	uint16_t delayMS;        // MSecs to wait before starting the sequence
	const sLedsStep *subSeq; // pointer to that sequence (array of steps)
	uint8_t loop;            // 0 = play once, n = repeat n times, 0xFF = endless
	// in case the loop is overlapping itself, what is the overlapping time i.e. at what point of time to start the next iteration of the loop while the previous is still playing
	// 0 - no overlapping, N - start again at the N's ms (relative to loop's length)
	uint16_t overlappingLoop;

} sLedsSequence;

// Flow is a list sequences played one after the other
typedef struct {
	const sLedsSequence *seq;
	uint8_t length;
} sLedsFlowDef;


uint8_t EaseLUT_PlaySegment(
    eLedEaseFuncs easeFunc,   // which LUT to use
    uint16_t step,            // current step, from 0..totalSteps-1
    uint16_t totalSteps,      // number of steps in the animation
    uint8_t startPct,         // start percent of LUT (0..255)
    uint8_t endPct);           // end percent of LUT (0..255)

void StartAnimation(eAnimations animation);
void StopCurrentAnimation(bool letLoopEnd);

#endif /* INC_LEDSPLAYER_H_ */
