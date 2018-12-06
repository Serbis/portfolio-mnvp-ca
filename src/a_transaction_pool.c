#include <stdlib.h>
#include "../include/a_transaction_pool.h"
#include "../libs/collections/includes/map2.h"
#include "../libs/miniakka/includes/actor_utils.h"
#include "../oscl/include/data.h"
#include "../include/transactions/transaction_commands.h"
#include "../libs/miniakka/includes/standard_messages.h"

/** Пул открытых транзакций узла. Любая транзацкция стартует через этот пул. При старте, ей выдается
  * идентификатор транзакции, который будет использоваться ей в качестве идентификатора группы пакетов.
  * При получении транзакционного пакета, он перенаправляется транзакции через этот пул. После завершения
  * свой реаботы транзакция передает сообщение TransactionFinished, которе приводит к удалению транзакции
  * из пула.
  */

typedef struct s_TransactionPool {
    /** Пул референсов транзакций */
    Map *pool;
} s_TransactionPool;

// =====================================================================================================================

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/** PoolAndExec
  *
  * Размещает транзакцию в пуле и отправляет ей команду Exec с присвоенным идентификатором транзакции
  *
  * @param transaction референс fsm транзакции
  */

void del_m_TransactionPool_PoolAndExec(b_TransactionPool_PoolAndExec *b) {
    AU_freeRef(b->transaction);
}

ActorMessage* new_m_TransactionPool_PoolAndExec(ActorRef *transaction, uint16_t stackSize) {
    ActorRef *dup_transaction = AU_copyRef(transaction);
    b_TransactionPool_PoolAndExec *body = pmalloc(sizeof(b_TransactionPool_PoolAndExec));
    body->transaction = dup_transaction;

    return new_ActorMessage(m_TransactionPool_PoolAndExec, body, stackSize, del_m_TransactionPool_PoolAndExec);
}

void mp_TransactionPool_PoolAndExec(Actor *this, b_TransactionPool_PoolAndExec *b) {
    s_TransactionPool *state = this->state;

    uint16_t label = 0;
    char *key;

    while (true) {
        label = (uint16_t) (rand() % 65535);
        if (label == 0) {
            continue;
        } else {
            key = itoa(label);
            if (MAP_get(key, state->pool) == NULL) {
                break;
            } else {
                pfree(key);
                continue;
            }
        }
    }

    ActorRef *dup_transaction = AU_copyRef(b->transaction);
    MAP_add(key, dup_transaction, state->pool);
    dup_transaction->tell(dup_transaction, new_m_Transaction_Exec(label, 32), this->context->self, this->context->system);

    pfree(key);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/** Get
  *
  * Возвращает референс транзакции по ее идентификатора
  *
  * @param tid идентификатор транзакции
  */

ActorMessage* new_m_TransactionPool_Get(uint32_t tid, uint16_t stackSize) {
    b_TransactionPool_Get *body = pmalloc(sizeof(b_TransactionPool_Get));
    body->tid = tid;

    return new_ActorMessage(m_TransactionPool_Get, body, stackSize, del_default);
}

void mp_TransactionPool_Get(Actor *this, b_TransactionPool_Get *b) {
    s_TransactionPool *state = this->state;

    char *key = itoa(b->tid);
    ActorRef *gwRef = MAP_get(key, state->pool);
    pfree(key);
    this->context->sender->tell(this->context->sender, new_r_TransactionPool_Transaction(gwRef, 32), this->context->self, this->context->system);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/** Transaction
  *
  * Ответ по сообщениям get. Содержит рефернс транзакции а или None если по каким-то причинам он
  * не обнуружен в пуле
  *
  * @param ref референс транзакции
  */

void del_TransactionPool_Transaction(b_TransactionPool_Transaction *b) {
    if (b->ref != NULL)
        AU_freeRef(b->ref);
}

ActorMessage* new_r_TransactionPool_Transaction(ActorRef *ref, uint16_t stackSize) {
    b_TransactionPool_Transaction *body = pmalloc(sizeof(b_TransactionPool_Transaction));
    if (ref != NULL)
        body->ref = AU_copyRef(ref);
    else
        body->ref = NULL;

    return new_ActorMessage(r_TransactionPool_Transaction, body, stackSize, del_TransactionPool_Transaction);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/** Route
  *
  * Перенаправляет полученный пакет к соответсвующей транзакции (packet.msgId == tid)
  *
  * @param packet пакет
  */

void del_TransactionPool_Route(b_TransactionPool_Route *b) {
    free_Packet(b->packet);
}

ActorMessage* new_m_TransactionPool_Route(Packet *packet, uint16_t stackSize) {
    b_TransactionPool_Route *body = pmalloc(sizeof(b_TransactionPool_Route));
    body->packet = copy_Packet(packet);

    return new_ActorMessage(m_TransactionPool_Route, body, stackSize, del_TransactionPool_Route);
}

void mp_TransactionPool_Route(Actor *this, b_TransactionPool_Route *b) {
    s_TransactionPool *state = this->state;

    ListIterator *iterator = state->pool->inner->iterator(state->pool->inner);

    while (iterator->hasNext(iterator)) {
        MapItem *item = iterator->next(iterator);
        if (atoi(item->key) == b->packet->msgId) {
            ActorRef *trRef = (ActorRef *) item->value;
            Packet *packet = copy_Packet(b->packet);
            trRef->tell(trRef, new_m_Transaction_ReceivePacket(packet, 32), this->context->sender, this->context->system);
            free_Packet(packet);
            break;
        }
    }

    pfree(iterator);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


void mp_Transaction_TransactionFinished(Actor *this, b_Transaction_TransactionFinished *b) {
    s_TransactionPool *state = this->state;

    char *key = itoa(b->tid);
    ActorRef *ref = MAP_remove(key, state->pool);
    if (ref != NULL)
        AU_freeRef(ref);
    pfree(key);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void TransactionPool_beforeStop(Actor *this) {
    s_TransactionPool *state = (s_TransactionPool *) this->state;

    MapIterator *iterator = MAP_ITERATOR_new(state->pool);

    while (MAP_ITERATOR_hasNext(iterator)) {
        ActorRef *ref = MAP_ITERATOR_next(iterator);
        AU_freeRef(ref);
        MAP_ITERATOR_remove(iterator);
    }

    del_List(state->pool->inner);
    pfree(state->pool);
    pfree(iterator);
}

bool TransactionPool_receive(Actor *this, uint16_t type, void *msg) {
    switch (type) {
        case m_TransactionPool_PoolAndExec:
            mp_TransactionPool_PoolAndExec(this, msg);
            break;
        case m_TransactionPool_Get:
            mp_TransactionPool_Get(this, msg);
            break;
        case m_TransactionPool_Route:
            mp_TransactionPool_Route(this, msg);
            break;

        /** Сообщение приходит от транзакций завершивших свое существование (независимо от результата). При
          * получении данного сообщения, транзакция удалсятся из пула */
        case r_Transaction_TransactionFinished:
            mp_Transaction_TransactionFinished(this, msg);
            break;
        default: break;
    }

    return true;
}

Actor* new_TransactionPool() {
    s_TransactionPool *state = pmalloc(sizeof(s_TransactionPool));

    state->pool = MAP_new();

    return new_Actor(TransactionPool_receive, TransactionPool_beforeStop, state);
}