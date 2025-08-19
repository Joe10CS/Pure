/*
 * WS2811.h
 *
 *  Created on: Aug 18, 2025
 *      Author: yossi
 */

#ifndef INC_WS2811_H_
#define INC_WS2811_H_

#define NUMBER_OF_WS2811_DEVICES      (6)

#define WS2811_LEDS_PER_DEVICE        (3)   // WS2811 has 3 channels per IC

#define NUMBER_OF_LEDS  (NUMBER_OF_WS2811_DEVICES * WS2811_LEDS_PER_DEVICE)

void            WS_InitLeds(void);
HAL_StatusTypeDef WS_SetLeds(uint8_t *leds_percent, uint16_t num_to_set_channels); // 0..100
HAL_StatusTypeDef WS_SetLedsRaw(uint8_t *grb_bytes, uint16_t num_to_set_channels); // 0..255
bool            WS_IsBusy(void);
#endif /* INC_WS2811_H_ */
