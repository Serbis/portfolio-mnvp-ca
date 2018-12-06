#include <stdlib.h>
#include "../include/a_core.h"
#include "../libs/miniakka/includes/actor_utils.h"
#include "../libs/miniakka/includes/standard_messages.h"
#include "../include/packet.h"
#include "../include/a_gateway.h"
#include "../include/a_gateway_pool.h"
#include "../include/transactions/a_preq_transaction.h"
#include "../include/a_transaction_pool.h"
#include "../include/a_send_packet_fsm.h"

typedef struct s_Core {
    ActorRef *gwPool;
    ActorRef *trPool;
    ActorRef *rTable;
    uint32_t netAdr;
} s_Core;

// =====================================================================================================================

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/** Hello
  *
  * Выполняет процедуру рукопожатия с окружающими узлами. Суть процедуры сводится к широковещательной
  * отправке HELLO пакетов во все имющиеся у узла шлюзы.
  */

ActorMessage* new_m_Core_Hello(uint16_t stackSize) {
    return new_ActorMessage(m_Core_Hello, NULL, stackSize, del_default);
}

void mp_Core_Hello(Actor *this, void *b) {
    s_Core *state = this->state;

    Packet *packet = Packet_createHelloPacket((uint32_t) (random() % INT_LEAST32_MAX), state->netAdr);
    state->gwPool->tell(state->gwPool, new_m_GatewayPool_RouteToAll(new_m_Gateway_Send(packet, 32), 32), this->context->self, this->context->system);
    free_Packet(packet);
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/** Preq
 *
 * Выполняет процедуру динамического поиска маршрута до целевого узла.
 *
 * @param dest целевой узел
 * @param timeout таймаут ожидания ответов
 * @param minFind минимальное кол-во найденных маршрутов для преждевременного завершения поиска
 * @param ttl стартовый ttl preq пакета
 */

ActorMessage* new_m_Core_Preq(uint32_t dest, uint32_t timeout, uint16_t minFind, uint16_t ttl, uint16_t stackSize) {
    b_Core_Preq *body = pmalloc(sizeof(b_Core_Preq));
    body->dest = dest;
    body->timeout = timeout;
    body->minFind = minFind;
    body->ttl = ttl;

    return new_ActorMessage(m_Core_Preq, body, stackSize, del_default);
}

void mp_Core_Preq(Actor *this, b_Core_Preq *b) {
    s_Core *state = this->state;

    ActorSystem *system = AU_system(this);
    ActorRef *self = this->context->self;

    ActorRef *transaction = system->actorOf(system, new_PreqTransaction(), NULL);
    transaction->tell(transaction, new_m_PreqTransaction_Initialize(b->dest, state->gwPool, this->context->sender,
                                                                    state->trPool, state->netAdr, b->timeout, b->minFind,
                                                                    b->ttl, 32), self, system);
    state->trPool->tell(state->trPool, new_m_TransactionPool_PoolAndExec(transaction, 32), self, system);
    AU_freeRef(transaction);
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/** SendPacket
  *
  * Выполняет отправку произвольного пакета в сеть. Данный запрос производит только статический поиск
  * маршрута. Ответом на данной запрос будет сообщение SendResult с кодом результата. Данное сообщение
  * описано в классе SendPacketFsm
  *
  * @param packet пакет для отправки в сеть
  */
void del_m_Core_SendPacket(b_Core_SendPacket *b) {
    free_Packet(b->packet);
}


ActorMessage* new_m_Core_SendPacket(Packet *packet, uint16_t stackSize) {
    b_Core_SendPacket *body = pmalloc(sizeof(b_Core_SendPacket));
    body->packet = copy_Packet(packet);

    return new_ActorMessage(m_Core_SendPacket, body, stackSize, del_m_Core_SendPacket);
}

void mp_Core_SendPacket(Actor *this, b_Core_SendPacket *b) {
    s_Core *state = this->state;

    ActorSystem *system = AU_system(this);
    ActorRef *self = this->context->self;

    ActorRef *ref = system->actorOf(system, new_SendPacketFsm(), NULL);
    ref->tell(ref, new_m_SendPacketFsm_Initialize(state->rTable, state->gwPool, false, 32), self, system);
    ref->tell(ref, new_m_SendPacketFsm_Exec(b->packet, 32), this->context->sender, system);
    AU_freeRef(ref);
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void Core_beforeStop(Actor *this) {
    s_Core *state = (s_Core *) this->state;
    AU_freeRef(state->gwPool);
    AU_freeRef(state->trPool);
    AU_freeRef(state->rTable);
}

bool Core_receive(Actor *this, uint16_t type, void *msg) {
    switch (type) {
        case m_Core_Hello:
            mp_Core_Hello(this, msg);
            break;
        case m_Core_Preq:
            mp_Core_Preq(this, msg);
            break;
        case m_Core_SendPacket:
            mp_Core_SendPacket(this, msg);
            break;
        default: break;
    }

    return true;
}

Actor* new_Core(ActorRef* gwPool, ActorRef* trPool, ActorRef* rTable, uint32_t netAdr) {
    s_Core *state = pmalloc(sizeof(s_Core));
    state->gwPool = AU_copyRef(gwPool);
    state->netAdr = netAdr;
    state->trPool = AU_copyRef(trPool);
    state->rTable = AU_copyRef(rTable);

    Actor *actor = new_Actor(Core_receive, Core_beforeStop, state);
    return actor;
}