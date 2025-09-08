/*
 * FRAM.c
 */
#include "FRAM.h"


extern I2C_HandleTypeDef hi2c1;

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
                                 I2C_MEMADD_SIZE_16BIT, // 2-byte address
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
                            I2C_MEMADD_SIZE_16BIT,
                            buf,
                            size,
                            HAL_MAX_DELAY);
}

