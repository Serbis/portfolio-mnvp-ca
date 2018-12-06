//
// Created by serbis on 06.07.18.
//

#ifndef MNVP_DRIVER_BA_THERA_H
#define MNVP_DRIVER_BA_THERA_H

#include "../../libs/miniakka/includes/actor_ref.h"
#include "../../libs/miniakka/includes/actor_system.h"

#define r_BaThread_Error 39029
#define r_BaThread_Connected 34901

typedef struct BaThreadArgs {
    ActorRef *creator;
    ActorSystem *system;
    char* address;
    uint16_t port;
} BaThreadArgs;

ActorMessage* new_r_BaThread_Error(char *reason, uint16_t stackSize);

ActorMessage* new_r_BaThread_Connected(int socket, uint16_t stackSize);

void BaThread_run(void *args);

#endif //MNVP_DRIVER_BA_THERA_H
