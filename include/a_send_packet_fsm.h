//
// Created by serbis on 08.08.18.
//

#ifndef MNVP_DRIVER_A_SEND_PACKET_FSM_H
#define MNVP_DRIVER_A_SEND_PACKET_FSM_H


//----------------------------------------------DATA-STATES block-------------------------------------------------------

#include "../libs/miniakka/includes/fsm.h"
#include "packet.h"

#define st_SendPacketFsm_Idle 0
#define st_SendPacketFsm_WaitRouteResponse 1

#define dt_SendPacketFsm_Uninitialized 0
#define dt_SendPacketFsm_WaitingRouteResponse 1


typedef struct dtb_SendPacketFsm_Uninitialized {
    ActorRef *rTable;
    ActorRef *gwPool;
    bool testMode;
} dtb_SendPacketFsm_Uninitialized;
fsm_data* new_dt_SendPacketFsm_Uninitialized(ActorRef *rTable, ActorRef *gwPool, bool testMode);

typedef struct dtb_SendPacketFsm_WaitingRouteResponse {
    ActorRef *rTable;
    ActorRef *gwPool;
    bool testMode;
    ActorRef *orig;
    Packet *packet;
} dtb_SendPacketFsm_WaitingRouteResponse;
fsm_data* new_dt_SendPacketFsm_WaitingRouteResponse(ActorRef *rTable, ActorRef *gwPool, bool testMode, ActorRef *orig,
                                             Packet *packet);

//----------------------------------------------- MESSAGES block--------------------------------------------------------

#define m_SendPacketFsm_Exec 32468
#define r_SendPacketFsm_SendResult 24644


typedef struct b_SendPacketFsm_Exec { Packet *packet; } b_SendPacketFsm_Exec;
ActorMessage* new_m_SendPacketFsm_Exec(Packet *packet, uint16_t stackSize);

typedef struct b_SendPacketFsm_SendResult { uint8_t code; } b_SendPacketFsm_SendResult;
ActorMessage* new_r_SendPacketFsm_SendResult(uint8_t code, uint16_t stackSize);

//------------------------------------------- FSM INITIALIZATION block--------------------------------------------------

typedef struct b_SendPacketFsm_Initialize {
    ActorRef *rTable;
    ActorRef *gwPool;
    bool testMode;
} b_SendPacketFsm_Initialize;
ActorMessage* new_m_SendPacketFsm_Initialize(ActorRef *rTable, ActorRef *gwPool, bool testMode, uint16_t stackSize);


Actor* new_SendPacketFsm();

#endif //MNVP_DRIVER_A_SEND_PACKET_FSM_H
