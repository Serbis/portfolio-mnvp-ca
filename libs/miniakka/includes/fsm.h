//
// Created by serbis on 30.07.18.
//

#ifndef MNVP_DRIVER_FSM_H
#define MNVP_DRIVER_FSM_H

#include <stdint.h>
#include "messages.h"
#include "actor.h"

#define m_FSM_StateTimeout 2
#define m_Initialize 3

typedef struct fsm_state { uint8_t code; char* timer; } fsm_state;
typedef struct fsm_data { uint8_t code; void* data; void (*destrutor)(struct fsm_data*); } fsm_data;
typedef struct fsm_box { fsm_state* state; fsm_data* data; } fsm_box;

fsm_box* new_fsm_box(fsm_data* data, fsm_state* state);
fsm_state* new_fsm_state(uint8_t code, char* timer);
fsm_data* new_fsm_data(uint8_t code, void* data, void (*destructor)(void*));

typedef struct b_FSM_StateTimeout { uint8_t stateCode; } b_FSM_StateTimeout;
ActorMessage* new_m_FSM_StateTimeout(uint8_t stateCode, uint16_t stackSize);

void FSM_toState(Actor *actor, fsm_data *data, uint8_t state, uint32_t stateTimeout, uint16_t stateTimeoutStack);
void FSM_stay(Actor *actor, uint32_t stateTimeout, uint16_t stateTimeoutStack);
void FSM_stop(Actor *actor, uint16_t stackSize);

#endif //MNVP_DRIVER_FSM_H
