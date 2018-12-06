//
// Created by serbis on 02.07.18.
//

#ifndef ACTORS_ACTOR_H
#define ACTORS_ACTOR_H

#include "messages.h"
#include "actor_context.h"

typedef struct Actor {
    void* state;
    ActorContext* context;
    bool (*receive) (void*, uint16_t, void* body);
    void (*beforeStop) (void*); //Used for free state data but not for the state pointer
} Actor;

Actor* new_Actor(bool (*receive)(Actor*, uint16_t, void*), void (*afterStop)(Actor*), void* state);

#endif //ACTORS_ACTOR_H
