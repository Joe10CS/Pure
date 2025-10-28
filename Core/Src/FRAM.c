/*
 * FRAM.c
 */
#include "FRAM.h"
#include <stddef.h>  // for offsetof()

extern I2C_HandleTypeDef hi2c1;

static const uint16_t gFRAMElementOffset[eFRAM_MAX] =
{
    [eFRAM_magicNumber]           = offsetof(sFRAMStorageData, magicNumber),
    [eFRAM_isFirstTimeSetupRequired] = offsetof(sFRAMStorageData, isFirstTimeSetupRequired),
    [eFRAM_isCO2OOTBResetRequired] = offsetof(sFRAMStorageData, isCO2OOTBResetRequired),
    [eFRAM_isFilterOOTBResetRequired] = offsetof(sFRAMStorageData, isFilterOOTBResetRequired),
    [eFRAM_lastCarbonationLevel]  = offsetof(sFRAMStorageData, lastCarbonationLevel)
};

void FRAM_Init(void)
{
    sFRAMStorageData data;
    FRAM_Read(0, (uint8_t*)&data, sizeof(data));

    if (data.magicNumber != FRAM_MAGIC_NUMBER)
    {
        // Fill with defaults
        data.magicNumber = FRAM_MAGIC_NUMBER;
        data.isFirstTimeSetupRequired = DEFAULT_isFirstTimeSetupRequired;
        data.isCO2OOTBResetRequired = DEFAULT_isCO2OOTBResetRequired;
        data.isFilterOOTBResetRequired = DEFAULT_isFilterOOTBResetRequired;
        data.lastCarbonationLevel = DEFAULT_lastCarbonationLevel;

        // Write back defaults
        FRAM_Write(0, (uint8_t*)&data, sizeof(data));
    }
}

HAL_StatusTypeDef FRAM_WriteElement(eFRAM_Element elem, uint32_t value)
{
    uint16_t addr = gFRAMElementOffset[elem];
    return FRAM_Write(addr, (uint8_t*)&value, sizeof(value));
}

HAL_StatusTypeDef FRAM_ReadElement(eFRAM_Element elem, uint32_t *value)
{
    uint16_t addr = gFRAMElementOffset[elem];
    return FRAM_Read(addr, (uint8_t*)value, sizeof(*value));
}




bool FRAM_IsValidRange(uint16_t addr, uint16_t size)
{
    return (addr + size - 1U) <= FRAM_LAST_ADDR;
}

HAL_StatusTypeDef FRAM_Write(uint16_t addr, const uint8_t *buf, uint16_t size)
{
    if (!FRAM_IsValidRange(addr, size))
        return HAL_ERROR;  // Address out of range

    return HAL_I2C_Mem_Write(&hi2c1,
                                 FRAM_I2C_ADDR,
                                 addr,
                                 I2C_MEMADD_SIZE_8BIT,
                                 (uint8_t*)buf,
                                 size,
                                 HAL_MAX_DELAY);
}

HAL_StatusTypeDef FRAM_Read(uint16_t addr, uint8_t *buf, uint16_t size)
{
    if (!FRAM_IsValidRange(addr, size))
        return HAL_ERROR;  // Address out of range

    return HAL_I2C_Mem_Read(&hi2c1,
                            FRAM_I2C_ADDR,
                            addr,
                            I2C_MEMADD_SIZE_8BIT,
                            buf,
                            size,
                            HAL_MAX_DELAY);
}

