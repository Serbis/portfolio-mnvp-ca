//
// Created by serbis on 06.07.18.
//

#ifndef MNVP_DRIVER_A_TCP_CONNECTION_CREATOR_H
#define MNVP_DRIVER_A_TCP_CONNECTION_CREATOR_H

#include "../../libs/miniakka/includes/messages.h"
#include "../../libs/miniakka/includes/actor.h"

//TODO неправильно тут сделано. Connection это унифицирванное сообщение, оно не должно быть привязано к конкретной реалзиции

#define m_TcpConnectionCreator_Create 41624
#define r_TcpConnectionCreator_Connection 54678
#define r_TcpConnectionCreator_ConnectionFailed 17586
#define r_TcpConnectionCreator_BindFailed 15498

ActorMessage* new_m_TcpConnectionCreator_Create(char *address, uint16_t port, uint16_t stackSize);

typedef struct b_TcpConnectionCreator_Connection { ActorRef *ref; } b_TcpConnectionCreator_Connection;
ActorMessage* new_r_TcpConnectionCreator_Connection(ActorRef *ref, uint16_t stackSize);

typedef struct b_TcpConnectionCreator_BindFailed { char *reason; } b_TcpConnectionCreator_BindFailed;
ActorMessage* new_r_TcpConnectionCreator_BindFailed(char *reason, uint16_t stackSize);

typedef struct b_TcpConnectionCreator_ConnectionFailed { char *reason; } b_TcpConnectionCreator_ConnectionFailed;
ActorMessage* new_r_TcpConnectionCreator_ConnectionFailed(char *reason, uint16_t stackSize);

Actor* new_TcpConnectionCreatorActor(void *selfRef);

#endif //MNVP_DRIVER_A_TCP_CONNECTION_CREATOR_H
