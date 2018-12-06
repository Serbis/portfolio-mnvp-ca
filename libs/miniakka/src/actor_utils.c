#include <string.h>
#include "../includes/actor_utils.h"
#include "../includes/router.h"
#include "../includes/fsm.h"

ActorSystem* AU_system(Actor* actor) {
    return (ActorSystem*) actor->context->system;
}

void* AU_fsm_data(Actor* actor) {
    return ((fsm_box*) actor->state)->data->data;
}

void AU_destroyActor(ActorCell *cell, bool fromMailBox) {
    if (cell->actor->beforeStop != NULL)
        cell->actor->beforeStop(cell->actor);

    pfree(cell->mailbox->messageQueue->mutex);
    pfree(cell->mailbox->messageQueue);

    pfree(cell->actor->context);
    pfree(cell->actor->state);
    pfree(cell->actor);


    pfree(cell->mailbox);
    pfree(cell->mutexTop);

    if (!fromMailBox) {
        MutexUnlock(cell->mutexBottom);
        pfree(cell->mutexBottom);
        pfree(cell);
    }
}

//Safely copies some ActorRef
ActorRef* AU_copyRef(ActorRef *ref) {
    if (ref != NULL) {
        ActorRef *dup_ref = pmalloc(sizeof(ActorRef));
        size_t actorNameLen = strlen(ref->actorName) + 1;
        char *dup_actorName = pmalloc(actorNameLen);

        memcpy(dup_actorName, ref->actorName, actorNameLen);
        memcpy(dup_ref, ref, sizeof(ActorRef));

        dup_ref->actorName = dup_actorName;

        return dup_ref;
    } else {
        return NULL;
    }
}

//Safely copies some Router
Router* AU_copyRouter(Router *router) {
    Router *dup_router = pmalloc(sizeof(Router));
    memcpy(dup_router, router, sizeof(Router));

    return dup_router;
}

void AU_freeMsg(ActorMessage *msg, bool destruct) {
    if (msg->destructor != NULL) {
        if (destruct) {
            msg->destructor(msg->body);
        }
    }
    if (msg->sender != NULL) {
        //ActorRef *r = msg->sender;
        AU_freeRef(msg->sender);
    }
    if (msg->body != NULL)
        pfree(msg->body);

    pfree(msg);
}

ActorMessage* AU_copyMsg(ActorMessage* msg, void* body) {
    ActorMessage *dup_msg = pmalloc(sizeof(ActorMessage));

    dup_msg->type = msg->type;
    dup_msg->body =  body;
    dup_msg->stackSize = msg->stackSize;
    dup_msg->destructor = msg->destructor;
    if(msg->sender == NULL)
        dup_msg->sender = NULL;
    //if(msg->sender != NULL)
    //    dup_msg->sender = AU_copyRef(msg->sender);

    return dup_msg;
}

//Safely free some ActorRef
void AU_freeRef(ActorRef *ref) {
    pfree(ref->actorName);
    pfree(ref);
}