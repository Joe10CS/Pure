/*
 * EventQueue.c
 *
 */
#include "EventQueue.h"


static SMEventQueue gSMEventQueue = { .head = 0, .tail = 0 };
static SMSodaStreamPure_EventId gSMEventQueuePurgeBuffer[SM_EVENT_QUEUE_SIZE]; // for SMEventQueue_PurgeStaleSetupEvents

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


/*
 * Remove setup-related events that were queued in the old "awake/ready" context
 * just before the machine committed to starting a drink.
 *
 * Motivation:
 * The button scanner runs before queued events are dispatched by the state machine.
 * Because of that ordering, a user can press BTN1 (start drink) and then press the
 * carbonation-level button a few milliseconds later, before the state machine has
 * actually transitioned out of the idle/setup state.
 *
 * In that race window, both events can be queued:
 *   1. EVENT_PRIMARYBUTTONPRESSED
 *   2. EVENT_CARBLEVELSHORTPRESSED
 *
 * If the start event is processed first, the machine moves into drink startup
 * (PreCheckWaterSensor / Filtering / Carbonating), but the old carbonation-level
 * event is still sitting in the queue from the previous context.
 *
 * That late setup event is no longer valid once drink startup has begun:
 * - it can silently change the logical carbonation level after the drink was already
 *   started
 * - it can trigger CO2 LED feedback at the wrong time
 * - it can leave the LEDs in a mixed/stale state when another animation interrupts
 *   the CO2 level animation
 *
 * We intentionally purge only stale setup/configuration events here.
 * We do NOT purge safety or stop-related events such as:
 * - EVENT_HWWATCHDOG
 * - EVENT_SAFETYFAIL
 * - EVENT_ANYKEYPRESS
 *
 * Those events remain relevant after startup and must be preserved.
 *
 * Today this function removes only EVENT_CARBLEVELSHORTPRESSED because that is the
 * event known to race with drink startup. If more setup-only events later show the
 * same problem, they can be added here deliberately.
 */
void SMEventQueue_PurgeStaleSetupEvents(void)
{
    uint8_t kept = 0;

    while (!SMEventQueue_IsEmpty())
    {
        SMSodaStreamPure_EventId ev = gSMEventQueue.buffer[gSMEventQueue.tail];
        gSMEventQueue.tail = (gSMEventQueue.tail + 1) % SM_EVENT_QUEUE_SIZE;

        if (ev == SMSodaStreamPure_EventId_EVENT_CARBLEVELSHORTPRESSED)
        {
            continue;
        }

        gSMEventQueuePurgeBuffer[kept++] = ev;
    }

    gSMEventQueue.head = 0;
    gSMEventQueue.tail = 0;

    for (uint8_t i = 0; i < kept; i++)
    {
        gSMEventQueue.buffer[gSMEventQueue.head] = gSMEventQueuePurgeBuffer[i];
        gSMEventQueue.head = (gSMEventQueue.head + 1) % SM_EVENT_QUEUE_SIZE;
    }
}

