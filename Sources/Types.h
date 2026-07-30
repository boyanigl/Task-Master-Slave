#ifndef TYPES_H
#define TYPES_H

#define FALSE 0
#define TRUE 1

typedef enum {
    Action_OnEn = 0u,
    Action_Do,
    Action_OnEx
}StateMachine_Action;

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef uint8 boolean;

#endif