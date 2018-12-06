#include <string.h>
#include "../includes/scheduler.h"
#include "../includes/actor_ref.h"
#include "../includes/actor_utils.h"
#include "../../../oscl/include/time.h"
#include "../../../oscl/include/data.h"

typedef struct Scheduler_scheduledTaskArgs {
    Scheduler *scheduler;
    ActorRef *receiver;
    ActorRef *sender;
    ActorMessage *message;
    uint32_t delay;
    char *key;
    bool stopped;
    //mutex_t *mutex;
} Scheduler_scheduledTaskArgs;

char* Scheduler_randKey(Map *map) { //TODO не самое красивое решение. Это линейный алгорим, его можно время выполенния можно уменьшить введя элемент рандомизации
    char *str = pmalloc(11);

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

void Scheduler_scheduledTask(Scheduler_scheduledTaskArgs *args) {
    DelayMillis(args->delay);

    mutex_t *sMutex = args->scheduler->mutex;

    MutexLock(sMutex);
    if (!args->stopped) {
        MAP_remove(args->key, args->scheduler->tasks);
        args->receiver->tell(args->receiver, args->message, args->sender, args->scheduler->system);
    } else {
        AU_freeMsg(args->message, true);
    }

    MutexUnlock(sMutex);

    AU_freeRef(args->sender);
    AU_freeRef(args->receiver);
    pfree(args->key);
    pfree(args);
}

char* Scheduler_scheduleOnce(Scheduler *this, char* key, uint32_t delay, ActorRef *receiver, ActorMessage *message, ActorRef *sender) {
    //Task *task = pmalloc(sizeof(Task));
    //task->exec = (void (*)(void *)) Scheduler_scheduledTask;
    //task->requredStackSize = message->stackSize;

    Scheduler_scheduledTaskArgs *args = pmalloc(sizeof(Scheduler_scheduledTaskArgs));
    args->receiver = AU_copyRef(receiver);
    args->sender = AU_copyRef(sender);
    args->stopped = false;
    args->message = message;
    args->scheduler = this;
    if (delay < 100)
        args->delay = 100;
    else
        args->delay = delay;
    //args->mutex = NewMutex();

    MutexLock(this->mutex);
    char *fKey;
    if (key == NULL) {
        fKey = Scheduler_randKey(this->tasks);
    } else {
        if (!MAP_contain(key, this->tasks)) {
            fKey = strcpy2(key);
        } else {
            fKey = Scheduler_randKey(this->tasks);
        }

    }
    args->key = strcpy2(fKey);
    MAP_add(fKey, args, this->tasks);
    NewThread(Scheduler_scheduledTask, args, 64, strcpy2(fKey), 0);
    MutexUnlock(this->mutex);

    return fKey;
}

void Scheduler_cancel(Scheduler *this, char* key) {
    MutexLock(this->mutex);
    Scheduler_scheduledTaskArgs *args = MAP_get(key, this->tasks);
    if (args != NULL) {
        args->stopped = true;
        MAP_remove(key, this->tasks);
    }
    MutexUnlock(this->mutex);
}

Scheduler* new_Scheduler(void *system) {
    Scheduler *scheduler = pmalloc(sizeof(Scheduler));

    scheduler->system = system;
    scheduler->tasks = MAP_new();
    scheduler->mutex = NewMutex();
    scheduler->scheduleOnce = Scheduler_scheduleOnce;
    scheduler->cancel = Scheduler_cancel;

    return scheduler;
}
