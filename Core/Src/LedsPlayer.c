/*
 * LedsPlayer.c
 */
#include "LedsPlayer.h"
#ifndef _MSC_VER
#include "WS2811.h"
#ifdef DEBUG_STATE_MACHINE
#include "RxTxMsgs.h"
#endif
#else
#include <stdint.h>
#endif // _MSC_VER
#include "RTC.h"
#include "RtcBackupMemory.h"

// Easing functions data
// Note: this defined as const to keep it in Flash memory
const uint8_t gLedEaseData[eLedEase_num_of_ease][LEDS_EASE_VECTOR_SIZE] = {
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

sLedsStep clearAllLedsStep = {
        eLED_ALL_LEDS,   0, 255, 0, 10, 100, eLedEase_OutExpo
};


// this is a for the waring blinking loop (currently for Filter when near expiry)
#define LEDFLOW_QUICK_BLINKING_LOOP_STEPS (4)
const sLedsStep stepsQuickBlinking[LEDFLOW_QUICK_BLINKING_LOOP_STEPS] = {
        {eLED_FilterOrange,   0,   0,   0, 12, 120, eLEdEase_constant},
        {eLED_FilterOrange, 120,   0, 255,  6,  60, eLedEase_OutExpo},
        {eLED_FilterOrange, 184, 255, 255, 12, 120, eLEdEase_constant},
        {eLED_FilterOrange, 304, 255,   0,  6,  60, eLedEase_OutExpo}
};

// This is used for Device fault indication
#define LEDFLOW_ERROR_BLINKING_LOOP_STEPS (4)
const sLedsStep stepsErrorBlinckingLoop[LEDFLOW_ERROR_BLINKING_LOOP_STEPS] = {
        {ALL_ORANGE_CO2_AND_FILTER_MASK,     0,   0,   0, 65, 650, eLEdEase_constant},
        {ALL_ORANGE_CO2_AND_FILTER_MASK,   650,   0, 255, 35, 350, eLedEase_OutExpo},
        {ALL_ORANGE_CO2_AND_FILTER_MASK,  1000, 255, 255, 65, 650, eLEdEase_constant},
        {ALL_ORANGE_CO2_AND_FILTER_MASK,  1650, 255,   0, 35, 350, eLedEase_OutExpo}
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
const sLedsStep stepsStartupCircle[LEDFLOW_STARTUP_CIRCLE_STEPS] = {
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

// Modified values to make the progress according to the figma design
// Factor: 1.8333 slower
#define LEDFLOW_RING_PROGRESS_LOOP_STEPS (16)
const sLedsStep stepsRingProgress[LEDFLOW_RING_PROGRESS_LOOP_STEPS] = {
        {eLED_Circle3,   0, 255,   0, 22, 220, eLedEase_InOutQuad},
        {eLED_Circle4,   0,   0, 255, 11, 110, eLedEase_InOutQuad},
        {eLED_Circle4, 110, 255,   0, 22, 220, eLedEase_InOutQuad},
        {eLED_Circle5, 110,   0, 255, 11, 110, eLedEase_InOutQuad},
        {eLED_Circle5, 220, 255,   0, 22, 220, eLedEase_InOutQuad},
        {eLED_Circle6, 220,   0, 255, 11, 110, eLedEase_InOutQuad},
        {eLED_Circle6, 330, 255,   0, 22, 220, eLedEase_InOutQuad},
        {eLED_Circle7, 330,   0, 255, 11, 110, eLedEase_InOutQuad},
        {eLED_Circle7, 440, 255,   0, 22, 220, eLedEase_InOutQuad},
        {eLED_Circle8, 440,   0, 255, 11, 110, eLedEase_InOutQuad},
        {eLED_Circle8, 550, 255,   0, 22, 220, eLedEase_InOutQuad},
        {eLED_Circle1, 550,   0, 255, 11, 110, eLedEase_InOutQuad},
        {eLED_Circle1, 660, 255,   0, 22, 220, eLedEase_InOutQuad},
        {eLED_Circle2, 660,   0, 255, 11, 110, eLedEase_InOutQuad},
        {eLED_Circle2, 770, 255,   0, 22, 220, eLedEase_InOutQuad},
        {eLED_Circle3, 770,   0, 255, 11, 110, eLedEase_InOutQuad}
};
#define LEDFLOW_RING_PROGRESS_INNER_LOOP_OVERLAPPING (110)

// Modified values to make the progress according to the figma design
// Factor: 1.8333 slower
#define LEDFLOW_RING_PROGRESS_SEQUENCE_LEN (4)
#define IN_RING_CO2_WARNING_OFF_STEP_INDEX (2)
#define IN_RING_CO2_WARNING_ON_STEP_INDEX  (3)
sLedsSequence sequenceRingProgress[LEDFLOW_RING_PROGRESS_SEQUENCE_LEN] = {
        { 1,   0, (sLedsStep[]) { { eLED_Circle3, 0, 0, 255, 40, 400, eLedEase_OutExpo } }, 0, 0 },
        { LEDFLOW_RING_PROGRESS_LOOP_STEPS, 400, (sLedsStep *)stepsRingProgress, ENDLESS_LOOP, LEDFLOW_RING_PROGRESS_INNER_LOOP_OVERLAPPING },
        // two dummy steps to enable turning off the CO2 level white leds and turning on the orange the orange leds in case CO2
        // counter exceeded during drink making
        // note that the masks here must be 0 so they have no affect as long as the the warning is not needed
        { 1, 0, (sLedsStep[]){ {0, 0, 255, 0, 10, 100, eLedEase_OutExpo}}, 0, 0 },
        { 1, 0, (sLedsStep[]){ {0, 0, 0, 255, 10, 100, eLedEase_OutExpo}}, 0, 0 },
};
// Compilation warning note: the casting to (sLedsStep *) is needed to avoid a warning about initializing pointer to non-const from const array
// because stepsRingProgress is defined as const to be in flash memory

//#define LEDFLOW_RING_SUCCESS_FINISH_RING_SEQUENCE_LEN (2)
//sLedsSequence sequenceMakeDrinkSuccessFinishRing[LEDFLOW_RING_SUCCESS_FINISH_RING_SEQUENCE_LEN] = {
//        { 1,   0, (sLedsStep[]){ {eLED_Circle3, 0, 128, 0, 11, 110, eLedEase_OutExpo}}, 0, 0 }, // 50% fade out left
//        { 1,   0, (sLedsStep[]){ {eLED_Circle4, 0, 255, 0, 22, 220, eLedEase_OutExpo}}, 0, 0 }, // 100% fade out left
//};
#define LEDFLOW_RING_SUCCESS_FINISH_RING_SEQUENCE_LEN (2)
sLedsSequence sequenceMakeDrinkSuccessFinishRing[LEDFLOW_RING_SUCCESS_FINISH_RING_SEQUENCE_LEN] = {
        { 1,   0, (sLedsStep[]){ {eLED_Circle2, 0, 128, 0, 11, 110, eLedEase_OutExpo}}, 0, 0 }, // 50% fade out left
        { 1,   0, (sLedsStep[]){ {eLED_Circle3, 0, 255, 0, 22, 220, eLedEase_OutExpo}}, 0, 0 }, // 100% fade out left
};

#define LEDFLOW_RING_SUCCESS_INTERSTITIAL_SEQUENCE_LEN (1)
sLedsSequence sequenceMakeDrinkSuccessInterstitial[LEDFLOW_RING_SUCCESS_INTERSTITIAL_SEQUENCE_LEN] = {
        { 1,   0, (sLedsStep[]){ {eLED_Circle3 | eLED_Circle2, 0, 0, 0, 36, 360, eLEdEase_constant}}, 0, 0 },
};

#define LEDFLOW_RING_SUCCESS_INNER_LOOP_STEPS (4)
const sLedsStep stepsRingSuccessInnerLoop[LEDFLOW_RING_SUCCESS_INNER_LOOP_STEPS] = {
        {ALL_RING_LEDS_MASK,   0, 255,   0,  6, 64, eLedEase_InOutQuad},
        {ALL_RING_LEDS_MASK,  64,   0,   0,  6, 64, eLEdEase_constant},
        {ALL_RING_LEDS_MASK, 128,   0, 255,  6, 64, eLedEase_OutExpo},
        {ALL_RING_LEDS_MASK, 192, 255, 255, 12, 120, eLEdEase_constant}
};
#define LEDFLOW_RING_SUCCESS_INNER_LOOP_REPEAT (3)

#define LEDFLOW_RING_SUCCESS_SEQUENCE_LEN (7)
const sLedsSequence sequenceRingSuccess[LEDFLOW_RING_SUCCESS_SEQUENCE_LEN] = {
        { 1, 360, (sLedsStep[]){ {eLED_Circle1 | eLED_Circle5, 0, 0, 255, 12, 120, eLedEase_InOutQuad}}, 0, 0 },
        { 1, 420, (sLedsStep[]){ {eLED_Circle2 | eLED_Circle4 | eLED_Circle8 | eLED_Circle6, 0, 0, 255, 12, 120, eLedEase_InOutQuad}}, 0, 0 },
        { 1, 480, (sLedsStep[]){ {eLED_Circle3 | eLED_Circle7 , 0, 0, 255, 12, 120, eLedEase_InOutQuad}}, 0, 0 },
        { LEDFLOW_RING_SUCCESS_INNER_LOOP_STEPS, 840, (sLedsStep *)stepsRingSuccessInnerLoop, LEDFLOW_RING_SUCCESS_INNER_LOOP_REPEAT, 0 },
        { 1, 2296, (sLedsStep[]){ {eLED_Circle1 | eLED_Circle5, 0, 255, 0, 12, 120, eLedEase_InOutQuad}}, 0, 0 },
        { 1, 2356, (sLedsStep[]){ {eLED_Circle2 | eLED_Circle4 | eLED_Circle8 | eLED_Circle6, 0, 255, 0, 12, 120, eLedEase_InOutQuad}}, 0, 0 },
        { 1, 2416, (sLedsStep[]){ {eLED_Circle3 | eLED_Circle7 , 0, 255, 0, 12, 120, eLedEase_InOutQuad}}, 0, 0 },
};


//--------------------- Ring Loader Steps (Rinsing/Filtering) -------------------
#define LEDFLOW_RING_LOADER_LOOP_STEPS (4)
const sLedsStep stepsRingLoaderLoop[LEDFLOW_RING_LOADER_LOOP_STEPS] = {
    {eLED_Circle2 | eLED_Circle3 | eLED_Circle4 | eLED_Circle6 | eLED_Circle7 | eLED_Circle8,   0, 255,   0, 32, 320, eLedEase_InOutQuad},
    {eLED_Circle1 | eLED_Circle5,                                                               0,   0, 255, 32, 320, eLedEase_InOutQuad},
    {eLED_Circle2 | eLED_Circle3 | eLED_Circle4 | eLED_Circle6 | eLED_Circle7 | eLED_Circle8, 320,   0, 255, 32, 320, eLedEase_InOutQuad},
    {eLED_Circle1 | eLED_Circle5,                                                             320, 255,   0, 32, 320, eLedEase_InOutQuad},
};

#define LEDFLOW_RING_LOADER_START_SEQUENCE_LEN (2)
const sLedsSequence sequenceRingLoaderStart[LEDFLOW_RING_LOADER_START_SEQUENCE_LEN] = {
        { 1,   0, (sLedsStep[]){ {eLED_Circle2 | eLED_Circle3 | eLED_Circle4 | eLED_Circle6 | eLED_Circle7 | eLED_Circle8,   0,   0, 255, 32, 320, eLedEase_InOutQuad}}, 0, 0 },
        { LEDFLOW_RING_LOADER_LOOP_STEPS, 320, (sLedsStep *)stepsRingLoaderLoop, ENDLESS_LOOP, 0 },
};

#define LEDFLOW_RING_LOADER_END_SEQUENCE_LEN (1)
const sLedsSequence sequenceRingLoaderEnd[LEDFLOW_RING_LOADER_END_SEQUENCE_LEN] = {
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
#define CO2_LEVEL_OFF_ON_CO2_EXPIRED_IDX (2)
#define LEDFLOW_DISPLAY_CO2_LEVEL_STEPS (4)
#define LEDFLOW_DISPLAY_CO2_LEVEL_STEPS_NO_CO2_EXPIRED (2)  // used when CO2 is not expired - only ON and OFF steps
// note that all leds masks (except last one) here are only placeholders - they will be set at run time
sLedsSequence sequenceCO2Level[LEDFLOW_DISPLAY_CO2_LEVEL_STEPS] = {
        // ON LEDS
        { 1, 0, (sLedsStep[]){ {eLED_LevelNoneWhite,   0,   0, 255, 10, 100, eLedEase_OutExpo}}, 0, 0 },
        // OFF LEDS
        { 1, 0, (sLedsStep[]){ {eLED_LevelNoneWhite,   0, 255,   0, 10, 100, eLedEase_OutExpo}}, 0, 0 },
        // OFF the leds we turned ON in in first step after a delay of 500 ms
        { 1, 0, (sLedsStep[]){ {eLED_LevelNoneWhite,   500, 255,   0, 10, 100, eLedEase_OutExpo}}, 0, 0 },
        // ON the orange leds after a delay the off step is finished
        { 1, 0, (sLedsStep[]){ {ALL_CO2_ORANGE_LEDS_MASK,   600,   0, 255, 10, 100, eLedEase_OutExpo}}, 0, 0 },
};



#define LEDFLOW_FILTER_NORMAL_STEPS (3)
const sLedsSequence sequenceFilterNormal[LEDFLOW_FILTER_NORMAL_STEPS] = {
        // White Filter ON
        { 1,    0, (sLedsStep[]){ {eLED_FilterWhite,   0,   0, 255,  10, 100, eLedEase_OutExpo}}, 0, 0 },
        // White Filter stays ON
        { 1,  100, (sLedsStep[]){ {eLED_FilterWhite,   0, 255, 255, 100, 1000, eLEdEase_constant}}, 0, 0 },
        // White Filter fades to OFF slowly
        { 1, 1100, (sLedsStep[]){ {eLED_FilterWhite,   0, 255,  0, 110, 1100, eLedEase_OutExpo}}, 0, 0 },
};

#define LEDFLOW_FILTER_WARNING_LOOP_STEPS (1)
const sLedsSequence sequenceFilterWarningLoop[LEDFLOW_FILTER_WARNING_LOOP_STEPS] = {
        { LEDFLOW_QUICK_BLINKING_LOOP_STEPS, 0, (sLedsStep *)stepsQuickBlinking, 3, 0 },
};

#define LEDFLOW_FILTER_EXPITED_STEPS (1)
const sLedsSequence sequenceFilterExpired[LEDFLOW_FILTER_EXPITED_STEPS] = {
        { 1, 0, (sLedsStep[]){ {eLED_FilterOrange,   0,   0, 255, 10, 100, eLedEase_OutExpo}}, 0, 0  },
};

#define LEDFLOW_DEVICE_ERROR_STEPS (1)
const sLedsSequence sequenceDeviceError[LEDFLOW_DEVICE_ERROR_STEPS] = {
        { LEDFLOW_ERROR_BLINKING_LOOP_STEPS, 0, (sLedsStep *)stepsErrorBlinckingLoop, ENDLESS_LOOP, 0 },
};


// Modifiable normal filter clear animation to be used when clearing warning
// this is for normal operation not for OOB
#define LEDFLOW_SIMPLE_CLEAR_STEPS (1)
sLedsSequence sequenceSimpleClear[LEDFLOW_SIMPLE_CLEAR_STEPS] = {
        // White Filter ON
        { 1,    0, (sLedsStep[]){ {0, 0, 255,  0,  10, 100, eLedEase_OutExpo}}, 0, 0 } // mask is 0, will be set at run time
};


// ////////////////////////////////////////////////////////  Main Animations  ////////////////////////////////////////////////////////

// These flows are played one after another - not in parallel

#define LEDS_FLOW_STARTUP_LEN (6)
#define STARTUP_FLOW_WARN_OFF_IDX (0)
#define STARTUP_FLOW_STATUS_SEQ_IDX (5)
sLedsFlowDef ledsFlowStartup[LEDS_FLOW_STARTUP_LEN] = {
    { (sLedsSequence[]){{1, 0, (sLedsStep[]){ {0, 0, 255,  0,  10, 100, eLedEase_OutExpo}}, 0, 0 }}, 1}, // first optional step of turning off warning leds if needed
    { (sLedsSequence[]){{LEDFLOW_STARTUP_CIRCLE_STEPS, 0, (sLedsStep *)stepsStartupCircle, 0, 0 }}, 1},
    { (sLedsSequence[]){{LEDFLOW_STARTUP_CARB_LEVEL_STEPS, 0, stepsStartupCarbLevel, 0, 0}}, 1},
    { (sLedsSequence[]){{LEDFLOW_STARTUP_FILTER_STEPS, 0, stepsStartupFilter, 0, 0 }}, 1 },
    { (sLedsSequence[]){{LEDFLOW_INTERSTITIAL_STEPS, 400, stepsInterstitial, 0, 0 }}, 1 }, // Delay of 400 keep previous state for 400ms
    { (sLedsSequence[]){{LEDFLOW_SHOWSTATUS_NORMAL_STEPS, 400, stepsShowStatusNormal, 0, 0 }}, 1 } // All ON by the mask: This step is used only on normal startup, skipped on OOTB
};
#define LEDS_FLOW_MAKE_A_DRINK_PROGRESS_LEN (1)
sLedsFlowDef ledsFlowMakeADrinkProgrees[LEDS_FLOW_MAKE_A_DRINK_PROGRESS_LEN] = {
        {sequenceRingProgress, LEDFLOW_RING_PROGRESS_SEQUENCE_LEN},
};

#define LEDS_FLOW_MAKE_A_DRINK_SUCCESS_LEN (3)
sLedsFlowDef ledsFlowMakeADrinkSuccess[LEDS_FLOW_MAKE_A_DRINK_SUCCESS_LEN] = {
        {sequenceMakeDrinkSuccessFinishRing, LEDFLOW_RING_SUCCESS_FINISH_RING_SEQUENCE_LEN},
        {sequenceMakeDrinkSuccessInterstitial, LEDFLOW_RING_SUCCESS_INTERSTITIAL_SEQUENCE_LEN},
        {(sLedsSequence *)sequenceRingSuccess, LEDFLOW_RING_SUCCESS_SEQUENCE_LEN},
};

#define LEDS_FLOW_START_LOADER_LEN (1)
sLedsFlowDef ledsFlowStartLoader[LEDS_FLOW_START_LOADER_LEN] = {
        {(sLedsSequence *)sequenceRingLoaderStart, LEDFLOW_RING_LOADER_START_SEQUENCE_LEN}
};

#define LEDS_FLOW_END_LOADER_LEN (1)
sLedsFlowDef ledsFlowEndLoader[LEDS_FLOW_END_LOADER_LEN] = {
        {(sLedsSequence *)sequenceRingLoaderEnd, LEDFLOW_RING_LOADER_END_SEQUENCE_LEN}
};

#define LEDS_FLOW_DISPLAY_STATUS_LEN (1)
sLedsFlowDef ledsFlowDisplayStatus[LEDS_FLOW_DISPLAY_STATUS_LEN] = {
        {sequenceDisplayStatus, LEDFLOW_DISPLAY_STATUS_STEPS}
};

#define LEDS_FLOW_CO2_ELVEL_LEN (1)
sLedsFlowDef ledsFlowCO2LevelStatus[LEDS_FLOW_CO2_ELVEL_LEN] = {
        {sequenceCO2Level, LEDFLOW_DISPLAY_CO2_LEVEL_STEPS},
};


#define LEDS_FLOW_SHOW_FILTER_NORMAL_LEN (1)
sLedsFlowDef ledsFlowShowFilterNormal[LEDS_FLOW_SHOW_FILTER_NORMAL_LEN] = {
        {(sLedsSequence *)sequenceFilterNormal, LEDFLOW_FILTER_NORMAL_STEPS}
};

#define LEDS_FLOW_SHOW_FILTER_WARNING_LEN (1)
sLedsFlowDef ledsFlowShowFilterWarning[LEDS_FLOW_SHOW_FILTER_WARNING_LEN] = {
        {(sLedsSequence *)sequenceFilterWarningLoop, LEDFLOW_FILTER_WARNING_LOOP_STEPS}
};

#define LEDS_FLOW_SHOW_FILTER_EXPIRED_LEN (1)
sLedsFlowDef ledsFlowShowFilterExpired[LEDS_FLOW_SHOW_FILTER_EXPIRED_LEN] = {
        {(sLedsSequence *)sequenceFilterExpired, LEDFLOW_FILTER_EXPITED_STEPS}
};

#define LEDS_FLOW_DEVICE_ERROR_LEN (1)
sLedsFlowDef ledsFlowDeviceErrorStatus[LEDS_FLOW_DEVICE_ERROR_LEN] = {
        {(sLedsSequence *)sequenceDeviceError, LEDFLOW_DEVICE_ERROR_STEPS}
};

#define LEDS_FLOW_SIMPLE_CLEAR_LEN (1)
sLedsFlowDef ledsFlowSimpleClear[LEDS_FLOW_SIMPLE_CLEAR_LEN] = {
        {(sLedsSequence *)sequenceSimpleClear, LEDFLOW_SIMPLE_CLEAR_STEPS}
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



#ifdef DEBUG_STATE_MACHINE
extern uint8_t gRawMsgForEcho[MAX_RX_BUFFER_LEN];
#endif

void ZeroGlobalAnimationParams(bool zeroCurrent, bool zeroPendingToo);
void SetCurrentFlowLoopEntryMSValues(sLedsSequence *seq, uint8_t len);
bool IsPendingAnimation(void);
uint32_t OOTBGetCarbLevelLedStatusMask(void);
uint32_t OOTBGetFilterStatusMask(void);
uint32_t GetCarbLevelLedStatusMask(void);
uint32_t GetFilterStatusMask(void);
bool GetCO2ChangedOnOffMasks(uint32_t *onMask, uint32_t *offMask);

eAnimations gLastAnimation = eAnimation_none; // TODO DEBUG Remove

void StartAnimation(eAnimations animation, bool forceStopPrevious)
{
#ifdef DEBUG_STATE_MACHINE
    // DEBUG REMOVE
    uint8_t msg_len = (uint8_t)BuildReply((char*)gRawMsgForEcho, eUARTCommand_dbug, (uint32_t[]){9999, animation, (forceStopPrevious?0:1)}, 3,false);
    COMM_UART_QueueTxMessage(gRawMsgForEcho, msg_len);
    // DEBUG REMOVE
#endif
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
        RBMEM_ReadElement(eRBMEM_isFirstTimeSetupRequired, &val);
        requestedFlow = ledsFlowStartup;
        requestedFlowTotalSteps = LEDS_FLOW_STARTUP_LEN;
        // by default don't include the warning leds off step
        ledsFlowStartup[STARTUP_FLOW_WARN_OFF_IDX].seq[0].subSeq[0].ledIdMask = 0;
        ledsFlowStartup[STARTUP_FLOW_WARN_OFF_IDX].seq[0].subSeq[0].totalSteps10ms = 1;
        ledsFlowStartup[STARTUP_FLOW_WARN_OFF_IDX].seq[0].subSeq[0].totalMs = 10;

        // check if normal startup
        if (val == 0) {
            // if CO2 counter expired and the leds are currently on then need to turn on the orange leds
            if (RBMEM_IsCO2CounterExpired() && (gLeds[eLEDnum_LevellowOrange] != 0)) // Check only the CO2 low orange led (they are all on together)
            {
                ledsFlowStartup[STARTUP_FLOW_WARN_OFF_IDX].seq[0].subSeq[0].ledIdMask = ALL_ORANGE_CO2_AND_FILTER_MASK;
                ledsFlowStartup[STARTUP_FLOW_WARN_OFF_IDX].seq[0].subSeq[0].totalSteps10ms = 10;
                ledsFlowStartup[STARTUP_FLOW_WARN_OFF_IDX].seq[0].subSeq[0].totalMs = 100;
            }
            // update the status display
            ledsFlowStartup[STARTUP_FLOW_STATUS_SEQ_IDX].seq[0].subSeq[0].ledIdMask = GetCarbLevelLedStatusMask() | GetFilterStatusMask() | ALL_RING_LEDS_MASK;
        } else {
            // if in OOTB and CO2 reset is required then orange leds are on - need to turn them off first
            RBMEM_ReadElement(eRBMEM_isCO2OOTBResetRequired, &val);
            if (val != 0) {
                ledsFlowStartup[STARTUP_FLOW_WARN_OFF_IDX].seq[0].subSeq[0].ledIdMask = ALL_ORANGE_CO2_AND_FILTER_MASK;
                ledsFlowStartup[STARTUP_FLOW_WARN_OFF_IDX].seq[0].subSeq[0].totalSteps10ms = 10;
                ledsFlowStartup[STARTUP_FLOW_WARN_OFF_IDX].seq[0].subSeq[0].totalMs = 100;
            }
            // ignore the last step of status (shown on other flows)
            requestedFlowTotalSteps--;
        }

        break;

    case eAnimation_MakeADrinkProgress:
        requestedFlow = ledsFlowMakeADrinkProgrees;
        requestedFlowTotalSteps = LEDS_FLOW_MAKE_A_DRINK_PROGRESS_LEN;
        // Make sure that the CO2 warning steps are cleared (no effect) unless need to set them later
        ledsFlowMakeADrinkProgrees[0].seq[IN_RING_CO2_WARNING_OFF_STEP_INDEX].subSeq[0].ledIdMask = 0;
        ledsFlowMakeADrinkProgrees[0].seq[IN_RING_CO2_WARNING_ON_STEP_INDEX].subSeq[0].ledIdMask = 0;
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
    {
        bool co2Expired = GetCO2ChangedOnOffMasks(
                &ledsFlowCO2LevelStatus[0].seq[CO2_LEVEL_ON_IDX].subSeq[0].ledIdMask,
                &ledsFlowCO2LevelStatus[0].seq[CO2_LEVEL_OFF_IDX].subSeq[0].ledIdMask);
        if (co2Expired) {
            // use all 4 steps (OFF mask, ON mask, OFF mask, Orange ON)
            ledsFlowCO2LevelStatus[0].length = LEDFLOW_DISPLAY_CO2_LEVEL_STEPS;
            // Update the CO2 epired level off step to the same mask as the first ON step
            ledsFlowCO2LevelStatus[0].seq[CO2_LEVEL_OFF_ON_CO2_EXPIRED_IDX].subSeq[0].ledIdMask =
                    ledsFlowCO2LevelStatus[0].seq[CO2_LEVEL_ON_IDX].subSeq[0].ledIdMask;
        } else {
            // use only ON and OFF steps
            ledsFlowCO2LevelStatus[0].length = LEDFLOW_DISPLAY_CO2_LEVEL_STEPS_NO_CO2_EXPIRED;
        }
        requestedFlow = ledsFlowCO2LevelStatus;
        requestedFlowTotalSteps = LEDS_FLOW_CO2_ELVEL_LEN;

        break;
    }
    case eAnimation_DeviceError:
        requestedFlow = ledsFlowDeviceErrorStatus;
        requestedFlowTotalSteps = LEDS_FLOW_DEVICE_ERROR_LEN;
        break;

    case eAnimation_CheckFilterStatus:
        switch (GetFilterStatus())
        {
        case eFilterStatus_Expired:
            requestedFlow = ledsFlowShowFilterExpired;
            requestedFlowTotalSteps = LEDS_FLOW_SHOW_FILTER_EXPIRED_LEN;
            break;
        case eFilterStatus_Warning:
            requestedFlow = ledsFlowShowFilterWarning;
            requestedFlowTotalSteps = LEDS_FLOW_SHOW_FILTER_WARNING_LEN;
            break;
        default:
            requestedFlow = ledsFlowShowFilterNormal;
            requestedFlowTotalSteps = LEDS_FLOW_SHOW_FILTER_NORMAL_LEN;
            break;
        }
        break;
        //eAnimation_ClearFilterWarning, // special animation to clear only the filter led from the orange value

    case eAnimation_ClearFilterWarning:
        // turn it off only if it is currently on/fading
        if (gLeds[eLEDnum_FilterOrange] != 0) {
            // only if the orange filter led is currently on
            ledsFlowSimpleClear[0].seq[0].subSeq[0].ledIdMask = eLED_FilterOrange;
            // start from current value - this covers the case of starting the rinsing while in the warning period
            // and the orange led was flashing on/off and we just stated when it was fading somewhere in between
            ledsFlowSimpleClear[0].seq[0].subSeq[0].startPercent = gLeds[eLEDnum_FilterOrange];
            requestedFlow = ledsFlowSimpleClear;
            requestedFlowTotalSteps = LEDS_FLOW_SIMPLE_CLEAR_LEN;
        }
        else {
            return; // nothing to do
        }
        break;
    case eAnimation_ClearCO2Warning:
        // turn it off only if it is currently on
        if (gLeds[eLEDnum_LevellowOrange] != 0) { // Check only the low orange led (they are all on together)
            ledsFlowSimpleClear[0].seq[0].subSeq[0].ledIdMask = ALL_CO2_ORANGE_LEDS_MASK;
            requestedFlow = ledsFlowSimpleClear;
            requestedFlowTotalSteps = LEDS_FLOW_SIMPLE_CLEAR_LEN;
        }
        else {
            return; // nothing to do
        }
        break;
    case eAnimation_CO2WarningWhileMakeingADrink:
        // This is a special case that suppose to happen while making a drink and the CO2 counter exceeded maximum
        // so need at the point to update ht e ring sequence to play the warning using the last two steps of the ring progress sequence
        // that by default have no effect since their masks are 0
        ledsFlowMakeADrinkProgrees[0].seq[IN_RING_CO2_WARNING_OFF_STEP_INDEX].subSeq[0].ledIdMask = GetCarbLevelLedStatusMask();// turn off the white CO2 leds by their current level
        ledsFlowMakeADrinkProgrees[0].seq[IN_RING_CO2_WARNING_ON_STEP_INDEX].subSeq[0].ledIdMask = ALL_CO2_ORANGE_LEDS_MASK;// turn on all the orange CO2 leds
        // Set the timings so it will will be played immediately in the current loop one after the other
        ledsFlowMakeADrinkProgrees[0].seq[IN_RING_CO2_WARNING_OFF_STEP_INDEX].subSeq[0].delayMS = HAL_GetTick() - gAnimationStartingMS;
        ledsFlowMakeADrinkProgrees[0].seq[IN_RING_CO2_WARNING_ON_STEP_INDEX].subSeq[0].delayMS =
                ledsFlowMakeADrinkProgrees[0].seq[IN_RING_CO2_WARNING_OFF_STEP_INDEX].subSeq[0].delayMS +
                ledsFlowMakeADrinkProgrees[0].seq[IN_RING_CO2_WARNING_OFF_STEP_INDEX].subSeq[0].totalMs;
        return; // do not continue to start a new animation or queue it

        // TODO - this is here to cover animations that are not yet implemented
    case eAnimation_StartUpCO2: // startup animation for CO2 only leds (part of the "StartUp (Splash)" animation)
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
            if (IsPendingAnimation()) { // if pending animation after the stop - play it
                pCurrentFlow = pPendingFlow;
                gCurrentFlowTotalSteps = gPendingFlowTotalSteps;
                gCurrentAnimation = gPendingAnimation;
                // this must be done after copying the pending to current
                ZeroGlobalAnimationParams(false, true);
                SetCurrentFlowLoopEntryMSValues(pCurrentFlow[0].seq, pCurrentFlow[0].length);
            } else {
                gCurrentAnimation = eAnimation_none;
            }
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
                                // need to move from the endless make to success
                                // i.e. success is pending and last updated led is 4 to around 100%
                                if ((gPendingAnimation == eAnimation_MakeADrinkSuccess) && (gLeds[2] >= 250)) {
//                                    // i.e. success is pending and last updated led is 4 to around 100%
//                                    if ((gPendingAnimation == eAnimation_MakeADrinkSuccess) && (gLeds[3] >= 250)) {
                                    currentFlowIsDone = true;
                                    break;
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
    RBMEM_ReadElement(eRBMEM_isCO2OOTBResetRequired, &val);
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
    RBMEM_ReadElement(eRBMEM_isFilterOOTBResetRequired, &val);
    return ((val == 0) ? eLED_FilterWhite : eLED_FilterOrange) | ALL_RING_LEDS_MASK;


}
uint32_t GetCarbLevelLedStatusMask(void)
{
    uint32_t val = 0;
    RBMEM_ReadElement(eRBMEM_isCO2OOTBResetRequired, &val);
    if ((val == 1) || RBMEM_IsCO2CounterExpired()) {
        // OOB led was not cleared yet or CO2 is expired
        // Set all CO2 level leds to orange
        val = ALL_CO2_ORANGE_LEDS_MASK;
    } else {
        // Set CO2 level leds according to current level
        val |= eLED_LevelNoneWhite;
        if (gCarbonationLevel >= eLevel_Low) {
            val  = 0; // clear the "none"
            val |= eLED_LevelLowWhite;
        }
        if (gCarbonationLevel >= eLevel_medium) {
            val |= eLED_LevelMedWhite;
        }
        if (gCarbonationLevel >= eLevel_high) {
            val |= eLED_LevelHighWhite;
        }
    }
    return val;
}

bool GetCO2ChangedOnOffMasks(uint32_t *onMask, uint32_t *offMask)
{
    bool isCO2Expired = false;
    // if already showing a warning of CO2 expired, need also to turn off the orange leds
    if (gLeds[eLEDnum_LevellowOrange] != 0) { // Check only the low orange led (they are all on together)
        *offMask = ALL_CO2_ORANGE_LEDS_MASK;
        isCO2Expired = true;
    } else {
        *offMask = 0;
    }
    switch (gCarbonationLevel)
    {
    case eLevel_off:
        *onMask = eLED_LevelNoneWhite;
        if (!isCO2Expired) {
            *offMask |= eLED_LevelLowWhite | eLED_LevelMedWhite | eLED_LevelHighWhite;
        }
        break;
    case eLevel_Low:
        *onMask = eLED_LevelLowWhite;
        if (!isCO2Expired) {
            *offMask |= eLED_LevelNoneWhite;
        }
        break;
    case eLevel_medium:
        *onMask = eLED_LevelMedWhite | (isCO2Expired ? eLED_LevelLowWhite : 0);
        break;
    case eLevel_high:
        *onMask = eLED_LevelHighWhite  | (isCO2Expired ? (eLED_LevelLowWhite | eLED_LevelMedWhite): 0);;
        break;
    default:
        *onMask = 0;
        break;
    }
    return isCO2Expired;
}


uint32_t GetFilterStatusMask(void)
{
    return ((IsInFilterReplacementWarningPeriod() || IsFilterExpired()) ? eLED_FilterOrange : 0); // nothing shown if filter is OK
}
