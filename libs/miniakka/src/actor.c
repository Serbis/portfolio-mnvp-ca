#include "../includes/actor.h"

Actor* new_Actor(bool (*receive)(Actor*, uint16_t, void*), void (*beforeStop)(Actor*), void* state) {
    Actor *actor = pmalloc(sizeof(Actor));
    actor->receive = (bool (*)(void*, uint16_t, void*)) receive;
    actor->state = state;
    actor->context = new_ActorContext(NULL, NULL);
    actor->beforeStop = (void (*)(void *)) beforeStop;

    return actor;
}