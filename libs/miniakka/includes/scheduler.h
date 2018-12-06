//
// Created by serbis on 10.07.18.
//

#ifndef MNVP_DRIVER_SCHEDULER_H
#define MNVP_DRIVER_SCHEDULER_H

#include "actor_ref.h"
#include "../../collections/includes/map2.h"


typedef struct ScheduledTask {

} ScheduledTask;

typedef struct Scheduler {
    void *system;
    Map *tasks;
    mutex_t *mutex;
    char* (*scheduleOnce)(void*, char*, uint32_t, ActorRef*, ActorMessage*, ActorRef*);
    void (*cancel)(struct Scheduler*, char*);
} Scheduler;

Scheduler* new_Scheduler(void *system);

#endif //MNVP_DRIVER_SCHEDULER_H
