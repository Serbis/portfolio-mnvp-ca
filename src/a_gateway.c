#include <string.h>
#include "../include/a_gateway.h"
#include "../oscl/include/time.h"
#include "../oscl/include/connection.h"
#include "../include/global_messages.h"
#include "../oscl/include/utils.h"
#include "../libs/collections/includes/rings.h"
#include "../include/a_receiver.h"
#include "../libs/miniakka/includes/standard_messages.h"
#include "../include/a_rtable.h"
#include "../include/a_heart.h"
#include "../libs/miniakka/includes/actor_utils.h"

typedef struct s_Gateway {
    uint16_t label;
    uint64_t lastIncomingActivity;
    uint32_t networkAddress;
    ActorRef *connection;
    ActorRef *rTable;
    ActorRef *connector;
    Router *receivers;
    Packet *packet;
    bool bodyMode;
    uint16_t bodyPos;
    RingBufferDef *buffer;
} s_Gateway;

// =====================================================================================================================

void completeBody(Actor *this, s_Gateway *state) {
    if (state->bodyMode == true) {
        state->bodyPos++;
        if (state->bodyPos >= state->packet->length) {
            state->bodyMode = false;

            uint16_t bfSize = RINGS_dataLenght(state->buffer);
            uint8_t *body = pmalloc(bfSize);
            RINGS_extractData(state->buffer->writer - bfSize, bfSize, body, state->buffer);
            state->packet->body = body;

            //state->packet->gatewayLabel = state->gatewayLabel;
            RINGS_dataClear(state->buffer);
            state->bodyPos = 0;

            /*char *ps = PACKET_UTILS_toString(this->packet);
            Incapsulator *I = (Incapsulator*) this->I;
            I->lc->log(I->lc, 34, 3, ps);
            vPortFree(ps);*/

            //Packet *newPacket = copy_Packet(state->packet);
            //pfree(state->packet->body);
            //printf("Packet - %s\n", Packet_toString(state->packet));
            state->lastIncomingActivity = SystemTime();
            state->receivers->route(state->receivers, new_m_Receiver_Receive(state->packet, state->label, state->networkAddress, 64), this->context->self, this->context->system);
            pfree(state->packet->body);
            state->packet->body = NULL;
            //this->packetPool->put(this->packetPool, newPacket);
        }
    }
}

// =====================================================================================================================

/* || Send || ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 * Send packet to the gateway
 */

void del_m_Gateway_Send(b_Gateway_Send *b) {
    free_Packet(b->packet);
}

ActorMessage* new_m_Gateway_Send(Packet *packet, uint16_t stackSize) {
    Packet *dup_packet = copy_Packet(packet);

    b_Gateway_Send *body = pmalloc(sizeof(b_Gateway_Send));
    body->packet = dup_packet;

    return new_ActorMessage(m_Gateway_Send, body, stackSize, del_m_Gateway_Send);
}

void mp_Gateway_Send(Actor* this, b_Gateway_Send *b) {
    s_Gateway *state = this->state;

    uint16_t *size = pmalloc(2);
    uint8_t *bin = Packet_toBinary(b->packet, size);

    //printf("Gateway send\n");
    state->connection->tell(state->connection, new_m_Connection_SendData(bin, *size, 32), this->context->self, this->context->system);
    pfree(size);
    pfree(bin);
};

/* || SetLabel || ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 * Set gateway label
 */

ActorMessage* new_m_Gateway_SetLabel(uint16_t label, uint16_t stackSize) {
    b_Gateway_SetLabel *body = pmalloc(sizeof(b_Gateway_SetLabel));
    body->label = label;

    return  new_ActorMessage(m_Gateway_SetLabel, body, stackSize, del_default);
}

void mp_Gateway_SetLabel(Actor* this, b_Gateway_SetLabel *b) {
    s_Gateway *state = this->state;
    state->label = b->label;
};

/* || GetLabel || ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 * Return gateway label
 */

ActorMessage* new_m_Gateway_GetLabel(uint16_t stackSize) {
    return new_ActorMessage(m_Gateway_GetLabel, NULL, stackSize, del_default);
}

void mp_Gateway_GetLabel(Actor* this, void *b) {
    s_Gateway *state = this->state;

    this->context->sender->tell(this->context->sender, new_r_Gateway_Label(state->label, 32), this->context->self, this->context->system);
};

/* || SetNetworkAddress || +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 * Set gateway network address
 */

ActorMessage* new_m_Gateway_SetNetworkAddress(uint32_t netAdr, uint16_t stackSize) {
    b_Gateway_SetNetworkAddress *body = pmalloc(sizeof(b_Gateway_SetNetworkAddress));
    body->netAdr = netAdr;

    return new_ActorMessage(m_Gateway_SetNetworkAddress, body, stackSize, del_default);
}

void mp_Gateway_SetNetworkAddress(Actor* this, b_Gateway_SetNetworkAddress *b) {
    s_Gateway *state = this->state;
    state->networkAddress = b->netAdr;
};

/* || GetNetworkAddress || +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 * Return gateway network address
 */

ActorMessage* new_m_Gateway_GetNetworkAddress(uint16_t stackSize) {
    return new_ActorMessage(m_Gateway_GetNetworkAddress, NULL, stackSize, del_default);
}

void mp_Gateway_GetNetworkAddress(Actor* this, void *b) {
    s_Gateway *state = this->state;

    this->context->sender->tell(this->context->sender, new_r_Gateway_NetworkAddress(state->networkAddress, 32), this->context->self, this->context->system);
};

/* || Resp - Stopped || ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 * Message indicates than gateway (and connection too) stopped
 */

ActorMessage* new_r_Gateway_Stopped(uint16_t stackSize) {
    return new_ActorMessage(r_Gateway_Stopped, NULL, stackSize, del_default);
}

/* || Resp - Label || ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 * Message that return gateway label
 */

ActorMessage* new_r_Gateway_Label(uint16_t label, uint16_t stackSize) {
    b_Gateway_Label *body = pmalloc(sizeof(b_Gateway_Label));
    body->label = label;

    return new_ActorMessage(r_Gateway_Label, body, stackSize, del_default);
}

/* || Resp - NetworkAddress || +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 * Message that return gateway network address
 */

ActorMessage* new_r_Gateway_NetworkAddress(uint32_t netAdr, uint16_t stackSize) {
    b_Gateway_NetworkAddress *body = pmalloc(sizeof(b_Gateway_NetworkAddress));
    body->netAdr = netAdr;

    return new_ActorMessage(r_Gateway_NetworkAddress, body, stackSize, del_default);
}

// A_Connection - Data +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void mp_A_Connection_Data(Actor *this, b_Connection_Data *body) {
    s_Gateway *state = this->state;
    //printf("Received data block - ");
    //Utils_printArrayAsHex(body->data, body->size);

    state->lastIncomingActivity = SystemTime(); //Установка времени последней активности шлюза

    for (uint32_t i = 0; i < body->size; i++) {
        //	printf("b=%d", b);
        uint8_t b = body->data[i];
        RINGS_write((uint8_t) b, state->buffer);
        if (b == 0x3f) { //Если получена метка SN2
            //printf("SN2");
            if (RINGS_dataLenght(state->buffer) >= 28) { //Если в буфере есть данные размером с заголовок или более
                //printf(">=28");
                if (RINGS_getByShiftFromWriter(-28, state->buffer) ==
                    0x1f) { //Если в буфере на позиции -30 есть метка SN1
                    //printf("SN1");
                    uint8_t *head = pmalloc(28);
                    RINGS_extractData((uint16_t) (state->buffer->writer - 28), 28, head, state->buffer);
                    Packet_parsePacketHeader(state->packet, head, 28);
                    pfree(head);
                    state->bodyMode = true;
                    state->bodyPos = 0;
                    RINGS_dataClear(state->buffer);
                } else {
                    completeBody(this, state);
                }
            } else {
                completeBody(this, state);
            }
        } else {
            completeBody(this, state);
        }
    }
}

// A_Connection - ConnectionClosed +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void mp_A_Connection_ConnectionClosed(Actor *this, void *body) {
    s_Gateway *state = this->state;

    //TODO нельзя остановить шлюз пока его label == 0. Так как если это так, то значит label летит от пула к шлюза и еще просто недошел до него. P.S. Тут нужно удалять из пула его еще. P.S.2 тоже самое в hearbeat
    state->connection->tell(state->connection, new_m_Stop(32), this->context->self, this->context->system);
    state->rTable->tell(state->rTable, new_m_Rtable_RemoveAllRoutesByGateway(state->networkAddress, 32), this->context->self, this->context->system);
    if (state->connector != NULL)
        state->connector->tell(state->connector, new_r_Gateway_Stopped(32), this->context->self, this->context->system);
    this->context->self->tell(this->context->self, new_m_Stop(32), NULL, this->context->system);
}

// MNVP_GLOBAL - HeartBeat +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void mp_Hearat_HeartBeat(Actor *this, void *b) {
    s_Gateway *state = this->state;

    if (state->lastIncomingActivity < SystemTime() - 60000) {
        state->connection->tell(state->connection, new_m_Stop(32), this->context->self, this->context->system);
        state->rTable->tell(state->rTable, new_m_Rtable_RemoveAllRoutesByGateway(state->networkAddress, 32), this->context->self, this->context->system);
        if (state->connector != NULL)
            state->connector->tell(state->connector, new_r_Gateway_Stopped(32), this->context->self, this->context->system);
        this->context->self->tell(this->context->self, new_m_Stop(32), NULL, this->context->system);
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void Gateway_beforeStop(Actor *this) {
    s_Gateway *state = (s_Gateway *) this->state;
    pfree(state->connection);
    pfree(state->rTable);
    if (state->connector != NULL)
        pfree(state->connector);
    free_Packet(state->packet);
    RINGS_Free(state->buffer);
    pfree(state->buffer);
}

bool Gateway_receive(Actor *this, uint16_t type, void *msg) {
    switch (type) {
        case m_Gateway_Send:
            mp_Gateway_Send(this, msg);
            break;
        case m_Gateway_SetLabel:
            mp_Gateway_SetLabel(this, msg);
            break;
        case m_Gateway_GetLabel:
            mp_Gateway_GetLabel(this, msg);
            break;
        case m_Gateway_SetNetworkAddress:
            mp_Gateway_SetNetworkAddress(this, msg);
            break;
        case m_Gateway_GetNetworkAddress:
            mp_Gateway_GetNetworkAddress(this, msg);
            break;
        case r_Connection_Data:
            mp_A_Connection_Data(this, msg);
            break;
        case r_Connection_ConnectionClosed:
            mp_A_Connection_ConnectionClosed(this, msg);
            break;
        case r_Heart_HeartBeat:
            mp_Hearat_HeartBeat(this, msg);
            break;
        default:
            break;
    }

    return true;
}


Actor* new_Gateway(ActorRef *connection, ActorRef *rTable, ActorRef *connector, Router *receivers, uint16_t label, bool testMode) {
    ActorRef *dup_connection = AU_copyRef(connection);
    ActorRef *dup_rTable = AU_copyRef(rTable);
    ActorRef *dup_connector;
    if (connector != NULL) {
        dup_connector = AU_copyRef(connector);
    } else {
        dup_connector = NULL;
    }

    Router *dup_receivers = pmalloc(sizeof(Router));
    memcpy(dup_receivers, receivers, sizeof(Router));

    s_Gateway *state = pmalloc(sizeof(s_Gateway));
    state->label = label;
    if (testMode)
        state->lastIncomingActivity = 0;
    else
        state->lastIncomingActivity = (uint32_t) SystemTime();
    state->networkAddress = 0;
    state->connection = dup_connection;
    state->rTable = dup_rTable;
    state->connector = dup_connector;
    state->receivers = dup_receivers;
    state->packet = new_Packet(0, NULL);
    state->bodyMode = false;
    state->bodyPos = 0;
    state->buffer = RINGS_createRingBuffer(256, RINGS_OVERFLOW_SHIFT, 1);

    Actor *actor = new_Actor(Gateway_receive, Gateway_beforeStop, state);

    return actor;
}