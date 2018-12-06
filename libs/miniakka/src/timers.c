#include "../includes/timers.h"
#include "../../../oscl/include/malloc.h"
#include "../includes/actor_system.h"

char* Timers_startSingleTimer(Timers *this, ActorContext *ctx, char *key, ActorMessage* msg, uint32_t  timeout) {
    ActorSystem *system = (ActorSystem*) ctx->system;

    system->scheduler->scheduleOnce(system->scheduler, timeout, ctx->self, msg, system->dispatcher->executor, ctx->self);
}

void Timers_cancel(Timers *this, char *key) {

}

Timers *new_Timers() {
    Timers *timers = pmalloc(sizeof(Timers));

    timers->startSingleTimer =  Timers_startSingleTimer;
    timers->cancel =  Timers_cancel;

    return timers;
}