/**
  ******************************************************************************
  * @file    stm32xx_STLclockrun.c 
  * @author  MCD Application Team
  * @version V2.3.0
  * @date    28-Jun-2019
  * @brief   Contains routines required to monitor CPU frequency during run-time.
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
  * @brief  This function verifies the frequency from the last clock
  *   period measurement
  * @param  : None
  * @retval : ClockStatus = (LSI_START_FAIL, HSE_START_FAIL,
  *   HSI_HSE_SWITCH_FAIL, TEST_ONGOING, EXT_SOURCE_FAIL,
  *   CLASS_B_VAR_FAIL, CTRL_FLOW_ERROR, FREQ_OK)
  */
ClockStatus STL_MainClockTest(void)
{
  ClockStatus result = TEST_ONGOING; /* In case of unexpected exit */

  CtrlFlowCnt += CLOCKPERIOD_TEST_CALLEE;
  /* ==============================================================================*/
  /* MISRA violation of rule 12.4 - "&&" operand can't contain side effects in normal operation
   - all the following pairs of volatile variables are changed consistently at timer ISR only */
  #ifdef __IAR_SYSTEMS_ICC__  /* IAR Compiler */
    #pragma diag_suppress=Pm026              
  #endif /* __IAR_SYSTEMS_ICC__ */
  /* checking result of HSE measurement done at TIM5 interrupt */
  if (((PeriodValue ^ PeriodValueInv) == 0xFFFFFFFFuL)\
  &&  ((LSIPeriodFlag ^ LSIPeriodFlagInv) == 0xFFFFFFFFuL)\
  &&   (LSIPeriodFlag != 0u) )
  #ifdef __IAR_SYSTEMS_ICC__  /* IAR Compiler */
    #pragma diag_default=Pm026              
  #endif /* __IAR_SYSTEMS_ICC__ */
  /* ==============================================================================*/
   
  {
#ifdef HSE_CLOCK_APPLIED    
    if (PeriodValue < HSE_LimitLow(SYSTCLK_AT_RUN_HSE))
#else
    if (PeriodValue < HSI_LimitLow(SYSTCLK_AT_RUN_HSI))
#endif
    {
      /* Switch back to internal clock */
      RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK;
      RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
      HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0);
      result = EXT_SOURCE_FAIL;	/* Sub-harmonics: HSE -25% below expected */
    }
    else
    {
#ifdef HSE_CLOCK_APPLIED    
      if (PeriodValue > HSE_LimitHigh(SYSTCLK_AT_RUN_HSE))
#else
      if (PeriodValue > HSI_LimitHigh(SYSTCLK_AT_RUN_HSI))
#endif
      {
        /* Switch back to internal clock */
        RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK;
        RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
        HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0);
        result = EXT_SOURCE_FAIL;	/* Harmonics: HSE +25% above expected */
      }
      else
      {
        result = FREQ_OK;         /* Crystal or Resonator started correctly */        
        /* clear flag here to ensure refresh LSI measurement result will be taken at next check */
        LSIPeriodFlag = 0u;
      } /* No harmonics */
    } /* No sub-harmonics */
  } /* Control Flow error */
  else
  {
    result = CLASS_B_VAR_FAIL;
  }

  CtrlFlowCntInv -= CLOCKPERIOD_TEST_CALLEE;

  return (result);
}

/**
  * @}
  */

/******************* (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
