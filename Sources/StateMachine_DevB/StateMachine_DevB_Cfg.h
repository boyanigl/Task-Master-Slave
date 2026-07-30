#ifndef STATEMACHINE_DEVB_CFG_H
#define STATEMACHINE_DEVB_CFG_H

#include "Testing.h"

#define TIMER_TIME_ALLOWED 10
#define FAULT_TRESHOLD_CONFIRMATION 15

#if defined(TEST1_DEVB_SELFHEALING)
#define FAULT_RECOVERY_TIME 5          /* < FAULT_TRESHOLD_CONFIRMATION: DevB recovers on its own before DevA ever confirms the fault */
#elif defined(TEST2_DEVA_RESETS)
#define FAULT_RECOVERY_TIME 30         /* > FAULT_TRESHOLD_CONFIRMATION: DevA confirms and resets DevB long before it would self-recover */
#elif defined(TEST3_DEVB_SLEEP_TO_FAULT)
#define FAULT_RECOVERY_TIME 30         /* DevA still confirms and resets after the direct jump into FAULT */
#else
#error "No Device test scenario selected in Testing.h"
#endif

#endif
