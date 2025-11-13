/*
 * EventQueue.c
 *
 */
#include "EventQueue.h"


static SMEventQueue gSMEventQueue = { .head = 0, .tail = 0 };

bool SMEventQueue_IsEmpty(void)
{
    return gSMEventQueue.head == gSMEventQueue.tail;
}

bool SMEventQueue_IsFull(void)
{
    return ((gSMEventQueue.head + 1) % SM_EVENT_QUEUE_SIZE) == gSMEventQueue.tail;
}
bool SMEventQueue_Add(SMSodaStreamPure_EventId event)
{
    bool inInterrupt = IsInInterruptContext();
    if (inInterrupt)
        __disable_irq();

    bool success = false;
    if (!SMEventQueue_IsFull())
    {
        gSMEventQueue.buffer[gSMEventQueue.head] = event;
        gSMEventQueue.head = (gSMEventQueue.head + 1) % SM_EVENT_QUEUE_SIZE;
        success = true;
    }

    if (inInterrupt)
        __enable_irq();

    return success;
}

bool SMEventQueue_Take(SMSodaStreamPure_EventId* outEvent)
{
    if (SMEventQueue_IsEmpty())
        return false;

    *outEvent = gSMEventQueue.buffer[gSMEventQueue.tail];
    gSMEventQueue.tail = (gSMEventQueue.tail + 1) % SM_EVENT_QUEUE_SIZE;
    return true;
}

bool IsInInterruptContext(void)
{
    return (__get_IPSR() != 0);
}
