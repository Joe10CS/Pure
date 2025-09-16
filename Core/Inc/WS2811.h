/*
 * WS2811.h
 *
 *  Created on: Aug 18, 2025
 *      Author: yossi
 */

#ifndef INC_WS2811_H_
#define INC_WS2811_H_

#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "stm32g0xx_hal.h"

/*
  WS2811/WS2812 single‑wire driver (STM32G030 TIM1_CH4 + TIM1_UP DMA, 64 MHz).
  - Wire protocol: GRB byte order, MSB‑first in each byte
  - This driver transmits one bit per 1.25 µs period using PWM duty to encode 0/1.
  - Uses TIM1 edge‑aligned PWM mode 1 on CH4 (PA11) and the TIM1 Update DMA request.

  Hardware assumptions:
    * PA11 is wired to LED data (through AHCT buffer).
    * TIM1 configured for PWM CH4, PSC=0, ARR=79 (1.25 µs), OC preload on CH4 DISABLED.
    * DMA TIM1_UP configured and NVIC enabled.
*/


// ===== User configuration =====
#ifndef NUMBER_OF_WS2811_DEVICES
//#define NUMBER_OF_WS2811_DEVICES      6      // physical ICs in chain
#define NUMBER_OF_WS2811_DEVICES      8      // TODO DEBUG REMOVE
#endif

#ifndef WS2811_LEDS_PER_DEVICE
#define WS2811_LEDS_PER_DEVICE        3      // WS2811 = 3 channels (G,R,B)
#endif

#define NUMBER_OF_LEDS  (NUMBER_OF_WS2811_DEVICES * WS2811_LEDS_PER_DEVICE)

// ===== Public API =====

/** Initialize the driver (idles line low, registers DMA callbacks). Call once after HAL init. */
void WS_InitLeds(void);

/** Non‑blocking send using raw bytes (0..255 per channel)
    `leds_percent` layout: GRB,GRB,GRB... (length at least num_to_set_channels).
    `num_to_set_channels` MUST be a multiple of 3 (whole devices).
    Transmits exactly num_to_set_channels*8 bits. Returns HAL_BUSY if previous frame active. */
HAL_StatusTypeDef WS_SetLeds(const uint8_t *leds_percent, uint16_t num_to_set_channels);

/** True while a frame is being sent (or until the DMA completion stops PWM). */
bool WS_IsBusy(void);

#endif /* INC_WS2811_H_ */
