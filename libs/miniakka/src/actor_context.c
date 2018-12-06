#include "../includes/actor_context.h"
#include "../../../oscl/include/malloc.h"



ActorContext* new_ActorContext(void* system, void *self) {
    ActorContext *actorContext = pmalloc(sizeof(ActorContext));
    actorContext->system = system;
    actorContext->sender = NULL;
    actorContext->self = self;
}