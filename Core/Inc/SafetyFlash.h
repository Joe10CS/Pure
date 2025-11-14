/*
 * SafetyFlash.h
 *
 *  Created on: Nov 13, 2025
 *      Author: yossi
 */

#ifndef INC_SAFETYFLASH_H_
#define INC_SAFETYFLASH_H_

#include <stdint.h>
#include "main.h"   // for hcrc and HAL types
#include "SMSodaStreamPure.h"  // for SMSodaStreamPure_EventId_EVENT_SAFETYFAIL
#include "EventQueue.h"        // for SMEventQueue_Add

// Adjust these if you later want to change region
#define FLASH_TEST_START_ADDR   (0x08000000u)    // start of flash
// We stop BEFORE the checksum word
extern uint32_t _Check_Sum;                     // from linker script
#define FLASH_TEST_END_ADDR     ((uint32_t)&_Check_Sum)

// 16 words = 64 bytes per cycle
#define FLASH_TEST_BLOCK_WORDS  (16u)

typedef enum {
    eSafetyErr_none = 0,
    eSafetyErr_CRC =  0x55,
}eSafetyErrors;

typedef struct
{
    uint32_t      currentAddr;
    uint32_t      runningCrc;
    uint8_t       active;   // 0 = not running, 1 = running
    eSafetyErrors safetyError;
} SafetyFlashContext_t;

void SafetyFlash_Init(void);
void SafetyFlash_Periodic(void);   // call every 10ms
void SafetyFlash_ForceCheck(void); // optional: run full check in one shot


#endif /* INC_SAFETYFLASH_H_ */
