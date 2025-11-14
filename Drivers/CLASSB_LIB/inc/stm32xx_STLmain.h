/**
  ******************************************************************************
  * @file    stm32xx_STLmain.h 
  * @author  MCD Application Team
  * @version V2.3.0
  * @date    28-June-2019
  * @brief   Contains the prototypes of Self-test functions used
  *          at main program execution.
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
#ifndef __STL_MAIN_H
#define __STL_MAIN_H

/* Includes ------------------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void STL_InitRunTimeChecks(void);
void STL_DoRunTimeChecks(void);

#endif /* __STL_MAIN_H */

/******************* (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
