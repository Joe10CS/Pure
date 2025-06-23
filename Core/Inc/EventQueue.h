/*
 * EventQueue.h
 */

#ifndef INC_EVENTQUEUE_H_
#define INC_EVENTQUEUE_H_

#include "main.h"
#include "cmsis_gcc.h"
#include "SMSodaStreamPure.h"

#define SM_EVENT_QUEUE_SIZE 16

typedef struct {
	SMSodaStreamPure_EventId buffer[SM_EVENT_QUEUE_SIZE];
    uint8_t head;
    uint8_t tail;
} SMEventQueue;



bool SMEventQueue_IsEmpty(void);
bool SMEventQueue_IsFull(void);
bool SMEventQueue_Add(SMSodaStreamPure_EventId event);
bool SMEventQueue_Take(SMSodaStreamPure_EventId* outEvent);
bool IsInInterruptContext(void);

#endif /* INC_EVENTQUEUE_H_ */
