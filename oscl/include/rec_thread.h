//
// Created by serbis on 06.07.18.
//

#ifndef MNVP_DRIVER_REC_THREAD_H
#define MNVP_DRIVER_REC_THREAD_H

#include "../../libs/miniakka/includes/actor_ref.h"
#include "../../libs/miniakka/includes/actor_system.h"

#define r_RecThread_Closed 21614
#define r_RecThread_Data 31366

typedef struct RecThreadArgs {
    ActorRef *creator;
    ActorSystem *system;
    int socket;
    uint16_t blockSize;
    bool stop;
} RecThreadArgs;


ActorMessage* new_r_RecThread_Closed();

typedef struct b_RecThread_Data {
    uint8_t *data;
    uint32_t size;
} b_RecThread_Data;

ActorMessage* new_r_RecThread_Data(uint8_t *data, uint16_t size, uint16_t stackSize);


void RecThread_run(void *args);

#endif //MNVP_DRIVER_REC_THREAD_H
