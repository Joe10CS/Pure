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

#define MAX_NUMBER_OF_PARAMETERS (6)

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
	eUARTCommand_rver,
	eUARTCommand_carb,
	eUARTCommand_prsw,
	eUARTCommand_strt,
	eUARTCommand_stop,
	eUARTCommand_home,
	eUARTCommand_injc,
	eUARTCommand_gist,

	eUARTCommand_num_commands,
	eUARTCommand_motor_current_beyond_threshold = 55,
	eUARTCommand_none = 99,
}eUARTCommandTypes;

/* types ---------------------------------------------------------------------*/
typedef struct {
	uint8_t len; // size following the $ sign
	char command[MAX_COMMAND_LEN];
	uint8_t num_params;
} sCDMCommandDef;

typedef struct {
	uint32_t cycleTime;
	uint32_t numberOfCycles;;
	uint32_t firstPSTimeout;
	uint32_t motorSpeedPercent;
	uint32_t numberOfPSCycles;
	uint32_t PSTotlaCyclesTimeout;
} sCarbParams;

typedef struct {
	uint32_t motorDiection;
	uint32_t motorSpeedPercent;
	uint32_t motorTime;
} sStartParams;

typedef struct {
	uint32_t numOfInjectSignals;
	uint32_t motorSpeedPercent;
} sInjectSyrupParams;

typedef struct {
	uint32_t motorSpeedPercent;
	uint32_t homePinState;
} sHomeParams;

typedef union {
    uint32_t list[MAX_NUMBER_OF_PARAMETERS];
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
extern eUARTCommandTypes glb_CDM_command;
extern sUartMessage glb_CDM_RxMessage;
/* functions ----------------------------------------------------------------*/

void COMM_UART_StartRx();
void COMM_UART_ClearRxBytes();
eUartStatus COMM_UART_CheckNewMessage(sUartMessage *newMsg, uint8_t* rawMsgPtr, uint32_t *rawMsgLen);
//uint16_t COMM_UART_PrepareTXMessage(uint8_t *before, uint8_t *after, uint16_t before_size);
bool COMM_UART_QueueTxMessage(uint8_t *msg, uint32_t msgLen);
void COMM_UART_SendNextTxQueue(void);
void SendMessageToCDMDesktop(eUARTCommandTypes msgType, uint16_t value1, uint16_t value2);
void CDMSendDoneMessage(eUartDoneStatus status);
#endif /* INC_RXTXMSGS_H_ */
