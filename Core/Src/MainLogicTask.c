/**
 ******************************************************************************
 * @file           : MainLogicTask.c
 * @brief          : MainLogic task
 ******************************************************************************
 */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "EventQueue.h"

/* Private includes ----------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
#define DO_EVENT_INTERVAL_TICKS (1)
/* Private macro -------------------------------------------------------------*/

/* External variables ---------------------------------------------------------*/
// TIM1 runs the main logic each 10ms
extern TIM_HandleTypeDef htim1;
/* Private variables ---------------------------------------------------------*/
SMSodaStreamPure mStateMachine;  // the state machine instance
// Defines the state of the pin at home position, default is 1 (SET)
/* Private function prototypes -----------------------------------------------*/

/* Private user code ---------------------------------------------------------*/
/**
 * @brief  Function implementing the Main Logic Task main loop
 * @param  None
 * @retval None
 */
void MainLogicInit(void) {

	// Initialize the state machine
	SMSodaStreamPure_ctor(&mStateMachine);
	SMSodaStreamPure_start(&mStateMachine);

	// starts the main logic timer
	HAL_TIM_Base_Start_IT(&htim1);

}

void MainLogicPeriodic() {

	// optional: dispatch DO every tick
	SMSodaStreamPure_dispatch_event(&mStateMachine, SMSodaStreamPure_EventId_DO);

	// dispatch queued events from your ring buffer (if any)
	SMSodaStreamPure_EventId ev;
	while (SMEventQueue_Take(&ev)) {
		SMSodaStreamPure_dispatch_event(&mStateMachine, ev);
	}
}
