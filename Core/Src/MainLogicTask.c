/**
 ******************************************************************************
 * @file           : MainLogicTask.c
 * @brief          : MainLogic task
 ******************************************************************************
 */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stdio.h"
#include "string.h"
#include "EventQueue.h"
#include "RxTxMsgs.h"
/* Private includes ----------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
#define DO_EVENT_INTERVAL_TICKS (1)
/* Private macro -------------------------------------------------------------*/

/* External variables ---------------------------------------------------------*/
// TIM1 runs the main logic each 10ms
extern TIM_HandleTypeDef htim1;

extern int8_t filtered_x;
extern int8_t filtered_y;
extern int8_t filtered_z;

/* Private variables ---------------------------------------------------------*/
SMSodaStreamPure mStateMachine;  // the state machine instance
eUartStatus glb_new_msg = eUART_NoMessage;
sUartMessage glb_last_RxMessage;
uint8_t gRawMsgForEcho[MAX_RX_BUFFER_LEN];
uint32_t gRawMessageLen = 0;
uint32_t gQueueErrors = 0;
extern uint16_t mWaterLevelSensorThreahsold; // Hold the threshold (A2D) value of the water sensor for bottle full detection
uint32_t gPeriodicStatusSendIntervalMilliSconds = 0; // 0 -don't send
bool gIsGuiControlMode = false;
bool gIsUVLEdOn = false;
bool gIsTilted = false;
bool gAccelerometerIsPresent = false;
// These variables store the current state of various values that, among other purposes, used for reading by the GUI
extern uint16_t mReadWaterLevelADC; // Hold the last read (A2D) value of the water level sensor
uint32_t gUVLedLastReadCurrentMilliAmp = 0;
extern uint16_t mReadWaterPumpCureentADC;
uint32_t gRTCTotalSecondsFromLastFilterReset = 0;
bool gFirstTime = true;
// Defines the state of the pin at home position, default is 1 (SET)
/* Private function prototypes -----------------------------------------------*/
void ProcessNewRxMessage(sUartMessage* msg, uint8_t *gRawMsgForEcho, uint32_t rawMessageLen);
void CheckHWAndGenerateEventsAsNeeded();

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


	gAccelerometerIsPresent = AccelerometerIsPresent();
	// Initialize the Accelerometer
	AccelerometerInit();

	// starts the main logic timer
	HAL_TIM_Base_Start_IT(&htim1);

}

bool dbgSMEnabled = false;
SMSodaStreamPure_StateId dbgCurrentState = SMSodaStreamPure_StateId_ROOT;
SMSodaStreamPure_StateId dbgNewState = SMSodaStreamPure_StateId_ROOT;

void MainLogicPeriodic() {


	if (gFirstTime)
	{
		gFirstTime = false;
		// Start reading from the UART
		COMM_UART_StartRx();
		// on powerup, send the version number
		glb_last_RxMessage.cmd = eUARTCommand_rver;
		ProcessNewRxMessage(&glb_last_RxMessage, gRawMsgForEcho, gRawMessageLen);
		StartADCConversion();
	}

	CheckHWAndGenerateEventsAsNeeded();

	// optional: dispatch DO every tick
	SMSodaStreamPure_dispatch_event(&mStateMachine, SMSodaStreamPure_EventId_DO);
	// DEBUG REMOVE
	if (dbgSMEnabled)
	{
		if (dbgCurrentState != mStateMachine.state_id)
		{
			sprintf((char *)gRawMsgForEcho, "%d[0]%d\r\n",(int)dbgCurrentState, (int)mStateMachine.state_id);
			dbgCurrentState = mStateMachine.state_id;
			COMM_UART_QueueTxMessage(gRawMsgForEcho, strlen((const char *)gRawMsgForEcho));
		}
	}
	// DEBUG REMOVE

	// dispatch queued events from your ring buffer (if any)
	SMSodaStreamPure_EventId ev;
	while (SMEventQueue_Take(&ev)) {
		SMSodaStreamPure_dispatch_event(&mStateMachine, ev);
		// DEBUG REMOVE
		if (dbgSMEnabled)
		{
			if ((dbgCurrentState != mStateMachine.state_id) || (ev != SMSodaStreamPure_EventId_DO))
			{
				sprintf((char *)gRawMsgForEcho, "%d[%d]%d\r\n",(int)dbgCurrentState, (int)ev, (int)mStateMachine.state_id);
				dbgCurrentState = mStateMachine.state_id;
				COMM_UART_QueueTxMessage(gRawMsgForEcho, strlen((const char *)gRawMsgForEcho));
			}
		}
		// DEBUG REMOVE
	}

	// process all pending commands
	while(eUART_MesssagePending == (glb_new_msg = COMM_UART_CheckNewMessage(&glb_last_RxMessage, gRawMsgForEcho, &gRawMessageLen))) {
		ProcessNewRxMessage(&glb_last_RxMessage, gRawMsgForEcho, gRawMessageLen);
	}

}


/**
  * @brief  Process the commands,
  * @param  sCDMMessage* msg - pointer to the received message
  * @param  uint8_t *gRawMsgForEcho - pointer to the raw received buffer (before parsing) - used for echoing the command - ask ACK
  * @param  uint32_t rawMessageLen - length of the raw received buffer
  * @retval none
  */
void ProcessNewRxMessage(sUartMessage* msg, uint8_t *gRawMsgForEcho, uint32_t rawMessageLen)
{
	// All command are echo by default, unless a response is sent (or error)
	bool echoCommand = true;
	SMSodaStreamPure_EventId event = SMSodaStreamPure_EventId_DO; // stam
	/*
	eUARTCommand_rsts,
	 *
	 */
	switch (msg->cmd)
	{
	case eUARTCommand_rver:
		sprintf((char *)gRawMsgForEcho, "$RVER Version %d.%d\r\n",(int)(SWVERSION_MAJOR),(int)(SWVERSION_MINOR));
		COMM_UART_QueueTxMessage(gRawMsgForEcho, strlen((const char *)gRawMsgForEcho));
		echoCommand = false;
		break;
	case eUARTCommand_mgui:
		gIsGuiControlMode = (msg->params.onOff.isOn == 1);
		if (! SMEventQueue_Add(gIsGuiControlMode? SMSodaStreamPure_EventId_EVENT_ENTER_GUI_CONTROLLED_MODE : SMSodaStreamPure_EventId_EVENT_EXIT_GUI_CONTROLLED_MODE))
			gQueueErrors++;
		break;
	case eUARTCommand_powr:
		if (! SMEventQueue_Add((msg->params.onOff.isOn == 1)? SMSodaStreamPure_EventId_EVENT_SOLENDOIDPUMPPOWERON : SMSodaStreamPure_EventId_EVENT_SOLENDOIDPUMPPOWEROFF))
			gQueueErrors++;
		break;
	case eUARTCommand_sled:
		if (! SMEventQueue_Add(SMSodaStreamPure_EventId_EVENT_SETLED))
			gQueueErrors++;
		break;
	case eUARTCommand_srgb:
		if (! SMEventQueue_Add(SMSodaStreamPure_EventId_EVENT_SETRGBLED))
			gQueueErrors++;
		break;
	case eUARTCommand_pump:
		if (msg->params.pump.isOn == 1)
		{
			mWaterLevelSensorThreahsold = (uint16_t)(msg->params.pump.sensorThreashold);
			event = (msg->params.pump.sensorThreashold == 0) ? SMSodaStreamPure_EventId_EVENT_WATERPUMOONNOSENSOR : SMSodaStreamPure_EventId_EVENT_WAREPUMPON;
		}
		else
		{
			event = SMSodaStreamPure_EventId_EVENT_WATERPUMPOFF;
		}
		if (! SMEventQueue_Add(event))
			gQueueErrors++;
		break;
	case eUARTCommand_carb:
		if (! SMEventQueue_Add((msg->params.onOff.isOn == 1)? SMSodaStreamPure_EventId_EVENT_CARBON : SMSodaStreamPure_EventId_EVENT_CARBOFF))
			gQueueErrors++;
		break;
	case eUARTCommand_stop:
		if (! SMEventQueue_Add(SMSodaStreamPure_EventId_EVENT_STOP))
			gQueueErrors++;
		break;
	case eUARTCommand_tilt: // Get Info - non state machine related command
		sprintf((char *)gRawMsgForEcho, "$TILT %d,%d,%d\r\n",(int)filtered_x,(int)filtered_y,(int)filtered_z);
		COMM_UART_QueueTxMessage(gRawMsgForEcho, strlen((const char *)gRawMsgForEcho));
		echoCommand = false;
		break;
	case eUARTCommand_wtrs: // Get Info - non state machine related command
		sprintf((char *)gRawMsgForEcho, "$WRTS %d\r\n",(int)mReadWaterLevelADC);
		COMM_UART_QueueTxMessage(gRawMsgForEcho, strlen((const char *)gRawMsgForEcho));
		echoCommand = false;
		break;
	case eUARTCommand_uvld:
		gIsUVLEdOn = (msg->params.onOff.isOn == 1);
		if (! SMEventQueue_Add(gIsUVLEdOn? SMSodaStreamPure_EventId_EVENT_UVLEDON : SMSodaStreamPure_EventId_EVENT_UVLEDOFF))
			gQueueErrors++;
		break;
	case eUARTCommand_uvla: // Get Info - non state machine related command
		sprintf((char *)gRawMsgForEcho, "$UVLA %d\r\n",(int)gUVLedLastReadCurrentMilliAmp);
		COMM_UART_QueueTxMessage(gRawMsgForEcho, strlen((const char *)gRawMsgForEcho));
		echoCommand = false;
		break;
	case eUARTCommand_pmpa: // Get Info - non state machine related command
		sprintf((char *)gRawMsgForEcho, "$PMPA %d\r\n",(int)mReadWaterPumpCureentADC);
		COMM_UART_QueueTxMessage(gRawMsgForEcho, strlen((const char *)gRawMsgForEcho));
		echoCommand = false;
		break;
	case eUARTCommand_rrtc: // Get Info - non state machine related command
		sprintf((char *)gRawMsgForEcho, "$RRTC %d\r\n",0); // TOOD get total seconds from RTC
		COMM_UART_QueueTxMessage(gRawMsgForEcho, strlen((const char *)gRawMsgForEcho));
		echoCommand = false;
		break;
	case eUARTCommand_rsts:
		if (! SMEventQueue_Add((msg->params.periodicStatus.periodicStatusInterval > 0) ? SMSodaStreamPure_EventId_EVENT_STARTSTATUSTRANSMIT : SMSodaStreamPure_EventId_EVENT_STOPSTATUSTRANSMIT))
			gQueueErrors++;
		break;

	default:
	}
	if (echoCommand == true)
	{
		COMM_UART_QueueTxMessage(gRawMsgForEcho, rawMessageLen);
	}
}


void CheckHWAndGenerateEventsAsNeeded()
{

	// check for tilt and set event if needed
	if (gAccelerometerIsPresent)
	{
		if (IsSlanted())
		{
			if (! gIsTilted) // just became tilted
			{
				SMEventQueue_Add(SMSodaStreamPure_EventId_EVENT_TILTDETECTED);
			}
			gIsTilted = true;

		}
		else if (gIsTilted) // if was tilted before and not tilted anymore
		{
			gIsTilted = false;
			SMEventQueue_Add(SMSodaStreamPure_EventId_EVENT_NOTTILTED);
		}
	}

	//if (gWaterLevelIsActive)
	{

	}
}
