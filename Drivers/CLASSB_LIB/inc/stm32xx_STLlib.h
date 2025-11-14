/**
  ******************************************************************************
  * @file    stm32xx_STLlib.h 
  * @author  MCD Application Team
  * @version V2.3.0
  * @date    28-June-2019
  * @brief   This file references all header files of the Self Test Library
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
#ifndef __STM32xx_STL_LIB_H
#define __STM32xx_STL_LIB_H

/* Includes ------------------------------------------------------------------*/
#include "stm32xx_STLparam.h"
/* Include Class B variables */
  /* ==============================================================================*/
  /* MISRA violation of rule 8.5 - alocation of variables done via header file jus at this place */
	#ifdef __IAR_SYSTEMS_ICC__  /* IAR Compiler */
		#pragma diag_suppress=Pm123
	#endif /* __IAR_SYSTEMS_ICC__ */
#include "stm32xx_STLclassBvar.h"
	#ifdef __IAR_SYSTEMS_ICC__  /* IAR Compiler */
		#pragma diag_default=Pm123
	#endif /* __IAR_SYSTEMS_ICC__ */
  /* ==============================================================================*/

/* Self Test library routines main flow after initialization and at run */
#include "stm32xx_STLstartup.h"
#include "stm32xx_STLmain.h"

/* Cortex-M4 CPU test */
#include "stm32xx_STLcpu.h"

/* Clock frequency test */
#include "stm32xx_STLclock.h"

/* Invariable memory test */
#include "stm32xx_STLcrc32.h"

/* Variable memory test */
#include "stm32xx_STLRam.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
extern UART_HandleTypeDef UartHandle;
extern IWDG_HandleTypeDef IwdgHandle;
#ifdef USE_WINDOW_WDOG // YCS
extern WWDG_HandleTypeDef WwdgHandle;
#endif // YCS
extern CRC_HandleTypeDef CrcHandle;
extern RCC_ClkInitTypeDef RCC_ClkInitStruct;
extern RCC_OscInitTypeDef RCC_OscInitStruct;

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

#endif /* __STM32xx_STL_LIB_H */

/******************* (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
