#include "../include/a_receiver.h"
#include "../libs/miniakka/includes/actor.h"
#include "../libs/miniakka/includes/actor_ref.h"
#include "../libs/miniakka/includes/actor_utils.h"
#include "../include/a_rtable.h"
#include "../include/a_gateway_pool.h"
#include "../include/a_gateway.h"
#include "../include/a_transaction_pool.h"
#include "../include/transactions/transaction_commands.h"

typedef struct s_Receiver {
    uint32_t networkAddress;
    ActorRef *gwPool;
    ActorRef *trPool;
    ActorRef *rTable;

} s_Receiver;

// =====================================================================================================================

/**
  * Обрабатывает входящий HELLO покет. Прсваивает шлюзу из которого пришел пакет
  * сетевой адрес отправителя пакета. Заносит однохоповый маршрут до источника
  * пакета в таблицу маршрутизации
  *
  * @param source адрес источника пакета
  * @param gwLabel метка шлюза из которого был получен пакет
  */

void processTargetHelloPacket(uint32_t source, Actor *this, uint16_t gwLabel) {
    s_Receiver *state = this->state;
    ActorRef *self = this->context->self;

    ActorMessage *snlMsg = new_m_Gateway_SetNetworkAddress(source, 32);

    state->gwPool->tell(state->gwPool, new_m_GatewayPool_RouteByLabel(gwLabel, snlMsg, 32), self, this->context->system);
    state->rTable->tell(state->rTable, new_m_Rtable_UpdateRoute(source, source, gwLabel, 1, 32), self, this->context->system);

    AU_freeMsg(snlMsg, true);
}

/** Обрабатывает целевой PREQ пакет. Если пакет является натуральной модификацией, то создает
  * PREQ_ACK пакет и отпаравляет его в тот шлюз откуда пришел оригинальный пакет. Если пакет
  * являтся ACK модификацией, то производит его роутинг к обрабатывающей транзакции.
  *
  * @param packet входящий пакет
  * @param gwLabel метка шлюза из которого был получен пакет
  */
void processTargetPreqPacket(Actor *this, Packet *packet, uint16_t gwLabel) {
    s_Receiver *state = this->state;
    ActorRef *self = this->context->self;

    if (packet->flags == 0x00) {
        Packet *ackPacket = Packet_ackPreqPacket(packet, packet->ttl);
        ActorMessage *gsMsg = new_m_Gateway_Send(ackPacket, 32);
        state->gwPool->tell(state->gwPool, new_m_GatewayPool_RouteByLabel(gwLabel, gsMsg, 32), self, this->context->system);
        AU_freeMsg(gsMsg, true);
        free_Packet(ackPacket);
    } else {
        state->trPool->tell(state->trPool, new_m_TransactionPool_Route(packet, 32), self, this->context->system);
    }
}

/** Обрабатывает транзитный PREQ пакет. Уменьшает ttl пакета на единицу и отправляет его во все
  * имеющиеся у узла шлюзы.
  *
  * @param packet входящий пакет
  */
void processTransitPreqPacket(Actor *this, Packet *packet) {
    s_Receiver *state = this->state;
    ActorRef *self = this->context->self;
    packet->ttl--;
    ActorMessage *gsMsg = new_m_Gateway_Send(packet, 32);
    state->gwPool->tell(state->gwPool, new_m_GatewayPool_RouteToAll(gsMsg, 32), self, this->context->system);
    AU_freeMsg(gsMsg, true);
}

//======================================================================================================================

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void del_m_Receiver_Receive(b_Receiver_Receive *body) {
    free_Packet(body->packet);
}

ActorMessage* new_m_Receiver_Receive(Packet *packet, uint16_t gwLabel,uint32_t gwNetAdr, uint16_t stackSize)  {
    Packet *dup_packet = copy_Packet(packet);

    b_Receiver_Receive *body = pmalloc(sizeof(b_Receiver_Receive));
    body->packet = dup_packet;
    body->gwLabel = gwLabel;
    body->gwNetAdr = gwNetAdr;

    return new_ActorMessage(m_Receiver_Receive, body, stackSize, del_m_Receiver_Receive);
}

void mp_Receiver_Receive(Actor *this, b_Receiver_Receive *b) {
    s_Receiver *state = this->state;
    Packet *packet = b->packet;

    int16_t dest = (int16_t) (packet->ttl);

    uint32_t source = packet->source;


    if (source != state->networkAddress && b->gwNetAdr != 0)
        state->rTable->tell(state->rTable, new_m_Rtable_UpdateRoute(source, b->gwNetAdr, b->gwLabel, dest, 32), this->context->self, this->context->system);

    if (packet->dest != state->networkAddress && packet->type != 0 && packet->ttl > 0) {
        switch (packet->type) {
            case PACKET_PREQ:
                processTransitPreqPacket(this, packet);
                break;
            default:break;
        }
    } else {
        switch (packet->type) {
            case PACKET_HELLO:
                processTargetHelloPacket(source, this, b->gwLabel);
                break;
            case PACKET_PREQ:
                processTargetPreqPacket(this, packet, b->gwLabel);
                break;
            default:break;
        }
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void Receiver_beforeStop(Actor *this) {
    s_Receiver *state = (s_Receiver *) this->state;
    AU_freeRef(state->gwPool);
    AU_freeRef(state->rTable);
    AU_freeRef(state->trPool);
}

bool Receiver_receive(Actor *this, uint16_t type, void *msg) {
    switch (type) {
        case m_Receiver_Receive:
            mp_Receiver_Receive(this, msg);
            break;
        default: break;
    }

    return true;
}

Actor* new_Receiver(ActorRef* gwPool, ActorRef* rTable, ActorRef *trPool, uint32_t netAdr) {
    struct s_Receiver *state = pmalloc(sizeof(s_Receiver));
    state->gwPool = AU_copyRef(gwPool);
    state->trPool = AU_copyRef(trPool);
    state->rTable = AU_copyRef(rTable);
    state->networkAddress = netAdr;

    Actor *actor = new_Actor(Receiver_receive, Receiver_beforeStop, state);
    return actor;
}