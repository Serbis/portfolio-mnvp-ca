#include <stdlib.h>
#include "../includes/mailbox.h"
#include "../../../oscl/include/malloc.h"
#include "../../../oscl/include/threads.h"
#include "../includes/actor_cell.h"
#include "../includes/dispatcher.h"
#include "../includes/standard_messages.h"
#include "../../../test/unit/include/acceptor_spec.h"
#include "../includes/actor_utils.h"

void MailBox_enqueue(MailBox *this, ActorCell* receiver, ActorMessage *msg) {
    this->messageQueue->enqueue(this->messageQueue, msg);
}

ActorMessage* MailBox_dequeue(MailBox *this) {
    if (this->messageQueue != NULL)
        return this->messageQueue->dequeue(this->messageQueue);
    else
        return NULL;
}

int32_t MailBox_processMailbox(MailBox *this, uint16_t left, uint16_t deadline, uint16_t curtaskStackSize) {
    int32_t ret = false;

    if (this->shouldProcessMessage) {
        bool shouldProcess = true;
        uint16_t inLeft = left;

        while (shouldProcess) {
            ActorMessage *next = this->dequeue(this);

            if (next != NULL) {
                if (next->stackSize <= curtaskStackSize) {
                    if (next->type != m_Stop) {
                        if (next->sender != NULL)
                            ((ActorCell *) this->actor)->actor->context->sender = AU_copyRef(next->sender);
                        else
                            ((ActorCell *) this->actor)->actor->context->sender = NULL;
                        ((ActorCell *) this->actor)->invoke(this->actor, next);
                        if /*(*/(inLeft > 1)/* && ((GetSystemTime() - deadline) < 0))*/ {
                            inLeft = (uint16_t) (inLeft - 1);
                        } else {
                            shouldProcess = false;
                        }

                        ret = 0;
                    } else {
                        AU_freeMsg(next, true);
                        ret = -1;
                        break;
                    }
                } else {
                    this->enqueue(this, this->actor, next);
                    ret = next->stackSize;
                    break;
                }
            } else {
                shouldProcess = false;
                //return true;
            }

        }
    } else {
        printf("bad\n");
    }

    return ret;
}

int32_t MailBox_run(MailBox *this, uint16_t curtaskStackSize) {
    return this->processMailbox(this, ((Dispatcher*)((ActorCell*) this->actor)->dispatcher)->throughput, ((Dispatcher*)((ActorCell*) this->actor)->dispatcher)->throughputDeadlineTime, curtaskStackSize);
}

void MailBox_exec(TaskArgs *args) {
    //if (body->stopper != NULL) {

        ExecBody *body = (ExecBody*) args->innterArgs;
        ActorCell *cell = (ActorCell *) body->mailBox->actor;
        //printf("(m0..");
        MutexLock(cell->mutexBottom);
        //if (!cell->stopped) {
        int32_t action = body->mailBox->run(body->mailBox, args->threadStackSize);


        if (action == 0) { //TODO механику запроса потока с большим стеком нужно очень хорошо проверить
            //Сценарий осутсвия новых сообщений для обработки
            body->mailBox->setAsIdle(body->mailBox);
            pfree(body);
            MutexUnlock(cell->mutexBottom);
        } else if (action == -1) {
            //Сценарий остановки актора
            ActorSystem *system = AU_system(cell->actor);
            MutexLock(system->mutex);
            MAP_remove(cell->actor->context->self->actorName, system->actors); //TODO тут поймана ошибка, похоже что имя удаляемого актора было где-то освобождено
            MutexUnlock(system->mutex);

            pfree(body);
            MutexUnlock(cell->mutexBottom);
            if (cell->actor->beforeStop != NULL)
                cell->actor->beforeStop(cell->actor);

            AU_freeRef(cell->actor->context->self);
            if (cell->actor->context->sender != NULL)
                AU_freeRef(cell->actor->context->sender);



            pfree(cell->actor->context);
            pfree(cell->actor->state);
            pfree(cell->actor);

            while (cell->mailbox->messageQueue->size(cell->mailbox->messageQueue) > 0) {
                ActorMessage *msg = cell->mailbox->dequeue(cell->mailbox);
                AU_freeMsg(msg, true);
            }
            del_LQB(cell->mailbox->messageQueue);

            pfree(cell->mailbox);
            pfree(cell->mutexTop);
            pfree(cell->mutexBottom);
            pfree(cell);



            //TODO что будет если система будет разрушена до выполнения данного кода?
            //TODO тут нет освобождения тайметов, добавть его
        } else {
            body->mailBox->setAsIdle(body->mailBox);
            ((Dispatcher *) ((ActorCell *) body->mailBox->actor)->dispatcher)->registerForExecution(((ActorCell *) body->mailBox->actor)->dispatcher, body->mailBox, true, (uint16_t) action);

            pfree(body);
            MutexUnlock(cell->mutexBottom);
            //Сценарий при коротом для обработки следующего сообщения у текущего потока недостаточно стека
        }


}

bool MailBox_setAsScheduled(MailBox *this) {
    //MutexLock(this->mutex);
    if (this->idle) {
        this->idle = false;
        //MutexUnlock(this->mutex);
        return true;
    } else {
        //MutexUnlock(this->mutex);
        return false;
    }
}

bool MailBox_setAsIdle(MailBox *this) {
    //MutexLock(this->mutex);
    if (!this->idle) {
        this->idle = true;
        //MutexUnlock(this->mutex);
        return true;
    } else {
        //MutexUnlock(this->mutex);
        return false;
    }
}

bool MailBox_canBeScheduled(MailBox *this) {
    //MutexLock(this->mutex);
    if (this->messageQueue != NULL) {
        //MutexUnlock(this->mutex);
        return this->messageQueue->size(this->messageQueue) > 0;
    } else {
        //MutexUnlock(this->mutex);
        return false;
    }
}

MailBox* new_MailBox(LinkedBlockingQueue* messageQueue) {
    MailBox *mailBox = pmalloc(sizeof(MailBox));
    mailBox->messageQueue = messageQueue;
    mailBox->idle = true;
    mailBox->actor = NULL; //ActorCell
    mailBox->shouldProcessMessage = true;

    mailBox->enqueue = (void (*)(void*, void*, ActorMessage*)) MailBox_enqueue;
    mailBox->dequeue = (ActorMessage*(*)(void*)) MailBox_dequeue;
    mailBox->processMailbox = (int32_t (*)(void*, uint16_t, uint16_t, uint16_t)) MailBox_processMailbox;
    mailBox->run = (int32_t (*)(void*, uint16_t)) MailBox_run;
    mailBox->exec = (void (*)(void*)) MailBox_exec;
    mailBox->setAsScheduled = (bool (*)(void*)) MailBox_setAsScheduled;
    mailBox->setAsIdle = (bool (*)(void*)) MailBox_setAsIdle;
    mailBox->canBeScheduled = (bool (*)(void*)) MailBox_canBeScheduled;

    return mailBox;
}