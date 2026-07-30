/* State machine - Device B*/
#include "FreeRTOS.h"
#include "Types.h"
#include <stdio.h>
#include "semphr.h"

#include "StateMachine_DevB.h"
#include "StateMachine_DevB_Cfg.h"
#include "Logger.h"

/* Pointer function to a transition activity - OnEntry - During - OnExit */
typedef void (*pFuncAct)(void);

/* Global shared variables */



/* Local variables definitions */
static StateMachine_DevB_States currState = State_Sleep;
static StateMachine_DevB_States prevState = State_Sleep;
static StateMachine_DevB_OutStatus outStatusData = { State_Sleep, State_Sleep, FALSE };
static uint16 timerStates = 0;
static uint8 consecutiveFaultCounter = 0u;
static SemaphoreHandle_t xStatusMutex = NULL;


/* Local functions declarations */
static void Dummy(void);
static void SleepState_Do(void);
static void ActiveState_Do(void);
static void FaultState_Do(void);
static void SleepState_OnEntry(void);
static void ActiveState_OnEntry(void);
static void FaultState_OnEntry(void);
static void FaultState_OnExit(void);

/* Const variables definitions */
static const pFuncAct stateMatrix[State_DevB_Last][3] = {
    /* OnEntry */				/* During */			/* OnExit */
    { SleepState_OnEntry,		SleepState_Do,		Dummy},
    { ActiveState_OnEntry,	    ActiveState_Do,	    Dummy},
    { FaultState_OnEntry,		FaultState_Do,		FaultState_OnExit}

};


/* Function definitions */
void StateMachine_DevB_Init(void) {

    currState = State_Sleep;
    prevState = State_Sleep;

    xStatusMutex = xSemaphoreCreateMutex();
    configASSERT(xStatusMutex != NULL);
}

void StateMachine_DevB_Task(void* pvParameters) {
    (void)pvParameters;

    for (; ; ) {

        /* currState/prevState/timerStates/consecutiveFaultCounter and outStatusData are
         * treated as one protected unit for the whole cycle, since StateMachine_DevB_Reset()
         * can write currState from Device A's task at any time. */
        if (xSemaphoreTake(xStatusMutex, portMAX_DELAY) == pdTRUE) {
            /* If change in state is catched - run the OnExit activity of previous state and OnEntry activity on current */
            if (currState != prevState) {
                stateMatrix[prevState][Action_OnEx]();
                stateMatrix[currState][Action_OnEn]();
            }

            /* Update the prevState var for next execution of task */
            prevState = currState;

            /* Execute the current state do activity */
            stateMatrix[currState][Action_Do]();

            outStatusData.curroutState = currState;
            outStatusData.prevoutState = prevState;

            (void)xSemaphoreGive(xStatusMutex);
        }

        /* Print current state of DevB */
        printf("DevB- State : ");
        printf("%d\n", currState);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }

}

boolean StateMachine_DevB_Get_OutStatus(StateMachine_DevB_OutStatus* pOutStatus) {
    boolean retVal = FALSE;
    if (xSemaphoreTake(xStatusMutex, pdMS_TO_TICKS(10000)) == pdTRUE) {
        pOutStatus->curroutState = outStatusData.curroutState;
        pOutStatus->prevoutState = outStatusData.prevoutState;
        pOutStatus->faultConfirmed = outStatusData.faultConfirmed;
        retVal = TRUE;

        //release semaphore
        (void)xSemaphoreGive(xStatusMutex);
    }
    else {
        Log_Event(LOG_WARNING, "DEVB", "Status read failed (mutex busy)");
    }

    return retVal;
}

boolean StateMachine_DevB_Reset() {
    boolean retVal = FALSE;
    if(xSemaphoreTake(xStatusMutex, pdMS_TO_TICKS(10000)) == pdTRUE) {
        currState = State_Sleep;
        (void)xSemaphoreGive(xStatusMutex);
        retVal = TRUE;
        Log_Event(LOG_INFO, "DEVB", "Reset accepted - returning to SLEEP");
    }
    else {
        Log_Event(LOG_WARNING, "DEVB", "Reset failed (mutex busy)");
    }

    return retVal;
}

/* LOCAL FUNCTIONS DEFINITIONS */
static void Dummy() {
    /* dummy implementation */
}
static void SleepState_OnEntry() {
    Log_Event(LOG_INFO, "DEVB", "Entered SLEEP");
    timerStates = 0u;
}

static void SleepState_Do() {
    //Increment timer
    timerStates++;
    if (timerStates >= TIMER_TIME_ALLOWED) {
#ifdef TEST3_DEVB_SLEEP_TO_FAULT
            currState = State_Fault;
#else
            currState = State_Active;
#endif
    }
}

static void ActiveState_Do() {
    //Increment timer
    timerStates++;
    if (timerStates >= TIMER_TIME_ALLOWED) {
        currState = State_Fault;
    }
}

static void ActiveState_OnEntry() {
    Log_Event(LOG_INFO, "DEVB", "Entered ACTIVE");
    timerStates = 0u;
}

static void FaultState_Do() {
    /* Called only from StateMachine_DevB_Task while xStatusMutex is already held. */
    if ((++consecutiveFaultCounter) >= FAULT_TRESHOLD_CONFIRMATION) {
        outStatusData.faultConfirmed = TRUE;
    }

    timerStates++;
    if (timerStates >= FAULT_RECOVERY_TIME) {
        currState = State_Sleep;
    }
}

static void FaultState_OnEntry() {
    Log_Event(LOG_ERROR, "DEVB", "Entered FAULT");
    consecutiveFaultCounter = 0u;
    timerStates = 0u;
}

static void FaultState_OnExit() {
    /* Exiting Fault State, should reset faultConfirmed status.
     * Called only from StateMachine_DevB_Task while xStatusMutex is already held. */
    outStatusData.faultConfirmed = FALSE;
}