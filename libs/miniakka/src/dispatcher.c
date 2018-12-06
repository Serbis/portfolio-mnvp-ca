#include "../includes/dispatcher.h"
#include "../includes/actor_cell.h"
#include "../includes/standard_messages.h"
#include "../includes/actor_utils.h"

bool Dispatcher_dispatch(Dispatcher *this, ActorCell* receiver, ActorMessage* invocation) {
    /*if (invocation->type == m_Stop) {
        receiver->mailbox->enqueue(receiver->mailbox, receiver, invocation);
        receiver->stop(receiver);
        this->registerForExecution(this, receiver->mailbox, false);

        return true;
        //printf("%d\n", receiver->mailbox->canBeScheduled(receiver->mailbox) && receiver->mailbox->idle);
        //this->registerForExecution(this, receiver->mailbox, false);

        //return false;
        //return true;
    } else {*/
        receiver->mailbox->enqueue(receiver->mailbox, receiver, invocation);
        this->registerForExecution(this, receiver->mailbox, false, invocation->stackSize);

        return true;
    //}
}

void Dispatcher_registerForExecution(Dispatcher *this, MailBox *mbox, bool fromMailbox, uint16_t requiredStackSize) {
    //TODO Крайне вероятно, что любые операции с почтовым ящиком, нужно зыкрывать мьютексом
    ActorCell *cell = (ActorCell*) mbox->actor;

    //printf("(l0...");
    /*if (fromMailbox)
        MutexLock(cell->mutexTop);
    else
        MutexLock(cell->mutexBottom);*/

    if (mbox->canBeScheduled(mbox) && mbox->idle) {
        if (mbox->setAsScheduled(mbox)) {
            //printf("a\n");
            Task *task = pmalloc(sizeof(Task));
            task->exec = mbox->exec;
            task->requredStackSize = requiredStackSize;

            ExecBody *execBody = pmalloc(sizeof(ExecBody));
            execBody->mailBox = mbox;

            TaskArgs *taskArgs = pmalloc(sizeof(TaskArgs));
            taskArgs->threadStackSize = 0;
            taskArgs->innterArgs = execBody;

            task->args = taskArgs;

            this->executor->execute(this->executor, task);
            //printf("a1)...");
        } else {
            //printf("b\n");
        }
    } else {  //Если актор остановлен и больше нет сообщений в очерди произвести его удаление из памяти
        //printf("c...");
        if (cell->stopped) {
            //printf("x");
            //printf("stopped and frommailbox - %d\n", fromMailbox);
            //if (!fromMailbox)
            //    MutexLock(cell->mutexBottom);

            /*if (cell->actor->beforeStop != NULL)
                cell->actor->beforeStop(cell->actor);

            pfree(cell->mailbox->messageQueue->mutex);
            pfree(cell->mailbox->messageQueue);

            if (cell->actor->context->timers != NULL)
                pfree(cell->actor->context->timers);
            pfree(cell->actor->context);
            pfree(cell->actor->state);
            pfree(cell->actor);


            pfree(cell->mailbox);

            if (!fromMailbox) {
                //MutexUnlock(cell->mutexBottom);
                MutexUnlock(cell->mutexTop);
                pfree(cell->mutexBottom);
                pfree(cell->mutexTop);
                pfree(cell);
            } else {

            }*/
        } else {
            //printf("xxx\n");
        }
    }

    /*if (fromMailbox)
        MutexUnlock(cell->mutexTop);
    else
        MutexUnlock(cell->mutexBottom);*/
    //printf("l1)\n");
}


Dispatcher* new_Dispatcher(Executor* executor) {
    Dispatcher* dispatcher = pmalloc(sizeof(Dispatcher));
    dispatcher->executor = executor;
    dispatcher->throughputDeadlineTime = 3;
    dispatcher->throughput = 10;
    dispatcher->dispatch = (bool (*)(void *, void *, ActorMessage *)) Dispatcher_dispatch;
    dispatcher->registerForExecution = (void (*)(void*, MailBox*, bool, uint16_t)) Dispatcher_registerForExecution;

    return dispatcher;
}