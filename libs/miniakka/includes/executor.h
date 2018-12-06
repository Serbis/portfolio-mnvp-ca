//
// Created by serbis on 01.07.18.
//

#ifndef ACTORS_EXECUTOR_H
#define ACTORS_EXECUTOR_H

#include "../../collections/includes/lbq.h"
#include "../../collections/includes/list.h"
#include "../../../oscl/include/threads.h"
#include <stdbool.h>

typedef struct TaskArgs {
    uint16_t threadStackSize;
    void *innterArgs;
} TaskArgs;

typedef struct Task {
    TaskArgs *args;
    uint16_t requredStackSize;
    void (*exec)(void*);
} Task;

typedef struct Executor {
    LinkedBlockingQueue *taskQueue;
    List* threads;
    thread_t executorThread;
    mutex_t *mutex;
    bool alive;
    void (*execute)(void *t, Task*);

} Executor;

typedef struct ExecutorThread {
    uint16_t stackSize;
    bool idle;
    Task* task;
    thread_t thread;
    mutex_t *mutex;
    bool alive;
    bool stopped;
} ExecutorThread ;

Task* TASK_New();
void EXECUTOR_del(Executor *executor);
Executor* EXECUTOR_New(uint16_t qSize, uint16_t* tVector, uint16_t tVectorSize, char *name);

#endif //ACTORS_EXECUTOR_H
