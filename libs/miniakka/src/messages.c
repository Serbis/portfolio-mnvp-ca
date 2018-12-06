#include "../includes/messages.h"
#include "../../../oscl/include/malloc.h"

ActorMessage* new_ActorMessage(MessageType type, void *body, uint16_t stackSize, void (*destructor)(void*)) {
    ActorMessage *actorMessage = pmalloc(sizeof(ActorMessage));
    actorMessage->type = type;
    actorMessage->body = body;
    actorMessage->stackSize = stackSize;
    actorMessage->sender = NULL;
    actorMessage->destructor = destructor;

    return actorMessage;
}