//
// Created by serbis on 10.07.18.
//

#ifndef MNVP_DRIVER_TIMERS_H
#define MNVP_DRIVER_TIMERS_H

#include <stdint.h>
#include "messages.h"

typedef struct Timers {
    char* (*startSingleTimer)(struct Timers*, void*, char* , ActorMessage*, uint32_t);
    void (*cancel)(struct Timers*, char*);
} Timers;

Timers *new_Timers();


#endif //MNVP_DRIVER_TIMERS_H
