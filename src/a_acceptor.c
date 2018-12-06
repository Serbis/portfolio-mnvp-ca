#include <string.h>
#include "../include/a_acceptor.h"
#include "../oscl/include/a_tpcio.h"
#include "../include/a_gateway_pool.h"
#include "../oscl/include/a_tcp_connection_creator.h"
#include "../libs/miniakka/includes/actor_utils.h"
#include "../include/a_gateway.h"

typedef struct s_Acceptor {
    ActorRef *gwPool;       //Gateway pool ref
    void *originalMessage;     //Original body for connectionIO
    int8_t mode;            //Mode -1 = not initialized, 0 = tcp
} s_Acceptor;



/* || AcceptTcp || +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 * Create new tcp bind and accept session
 */

void del_m__Acceptor_AcceptTcp(b_AcceptTcp *b) {
    pfree(b->host);
    pfree(b->tcpIo);
}

ActorMessage* new_m_Acceptor_AcceptTcp(ActorRef *tcpIo, char *host, uint16_t port, uint16_t stackSize) {
    ActorRef *dup_tcpIo = AU_copyRef(tcpIo);
    char *dup_host = pmalloc(strlen(host) + 1);
    memcpy(dup_host, host, strlen(host) + 1);

    b_AcceptTcp *body = pmalloc(sizeof(b_AcceptTcp));
    body->tcpIo = dup_tcpIo;
    body->host = dup_host;
    body->port = port;

    return new_ActorMessage(m_Accptor_AcceptTpc, body, stackSize, del_m__Acceptor_AcceptTcp);
}

void mp_Acceptor_AcceptTcp(Actor* this, b_AcceptTcp *b) {
    s_Acceptor *state = this->state;

    b_AcceptTcp *dup_b = pmalloc(sizeof(b_AcceptTcp));
    char *dup_b_host = pmalloc(strlen(b->host) + 1);
    ActorRef *dub_b_tcpIo = AU_copyRef(b->tcpIo);
    memcpy(dup_b_host, b->host, strlen(b->host) + 1);
    dup_b->host = dup_b_host;
    dup_b->tcpIo = dub_b_tcpIo;
    dup_b->port = b->port;

    state->mode = 1;
    state->originalMessage = dup_b;

    dup_b->tcpIo->tell(dup_b->tcpIo, new_m_TcpIO_BindAndAcceptMsg(dup_b->host, dup_b->port, 32) , this->context->self, this->context->system);
};

/* || TcpConnectionCreator - Connection || +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 * Create new tcp bind and accept session
 */

void mp_Acceptor_TcpConnectionCreator_Connection(Actor *this, b_TcpConnectionCreator_Connection *body) {
    s_Acceptor *state = this->state;

    ActorRef *dup_ref = AU_copyRef(body->ref);

    //TODO А как насчет создать шлюз и запихнуть его в пул !?
    //ActorRef *gateway = AU_system(this)->actorOf(AU_system(this), new_Gateway())

    state->gwPool->tell(state->gwPool, new_m_GatewayPool_Add(dup_ref, 32), this->context->self, this->context->system);

    b_AcceptTcp *orBody = (b_AcceptTcp*) state->originalMessage;
    orBody->tcpIo->tell(orBody->tcpIo, new_m_TcpIO_BindAndAcceptMsg(orBody->host, orBody->port, 32) , this->context->self, this->context->system);
};

/* || TcpConnectionCreator - BindFailed || +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 * Create new tcp bind and accept session
 */

void mp_TcpConnectionCreator_BindFailed(Actor* this, b_TcpConnectionCreator_BindFailed *body) {
    s_Acceptor *state = this->state;

    ActorSystem *system = AU_system(this);
    ActorRef *self = this->context->self;
    char *key = system->scheduler->scheduleOnce(system->scheduler, NULL, 3000, self, state->originalMessage, self);
    pfree(key);
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void Acceptor_beforeStop(Actor *this) {
    s_Acceptor *state = (s_Acceptor *) this->state;
    pfree(state->gwPool);

    void *orBody = state->originalMessage;

    switch (state->mode) {
        case 1:
            pfree(((b_AcceptTcp*) orBody)->tcpIo);
            pfree(((b_AcceptTcp*) orBody)->host);
            pfree(orBody);

            break;
        default: break;
    }
}

bool Acceptor_receive(Actor *this, uint16_t type, void *msg) {
    switch (type) {
        case m_Accptor_AcceptTpc:
            mp_Acceptor_AcceptTcp(this, msg);
            break;
        case r_TcpConnectionCreator_Connection:
            mp_Acceptor_TcpConnectionCreator_Connection(this, msg);
            break;
        case r_TcpConnectionCreator_BindFailed:
            mp_TcpConnectionCreator_BindFailed(this, msg);
            break;
        default: break;
    }

    return true;
}

Actor* new_Acceptor(ActorRef* gwPool) {
    s_Acceptor *state = pmalloc(sizeof(s_Acceptor));
    state->gwPool = AU_copyRef(gwPool);;
    state->mode = -1;           //Will be changed in some Accept message



    Actor *actor = new_Actor(Acceptor_receive, Acceptor_beforeStop, state);
    return actor;
}