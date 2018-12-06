#include <string.h>
#include "../include/a_tpcio.h"
#include "../../libs/miniakka/includes/actor_system.h"
#include "../include/a_tcp_connection_creator.h"

// BindAndAcceptMsg  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void del_TcpIO_BindAndAcceptMsg(b_TcpIo_BindAndAccept* msg) {
    pfree(msg->address);
}

ActorMessage* new_m_TcpIO_BindAndAcceptMsg(char *address, uint16_t port, uint16_t stackSize) {
    b_TcpIo_BindAndAccept *body = pmalloc(sizeof(b_TcpIo_BindAndAccept));
    body->address = pmalloc(strlen(address) + 1);
    memcpy(body->address, address, strlen(address) + 1);
    body->port = port;

    return new_ActorMessage(m_TcpIO_BindAndAccept, body, stackSize, del_TcpIO_BindAndAcceptMsg);
}

void mp_TcpIO_BindAndAccept(Actor* this, void *msg) {
    b_TcpIo_BindAndAccept *m = (b_TcpIo_BindAndAccept*) msg;
    size_t *creatorState = pmalloc(4);
    *creatorState = (size_t) this->context->sender;
    ActorRef *ref = ((ActorSystem*)this->context->system)->actorOf(this->context->system, new_TcpConnectionCreatorActor(creatorState), NULL);
    //*creatorState = (size_t) ref;

    //ActorRef *revRef = (ActorRef*) *creatorState;

    ref->tell(ref, new_m_TcpConnectionCreator_Create(m->address, m->port, 64), this->context->sender, this->context->system);
}

// Connect  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void del_TcpIO_Connect(b_TcpIo_Connect* b) {
    pfree(b->host);
}

ActorMessage* new_m_TcpIO_Connect(char *host, uint16_t port, uint16_t stackSize) {
    b_TcpIo_Connect *body = pmalloc(sizeof(b_TcpIo_Connect));
    body->host = pmalloc(strlen(host) + 1);
    memcpy(body->host, host, strlen(host) + 1);
    body->port = port;

    return new_ActorMessage(m_TcpIO_Connect, body, stackSize, del_TcpIO_Connect);
}

void mp_TcpIO_Connect(Actor* this, b_TcpIo_BindAndAccept *b) {
    //TODO Реализовать логику (см тест connector_spec)
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool TcpIO_receive(Actor *this, uint16_t type, void *msg) {
    switch (type) {
        case m_TcpIO_BindAndAccept:
            mp_TcpIO_BindAndAccept(this, msg);
            break;
        case m_TcpIO_Connect:
            mp_TcpIO_Connect(this, msg);
            break;
        default:break;
    }

    return true;
}

Actor* new_TcpIOActor() {
    return new_Actor(TcpIO_receive, NULL, NULL);
}