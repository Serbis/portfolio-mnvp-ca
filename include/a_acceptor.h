//
// Created by serbis on 08.07.18.
//

#ifndef MNVP_DRIVER_A_ACCEPTOR_H
#define MNVP_DRIVER_A_ACCEPTOR_H

#include "../libs/miniakka/includes/actor.h"

#define m_Accptor_AcceptTpc 1000

typedef struct b_AcceptTcp { ActorRef *tcpIo; char *host; uint16_t port; } b_AcceptTcp;
ActorMessage* new_m_Acceptor_AcceptTcp(ActorRef *tcpIo, char *host, uint16_t port, uint16_t stackSize);

Actor* new_Acceptor(ActorRef* gwPool);

#endif //MNVP_DRIVER_A_ACCEPTOR_H
