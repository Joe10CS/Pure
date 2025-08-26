/**
  ******************************************************************************
  * @file           : RxTxMsgs.h
  * @brief          : Header for CDM UART messages processing
   ******************************************************************************
  */
#ifndef INC_RXTXMSGS_H_
#define INC_RXTXMSGS_H_

/* defines ------------------------------------------------------------------*/
#define TX_BUFFER_PTR			txBuffer

#define MAX_TX_BUFFER_LEN       (30)  // longest message can be "$RVER version 111.000\r\n"
#define MAX_TX_QUEUE_SIZE       (20)

#define RX_BUFFER_PTR			rxBuffer
#define MAX_RX_BUFFER_LEN       (256)
#define MAX_TMP_MSG_BUFFER_LEN  (256)

#define MAX_RX_RETRIES 			(3)

#define MAX_TX_TIMEOUT_MSECS	(5000)

#define MAX_COMMAND_LEN        (4)
#define COMMAND_START_SYMBOL   ('$')
#define COMMAND_PRE_END_MARKER ('\r')
#define COMMAND_END_MARKER     ('\n')

#define MAX_NUMBER_OF_PARAMETERS (9)

/* enums ---------------------------------------------------------------------*/
typedef enum {
	eUART_ERROR = -1,
	eUART_OK = 0, // TODO: needed ?
	eUART_NoMessage = 1,
    eUART_MesssagePending = 2,
} eUartStatus;

typedef enum {
	eUartDoneStatus_OK = 0,
	eUartDoneStatus_First_Pressure_Switch_Timeout = 100,
	eUartDoneStatus_Pressure_Found_Before_Carbonation_Started = 101,
	eUartDoneStatus_Pump_Failed = 102,
	eUartDoneStatus_Total_Pressure_Switch_Cycles_Timeout = 110,
	eUartDoneStatus_Engine_Error = 200,
}eUartDoneStatus;

// commands from PC
typedef enum {

	eUARTCommand_mgui,
	eUARTCommand_powr,
	eUARTCommand_sled,
	eUARTCommand_srgb,
	eUARTCommand_pump,
	eUARTCommand_carb,
	eUARTCommand_stop,
	eUARTCommand_tilt,
	eUARTCommand_wtrs,
	eUARTCommand_uvld,
	eUARTCommand_uvla,
	eUARTCommand_pmpa,
	eUARTCommand_rrtc,
	eUARTCommand_rsts,
	eUARTCommand_rver,
	eUARTCommand_stbl,
	eUARTCommand_conf,
	eUARTCommand_swsp,
	eUARTCommand_lptm,
	eUARTCommand_done, // just for replay
	eUARTCommand_dbug, // debug messages


	eUARTCommand_num_commands,
	//eUARTCommand_motor_current_beyond_threshold = 55,
	eUARTCommand_none = 99,
}eUARTCommandTypes;

/* types ---------------------------------------------------------------------*/
typedef struct {
	uint8_t len; // size following the $ sign
	char command[MAX_COMMAND_LEN];
	uint8_t num_params;
} sCommandDef;

typedef struct {
	uint16_t isOn;
} sOnOffParams;

typedef struct {
	uint16_t isOn;
	uint16_t sensorThreashold; // 0 – ignore water sensor, N – automatically stop on sensor value above N
} sPumpParams;

typedef struct {
	uint16_t periodicStatusMask;     // Mask of messages to send
	uint16_t periodicStatusInterval;
} sPeriodicStatusParams;

typedef struct {
	uint16_t configurationParamID;     // Mask of messages to send
	uint32_t value;
} sConfigureParams;

typedef struct {
	uint16_t valueR;
	uint16_t valueG;
	uint16_t valueB;
} sSetRGBLEdParams;
typedef struct {
	uint16_t ledNumber;
	uint16_t intensity;
} sSetLEdParams;


// !@#!@#  CDM stuff below
typedef struct {
	uint16_t cycleTime;
	uint16_t numberOfCycles;;
	uint16_t firstPSTimeout;
	uint16_t motorSpeedPercent;
	uint16_t numberOfPSCycles;
	uint16_t PSTotlaCyclesTimeout;
} sCarbParams;

typedef struct {
	uint16_t motorDiection;
	uint16_t motorSpeedPercent;
	uint16_t motorTime;
} sStartParams;

typedef struct {
	uint16_t numOfInjectSignals;
	uint16_t motorSpeedPercent;
} sInjectSyrupParams;

typedef struct {
	uint16_t motorSpeedPercent;
	uint16_t homePinState;
} sHomeParams;

typedef union {
	sOnOffParams onOff;
	sPumpParams pump;
	sPeriodicStatusParams periodicStatus;
	uint16_t list[MAX_NUMBER_OF_PARAMETERS];
	sConfigureParams config;
	sSetRGBLEdParams srgb;
	sSetLEdParams sled;

	// !@#!@#  CDM stuff below

    sCarbParams carbParams;
    sStartParams startParams;
    sInjectSyrupParams injectSytupParams;
    sHomeParams homeParams;
} uParams;

typedef struct {
	eUARTCommandTypes cmd;
	uParams params;
} sUartMessage;
/* variables -----------------------------------------------------------------*/
extern eUARTCommandTypes glb_last_msg_type;
extern sUartMessage glb_last_RxMessage;
extern sCommandDef gCDMCommands[eUARTCommand_num_commands];
/* functions ----------------------------------------------------------------*/

void COMM_UART_StartRx();
void COMM_UART_ClearRxBytes();
eUartStatus COMM_UART_CheckNewMessage(sUartMessage *newMsg, uint8_t* rawMsgPtr, uint32_t *rawMsgLen);
//uint16_t COMM_UART_PrepareTXMessage(uint8_t *before, uint8_t *after, uint16_t before_size);
bool COMM_UART_QueueTxMessage(uint8_t *msg, uint32_t msgLen);
void TxIllegalCommandResponse();
void COMM_UART_SendNextTxQueue(void);
void SendMessageToCDMDesktop(eUARTCommandTypes msgType, uint16_t value1, uint16_t value2);
void CDMSendDoneMessage(eUartDoneStatus status);
size_t BuildReply(char *dst, const eUARTCommandTypes cmd_id, const uint32_t *nums, uint8_t nums_count, bool ok_suffix);
size_t BuildReplySigned(char *dst, const eUARTCommandTypes cmd_id, const int32_t *nums, uint8_t nums_count, bool ok_suffix);
char* u32_to_str(char *dst, uint32_t val);
char* s32_to_str(char* dst, int32_t v);
#endif /* INC_RXTXMSGS_H_ */
