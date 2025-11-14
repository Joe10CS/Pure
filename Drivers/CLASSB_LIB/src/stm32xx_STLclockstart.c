/**
  ******************************************************************************
  * @file    stm32xx_STLclockstart.c 
  * @author  MCD Application Team
  * @version V2.3.0
  * @date    28-Jun-2019
  * @brief   This file contains the test function for clock circuitry
  *          and wrong frequency detection at start-up.
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
  * @brief  Start up the internal and external oscillators and verifies
  *   that clock source is within the expected range
  * @param  : None
  * @retval : ClockStatus = (LSI_START_FAIL, HSE_START_FAIL,
  *   HSI_HSE_SWITCH_FAIL, XCROSS_CONFIG_FAIL, EXT_SOURCE_FAIL, FREQ_OK)
  */
ClockStatus STL_ClockStartUpTest(void)
{
  ClockStatus clck_sts = TEST_ONGOING;
  CtrlFlowCnt += CLOCK_TEST_CALLEE;

  /* Start low speed internal (LSI) oscillator */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    /* LSI clock fails */
    clck_sts = LSI_START_FAIL;
  }
  
#ifdef HSE_CLOCK_APPLIED  
  /* if LSI started OK - Start High-speed external oscillator (HSE) */
  if(clck_sts == TEST_ONGOING)
  {
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    /* Use HSEState parameter RCC_HSE_BYPASS when external generator is applied */
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
    if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
      /* HSE clock fails */
      clck_sts = HSE_START_FAIL;
    }
  }
  
  /* if HSE started OK - enable CSS */
  if(clck_sts == TEST_ONGOING)
  {
    HAL_RCC_EnableCSS();
    /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 
       clocks dividers */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSE;
    if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
    {
      /* switch to HSE clock fails */
      clck_sts = HSI_HSE_SWITCH_FAIL;
    }
  }
  
  /*-------------- Start Reference Measurement -------------------------------*/
  if(clck_sts == TEST_ONGOING)
  {  
    /* Configure dedicated timer to measure LSI period */
    if(STL_InitClock_Xcross_Measurement() == ERROR)
    {
      clck_sts = XCROSS_CONFIG_FAIL;
    }
    else
    {   
      /* Wait for two subsequent LSI periods measurements */
      LSIPeriodFlag = 0u;
      while (LSIPeriodFlag == 0u)
      { }
      LSIPeriodFlag = 0u;
      while (LSIPeriodFlag == 0u)
      { }

      /*-------------------- HSE measurement check -------------------------*/
      if (PeriodValue < HSE_LimitLow(HSE_VALUE))
      {
        /* Sub-harmonics: HSE -25% below expected */
        clck_sts = EXT_SOURCE_FAIL;
      }
      else if (PeriodValue > HSE_LimitHigh(HSE_VALUE))
      {
        /* Harmonics: HSE +25% above expected */
        clck_sts = EXT_SOURCE_FAIL;
      }
    }
  }
#else
  if(clck_sts == TEST_ONGOING)
  {
    /* Configure dedicated timer to measure LSI period */
    if(STL_InitClock_Xcross_Measurement() == ERROR)
    {
      clck_sts = XCROSS_CONFIG_FAIL;
    }
    else
    {   
      /* Wait for two subsequent LSI periods measurements */
      LSIPeriodFlag = 0u;
      while (LSIPeriodFlag == 0u)
      { }
      LSIPeriodFlag = 0u;
      while (LSIPeriodFlag == 0u)
      { }

      /*-------------------- HSI measurement check -------------------------*/
      if (PeriodValue < HSI_LimitLow(SYSTCLK_AT_STARTUP))
      {
        /* HSI -20% below expected */
        clck_sts = EXT_SOURCE_FAIL;
      }
      else if (PeriodValue > HSI_LimitHigh(SYSTCLK_AT_STARTUP))
      {
        /* HSI +20% above expected */
        clck_sts = EXT_SOURCE_FAIL;
      }
      else
      { }
    }
  }
#endif
  
  /* Switch back HSI internal clock at any case */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, MAX_FLASH_LATENCY) != HAL_OK)
  {
    /* switch to HSE clock fails */
    clck_sts = HSE_HSI_SWITCH_FAIL;
  }
  else
  {
    /* Switch off PLL */
    RCC_OscInitStruct.OscillatorType = 0u;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_OFF;
    if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
      /* PLL off fails */
      clck_sts = PLL_OFF_FAIL;
    }
    else
    {
      if(clck_sts == TEST_ONGOING)
      {
        /* the test was finished correctly */
        clck_sts = FREQ_OK;
      }
    }
  }
  
  CtrlFlowCntInv -= CLOCK_TEST_CALLEE;
  
  return(clck_sts);
}

/**
  * @}
  */

/******************* (C) COPYRIGHT 2013 STMicroelectronics *****END OF FILE****/
