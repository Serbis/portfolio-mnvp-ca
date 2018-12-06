//
// Created by serbis on 08.07.18.
//

#ifndef MNVP_DRIVER_A_GATEWAY_H
#define MNVP_DRIVER_A_GATEWAY_H

#include "../libs/miniakka/includes/actor.h"
#include "packet.h"
#include "../libs/miniakka/includes/router.h"

#define m_Gateway_Send 48627
#define m_Gateway_SetLabel 12958
#define m_Gateway_GetLabel 23548
#define m_Gateway_SetNetworkAddress 13649
#define m_Gateway_GetNetworkAddress 12356
#define r_Gateway_Stopped 51945
#define r_Gateway_Label 24586
#define r_Gateway_NetworkAddress 14364


typedef struct b_Gateway_Send { Packet *packet; } b_Gateway_Send;
ActorMessage* new_m_Gateway_Send(Packet *packet, uint16_t stackSize);
typedef struct b_Gateway_SetLabel { uint16_t label; } b_Gateway_SetLabel;
ActorMessage* new_m_Gateway_SetLabel(uint16_t label, uint16_t stackSize);
//----
ActorMessage* new_m_Gateway_GetLabel(uint16_t stackSize);
typedef struct b_Gateway_SetNetworkAddress { uint32_t netAdr; } b_Gateway_SetNetworkAddress;
ActorMessage* new_m_Gateway_SetNetworkAddress(uint32_t netAdr, uint16_t stackSize);
//----
ActorMessage* new_m_Gateway_GetNetworkAddress(uint16_t stackSize);
//----
ActorMessage* new_r_Gateway_Stopped(uint16_t stackSize);
typedef struct b_Gateway_Label { uint16_t label; } b_Gateway_Label;
ActorMessage* new_r_Gateway_Label(uint16_t label, uint16_t stackSize);
typedef struct b_Gateway_NetworkAddress { uint32_t netAdr; } b_Gateway_NetworkAddress;
ActorMessage* new_r_Gateway_NetworkAddress(uint32_t netAdr, uint16_t stackSize);

Actor* new_Gateway(ActorRef *connection, ActorRef *rTable, ActorRef *connector, Router *receivers, uint16_t label, bool testMode);

#endif //MNVP_DRIVER_A_GATEWAY_H
