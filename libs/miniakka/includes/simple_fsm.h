//
// Created by serbis on 31.07.18.
//

#ifndef MNVP_DRIVER_SIMPLE_FSM_H
#define MNVP_DRIVER_SIMPLE_FSM_H

#include "actor.h"
#include "fsm.h"
#include "actor_system.h"

//----------------------------------------------DATA-STATES block-------------------------------------------------------

#define st_SimpleFsm_idle 0
#define st_SimpleFsm_one 1
#define st_SimpleFsm_two 2

#define dt_SimpleFsm_idle 0
#define dt_SimpleFsm_one 0
#define dt_SimpleFsm_two 0


typedef struct dtb_SimpleFsm_idle { char *str0; } dtb_SimpleFsm_idle;
fsm_data* new_dtb_SimpleFsm_idle(char *str0);

typedef struct dtb_SimpleFsm_one { char *str0; char *str1; } dtb_SimpleFsm_one;
fsm_data* new_dtb_SimpleFsm_one(char *str0, char *str1);

typedef struct dtb_SimpleFsm_two { char *str0; char *str1; char *str2; } dtb_SimpleFsm_two;
fsm_data* new_dtb_SimpleFsm_two(char *str0, char *str1, char *str2);

//----------------------------------------------- MESSAGES block--------------------------------------------------------

#define m_SimpleFsm_Message0 51697
#define m_SimpleFsm_Message1 15647
#define m_SimpleFsm_Message2 12654


typedef struct b_SimpleFsm_Message0 { char* str1; } b_SimpleFsm_Message0;
ActorMessage* new_m_SimpleFsm_Message0(char* str1, uint16_t stackSize);

typedef struct b_SimpleFsm_Message1 { char* str2; } b_SimpleFsm_Message1;
ActorMessage* new_m_SimpleFsm_Message1(char* str2, uint16_t stackSize);

//>>>
ActorMessage* new_m_SimpleFsm_Message2(uint16_t stackSize);

//------------------------------------------- FSM INITIALIZATION block--------------------------------------------------

typedef struct b_SimpleFsm_Initialize { char* str0; } b_SimpleFsm_Initialize;
ActorMessage* new_m_SimpleFsm_Initialize(char* str0, uint16_t stackSize);


Actor* new_SimpleFsm();

#endif //MNVP_DRIVER_SIMPLE_FSM_H