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

#define RTC_BKP_RTC_STARTUP_MAGIC_NUMBER (0xC0DE32F2)
#define RBM_DATA_MAGIC_NUMBER    (0xFEEDCAFE)
typedef struct {
    uint32_t magicNumber;
    uint32_t isFirstTimeSetupRequired;
    uint32_t isCO2OOTBResetRequired;
    uint32_t isFilterOOTBResetRequired;
    uint32_t lastCarbonationLevel;
}sRBMEMStorageData;

#define DEFAULT_isFirstTimeSetupRequired (1)
#define DEFAULT_isCO2OOTBResetRequired (1)
#define DEFAULT_isFilterOOTBResetRequired (1)
#define DEFAULT_lastCarbonationLevel   (3)

/* Backup register assignments */
#define RBMEM_RTC_DR_MAGIC_START  RTC_BKP_DR0   // Magic number of RTC Timer Need to restart (no magic = need to start)
#define RBMEM_RTC_DR_MAGIC_MEM    RTC_BKP_DR1   // Magic number of data stored (no magic - need to set defaults
#define RBMEM_RTC_DR_FLAG_BITS    RTC_BKP_DR2   // all flags + carbonation level
#define RBMEM_RTC_DR_TOTAL_CO2    RTC_BKP_DR3   // Total milliseconds of CO2 used since last reset
#define RBMEM_RTC_DR_MAX_CO2      RTC_BKP_DR4   // Maximum milliseconds of CO2

#define RBMEM_FIRST_TIME_SETUP_MASK       (0x00000001)
#define RBMEM_CO2_OOTB_RESET_MASK         (0x00000002)
#define RBMEM_FILTER_OOTB_RESET_MASK      (0x00000004)
#define RBMEM_LAST_CARBONATION_LEVEL_MASK (0x00000030)
#define RBMEM_LAST_CARBONATION_LEVEL_SHIFT (4U)   // bits [5:4]

typedef enum
{
    eRBMEM_RTC_Time_Start_magicNumber = 0,
    eRBMEM_RTC_Memory_magicNumber,
    eRBMEM_isFirstTimeSetupRequired, // Is first time setup required (OOTB)
    eRBMEM_isCO2OOTBResetRequired, // Is CO2 status out-of-the-box (OOTB) reset required
    eRBMEM_isFilterOOTBResetRequired, // Is filter status out-of-the-box (OOTB) reset required
    eRBMEM_lastCarbonationLevel,
    eRBMEM_total_CO2_msecs_used, // Total milliseconds of CO2 used since last reset
    eRBMEM_total_CO2_msecs_max, // Maximum of Total milliseconds of CO2
    eRBMEM_MAX
} eRBMEM_Element;

uint32_t RBMEM_Data_Init(void);
HAL_StatusTypeDef RBMEM_WriteElement(eRBMEM_Element elem, uint32_t value);
HAL_StatusTypeDef RBMEM_ReadElement(eRBMEM_Element elem, uint32_t *value);
HAL_StatusTypeDef RBMEM_ResetDataToDefaults(void);
bool RBMEM_IsRTCMagicNunberOK(void);
void RBMEM_WriteRTCMagicNunber(void);

HAL_StatusTypeDef RBMEM_AddMSecsToCO2Counter(uint32_t value);
bool RBMEM_IsCO2CounterExpired();

#endif /* INC_RTCBACKUPMEMORY_H_ */
