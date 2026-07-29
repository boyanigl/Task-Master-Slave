/* State machine - Device A*/
#include "FreeRTOS.h"
#include "Types.h"
#include <stdio.h>
#include "semphr.h"
#include "StateMachine_DevA.h"
#include "StateMachine_DevB.h"

/* Pointer function to a transition activity - OnEntry - During - OnExit */
typedef void (*pFuncAct)(void);

/* Local variables definitions */
static StateMachine_DevA_States currState = State_Idle;
static StateMachine_DevA_States prevState = State_Idle;

/* Local functions declarations */
static void Dummy(void);
static void IdleState_Do(void);;
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
}

void StateMachine_DevA_Task(void* pvParameters) {
    (void)pvParameters;
    StateMachine_DevB_OutStatus tOutStatusDevB;

    for (; ;) {
        /* If change in state is catched - run the OnExit activity of previous state and OnEntry activity on current */
        if (currState != prevState) {
            stateMatrix[prevState][Action_OnEx]();
            stateMatrix[currState][Action_OnEn]();
        }

        /* Read state of Device B - and assign it as current state of Device A */
        StateMachine_DevB_Get_OutStatus(&tOutStatusDevB);
        if ((StateMachine_DevA_States)tOutStatusDevB.curroutState != currState) {
            currState = (StateMachine_DevA_States)tOutStatusDevB.curroutState;
        }

        /* Update the prevState var for next execution of task */
        prevState = currState;

        /* Execute the current state do activity */
        stateMatrix[currState][Action_Do]();

        /* Print current state of DevA */
        printf("DevA- State : ");
        printf("%d\n", currState);


        /* Fault state is configrmed for the predefined time, Master Device should reset the Slave*/
        if (tOutStatusDevB.faultConfirmed == TRUE) {
            StateMachine_DevB_Reset();
        }
        (void)vTaskDelay((const TickType_t )1000);
    }

}

void Dummy() {
    /* dummy implementation */
}
void IdleState_OnEntry() {

}

void IdleState_Do() {

}

void ProcessingState_Do() {

}

void ProcessingState_OnEntry() {

}

void ErrorState_Do() {
}

void ErrorState_OnEntry() {
    /* Log state changed to Error */
}