/**
  ******************************************************************************
  * @file    stm32xx_STLclock.h 
  * @author  MCD Application Team
  * @version V2.3.0
  * @date    28-June-2019
  * @brief   This file contains the prototype of test function for
  *          clock circuitry and frequency at start-up.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2019 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */ 


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STL_CLOCK_H
#define __STL_CLOCK_H

/* Includes ------------------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/
typedef enum {
              LSI_START_FAIL,
              HSE_START_FAIL,
              HSI_HSE_SWITCH_FAIL,
              TEST_ONGOING,
              EXT_SOURCE_FAIL,
              XCROSS_CONFIG_FAIL,
              HSE_HSI_SWITCH_FAIL,
              PLL_OFF_FAIL,
              CLASS_B_VAR_FAIL,
              CTRL_FLOW_ERROR,
              FREQ_OK
              } ClockStatus;

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
ClockStatus STL_ClockStartUpTest(void);
ClockStatus STL_MainClockTest(void);
ErrorStatus STL_InitClock_Xcross_Measurement(void);

#endif /* __STL_CLOCK_H */

/******************* (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
