/**
  ******************************************************************************
  * @file    stm32xx_STLcrc32Run.c 
  * @author  MCD Application Team
  * @version V2.3.0
  * @date    28-Jun-2019
  * @brief   Contains the functions performing run time invariable
  *          memory checks based on 32-bit CRC
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

  /* ==============================================================================*/
  /* MISRA violation of rule 10.3,11.3,11.4,17.4 - pointer arithmetic & casting is used for RAM area testing */
  #ifdef __IAR_SYSTEMS_ICC__  /* IAR Compiler */
    #pragma diag_suppress= Pm088,Pm136,Pm140,Pm141
  #endif /* __IAR_SYSTEMS_ICC__ */
/**
  * @brief  Initializes the pointers to the Flash memory required during
  *   run-time
  * @param  : None
  * @retval : None
  */
void STL_FlashCrc32Init(void)
{
  pRunCrc32Chk = (uint32_t*)ROM_START;
  pRunCrc32ChkInv = ((uint32_t *)(uint32_t)(~(uint32_t)(ROM_START)));
  
  CrcHandle.Instance = CRC;

  /* Reset CRC Calculation Unit */
  __HAL_CRC_DR_RESET(&CrcHandle);

  #ifdef  CRC_UNIT_CONFIGURABLE
    CrcHandle.Init.DefaultPolynomialUse    = DEFAULT_POLYNOMIAL_ENABLE;
    CrcHandle.Init.DefaultInitValueUse     = DEFAULT_INIT_VALUE_ENABLE;
    CrcHandle.Init.InputDataInversionMode  = CRC_INPUTDATA_INVERSION_NONE;
    CrcHandle.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_DISABLED;
    CrcHandle.InputDataFormat              = CRC_INPUTDATA_FORMAT_WORDS;
  #endif
  HAL_CRC_Init(&CrcHandle);
}

uint32_t dallcrc = 0;

/**
  * @brief  Computes the crc in multiple steps and compare it with the
  *   ref value when the whole memory has been tested
  * @param  : None
  * @retval : ClassBTestStatus (TEST_RUNNING, CLASS_B_DATA_FAIL,
  *   TEST_FAILURE, TEST_OK)
  */
ClassBTestStatus STL_crc32Run(void)
{
  ClassBTestStatus result = CTRL_FLW_ERROR; /* In case of abnormal func exit*/

  CtrlFlowCnt += CRC32_RUN_TEST_CALLEE;

  /* Check Class B var integrity */
  if ((((uint32_t)pRunCrc32Chk) ^ ((uint32_t)pRunCrc32ChkInv)) == 0xFFFFFFFFuL)
  {
    if (pRunCrc32Chk < (uint32_t *)ROM_END)
    {
      /* the next for cycle replaces the HAL function call
         HAL_CRC_Accumulate(&CrcHandle, (uint32_t *)pRunCrc32Chk, (uint32_t)FLASH_BLOCK_WORDS);
         due to bug at IAR linker - check sum computation can't support both big & little endian  */
      uint32_t index;
      for(index = 0; index < (uint32_t)FLASH_BLOCK_WORDS; index++)
      {
        CRC->DR = __REV(*(pRunCrc32Chk + index));
      }
      pRunCrc32Chk += FLASH_BLOCK_WORDS;     /* Increment pointer to next block */
      pRunCrc32ChkInv = ((uint32_t *)~((uint32_t)pRunCrc32Chk));
      result = TEST_RUNNING;
    }
    else
    {
      if ((RefCrc32 ^ RefCrc32Inv) == 0xFFFFFFFFuL)
      {
        CtrlFlowCnt += CRC32_INIT_CALLER;
        dallcrc = CRC->DR;  // DEBUG REMOVE
        if(CRC->DR == REF_CRC32)
        {
          result = TEST_OK;
        }
        else
        {
          result = TEST_FAILURE;
        }
        STL_FlashCrc32Init(); /* Prepare next test (or redo it if this one failed) */
        
        CtrlFlowCntInv -= CRC32_INIT_CALLER;
      }
      else /* Class B error on RefCrc32 */
      {
        result = CLASS_B_DATA_FAIL;
      }
    }
  }
  else  /* Class B error pRunCrc32Chk */
  {
    result = CLASS_B_DATA_FAIL;
  }

  CtrlFlowCntInv -= CRC32_RUN_TEST_CALLEE;

  return (result);

}
  #ifdef __IAR_SYSTEMS_ICC__  /* IAR Compiler */
    #pragma diag_default=Pm088,Pm136,Pm140,Pm141
  #endif  /* __IAR_SYSTEMS_ICC__ */
  /* ==============================================================================*/

/**
  * @}
  */
/******************* (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
