//
// Created by serbis on 17.07.18.
//

#ifndef MNVP_DRIVER_HEART_H
#define MNVP_DRIVER_HEART_H

#include "../libs/miniakka/includes/messages.h"

#define r_Heart_HeartBeat 51945

ActorMessage* new_r_Heart_HeartBeat(uint16_t stackSize);

#endif //MNVP_DRIVER_HEART_H
