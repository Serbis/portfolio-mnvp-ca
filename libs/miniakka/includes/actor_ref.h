//
// Created by serbis on 02.07.18.
//

#ifndef ACTORS_LOCALACTORREF_H
#define ACTORS_LOCALACTORREF_H

#include "messages.h"
#include "dispatcher.h"

typedef struct ActorRef {
    void *actorCell;
    char *actorName;
    void (*tell)(void*, ActorMessage*, void*, void*);
} ActorRef;

ActorRef* new_LocalActorRef(void* actor, void* dispatcher, char *actorName);

#endif //ACTORS_LOCALACTORREF_H
