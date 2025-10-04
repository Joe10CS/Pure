/*
 * LedsPlayer.c
 */
#include "LedsPlayer.h"
#include "WS2811.h"

uint8_t gLedEaseData[eLedEase_num_of_ease][LEDS_EASE_VECTOR_SIZE] = {
	{ // eLedEase_InOutQuad
	0,0,0,0,0,0,0,0,1,1,1,1,1,1,2,2,2,2,3,3,3,3,4,4,5,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,13,13,14,15,15,16,17,17,18,19,20,20,21,22,23,24,25,25,26,27,28,29,30,31,
	32,33,34,35,36,37,38,40,41,42,43,44,45,47,48,49,50,51,53,54,55,57,58,59,61,62,64,65,66,68,69,71,72,74,75,77,78,80,82,83,85,86,88,90,91,93,95,97,98,100,102,104,106,107,109,111,113,115,117,119,121,123,125,127,
	128,130,132,134,136,138,140,142,144,146,148,149,151,153,155,157,158,160,162,164,165,167,169,170,172,173,175,177,178,180,181,183,184,186,187,189,190,191,193,194,196,197,198,200,201,202,204,205,206,207,208,210,211,212,213,214,215,217,218,219,220,221,222,223,
	224,225,226,227,228,229,230,230,231,232,233,234,235,235,236,237,238,238,239,240,240,241,242,242,243,244,244,245,245,246,246,247,247,248,248,249,249,250,250,250,251,251,252,252,252,252,253,253,253,253,254,254,254,254,254,254,255,255,255,255,255,255,255,255,
	},
	{ // eLedEase_OutExpo
	0,7,13,20,26,32,38,44,50,55,61,66,71,76,81,85,90,94,99,103,107,111,115,119,122,126,129,133,136,139,142,145,148,151,154,157,159,162,164,167,169,171,174,176,178,180,182,184,186,188,189,191,193,195,196,198,199,201,202,204,205,206,208,209,
	210,211,213,214,215,216,217,218,219,220,221,222,223,224,224,225,226,227,228,228,229,230,230,231,232,232,233,234,234,235,235,236,236,237,237,238,238,239,239,239,240,240,241,241,241,242,242,243,243,243,243,244,244,244,245,245,245,245,246,246,246,246,247,247,
	247,247,248,248,248,248,248,249,249,249,249,249,249,249,250,250,250,250,250,250,250,251,251,251,251,251,251,251,251,251,252,252,252,252,252,252,252,252,252,252,252,252,252,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,254,254,254,
	254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	}
};

/*
typedef struct {
	int32_t ledIdMask;
	uint16_t delayMS;     // steps to wait before starting
	uint8_t startPercent;   // starting intensity 0-255
	uint8_t endPercent;     // ending intensity 0-255
	uint8_t totalSteps10ms; // total playing steps
	uint16_t totalMs;      // actual length in ms for precision
	eLedEaseFuncs easeFunc; // ease function
}sOneLedSeq;
 */


#define LEDFLOW_TEST_LOOP_STEPS (18)
sOneLedSeq testLoopFlow [LEDFLOW_TEST_LOOP_STEPS] = {
//		                                          delay start end steps totalMs
//	{eLEd_Circle3B,   0, 255,   0, 12, 128, eLedEase_InOutQuad},
	{eLEd_Circle4B,   0,   0, 255,  6,  64, eLedEase_InOutQuad},
	{eLEd_Circle4B,  64, 255,   0, 12, 128, eLedEase_InOutQuad},
	{eLEd_Circle5B,  64,   0, 255,  6,  64, eLedEase_InOutQuad},
	{eLEd_Circle5B, 128, 255,   0, 12, 128, eLedEase_InOutQuad},
	{eLEd_Circle6B, 128,   0, 255,  6,  64, eLedEase_InOutQuad},
	{eLEd_Circle6B, 192, 255,   0, 12, 128, eLedEase_InOutQuad},
	{eLEd_Circle7B, 192,   0, 255,  6,  64, eLedEase_InOutQuad},
	{eLEd_Circle7B, 356, 255,   0, 12, 128, eLedEase_InOutQuad},
	{eLEd_Circle8B, 356,   0, 255,  6,  64, eLedEase_InOutQuad},
	{eLEd_Circle8B, 320, 255,   0, 12, 128, eLedEase_InOutQuad},
	{eLEd_Circle1B, 320,   0, 255,  6,  64, eLedEase_InOutQuad},
	{eLEd_Circle1B, 384, 255,   0, 12, 128, eLedEase_InOutQuad},
	{eLEd_Circle2B, 384,   0, 255,  6,  64, eLedEase_InOutQuad},
	{eLEd_Circle2B, 448, 255,   0, 12, 128, eLedEase_InOutQuad},
	{eLEd_Circle3B, 448,   0, 255,  6,  64, eLedEase_InOutQuad},
	{eLEd_Circle3B, 512, 255,   0, 12, 128, eLedEase_InOutQuad},
	{eLEd_Circle4B, 512,   0, 255,  6,  64, eLedEase_InOutQuad},
	{eLEd_Circle4B, 576, 255,   0, 12, 128, eLedEase_InOutQuad},
};

/*
#define LEDFLOW_TEST_LOOP_STEPS (19)
sOneLedSeq testLoopFlow [LEDFLOW_TEST_LOOP_STEPS] = {
//		                                          delay start end steps totalMs
	{eLEd_Circle3R | eLEd_Circle3G | eLEd_Circle3B,   0, 255,   0, 12, 128, eLedEase_InOutQuad},
	{eLEd_Circle4R | eLEd_Circle4G | eLEd_Circle4B,   0,   0, 255,  6,  64, eLedEase_InOutQuad},
	{eLEd_Circle4R | eLEd_Circle4G | eLEd_Circle4B, 304, 255, 128,  6,  64, eLedEase_InOutQuad},
	{eLEd_Circle5R | eLEd_Circle5G | eLEd_Circle5B, 304,   0, 255,  6,  64, eLedEase_InOutQuad},
	{eLEd_Circle5R | eLEd_Circle5G | eLEd_Circle5B, 368, 255, 128,  6,  64, eLedEase_InOutQuad},
	{eLEd_Circle6R | eLEd_Circle6G | eLEd_Circle6B, 368,   0, 255,  6,  64, eLedEase_InOutQuad},
	{eLEd_Circle6R | eLEd_Circle6G | eLEd_Circle6B, 432, 255, 128,  6,  64, eLedEase_InOutQuad},
	{eLEd_Circle7R | eLEd_Circle7G | eLEd_Circle7B, 432,   0, 255,  6,  64, eLedEase_InOutQuad},
	{eLEd_Circle7R | eLEd_Circle7G | eLEd_Circle7B, 496, 255, 128,  6,  64, eLedEase_InOutQuad},
	{eLEd_Circle8R | eLEd_Circle8G | eLEd_Circle8B, 496,   0, 255,  6,  64, eLedEase_InOutQuad},
	{eLEd_Circle8R | eLEd_Circle8G | eLEd_Circle8B, 560, 255, 128,  6,  64, eLedEase_InOutQuad},
	{eLEd_Circle1R | eLEd_Circle1G | eLEd_Circle1B, 560,   0, 255,  6,  64, eLedEase_InOutQuad},
	{eLEd_Circle1R | eLEd_Circle1G | eLEd_Circle1B, 624, 255, 128,  6,  64, eLedEase_InOutQuad},
	{eLEd_Circle2R | eLEd_Circle2G | eLEd_Circle2B, 624,   0, 255,  6,  64, eLedEase_InOutQuad},
	{eLEd_Circle2R | eLEd_Circle2G | eLEd_Circle2B, 688, 255, 128,  6,  64, eLedEase_InOutQuad},
	{eLEd_Circle3R | eLEd_Circle3G | eLEd_Circle3B, 688,   0, 255,  6,  64, eLedEase_InOutQuad},
	{eLEd_Circle3R | eLEd_Circle3G | eLEd_Circle3B, 752, 255, 128,  6,  64, eLedEase_InOutQuad},
	{eLEd_Circle4R | eLEd_Circle4G | eLEd_Circle4B, 752,   0, 255,  6,  64, eLedEase_InOutQuad},
	{eLEd_Circle4R | eLEd_Circle4G | eLEd_Circle4B, 816, 255,   0,  6,  64, eLedEase_InOutQuad},
};

 */
uint8_t gLeds[NUMBER_OF_LEDS] = {0};
uint32_t start_time = 0;
bool l_first = true;
uint32_t rel_now = 0;
int i = 0;
int j = 0;
void PlayLedsPeriodic()
{
	if (l_first)
	{
		l_first = false;
		start_time = HAL_GetTick();
	}

	rel_now = HAL_GetTick() - start_time;
	if (rel_now > 704)
	{
		rel_now = 0;
		start_time = HAL_GetTick();
	}
	for (i = 0; i < LEDFLOW_TEST_LOOP_STEPS; i++)
	{
		if ((rel_now >= testLoopFlow[i].delayMS) && (rel_now <= testLoopFlow[i].delayMS + testLoopFlow[i].totalMs))
		{

			uint8_t val = EaseLUT_PlaySegment(testLoopFlow[i].easeFunc,
					(rel_now - testLoopFlow[i].delayMS)/10,
					testLoopFlow[i].totalSteps10ms,
					testLoopFlow[i].startPercent,
					testLoopFlow[i].endPercent);
			uint32_t mask = 1;
			for (j = 0; j < NUMBER_OF_LEDS; j++)
			{
				if (testLoopFlow[i].ledIdMask & mask)
				{
					gLeds[j] = val;
				}
				mask <<= 1;
			}
		}
	}
	WS_SetLeds(gLeds, NUMBER_OF_LEDS);
}


uint16_t lutIndex = 0;
uint8_t EaseLUT_PlaySegment(
    eLedEaseFuncs easeFunc,   // which LUT to use
    uint16_t step,            // current step, from 0..totalSteps-1
    uint16_t totalSteps,      // number of steps in the animation
    uint8_t startPct,         // start percent of LUT (0..255)
    uint8_t endPct)           // end percent of LUT (0..255)
{
    if (easeFunc >= eLedEase_num_of_ease || totalSteps < 2) {
        return 0; // simple guard against invalid enum or divide by zero
    }
    int32_t delta = (int32_t)endPct - (int32_t)startPct;
    int32_t lutIndex = (int32_t)startPct + (delta * step) / (int32_t)(totalSteps - 1);

    // clip to valid LUT range [0..LEDS_EASE_VECTOR_SIZE-1]
    if (lutIndex < 0) lutIndex = 0;
    if (lutIndex >= LEDS_EASE_VECTOR_SIZE) {
    	lutIndex = LEDS_EASE_VECTOR_SIZE - 1;
    }
    return gLedEaseData[easeFunc][lutIndex];
}

