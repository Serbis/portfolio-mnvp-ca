//
// Created by serbis on 08.07.18.
//

#include <stddef.h>
#include "messages.h"

#ifndef MNVP_DRIVER_STANDARD_MESSAGES_H
#define MNVP_DRIVER_STANDARD_MESSAGES_H

#define m_Stop 100

ActorMessage* new_m_Stop(uint16_t stackSize);

void del_default(void *b);


void *new_DefaultBody();

#endif //MNVP_DRIVER_STANDARD_MESSAGES_H
