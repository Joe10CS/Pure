/**
  ******************************************************************************
  * @file    stm32xx_STLtranspRam.c 
  * @author  MCD Application Team
  * @version V2.3.0
  * @date    28-Jun-2019
  * @brief   This module contains functions for transparent RAM functional 
  *          testing March C- & X algorithms, to be done during run-time.
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


/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32xx_STLparam.h"
#include "stm32xx_STLlib.h"

/** @addtogroup STM32xxSelfTestLib_src
  * @{
  */ 

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Initializes the pointer to the RAM for the run-time
  *   transparent functional test.
  * @param  : None
  * @retval : None
  */
void STL_TranspMarchInit(void)
{
  /* start address of the test has to be aligned to 16 address range */	  
   pRunTimeRamChk = (uint32_t *)((uint32_t)CLASS_B_START & 0xFFFFFFFCuL);
   pRunTimeRamChkInv = (uint32_t *)(uint32_t)(~(uint32_t)pRunTimeRamChk);
}

/* ---------------------------------------------------------------------------*/
/**
  * @brief  This function verifies that 6 words of RAM are functional,
  *   overlapping) using the March C- or March X algorithm.
  * @param  : None
  * @retval : ClassBTestStatus (TEST_RUNNING, CLASS_B_DATA_FAIL,
  *   TEST_FAILURE, TEST_OK)
  */
ClassBTestStatus STL_TranspMarch(void)
{
  ClassBTestStatus result = TEST_RUNNING;
  
  /* Check Class B var integrity */
  if ((((uint32_t)pRunTimeRamChk) ^ ((uint32_t)pRunTimeRamChkInv)) == 0xFFFFFFFFuL)
  {
    if (pRunTimeRamChk >= CLASS_B_END)
    {
      /*------------- March test applied on the RAM Buffer itself --------------- */
      if (STL_TranspRamMarchCXStep(&aRunTimeRamBuf[0], &aRunTimeRamBuf[0], BCKGRND) == SRAMTEST_SUCCESS)         
      {
        /* All the RAM test is completed successfully */
        result = TEST_OK;
      }
      else  /* Buffer is not functional */
      {
        result = TEST_FAILURE;
      }
      /* Init next cycle of the transparent RAM test starting from the begin of the Class B area */
      pRunTimeRamChk = CLASS_B_START;
      pRunTimeRamChkInv = ((uint32_t *)~((uint32_t)CLASS_B_START));
    }
    else
    { /* ------------- March test applied on Class B data area ------------------ */
      /* !!! Application has to ensure that no write or read operation
               is performed within Class B RAM area during this test !!!      */
      if (STL_TranspRamMarchCXStep(pRunTimeRamChk, &aRunTimeRamBuf[1], BCKGRND) == SRAMTEST_SUCCESS)         
      {
        /* Prepare next Row Transparent RAM test */
        pRunTimeRamChk += RT_RAM_BLOCKSIZE - (2u * RT_RAM_BLOCK_OVERLAP);
        pRunTimeRamChkInv = (uint32_t *)(uint32_t)(~(uint32_t)pRunTimeRamChk);
      }
      else
      {
        result = TEST_FAILURE;  /* Word line under test was not functional */
      }
    }
  }
  else  /* Class B error on pRunTimeRamChk */
  {
    result = CLASS_B_DATA_FAIL;
  }
  return (result);  
}

/**
  * @}
  */

/******************* (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
