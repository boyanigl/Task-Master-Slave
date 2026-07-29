/* State machine - Device B*/
#include "FreeRTOS.h"
#include "Types.h"
#include <stdio.h>
#include "semphr.h"

#include "StateMachine_DevB.h"
#include "StateMachine_DevB_Cfg.h"

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
}

void StateMachine_DevB_Task(void* pvParameters) {

    for (; ; ) {

        /* If change in state is catched - run the OnExit activity of previous state and OnEntry activity on current */
        if (currState != prevState) {
            stateMatrix[prevState][Action_OnEx]();
            stateMatrix[currState][Action_OnEn]();
        }

        /* Update the prevState var for next execution of task */
        prevState = currState;

        /* Execute the current state do activity */
        stateMatrix[currState][Action_Do]();


        /* Print current state of DevB */
        printf("DevB- State : ");
        printf("%d\n", currState);


        if (xSemaphoreTake(xStatusMutex, portMAX_DELAY) == pdTRUE) {
            outStatusData.curroutState = currState;
            outStatusData.prevoutState = prevState;
            (void)xSemaphoreGive(xStatusMutex);
        }
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
        /* todo: Log unable to get status */
    }

    return retVal;
}

boolean StateMachine_DevB_Reset() {
    boolean retVal = FALSE;
    if(xSemaphoreTake(xStatusMutex, pdMS_TO_TICKS(10000)) == pdTRUE) {
        currState = State_Sleep;
        (void)xSemaphoreGive(xStatusMutex);
        retVal = TRUE;
    }
    else {
        //resource is busy for 2 long
        /* todo: Log unable to reset */

    }

    /*todo : Log the reset event at specific time */

    return retVal;
}

/* LOCAL FUNCTIONS DEFINITIONS */
static void Dummy() {
    /* dummy implementation */
}
static void SleepState_OnEntry() {
    timerStates = 0u;

}

static void SleepState_Do() {
    //Increment timer
    timerStates++;
    if (timerStates >= TIMER_TIME_ALLOWED) {
        //semaphore logic
        currState = State_Active;
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
    timerStates = 0u;
}

static void FaultState_Do() {
    if ((++consecutiveFaultCounter) >= FAULT_TRESHOLD_CONFIRMATION) {
        if (xSemaphoreTake(xStatusMutex, pdMS_TO_TICKS(10000)) == pdTRUE) {
            outStatusData.faultConfirmed = TRUE;
            (void)xSemaphoreGive(xStatusMutex);
        }
    }

    timerStates++;
    if (timerStates >= 30) {
        currState = State_Sleep;
    }
}

static void FaultState_OnEntry() {
    /* todo: Log fault */
    /* Log state changed to Fault */
    consecutiveFaultCounter = 0u;
    timerStates = 0u;
}

static void FaultState_OnExit() {
    /* Exiting Fault State, should reset faultConfirmed status */
    if (xSemaphoreTake(xStatusMutex, pdMS_TO_TICKS(10000)) == pdTRUE) {
        outStatusData.faultConfirmed = FALSE;
        (void)xSemaphoreGive(xStatusMutex);
    }
}