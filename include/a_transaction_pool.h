//
// Created by serbis on 27.07.18.
//

#ifndef MNVP_DRIVER_A_TRANSACTION_POOL_H
#define MNVP_DRIVER_A_TRANSACTION_POOL_H

#include "../libs/miniakka/includes/actor.h"
#include "packet.h"

#define  m_TransactionPool_PoolAndExec 16548
#define  m_TransactionPool_Route 15467
#define  m_TransactionPool_Get 20511

#define  r_TransactionPool_Transaction 32136

typedef struct b_TransactionPool_PoolAndExec { ActorRef *transaction; } b_TransactionPool_PoolAndExec;
ActorMessage* new_m_TransactionPool_PoolAndExec(ActorRef *transaction, uint16_t stackSize);
typedef struct b_TransactionPool_Route { Packet *packet; } b_TransactionPool_Route;
ActorMessage* new_m_TransactionPool_Route(Packet *packet, uint16_t stackSize);
typedef struct b_TransactionPool_Get { uint32_t tid; } b_TransactionPool_Get;
ActorMessage* new_m_TransactionPool_Get(uint32_t tid, uint16_t stackSize);

typedef struct b_TransactionPool_Transaction { ActorRef *ref; } b_TransactionPool_Transaction;
ActorMessage* new_r_TransactionPool_Transaction(ActorRef *ref, uint16_t stackSize);

Actor* new_TransactionPool();

#endif //MNVP_DRIVER_A_TRANSACTION_POOL_H
