//
// Created by serbis on 08.07.18.
//

#ifndef MNVP_DRIVER_A_GATEWAY_POOL_H
#define MNVP_DRIVER_A_GATEWAY_POOL_H

#include "../libs/miniakka/includes/actor.h"

#define  m_GatewayPool_Add 1001
#define  m_GatewayPool_GetByLabel 1002
#define  m_GatewayPool_RemoveByLabel 1003
#define  m_GatewayPool_RouteToAll 1004
#define  m_GatewayPool_RouteByLabel 1005
#define  r_GatewayPool_Gateway 1006

typedef struct b_GatewayPool_Add { ActorRef *ref; } b_GatewayPool_Add;
ActorMessage* new_m_GatewayPool_Add(ActorRef *ref, uint16_t stackSize);
typedef struct b_GatewayPool_GetByLabel { uint32_t label; } b_GatewayPool_GetByLabel;
ActorMessage* new_m_GatewayPool_GetByLabel(uint32_t label, uint16_t stackSize);
typedef struct b_GatewayPool_RemoveByLabel { uint32_t label; } b_GatewayPool_RemoveByLabel;
ActorMessage* new_m_GatewayPool_RemoveByLabel(uint32_t label, uint16_t stackSize);
typedef struct b_GatewayPool_RouteToAll { ActorMessage *message; } b_GatewayPool_RouteToAll;
ActorMessage* new_m_GatewayPool_RouteToAll(ActorMessage *message, uint16_t stackSize);
typedef struct b_GatewayPool_RouteByLabel { uint16_t gwLabel; ActorMessage *message; } b_GatewayPool_RouteByLabel;
ActorMessage* new_m_GatewayPool_RouteByLabel(uint16_t gwLabel, ActorMessage *message, uint16_t stackSize);

typedef struct b_GatewayPool_Gateway { ActorRef *ref; } b_GatewayPool_Gateway;
ActorMessage* new_r_GatewayPool_Gateway(ActorRef *ref, uint16_t stackSize);


Actor* new_GatewayPool();

#endif //MNVP_DRIVER_A_GATEWAY_POOL_H
