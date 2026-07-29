#ifndef STATEMACHINE_DEVB_H
#define STATEMACHINE_DEVB_H
#include "Types.h"
typedef enum {
    State_Sleep = 0u,
    State_Active,
    State_Fault,
    State_DevB_Last
}StateMachine_DevB_States;

typedef struct {
    StateMachine_DevB_States curroutState;
    StateMachine_DevB_States prevoutState;
    uint8 faultConfirmed;
}StateMachine_DevB_OutStatus;

/* Public functions declarations */
void StateMachine_DevB_Task(void* pvParameters);
void StateMachine_DevB_Init(void);
boolean StateMachine_DevB_Get_OutStatus(StateMachine_DevB_OutStatus* state);
boolean StateMachine_DevB_Reset(void);

#endif
