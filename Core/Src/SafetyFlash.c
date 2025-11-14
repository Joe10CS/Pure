#include "SafetyFlash.h"

extern CRC_HandleTypeDef hcrc;
SafetyFlashContext_t gSafetyFlashCtx;
extern uint32_t _Check_Sum;

static void SafetyFlash_ErrorHandler(void)
{
    SMEventQueue_Add(SMSodaStreamPure_EventId_EVENT_SAFETYFAIL);
}

void SafetyFlash_Init(void)
{
    gSafetyFlashCtx.currentAddr = FLASH_TEST_START_ADDR;
    gSafetyFlashCtx.runningCrc  = 0xFFFFFFFFu;
    gSafetyFlashCtx.active      = 1u;
    gSafetyFlashCtx.safetyError = eSafetyErr_none;

    __HAL_CRC_DR_RESET(&hcrc);
}

void SafetyFlash_Periodic(void)
{
    if (!gSafetyFlashCtx.active)
        return;

    uint32_t addr = gSafetyFlashCtx.currentAddr;

    // End of region?
    if (addr >= FLASH_TEST_END_ADDR)
    {
        uint32_t storedCrc = _Check_Sum;

        if (gSafetyFlashCtx.runningCrc != storedCrc)
        {
            gSafetyFlashCtx.active = 0u;
            gSafetyFlashCtx.safetyError = eSafetyErr_CRC;
            SafetyFlash_ErrorHandler();
        }

        // Restart for next cycle
        gSafetyFlashCtx.currentAddr = FLASH_TEST_START_ADDR;
        gSafetyFlashCtx.runningCrc  = 0xFFFFFFFFu;
        __HAL_CRC_DR_RESET(&hcrc);
        return;
    }

    // Compute remaining 32-bit words
    uint32_t bytesRemaining = FLASH_TEST_END_ADDR - addr;
    uint32_t totalWords     = bytesRemaining / 4u;
    uint32_t wordsToProcess = FLASH_TEST_BLOCK_WORDS;

    if (wordsToProcess > totalWords)
        wordsToProcess = totalWords;

    // Feed FLASH directly (aligned 32-bit reads)
    uint32_t *pWord = (uint32_t *)addr;

    gSafetyFlashCtx.runningCrc =
        HAL_CRC_Accumulate(&hcrc, pWord, wordsToProcess);

    gSafetyFlashCtx.currentAddr = addr + (wordsToProcess * 4u);
}

// Optional: blocking full check (e.g., only used at startup)
void SafetyFlash_ForceCheck(void)
{
    SafetyFlash_Init();

    while (gSafetyFlashCtx.currentAddr < FLASH_TEST_END_ADDR)
    {
        SafetyFlash_Periodic();
    }
}
