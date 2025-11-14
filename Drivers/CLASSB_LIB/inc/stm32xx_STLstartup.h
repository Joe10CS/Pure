/**
  ******************************************************************************
  * @file    stm32xx_STLstartup.h 
  * @author  MCD Application Team
  * @version V2.3.0
  * @date    29-June-2019
  * @brief   This file contains the prototype of the FailSafe routines and
  *          the very first self-test function to be executed after the reset.
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
#ifndef __SELF_TEST_START_UP_H
#define __SELF_TEST_START_UP_H

/* Includes ------------------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void STL_StartUp(void);
void STL_WDGSelfTest(void);

#if defined STL_EVAL_LCD
  extern uint8_t Is_LCD_Initialized;
#endif /* STL_EVAL_LCD */

#endif /* __SELF_TEST_START_UP_H */

/******************* (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
