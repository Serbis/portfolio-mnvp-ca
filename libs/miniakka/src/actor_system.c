#include <string.h>
#include "../includes/actor_system.h"
#include "../includes/actor_context.h"
#include "../../collections/includes/map2.h"
#include "../../../oscl/include/data.h"
#include "../includes/actor_utils.h"
#include "../../../oscl/include/time.h"

char* ActorSystem_randName(Map *map) { //TODO не самое красивое решение. Это линейный алгорим, его можно время выполенния можно уменьшить введя элемент рандомизации
    char *str = (char*) pmalloc(11);

    for (int i = 0; i < 10; i++) {
        for (char j = 97; j <= 122; j++ ) {
            str[i] = j;
            if (!MAP_contain(str, map)) {
                size_t size = strlen(str) + 1;
                char *nstr = pmalloc(size);
                memcpy(nstr, str, size);
                pfree(str);
                return nstr;
            }

        }
    }

    return NULL;
}

ActorRef* ActorSystem_actorOf(ActorSystem* this, Actor *actor, char *actorName) {
    char *finalActorName;
    MutexLock(this->mutex);
    if (actorName == NULL) {
        finalActorName = ActorSystem_randName(this->actors);
    } else {
        if (!MAP_contain(actorName, this->actors)) {
            finalActorName = strcpy2(actorName);
        } else {
            MutexUnlock(this->mutex);
            return NULL; //TODO нужно придумать, как кросплотфоорменно сообщать о том, что такое имя актора уже зарегистрировано
        }
    }
    MutexUnlock(this->mutex);

    MAP_add(finalActorName, NULL, this->actors);

    ActorRef *ref = new_LocalActorRef(actor, this->dispatcher, finalActorName);
    actor->context->system = this;
    actor->context->self = AU_copyRef(ref);

    pfree(finalActorName);
    return ref;
}

void ActorSystem_stop(ActorSystem* this) {
    while (true) {
        MutexLock(this->mutex);
        if (this->actors->inner->size == 0) {
            MutexUnlock(this->mutex);
            break;
        } else {
            MutexUnlock(this->mutex);
            printf("Exist\n");
            DelayMillis(200);
        }
    }

    ListIterator *iterator = this->actors->inner->iterator(this->actors->inner);
    while(iterator->hasNext(iterator)) {
        iterator->remove;
    }
    pfree(iterator);
    MAP_del(this->actors);

    pfree(this->mutex);
    pfree(this->scheduler->mutex);

    iterator = this->scheduler->tasks->inner->iterator(this->scheduler->tasks->inner);
    while(iterator->hasNext(iterator)) {
        iterator->remove;
    }
    pfree(iterator);
    MAP_del(this->scheduler->tasks);

    pfree(this->scheduler);

    EXECUTOR_del(this->dispatcher->executor);
    pfree(this->dispatcher);
    //TODO вот тут нужно будет освободить эксекутор
}

ActorSystem* new_ActorSystem(Dispatcher* dispatcher) {
    ActorSystem* actorSystem = pmalloc(sizeof(ActorSystem));
    actorSystem->dispatcher = dispatcher;
    actorSystem->actors = MAP_new();
    actorSystem->mutex = NewMutex();
    actorSystem->actorOf = (ActorRef* (*)(void*, Actor*, char*)) ActorSystem_actorOf;
    actorSystem->stop = ActorSystem_stop;
    actorSystem->scheduler = new_Scheduler(actorSystem);

    return actorSystem;
}