#include <string.h>
#include <sys/socket.h>
#include "../include/tcp_connection.h"

typedef struct s_TcpConnection {
    RecThreadArgs *tArgs;
    ActorRef *dataHandler;
} s_TcpConnection;


//  ||| SetDataHandler ||| +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void mp_TcpConnection_SetDataHandler(Actor *this, b_Connection_SetDataHandler *b) {
    s_TcpConnection *state = this->state;
    state->dataHandler = b->handler;
}

//  ||| SendData ||| +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void mp_TcpConnection_SendData(Actor *this, b_Connection_SendData *body) {
    s_TcpConnection *state = this->state;
    if (send(state->tArgs->socket, body->data, body->size, 0) == -1) {
        printf("xxx");
        //THREAD - STOP
        //HANDLER CONNECTION CLOSED
        //SELF STOP
    }
}

//  ||| Close ||| +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void mp_TcpConnection_Close(Actor *this) {
    s_TcpConnection *state = this->state;
    state->tArgs->stop = true;
    state->dataHandler->tell(state->dataHandler, new_r_Connection_ConnectionClosed(64), this->context->self, this->context->system);
    //SELF->STOP
}

//  ||| Resp - Data ||| +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---->Defined in connection.c

//  ||| Resp - ConnectionClosed ||| +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---->Defined in connection.c

// ||| RecThread - Closed ||| ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void mp_RecThread_Closed(Actor *this) {
    s_TcpConnection *state = this->state;
    state->dataHandler->tell(state->dataHandler, new_r_Connection_ConnectionClosed(64), this->context->self, this->context->system);
    //SELF->STOP
}

// ||| RecThread - Data ||| ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void mp_RecThread_Data(Actor *this, b_RecThread_Data *r) {
    s_TcpConnection *state = this->state;

    if (state->dataHandler != NULL) {
        state->dataHandler->tell(state->dataHandler, new_r_Connection_Data(r->data, r->size, 64),
                                 this->context->self, this->context->system);
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


//TODO Если актор был остановлен, нужно закрывать соединение

bool TcpConnection_receive(Actor *this, uint16_t type, void *msg) {
    switch (type) {
        case m_Connection_SetDataHandler: // From: Any
            mp_TcpConnection_SetDataHandler(this, msg);
            break;
        case m_Connection_SendData: // From: Any
            mp_TcpConnection_SendData(this, msg);
            break;
        case m_Connection_Close: // From: Any
            mp_TcpConnection_Close(this);
            break;
        case r_RecThread_Closed: //From: RecThread
            mp_RecThread_Closed(this);
            break;
        case r_RecThread_Data: //From: RecThread
            mp_RecThread_Data(this, msg);
            break;
        default:break;
    }

    return true;
}

Actor* new_TcpConnectionActor(RecThreadArgs *tArgs) {
    s_TcpConnection *state = pmalloc(sizeof(struct s_TcpConnection));
    state ->tArgs = tArgs;
    state->dataHandler = NULL;

    NewThread(RecThread_run, tArgs, 0, NULL, 0);

    return new_Actor(TcpConnection_receive, NULL, state);
}