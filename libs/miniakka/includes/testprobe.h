//
// Created by serbis on 05.07.18.
//

#ifndef ACTORS_TESTPROBE_H
#define ACTORS_TESTPROBE_H

#include <stdint.h>
#include "actor.h"
#include "actor_system.h"

typedef struct ProbeResult {
    bool seccess;
    void* result;
} ProbeResult;

typedef struct TestProbe {
    Actor *actorOject;
    ActorRef *ref;
    ActorSystem *testSystem;
    ActorSystem *targetSystem;
    char* name;
    void* (*expectMsgType)(void*, uint16_t, void* (*msgExtractor)(void*), uint32_t timeout);
    void (*send)(void*, ActorRef*, ActorMessage*);
    void (*reply)(void*, ActorMessage*);
    void (*capture)(void*);
    void (*destroy)(void*);
} TestProbe;

void* empty_extr(void *body);


TestProbe* new_TestProbe(ActorSystem *testSystem, ActorSystem *targetSystem, char* name);
TestProbe* new_LockedTestProbe(ActorSystem *testSystem, ActorSystem *targetSystem, char* name);

#endif //ACTORS_TESTPROBE_H
