#ifndef STATEMACHINE_DEVA_H
#define STATEMACHINE_DEVA_H

typedef enum {
    State_Idle = 0u,
    State_Processing,
    State_Error,
    State_DevA_Last
}StateMachine_DevA_States;


/* Public functions declarations */
void StateMachine_DevA_Task(void* pvParameters);
void StateMachine_DevA_Init(void);



#endif