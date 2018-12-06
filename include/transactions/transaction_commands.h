//
// Created by serbis on 27.07.18.
//

#ifndef MNVP_DRIVER_TRANSACTION_COMMANDS_H
#define MNVP_DRIVER_TRANSACTION_COMMANDS_H

#include <stdint.h>
#include "../../libs/miniakka/includes/messages.h"
#include "../packet.h"

#define  m_Transaction_Exec 13156
#define  m_Transaction_ReceivePacket 16361
#define  r_Transaction_TransactionFinished 16447

typedef struct b_Transaction_Exec { uint32_t tid; } b_Transaction_Exec;
ActorMessage* new_m_Transaction_Exec(uint32_t tid, uint16_t stackSize);
typedef struct b_Transaction_ReceivePacket { Packet *packet; } b_Transaction_ReceivePacket;
ActorMessage* new_m_Transaction_ReceivePacket(Packet *packet, uint16_t stackSize);
typedef struct b_Transaction_TransactionFinished { uint32_t tid; } b_Transaction_TransactionFinished;
ActorMessage* new_r_Transaction_TransactionFinished(uint32_t tid, uint16_t stackSize);

#endif //MNVP_DRIVER_TRANSACTION_COMMANDS_H
