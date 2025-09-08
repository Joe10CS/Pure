/*
 * FRAM.h
 * Header for the: CY15B004J-
 */

#ifndef INC_FRAM_H_
#define INC_FRAM_H_
#include "main.h"

#define FRAM_I2C_ADDR       (0x50<<1)
#define FRAM_SIZE_BYTES     (512) // 0.5K
#define FRAM_LAST_ADDR      (FRAM_SIZE_BYTES - 1U)

#define FRAM_IS_VALID_ADDR(addr)   ((addr)<FRAM_SIZE)

/*
 * Important note:
 * ===============
 * The I2C bus can be defined in 3 speed modes: normal (100kHz), fast (400kHz) and super fast (1MHz)
 * this FRAM CY15B004J supports all speeds, but on the first Pure board the LED driver LP5009 is also on the same bus
 * and the LP5009 can work at most at fast mode (400kHz) so this is the current I2C speed
 *
 * So the data transfer rate takes long time,
 *   #     | 400 kHz I2C Bus        | 1 MHz I2C Bus
 *   Bytes | Read (ms) | Write (ms) | Read (ms) | Write (ms)
 *  -------|-----------|------------|-----------|------------
 *   128   | 2.98      | 2.95       | 1.19      | 1.18
 *   256   | 5.85      | 5.83       | 2.34      | 2.33
 *   384   | 8.73      | 8.70       | 3.49      | 3.48
 *   512   | 11.61     | 11.59      | 4.64      | 4.63
 *
 * On the Pure VDL - it is better to configure 1 MHz
 * in any case the FRAM need to be defined with DMA -  PROBLEMATIC: Only one direction is allowed (either TX or RX) !!!
 * in reading - need to assume that the DMA Isr is asyc so results of reading stuff from FRAM are coming "later"
 * for configuration params - not an issue, but for UART reading - need to revise it (if will be supported at all)
 */

bool FRAM_IsValidRange(uint16_t addr, uint16_t size);
HAL_StatusTypeDef FRAM_Write(uint16_t addr, const uint8_t *buf, uint16_t size);
HAL_StatusTypeDef FRAM_Read(uint16_t addr, uint8_t *buf, uint16_t size);

#endif /* INC_FRAM_H_ */
