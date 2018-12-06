#include <string.h>
#include "../include/a_tcp_connection_creator.h"
#include "../../libs/miniakka/includes/messages.h"
#include "../include/malloc.h"
#include "../include/ba_therad.h"
#include "../include/rec_thread.h"
#include "../../libs/miniakka/includes/actor_system.h"
#include "../include/tcp_connection.h"
#include "../../libs/miniakka/includes/actor_utils.h"


//  ||| Create ||| +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

typedef struct b_Create {
    char *address;
    uint16_t port;
} b_Create;

void del_m_TcpConnectionCreator_Create(b_Create *msg) {
    pfree(msg->address);
}

ActorMessage* new_m_TcpConnectionCreator_Create(char *address, uint16_t port, uint16_t stackSize) {
    b_Create *body = pmalloc(sizeof(b_Create));
    body->address = pmalloc(sizeof strlen(address) + 1);
    strcpy(body->address, address);
    body->port = port;

    return new_ActorMessage(m_TcpConnectionCreator_Create, body, stackSize, del_m_TcpConnectionCreator_Create);
}

void mp_TcpConnectionCreator_CreateMsg(Actor *this, b_Create *msg) {
    BaThreadArgs *args = pmalloc(sizeof(BaThreadArgs));

    args->address = pmalloc(sizeof(msg->address) + 1);
    args->port = msg->port;

    strcpy(args->address, msg->address);
    ActorRef *nSelf = AU_copyRef(this->context->self);

    args->creator = nSelf;
    args->system = this->context->system;

    NewThread(BaThread_run, args, 0, NULL, 0);
}

//  ||| Resp - Connection ||| +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void del_m_TcpConnectionCreator_Connection(b_TcpConnectionCreator_Connection *b) {
    pfree(b->ref);
}

ActorMessage* new_r_TcpConnectionCreator_Connection(ActorRef *ref, uint16_t stackSize) {
    b_TcpConnectionCreator_Connection *b = pmalloc(sizeof(b_TcpConnectionCreator_Connection));
    b->ref = AU_copyRef(ref);

    return new_ActorMessage(r_TcpConnectionCreator_Connection, b, stackSize, del_m_TcpConnectionCreator_Connection);
}

//  ||| Resp - BindFailed ||| +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void del_m_TcpConnectionCreator_BindFailed(b_TcpConnectionCreator_BindFailed *b) {
    pfree(b->reason);
}

ActorMessage* new_r_TcpConnectionCreator_BindFailed(char *reason, uint16_t stackSize) {
    b_TcpConnectionCreator_BindFailed *b = pmalloc(sizeof(b_TcpConnectionCreator_BindFailed));
    char *dup_reason = pmalloc(strlen(reason) + 1);
    memcpy(dup_reason, reason, strlen(reason) + 1);
    b->reason = dup_reason;

    return new_ActorMessage(r_TcpConnectionCreator_BindFailed, b, stackSize, del_m_TcpConnectionCreator_BindFailed);
}

//  ||| Resp - BindFailed ||| +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void del_m_TcpConnectionCreator_ConnectionFailed(b_TcpConnectionCreator_ConnectionFailed *b) {
    pfree(b->reason);
}

ActorMessage* new_r_TcpConnectionCreator_ConnectionFailed(char *reason, uint16_t stackSize) {
    b_TcpConnectionCreator_ConnectionFailed *b = pmalloc(sizeof(b_TcpConnectionCreator_ConnectionFailed));
    char *dup_reason = pmalloc(strlen(reason) + 1);
    memcpy(dup_reason, reason, strlen(reason) + 1);
    b->reason = dup_reason;

    return new_ActorMessage(r_TcpConnectionCreator_ConnectionFailed, b, stackSize, del_m_TcpConnectionCreator_ConnectionFailed);
}

//  ||| BaThread_Error ||| +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void mp_BaThread_Error(Actor *this, char *reason) {
    ActorRef *origintator = (ActorRef*)*(size_t*) this->state;
    char *nReason = pmalloc(strlen(reason) + 1);
    strcpy(nReason, reason);
    origintator->tell(origintator, new_r_TcpConnectionCreator_BindFailed(nReason, 64), this->context->self, this->context->system);
}

//  ||| BaThread_Connected ||| +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void mp_BaThread_Connected(Actor *this, int *socket) {
    RecThreadArgs *tArgs = pmalloc(sizeof(RecThreadArgs));
    tArgs->blockSize = 32000;
    tArgs->socket = *socket;
    tArgs->stop = false;
    tArgs->creator = NULL;
    ActorRef *ref = ((ActorSystem*)this->context->system)->actorOf(this->context->system, new_TcpConnectionActor(tArgs), NULL);
    tArgs->creator = ref;

    ActorRef *origintator = (ActorRef*)*(size_t*) this->state;
    origintator->tell(origintator, new_r_TcpConnectionCreator_Connection(ref, 64), this->context->self, this->context->system);
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool TcpConnectionCreator_receive(Actor *this, uint16_t type, void *msg) {
    switch (type) {
        case m_TcpConnectionCreator_Create: // From: Any
            mp_TcpConnectionCreator_CreateMsg(this, msg);
            break;
        case r_BaThread_Error:  // From: BaThread
            mp_BaThread_Error(this, msg);
            break;
        case r_BaThread_Connected:  // From: BaThread
            mp_BaThread_Connected(this, msg);
            break;
        default:break;
    }

    return true;
}

Actor* new_TcpConnectionCreatorActor(void *selfRef) {
    return new_Actor(TcpConnectionCreator_receive, NULL, selfRef);
}