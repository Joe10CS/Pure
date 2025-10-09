/*
 * LedsPlayer.c
 */
#include "LedsPlayer.h"
#include "WS2811.h"

void SetCurrentFlowLoopEntryMSValues(const sLedsSequence *seq, uint8_t len);

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
}sLedsStep;
 */

#define LEDFLOW_RING_STARTUP_STEPS (5)
sLedsStep stepsRingStartup[LEDFLOW_RING_STARTUP_STEPS] = {
        {eLEd_Circle7B,   0,   0, 255, 16, 160, eLedEase_InOutQuad},
        {eLEd_Circle8B | eLEd_Circle6B,  80,   0, 255, 16, 160, eLedEase_InOutQuad},
        {eLEd_Circle1B | eLEd_Circle5B, 160,   0, 255, 16, 160, eLedEase_InOutQuad},
        {eLEd_Circle2B | eLEd_Circle4B, 240,   0, 255, 16, 160, eLedEase_InOutQuad},
        {eLEd_Circle3B, 320,   0, 255, 16, 160, eLedEase_InOutQuad}
};


#define LEDFLOW_RING_PROGRESS_LOOP_STEPS (16)
sLedsStep stepsRingProgress[LEDFLOW_RING_PROGRESS_LOOP_STEPS] = {
        {eLEd_Circle3B,   0, 255,   0, 13, 128, eLedEase_InOutQuad},
        {eLEd_Circle4B,   0,   0, 255,  6,  64, eLedEase_InOutQuad},
        {eLEd_Circle4B,  64, 255,   0, 13, 128, eLedEase_InOutQuad},
        {eLEd_Circle5B,  64,   0, 255,  6,  64, eLedEase_InOutQuad},
        {eLEd_Circle5B, 128, 255,   0, 13, 128, eLedEase_InOutQuad},
        {eLEd_Circle6B, 128,   0, 255,  6,  64, eLedEase_InOutQuad},
        {eLEd_Circle6B, 192, 255,   0, 13, 128, eLedEase_InOutQuad},
        {eLEd_Circle7B, 192,   0, 255,  6,  64, eLedEase_InOutQuad},
        {eLEd_Circle7B, 256, 255,   0, 13, 128, eLedEase_InOutQuad},
        {eLEd_Circle8B, 256,   0, 255,  6,  64, eLedEase_InOutQuad},
        {eLEd_Circle8B, 320, 255,   0, 13, 128, eLedEase_InOutQuad},
        {eLEd_Circle1B, 320,   0, 255,  6,  64, eLedEase_InOutQuad},
        {eLEd_Circle1B, 384, 255,   0, 13, 128, eLedEase_InOutQuad},
        {eLEd_Circle2B, 384,   0, 255,  6,  64, eLedEase_InOutQuad},
        {eLEd_Circle2B, 448, 255,   0, 13, 128, eLedEase_InOutQuad},
        {eLEd_Circle3B, 448,   0, 255,  6,  64, eLedEase_InOutQuad}
};
#define LEDFLOW_RING_PROGRESS_INNER_LOOP_OVERLAPPING (64)

#define LEDFLOW_RING_PROGRESS_SEQUENCE_LEN (2)
const sLedsSequence sequenceRingProgress[LEDFLOW_RING_PROGRESS_SEQUENCE_LEN] = {
        { 1,   0, (const sLedsStep[]){ {eLEd_Circle3B, 0, 0, 255, 24, 240, eLedEase_OutExpo}}, 0, 0 },
        { LEDFLOW_RING_PROGRESS_LOOP_STEPS, 240, stepsRingProgress, ENDLESS_LOOP, LEDFLOW_RING_PROGRESS_INNER_LOOP_OVERLAPPING },
};

#define LEDFLOW_RING_SUCCESS_INTERSTITIAL_SEQUENCE_LEN (1)
const sLedsSequence sequenceMakeDrinkSuccessInterstitial[LEDFLOW_RING_SUCCESS_INTERSTITIAL_SEQUENCE_LEN] = {
        { 1,   0, (const sLedsStep[]){ {eLEd_Circle3B, 0, 255, 0, 36, 360, eLedEase_OutExpo}}, 0, 0 },
};

#define LEDFLOW_RING_SUCCESS_INNER_LOOP_STEPS (4)
sLedsStep stepsRingSuccessInnerLoop[LEDFLOW_RING_SUCCESS_INNER_LOOP_STEPS] = {
        {eLEd_Circle1B | eLEd_Circle5B | eLEd_Circle2B | eLEd_Circle4B | eLEd_Circle2B | eLEd_Circle4B | eLEd_Circle3B | eLEd_Circle7B,   0, 255,   0, 6, 64, eLedEase_InOutQuad},
        {eLEd_Circle1B | eLEd_Circle5B | eLEd_Circle2B | eLEd_Circle4B | eLEd_Circle2B | eLEd_Circle4B | eLEd_Circle3B | eLEd_Circle7B,  64,   0,   0, 6, 64, eLEdEase_constant},
        {eLEd_Circle1B | eLEd_Circle5B | eLEd_Circle2B | eLEd_Circle4B | eLEd_Circle2B | eLEd_Circle4B | eLEd_Circle3B | eLEd_Circle7B, 128,   0, 255, 6, 64, eLedEase_OutExpo},
        {eLEd_Circle1B | eLEd_Circle5B | eLEd_Circle2B | eLEd_Circle4B | eLEd_Circle2B | eLEd_Circle4B | eLEd_Circle3B | eLEd_Circle7B, 192, 255, 255, 6, 64, eLEdEase_constant}
};
#define LEDFLOW_RING_SUCCESS_INNER_LOOP_REPEAT (3)

#define LEDFLOW_RING_SUCCESS_SEQUENCE_LEN (7)
sLedsSequence sequenceRingSuccess[LEDFLOW_RING_SUCCESS_SEQUENCE_LEN] = {
        { 1, 360, (const sLedsStep[]){ {eLEd_Circle1B | eLEd_Circle5B, 0, 0, 255, 12, 120, eLedEase_InOutQuad}}, 0, 0 },
        { 1, 420, (const sLedsStep[]){ {eLEd_Circle2B | eLEd_Circle4B | eLEd_Circle8B | eLEd_Circle6B, 0, 0, 255, 12, 120, eLedEase_InOutQuad}}, 0, 0 },
        { 1, 480, (const sLedsStep[]){ {eLEd_Circle3B | eLEd_Circle7B , 0, 0, 255, 12, 120, eLedEase_InOutQuad}}, 0, 0 },
        { LEDFLOW_RING_SUCCESS_INNER_LOOP_STEPS, 840, stepsRingSuccessInnerLoop, LEDFLOW_RING_SUCCESS_INNER_LOOP_REPEAT, 0 },
        { 1, 2296, (const sLedsStep[]){ {eLEd_Circle1B | eLEd_Circle5B, 0, 255, 0, 12, 120, eLedEase_InOutQuad}}, 0, 0 },
        { 1, 2356, (const sLedsStep[]){ {eLEd_Circle2B | eLEd_Circle4B | eLEd_Circle8B | eLEd_Circle6B, 0, 255, 0, 12, 120, eLedEase_InOutQuad}}, 0, 0 },
        { 1, 2416, (const sLedsStep[]){ {eLEd_Circle3B | eLEd_Circle7B , 0, 255, 0, 12, 120, eLedEase_InOutQuad}}, 0, 0 },
};


//#define LEDFLOW_RING_OUT_STEPS (1)
//const sLedsSequence ledsFlow_RingOut[LEDFLOW_RING_OUT_STEPS] = {
//	//   len delay                     led          delay start% end%  10ms  totalms  ease               repeat
//	    { 1, 0, (const sLedsStep[]){ {eLEd_Circle1B, 0,   0,       0,  36,    360,   eLEdEase_constant}}, 0 }, // this is just a dummy sequence for the delay
//};


//#define LEDFLOW_RING_SUCCESS_INNER_LOOP_STEPS (4)
//sLedsStep ledsFlow_RingSuccessInnerLoop[LEDFLOW_RING_SUCCESS_INNER_LOOP_STEPS] = {
//	{eLEd_Circle1B | eLEd_Circle5B | eLEd_Circle2B | eLEd_Circle4B | eLEd_Circle2B | eLEd_Circle4B | eLEd_Circle3B | eLEd_Circle7B,   0, 255,   0, 6, 64, eLedEase_InOutQuad},
//	{eLEd_Circle1B | eLEd_Circle5B | eLEd_Circle2B | eLEd_Circle4B | eLEd_Circle2B | eLEd_Circle4B | eLEd_Circle3B | eLEd_Circle7B,  64,   0,   0, 6, 64, eLEdEase_constant},
//	{eLEd_Circle1B | eLEd_Circle5B | eLEd_Circle2B | eLEd_Circle4B | eLEd_Circle2B | eLEd_Circle4B | eLEd_Circle3B | eLEd_Circle7B, 128,   0, 255, 6, 64, eLedEase_OutExpo},
//	{eLEd_Circle1B | eLEd_Circle5B | eLEd_Circle2B | eLEd_Circle4B | eLEd_Circle2B | eLEd_Circle4B | eLEd_Circle3B | eLEd_Circle7B, 192,   0,   0, 6, 64, eLEdEase_constant}
//}
//#define LEDFLOW_RING_SUCCESS_INNER_LOOP_REPEAT (3)
//
//#define LEDFLOW_RING_SUCCESS_STEPS (7)
//const sLedsSequence ledsFlow_RingSuccess[LEDFLOW_RING_SUCCESS_STEPS] = {
//	    { 1, 360, (const sLedsStep[]){ {eLEd_Circle1B | eLEd_Circle5B, 0, 0, 255, 12, 120, eLedEase_InOutQuad}}, 0, 0 },
//	    { 1, 420, (const sLedsStep[]){ {eLEd_Circle2B | eLEd_Circle4B | eLEd_Circle2B | eLEd_Circle4B, 0, 0, 255, 12, 120, eLedEase_InOutQuad}}, 0, 0 },
//	    { 1, 480, (const sLedsStep[]){ {eLEd_Circle3B | eLEd_Circle7B , 0, 0, 255, 12, 120, eLedEase_InOutQuad}}, 0, 0 },
//		{ LEDFLOW_RING_SUCCESS_INNER_LOOP_STEPS, 840, ledsFlow_RingSuccessInnerLoop, LEDFLOW_RING_SUCCESS_INNER_LOOP_REPEAT, 0 },
//	    { 1, 360, (const sLedsStep[]){ {eLEd_Circle1B | eLEd_Circle5B, 0, 255, 0, 12, 120, eLedEase_InOutQuad}}, 0, 0 },
//	    { 1, 420, (const sLedsStep[]){ {eLEd_Circle2B | eLEd_Circle4B | eLEd_Circle2B | eLEd_Circle4B, 0, 255, 0, 12, 120, eLedEase_InOutQuad}}, 0, 0 },
//	    { 1, 480, (const sLedsStep[]){ {eLEd_Circle3B | eLEd_Circle7B , 0, 255, 0, 12, 120, eLedEase_InOutQuad}}, 0, 0 },
//};


#define LEDS_FLOW_MAKE_A_DRINK_PROGRESS_LEN (1)
sLedsFlowDef ledsFlowMakeADrinkProgrees[LEDS_FLOW_MAKE_A_DRINK_PROGRESS_LEN] = {
        {sequenceRingProgress, LEDFLOW_RING_PROGRESS_SEQUENCE_LEN},
};

#define LEDS_FLOW_MAKE_A_DRINK_SUCCESS_LEN (2)
sLedsFlowDef ledsFlowMakeADrinkSuccess[LEDS_FLOW_MAKE_A_DRINK_SUCCESS_LEN] = {
        {sequenceMakeDrinkSuccessInterstitial, LEDFLOW_RING_SUCCESS_INTERSTITIAL_SEQUENCE_LEN},
        {sequenceRingSuccess, LEDFLOW_RING_SUCCESS_SEQUENCE_LEN},
};

uint8_t gLeds[NUMBER_OF_LEDS] = {0};
uint32_t gAnimationStartingMS = 0;
uint8_t gCurrentFlowStep = 0;
uint16_t gCurrentFlowLoopEntryMS[MAX_NUMBER_OF_SUBSEQ] = {0};
bool gStopRequested = false;

uint16_t gCurrentFlowTotalSteps = 0;
sLedsFlowDef *pCurrentFlow = NULL;  // A flow of sequences

void StartAnimation(eAnimations animation)
{
    switch(animation)
    {
    case eAnimation_InitalSetup:
        pCurrentFlow = NULL;
        gCurrentFlowTotalSteps = 0;
        break;

    case eAnimation_StartUp:
        break;

    case eAnimation_MakeADrinkProgress:
        pCurrentFlow = ledsFlowMakeADrinkProgrees;
        gCurrentFlowTotalSteps = LEDS_FLOW_MAKE_A_DRINK_PROGRESS_LEN;
        break;

    case eAnimation_MakeADrinkSuccess:
        pCurrentFlow = ledsFlowMakeADrinkSuccess;
        gCurrentFlowTotalSteps = LEDS_FLOW_MAKE_A_DRINK_SUCCESS_LEN;
        break;

    default: // also for eAnimation_none
        pCurrentFlow = NULL;
        break;
    }
    // some general initialisations of the flow

    gStopRequested = false;

    if (pCurrentFlow != NULL) {
        gCurrentFlowStep = 0;
        gAnimationStartingMS = HAL_GetTick();
        SetCurrentFlowLoopEntryMSValues(pCurrentFlow[0].seq, pCurrentFlow[0].length);
    }

}

void StopCurrentAnimation(bool letLoopEnd)
{
    if (letLoopEnd) {
        gStopRequested = true;
    } else {
        // immediate stop
        pCurrentFlow = NULL;
        gCurrentFlowTotalSteps = 0;
    }

}
uint32_t ddd = 0;
void PlayLedsPeriodic(void)
{
    if (pCurrentFlow == NULL)
        return;

    uint32_t now = HAL_GetTick();

    // Get the current Flow element
    const sLedsSequence *fseq = pCurrentFlow[gCurrentFlowStep].seq;
    uint8_t fseqLen = pCurrentFlow[gCurrentFlowStep].length;
    uint32_t elapsed = now - gAnimationStartingMS;
    // This assume: (1) done, unless a sequence is still active
    // (2) at lease one sequence is starting from the beginning of the flow element (otherwise we need to track that not all are before delayMS)
    bool currentFlowIsDone = true;

    // Loop over the sequences of the current flow element
    for (uint8_t sq = 0; sq < fseqLen; sq++) {
        const sLedsSequence *seq = &fseq[sq];
        uint8_t sqlen = seq->sequenceLen;
        const sLedsStep *stp;
        uint8_t j;
        for (uint8_t st = 0; st < sqlen; st++) {
            // Handle loops
            if (seq->loop > 0) {  // loop
                // if we are inside of it
                if ((elapsed >= seq->delayMS) && // already time to play it
                        ((seq->loop == ENDLESS_LOOP) || (elapsed < seq->delayMS + (seq->loop) * gCurrentFlowLoopEntryMS[sq]))) { // not yet finished all loops
                    currentFlowIsDone = false; // at least one sequence still active
                    uint16_t offset_in_loop = (elapsed - seq->delayMS) % (gCurrentFlowLoopEntryMS[sq] - seq->overlappingLoop);
                    // Go over the loop elements
                    for (uint8_t ssi = 0; ssi < seq->sequenceLen; ssi++) {
                        if ((ssi == 14) && offset_in_loop > 500) {
                            ddd++;
                        }
                        stp = &seq->subSeq[ssi];
                        if ((offset_in_loop >= stp->delayMS) && (offset_in_loop <= (stp->delayMS + stp->totalMs))) {
                            uint8_t val = EaseLUT_PlaySegment(stp->easeFunc,
                                    (offset_in_loop - stp->delayMS) / 10,
                                    stp->totalSteps10ms,
                                    stp->startPercent,
                                    stp->endPercent);

                            uint32_t mask = 1;
                            for (j = 0; j < NUMBER_OF_LEDS; j++) {
                                if (stp->ledIdMask & mask)
                                    gLeds[j] = val; // or blend logic
                                mask <<= 1;
                            }
                        }
                    }
                    // if loop is overlapping itself, check if previous iteration should still play
                    if (offset_in_loop < seq->overlappingLoop) { // we are in the overlapping time
                        uint16_t offset_in_prev_loop = offset_in_loop + (gCurrentFlowLoopEntryMS[sq] - seq->overlappingLoop);
                        // we are in the overlapping time, so play the last steps that are in that time
                        for (uint8_t ssi = 0; ssi < seq->sequenceLen; ssi++) {
                            stp = &seq->subSeq[ssi];
                            if ((offset_in_prev_loop >= stp->delayMS) && (offset_in_prev_loop <= (stp->delayMS + stp->totalMs))) {
                                uint8_t val = EaseLUT_PlaySegment(stp->easeFunc,
                                        (offset_in_prev_loop - stp->delayMS) / 10,
                                        stp->totalSteps10ms,
                                        stp->startPercent,
                                        stp->endPercent);

                                uint32_t mask = 1;
                                for (j = 0; j < NUMBER_OF_LEDS; j++) {
                                    if (stp->ledIdMask & mask)
                                        gLeds[j] = val; // or blend logic
                                    mask <<= 1;
                                }
                            }
                        }
}
                    // if loop is overlapping itself, check if next iteration should start
                    if (offset_in_loop > (gCurrentFlowLoopEntryMS[sq] - seq->overlappingLoop)) { // we are in the overlapping time
                        uint16_t offset_in_next_loop = offset_in_loop - (gCurrentFlowLoopEntryMS[sq] - seq->overlappingLoop);
                        // we are in the overlapping time, so play the first steps that are in that time
                        for (uint8_t ssi = 0; ssi < seq->sequenceLen; ssi++) {
                            stp = &seq->subSeq[ssi];
                            if ((offset_in_next_loop >= stp->delayMS) && (offset_in_next_loop <= (stp->delayMS + stp->totalMs))) {
                                uint8_t val = EaseLUT_PlaySegment(stp->easeFunc,
                                        (offset_in_next_loop - stp->delayMS) / 10,
                                        stp->totalSteps10ms,
                                        stp->startPercent,
                                        stp->endPercent);

                                uint32_t mask = 1;
                                for (j = 0; j < NUMBER_OF_LEDS; j++) {
                                    if (stp->ledIdMask & mask)
                                        gLeds[j] = val; // or blend logic
                                    mask <<= 1;
                                }
                            }
                        }

                    }
                }
            } else {
                stp = &seq->subSeq[st];
                if ((elapsed >= stp->delayMS) && (elapsed <= stp->delayMS + stp->totalMs)) {
                    currentFlowIsDone = false; // at least one sequence still active
                    uint8_t val = EaseLUT_PlaySegment(stp->easeFunc,
                            (elapsed - stp->delayMS)/10,
                            stp->totalSteps10ms,
                            stp->startPercent,
                            stp->endPercent);
                    uint32_t mask = 1;
                    for (j = 0; j < NUMBER_OF_LEDS; j++)
                    {
                        if (stp->ledIdMask & mask)
                        {
                            gLeds[j] = val;
                        }
                        mask <<= 1;
                    }
                }
            }
        }
    }
    // Set the LEDs
    WS_SetLeds(gLeds, NUMBER_OF_LEDS);

    // move to next flow element if all sequences done
    if (currentFlowIsDone && (!gStopRequested)) {
        gCurrentFlowStep++;
        if (gCurrentFlowStep >= gCurrentFlowTotalSteps) {
            // finished all flows
            pCurrentFlow = NULL;
            gCurrentFlowTotalSteps = 0;
            gCurrentFlowStep = 0;
        } else {
            // prepare next flow entry
            gAnimationStartingMS = now;
            SetCurrentFlowLoopEntryMSValues(pCurrentFlow[gCurrentFlowStep].seq, pCurrentFlow[gCurrentFlowStep].length);
        }
    }
}


void SetCurrentFlowLoopEntryMSValues(const sLedsSequence *seq, uint8_t len)
{
    for (uint8_t i = 0; i < len; i++)	{
        uint16_t max = 0;
        // calculate only for loop entries
        if (seq[i].loop > 0) {
            for (uint8_t j = 0; j < seq[i].sequenceLen; j++) {
                uint16_t ms = seq[i].subSeq[j].delayMS + seq[i].subSeq[j].totalMs;
                if (max < ms) {
                    max = ms;
                }
            }
        }
        gCurrentFlowLoopEntryMS[i] = max;
    }
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
    if (easeFunc == eLEdEase_constant) {
        return startPct * 255 / 100; // no transition
    }
    int32_t delta = (int32_t)endPct - (int32_t)startPct;
    int32_t lutIndex = (int32_t)startPct + (delta * step) / (int32_t)(totalSteps - 1);

    // clip to valid LUT range [0..LEDS_EASE_VECTOR_SIZE-1]
    if ((lutIndex < 0)  ||
        ((step >= (totalSteps - 1)) && (startPct > endPct)))
            {
        lutIndex = 0;
    }
    if ((lutIndex >= LEDS_EASE_VECTOR_SIZE) ||
       ((step >= (totalSteps - 1))&&(startPct < endPct))) {
        lutIndex = LEDS_EASE_VECTOR_SIZE - 1;
    }
    return gLedEaseData[easeFunc][lutIndex];
}

