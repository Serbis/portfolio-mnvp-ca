#include <stdlib.h>
#include "../includes/actor_cell.h"
#include "../../../oscl/include/malloc.h"
#include "../includes/dispatcher.h"
#include "../../../oscl/include/a_tpcio.h"
#include "../includes/actor_utils.h"

void ActorCell_receiveMessage(ActorCell* this, ActorMessage* msg) {
    bool destruct = this->actor->receive(this->actor, msg->type, msg->body);
    if (this->actor->context->sender != NULL) {
        AU_freeRef(this->actor->context->sender);
        this->actor->context->sender = NULL;
    }
    AU_freeMsg(msg, destruct);

    /*if (msg->destructor != NULL) {
        if (destruct) {
            msg->destructor(msg->body);
        }
    }
    pfree(msg->sender); //TODO уверен в этом?
    if (msg->body != NULL)
        pfree(msg->body);

    pfree(msg);*/
}

void ActorCell_invoke(ActorCell* this, ActorMessage* msg) {
    this->receiveMessage(this, msg);
}

bool ActorCell_sendMessage(ActorCell* this, ActorMessage* msg) {
    return ((Dispatcher*) this->dispatcher)->dispatch(this->dispatcher, this, msg);
}

void ActorCell_stop(ActorCell* this) {
    //MutexLock(this->mutexTop); ---> Уже заблокирован


    MutexLock(this->mutexBottom);

    this->stopped = true;

    MutexUnlock(this->mutexBottom);

    /*if (this->mailbox->canBeScheduled(this->mailbox) && this->mailbox->idle) {
        AU_destroyActor(this, false);
    } else {
        MutexUnlock(this->mutexBottom);
    }*/
}

ActorCell* new_ActorCell(Actor* actor, void* dispatcher) {
    ActorCell *actorCell = pmalloc(sizeof(ActorCell));
    actorCell->mailbox = new_MailBox(new_LQB(10));
    actorCell->mailbox->actor = actorCell;
    actorCell->dispatcher = dispatcher;
    actorCell->actor = actor;
    actorCell->stopper = (void *) 100;
    actorCell->stopped = false;
    actorCell->mutexTop = NewMutex();
    actorCell->mutexBottom = NewMutex();
    actorCell->receiveMessage = (void (*)(void*, ActorMessage*)) ActorCell_receiveMessage;
    actorCell->invoke = (void (*)(void*, ActorMessage*)) ActorCell_invoke;
    actorCell->sendMessage = (bool (*)(void*, ActorMessage*)) ActorCell_sendMessage;
    actorCell->stop = (void (*)(void *)) ActorCell_stop;

    return actorCell;
}