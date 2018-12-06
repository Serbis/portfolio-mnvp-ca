//
// Created by serbis on 02.07.18.
//

#include <stdint.h>
#include <stdbool.h>

#ifndef ACTORS_MESSAGES_H
#define ACTORS_MESSAGES_H

typedef uint16_t MessageType;

typedef struct ActorMessage {
    MessageType type;
    void *sender;
    void *body;
    uint16_t stackSize;
    void (*destructor)(void*);
} ActorMessage;

ActorMessage* new_ActorMessage(MessageType type, void *body, uint16_t stackSize, void (*destructor)(void*));

#endif //ACTORS_MESSAGES_H
