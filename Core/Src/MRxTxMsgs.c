/**
  ******************************************************************************
  * @file           : RxTxMsgs.c
  * @brief          : UART messages processing
   ******************************************************************************
  */

/* Includes -----------------------------------------------------------*/
#include "main.h"
#include "RxTxMsgs.h"

/* Local Definitions --------------------------------------------------*/
#define ISDIGIT(x)  (((x)>='0')&&((x)<='9'))
#define DIGITCHAR_2_VAL(x)  ((x)-'0')
#define DIGITVAL_2_CHAR(x)  ((x)+'0')
#define ILLEGAL_VALUE (0xffff)
/* Local Variables ----------------------------------------------------*/
uint8_t const * rx_tail_ptr;
uint8_t g_tmpRxRawMsgIdx = 0;
bool g_tmpRxRawMsgStartMarkerFound = false;
uint32_t gIllegalCommands = 0;
uint8_t tmpRxRawMsg[MAX_TMP_MSG_BUFFER_LEN]; 	//Buffer used for holding the next raw message while it is received
uint8_t rxBuffer[MAX_RX_BUFFER_LEN]; 		//Buffer used for reception

// Tx cyclic messages queue
uint8_t txBuffer[MAX_TX_QUEUE_SIZE][MAX_TX_BUFFER_LEN];
uint8_t gTxQueueHead = 0;
uint8_t gTxQueueTail = 0;
uint8_t txBufferSizes[MAX_TX_QUEUE_SIZE]; // holds the actual message size in each queue buffer

sUartMessage gTxMessage;

sCDMCommandDef gCDMCommands[eUARTCommand_num_commands] =
{
		{4, {'R','V','E','R'}, 0},  // eUARTCommand_rver,
		{4, {'C','A','R','B'}, 5},  // eUARTCommand_carb,   // if PS counter is implemented change the number of params to  6
		{4, {'P','R','S','W'}, 0},  // eUARTCommand_prsw,
		{4, {'S','T','R','T'}, 3},  // eUARTCommand_strt,
		{4, {'S','T','O','P'}, 0},  // eUARTCommand_stop,
		{4, {'H','O','M','E'}, 2},  // eUARTCommand_home, - syrup injection to home, parameter: % PWM (speed), 0/1 home pin state
		{4, {'I','N','J','C'}, 2},  // eUARTCommand_injC, - syrup inject, parameter: N signals, % PWM (speed)
		{4, {'G','I','S','T'}, 0},  // eUARTCommand_gist, - syrup injection status
};

/* Extern Variables ---------------------------------------------------*/
extern UART_HandleTypeDef huart1;
/* Local Methods ------------------------------------------------------*/
bool CheckAndProcessUartMessage(sUartMessage *newMsg, uint8_t* msgPtr, uint32_t msgSize);
uint16_t ParamValToString(uint8_t* buffer, uint16_t val);


/* @brief  Return number of waiting (not readed yet) rx bytes
 * @param  None
 * @return Number of waiting bytes
 */
static uint16_t COMM_UART_PeekReadyBytes()
{
	uint8_t const * head = RX_BUFFER_PTR + MAX_RX_BUFFER_LEN - __HAL_DMA_GET_COUNTER(huart1.hdmarx);
	uint8_t const * tail = rx_tail_ptr;

	if( head>=tail )
		return head-tail;
	else
		return head-tail+MAX_RX_BUFFER_LEN;
}

/* @brief  Copy numBytes from the RX buffer to the user buffer and update the internal pointer
 * @param
 * @return None
 */
static void COMM_UART_CopyRxBytes(uint8_t *targetBuffer, uint16_t numBytes)
{
	if ((rx_tail_ptr + numBytes) <= (RX_BUFFER_PTR + MAX_RX_BUFFER_LEN))
	{
		memcpy(targetBuffer ,rx_tail_ptr, numBytes);
		rx_tail_ptr += numBytes;
		// Wrap around the tail pointer if needed
		if (rx_tail_ptr >= RX_BUFFER_PTR + MAX_RX_BUFFER_LEN)
			rx_tail_ptr = RX_BUFFER_PTR;
	}
	else
	{
		int firstPartLength = RX_BUFFER_PTR + MAX_RX_BUFFER_LEN - rx_tail_ptr;
		memcpy(targetBuffer, rx_tail_ptr, firstPartLength);
		memcpy(targetBuffer+firstPartLength, RX_BUFFER_PTR, numBytes - firstPartLength);
		rx_tail_ptr = RX_BUFFER_PTR + numBytes - firstPartLength;
	}
}

/* @brief  Clear the current waiting bytes in the rx buffer, if at all
 * @param  None
 * @return None
 */
void COMM_UART_ClearRxBytes()
{
	rx_tail_ptr = RX_BUFFER_PTR + MAX_RX_BUFFER_LEN - __HAL_DMA_GET_COUNTER(huart1.hdmarx);
}

/* @brief  Start to receive RX bytes
 * @param  None
 * @return None
 */
void COMM_UART_StartRx()
{
	rx_tail_ptr = RX_BUFFER_PTR;
	HAL_UART_Receive_DMA(&huart1, RX_BUFFER_PTR, MAX_RX_BUFFER_LEN);
}

/* @brief  Try to read numBytes bytes from UART RX buffer
 * @param
 * @return
 */
uint32_t ddc = 0;
eUartStatus COMM_UART_CheckNewMessage(sUartMessage *newMsg, uint8_t* rawMsgPtr, uint32_t *rawMsgLen)
{
    bool error = false;

    // the following suppose to be a endless loop such as while(true) but
    // if noisy input fills buffer endlessly with garbage the loop will never return
    // therefore it will only handle the next 100 bytes and return
    for (uint8_t i = 0; i < 100 ; i++)
    {
        if (error) {
            g_tmpRxRawMsgIdx = 0;
            g_tmpRxRawMsgStartMarkerFound = false;
        }
        if (0 == COMM_UART_PeekReadyBytes()) {
            return eUART_NoMessage;
        }
        // 1 or more bytes are ready in the DMA Rx buffer
        COMM_UART_CopyRxBytes(&(tmpRxRawMsg[g_tmpRxRawMsgIdx]), 1);
        ddc++;
        // first look for start marker
        if (! g_tmpRxRawMsgStartMarkerFound) {
            if (tmpRxRawMsg[g_tmpRxRawMsgIdx] == COMMAND_START_SYMBOL) {
                g_tmpRxRawMsgStartMarkerFound = true;
            }
            // don't increment g_tmpRxRawMsgIdx
            continue;
        }
        // at this point start marker found. process the message
        if (tmpRxRawMsg[g_tmpRxRawMsgIdx] == COMMAND_END_MARKER) {
            // found the end of the message
            if (tmpRxRawMsg[g_tmpRxRawMsgIdx - 1] != COMMAND_PRE_END_MARKER) {
                // TODO: invalid message
                error = true;
                continue;
            }
            // check and convert the RXed message
            if (CheckAndProcessUartMessage(newMsg, tmpRxRawMsg, g_tmpRxRawMsgIdx)) {
            	rawMsgPtr[0] = COMMAND_START_SYMBOL;
				memcpy(&rawMsgPtr[1], tmpRxRawMsg, g_tmpRxRawMsgIdx+1);
				*rawMsgLen = g_tmpRxRawMsgIdx + 2;
				g_tmpRxRawMsgIdx = 0;
				g_tmpRxRawMsgStartMarkerFound = false;
				return eUART_MesssagePending;
            } else {
                error = true;
        		COMM_UART_QueueTxMessage((uint8_t *)"$Illegal Command\r\n", 18);
                continue;
            }
        }
		g_tmpRxRawMsgIdx++;
    }
    // TODO: if we got to here, handle noisy input on UART
    return eUART_NoMessage;
}

/* @brief  Try to read numBytes bytes from UART RX buffer
 * @param sUartMessage *newMsg - new message structure to fill
 * @param uint8_t* msgPtr - input message buffer to process
 * @param sUartMessage *newMsg - input message buffer size to process
 * @return if processed valid message - true
 */
bool CheckAndProcessUartMessage(sUartMessage *newMsg, uint8_t* msgPtr, uint32_t msgSize)
{
	int cmd_idx = ILLEGAL_VALUE;
	int idx = ILLEGAL_VALUE;
	// message should be starting with the first char of the command
	// Assuming that each command starts with a different character
	for (int i = 0; (i < eUARTCommand_num_commands) && (cmd_idx == ILLEGAL_VALUE); i++) {
		if (gCDMCommands[i].command[0] == msgPtr[0]) {
			cmd_idx = i; // assume OK
			// found first matching character of command
			for (int j = 0; j < gCDMCommands[i].len; j++) {
				if (gCDMCommands[i].command[j] != msgPtr[j]) {
					cmd_idx = ILLEGAL_VALUE; // mismatch char - not OK...
					break; // try next command
				}
			}
		}
	}
	if (cmd_idx == ILLEGAL_VALUE) {
		gIllegalCommands++;
		return false;
	}

	// at this point, we have a valid command name, check parameters
	idx = gCDMCommands[cmd_idx].len;
	if (idx >= msgSize) { gIllegalCommands++; return false; }
	if (gCDMCommands[cmd_idx].num_params > 0) {
		// validate the space
		if (msgPtr[idx] != ' '){
			gIllegalCommands++;
			return false;
		}
		idx++;
		int param_counter = 0;
		while (param_counter < gCDMCommands[cmd_idx].num_params)
		{
			newMsg->params.list[param_counter] = 0;
			while ((idx < msgSize) && (msgPtr[idx] != ',') && (msgPtr[idx] != COMMAND_PRE_END_MARKER)) // look for end of param
			{
				if (! ISDIGIT(msgPtr[idx])) { gIllegalCommands++; return false; }
				newMsg->params.list[param_counter] = newMsg->params.list[param_counter] * 10 + DIGITCHAR_2_VAL(msgPtr[idx]);
				idx++;
			}
			param_counter++;
			if (msgPtr[idx] == COMMAND_PRE_END_MARKER)
			{
				break;
			}
			idx++;
		}
		if ((param_counter != gCDMCommands[cmd_idx].num_params) || (msgPtr[idx] != COMMAND_PRE_END_MARKER))
		{
			gIllegalCommands++;
			return false;
		}
	}
	// at this point we have a valid command
	newMsg->cmd = (eUARTCommandTypes)cmd_idx;
	return true;
}


// Convert 1 ot 2 digits value to string
uint16_t ParamValToString(uint8_t* buffer, uint16_t val)
{
	uint16_t ret_size = 0;
	uint16_t tens = 0;
	if (val > 9)
	{
		tens = val / 10;
		buffer[ret_size] = DIGITVAL_2_CHAR(tens);
		ret_size++;
		tens *= 10;
	}
	buffer[ret_size] = DIGITVAL_2_CHAR(val-tens);
	ret_size++;
	return ret_size;
}

bool COMM_UART_QueueTxMessage(uint8_t *msg, uint32_t msgLen)
{
    // Check if prepared message fits in the buffer
    if (msgLen > MAX_TX_BUFFER_LEN) {
        // Message is too large for the buffer
        return false;
    }

    memcpy(txBuffer[gTxQueueHead], msg, msgLen);

    // Check if DMA is not busy and the queue is empty
    if ((huart1.hdmatx->State == HAL_DMA_STATE_READY) && (gTxQueueHead == gTxQueueTail)) {
        // Send message directly
        HAL_UART_Transmit_DMA(&huart1, txBuffer[gTxQueueHead], msgLen);

        // Move head to next position
        gTxQueueHead = (gTxQueueHead + 1) % MAX_TX_QUEUE_SIZE;

        return true;
    }

    // Check if queue is full
    if (((gTxQueueHead + 1) % MAX_TX_QUEUE_SIZE) == gTxQueueTail) {
        // Queue is full
        return false;
    }

    // Store the size of the prepared message
    txBufferSizes[gTxQueueHead] = msgLen;

    // Move head to next position for the next message
    gTxQueueHead = (gTxQueueHead + 1) % MAX_TX_QUEUE_SIZE;

    return true;
}

// Check if a message is pending in the TX queue and send it
// called by HAL_UART_TxCpltCallback
void COMM_UART_SendNextTxQueue(void)
{
    // Check if there are messages pending in the queue
    if (gTxQueueTail != gTxQueueHead) {
        // There is a message in the queue, send it
        HAL_UART_Transmit_DMA(&huart1, txBuffer[gTxQueueTail], txBufferSizes[gTxQueueTail]);
    }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    // Check if the callback is for the correct UART
    if (huart == &huart1) {

        // Move tail to the next message
        gTxQueueTail = (gTxQueueTail + 1) % MAX_TX_QUEUE_SIZE;

        // Check and send next message if available
        COMM_UART_SendNextTxQueue();
    }
}


