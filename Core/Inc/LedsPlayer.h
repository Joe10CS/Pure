/*
 * LedsPlayer.h
 *
 */

#ifndef INC_LEDSPLAYER_H_
#define INC_LEDSPLAYER_H_

#ifndef _MSC_VER
#include "main.h"
#else
#include <stdint.h>
#include <stdbool.h>
#define NUMBER_OF_LEDS 24
#endif

#define LEDS_EASE_VECTOR_SIZE (256)

// This the actual Pure VDL LEDs mapping based on the Schematics
// Where the order of the WS2811 devices is as follows (order starting from the PWM output and on to the chain):
// U13 R - LED1 - 0x00001
// U13 G - LED2 - 0x00002
// U13 B - LED3 - 0x00004
// U14 R - LED4 - 0x00008
// U14 G - LED5 - 0x00010
// U14 B - LED6 - 0x00020
// U15 R - LED7 - 0x00040
// U15 G - LED8 - 0x00080
// U15 B - LED9 - Hw - Carbonation Level High white led - 0x00100
// U16 R - LED10 - Ho - Carbonation Level High orange led - 0x00200
// U16 G - LED11 - Rw - Filter Replace white led - 0x00400
// U16 B - LED12 - Ro - Filter Replace orange led - 0x00800
// U18 R - LED13 - Nw - No carbonation white led - 0x01000
// U18 G - LED14 - No - No carbonation orange led - 0x02000
// U18 B - LED15 - Lw - Carbonation Level Low white led - 0x04000
// U17 R - LED16 - Lo - Carbonation Level Low orange led - 0x08000
// U17 G - LED17 - Mw - Carbonation Level Medium white led - 0x10000
// U17 B - LED18 - Mo - Carbonation Level Medium orange led - 0x20000

typedef enum {
    eLEd_LED1 = 0x01,
    eLEd_LED2 = 0x02,
    eLEd_LED3 = 0x04,
    eLEd_LED4 = 0x08,
    eLEd_LED5 = 0x10,
    eLEd_LED6 = 0x20,
    eLEd_LED7 = 0x40,
    eLEd_LED8 = 0x80,
    eLEd_LED9 = 0x100,
    eLEd_LED10 = 0x200,
    eLEd_LED11 = 0x400,
    eLEd_LED12 = 0x800,
    eLEd_LED13 = 0x1000,
    eLEd_LED14 = 0x2000,
    eLEd_LED15 = 0x4000,
    eLEd_LED16 = 0x8000,
    eLEd_LED17 = 0x10000,
    eLEd_LED18 = 0x20000
}eLedMappings;

//#define USE_DEBUG_LEDS_MAPPING
#ifdef USE_DEBUG_LEDS_MAPPING
#ifndef _MSC_VER
#warning "Using debug LED mapping - for test hardware only"
#endif
#define ALL_LEDS_MASK (eLEd_Circle1B|eLEd_Circle2B|eLEd_Circle3B|eLEd_Circle4B|eLEd_Circle5B|eLEd_Circle6B|eLEd_Circle7B|eLEd_Circle8B)
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
}eLedIdsDebug;
// DEBUG REMOVE


// The values here should be bits corresponding to the actual LED hardware
// DEBUG all the values here are temporary for test hardware - need to be updated to real hardware LED mapping
typedef enum {
    eLED_Circle1 = eLEd_Circle1B,
    eLED_Circle2 = eLEd_Circle8B,
    eLED_Circle3 = eLEd_Circle7B,
    eLED_Circle4 = eLEd_Circle6B,
    eLED_Circle5 = eLEd_Circle5B,
    eLED_Circle6 = eLEd_Circle4B,
    eLED_Circle7 = eLEd_Circle3B,
    eLED_Circle8 = eLEd_Circle2B,
    eLED_LevelNoneWhite = 0x800001,  // TODO Stam value - update to real
    eLED_LevelNoneOrange = 0x800002,
    eLED_LevelLowWhite = 0x800003,
    eLED_LevellowOrange = 0x800004,
    eLED_LevelMedWhite = 0x800005,
    eLED_LevelMedrange = 0x800006,
    eLED_LevelHighWhite = 0x800007,
    eLED_LevelHighOrange = 0x800008,
    eLED_FilterWhite = 0x800009,
    eLED_FilterOrange = 0x80000A,
    eLED_ALL_LEDS = ALL_LEDS_MASK
}eLedIds;
#else
#define ALL_RING_LEDS_MASK (0xFF)
#define ALL_LEDS_MASK (0x3FFFF)
// The values here should be bits corresponding to the actual LED hardware
typedef enum {
    eLED_Circle1 = eLEd_LED1,
    eLED_Circle2 = eLEd_LED2,
    eLED_Circle3 = eLEd_LED3,
    eLED_Circle4 = eLEd_LED4,
    eLED_Circle5 = eLEd_LED5,
    eLED_Circle6 = eLEd_LED6,
    eLED_Circle7 = eLEd_LED7,
    eLED_Circle8 = eLEd_LED8,
    eLED_LevelNoneWhite = eLEd_LED13,
    eLED_LevelNoneOrange = eLEd_LED14,
    eLED_LevelLowWhite = eLEd_LED15,
    eLED_LevellowOrange = eLEd_LED16,
    eLED_LevelMedWhite = eLEd_LED17,
    eLED_LevelMedrange = eLEd_LED18,
    eLED_LevelHighWhite = eLEd_LED9,
    eLED_LevelHighOrange = eLEd_LED10,
    eLED_FilterWhite = eLEd_LED11,
    eLED_FilterOrange = eLEd_LED12,
    eLED_ALL_LEDS = ALL_LEDS_MASK
}eLedIds;
#endif
// U15 B - LED9 - Hw - Carbonation Level High white led - 0x00100
// U16 R - LED10 - Ho - Carbonation Level High orange led - 0x00200
// U16 G - LED11 - Rw - Filter Replace white led - 0x00400
// U16 B - LED12 - Ro - Filter Replace orange led - 0x00800
// U18 R - LED13 - Nw - No carbonation white led - 0x01000
// U18 G - LED14 - No - No carbonation orange led - 0x02000
// U18 B - LED15 - Lw - Carbonation Level Low white led - 0x04000
// U17 R - LED16 - Lo - Carbonation Level Low orange led - 0x08000
// U17 G - LED17 - Mw - Carbonation Level Medium white led - 0x10000
// U17 B - LED18 - Mo - Carbonation Level Medium orange led - 0x20000


typedef enum {
	eAnimation_none,
	eAnimation_InitalSetup,
	eAnimation_MakeADrinkProgress,
	eAnimation_MakeADrinkSuccess,
	eAnimation_StartUp,
	eAnimation_MakeADrink,
    eAnimation_RingLoaderStart, // Used for rinsing/priming the filter
    eAnimation_RingLoaderEnd, // Used for rinsing/priming the filter
    eAnimation_OOTBCO2Down,
    eAnimation_OOTBFilterDown,
    eAnimation_Status,  // generic status display animation (filter warning, CO2 warning)
    eAnimation_StartUpCO2, // startup animation for CO2 only leds (part of the "StartUp (Splash)" animation)
    eAnimation_FilterWarning, // filter warning animation base on number of days left
    eAnimation_CO2Warning, // CO2 warning animation, currently implemented only on OOTB state

	// special animation to clear leds from last value -
	// i.e if led is not at 100% it will go down from the last value it was
	// This can be a pending animation too so it will just take the leds from their last value
	// and turn them off smoothly
    eAnimation_ClearLedsFromLastValue,
}eAnimations;


typedef enum {
	eLedEase_InOutQuad,
	eLedEase_OutExpo,

	eLedEase_num_of_ease, // up to here - real ease functions with entries in gLedEaseData

    eLEdEase_constant, // when there is no transition
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
	sLedsStep *subSeq;       // pointer to that sequence (array of steps)
	uint8_t loop;            // 0 = play once, n = repeat n times, 0xFF = endless
	// in case the loop is overlapping itself, what is the overlapping period
	// i.e. total iteration time - overlappingLoop = time when next iteration starts
	// 0 - no overlapping, N - the preriod
	uint16_t overlappingLoop;

} sLedsSequence;

// Flow is a list sequences played one after the other
typedef struct {
	sLedsSequence *seq;
	uint8_t length;
} sLedsFlowDef;


uint8_t EaseLUT_PlaySegment(
    sLedsStep *StepInfo, // pointer to the step info
    uint16_t step);            // current step, from 0..totalSteps-1

void StartAnimation(eAnimations animation, bool forceStopPrevious);
void StopCurrentAnimation(bool letLoopEnd);
bool IsAnimationActive(void);
bool IsPendingAnimation(void);


#endif /* INC_LEDSPLAYER_H_ */
