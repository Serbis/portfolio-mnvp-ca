//
// Created by serbis on 03.07.18.
//

#ifndef ACTORS_ACTOR_CONTEXT_H
#define ACTORS_ACTOR_CONTEXT_H


#include "actor_ref.h"
#include "timers.h"

typedef struct ActorContext {
    ActorRef *self;
    ActorRef *sender;
    void *system;
} ActorContext;

ActorContext* new_ActorContext(void* system, void *self);

#endif //ACTORS_ACTOR_CONTEXT_H
