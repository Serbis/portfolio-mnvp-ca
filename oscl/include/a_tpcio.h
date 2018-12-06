//
// Created by serbis on 06.07.18.
//

#ifndef MNVP_DRIVER_A_TPCIO_H
#define MNVP_DRIVER_A_TPCIO_H

#include "../../libs/miniakka/includes/messages.h"
#include "../../libs/miniakka/includes/actor.h"

#define m_TcpIO_BindAndAccept 27593
#define m_TcpIO_Connect 21684

typedef struct b_TcpIo_BindAndAccept { char *address; uint16_t port; } b_TcpIo_BindAndAccept;
ActorMessage* new_m_TcpIO_BindAndAcceptMsg(char *address, uint16_t port, uint16_t stackSize);
typedef struct b_TcpIo_Connect { char *host; uint16_t port; } b_TcpIo_Connect;
ActorMessage* new_m_TcpIO_Connect(char *host, uint16_t port, uint16_t stackSize);


Actor* new_TcpIOActor();

#endif //MNVP_DRIVER_A_TPCIO_H
