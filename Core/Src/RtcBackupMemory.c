/*
 * RtcBackupMemory.c
 *
 */

#include "RtcBackupMemory.h"
#include "RTC.h"

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
    case eRBMEM_Rinsing2ndWaiting:
        if (value)
            regVal |= RBMEM_RINSING_2ND_WAITING_MASK;
        else
            regVal &= ~RBMEM_RINSING_2ND_WAITING_MASK;
        break;
    case eRBMEM_lastCarbonationLevel:
        regVal &= ~RBMEM_LAST_CARBONATION_LEVEL_MASK;
        regVal |= ((value & 0x03) << RBMEM_LAST_CARBONATION_LEVEL_SHIFT);
        break;

    case eRBMEM_FilteringCounter:
		regVal &= ~RBMEM_FILTERING_COUNTER_MASK;
		regVal |= ((value << RBMEM_FILTERING_COUNTER_SHIFT) & RBMEM_FILTERING_COUNTER_MASK);
		break;
    case eRBMEM_RTC_Time_Start_magicNumber:
        HAL_RTCEx_BKUPWrite(&hrtc, RBMEM_RTC_DR_MAGIC_START, value);
        return HAL_OK;

    case eRBMEM_RTC_Memory_magicNumber:
        HAL_RTCEx_BKUPWrite(&hrtc, RBMEM_RTC_DR_MAGIC_MEM, value);
        return HAL_OK;

    case eRBMEM_total_CO2_msecs_used:
        HAL_RTCEx_BKUPWrite(&hrtc, RBMEM_RTC_DR_TOTAL_CO2, value);
        return HAL_OK;
    case eRBMEM_total_CO2_msecs_max:
        HAL_RTCEx_BKUPWrite(&hrtc, RBMEM_RTC_DR_MAX_CO2, value);
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
    case eRBMEM_Rinsing2ndWaiting:
        *value = (regVal & RBMEM_RINSING_2ND_WAITING_MASK) ? 1U : 0U;
        break;
    case eRBMEM_lastCarbonationLevel:
        *value = (regVal & RBMEM_LAST_CARBONATION_LEVEL_MASK) >> RBMEM_LAST_CARBONATION_LEVEL_SHIFT;
        break;
    case eRBMEM_FilteringCounter:
    	*value = (regVal & RBMEM_FILTERING_COUNTER_MASK) >> RBMEM_FILTERING_COUNTER_SHIFT;
		break;
    case eRBMEM_RTC_Time_Start_magicNumber:
        *value = HAL_RTCEx_BKUPRead(&hrtc, RBMEM_RTC_DR_MAGIC_START);
        break;

    case eRBMEM_RTC_Memory_magicNumber:
        *value = HAL_RTCEx_BKUPRead(&hrtc, RBMEM_RTC_DR_MAGIC_MEM);
        break;

    case eRBMEM_total_CO2_msecs_used:
        *value = HAL_RTCEx_BKUPRead(&hrtc, RBMEM_RTC_DR_TOTAL_CO2);
        break;
    case eRBMEM_total_CO2_msecs_max:
        *value = HAL_RTCEx_BKUPRead(&hrtc, RBMEM_RTC_DR_MAX_CO2);
        break;
    default:
        return HAL_ERROR;
    }

    return HAL_OK;
}

HAL_StatusTypeDef RBMEM_AddMSecsToCO2Counter(uint32_t value)
{
    if ((value > 0) && (value < CO2_MAX_SINGLE_PULSE_MSECS))
    {
        uint32_t currentVal = HAL_RTCEx_BKUPRead(&hrtc, RBMEM_RTC_DR_TOTAL_CO2);
        currentVal += value;
        HAL_RTCEx_BKUPWrite(&hrtc, RBMEM_RTC_DR_TOTAL_CO2, currentVal);
    }
    return HAL_OK;
}

bool RBMEM_IsCO2CounterExpired()
{
    uint32_t totalCO2Msecs = HAL_RTCEx_BKUPRead(&hrtc, RBMEM_RTC_DR_TOTAL_CO2);
    uint32_t maxCO2Msecs = HAL_RTCEx_BKUPRead(&hrtc, RBMEM_RTC_DR_MAX_CO2);
    return (totalCO2Msecs >= maxCO2Msecs);
}


HAL_StatusTypeDef RBMEM_ResetDataToDefaults(void)
{
    HAL_StatusTypeDef status = HAL_OK;

    // Write both magic numbers
    status |= RBMEM_WriteElement(eRBMEM_RTC_Memory_magicNumber, RBM_DATA_MAGIC_NUMBER);

    status |= RBMEM_WriteElement(eRBMEM_Rinsing2ndWaiting, DEFAULT_Rinsing2ndWaiting);
    // Set default carbonation level
    status |= RBMEM_WriteElement(eRBMEM_lastCarbonationLevel, DEFAULT_lastCarbonationLevel);

    // Set default filtering counter
    status |= RBMEM_WriteElement(eRBMEM_FilteringCounter, 0);

    // Set default total CO2 used
    status |= RBMEM_WriteElement(eRBMEM_total_CO2_msecs_used, 0);
    status |= RBMEM_WriteElement(eRBMEM_total_CO2_msecs_max, CO2_LIFETIME_MSECS); // default max

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

HAL_StatusTypeDef RBMEM_IncreaseFilteringCounter()
{
	uint32_t currentCounter = 0;
	RBMEM_ReadElement(eRBMEM_FilteringCounter, &currentCounter);
	if (currentCounter < MAX_FILTERING_COUNTER)
	{
		currentCounter++;
		return RBMEM_WriteElement(eRBMEM_FilteringCounter, currentCounter);
	}
	return HAL_OK;
}
bool RBMEM_IsFilteringCounterExpired()
{
	uint32_t currentCounter = 0;
	RBMEM_ReadElement(eRBMEM_FilteringCounter, &currentCounter);
	return (currentCounter >= MAX_FILTERING_COUNTER);
}

