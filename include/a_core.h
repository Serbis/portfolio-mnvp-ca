//
// Created by serbis on 22.07.18.
//

#ifndef MNVP_DRIVER_A_CORE_H
#define MNVP_DRIVER_A_CORE_H

#include "../libs/miniakka/includes/messages.h"
#include "../libs/miniakka/includes/actor.h"
#include "packet.h"

#define m_Core_Hello 15552
#define m_Core_Preq 32145
#define m_Core_SendPacket 45641

//>>>
ActorMessage* new_m_Core_Hello(uint16_t stackSize);
typedef struct b_Core_Preq { uint32_t dest; uint32_t timeout; uint16_t minFind; uint16_t ttl; } b_Core_Preq;
ActorMessage* new_m_Core_Preq(uint32_t dest, uint32_t timeout, uint16_t minFind, uint16_t ttl, uint16_t stackSize);
typedef struct b_Core_SendPacket { Packet *packet; } b_Core_SendPacket;
ActorMessage* new_m_Core_SendPacket(Packet *packet, uint16_t stackSize);

Actor* new_Core(ActorRef* gwPool, ActorRef* trPool, ActorRef* rTable, uint32_t netAdr);

#endif //MNVP_DRIVER_A_CORE_H
