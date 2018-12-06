//
// Created by serbis on 02.07.18.
//

#ifndef ACTORS_ACTOR_SYSTEM_H
#define ACTORS_ACTOR_SYSTEM_H

#include "actor.h"
#include "actor_ref.h"
#include "../../../oscl/include/malloc.h"
#include "dispatcher.h"
#include "scheduler.h"
#include "../../collections/includes/map2.h"

typedef struct ActorSystem {
    Dispatcher *dispatcher;
    Scheduler *scheduler;
    Map *actors;
    mutex_t *mutex;
    ActorRef* (*actorOf)(void*, Actor*, char*);
    void (*stop)(struct ActorSystem*);
} ActorSystem;

ActorSystem* new_ActorSystem(Dispatcher* dispatcher);

#endif //ACTORS_ACTOR_SYSTEM_H
