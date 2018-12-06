//
// Created by serbis on 16.07.18.
//

#ifndef MNVP_DRIVER_A_CONNECTOR_H
#define MNVP_DRIVER_A_CONNECTOR_H

#include <stdint.h>
#include "../libs/miniakka/includes/messages.h"
#include "../libs/miniakka/includes/actor.h"
#include "../libs/miniakka/includes/router.h"

#define m_Connector_ConnectTcp 51697
#define r_Connector_AlreadyRun 16485

typedef struct b_Connector_ConnectTcp { ActorRef *tcpIo; char* host; uint16_t port; uint32_t retryMs; } b_Connector_ConnectTcp;
ActorMessage* new_m_Connector_ConnectTcp(ActorRef *tcpIo, char *host, uint16_t port, uint32_t retryMs, uint16_t stackSize);
//----
ActorMessage* new_r_Connector_AlreadyRun(uint16_t stackSize);

Actor* new_Connector(ActorRef* gwPool, ActorRef* rTable, Router* receivers);

#endif //MNVP_DRIVER_A_CONNECTOR_H
