//
// Created by serbis on 02.07.18.
//

#ifndef ACTORS_ACTOR_CELL_H
#define ACTORS_ACTOR_CELL_H

#include "messages.h"
#include "actor.h"
#include "mailbox.h"

typedef struct ActorCell {
    MailBox* mailbox;
    Actor* actor;
    mutex_t *mutexTop;
    mutex_t *mutexBottom;
    void *stopper;
    bool stopped;
    void *dispatcher; //Dispatcher*
    void (*receiveMessage)(void*, ActorMessage*);
    void (*invoke)(void*, ActorMessage*);
    bool (*sendMessage)(void*, ActorMessage*);
    void (*stop)(void*);
} ActorCell;

ActorCell* new_ActorCell(Actor* actor, void* dispatcher);

#endif //ACTORS_ACTOR_CELL_H
