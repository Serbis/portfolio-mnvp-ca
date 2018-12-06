//
// Created by serbis on 02.07.18.
//

#ifndef ACTORS_DISPATCHER_H
#define ACTORS_DISPATCHER_H

#include "executor.h"
#include "mailbox.h"
#include "../../../oscl/include/malloc.h"
#include <stdbool.h>


typedef struct Dispatcher {
    Executor *executor;
    uint16_t throughputDeadlineTime;
    uint16_t throughput;
    bool (*dispatch)(void*, void*, ActorMessage*);
    void (*registerForExecution)(void*, MailBox*, bool, uint16_t);
} Dispatcher;

Dispatcher* new_Dispatcher(Executor* executor);

#endif //ACTORS_DISPATCHER_H
