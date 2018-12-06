#include <stdlib.h>
#include "../includes/executor.h"
#include "../../../oscl/include/malloc.h"
#include "../../../oscl/include/time.h"
#include "../../../oscl/include/data.h"

bool findft(void* ethread) {
    ExecutorThread *t = (ExecutorThread*) ethread;

    if (MutexTryLock(t->mutex) == 0) {
        if (t->idle == true) {
            MutexUnlock(t->mutex);
            return true;
        } else {
            MutexUnlock(t->mutex);
            return false;
        }
    } else {
        return false;
    }
}

ExecutorThread* findFreeThread(Executor *executor) {
    ExecutorThread *finded = NULL;
    while (finded == NULL) {
        finded = executor->threads->find(executor->threads, findft);
        if (finded == NULL) {
            //printf("-\n");
            //DelayMillis(500);
        }
    }

    return finded;
}

/**
 * Planned some new task to executor.
 *
 * WARNING - task pointer is free up in execution thread
 */
void executorExecute(void *self, Task *task) {
    //printf("(e0..");
    Executor *this = (Executor*) self;
    //MutexLock(this->mutex);
    this->taskQueue->enqueue(this->taskQueue, task);
    //MutexUnlock(this->mutex);
    //printf("e1)..");
}


void executorRun(void *args) {
    Executor *this = (Executor*) args;

    //printf("I am main executor thread\n");
    while(this->alive) {
        MutexLock(this->mutex);
        if (this->taskQueue->size(this->taskQueue) > 0) {
            Task *task = (Task *) this->taskQueue->dequeue(this->taskQueue);
            ExecutorThread *thread = findFreeThread(this);
            thread->idle = false;
            thread->task = task;
        }
        MutexUnlock(this->mutex);
    }
}

void threadRun(void *args) {
    ExecutorThread *this = (ExecutorThread*) args;
    while(true) {
        MutexLock(this->mutex);
        if (this->task != NULL) {
            this->task->args->threadStackSize = this->stackSize;
            this->task->exec(this->task->args);
            pfree(this->task->args);
            pfree(this->task);
            this->idle = true;
            this->task = NULL;
        }
        if (!this->alive) {
            this->stopped = true;
            MutexUnlock(this->mutex);
            break;
        }
        MutexUnlock(this->mutex);
    }
}


Task* TASK_New() {
    Task *task = (Task*) malloc(sizeof(Task));

    return task;

}

void EXECUTOR_del(Executor *executor) {
    MutexLock(executor->mutex);
    executor->alive = false;
    ListIterator *iterator = executor->threads->iterator(executor->threads);
    while (iterator->hasNext(iterator)) {
        ExecutorThread *thread = iterator->next(iterator);
        MutexLock(thread->mutex);
        thread->alive = false;
        MutexUnlock(thread->mutex);
    }
    pfree(iterator);

    //Ждем пока все потоки не перейдут в состояние idle
    while (true) {
        iterator = executor->threads->iterator(executor->threads);
        uint16_t stopped = 0;
        uint16_t size = executor->threads->size;

        while (iterator->hasNext(iterator)) {
            ExecutorThread *thread = iterator->next(iterator);

            if (thread->stopped)
                stopped++;
        }
        pfree(iterator);

        if (stopped == size) {
            break;
        } else {
            printf("Await stop all threads\n");
            DelayMillis(50);
        }
    }

    iterator = executor->threads->iterator(executor->threads);
    while (iterator->hasNext(iterator)) {
        ExecutorThread *thread = iterator->next(iterator);
        MutexLock(thread->mutex);
        MutexUnlock(thread->mutex);
        pfree(thread->mutex);
        pfree(thread);
        iterator->remove(iterator);
    }
    pfree(iterator);

    del_List(executor->threads);
    del_LQB(executor->taskQueue);

    MutexUnlock(executor->mutex);

    pfree(executor->mutex);
    pfree(executor);
}

Executor* EXECUTOR_New(uint16_t qSize, uint16_t* tVector, uint16_t tVectorSize, char *name) {
    Executor *executor = pmalloc(sizeof(Executor));
    executor->taskQueue = new_LQB(qSize);
    executor->threads = new_List();
    executor->execute = executorExecute;
    executor->mutex = NewMutex();
    executor->alive = true;

    for (uint16_t i = 0; i < tVectorSize; i++) {
        ExecutorThread * thread = pmalloc(sizeof(ExecutorThread));
        thread->stackSize = tVector[i];
        thread->task = NULL;
        thread->idle = true;
        thread->alive = true;
        thread->stopped = false;
        thread->mutex = NewMutex();
        executor->threads->prepend(executor->threads, thread);

        char *tNum = itoa(i);
        size_t tNameSize = strlen(tNum) + strlen(name) + 1;
        char *tName = pmalloc(tNameSize);
        snprintf(tName, tNameSize, "%s%s", name, tNum);
        thread_t t = NewThread(threadRun, thread, tVector[i], tName, 0);
        pfree(tNum);
        thread->thread = t;
    }

    thread_t t = NewThread(executorRun, executor, 64, strcpy2(name), 0);
    executor->executorThread = t;
    return executor;
}