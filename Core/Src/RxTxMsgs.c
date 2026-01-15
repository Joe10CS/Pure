/**
  ******************************************************************************
  * @file           : RxTxMsgs.c
  * @brief          : UART messages processing
   ******************************************************************************
  */

/* Includes -----------------------------------------------------------*/
#include "main.h"
#include "string.h"
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

#define MAKE_CMD_KEY(a, b) (((uint16_t)(a) << 8) | (b))

// IMPORTANT!! first two character of each command should be unique !!!
sCommandDef gCDMCommands[eUARTCommand_num_commands] =
{
		{4, {'M','G','U','I'}, 1},  // eUARTCommand_mgui,
		{4, {'P','O','W','R'}, 1},  // eUARTCommand_powr,
		{4, {'S','L','E','D'}, 2},  // eUARTCommand_sled,
		{4, {'S','R','G','B'}, 3},  // eUARTCommand_srgb,
		{4, {'P','U','M','P'}, 2},  // eUARTCommand_pump,
		{4, {'C','A','R','B'}, 1},  // eUARTCommand_carb,
		{4, {'S','T','O','P'}, 0},  // eUARTCommand_stop,
		{4, {'T','I','L','T'}, 0},  // eUARTCommand_tilt,
		{4, {'W','T','R','S'}, 0},  // eUARTCommand_wtrs,
		{4, {'U','V','L','D'}, 1},  // eUARTCommand_uvld,
		{4, {'U','V','L','A'}, 0},  // eUARTCommand_uvla,
		{4, {'P','M','P','A'}, 0},  // eUARTCommand_pmpa,
		{4, {'R','R','T','C'}, 0},  // eUARTCommand_rrtc,
		{4, {'R','S','T','S'}, 2},  // eUARTCommand_rsts,
		{4, {'R','V','E','R'}, 0},  // eUARTCommand_rver
		{4, {'S','T','B','L'}, 9},  // eUARTCommand_stbl
		{4, {'C','O','N','F'}, 2},  // eUARTCommand_conf
		{4, {'S','W','S','P'}, 1},  // eUARTCommand_swsp
		{4, {'L','P','T','M'}, 0},  // eUARTCommand_lptm
		{4, {'D','O','N','E'}, 0},  // eUARTCommand_done,  just for replay

		// extra commands for for debugging and testing
		{4, {'A','N','I','M'}, 3},  // eUARTCommand_anim
        {4, {'F','D','A','Y'}, 2},  // eUARTCommand_fday
        {4, {'C','S','E','C'}, 2},  // eUARTCommand_csec
        {4, {'C','M','A','X'}, 1},  // eUARTCommand_cmax
        {4, {'F','M','E','M'}, 3},  // eUARTCommand_fmem
        {4, {'F','L','S','C'}, 0},  // eUARTCommand_flsc
        {4, {'V','B','A','T'}, 0},  // eUARTCommand_vbat
		{4, {'O','O','T','B'}, 1},  // eUARTCommand_ootb
		{4, {'F','C','N','T'}, 2},  // eUARTCommand_fcnt
        {4, {'D','B','U','G'}, 2},  // eUARTCommand_dbug
};

/* Extern Variables ---------------------------------------------------*/
extern UART_HandleTypeDef huart2;
/* Local Methods ------------------------------------------------------*/
bool CheckAndProcessUartMessage(sUartMessage *newMsg, uint8_t* msgPtr, uint32_t msgSize);
uint16_t ParamValToString(uint8_t* buffer, uint16_t val);


/* @brief  Return number of waiting (not readed yet) rx bytes
 * @param  None
 * @return Number of waiting bytes
 */
static uint16_t COMM_UART_PeekReadyBytes()
{
	uint8_t const * head = RX_BUFFER_PTR + MAX_RX_BUFFER_LEN - __HAL_DMA_GET_COUNTER(huart2.hdmarx);
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
	rx_tail_ptr = RX_BUFFER_PTR + MAX_RX_BUFFER_LEN - __HAL_DMA_GET_COUNTER(huart2.hdmarx);
}

/* @brief  Start to receive RX bytes
 * @param  None
 * @return None
 */
void COMM_UART_StartRx()
{
	rx_tail_ptr = RX_BUFFER_PTR;
	HAL_UART_Receive_DMA(&huart2, RX_BUFFER_PTR, MAX_RX_BUFFER_LEN);
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
                TxIllegalCommandResponse();
                continue;
            }
        }
		g_tmpRxRawMsgIdx++;
    }
    // TODO: if we got to here, handle noisy input on UART
    return eUART_NoMessage;
}

void TxIllegalCommandResponse()
{
	COMM_UART_QueueTxMessage((uint8_t *)"$Illegal Command\r\n", 18);
}
/* @brief  Try to read numBytes bytes from UART RX buffer
 * @param sUartMessage *newMsg - new message structure to fill
 * @param uint8_t* msgPtr - input message buffer to process
 * @param sUartMessage *newMsg - input message buffer size to process
 * @return if processed valid message - true
 */
bool CheckAndProcessUartMessage(sUartMessage *newMsg, uint8_t* msgPtr, uint32_t msgSize)
{
	if (msgSize < 2) { gIllegalCommands++; return false; }

	// The 'key' of the received command
	uint16_t key = MAKE_CMD_KEY(msgPtr[0], msgPtr[1]);

	int cmd_idx = ILLEGAL_VALUE;
	for (int i = 0; i < eUARTCommand_num_commands; i++) {
		if (MAKE_CMD_KEY(gCDMCommands[i].command[0], gCDMCommands[i].command[1]) != key)
			continue;

		if (msgSize < gCDMCommands[i].len)
			continue;

		bool match = true;
		for (int j = 2; j < gCDMCommands[i].len; j++) {
			if (msgPtr[j] != gCDMCommands[i].command[j]) {
				match = false;
				break;
			}
		}
		if (match) {
			cmd_idx = i;
			break;
		}
	}
	if (cmd_idx == ILLEGAL_VALUE) {
		gIllegalCommands++;
		return false;
	}

	// at this point, we have a valid command name, check parameters
	int idx = gCDMCommands[cmd_idx].len;
	if (idx >= msgSize) { gIllegalCommands++; return false; }
	if (gCDMCommands[cmd_idx].num_params > 0) {
		// validate the space
		if (msgPtr[idx] != ' '){
			gIllegalCommands++;
			return false;
		}
		idx++;
		int param_counter = 0;
		if (cmd_idx == eUARTCommand_conf)  {
			newMsg->params.config.configurationParamID = 0;
			newMsg->params.config.value = 0;
		}
		while (param_counter < gCDMCommands[cmd_idx].num_params)
		{
			if (cmd_idx != eUARTCommand_conf)  {
				newMsg->params.list[param_counter] = 0;
			}
			while ((idx < msgSize) && (msgPtr[idx] != ',') && (msgPtr[idx] != COMMAND_PRE_END_MARKER)) // look for end of param
			{
				if (! ISDIGIT(msgPtr[idx])) { gIllegalCommands++; return false; }
				if (cmd_idx == eUARTCommand_conf)  { // configuration parameters are 32 bits so cannot use params.list
					if  (param_counter == 0) { // param id
						newMsg->params.config.configurationParamID = newMsg->params.config.configurationParamID * 10 + DIGITCHAR_2_VAL(msgPtr[idx]);
					} else if  (param_counter == 1) { // value
						newMsg->params.config.value = newMsg->params.config.value * 10 + DIGITCHAR_2_VAL(msgPtr[idx]);
					}
				} else {
 				    newMsg->params.list[param_counter] = newMsg->params.list[param_counter] * 10 + DIGITCHAR_2_VAL(msgPtr[idx]);
				}
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
    if ((huart2.hdmatx->State == HAL_DMA_STATE_READY) && (gTxQueueHead == gTxQueueTail)) {
        // Send message directly
        HAL_UART_Transmit_DMA(&huart2, txBuffer[gTxQueueHead], msgLen);

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
        HAL_UART_Transmit_DMA(&huart2, txBuffer[gTxQueueTail], txBufferSizes[gTxQueueTail]);
    }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    // Check if the callback is for the correct UART
    if (huart == &huart2) {

        // Move tail to the next message
        gTxQueueTail = (gTxQueueTail + 1) % MAX_TX_QUEUE_SIZE;

        // Check and send next message if available
        COMM_UART_SendNextTxQueue();
    }
}

// //////////////////////////////////////// response builders ///////////////////////////////////////////

// ---------- tiny integer printer (no libc needed) ----------
char* u32_to_str(char *dst, uint32_t val)
{
    char tmp[10];
    int i = 0;
    if (val == 0) { *dst++ = '0'; return dst; }
    while (val) { tmp[i++] = (char)('0' + (val % 10)); val /= 10; }
    while (i--) { *dst++ = tmp[i]; }
    return dst;
}

char* s32_to_str(char* dst, int32_t v) {
    if (v < 0) { *dst++ = '-'; v = -v; }
    return u32_to_str(dst, (uint32_t)v);
}

// ---------- core reply builder ----------
// nums_count: 1..4 (can be 0 if you have such a reply shape)
// ok_suffix: pass true to append ",OK" (only for $STBL), else false
// returns number of bytes written (excluding '\0')
size_t BuildReply(char *dst,
		const eUARTCommandTypes cmd_id,
        const uint32_t *nums,
        uint8_t nums_count,
        bool ok_suffix)
{
    char *p = dst;

    *p++ = '$';
    *p++ = gCDMCommands[cmd_id].command[0];
    *p++ = gCDMCommands[cmd_id].command[1];
    *p++ = gCDMCommands[cmd_id].command[2];
    *p++ = gCDMCommands[cmd_id].command[3];

    if (nums_count || ok_suffix) *p++ = ' ';
    if (cmd_id == eUARTCommand_rver)
    {
    	// add "Version"
    	*p++ = 'V'; *p++ = 'e';*p++ = 'r'; *p++ = 's';*p++ = 'i'; *p++ = 'o';*p++ = 'n';*p++ = ' ';
    }

    for (uint8_t i = 0; i < nums_count; i++) {
        p = u32_to_str(p, nums[i]);
        if (i + 1u < nums_count) *p++ = ',';
    }

    if (ok_suffix) {
        if (nums_count) *p++ = ',';
        *p++ = 'O'; *p++ = 'K';
    }

    *p++ = '\r';
    *p++ = '\n';

    *p = '\0';  // keep NUL for convenience
    return (size_t)(p - dst); // length WITHOUT the NUL
}
// signed version
size_t BuildReplySigned(char *dst,
		const eUARTCommandTypes cmd_id,
        const int32_t *nums,
		uint8_t nums_count,
        bool ok_suffix)
{
    char *p = dst;

    *p++ = '$';
    *p++ = gCDMCommands[cmd_id].command[0];
    *p++ = gCDMCommands[cmd_id].command[1];
    *p++ = gCDMCommands[cmd_id].command[2];
    *p++ = gCDMCommands[cmd_id].command[3];

    if (nums_count || ok_suffix) *p++ = ' ';

    for (uint8_t i = 0; i < nums_count; i++) {
        p = s32_to_str(p, nums[i]);
        if (i + 1u < nums_count) *p++ = ',';
    }

    if (ok_suffix) {
        if (nums_count) *p++ = ',';
        *p++ = 'O'; *p++ = 'K';
    }

    *p++ = '\r';
    *p++ = '\n';

    *p = '\0';  // keep NUL for convenience
    return (size_t)(p - dst); // length WITHOUT the NUL
}

// ---------- examples ----------
/*
extern sCommandDef gCDMCommands[];

uint8_t Make_RVER(char *out, uint32_t maj, uint32_t min)
{
    uint32_t nums[2] = { maj, min };
    BuildReply(out, eUARTCommand_rver, nums, 2, false);
    return (uint8_t)strlen(out); // or track length in BuildReply if you want to avoid strlen
}

uint8_t Make_STBL(char *out, uint32_t nn)
{
    BuildReply(out, eUARTCommand_stbl, &nn, 1, true);  // "$STBL nn,OK\r\n"
    return (uint8_t)strlen(out);
}

// Generic for 1..4 numbers:
uint8_t Make_GenericReply(char *out, eUARTCommandTypes cmd, const uint32_t *nums, uint8_t n)
{
    BuildReply(out, cmd, nums, n, false);
    return (uint8_t)strlen(out);
}
*/
