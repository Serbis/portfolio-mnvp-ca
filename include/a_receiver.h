//
// Created by serbis on 15.07.18.
//

#include "../libs/miniakka/includes/messages.h"
#include "packet.h"
#include "../oscl/include/malloc.h"
#include "../libs/miniakka/includes/actor.h"

#ifndef MNVP_DRIVER_A_RECEIVER_H
#define MNVP_DRIVER_A_RECEIVER_H

#define m_Receiver_Receive 46325

typedef struct b_Receiver_Receive { Packet *packet; uint16_t gwLabel; uint32_t gwNetAdr; } b_Receiver_Receive;
ActorMessage* new_m_Receiver_Receive(Packet *packet, uint16_t gwLabel, uint32_t gwNetAdr, uint16_t stackSize);

Actor* new_Receiver(ActorRef* gwPool, ActorRef* rTable, ActorRef *trPool, uint32_t netAdr);

#endif //MNVP_DRIVER_A_RECEIVER_H
