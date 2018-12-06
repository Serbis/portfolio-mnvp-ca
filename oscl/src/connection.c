#include <string.h>
#include "../include/connection.h"
#include "../../libs/miniakka/includes/actor_utils.h"
#include "../../libs/miniakka/includes/standard_messages.h"

//  ||| SetDataHandler ||| +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void del_m_TcpConnection_SetDataHandler(b_Connection_SetDataHandler *b) {
    ;
}

ActorMessage* new_m_Connection_SetDataHandler(ActorRef *ref, uint16_t stackSize) {
    b_Connection_SetDataHandler *b = pmalloc(sizeof(b_Connection_SetDataHandler));
    b->handler = AU_copyRef(ref);;

    return new_ActorMessage(m_Connection_SetDataHandler, b, stackSize, del_m_TcpConnection_SetDataHandler);
}

//  ||| SendData ||| +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


void del_m_TcpConnection_SendData(b_Connection_SendData *b) {
    pfree(b->data);
}

ActorMessage* new_m_Connection_SendData(uint8_t *data, uint32_t size, uint16_t stackSize) {
    uint8_t *nData = pmalloc(size);
    memcpy(nData, data, size);

    b_Connection_SendData *body = pmalloc(sizeof(b_Connection_SendData));
    body->data = nData;
    body->size = size;

    return new_ActorMessage(m_Connection_SendData, body, stackSize, del_m_TcpConnection_SendData);
}

//  ||| Close ||| +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

ActorMessage* new_m_Connection_Close(uint16_t stackSize) {
    ActorMessage *msg = pmalloc(sizeof(ActorMessage));
    msg->type = m_Connection_Close;
    msg->body = NULL;
    msg->stackSize = stackSize;
    msg->destructor = NULL;

    return new_ActorMessage(m_Connection_Close, NULL, stackSize, del_default);
}

//  ||| Resp - Data ||| +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void del_m_Connection_Data(b_Connection_Data *body) {
    pfree(body->data);
}

ActorMessage* new_r_Connection_Data(uint8_t *data, uint32_t size, uint16_t stackSize) {
    uint8_t *nData = pmalloc(size);
    memcpy(nData, data, size);

    b_Connection_Data *body = pmalloc(sizeof(b_Connection_Data));
    body->data = nData;
    body->size = size;

    return new_ActorMessage(r_Connection_Data, body, stackSize, del_m_Connection_Data);
}

//  ||| Resp - ConnectionClosed ||| +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

ActorMessage* new_r_Connection_ConnectionClosed(uint16_t stackSize) {
    return new_ActorMessage(r_Connection_ConnectionClosed, NULL, stackSize, del_default);
}