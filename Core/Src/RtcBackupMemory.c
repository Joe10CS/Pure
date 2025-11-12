/*
 * RtcBackupMemory.c
 *
 */

#include "RtcBackupMemory.h"

extern RTC_HandleTypeDef hrtc;

uint32_t RBMEM_Data_Init(void)
{
    uint32_t val32 = 0;
    RBMEM_ReadElement(eRBMEM_RTC_Memory_magicNumber, &val32);

    if (val32 != RBM_DATA_MAGIC_NUMBER)
    {
        RBMEM_ResetDataToDefaults();
    }
    RBMEM_ReadElement(eRBMEM_lastCarbonationLevel, &val32);
    return val32;
}


HAL_StatusTypeDef RBMEM_WriteElement(eRBMEM_Element elem, uint32_t value)
{
    uint32_t regVal = HAL_RTCEx_BKUPRead(&hrtc, RBMEM_RTC_DR_FLAG_BITS);

    switch (elem)
    {
    case eRBMEM_isFirstTimeSetupRequired:
        if (value)
            regVal |= RBMEM_FIRST_TIME_SETUP_MASK;
        else
            regVal &= ~RBMEM_FIRST_TIME_SETUP_MASK;
        break;

    case eRBMEM_isCO2OOTBResetRequired:
        if (value)
            regVal |= RBMEM_CO2_OOTB_RESET_MASK;
        else
            regVal &= ~RBMEM_CO2_OOTB_RESET_MASK;
        break;

    case eRBMEM_isFilterOOTBResetRequired:
        if (value)
            regVal |= RBMEM_FILTER_OOTB_RESET_MASK;
        else
            regVal &= ~RBMEM_FILTER_OOTB_RESET_MASK;
        break;

    case eRBMEM_lastCarbonationLevel:
        regVal &= ~RBMEM_LAST_CARBONATION_LEVEL_MASK;
        regVal |= ((value & 0x03) << RBMEM_LAST_CARBONATION_LEVEL_SHIFT);
        break;

    case eRBMEM_RTC_Time_Start_magicNumber:
        HAL_RTCEx_BKUPWrite(&hrtc, RBMEM_RTC_DR_MAGIC_START, value);
        return HAL_OK;

    case eRBMEM_RTC_Memory_magicNumber:
        HAL_RTCEx_BKUPWrite(&hrtc, RBMEM_RTC_DR_MAGIC_MEM, value);
        return HAL_OK;

    default:
        return HAL_ERROR;
    }

    HAL_RTCEx_BKUPWrite(&hrtc, RBMEM_RTC_DR_FLAG_BITS, regVal);
    return HAL_OK;
}

/* ========== Read function ========== */
HAL_StatusTypeDef RBMEM_ReadElement(eRBMEM_Element elem, uint32_t *value)
{
    if (value == NULL)
        return HAL_ERROR;

    uint32_t regVal = HAL_RTCEx_BKUPRead(&hrtc, RBMEM_RTC_DR_FLAG_BITS);

    switch (elem)
    {
    case eRBMEM_isFirstTimeSetupRequired:
        *value = (regVal & RBMEM_FIRST_TIME_SETUP_MASK) ? 1U : 0U;
        break;

    case eRBMEM_isCO2OOTBResetRequired:
        *value = (regVal & RBMEM_CO2_OOTB_RESET_MASK) ? 1U : 0U;
        break;

    case eRBMEM_isFilterOOTBResetRequired:
        *value = (regVal & RBMEM_FILTER_OOTB_RESET_MASK) ? 1U : 0U;
        break;

    case eRBMEM_lastCarbonationLevel:
        *value = (regVal & RBMEM_LAST_CARBONATION_LEVEL_MASK) >> RBMEM_LAST_CARBONATION_LEVEL_SHIFT;
        break;

    case eRBMEM_RTC_Time_Start_magicNumber:
        *value = HAL_RTCEx_BKUPRead(&hrtc, RBMEM_RTC_DR_MAGIC_START);
        break;

    case eRBMEM_RTC_Memory_magicNumber:
        *value = HAL_RTCEx_BKUPRead(&hrtc, RBMEM_RTC_DR_MAGIC_MEM);
        break;

    default:
        return HAL_ERROR;
    }

    return HAL_OK;
}


HAL_StatusTypeDef RBMEM_ResetDataToDefaults(void)
{
    HAL_StatusTypeDef status = HAL_OK;

    // Write both magic numbers
    status |= RBMEM_WriteElement(eRBMEM_RTC_Memory_magicNumber, RBM_DATA_MAGIC_NUMBER);

    // Set all default flags
    status |= RBMEM_WriteElement(eRBMEM_isFirstTimeSetupRequired, DEFAULT_isFirstTimeSetupRequired);
    status |= RBMEM_WriteElement(eRBMEM_isCO2OOTBResetRequired, DEFAULT_isCO2OOTBResetRequired);
    status |= RBMEM_WriteElement(eRBMEM_isFilterOOTBResetRequired, DEFAULT_isFilterOOTBResetRequired);

    // Set default carbonation level
    status |= RBMEM_WriteElement(eRBMEM_lastCarbonationLevel, DEFAULT_lastCarbonationLevel);

    return status;
}

bool RBMEM_IsRTCMagicNunberOK(void)
{
    return (HAL_RTCEx_BKUPRead(&hrtc, RBMEM_RTC_DR_MAGIC_START) == RTC_BKP_RTC_STARTUP_MAGIC_NUMBER);
}
void RBMEM_WriteRTCMagicNunber(void)
{
    HAL_RTCEx_BKUPWrite(&hrtc, RBMEM_RTC_DR_MAGIC_START, RTC_BKP_RTC_STARTUP_MAGIC_NUMBER);
}
