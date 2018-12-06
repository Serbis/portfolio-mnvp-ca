#include "actor.h"

#ifndef ACTORS_SIMPLE_ACTOR_H
#define ACTORS_SIMPLE_ACTOR_H

typedef struct b_SimpleActorBody { char *somestring; uint16_t somenumber; } b_AcceptTcp;

ActorMessage* new_DoWorkMsg(uint16_t workNumber);

Actor* new_SimpleActor(int8_t r);

#endif //ACTORS_SIMPLE_ACTOR_H
