//
// Created by serbis on 02.07.18.
//

#ifndef ACTORS_MAILBOX_H
#define ACTORS_MAILBOX_H

#include <stdbool.h>
#include "../../collections/includes/lbq.h"
#include "messages.h"


typedef struct MailBox {
    LinkedBlockingQueue* messageQueue;
    bool idle;
    void *actor; //ActorCell - set in a mailbox constructor
    bool shouldProcessMessage;
    void (*enqueue)(void*, void*, ActorMessage*);
    ActorMessage* (*dequeue)(void*);
    int32_t (*processMailbox)(void*, uint16_t, uint16_t, uint16_t);
    int32_t (*run)(void*, uint16_t);
    void (*exec)(void*);
    bool (*setAsScheduled)(void*);
    bool (*setAsIdle)(void*);
    bool (*canBeScheduled)(void*);

} MailBox;

typedef struct ExecBody {
    MailBox *mailBox;
} ExecBody;


MailBox* new_MailBox(LinkedBlockingQueue* messageQueue);

#endif //ACTORS_MAILBOX_H
