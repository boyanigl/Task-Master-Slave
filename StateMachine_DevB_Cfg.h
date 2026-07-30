#ifndef STATEMACHINE_DEVB_CFG_H
#define STATEMACHINE_DEVB_CFG_H

#include "Testing.h"

#define TIMER_TIME_ALLOWED 10
#define FAULT_TRESHOLD_CONFIRMATION 15

#if defined(TEST1_DEVB_SELFHEALING)
#define FAULT_RECOVERY_TIME 5          /* < FAULT_TRESHOLD_CONFIRMATION: DevB recovers on its own before DevA ever confirms the fault */
#define SLEEP_TRANSITIONS_TO_FAULT 0
#elif defined(TEST2_DEVA_RESETS)
#define FAULT_RECOVERY_TIME 30         /* > FAULT_TRESHOLD_CONFIRMATION: DevA confirms and resets DevB long before it would self-recover */
#define SLEEP_TRANSITIONS_TO_FAULT 0
#elif defined(TEST3_DEVB_SLEEP_TO_FAULT)
#define FAULT_RECOVERY_TIME 30         /* DevA still confirms and resets after the direct jump into FAULT */
#define SLEEP_TRANSITIONS_TO_FAULT 1   /* SLEEP exits straight to FAULT instead of ACTIVE, skipping DevA's IDLE->PROCESSING path entirely */
#else
#error "No DevB test scenario selected in Testing.h"
#endif

#endif
