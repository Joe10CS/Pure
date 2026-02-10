/*
 * RtcBackupMemory.h
 *
 *  Created on: Nov 12, 2025
 *      Author: yossi
 */

#ifndef INC_RTCBACKUPMEMORY_H_
#define INC_RTCBACKUPMEMORY_H_

#include "stdbool.h"
#ifndef _MSC_VER
#include "stm32g0xx_hal.h"
#endif

#define RTC_BKP_RTC_STARTUP_MAGIC_NUMBER (0xBAFACEBA)
#define RBM_DATA_MAGIC_NUMBER    (0x2BADCAFE)
typedef struct {
    uint32_t magicNumber;
    uint32_t lastCarbonationLevel;
}sRBMEMStorageData;

#define DEFAULT_isFirstTimeSetupRequired (1)
#define DEFAULT_Rinsing2ndWaiting        (0)
#define DEFAULT_lastCarbonationLevel     (3)

/* Backup register assignments */
#define RBMEM_RTC_DR_MAGIC_START  RTC_BKP_DR0   // Magic number of RTC Timer Need to restart (no magic = need to start)
#define RBMEM_RTC_DR_MAGIC_MEM    RTC_BKP_DR1   // Magic number of data stored (no magic - need to set defaults
#define RBMEM_RTC_DR_FLAG_BITS    RTC_BKP_DR2   // all flags + carbonation level + filtering counter
#define RBMEM_RTC_DR_TOTAL_CO2    RTC_BKP_DR3   // Total milliseconds of CO2 used since last reset
#define RBMEM_RTC_DR_MAX_CO2      RTC_BKP_DR4   // Maximum milliseconds of CO2

#define RBMEM_LAST_CARBONATION_LEVEL_MASK (0x00000003)
#define RBMEM_LAST_CARBONATION_LEVEL_SHIFT (0U)     // bits [1:0]
#define RBMEM_RINSING_2ND_WAITING_MASK (0x00000004) // bit  [2]
#define RBMEM_FILTERING_COUNTER_MASK (0x0000FF8)  // bits [11:3]
#define RBMEM_FILTERING_COUNTER_SHIFT (3U)

typedef enum
{
    eRBMEM_RTC_Time_Start_magicNumber = 0,
    eRBMEM_RTC_Memory_magicNumber,
    eRBMEM_lastCarbonationLevel,
    eRBMEM_Rinsing2ndWaiting,  // if set, the user is expected to perform the second stage of rinsing
    eRBMEM_FilteringCounter, // number of filtering cycles done
    eRBMEM_total_CO2_msecs_used, // Total milliseconds of CO2 used since last reset
    eRBMEM_total_CO2_msecs_max, // Maximum of Total milliseconds of CO2
    eRBMEM_MAX
} eRBMEM_Element;

uint32_t RBMEM_Data_Init(void);
HAL_StatusTypeDef RBMEM_WriteElement(eRBMEM_Element elem, uint32_t value);
HAL_StatusTypeDef RBMEM_ReadElement(eRBMEM_Element elem, uint32_t *value);
HAL_StatusTypeDef RBMEM_ResetDataToDefaults(void);
bool RBMEM_IsRTCMagicNumberOK(void);
void RBMEM_WriteRTCMagicNunber(void);

HAL_StatusTypeDef RBMEM_AddMSecsToCO2Counter(uint32_t value);
bool RBMEM_IsCO2CounterExpired();
HAL_StatusTypeDef RBMEM_IncreaseFilteringCounter();
bool RBMEM_IsFilteringCounterExpired();
#endif /* INC_RTCBACKUPMEMORY_H_ */
