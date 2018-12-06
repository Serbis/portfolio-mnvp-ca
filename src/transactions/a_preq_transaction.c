#include "../../include/transactions/a_preq_transaction.h"
#include "../../libs/miniakka/includes/standard_messages.h"
#include "../../include/transactions/transaction_commands.h"
#include "../../libs/miniakka/includes/actor_utils.h"
#include "../../include/a_gateway_pool.h"
#include "../../include/a_gateway.h"

/**
  * Транзакция выполнения динамического поиска маршрута
  */


/* -----------------------------------Messages creation section---------------------------------------- */

/** Маршруты до цели найдены. Сообщение отправляется получаетлю результата */

//RESPONSE ------> Found
ActorMessage* new_r_PreqTransaction_Found(uint16_t stackSize) {
    return new_ActorMessage(r_PreqTransaction_Found, NULL, stackSize, del_default);
}

/** Маршруты до цели не найдены. Сообщение отправляется получаетлю результата */

//RESPONSE ------> NotFound
ActorMessage* new_r_PreqTransaction_NotFound(uint16_t stackSize) {
    return new_ActorMessage(r_PreqTransaction_NotFound, NULL, stackSize, del_default);
}

/* -------------------------------------Data creation section------------------------------------------ */

//DATA --------> Uninitialized
fsm_data* del_dt_PreqTransaction_Uninitialized(dtb_PreqTransaction_Uninitialized *b) {
    AU_freeRef(b->gwPool);
    AU_freeRef(b->resultRecipient);
    AU_freeRef(b->transactionCtl);
}

fsm_data* new_dt_PreqTransaction_Uninitialized(uint32_t dest, ActorRef *gwPool, ActorRef *resultRecipient,
                                               ActorRef *transactionCtl, uint32_t netAdr, uint32_t timeout,
                                               uint16_t minFinds,
                                               uint16_t ttl) {
    dtb_PreqTransaction_Uninitialized *db = pmalloc(sizeof(dtb_PreqTransaction_Uninitialized));
    db->dest = dest;
    db->gwPool = AU_copyRef(gwPool);
    db->resultRecipient = AU_copyRef(resultRecipient);
    db->transactionCtl = AU_copyRef(transactionCtl);
    db->netAdr = netAdr;
    db->timeout = timeout;
    db->minFinds = minFinds;
    db->ttl = ttl;

    return new_fsm_data(dt_PreqTransaction_Uninitialized, db, del_dt_PreqTransaction_Uninitialized);
}

/** Данные состояния ожидания ответов*/

//DATA --------> WaitingAcks
fsm_data* del_dt_PreqTransaction_WaitingAcks(dtb_PreqTransaction_WaitingAcks *b) {
    AU_freeRef(b->gwPool);
    AU_freeRef(b->resultRecipient);
    AU_freeRef(b->transactionCtl);
}

fsm_data* new_dt_PreqTransaction_WaitingAcks(uint32_t tid, ActorRef *gwPool, ActorRef *resultRecipient,
                                             ActorRef *transactionCtl, uint32_t netAdr, uint32_t timeout,
                                             uint16_t minFinds) {
    dtb_PreqTransaction_WaitingAcks* data = pmalloc(sizeof(dtb_PreqTransaction_WaitingAcks));
    data->tid = tid;
    data->found = 0;
    data->gwPool = AU_copyRef(gwPool);
    data->resultRecipient = AU_copyRef(resultRecipient);
    data->transactionCtl = AU_copyRef(transactionCtl);
    data->netAdr = netAdr;
    data->timeout = timeout;
    data->minFinds = minFinds;

    return new_fsm_data(dt_PreqTransaction_WaitingAcks, data, del_dt_PreqTransaction_WaitingAcks);
}

/* --------------------------------------States logic section------------------------------------------ */

//STATE --------> Idle / DATA ----------> Uninitialized

/** Стартовое состояние fsm. В нем fsm ожидает получения универсального сообщения Exec с кодом транзакции.
  * Получив таковое направляет запрос в пул шлюзов на отправку шировковещательную отправку preq пакетов.
  * Таймат состояния приводит к остановке fsm.
  */

void ev_PreqTransaction_Idle_Uninitialized_Exec(Actor *this, b_Transaction_Exec *b) {
    dtb_PreqTransaction_Uninitialized *d = AU_fsm_data(this);

    Packet *packet = Packet_createPreqPacket(b->tid, d->netAdr, d->dest, d->ttl);
    ActorMessage *m = new_m_Gateway_Send(packet, 32);
    d->gwPool->tell(d->gwPool, new_m_GatewayPool_RouteToAll(m, 32), NULL, this->context->system);
    fsm_data *newData = new_dt_PreqTransaction_WaitingAcks(b->tid, d->gwPool, d->resultRecipient, d->transactionCtl,
                                                           d->netAdr, d->timeout, d->minFinds);
    AU_freeMsg(m, true);
    free_Packet(packet);

    FSM_toState(this, newData, st_PreqTransaction_WaitAcks, d->timeout, 32);
}

void ev_PreqTransaction_Idle_Uninitialized_StateTimeout(Actor *this, b_FSM_StateTimeout *b) {
    FSM_stop(this, 32);
}

//STATE --------> WaitAcks / DATA ----------> WaitingAcks

/** Состояние сборки ответов на preq пакеты. В нет каждый полученный PREQ_ACK пакет инкрементирует счетик
  * найденных маршрутов. Если последний достигает значения minFinds, транзакция завершает свою работу
  * с результатом Found, иначе ждет новых ответов. По достижении таймаута, если кол-во найденных маршрутов
  * > 0 завершается с результатом Found иначе с результатом NotFound
  */

void ev_PreqTransaction_Idle_WaitingAcks_ReceivePacket(Actor *this, b_Transaction_ReceivePacket *b) {
    dtb_PreqTransaction_WaitingAcks *d = AU_fsm_data(this);

    if (b->packet->type == PACKET_PREQ) {
        if (d->found + 1 == d->minFinds) {
            d->resultRecipient->tell(d->resultRecipient, new_r_PreqTransaction_Found(32), this->context->self, this->context->system);
            d->transactionCtl->tell(d->transactionCtl, new_r_Transaction_TransactionFinished(d->tid, 32), this->context->self, this->context->system);
            FSM_stop(this, 32);
        } else {
            d->found++;
            FSM_stay(this, d->timeout, 32);
        }
    } else {
        FSM_stay(this, d->timeout, 32);
    }
}

void ev_PreqTransaction_Idle_WaitingAcks_StateTimeout(Actor *this, b_FSM_StateTimeout *b) {
    dtb_PreqTransaction_WaitingAcks *d = AU_fsm_data(this);

    if (d->found > 0) {
        d->resultRecipient->tell(d->resultRecipient, new_r_PreqTransaction_Found(32), this->context->self, this->context->system);
    } else {
        d->resultRecipient->tell(d->resultRecipient, new_r_PreqTransaction_NotFound(32), this->context->self, this->context->system);
    }
    d->transactionCtl->tell(d->transactionCtl, new_r_Transaction_TransactionFinished(d->tid, 32), this->context->self, this->context->system);

    FSM_stop(this, 32);
}

/* ----------------------------------------Initialize message-------------------------------------------- */

/** @param dest адрес узла до которого ищется маршрут
  * @param gwPool пул шлюзов
  * @param resultRecipient получатель результата preq запроса
  * @param transactionCtl контроллер транзации (читай хранитель пула транзакцй. К нему будет отправлено сообщение
  *                       TransactionFinished после завршения работы fsm
  * @param netAdr сетевой адрес узла
  * @param timeout время сборки ack пакетов по запросу
  * @param minFinds минимальное количество маршрутов для предварительного завршениния транзакции (до таймаута)
  * @param ttl старторвый ttl preq пакета (рекомендумое значение - 10)
  */

void del_m_PreqTransaction_Initialize(b_PreqTransaction_Initialize *b) {
    AU_freeRef(b->gwPool);
    AU_freeRef(b->resultRecipient);
    AU_freeRef(b->transactionCtl);
}

ActorMessage* new_m_PreqTransaction_Initialize(uint32_t dest, ActorRef *gwPool, ActorRef *resultRecipient,
                                         ActorRef *transactionCtl, uint32_t netAdr, uint32_t timeout, uint16_t minFinds,
                                         uint16_t ttl, uint16_t stackSize) {
    b_PreqTransaction_Initialize *body = pmalloc(sizeof(b_PreqTransaction_Initialize));
    body->dest = dest;
    body->gwPool = AU_copyRef(gwPool);
    body->resultRecipient = AU_copyRef(resultRecipient);
    body->transactionCtl = AU_copyRef(transactionCtl);
    body->netAdr = netAdr;
    body->timeout = timeout;
    body->minFinds = minFinds;
    body->ttl = ttl;

    return new_ActorMessage(m_Initialize, body, stackSize, del_m_PreqTransaction_Initialize);
}

void mp_PreqTransaction_Initialize(Actor *this, b_PreqTransaction_Initialize* b) {
    fsm_data *data = new_dt_PreqTransaction_Uninitialized(b->dest, b->gwPool, b->resultRecipient, b->transactionCtl,
            b->netAdr, b->timeout, b->minFinds, b->ttl);
    FSM_toState(this, data, st_PreqTransaction_Idle, 3000, 32);
}

void PreqTransaction_beforeStop(Actor *this) {

}

/* -------------------------------------------Actor section---------------------------------------------- */

bool PreqTransaction_receive(Actor *this, uint16_t type, void *msg) {
    if (type != m_Initialize) {
        fsm_box *box = this->state;

        switch (box->state->code) {
            case st_PreqTransaction_Idle:
                switch (box->data->code) {
                    case dt_PreqTransaction_Uninitialized:
                        switch (type) {
                            case m_Transaction_Exec:
                                ev_PreqTransaction_Idle_Uninitialized_Exec(this, msg);
                                break;
                            case m_FSM_StateTimeout:
                                ev_PreqTransaction_Idle_Uninitialized_StateTimeout(this, msg);
                                break;
                            default:
                                break;
                        }
                        break;
                    default:
                        break;
                }
                break;
            case st_PreqTransaction_WaitAcks:
                switch (box->data->code) {
                    case dt_PreqTransaction_WaitingAcks:
                        switch (type) {
                            case m_Transaction_ReceivePacket:
                                ev_PreqTransaction_Idle_WaitingAcks_ReceivePacket(this, msg);
                                break;
                            case m_FSM_StateTimeout:
                                ev_PreqTransaction_Idle_WaitingAcks_StateTimeout(this, msg);
                                break;
                            default:
                                break;
                        }
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }
    } else {
        mp_PreqTransaction_Initialize(this, msg);
    }

    return true;
}

Actor* new_PreqTransaction() {
    Actor *actor = new_Actor(PreqTransaction_receive, PreqTransaction_beforeStop, new_fsm_box(NULL, NULL));

    return actor;
}