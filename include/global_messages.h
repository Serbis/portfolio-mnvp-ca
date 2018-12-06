//
// Created by serbis on 13.07.18.
//

#ifndef MNVP_DRIVER_GLOBAL_MESSAGES_H
#define MNVP_DRIVER_GLOBAL_MESSAGES_H

#include "../libs/miniakka/includes/messages.h"

#define m_MnvpGlobal_HeartBeat 31236

ActorMessage* new_m_MnvpGlobal_HeartBeat(uint16_t stackSize);

#endif //MNVP_DRIVER_GLOBAL_MESSAGES_H
