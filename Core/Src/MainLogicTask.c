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
#include "WS2811.h"
#include "LedsPlayer.h"
#include "RtcBackupMemory.h"
#include "RTC.h"
/* Private includes ----------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
#define DO_EVENT_INTERVAL_TICKS (1)

#define PERIODIC_STATUS_SEND_MASK_STATEBUTS (1)
#define PERIODIC_STATUS_SEND_MASK_ADC 		(2)
#define PERIODIC_STATUS_SEND_MASK_RTCTILT 	(4)
#define PERIODIC_STATUS_SEND_MASK_LEDS   	(8)


/* Private macro -------------------------------------------------------------*/

/* External variables ---------------------------------------------------------*/
// TIM1 runs the main logic each 10ms
extern TIM_HandleTypeDef htim14;

extern int8_t filtered_x;
extern int8_t filtered_y;
extern int8_t filtered_z;
extern uint16_t gFalseButMainCounter;
extern uint16_t gFalseButCarbLevelCounter;
extern uint16_t gFalseButFilterCounter;
extern uint16_t gKeyPressButMainMS;
extern uint16_t gKeyPressButCarbLevelMS;
extern uint16_t gKeyPressButFilterMS;

uint32_t echoParams[6];
/* Private variables ---------------------------------------------------------*/
SMSodaStreamPure gStateMachine;  // the state machine instance
eUartStatus glb_new_msg = eUART_NoMessage;
sUartMessage glb_last_RxMessage;
uint8_t gRawMsgForEcho[MAX_RX_BUFFER_LEN];
uint32_t gRawMessageLen = 0;
uint32_t gQueueErrors = 0;
extern uint16_t gWaterLevelSensorThreahsold; // Hold the threshold (A2D) value of the water sensor for bottle full detection
uint32_t gPeriodicStatusSendIntervalMilliSconds = 0; // 0 -don't send
bool gIsGuiControlMode = false;
bool gIsUVLEdOn = false;
bool gIsTilted = false;
bool gAccelerometerIsPresent = false;
//bool gLP5009InitOK = false;
// These variables store the current state of various values that, among other purposes, used for reading by the GUI
extern volatile uint16_t gReadWaterLevelADC; // Hold the last read (A2D) value of the water level sensor
extern volatile uint16_t gReadWaterPumpCurrentADC;
extern volatile uint16_t gReadUVCurrentADC;
extern uint32_t gLastPumpTimeMSecs;
uint32_t gRTCTotalSecondsFromLastFilterReset = 0;
bool gFirstTime = true;

uint32_t gPumpTimoutMsecs = 120000;//30000;
uint32_t gBottleSizeThresholdmSecs = 15000;

uint16_t gPeriodicStatusSendInterval = 0;
uint16_t gPeriodicStatusSendMask = 0;
uint32_t gPeriodicStatusSendLastTickSent = 0;

uint32_t glb_safty_error_state = SAFETY_OK_STATE;
extern IWDG_HandleTypeDef hiwdg;

extern void PlayLedsPeriodic();

// Defines the state of the pin at home position, default is 1 (SET)
/* Private function prototypes -----------------------------------------------*/
void ProcessNewRxMessage(sUartMessage* msg, uint8_t *gRawMsgForEcho, uint32_t rawMessageLen);
void CheckHWAndGenerateEventsAsNeeded();
void HandleStatusSend();
/* Carbonation Time Table ----------------------------------------------------*/
//uint16_t gCarbTimeTable[eLevel_number_of_levels*2][eCycle_number_of_cycles][MAX_NUMBER_OF_CARBONATION_STEPS] = {
//// --- 1 Litter bottle size ---------------------------------------------
////		eLevel_Low,
//				{ /* ON */{ 700, 1000, 800, 0, 0, 0, 0, 0, },
//				/* OFF */{ 500, 500, 2000, 0, 0, 0, 0, 0, } },
////		eLevel_medium,
//				{ /* ON */{ 700, 1000, 1000, 1000, 1000, 0, 0, 0, },
//				/* OFF */{ 500, 500, 500, 500, 2000, 0, 0, 0, } },
////		eLevel_high,
//				{ /* ON */{ 700, 1000, 1000, 1250, 1250, 1000, 0, 0, },
//				/* OFF */{ 500, 500, 500, 500, 500, 2000, 0, 0, } },
//// --- 0.5 Litter bottle size ---------------------------------------------
////		eLevel_Low,
//				{ /* ON */{ 700, 1000, 0, 0, 0, 0, 0, 0, },
//				/* OFF */{ 500, 500, 0, 0, 0, 0, 0, 0, } },
////		eLevel_medium,
//				{ /* ON */{ 700, 1000, 1000, 0, 0, 0, 0, 0, },
//				/* OFF */{ 500, 500, 500, 0, 0, 0, 0, 0, } },
////		eLevel_high,
//				{ /* ON */{ 700, 1000, 1000, 1250, 1250, 0, 0, 0, },
//				/* OFF */{ 500, 500, 500, 500, 500, 0, 0, 0, } },
//		};

uint16_t gCarbTimeTable[eLevel_number_of_levels*2][eCycle_number_of_cycles][MAX_NUMBER_OF_CARBONATION_STEPS] = {
// --- 1 Litter bottle size ---------------------------------------------
//      eLevel_Low,
                { /* ON */{ 700, 1000, 0, 0, 0, 0, 0, 0, },
                /* OFF */{ 500, 500, 0, 0, 0, 0, 0, 0, } },
//      eLevel_medium,
                { /* ON */{ 700, 1000, 1000, 0, 0, 0, 0, 0, },
                /* OFF */{ 500, 500, 500, 0, 0, 0, 0, 0, } },
//      eLevel_high,
                { /* ON */{ 700, 1000, 1000, 1250, 0, 0, 0, 0, },
                /* OFF */{ 500, 500, 500, 500, 500, 0, 0, 0, } },
// --- 0.5 Litter bottle size ---------------------------------------------
//      eLevel_Low,
                { /* ON */{ 700, 1000, 0, 0, 0, 0, 0, 0, },
                /* OFF */{ 500, 500, 0, 0, 0, 0, 0, 0, } },
//      eLevel_medium,
                { /* ON */{ 700, 1000, 1000, 0, 0, 0, 0, 0, },
                /* OFF */{ 500, 500, 500, 0, 0, 0, 0, 0, } },
//      eLevel_high,
                { /* ON */{ 700, 1000, 1000, 1250, 0, 0, 0, 0, },
                /* OFF */{ 500, 500, 500, 500, 500, 0, 0, 0, } },
        };
/* Private user code ---------------------------------------------------------*/
/**
 * @brief  Function implementing the Main Logic Task main loop
 * @param  None
 * @retval None
 */
void MainLogicInit(void) {

#ifndef DEBUG_NO_SAFETY
    PURE_STL_Init();
#endif

    // Initialize the RBMEM Storage
    gCarbonationLevel = (eCarbonationLevel)RBMEM_Data_Init();
    gPrevCarbonationLevel = gCarbonationLevel;

	// Initialize the state machine
	SMSodaStreamPure_ctor(&gStateMachine);
	SMSodaStreamPure_start(&gStateMachine);


	// Initialize the Accelerometer
	AccelerometerInit();
	gAccelerometerIsPresent = AccelerometerIsPresent();


	// Pure VDL LEds
	WS_InitLeds();

#ifndef DEBUG_NO_SAFETY
    STL_InitRunTimeChecks();
#endif
	// starts the main logic timer
	HAL_TIM_Base_Start_IT(&htim14);

}

#ifdef DEBUG_STATE_MACHINE
SMSodaStreamPure_StateId dbgCurrentState = SMSodaStreamPure_StateId_ROOT;
SMSodaStreamPure_StateId dbgNewState = SMSodaStreamPure_StateId_ROOT;
#endif

uint8_t mLedsp[NUMBER_OF_LEDS] = {0};

//	WS_SetLeds(mLedsp, 9); // TODO debug remove

void MainLogicPeriodic() {

    // periodic reset watchdog
#ifndef DEBUG_NO_SAFETY
    HAL_IWDG_Refresh(&hiwdg);
#endif

	if (gFirstTime)
	{
#ifndef DEBUG_NO_SAFETY
        STL_FlashCrc32Init();
#endif

		gFirstTime = false;
		// Start reading from the UART
		COMM_UART_StartRx();
		// on powerup, send the version number
		glb_last_RxMessage.cmd = eUARTCommand_rver;
		ProcessNewRxMessage(&glb_last_RxMessage, gRawMsgForEcho, gRawMessageLen);
		StartADCConversion();

	//	StartAnimation(eAnimation_MakeADrinkProgress);
	}
#ifndef DEBUG_NO_SAFETY
    // Periodic safety check
    STL_DoRunTimeChecks();
#endif

	PlayLedsPeriodic();
	CheckLongPressButtonsPeriodic();

	CheckHWAndGenerateEventsAsNeeded();

	// optional: dispatch DO every tick
	SMSodaStreamPure_dispatch_event(&gStateMachine, SMSodaStreamPure_EventId_DO);
#ifdef DEBUG_STATE_MACHINE
	// DEBUG REMOVE
	if (dbgCurrentState != gStateMachine.state_id)
	{
		uint8_t msg_len = (uint8_t)BuildReply((char*)gRawMsgForEcho, eUARTCommand_dbug, (uint32_t[]){dbgCurrentState, 0, gStateMachine.state_id}, 3,false);
		dbgCurrentState = gStateMachine.state_id;
		COMM_UART_QueueTxMessage(gRawMsgForEcho, msg_len);
        if (SMSodaStreamPure_StateId_STATE_SHOWSTATUS == dbgCurrentState)
        {
            glb_last_RxMessage.cmd = eUARTCommand_flsc;
            ProcessNewRxMessage(&glb_last_RxMessage, gRawMsgForEcho, gRawMessageLen);
        }
	}
	// DEBUG REMOVE
#endif

	// dispatch queued events from the ring buffer (if any)
	SMSodaStreamPure_EventId ev;
	while (SMEventQueue_Take(&ev)) {
		SMSodaStreamPure_dispatch_event(&gStateMachine, ev);
#ifdef DEBUG_STATE_MACHINE
		// DEBUG REMOVE
		if ((dbgCurrentState != gStateMachine.state_id) || (ev != SMSodaStreamPure_EventId_DO))
		{
			uint8_t msg_len = (uint8_t)BuildReply((char*)gRawMsgForEcho, eUARTCommand_dbug, (uint32_t[]){dbgCurrentState, ev, gStateMachine.state_id}, 3, false);
			dbgCurrentState = gStateMachine.state_id;
			COMM_UART_QueueTxMessage(gRawMsgForEcho, msg_len);
	        if (SMSodaStreamPure_StateId_STATE_SHOWSTATUS == dbgCurrentState)
	        {
		        glb_last_RxMessage.cmd = eUARTCommand_flsc;
		        ProcessNewRxMessage(&glb_last_RxMessage, gRawMsgForEcho, gRawMessageLen);
			}
		}
		// DEBUG REMOVE
#endif
	}

	// process all pending commands
	while(eUART_MesssagePending == (glb_new_msg = COMM_UART_CheckNewMessage(&glb_last_RxMessage, gRawMsgForEcho, &gRawMessageLen))) {
		ProcessNewRxMessage(&glb_last_RxMessage, gRawMsgForEcho, gRawMessageLen);
	}

	// Send status as needed
	HandleStatusSend();
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
	bool illegalCommand = false;
	uint8_t msg_len = 0;
    uint32_t value32 = 0; // general purpose 32 bits value
    uint32_t value32a = 0; // general purpose 32 bits value
	/*
	eUARTCommand_rsts,
	 *
	 */
	switch (msg->cmd)
	{
	case eUARTCommand_rver:
		msg_len = (uint8_t)BuildReply((char*)gRawMsgForEcho, eUARTCommand_rver, (uint32_t[]){SWVERSION_MAJOR, SWVERSION_MINOR}, 2, false);
		COMM_UART_QueueTxMessage(gRawMsgForEcho, msg_len);
		echoCommand = false;
		break;
	case eUARTCommand_mgui:
		gIsGuiControlMode = (msg->params.onOff.isOn == 1);
		if (gIsGuiControlMode) {
			StopWaterPump();
			StopUVLed();
		}
		if (! SMEventQueue_Add(gIsGuiControlMode? SMSodaStreamPure_EventId_EVENT_ENTER_GUI_CONTROLLED_MODE : SMSodaStreamPure_EventId_EVENT_EXIT_GUI_CONTROLLED_MODE))
			gQueueErrors++;
		break;
	case eUARTCommand_powr:
		if (gIsGuiControlMode){
			SolenoidPumpUVPower(msg->params.onOff.isOn);
		} else {
			illegalCommand = true;
		}
		break;
	case eUARTCommand_sled:
//		if (gLP5009InitOK)
//		{
//			if ((msg->params.sled.ledNumber < 1) || (msg->params.sled.ledNumber > 5) || (msg->params.sled.intensity > 100))
//			{
//				illegalCommand = true;
//				break;
//			}
//			LP5009_SetLed(&hi2c1, (uint8_t)(msg->params.sled.ledNumber - 1), 100 - (uint8_t)(msg->params.sled.intensity));
//		}

		if ((msg->params.sled.ledNumber < 1) || (msg->params.sled.ledNumber > NUMBER_OF_LEDS) || (msg->params.sled.intensity > 100))
		{
			illegalCommand = true;
			break;
		}
		mLedsp[msg->params.sled.ledNumber - 1] = msg->params.sled.intensity * 255  / 100 ;
		WS_SetLeds(mLedsp, NUMBER_OF_LEDS);
		break;
	case eUARTCommand_srgb:
//		if (gLP5009InitOK) {
//			if ((msg->params.srgb.valueR > 255) ||(msg->params.srgb.valueG > 255) || (msg->params.srgb.valueB > 255)) {
//				illegalCommand = true;
//				break;
//			}
//			if ((msg->params.srgb.valueR == 0) && (msg->params.srgb.valueG == 0) && (msg->params.srgb.valueB == 0)) {
//				LP5009_SetLedOff(&hi2c1);
//				//LP5009_RGB_Off(&hi2c1);
//			} else {
//				LP5009_RGB_EnableGroups(&hi2c1);
//
//				// Blue and Red switched
//				LP5009_RGB(&hi2c1,(uint8_t)(msg->params.srgb.valueB),(uint8_t)(msg->params.srgb.valueG),(uint8_t)(msg->params.srgb.valueR));
//				// // Red and green are reverese
//				// LP5009_RGB(&hi2c1,(uint8_t)(msg->params.srgb.valueG),(uint8_t)(msg->params.srgb.valueR),(uint8_t)(msg->params.srgb.valueB));
//			}
//
//		}
		break;
	case eUARTCommand_pump:
		if (gIsGuiControlMode){
			echoCommand = false; // done here since might need to send done message after it
			COMM_UART_QueueTxMessage(gRawMsgForEcho, rawMessageLen);

			if (msg->params.pump.isOn == 1) {
				gStateMachine.vars.pumpStopsOnSensor = (msg->params.pump.sensorThreashold != 0);
				StartWaterPump();
			} else {
				StopWaterPump();
				SendDonePumpOK();
			}
		} else {
			illegalCommand = true;
		}
		break;
	case eUARTCommand_carb:
		if (gIsGuiControlMode){
			SolenoidPump(msg->params.onOff.isOn);
		} else {
			illegalCommand = true;
		}
		break;
	case eUARTCommand_stop:
		if (gIsGuiControlMode){
			echoCommand = false; // done here since might need to send done message after it
			COMM_UART_QueueTxMessage(gRawMsgForEcho, rawMessageLen);

			if (gStateMachine.vars.pumpStopsOnSensor) {
				gStateMachine.vars.pumpStopsOnSensor = false;
				SendDoneMessage(eDone_OK);
			}
			SolenoidPump(0);
			SolenoidPumpUVPower(0);
			StopWaterPump();
		} else {
			illegalCommand = true;
		}
		break;
	case eUARTCommand_tilt: // Get Info - non state machine related command
		msg_len = (uint8_t)BuildReplySigned((char*)gRawMsgForEcho, eUARTCommand_tilt, (int32_t[]){filtered_x, filtered_y, filtered_z}, 3, false);
		COMM_UART_QueueTxMessage(gRawMsgForEcho, msg_len);
		echoCommand = false;
		break;
	case eUARTCommand_wtrs: // Get Info - non state machine related command
		msg_len = (uint8_t)BuildReply((char*)gRawMsgForEcho, eUARTCommand_wtrs, (uint32_t[]){gReadWaterLevelADC}, 1, false);
		COMM_UART_QueueTxMessage(gRawMsgForEcho, msg_len);
		echoCommand = false;
		break;
	case eUARTCommand_uvld:
		if (gIsGuiControlMode){
			gIsUVLEdOn = (msg->params.onOff.isOn == 1);
			if (gIsUVLEdOn) {
				StartUVLEd();
			} else {
				StopUVLed();
			}
		} else {
			illegalCommand = true;
		}
		break;
	case eUARTCommand_uvla: // Get Info - non state machine related command
		msg_len = (uint8_t)BuildReply((char*)gRawMsgForEcho, eUARTCommand_uvla, (uint32_t[]){gReadUVCurrentADC}, 1, false);
		COMM_UART_QueueTxMessage(gRawMsgForEcho, msg_len);
		echoCommand = false;
		break;
	case eUARTCommand_pmpa: // Get Info - non state machine related command
		msg_len = (uint8_t)BuildReply((char*)gRawMsgForEcho, eUARTCommand_pmpa, (uint32_t[]){gReadWaterPumpCurrentADC}, 1, false);
		COMM_UART_QueueTxMessage(gRawMsgForEcho, msg_len);
		echoCommand = false;
		break;
	case eUARTCommand_rrtc: // Get Info - non state machine related command
	{
	    extern RTC_HandleTypeDef hrtc;
	    RTC_TimeTypeDef sTime;
	    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
		msg_len = (uint8_t)BuildReply((char*)gRawMsgForEcho, eUARTCommand_rrtc, (uint32_t[]){GetDaysSinceFilterReplacement(),
		sTime.Hours*3600+sTime.Minutes*60+sTime.Seconds}, 2, false);
		COMM_UART_QueueTxMessage(gRawMsgForEcho, msg_len);
		echoCommand = false;
		break;
	}
	case eUARTCommand_rsts:
		gPeriodicStatusSendInterval = msg->params.periodicStatus.periodicStatusInterval;
		if ((gPeriodicStatusSendMask == 0) && (msg->params.periodicStatus.periodicStatusMask != 0))
		{
			// just started - set the start time
			gPeriodicStatusSendLastTickSent = HAL_GetTick();
		}
		gPeriodicStatusSendMask = msg->params.periodicStatus.periodicStatusMask;
		break;

	case eUARTCommand_stbl:
		{
			echoCommand = false;
			// msg->params.list[0] holds the row ID (1 to 12)
			// msg->params.list[1..8] - the values
			if ((msg->params.list[0] < 1) || (msg->params.list[0] > 12))
			{
				illegalCommand = true;
				break;
			}
			int level = (msg->params.list[0] - 1) / 2; // maps 1,2,3,4,5,6,7,8,9,10,11,12 -> 0,0,1,1,2,2,3,3,4,4,5,5
			int onOff = (msg->params.list[0] - 1) % 2; // maps 1,2,3,4,5,6 -> 0,1,0,1,0,1
			for (int i = 0; i < MAX_NUMBER_OF_CARBONATION_STEPS; i++)
			{
				gCarbTimeTable[level][onOff][i] = msg->params.list[i+1];
			}
			msg_len = (uint8_t)BuildReply((char*)gRawMsgForEcho, eUARTCommand_stbl, (uint32_t[]){msg->params.list[0]}, 1, true);
			COMM_UART_QueueTxMessage(gRawMsgForEcho, msg_len);
		}
		break;
	case eUARTCommand_conf:
		switch (msg->params.config.configurationParamID)
		{
		case 1:
			gBottleSizeThresholdmSecs = msg->params.config.value;
			break;
		case 2:
			gPumpTimoutMsecs = msg->params.config.value;
			break;
		case 3:
			gWaterLevelSensorThreahsold = msg->params.config.value;
			break;
		}
		break;
	case eUARTCommand_anim:
	    if (! gIsGuiControlMode) {
            illegalCommand = true;
            break;
        }
	    if (msg->params.animation.isStart == 1) {
            StartAnimation((eAnimations)msg->params.animation.animationNum, msg->params.animation.forceStopPrev == 1);
	    } else {
	        StopCurrentAnimation(msg->params.animation.animationNum == 1);
	    }
	    break;
	case eUARTCommand_swsp:
		if (! gIsGuiControlMode)
		{
			illegalCommand = true;
			break;
		}
		WaterPumpSensor((int)(msg->params.onOff.isOn));
		break;
	case eUARTCommand_lptm:
		msg_len = (uint8_t)BuildReply((char*)gRawMsgForEcho, eUARTCommand_lptm, (uint32_t[]){gLastPumpTimeMSecs}, 1, false);
		COMM_UART_QueueTxMessage(gRawMsgForEcho, msg_len);
		echoCommand = false;
		break;
	case eUARTCommand_fday:
	    if (msg->params.fday.isSet == 1) {
            // set
	        SetDaysSinceLastFilterReplacement(msg->params.fday.days);
        } else {
            // get days since last filter replacemeent
            msg_len = (uint8_t)BuildReply((char*)gRawMsgForEcho, eUARTCommand_fday, (uint32_t[]){FILTER_LIFETIME_DAYS - GetFilterDaysLeft(),FILTER_LIFETIME_DAYS,FILTER_WARNING_DAYS}, 3, false);
            COMM_UART_QueueTxMessage(gRawMsgForEcho, msg_len);
            echoCommand = false;
        }
        break;
	case eUARTCommand_csec:
        if (msg->params.csec.isSet == 1) {
            // set
            RBMEM_WriteElement(eRBMEM_total_CO2_msecs_used, msg->params.csec.secs * 1000);
        } else {
            // get seconds since last CO2 replacement
            RBMEM_ReadElement(eRBMEM_total_CO2_msecs_used,&value32);
            RBMEM_ReadElement(eRBMEM_total_CO2_msecs_max,&value32a);
            msg_len = (uint8_t)BuildReply((char*)gRawMsgForEcho, eUARTCommand_csec, (uint32_t[]){(value32+500)/1000,value32a/1000}, 2, false);
            COMM_UART_QueueTxMessage(gRawMsgForEcho, msg_len);
            echoCommand = false;
        }
        break;
    case eUARTCommand_cmax:
        RBMEM_WriteElement(eRBMEM_total_CO2_msecs_max, msg->params.cmax.secs * 1000);
        break;
    case eUARTCommand_fmem:
        if (msg->params.fmem.isSet == 1) {
            if (msg->params.fmem.id == 99) {
                RBMEM_ResetDataToDefaults();
                NVIC_SystemReset();
            } else {
                // set
                RBMEM_WriteElement(msg->params.fmem.id,
                        (uint32_t) (msg->params.fmem.value));
            }
        } else {
            // get
            RBMEM_ReadElement(msg->params.fmem.id, &value32);
            msg_len = (uint8_t)BuildReply((char*)gRawMsgForEcho, eUARTCommand_fmem, (uint32_t[]){msg->params.fmem.id, value32}, 2, false);
            COMM_UART_QueueTxMessage(gRawMsgForEcho, msg_len);
            echoCommand = false;
        }
        break;
    case eUARTCommand_flsc:
        msg_len = (uint8_t)BuildReply((char*)gRawMsgForEcho, eUARTCommand_flsc, (uint32_t[]){(uint32_t)gFalseButMainCounter,(uint32_t)gFalseButCarbLevelCounter,(uint32_t)gFalseButFilterCounter}, 3, false);
        COMM_UART_QueueTxMessage(gRawMsgForEcho, msg_len);
        msg_len = (uint8_t)BuildReply((char*)gRawMsgForEcho, eUARTCommand_flsc, (uint32_t[]){(uint32_t)gKeyPressButMainMS,(uint32_t)gKeyPressButCarbLevelMS,(uint32_t)gKeyPressButFilterMS}, 3, false);
        COMM_UART_QueueTxMessage(gRawMsgForEcho, msg_len);
        echoCommand = false;
        break;

	default:
	}
	if (illegalCommand) {
		TxIllegalCommandResponse();
	} else if (echoCommand)
	{
		COMM_UART_QueueTxMessage(gRawMsgForEcho, rawMessageLen);
	}
}

void HandleStatusSend()
{
	if (gIsGuiControlMode && (gPeriodicStatusSendMask != 0)) // send is enabled
	{
		// check to need to send now
		if (gPeriodicStatusSendLastTickSent + gPeriodicStatusSendInterval < HAL_GetTick())
		{
			// sending now
			gPeriodicStatusSendLastTickSent = HAL_GetTick();

			// send messages by mask
			if (gPeriodicStatusSendMask & PERIODIC_STATUS_SEND_MASK_STATEBUTS)
			{
				uint8_t msg_len = (uint8_t)BuildReply((char*)gRawMsgForEcho,
					eUARTCommand_rsts,
					(uint32_t[]){PERIODIC_STATUS_SEND_MASK_STATEBUTS,
					1,  // TODO add error states instead of 1
					(HAL_GPIO_ReadPin(BTN1_GPIO_Port, BTN1_Pin) == GPIO_PIN_RESET) ? 1 : 0,
					(HAL_GPIO_ReadPin(BTN2_GPIO_Port, BTN2_Pin) == GPIO_PIN_RESET) ? 1 : 0,
					(HAL_GPIO_ReadPin(BTN3_GPIO_Port, BTN3_Pin) == GPIO_PIN_RESET) ? 1 : 0,
					(HAL_GPIO_ReadPin(BTN4_GPIO_Port, BTN4_Pin) == GPIO_PIN_RESET) ? 1 : 0},
					6,
					false);
				COMM_UART_QueueTxMessage(gRawMsgForEcho, msg_len);
			}
			if (gPeriodicStatusSendMask & PERIODIC_STATUS_SEND_MASK_ADC)
			{
				uint8_t msg_len = (uint8_t)BuildReply((char*)gRawMsgForEcho,
					eUARTCommand_rsts,
					(uint32_t[]){PERIODIC_STATUS_SEND_MASK_ADC,gReadWaterLevelADC,gReadUVCurrentADC,gReadWaterPumpCurrentADC},
					4,
					false);
				COMM_UART_QueueTxMessage(gRawMsgForEcho, msg_len);
			}
			if (gPeriodicStatusSendMask & PERIODIC_STATUS_SEND_MASK_RTCTILT)
			{
				uint8_t msg_len = (uint8_t)BuildReplySigned((char*)gRawMsgForEcho,
					eUARTCommand_rsts,
					(int32_t[]){PERIODIC_STATUS_SEND_MASK_RTCTILT,
						(int32_t)GetDaysSinceFilterReplacement(),
						filtered_x,filtered_y,filtered_z},
					5,
					false);
				COMM_UART_QueueTxMessage(gRawMsgForEcho, msg_len);
			}
			if (gPeriodicStatusSendMask & PERIODIC_STATUS_SEND_MASK_LEDS)
			{
				// TODO not implemented yet
			}
		}
	}
}

void SendDoneMessage(eDoneResults result)
{
	uint8_t msg_len = (uint8_t)BuildReply((char*)gRawMsgForEcho,
	        eUARTCommand_done,
		(uint32_t[]){result},
		1,
		false);
	COMM_UART_QueueTxMessage(gRawMsgForEcho, msg_len);
}

//extern UART_HandleTypeDef huart2;
//bool prevTilted = false;
void CheckHWAndGenerateEventsAsNeeded()
{

	// check for tilt and set event if needed
	if (gAccelerometerIsPresent && (HAL_GetTick() > 100)) // wait 100 msecs after powerup
	{
		gIsTilted = IsSlanted();

//		// DEBUG REMOVE
//		if (prevTilted != gIsTilted)
//		{
//		    prevTilted = gIsTilted;
//		    HAL_UART_Transmit_DMA(&huart2, (const uint8_t *)(gIsTilted ? "not tilted->TILTED\r\n" : "TILTED->not tilted\r\n") , 20);
//		}
//		// DEBUG REMOVE

	}
	//if (gWaterLevelIsActive)
//	{
//
//	}
}

//void DBGSendMessage(char *msg)
//{
//	sprintf((char *)gRawMsgForEcho, "$%s\r\n",msg);
//	COMM_UART_QueueTxMessage(gRawMsgForEcho, strlen((const char *)gRawMsgForEcho));
//}
void PURE_STL_Init(void)
{
//  uint32_t crc_result;
//  uint32_t index;
//  ClockStatus clk_sts;

  /* block run time tests performed at SysTick interrupt */
  TickCounter= TickCounterInv= 0;

//  /*--------------------------------------------------------------------------*/
//  /*--------------------- Invariable memory CRC check ------------------------*/
//  /*--------------------------------------------------------------------------*/
//
//  control_flow_call(CRC32_TEST_CALLER);
//  /* Compute the 32-bit crc of the whole Flash by CRC unit except the checksum
//     pattern stored at top of FLASH */
//
//  __CRC_CLK_ENABLE();
//
//  CrcHandle.Instance = CRC;
//  CrcHandle.State = HAL_CRC_STATE_RESET;
//    CrcHandle.Init.DefaultPolynomialUse    = DEFAULT_POLYNOMIAL_ENABLE;
//    CrcHandle.Init.DefaultInitValueUse     = DEFAULT_INIT_VALUE_ENABLE;
//    CrcHandle.Init.InputDataInversionMode  = CRC_INPUTDATA_INVERSION_NONE;
//    CrcHandle.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_DISABLED;
//    CrcHandle.InputDataFormat              = CRC_INPUTDATA_FORMAT_WORDS;
//  HAL_CRC_Init(&CrcHandle);
//
//    /* the next for cycle replaces the standard HAL function call
//       crc_result = HAL_CRC_Calculate(&CrcHandle, (uint32_t *)ROM_START, (uint32_t)ROM_SIZEinWORDS);
//       due to bug at IAR linker - check sum computation can't support both big & little endian  */
//
//    for(index = 0; index < (uint32_t)ROM_SIZEinWORDS; index++)
//    {
//      CRC->DR = __REV(*((uint32_t *)ROM_START + index));
//    }
//    crc_result = CRC->DR;
//
//  control_flow_resume(CRC32_TEST_CALLER);
//
//  /* Store pattern for regular 32-bit crc computation */
//  control_flow_call(CRC_TEST_CALLER);
//  /* ==============================================================================*/
//  /* MISRA violation of rule 10.1, 11.3 and 11.4: integral casting and pointer arithmetic
//     is used here to manage the crc computation and Check Class B var integrity */
//
//  /* ==============================================================================*/
//
//    /* Computed 32-bit crc check is temporary stored at crc_result. This value must be copied
//       into __Check_Sum address placed at the end of Flash area (see file startup_stm32xx.s)
//       Condition here can be reversed for debugging */
//
//    if(crc_result != *(uint32_t *)(&REF_CRC32))
//    {
//      FailSafePOR();
//    }
//    else
//    { /* Test OK */
//
//      /* If else statement is not executed, it will be detected by control flow monitoring */
//      control_flow_resume(CRC_TEST_CALLER);
//    }
//
//  HAL_CRC_DeInit(&CrcHandle);
//
//
//  /*--------------------------------------------------------------------------*/
//  /* --------------------- Variable memory functional test -------------------*/
//  /*--------------------------------------------------------------------------*/
//
//  /* no stack operation can be performed during the test */
//  __disable_irq();
//
//  /* WARNING: The stack is initialized into background pattern when exiting from this routine */
//  if (STL_FullRamMarchC(RAM_START, RAM_END, BCKGRND, RAM_BCKUP) != SRAMTEST_SUCCESS)
//  {
//    FailSafePOR();
//  }
//
//  /* restore interrupt capability */
//  __enable_irq();
//
//
//  /* Initialization of counters for control flow monitoring
//     (destroyed during RAM test) */
//  init_control_flow();

  /*--------------------------------------------------------------------------*/
  /*------------- Store reference 32-bit CRC in RAM after RAM test -----------*/
  /*--------------------------------------------------------------------------*/
  control_flow_call(CRC_STORE_CALLER);

  /* restore destroyed content of HAL structure for CRC */
  CrcHandle.Instance = CRC;
  CrcHandle.State= HAL_CRC_STATE_READY;

  /* read and store content of CRC calculation result */
  RefCrc32 = HAL_CRC_Accumulate(&CrcHandle, 0u, 0u);
  RefCrc32Inv = ~RefCrc32;

  control_flow_resume(CRC_STORE_CALLER);

//  /*--------------------------------------------------------------------------*/
//  /*----------------------- Clock Frequency Self Test ------------------------*/
//  /*--------------------------------------------------------------------------*/
//  control_flow_call(CLOCK_TEST_CALLER);
//
//  /* refresh HAL static variables keeping system tick frequency value */
//  uwTickFreq = HAL_TICK_FREQ_DEFAULT;
//  uwTickPrio = TICK_INT_PRIORITY;
//  HAL_SetTickFreq(HAL_TICK_FREQ_DEFAULT);
//
//  /* test LSI & HSE clock systems */
//  clk_sts = STL_ClockStartUpTest();
//
//
//  if(clk_sts != FREQ_OK)
//  {
//      FailSafePOR();
//  }
//
//  control_flow_resume(CLOCK_TEST_CALLER);
//
//
//  /*--------------------------------------------------------------------------*/
//  /* --------------- Initialize stack overflow pattern ---------------------- */
//  /*--------------------------------------------------------------------------*/

  control_flow_call(STACK_OVERFLOW_TEST);

  aStackOverFlowPtrn[0] = 0xEEEEEEEEuL;
  aStackOverFlowPtrn[1] = 0xCCCCCCCCuL;
  aStackOverFlowPtrn[2] = 0xBBBBBBBBuL;
  aStackOverFlowPtrn[3] = 0xDDDDDDDDuL;

//  control_flow_resume(STACK_OVERFLOW_TEST);
//
//  /*--------------------------------------------------------------------------*/
//  /* -----  Verify Control flow before Starting main program execution ------ */
//  /*--------------------------------------------------------------------------*/
//
//  if (control_flow_check_point(CHECKPOINT2) == ERROR)
//  {
//     FailSafePOR();
//  }
//
//  /* startup test completed successfully - restart the application */
//  GotoCompilerStartUp();

}
void StartUpClock_Config() {

}

void Pure_STL_SysTick_Handler(void) {
    /* Verify TickCounter integrity */
    if ((TickCounter ^ TickCounterInv) == 0xFFFFFFFFuL) {
        TickCounter++;
        TickCounterInv = ~TickCounter;

        if (TickCounter >= SYSTICK_10ms_TB) {
            uint32_t RamTestResult;

            /* Reset timebase counter */
            TickCounter = 0u;
            TickCounterInv = 0xFFFFFFFFuL;

            /* Set Flag read in main loop */
            TimeBaseFlag = 0xAAAAAAAAuL;
            TimeBaseFlagInv = 0x55555555uL;

            ISRCtrlFlowCnt += RAM_MARCHC_ISR_CALLER;
            __disable_irq();
            RamTestResult = STL_TranspMarch();
            __enable_irq();
            ISRCtrlFlowCntInv -= RAM_MARCHC_ISR_CALLER;

            switch (RamTestResult) {
            case TEST_RUNNING:
                break;
            case TEST_OK:
                break;
            case TEST_FAILURE:
            case CLASS_B_DATA_FAIL:
            default:
                FailSafePOR();
                break;
            } /* End of the switch */

            /* Do we reached the end of RAM test? */
            /* Verify 1st ISRCtrlFlowCnt integrity */
            if ((ISRCtrlFlowCnt ^ ISRCtrlFlowCntInv) == 0xFFFFFFFFuL) {
                if (RamTestResult == TEST_OK) {
                    /* ==============================================================================*/
                    /* MISRA violation of rule 17.4 - pointer arithmetic is used to check RAM test control flow */
                    if (ISRCtrlFlowCnt != RAM_TEST_COMPLETED)
                    /* ==============================================================================*/
                    {
                        FailSafePOR();
                    } else /* Full RAM was scanned */
                    {
                        ISRCtrlFlowCnt = 0u;
                        ISRCtrlFlowCntInv = 0xFFFFFFFFuL;
                    }
                } /* End of RAM completed if*/
            } /* End of control flow monitoring */
            else {
                FailSafePOR();
            }
        } /* End of the 10 ms timebase interrupt */
    }

    /* USER CODE END SysTick_IRQn 1 */
}

