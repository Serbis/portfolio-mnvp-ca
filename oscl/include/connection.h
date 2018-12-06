//
// Created by serbis on 08.07.18.
//

#ifndef MNVP_DRIVER_CONNECTION_H
#define MNVP_DRIVER_CONNECTION_H

#include <stdint.h>
#include "../../libs/miniakka/includes/actor_ref.h"

#define m_Connection_SetDataHandler 3554
#define m_Connection_SendData 21465
#define m_Connection_Close 54654

#define r_Connection_Data 53746
#define r_Connection_ConnectionClosed 32465

typedef struct b_Connection_SetDataHandler { ActorRef *handler; } b_Connection_SetDataHandler;
ActorMessage* new_m_Connection_SetDataHandler(ActorRef *ref, uint16_t stackSize);

typedef struct b_Connection_SendData { uint8_t *data; uint32_t size; } b_Connection_SendData;
ActorMessage* new_m_Connection_SendData(uint8_t *data, uint32_t size, uint16_t stackSize);

ActorMessage* new_m_Connection_Close(uint16_t stackSize);

typedef struct b_Connection_Data { uint8_t *data; uint32_t size; } b_Connection_Data;
ActorMessage* new_r_Connection_Data(uint8_t *data, uint32_t size, uint16_t stackSize);

ActorMessage* new_r_Connection_ConnectionClosed(uint16_t stackSize);

#endif //MNVP_DRIVER_CONNECTION_H
