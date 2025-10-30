/*
 * LedsPlayer.c
 */
#include "LedsPlayer.h"
#ifndef _MSC_VER
#include "WS2811.h"
#include "RTC.h"
#include "FRAM.h"
#endif

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


sLedsStep clearAllLedsStep = {
        eLED_ALL_LEDS,   0, 255, 0, 10, 100, eLedEase_OutExpo
};

// this is a generick blinking sequence, when playing, the ledIdMask is set dynamically
#define LEDFLOW_BLINKING_LOOP_STEPS (4)
sLedsStep stepsBlinking[LEDFLOW_BLINKING_LOOP_STEPS] = {
        {eLED_FilterOrange,   0,   0, 0, 12, 120, eLEdEase_constant},
        {eLED_FilterOrange,   120,   0, 255, 6, 64, eLedEase_OutExpo},
        {eLED_FilterOrange,   184,   255, 255, 12, 120, eLEdEase_constant},
        {eLED_FilterOrange,   184,   255, 0, 6, 64, eLedEase_OutExpo}
};

//#define LEDFLOW_RING_STARTUP_STEPS (5)
//sLedsStep stepsRingStartup[LEDFLOW_RING_STARTUP_STEPS] = {
//        {eLED_Circle7,   0,   0, 255, 16, 160, eLedEase_InOutQuad},
//        {eLED_Circle8 | eLED_Circle6,  80,   0, 255, 16, 160, eLedEase_InOutQuad},
//        {eLED_Circle1 | eLED_Circle5, 160,   0, 255, 16, 160, eLedEase_InOutQuad},
//        {eLED_Circle2 | eLED_Circle4, 240,   0, 255, 16, 160, eLedEase_InOutQuad},
//        {eLED_Circle3, 320,   0, 255, 16, 160, eLedEase_InOutQuad}
//};

#define LEDFLOW_STARTUP_CIRCLE_STEPS (5)
sLedsStep stepsStartupCircle[LEDFLOW_STARTUP_CIRCLE_STEPS] = {
        // Steps of "RING Strtup
        {eLED_Circle7,          0,   0, 255, 16, 160, eLedEase_InOutQuad},
        {eLED_Circle8 | eLED_Circle6,  80,   0, 255, 16, 160, eLedEase_InOutQuad},
        {eLED_Circle1 | eLED_Circle5, 160,   0, 255, 16, 160, eLedEase_InOutQuad},
        {eLED_Circle2 | eLED_Circle4, 240,   0, 255, 16, 160, eLedEase_InOutQuad},
        {eLED_Circle3,        320,   0, 255, 16, 160, eLedEase_InOutQuad},
};

#define LEDFLOW_STARTUP_CARB_LEVEL_STEPS (4)
sLedsStep stepsStartupCarbLevel[LEDFLOW_STARTUP_CARB_LEVEL_STEPS] = {
        // Steps of "Startup (Splash)"
        {eLED_LevelNoneWhite,   0,   0, 255, 10, 100, eLedEase_OutExpo},
        {eLED_LevelLowWhite,  100,   0, 255, 10, 100, eLedEase_OutExpo},
        {eLED_LevelMedWhite,  200,   0, 255, 10, 100, eLedEase_OutExpo},
        {eLED_LevelHighWhite, 300,   0, 255, 10, 100, eLedEase_OutExpo},
};

#define LEDFLOW_STARTUP_FILTER_STEPS (1)
sLedsStep stepsStartupFilter[LEDFLOW_STARTUP_FILTER_STEPS] = {
        {eLED_FilterWhite,    0,   0, 255, 10, 100, eLedEase_OutExpo},
};

#define LEDFLOW_INTERSTITIAL_STEPS (1)
sLedsStep stepsInterstitial[LEDFLOW_INTERSTITIAL_STEPS] = {
        {ALL_WHITE_LEDS_MASK,      0, 255,   0, 10, 100, eLedEase_OutExpo},
};

#define LEDFLOW_SHOWSTATUS_NORMAL_STEPS (1)
sLedsStep stepsShowStatusNormal[LEDFLOW_SHOWSTATUS_NORMAL_STEPS] = {
        // ON leds (will be set at run time)
        {eLED_ALL_LEDS,      0,   0, 255, 10, 100, eLedEase_OutExpo},
};

#if 0
// Original values taken from the Excel file
#define LEDFLOW_RING_PROGRESS_LOOP_STEPS (16)
sLedsStep stepsRingProgress[LEDFLOW_RING_PROGRESS_LOOP_STEPS] = {
        {eLED_Circle3,   0, 255,   0, 13, 128, eLedEase_InOutQuad},
        {eLED_Circle4,   0,   0, 255,  6,  64, eLedEase_InOutQuad},
        {eLED_Circle4,  64, 255,   0, 13, 128, eLedEase_InOutQuad},
        {eLED_Circle5,  64,   0, 255,  6,  64, eLedEase_InOutQuad},
        {eLED_Circle5, 128, 255,   0, 13, 128, eLedEase_InOutQuad},
        {eLED_Circle6, 128,   0, 255,  6,  64, eLedEase_InOutQuad},
        {eLED_Circle6, 192, 255,   0, 13, 128, eLedEase_InOutQuad},
        {eLED_Circle7, 192,   0, 255,  6,  64, eLedEase_InOutQuad},
        {eLED_Circle7, 256, 255,   0, 13, 128, eLedEase_InOutQuad},
        {eLED_Circle8, 256,   0, 255,  6,  64, eLedEase_InOutQuad},
        {eLED_Circle8, 320, 255,   0, 13, 128, eLedEase_InOutQuad},
        {eLED_Circle1, 320,   0, 255,  6,  64, eLedEase_InOutQuad},
        {eLED_Circle1, 384, 255,   0, 13, 128, eLedEase_InOutQuad},
        {eLED_Circle2, 384,   0, 255,  6,  64, eLedEase_InOutQuad},
        {eLED_Circle2, 448, 255,   0, 13, 128, eLedEase_InOutQuad},
        {eLED_Circle3, 448,   0, 255,  6,  64, eLedEase_InOutQuad}
};
#define LEDFLOW_RING_PROGRESS_INNER_LOOP_OVERLAPPING (64)
#endif
// Modified values to make the progress according to the figma design
// Factor: 1.8333 slower
#define LEDFLOW_RING_PROGRESS_LOOP_STEPS (16)
sLedsStep stepsRingProgress[LEDFLOW_RING_PROGRESS_LOOP_STEPS] = {
        {eLED_Circle3,   0, 255,   0, 24, 240, eLedEase_InOutQuad},
        {eLED_Circle4,   0,   0, 255, 12, 120, eLedEase_InOutQuad},
        {eLED_Circle4, 120, 255,   0, 24, 240, eLedEase_InOutQuad},
        {eLED_Circle5, 120,   0, 255, 12, 120, eLedEase_InOutQuad},
        {eLED_Circle5, 240, 255,   0, 24, 240, eLedEase_InOutQuad},
        {eLED_Circle6, 240,   0, 255, 12, 120, eLedEase_InOutQuad},
        {eLED_Circle6, 360, 255,   0, 24, 240, eLedEase_InOutQuad},
        {eLED_Circle7, 360,   0, 255, 12, 120, eLedEase_InOutQuad},
        {eLED_Circle7, 480, 255,   0, 24, 240, eLedEase_InOutQuad},
        {eLED_Circle8, 480,   0, 255, 12, 120, eLedEase_InOutQuad},
        {eLED_Circle8, 600, 255,   0, 24, 240, eLedEase_InOutQuad},
        {eLED_Circle1, 600,   0, 255, 12, 120, eLedEase_InOutQuad},
        {eLED_Circle1, 720, 255,   0, 24, 240, eLedEase_InOutQuad},
        {eLED_Circle2, 720,   0, 255, 12, 120, eLedEase_InOutQuad},
        {eLED_Circle2, 840, 255,   0, 24, 240, eLedEase_InOutQuad},
        {eLED_Circle3, 840,   0, 255, 12, 120, eLedEase_InOutQuad}
};
#define LEDFLOW_RING_PROGRESS_INNER_LOOP_OVERLAPPING (120)

#if 0
// Original values taken from the Excel file
#define LEDFLOW_RING_PROGRESS_SEQUENCE_LEN (2)
sLedsSequence sequenceRingProgress[LEDFLOW_RING_PROGRESS_SEQUENCE_LEN] = {
        { 1,   0, (sLedsStep[]){ {eLED_Circle3, 0, 0, 255, 24, 240, eLedEase_OutExpo}}, 0, 0 },
        { LEDFLOW_RING_PROGRESS_LOOP_STEPS, 240, stepsRingProgress, ENDLESS_LOOP, LEDFLOW_RING_PROGRESS_INNER_LOOP_OVERLAPPING },
};
#endif
// Modified values to make the progress according to the figma design
// Factor: 1.8333 slower
#define LEDFLOW_RING_PROGRESS_SEQUENCE_LEN (2)
sLedsSequence sequenceRingProgress[LEDFLOW_RING_PROGRESS_SEQUENCE_LEN] = {
        { 1,   0, (sLedsStep[]) { { eLED_Circle3, 0, 0, 255, 44, 440, eLedEase_OutExpo } }, 0, 0 },
        { LEDFLOW_RING_PROGRESS_LOOP_STEPS, 440, stepsRingProgress, ENDLESS_LOOP, LEDFLOW_RING_PROGRESS_INNER_LOOP_OVERLAPPING },
};

#define LEDFLOW_RING_SUCCESS_INTERSTITIAL_SEQUENCE_LEN (1)
sLedsSequence sequenceMakeDrinkSuccessInterstitial[LEDFLOW_RING_SUCCESS_INTERSTITIAL_SEQUENCE_LEN] = {
        { 1,   0, (sLedsStep[]){ {eLED_Circle3, 0, 255, 0, 36, 360, eLedEase_OutExpo}}, 0, 0 },
};

#define LEDFLOW_RING_SUCCESS_INNER_LOOP_STEPS (4)
sLedsStep stepsRingSuccessInnerLoop[LEDFLOW_RING_SUCCESS_INNER_LOOP_STEPS] = {
        {eLED_Circle1 | eLED_Circle2 | eLED_Circle3 | eLED_Circle4 | eLED_Circle5 | eLED_Circle6 | eLED_Circle7 | eLED_Circle8,   0, 255,   0,  6, 64, eLedEase_InOutQuad},
        {eLED_Circle1 | eLED_Circle2 | eLED_Circle3 | eLED_Circle4 | eLED_Circle5 | eLED_Circle6 | eLED_Circle7 | eLED_Circle8,  64,   0,   0,  6, 64, eLEdEase_constant},
        {eLED_Circle1 | eLED_Circle2 | eLED_Circle3 | eLED_Circle4 | eLED_Circle5 | eLED_Circle6 | eLED_Circle7 | eLED_Circle8, 128,   0, 255,  6, 64, eLedEase_OutExpo},
        {eLED_Circle1 | eLED_Circle2 | eLED_Circle3 | eLED_Circle4 | eLED_Circle5 | eLED_Circle6 | eLED_Circle7 | eLED_Circle8, 192, 255, 255, 12, 120, eLEdEase_constant}
};
#define LEDFLOW_RING_SUCCESS_INNER_LOOP_REPEAT (3)

#define LEDFLOW_RING_SUCCESS_SEQUENCE_LEN (7)
sLedsSequence sequenceRingSuccess[LEDFLOW_RING_SUCCESS_SEQUENCE_LEN] = {
        { 1, 360, (sLedsStep[]){ {eLED_Circle1 | eLED_Circle5, 0, 0, 255, 12, 120, eLedEase_InOutQuad}}, 0, 0 },
        { 1, 420, (sLedsStep[]){ {eLED_Circle2 | eLED_Circle4 | eLED_Circle8 | eLED_Circle6, 0, 0, 255, 12, 120, eLedEase_InOutQuad}}, 0, 0 },
        { 1, 480, (sLedsStep[]){ {eLED_Circle3 | eLED_Circle7 , 0, 0, 255, 12, 120, eLedEase_InOutQuad}}, 0, 0 },
        { LEDFLOW_RING_SUCCESS_INNER_LOOP_STEPS, 840, stepsRingSuccessInnerLoop, LEDFLOW_RING_SUCCESS_INNER_LOOP_REPEAT, 0 },
        { 1, 2296, (sLedsStep[]){ {eLED_Circle1 | eLED_Circle5, 0, 255, 0, 12, 120, eLedEase_InOutQuad}}, 0, 0 },
        { 1, 2356, (sLedsStep[]){ {eLED_Circle2 | eLED_Circle4 | eLED_Circle8 | eLED_Circle6, 0, 255, 0, 12, 120, eLedEase_InOutQuad}}, 0, 0 },
        { 1, 2416, (sLedsStep[]){ {eLED_Circle3 | eLED_Circle7 , 0, 255, 0, 12, 120, eLedEase_InOutQuad}}, 0, 0 },
};


//--------------------- Ring Loader Steps (Rinsing/Filtering) -------------------
#define LEDFLOW_RING_LOADER_LOOP_STEPS (4)
sLedsStep stepsRingLoaderLoop[LEDFLOW_RING_LOADER_LOOP_STEPS] = {
    {eLED_Circle2 | eLED_Circle3 | eLED_Circle4 | eLED_Circle6 | eLED_Circle7 | eLED_Circle8,   0, 255,   0, 32, 320, eLedEase_InOutQuad},
    {eLED_Circle1 | eLED_Circle5,   0,  0, 255, 32, 320, eLedEase_InOutQuad},
    {eLED_Circle2 | eLED_Circle3 | eLED_Circle4 | eLED_Circle6 | eLED_Circle7 | eLED_Circle8, 320,   0, 255, 32, 320, eLedEase_InOutQuad},
    {eLED_Circle1 | eLED_Circle5, 320, 255,  0, 32, 640, eLedEase_InOutQuad},
};

#define LEDFLOW_RING_LOADER_START_SEQUENCE_LEN (2)
sLedsSequence sequenceRingLoaderStart[LEDFLOW_RING_LOADER_START_SEQUENCE_LEN] = {
        { 1,   0, (sLedsStep[]){ {eLED_Circle2 | eLED_Circle3 | eLED_Circle4 | eLED_Circle6 | eLED_Circle7 | eLED_Circle8,   0,   0, 255, 32, 320, eLedEase_InOutQuad}}, 0, 0 },
        { LEDFLOW_RING_LOADER_LOOP_STEPS, 320, stepsRingLoaderLoop, ENDLESS_LOOP, 0 },
};

#define LEDFLOW_RING_LOADER_END_SEQUENCE_LEN (1)
sLedsSequence sequenceRingLoaderEnd[LEDFLOW_RING_LOADER_END_SEQUENCE_LEN] = {
        { 1,   0, (sLedsStep[]){ {eLED_Circle2 | eLED_Circle3 | eLED_Circle4 | eLED_Circle6 | eLED_Circle7 | eLED_Circle8,   0, 255, 0, 32, 320, eLedEase_InOutQuad}}, 0, 0 },
};


//--------------------- Display status of CO2 and Filter  -------------------


// This is a special sequence that is overwritten before starting to play is according to the actual status
// of the CO2 and Filter
#define STATUS_CARB_IDX (0)
#define STATUS_FILTER_IDX (1)
#define LEDFLOW_DISPLAY_STATUS_STEPS (2)
sLedsSequence sequenceDisplayStatus[LEDFLOW_DISPLAY_STATUS_STEPS] = {
        { 1, 0,(sLedsStep[]){ {eLED_LevelNoneWhite,   0, 0,   255, 10, 100, eLedEase_OutExpo}}, 0, 0 },
        { 1, 0, (sLedsStep[]){ {eLED_FilterWhite,   0, 0,   255, 10, 100, eLedEase_OutExpo}}, 0, 0 },
};


#define CO2_LEVEL_ON_IDX (0)
#define CO2_LEVEL_OFF_IDX (1)
#define LEDFLOW_DISPLAY_CO2_LEVEL_STEPS (2)
sLedsSequence sequenceCO2Level[LEDFLOW_DISPLAY_CO2_LEVEL_STEPS] = {
        // ON LEDS
        { 1, 0, (sLedsStep[]){ {eLED_LevelNoneWhite,   0,   0, 255, 10, 100, eLedEase_OutExpo}}, 0, 0 },
        // OFF LEDS
        { 1, 0, (sLedsStep[]){ {eLED_LevelNoneWhite,   0, 255,   0, 10, 100, eLedEase_OutExpo}}, 0, 0 },
};



// ////////////////////////////////////////////////////////  Main Animations  ////////////////////////////////////////////////////////
#define LEDS_FLOW_STARTUP_LEN (5)
#define STARTUP_FLOW_STATUS_SEQ_IDX (4)
sLedsFlowDef ledsFlowStartup[LEDS_FLOW_STARTUP_LEN] = {
    { (sLedsSequence[]){{LEDFLOW_STARTUP_CIRCLE_STEPS, 0, stepsStartupCircle, 0, 0 }}, 1},
    { (sLedsSequence[]){{LEDFLOW_STARTUP_CARB_LEVEL_STEPS, 0, stepsStartupCarbLevel, 0, 0}}, 1},
    { (sLedsSequence[]){{LEDFLOW_STARTUP_FILTER_STEPS, 0, stepsStartupFilter, 0, 0 }}, 1 },
    { (sLedsSequence[]){{LEDFLOW_INTERSTITIAL_STEPS, 400, stepsInterstitial, 0, 0 }}, 1 },
    { (sLedsSequence[]){{LEDFLOW_SHOWSTATUS_NORMAL_STEPS, 400, stepsShowStatusNormal, 0, 0 }}, 1 }
};
#define LEDS_FLOW_MAKE_A_DRINK_PROGRESS_LEN (1)
sLedsFlowDef ledsFlowMakeADrinkProgrees[LEDS_FLOW_MAKE_A_DRINK_PROGRESS_LEN] = {
        {sequenceRingProgress, LEDFLOW_RING_PROGRESS_SEQUENCE_LEN},
};

#define LEDS_FLOW_MAKE_A_DRINK_SUCCESS_LEN (2)
sLedsFlowDef ledsFlowMakeADrinkSuccess[LEDS_FLOW_MAKE_A_DRINK_SUCCESS_LEN] = {
        {sequenceMakeDrinkSuccessInterstitial, LEDFLOW_RING_SUCCESS_INTERSTITIAL_SEQUENCE_LEN},
        {sequenceRingSuccess, LEDFLOW_RING_SUCCESS_SEQUENCE_LEN},
};

#define LEDS_FLOW_START_LOADER_LEN (1)
sLedsFlowDef ledsFlowStartLoader[LEDS_FLOW_START_LOADER_LEN] = {
        {sequenceRingLoaderStart, LEDFLOW_RING_LOADER_START_SEQUENCE_LEN}
};

#define LEDS_FLOW_END_LOADER_LEN (1)
sLedsFlowDef ledsFlowEndLoader[LEDS_FLOW_END_LOADER_LEN] = {
        {sequenceRingLoaderEnd, LEDFLOW_RING_LOADER_END_SEQUENCE_LEN}
};

#define LEDS_FLOW_DISPLAY_STATUS_LEN (1)
sLedsFlowDef ledsFlowDisplayStatus[LEDS_FLOW_DISPLAY_STATUS_LEN] = {
        {sequenceDisplayStatus, LEDFLOW_DISPLAY_STATUS_STEPS}
};

#define LEDS_FLOW_CO2_ELVEL_LEN (1)
sLedsFlowDef ledsFlowCO2LevelStatus[LEDS_FLOW_CO2_ELVEL_LEN] = {
        {sequenceCO2Level, LEDFLOW_DISPLAY_CO2_LEVEL_STEPS}
};


///--- Global Animation Parameters ----------------------------------------------------------------------------------------
uint8_t gLeds[NUMBER_OF_LEDS] = {0};
uint32_t gAnimationStartingMS = 0;
uint8_t gCurrentFlowStep = 0;
uint16_t gCurrentFlowLoopEntryMS[MAX_NUMBER_OF_SUBSEQ] = {0};
bool gStopRequested = false;


// Current playing animation
uint16_t gCurrentFlowTotalSteps = 0;
sLedsFlowDef *pCurrentFlow = NULL;  // A flow of sequences
eAnimations gCurrentAnimation = eAnimation_none;

// Pending animation to start after the current one ends
uint16_t gPendingFlowTotalSteps = 0;
sLedsFlowDef *pPendingFlow = NULL;  // A flow of sequences
eAnimations gPendingAnimation = eAnimation_none;


void ZeroGlobalAnimationParams(bool zeroCurrent, bool zeroPendingToo);
void SetCurrentFlowLoopEntryMSValues(sLedsSequence *seq, uint8_t len);
bool IsPendingAnimation(void);
uint32_t OOTBGetCarbLevelLedStatusMask(void);
uint32_t OOTBGetFilterStatusMask(void);
uint32_t GetCarbLevelLedStatusMask(void);
uint32_t GetFilterStatusMask(void);
void GetCO2ChangedOnOffMasks(uint32_t *onMask, uint32_t *offMask);

eAnimations gLastAnimation = eAnimation_none; // TODO DEBUG Remove

void StartAnimation(eAnimations animation, bool forceStopPrevious)
{
    uint16_t requestedFlowTotalSteps = 0;
    sLedsFlowDef *requestedFlow = NULL;  // A flow of sequences
    uint32_t val;

    switch(animation)
    {
    case eAnimation_InitalSetup:
        requestedFlow = NULL;
        requestedFlowTotalSteps = 0;
        break;

    case eAnimation_StartUp:
        FRAM_ReadElement(eFRAM_isFirstTimeSetupRequired, &val);
        requestedFlow = ledsFlowStartup;
        requestedFlowTotalSteps = LEDS_FLOW_STARTUP_LEN;
        // check if normal startup
        if (val == 0) {
            // update the status display
            ledsFlowStartup[STARTUP_FLOW_STATUS_SEQ_IDX].seq[0].subSeq[0].ledIdMask = GetCarbLevelLedStatusMask() | GetFilterStatusMask() | ALL_RING_LEDS_MASK;
        } else {
            // ignore the last step of status (shown on other flows)
            requestedFlowTotalSteps--;
        }

        break;

    case eAnimation_MakeADrinkProgress:
        requestedFlow = ledsFlowMakeADrinkProgrees;
        requestedFlowTotalSteps = LEDS_FLOW_MAKE_A_DRINK_PROGRESS_LEN;
        break;

    case eAnimation_MakeADrinkSuccess:
        requestedFlow = ledsFlowMakeADrinkSuccess;
        requestedFlowTotalSteps = LEDS_FLOW_MAKE_A_DRINK_SUCCESS_LEN;
        break;

    case eAnimation_RingLoaderStart:
        requestedFlow = ledsFlowStartLoader;
        requestedFlowTotalSteps = LEDS_FLOW_START_LOADER_LEN;
        break;

    case eAnimation_RingLoaderEnd:
        requestedFlow = ledsFlowEndLoader;
        requestedFlowTotalSteps = LEDS_FLOW_END_LOADER_LEN;
        break;

    case eAnimation_Status:
        // Set the required leds based on current status
        ledsFlowDisplayStatus[0].seq[STATUS_CARB_IDX].subSeq[0].ledIdMask = GetCarbLevelLedStatusMask() | ALL_RING_LEDS_MASK;
        ledsFlowDisplayStatus[0].seq[STATUS_FILTER_IDX].subSeq[0].ledIdMask = GetFilterStatusMask();
        requestedFlow = ledsFlowDisplayStatus;
        requestedFlowTotalSteps = LEDS_FLOW_DISPLAY_STATUS_LEN;
        break;

    case eAnimation_OOTBStatus:
		// Set the required leds based on current OOTB status
        ledsFlowDisplayStatus[0].seq[STATUS_CARB_IDX].subSeq[0].ledIdMask = OOTBGetCarbLevelLedStatusMask();
        ledsFlowDisplayStatus[0].seq[STATUS_FILTER_IDX].subSeq[0].ledIdMask = OOTBGetFilterStatusMask();
        requestedFlow = ledsFlowDisplayStatus;
        requestedFlowTotalSteps = LEDS_FLOW_DISPLAY_STATUS_LEN;
        break;

    case eAnimation_CO2Level:
        GetCO2ChangedOnOffMasks(
                &ledsFlowCO2LevelStatus[0].seq[CO2_LEVEL_ON_IDX].subSeq[0].ledIdMask,
                &ledsFlowCO2LevelStatus[0].seq[CO2_LEVEL_OFF_IDX].subSeq[0].ledIdMask);
        requestedFlow = ledsFlowCO2LevelStatus;
        requestedFlowTotalSteps = LEDS_FLOW_CO2_ELVEL_LEN;
        break;

    // TODO - this is here to cover animations that are not yet implemented
    case eAnimation_StartUpCO2: // startup animation for CO2 only leds (part of the "StartUp (Splash)" animation)
    case eAnimation_FilterWarning: // filter warning animation base on number of days left
    case eAnimation_CO2Warning: // CO2 warning animation, currently implemented only on OOTB state
        animation = eAnimation_none;
        break;

    default: // also for eAnimation_none
        requestedFlow = NULL;
        requestedFlowTotalSteps = 0;
        break;
    }

    // If need toe queue as pending
    if (IsAnimationActive() && !forceStopPrevious) {
        // queue as pending
        gPendingAnimation = animation;
        pPendingFlow = requestedFlow;
        gPendingFlowTotalSteps = requestedFlowTotalSteps;

        return; // done
    }
    // else: start immediately (cancelling any pending)
    gCurrentAnimation = animation;
    gLastAnimation = gCurrentAnimation; // TODO DEBUG Remove
    pCurrentFlow = requestedFlow;
    gCurrentFlowTotalSteps = requestedFlowTotalSteps;

    // some general initialisations of the flow

    gStopRequested = false;

    gCurrentFlowStep = 0;
    gAnimationStartingMS = HAL_GetTick();
    if (pCurrentFlow != NULL) {
        SetCurrentFlowLoopEntryMSValues(pCurrentFlow[0].seq, pCurrentFlow[0].length);
    }

}

// This will stop the current animation and optionally let it end the current loop
// If there is a pending animation, it will CANCEL it !!
void StopCurrentAnimation(bool letLoopEnd)
{
    // cancel pending animation
    ZeroGlobalAnimationParams(false, true);

    if (letLoopEnd) {
        gStopRequested = true;
    } else {
        // immediate stop
        ZeroGlobalAnimationParams(true, false); // the second param is not needed as pending is already cleared above
    }

}
bool IsAnimationActive(void)
{
    return (gCurrentAnimation != eAnimation_none);
}
bool IsPendingAnimation(void)
{
    return (gPendingAnimation != eAnimation_none);
}

//uint32_t ddii = 0;
//uint32_t ddolt[32];
//uint32_t ddnt[32];
//uint32_t ddoplt[32];
//
//uint32_t ddsq = 0;

uint16_t offset_in_loop = 0;
uint16_t offset_in_prev_loop = 0;
uint16_t offset_in_inner_step = 0;
uint16_t sqlen = 0;

uint32_t ddd = 0;

void PlayLedsPeriodic(void)
{
    uint32_t now = HAL_GetTick();
    uint32_t elapsed = now - gAnimationStartingMS;
    uint8_t j;

    // Handle special animations

    // Clear leds from last value
    if (gCurrentAnimation & CLEAR_ALL_LEDS_ANIMATION_MASK) {
        uint8_t val = 0;
        if (elapsed < clearAllLedsStep.totalMs) {
            val = EaseLUT_PlaySegment(&clearAllLedsStep, elapsed / 10);
        }
        uint32_t mask = 1;
        for (j = 0; j < NUMBER_OF_LEDS; j++) {
            // if the current animation is OOTB CO2/Filter down , skip leds that are not in the mask
            if (((gCurrentAnimation == eAnimation_OOTBCO2Down) && !(mask & CLEAR_OOTB_CO2_LEDS_MASK)) ||
                ((gCurrentAnimation == eAnimation_OOTBFilterDown) && !(mask & CLEAR_OOTB_FILTER_LEDS_MASK))) {
                mask <<= 1;
             continue;
            }
            // this is the "trick" - setting value only id smaller then previous (and member of the mask)
            if ((clearAllLedsStep.ledIdMask & mask) && (val < gLeds[j])){
                gLeds[j] = val; // or blend logic
            }
            mask <<= 1;
        }
        // Set the LEDs
        WS_SetLeds(gLeds, NUMBER_OF_LEDS);
        // To stop it when done (done after the above for loop since gCurrentAnimation may be used there)
        if (elapsed >= clearAllLedsStep.totalMs) {
            gCurrentAnimation = eAnimation_none;
        }
        return;
    }

    if (pCurrentFlow == NULL)
        return;


    // Get the current Flow element
    sLedsSequence *fseq = pCurrentFlow[gCurrentFlowStep].seq;
    uint8_t fseqLen = pCurrentFlow[gCurrentFlowStep].length;
    // This assume: (1) done, unless a sequence is still active
    // (2) at lease one sequence is starting from the beginning of the flow element (otherwise we need to track that not all are before delayMS)
    bool currentFlowIsDone = true;

#ifdef _MSC_VER
    extern uint32_t winElapsedTime;
    if (winElapsedTime == 80)
    {
        ddd = 1;
	}
#endif

    // Loop over the sequences of the current flow element
    for (uint8_t sq = 0; sq < fseqLen; sq++) {
        //ddsq = sq;
        sLedsSequence *seq = &fseq[sq];
        sqlen = seq->sequenceLen;
        sLedsStep *stp;
        uint8_t ssi = 0;
        // Handle loops
        if (seq->loop > 0) {  // loop
            // if we are inside of it
            if (elapsed >= seq->delayMS) { // already time to play it
                if (((seq->loop == ENDLESS_LOOP) || (elapsed <= seq->delayMS + (seq->loop) * gCurrentFlowLoopEntryMS[sq]))) { // not yet finished all loops
                    currentFlowIsDone = false; // at least one sequence still active
                    /*uint16_t*/ offset_in_loop = (elapsed - seq->delayMS)
                        % (gCurrentFlowLoopEntryMS[sq] - seq->overlappingLoop);
                    // Go over the loop elements
                    for (ssi = 0; ssi < seq->sequenceLen; ssi++) {
                        stp = &seq->subSeq[ssi];
                        if ((offset_in_loop >= stp->delayMS) && (offset_in_loop <= (stp->delayMS + stp->totalMs))) {
                            uint8_t val = EaseLUT_PlaySegment(stp, (offset_in_loop - stp->delayMS) / 10);
                            uint32_t mask = 1;
                            for (j = 0; j < NUMBER_OF_LEDS; j++) {
                                if (stp->ledIdMask & mask) {
                                    gLeds[j] = val; // or blend logic
                                }
                                mask <<= 1;
                            }
                        }
                    }
                    if ((seq->overlappingLoop) && // loop is overlapping itself
                        (offset_in_loop <= seq->overlappingLoop) && // previous iteration should still play
                        (elapsed > (seq->delayMS + (gCurrentFlowLoopEntryMS[sq] - seq->overlappingLoop)))) { // not the first iteration
                        offset_in_prev_loop = offset_in_loop + (gCurrentFlowLoopEntryMS[sq] - seq->overlappingLoop);
                        // we are in the overlapping time, so play the last steps that are in that time
                        for (uint8_t ssi = 0; ssi < seq->sequenceLen; ssi++) {
                            stp = &seq->subSeq[ssi];
                            if ((offset_in_prev_loop >= stp->delayMS) && (offset_in_prev_loop <= (stp->delayMS + stp->totalMs))) {

                                uint8_t val = EaseLUT_PlaySegment(stp, (offset_in_prev_loop - stp->delayMS) / 10);

                                uint32_t mask = 1;
                                for (j = 0; j < NUMBER_OF_LEDS; j++) {

                                    if (stp->ledIdMask & mask) {
                                        gLeds[j] = val; // or blend logic
                                    }
                                    mask <<= 1;
                                }
                            }
                        }
                    }
                }
            }
            else {
                // not yet time to play the loop
                currentFlowIsDone = false; // still active
            }
		}
        else { 
            // check if time to play this step
            if (elapsed >= seq->delayMS) {
                offset_in_inner_step = elapsed - seq->delayMS;
                for (ssi = 0; ssi < seq->sequenceLen; ssi++)
                {
                    stp = &seq->subSeq[ssi];
                    // check if time to play the inner step
                    if (offset_in_inner_step >= stp->delayMS) {
                        if (offset_in_inner_step <= stp->delayMS + stp->totalMs) {
                            currentFlowIsDone = false; // at least one sequence still active
                            uint8_t val = EaseLUT_PlaySegment(stp, (offset_in_inner_step - stp->delayMS) / 10);
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
                    else {
                        // not yet time to play the inner step
                        currentFlowIsDone = false; // still active
                    }
                }
            } else {
                // not yet time to play this step
				currentFlowIsDone = false; //  still active
            }
        }
    }
    // Set the LEDs
    WS_SetLeds(gLeds, NUMBER_OF_LEDS);

    if ((gStopRequested) || ((currentFlowIsDone) && ((gCurrentFlowStep+1) >= gCurrentFlowTotalSteps))){  // Stop requested or entire all flows are done
        // finished all flows
        // If there is a pending animation, start it now
        if (IsPendingAnimation()) {
            pCurrentFlow = pPendingFlow;
            gCurrentFlowTotalSteps = gPendingFlowTotalSteps;
            gCurrentAnimation = gPendingAnimation;
            // this must be done after copying the pending to current
            ZeroGlobalAnimationParams(false, true);
            SetCurrentFlowLoopEntryMSValues(pCurrentFlow[0].seq, pCurrentFlow[0].length);
        } else {
            // no pending animation, just stop
            ZeroGlobalAnimationParams(true, true); // The second true is redundant here (no pending) but just in case
        }
        gCurrentFlowStep = 0;
    } else if (currentFlowIsDone) { // move to next flow entry
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

void ZeroGlobalAnimationParams(bool zeroCurrent, bool zeroPendingToo)
{
    gStopRequested = false;
    if (zeroCurrent)
    {
        gCurrentAnimation = eAnimation_none;
        pCurrentFlow = NULL;
        gCurrentFlowTotalSteps = 0;
        gCurrentFlowStep = 0;
        gAnimationStartingMS = 0;
    }
    if (zeroPendingToo)
    {
        gPendingAnimation = eAnimation_none;
        pPendingFlow = NULL;
        gPendingFlowTotalSteps = 0;
    }
}

void SetCurrentFlowLoopEntryMSValues(sLedsSequence *seq, uint8_t len)
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
//uint8_t dd = 0;
uint8_t EaseLUT_PlaySegment(
    sLedsStep *StepInfo, // pointer to the step info
    uint16_t step)            // current step, from 0..totalSteps-1
{
    if (StepInfo->easeFunc == eLEdEase_constant) {
        return StepInfo->startPercent; // no transition
    }

    if (StepInfo->easeFunc >= eLedEase_num_of_ease || StepInfo->totalSteps10ms < 1) {
        return 0; // simple guard against invalid enum or divide by zero
    }
    // if we are at the last step just return the endPercent
    if (step >= (StepInfo->totalSteps10ms - 1)) {
    //  if ((StepInfo->totalMs - (step * 10)) < 10)  {
    //      if  ((StepInfo->totalMs - (step * 10)) > 7) {
    //          int dd = 1;
		  //}
        return StepInfo->endPercent;
    }

    // Delta can be negative
    int32_t delta = (int32_t)StepInfo->endPercent - (int32_t)StepInfo->startPercent;
    // Calculate the index in the LUT (0..255)
    uint32_t lutIndex = (int32_t)StepInfo->startPercent + (delta * step) / (int32_t)(StepInfo->totalSteps10ms);

    // also clip if we are at the end and going up
    if (lutIndex >= LEDS_EASE_VECTOR_SIZE) {
        lutIndex = LEDS_EASE_VECTOR_SIZE - 1;
    }
    return gLedEaseData[StepInfo->easeFunc][lutIndex];
}

uint32_t OOTBGetCarbLevelLedStatusMask(void)
{
    // This code assumes that
    uint32_t val = 0;
    FRAM_ReadElement(eFRAM_isCO2OOTBResetRequired, &val);
    bool isCo2Warning = (val != 0);

    val = 0; // used as mask now
    val |= eLED_LevelNoneWhite;
    if (gCarbonationLevel >= eLevel_Low) {
        val  = 0; // clear the "none"
        val |= ((isCo2Warning) ? eLED_LevellowOrange : eLED_LevelLowWhite);
    }
    if (gCarbonationLevel >= eLevel_medium) {
        val |= ((isCo2Warning) ? eLED_LevelMedOrange : eLED_LevelMedWhite);
    }
    if (gCarbonationLevel >= eLevel_high) {
        val |= ((isCo2Warning) ? eLED_LevelHighOrange : eLED_LevelHighWhite);
    }
    return val | ALL_RING_LEDS_MASK;
}

uint32_t OOTBGetFilterStatusMask(void)
{
    uint32_t val = 0;
    FRAM_ReadElement(eFRAM_isFilterOOTBResetRequired, &val);
    return ((val == 0) ? eLED_FilterWhite : eLED_FilterOrange) | ALL_RING_LEDS_MASK;


}
uint32_t GetCarbLevelLedStatusMask(void)
{
    // This code assumes that
    uint32_t mask = 0;
    mask |= eLED_LevelNoneWhite;
    if (gCarbonationLevel >= eLevel_Low) {
        mask  = 0; // clear the "none"
        mask |= eLED_LevelLowWhite;
    }
    if (gCarbonationLevel >= eLevel_medium) {
        mask |= eLED_LevelMedWhite;
    }
    if (gCarbonationLevel >= eLevel_high) {
        mask |= eLED_LevelHighWhite;
    }
    return mask;
}

void GetCO2ChangedOnOffMasks(uint32_t *onMask, uint32_t *offMask)
{
    switch (gCarbonationLevel)
    {
    case eLevel_off:
        *onMask = eLED_LevelNoneWhite;
        *offMask = eLED_LevelLowWhite | eLED_LevelMedWhite | eLED_LevelHighWhite;
        break;
    case eLevel_Low:
        *onMask = eLED_LevelLowWhite;
        *offMask = eLED_LevelNoneWhite;
        break;
    case eLevel_medium:
        *onMask = eLED_LevelMedWhite;
        *offMask = 0;
        break;
    case eLevel_high:
        *onMask = eLED_LevelHighWhite;
        *offMask = 0;
        break;
    default:
        *onMask = 0;
        *offMask = 0;
        break;
    }
}


uint32_t GetFilterStatusMask(void)
{
    return ((IsInFilterReplacementWarningPeriod() || IsFilterExpired()) ? eLED_FilterOrange : 0); // nothing shown if filter is OK
}
