/* State machine - Device A*/
#include "FreeRTOS.h"
#include "Types.h"
#include <stdio.h>
#include "semphr.h"
#include "StateMachine_DevA.h"
#include "StateMachine_DevB.h"
#include "Logger.h"

/* Pointer function to a transition activity - OnEntry - During - OnExit */
typedef void (*pFuncAct)(void);

/* Local variables definitions */
static StateMachine_DevA_States currState = State_Idle;
static StateMachine_DevA_States prevState = State_Idle;
/* Latest Device B status snapshot, refreshed once per task cycle and consulted
 * by the state Do activities below to decide their own next state. */
static StateMachine_DevB_OutStatus devBStatus;

/* Local functions declarations */
static void Dummy(void);
static void IdleState_Do(void);
static void ProcessingState_Do(void);
static void ErrorState_Do(void);
static void IdleState_OnEntry(void);
static void ProcessingState_OnEntry(void);
static void ErrorState_OnEntry(void);


/* Const variables definitions */
static const pFuncAct stateMatrix[State_DevA_Last][3] = {
    /* OnEntry */				/* During */			/* OnExit */
    { IdleState_OnEntry,		IdleState_Do,		Dummy},
    { ProcessingState_OnEntry,	ProcessingState_Do,	Dummy},
    { ErrorState_OnEntry,		ErrorState_Do,		Dummy}

};


/* Function definitions */
void StateMachine_DevA_Init(void) {
    currState = State_Idle;
    prevState = State_Idle;

    /* First OnEnter action for Idle state*/
    stateMatrix[currState][Action_OnEn]();
}

void StateMachine_DevA_Task(void* pvParameters) {
    (void)pvParameters;

    for (; ;) {

        /* If change in state is catched - run the OnExit activity of previous state and OnEntry activity on current */
        if (currState != prevState) {
            stateMatrix[prevState][Action_OnEx]();
            stateMatrix[currState][Action_OnEn]();
        }

        /* Update the prevState var for next execution of task */
        prevState = currState;

        /* Refresh the Device B status snapshot */
        if (StateMachine_DevB_Get_OutStatus(&devBStatus) != TRUE) {
            Log_Event(LOG_ERROR, "DEVA", "Failed to read Device B status (mutex busy) - using stale data");
        }

        /* Fault state is configrmed for the predefined time, Master Device should reset the Slave*/
        if (devBStatus.faultConfirmed == TRUE) {
            Log_Event(LOG_WARNING, "DEVA", "Device B FAULT confirmed - issuing reset");
            if (StateMachine_DevB_Reset() != TRUE) {
                Log_Event(LOG_ERROR, "DEVA", "Device B reset failed (mutex busy)");
            }
        }

        /* Execute the current state do activity - it decides its own next state */
        stateMatrix[currState][Action_Do]();

        (void)vTaskDelay(pdMS_TO_TICKS(1000));
    }

}

void Dummy() {
    /* dummy implementation */
}
void IdleState_OnEntry() {
    Log_Event(LOG_INFO, "DEVA", "Entered IDLE");
}

void IdleState_Do() {
    /* React to Device B becoming ACTIVE by starting processing */
    if (devBStatus.curroutState == State_Active) {
        currState = State_Processing;
    }else if (devBStatus.curroutState == State_Fault) {
        /* React to Device B becoming FAULT by entering error state */
        currState = State_Error;
    }
}

void ProcessingState_Do() {
    /* Device B returned to SLEEP - consider processing complete */
    if (devBStatus.curroutState == State_Sleep) {
        currState = State_Idle;
    }else if (devBStatus.curroutState == State_Fault) {
        /* React to Device B becoming FAULT by entering error state */
        currState = State_Error;
    }
}

void ProcessingState_OnEntry() {
    Log_Event(LOG_INFO, "DEVA", "Entered PROCESSING");
}

void ErrorState_Do() {
    /* Recover once Device B is no longer in FAULT */
    if (devBStatus.curroutState != State_Fault) {
        currState = State_Idle;
    }else{
        /* Still in FAULT */
    }
}

void ErrorState_OnEntry() {
    Log_Event(LOG_WARNING, "DEVA", "Entered ERROR (Device B FAULT)");
}