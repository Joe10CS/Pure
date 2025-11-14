/**
  ******************************************************************************
  * @file    stm32xx_STLmain.c 
  * @author  MCD Application Team
  * @version V2.3.0
  * @date    28-Jun-2019
  * @brief   Contains the Self-test functions executed during main program execution.
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
ErrorStatus STL_CheckStack(void);

/******************************************************************************/
/**
  * @brief  Initializes the Class B variables and their inverted redundant 
  *   counterparts. Init also the Systick for clock frequency monitoring.
  * @param  : None
  * @retval : None
  */
void STL_InitRunTimeChecks(void)
{
  uint32_t tout;
  
  /* Initialize variables for invariable memory check */
  STL_TranspMarchInit();

  /* Init Class B variables required in main routine and SysTick interrupt
  service routine for timing purposes */
  TickCounter = 0u;
  TickCounterInv = 0xFFFFFFFFuL;

  TimeBaseFlag = 0u;
  TimeBaseFlagInv = 0xFFFFFFFFuL;

  LastCtrlFlowCnt = 0u;
  LastCtrlFlowCntInv = 0xFFFFFFFFuL;
  init_control_flow();

  /* Initialize variables for SysTick interrupt routine control flow monitoring */
  ISRCtrlFlowCnt = 0u;
  ISRCtrlFlowCntInv = 0xFFFFFFFFuL;

  /* Initialize SysTick to generate 1ms time base */
#ifdef HSE_CLOCK_APPLIED    
  if (HAL_SYSTICK_Config(SYSTCLK_AT_RUN_HSE/1000uL) != HAL_OK)
#else
  if (HAL_SYSTICK_Config(SYSTCLK_AT_RUN_HSI/1000uL) != HAL_OK)
#endif
  {
    #ifdef STL_VERBOSE_POR
      printf("Run time base init failure\n\r");
    #endif /* STL_VERBOSE_POR */
    FailSafePOR();
  }

  /* Initialize variables for invariable memory check */
  STL_FlashCrc32Init();

  /* wait till HSE measurement is completed & check timer system */
  tout = HAL_GetTick() + 5u;
  LSIPeriodFlag = 0u;
  /* ==============================================================================*/
  /* MISRA violation of rule 12.4, 12.5 - "&&" operand can't contain side effects 
    and shall be primary expressions  */
  #ifdef __IAR_SYSTEMS_ICC__  /* IAR Compiler */
    #pragma diag_suppress= Pm026,Pm027              
  #endif /* __IAR_SYSTEMS_ICC__ */
  while ( LSIPeriodFlag == 0u  &&  HAL_GetTick() < tout )
  { }
  LSIPeriodFlag = 0u;
  while ( LSIPeriodFlag == 0u  &&  HAL_GetTick() < tout )
  { }
  #ifdef __IAR_SYSTEMS_ICC__  /* IAR Compiler */
    #pragma diag_default= Pm026,Pm027
  #endif /* __IAR_SYSTEMS_ICC__ */
  /* ==============================================================================*/

  if(HAL_GetTick() >= tout)
  {
    #ifdef STL_VERBOSE_POR
      printf("Run time clock measurement failure\n\r");
    #endif /* STL_VERBOSE_POR */
    //FailSafePOR();
  }
    
  /* Initialize variables for main routine control flow monitoring */
  CtrlFlowCnt = 0u;
  CtrlFlowCntInv = 0xFFFFFFFFuL;
  
#ifdef USE_INDEPENDENT_WDOG
  /* Initialize IWDG for run time if applicable */  
  #if defined(STL_EVAL_MODE)
    /* setup DBGMCU block - stop IWDG at break in debug mode */
    __DBGMCU_CLK_ENABLE();
    __DBGMCU_FREEZE_IWDG();
  #endif  /* STL_EVAL_MODE */
    
  /* Setup cca 50 msec period - to be adjusted in according w/ appli need */
  IwdgHandle.Instance = IWDG;
  IwdgHandle.Init.Prescaler = IWDG_PRESCALER_256;
  IwdgHandle.Init.Reload = 8U;
  /* Enable IWDG (LSI automatically enabled by HW) */
  #ifdef IWDG_FEATURES_BY_WINDOW_OPTION
    IwdgHandle.Init.Window = 8U;
    /* if window feature is applied, Init() function has to include __HAL_IWDG_START() prior any initialization */
    if( HAL_IWDG_Init(&IwdgHandle) != HAL_OK )
    {
      #ifdef STL_VERBOSE
        printf(" IWDG init Error\n\r");
      #endif  /* STL_VERBOSE */
      FailSafePOR();
    }
  #else
#if 0 // YCS - already initialized
    /* if window feature is not applied Init() precedes Start() */
    if( HAL_IWDG_Init(&IwdgHandle) != HAL_OK )
    {
      #ifdef STL_VERBOSE
        printf(" IWDG init Error\n\r");
      #endif  /* STL_VERBOSE */
      FailSafePOR();
    }
    __HAL_IWDG_START(&IwdgHandle);
#endif
  #endif /* IWDG_FEATURES_BY_WINDOW_OPTION */
#endif /* USE_INDEPENDENT_WDOG */

#ifdef USE_WINDOW_WDOG
  /* Initialize WWDG for run time if applicable */  
  #if defined(STL_EVAL_MODE)    
    /* setup DBGMCU block - stop WWDG at break in debug mode */
    __DBGMCU_CLK_ENABLE();
    __DBGMCU_FREEZE_WWDG();
  #endif  /* STL_EVAL_MODE */
  
  /* Setup period - to be adjusted in according w/ appli need */
  __WWDG_CLK_ENABLE();
  WwdgHandle.Instance = WWDG;
  WwdgHandle.Init.Prescaler = WWDG_PRESCALER_8;
  WwdgHandle.Init.Counter = 127U;
  WwdgHandle.Init.Window = 127U;
  if( HAL_WWDG_Init(&WwdgHandle) != HAL_OK )
  {
    #ifdef STL_VERBOSE
      printf(" WWDG init Error\n\r");
    #endif  /* STL_VERBOSE */
    FailSafePOR();
  }
#endif /* USE_WINDOW_WDOG */ 
}

/******************************************************************************/
/**
  * @brief  Perform set of generic tests at run time from main
  *         (can be divided into more separated segments if necessary)
  * @param  : None
  * @retval : None
  */
void STL_DoRunTimeChecks(void)
{
  uint32_t rom_test;
  
  /* Is the time base duration elapsed? */
  if (TimeBaseFlag == 0xAAAAAAAAuL)
  {
    /* Toggle LED_Non Volatile Memory */
    #if defined STL_EVAL_MODE
      BSP_LED_Toggle(LED_NVM);
    #endif  /* STL_EVAL_MODE */
      
    #if defined STL_USER_AUX_MODE
      User_AUX_Toggle(AUX_NVM);
    #endif /* STL_USER_AUX_MODE */
      
    /* Verification of TimeBaseFlag integrity */
    /* The following pair of volatile variables is changed consistently at Systick ISR only */
    if ((TimeBaseFlag ^ TimeBaseFlagInv) == 0xFFFFFFFFuL)
    {
      TimeBaseFlag = 0u;
    
      /*----------------------------------------------------------------------*/
      /*---------------------------- CPU registers ----------------------------*/
      /*----------------------------------------------------------------------*/
      control_flow_call(CPU_TEST_CALLER);
      
      if (STL_RunTimeCPUTest() != CPUTEST_SUCCESS)
      {
        #ifdef STL_VERBOSE
          printf("Run-time CPU Test Failure\n\r");
        #endif /* STL_VERBOSE */
        FailSafePOR();
      }
      else
      {
        control_flow_resume(CPU_TEST_CALLER);
      }
  
      /*----------------------------------------------------------------------*/
      /*------------------------- Stack overflow -----------------------------*/
      /*----------------------------------------------------------------------*/
      control_flow_call(STACK_OVERFLOW_TEST);
      
      if (STL_CheckStack() != SUCCESS)
      {
        #ifdef STL_VERBOSE
          printf("Stack overflow\n\r");
        #endif /* STL_VERBOSE */
        FailSafePOR();
      }
      else
      {
        control_flow_resume(STACK_OVERFLOW_TEST);
      }
      
      /*----------------------------------------------------------------------*/
      /*------------------------- Clock monitoring ---------------------------*/
      /*----------------------------------------------------------------------*/
      control_flow_call(CLOCK_TEST_CALLER);
      
      switch ( STL_MainClockTest() )
      {
        case FREQ_OK:
          control_flow_resume(CLOCK_TEST_CALLER);
          break;
  
        case EXT_SOURCE_FAIL:
          #ifdef STL_VERBOSE
            /* finish communication flow prior system clock change */
            UartHandle.Instance = USARTx;
            while (HAL_UART_GetState(&UartHandle) == HAL_UART_STATE_BUSY_TX)
            { }
            /* Re-init communication channel with modified clock setting */
            HAL_UART_DeInit(&UartHandle);
            UartHandle.Init.BaudRate   = 115200;
            UartHandle.Init.WordLength = UART_WORDLENGTH_8B;
            UartHandle.Init.StopBits   = UART_STOPBITS_1;
            UartHandle.Init.Parity     = UART_PARITY_NONE;
            UartHandle.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
            UartHandle.Init.Mode       = UART_MODE_TX;
            HAL_UART_Init(&UartHandle);
            printf("\n\r Clock Source failure (Run-time)\n\r");
          #endif /* STL_VERBOSE */
          FailSafePOR();
          break;
  
        case CLASS_B_VAR_FAIL:
          #ifdef STL_VERBOSE
            printf("\n\r Class B variable error (clock test)\n\r");
          #endif /* STL_VERBOSE */
          FailSafePOR();
          break;
  
        case LSI_START_FAIL:
        case HSE_START_FAIL:
        case HSI_HSE_SWITCH_FAIL:
        case TEST_ONGOING:
        case CTRL_FLOW_ERROR:
        default:
          #ifdef STL_VERBOSE
            printf("Abnormal Clock Test routine termination \n\r");
          #endif  /* STL_VERBOSE */
          FailSafePOR();
          break;
      }
  
  
      /*----------------------------------------------------------------------*/
      /*------------------ Invariable memory CRC check -----------------------*/
      /*----------------------------------------------------------------------*/
      control_flow_call(FLASH_TEST_CALLER);
			
      rom_test = STL_crc32Run();  /* Requires the control flow check to be modified */
      switch ( rom_test )
      {
        case TEST_RUNNING:
            control_flow_resume(FLASH_TEST_CALLER);
          break;
  
        case TEST_OK:
          #ifdef STL_VERBOSE
            /* avoid any long string output here, '*' just marks the flash test completed ok */
            #ifndef __GNUC__
              putchar((int16_t)'*');
            #else
              __io_putchar((int16_t)'*');
            #endif /* __GNUC__ */
          #endif /* STL_VERBOSE */
          #if defined STL_EVAL_MODE
            /* Toggle LED_NVM for debug purpose */
            BSP_LED_Toggle(LED_NVM);
          #endif  /* STL_EVAL_MODE */
          #if defined STL_USER_AUX_MODE
            User_AUX_Toggle(AUX_NVM);
          #endif /* STL_USER_AUX_MODE */
          #if defined(STL_EVAL_LCD)
            ++MyFLASHCounter;
          #endif /* STL_EVAL_LCD */
          CtrlFlowCntInv -= FLASH_TEST_CALLER;
          break;
  
        case TEST_FAILURE:
        case CLASS_B_DATA_FAIL:
        default:
          #ifdef STL_VERBOSE
            printf(" Run-time FLASH CRC Error\n\r");
          #endif  /* STL_VERBOSE */
          FailSafePOR();
          break;
      }
  
      /*----------------------------------------------------------------------*/
      /*---------------- Check Safety routines Control flow  -----------------*/
      /*------------- Refresh Window and independent watchdogs ---------------*/
      /*----------------------------------------------------------------------*/
      #ifdef USE_WINDOW_WDOG
        /* Update WWDG counter */
        WwdgHandle.Instance = WWDG;
        if( HAL_WWDG_Refresh(&WwdgHandle) != HAL_OK )
        {
          #ifdef STL_VERBOSE
            printf(" WWDG refresh error\n\r");
          #endif  /* STL_VERBOSE */
          FailSafePOR();
        }
      #endif /* USE_WINDOW_WDOG */

        /* Reload IWDG counter */
      #ifdef USE_INDEPENDENT_WDOG
        IwdgHandle.Instance = IWDG;
        if( HAL_IWDG_Refresh(&IwdgHandle) != HAL_OK )
        {
          #ifdef STL_VERBOSE
            printf(" IWDG refresh error\n\r");
          #endif  /* STL_VERBOSE */
          FailSafePOR();
        }
      #endif /* USE_INDEPENDENT_WDOG */
  
      if (((CtrlFlowCnt ^ CtrlFlowCntInv) == 0xFFFFFFFFuL)
        &&((LastCtrlFlowCnt ^ LastCtrlFlowCntInv) == 0xFFFFFFFFuL))
      {
        if (rom_test == TEST_OK)
        {
          /* ==============================================================================*/
          /* MISRA violation of rule 11.4,17.4 - pointer arithmetic & casting has is used */
          #ifdef __IAR_SYSTEMS_ICC__  /* IAR Compiler */
            #pragma diag_suppress=Pm088,Pm141
          #endif /* __IAR_SYSTEMS_ICC__ */
          if ((CtrlFlowCnt == FULL_FLASH_CHECKED) 
            && ((CtrlFlowCnt - LastCtrlFlowCnt) == (LAST_DELTA_MAIN)))
          #ifdef __IAR_SYSTEMS_ICC__  /* IAR Compiler */
            #pragma diag_default=Pm088,Pm141
          #endif /* __IAR_SYSTEMS_ICC__ */
          /* ==============================================================================*/
          {
            CtrlFlowCnt = 0u;
            CtrlFlowCntInv = 0xFFFFFFFFuL;
          }
          else  /* Return value form crc check was corrupted */
          {
            #ifdef STL_VERBOSE
              printf("Control Flow Error (main loop, Flash CRC)\n\r");
            #endif  /* STL_VERBOSE */
            FailSafePOR();
          }
        }
        else  /* Flash test not completed yet */
        {
          if ((CtrlFlowCnt - LastCtrlFlowCnt) != DELTA_MAIN)
          {
            #ifdef STL_VERBOSE
              printf("Control Flow Error (main loop, Flash CRC on-going)\n\r");
            #endif  /* STL_VERBOSE */
            FailSafePOR();
          }
        }
  
        LastCtrlFlowCnt = CtrlFlowCnt;
        LastCtrlFlowCntInv = CtrlFlowCntInv;
      }
      else
      {
        #ifdef STL_VERBOSE
          printf("Class B variable Error (main loop)\n\r");
        #endif  /* STL_VERBOSE */
        FailSafePOR();
      }
    }
    else
    {
      #ifdef STL_VERBOSE
        printf("Class B variable Error (main loop)\n\r");
      #endif  /* STL_VERBOSE */
      FailSafePOR();
    }
    #if defined STL_EVAL_MODE
      /* Toggle LD3 for debug purpose */
      BSP_LED_Toggle(LED_NVM);
    #endif  /* STL_EVAL_MODE */
    #if defined STL_USER_AUX_MODE
      User_AUX_Toggle(AUX_NVM);
    #endif /* STL_USER_AUX_MODE */
  } /* End of periodic Self-test routine */
}

/******************************************************************************/
/**
  * @brief  This function verifies that Stack didn't overflow
  * @param  : None
  * @retval : ErrorStatus = (ERROR, SUCCESS)
  */
ErrorStatus STL_CheckStack(void)
{
  ErrorStatus result = SUCCESS;
  
  CtrlFlowCnt += STACK_OVERFLOW_CALLEE;

  if ( aStackOverFlowPtrn[0] != 0xEEEEEEEEuL )
  {
    result = ERROR;
  }
  if ( aStackOverFlowPtrn[1] != 0xCCCCCCCCuL )
  {
    result = ERROR;
  }
  if ( aStackOverFlowPtrn[2] != 0xBBBBBBBBuL )
  {
    result = ERROR;
  }
  if ( aStackOverFlowPtrn[3] != 0xDDDDDDDDuL )
  {
    result = ERROR;
  }

  CtrlFlowCntInv -= STACK_OVERFLOW_CALLEE;

  return (result);
}

/**
  * @}
  */

/******************* (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
