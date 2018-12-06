#include "../include/a_send_packet_fsm.h"
#include "../libs/miniakka/includes/standard_messages.h"
#include "../libs/miniakka/includes/actor_utils.h"
#include "../include/a_rtable.h"
#include "../include/a_gateway_pool.h"
#include "../include/a_gateway.h"


/* -----------------------------------Messages creation section---------------------------------------- */

//COMMAND ------> Exec

/** Стартовое сообщение
  *
  * @param packet пакет для отправки
  */

void del_SendPacketFsm_Exec(b_SendPacketFsm_Exec *b) {
    free_Packet(b->packet);
}

ActorMessage* new_m_SendPacketFsm_Exec(Packet *packet, uint16_t stackSize) {
    b_SendPacketFsm_Exec *body = pmalloc(sizeof(b_SendPacketFsm_Exec));
    body->packet = copy_Packet(packet);

    return new_ActorMessage(m_SendPacketFsm_Exec, body, stackSize, del_SendPacketFsm_Exec);
}

//RESPONSE ------> SendResult

/** Результат выполнения отправки пакте. Возможны следующие коды:
  *   0 - пакет отправлен в шлюз
  *   1 - не найдено маршрутных записей до цели
  *   2 - таблица маршрутизации не ответала на запрос о поиске маршрута
  *
  * @param code код результата
  */

ActorMessage* new_r_SendPacketFsm_SendResult(uint8_t code, uint16_t stackSize) {
    b_SendPacketFsm_SendResult *body = pmalloc(sizeof(b_SendPacketFsm_SendResult));
    body->code = code;

    return new_ActorMessage(r_SendPacketFsm_SendResult, body, stackSize, del_default);
}

/* -------------------------------------Data creation section------------------------------------------ */

/** @param gwPool референс пула шлюзов
  * @param rTable референс таблицы маршрутизации
  */

//DATA --------> Uninitialized
fsm_data* del_dt_SendPacketFsm_Uninitialized(dtb_SendPacketFsm_Uninitialized *b) {
    AU_freeRef(b->gwPool);
    AU_freeRef(b->rTable);
}

fsm_data* new_dt_SendPacketFsm_Uninitialized(ActorRef *rTable, ActorRef *gwPool, bool testMode) {
    dtb_SendPacketFsm_Uninitialized *db = pmalloc(sizeof(dtb_SendPacketFsm_Uninitialized));
    db->rTable = AU_copyRef(rTable);
    db->gwPool = AU_copyRef(gwPool);
    db->testMode = testMode;

    return new_fsm_data(dt_SendPacketFsm_Uninitialized, db, del_dt_SendPacketFsm_Uninitialized);
}

//DATA --------> WaitingRouteResponse

/** @param gwPool референс пула шлюзов
  * @param rTable референс таблицы маршрутизации
  * @param orig ориджинатор
  * @param packet отправляемый пакет
  */

fsm_data* del_dt_SendPacketFsm_WaitingRouteResponse(dtb_SendPacketFsm_WaitingRouteResponse *b) {
    AU_freeRef(b->gwPool);
    AU_freeRef(b->rTable);
    AU_freeRef(b->orig);
    free_Packet(b->packet);
}

fsm_data* new_dt_SendPacketFsm_WaitingRouteResponse(ActorRef *rTable, ActorRef *gwPool, bool testMode, ActorRef *orig,
                                                    Packet *packet) {
    dtb_SendPacketFsm_WaitingRouteResponse* data = pmalloc(sizeof(dtb_SendPacketFsm_WaitingRouteResponse));
    data->rTable = AU_copyRef(rTable);
    data->gwPool = AU_copyRef(gwPool);
    data->testMode = testMode;
    data->packet = copy_Packet(packet);
    data->orig = AU_copyRef(orig);

    return new_fsm_data(dt_SendPacketFsm_WaitingRouteResponse, data, del_dt_SendPacketFsm_WaitingRouteResponse);
}

/* --------------------------------------States logic section------------------------------------------ */

//STATE --------> Idle / DATA ----------> Uninitialized

/** Стартовое состояние. Поличив сообщение Exec запрошивает поиск маршрута до целевогоу узла из пакета
  * у таблицы паршрутизации.
  */

void ev_SendPacketFsm_Idle_Uninitialized_Exec(Actor *this, b_SendPacketFsm_Exec *b) {
    dtb_SendPacketFsm_Uninitialized *d = AU_fsm_data(this);

    d->rTable->tell(d->rTable, new_m_Rtable_FindRoute(b->packet->dest, 32), this->context->self, this->context->system);
    fsm_data *newData = new_dt_SendPacketFsm_WaitingRouteResponse(d->rTable, d->gwPool, d->testMode,
            this->context->sender, b->packet);
    if (d->testMode)
        FSM_toState(this, newData, st_SendPacketFsm_WaitRouteResponse, 1000, 32);
    else
        FSM_toState(this, newData, st_SendPacketFsm_WaitRouteResponse, 5000, 32);
}

void ev_SendPacketFsm_Idle_Uninitialized_StateTimeout(Actor *this, b_FSM_StateTimeout *b) {
    FSM_stop(this, 32);
}

//STATE --------> WaitAcks / DATA ----------> WaitingAcks

/** Состояние ожидания ответа от таблицы маршрутизации. Если маршрут найден, отправляет пакет в целевой шлюз и
  * отвечает ориджинатору результатом с кодом 1. Если нет,то отвечает результатом с кодом 1. Если таблица
  * маршрутизации своевременно не прислала ответ, отправляет результат с кодом 2.
  */

void ev_SendPacketFsm_Idle_WaitingAcks_Route(Actor *this, b_Rtable_Route *b) {
    dtb_SendPacketFsm_WaitingRouteResponse *d = AU_fsm_data(this);

    if (b->gwl != 0) {
        ActorMessage *gwsMsg = new_m_Gateway_Send(d->packet, 32);
        d->gwPool->tell(d->gwPool, new_m_GatewayPool_RouteByLabel(b->gwl, gwsMsg, 32), this->context->self, this->context->system);
        AU_freeMsg(gwsMsg, true);
        d->orig->tell(d->orig, new_r_SendPacketFsm_SendResult(0, 32), this->context->self, this->context->system);
    } else {
        d->orig->tell(d->orig, new_r_SendPacketFsm_SendResult(1, 32), this->context->self, this->context->system);
    }

    FSM_stop(this, 32);
}

void ev_SendPacketFsm_Idle_WaitingAcks_StateTimeout(Actor *this, b_FSM_StateTimeout *b) {
    dtb_SendPacketFsm_WaitingRouteResponse *d = AU_fsm_data(this);

    d->orig->tell(d->orig, new_r_SendPacketFsm_SendResult(2, 32), this->context->self, this->context->system);
    FSM_stop(this, 32);
}

/* ----------------------------------------Initialize message-------------------------------------------- */


void del_m_SendPacketFsm_Initialize(struct b_SendPacketFsm_Initialize *b) {
    AU_freeRef(b->gwPool);
    AU_freeRef(b->rTable);
}

ActorMessage* new_m_SendPacketFsm_Initialize(ActorRef *rTable, ActorRef *gwPool, bool testMode, uint16_t stackSize) {
    b_SendPacketFsm_Initialize *body = pmalloc(sizeof(b_SendPacketFsm_Initialize));
    body->rTable = AU_copyRef(rTable);
    body->gwPool = AU_copyRef(gwPool);
    body->testMode = testMode;

    return new_ActorMessage(m_Initialize, body, stackSize, del_m_SendPacketFsm_Initialize);
}

void mp_SendPacketFsm_Initialize(Actor *this, b_SendPacketFsm_Initialize* b) {
    fsm_data *data = new_dt_SendPacketFsm_Uninitialized(b->rTable, b->gwPool, b->testMode);
    FSM_toState(this, data, st_SendPacketFsm_Idle, 5000, 32);
}

void SendPacketFsm_beforeStop(Actor *this) {

}

/* -------------------------------------------Actor section---------------------------------------------- */

bool SendPacketFsm_receive(Actor *this, uint16_t type, void *msg) {
    if (type != m_Initialize) {
        fsm_box *box = this->state;

        switch (box->state->code) {
            case st_SendPacketFsm_Idle:
                switch (box->data->code) {
                    case dt_SendPacketFsm_Uninitialized:
                        switch (type) {
                            case m_SendPacketFsm_Exec:
                                ev_SendPacketFsm_Idle_Uninitialized_Exec(this, msg);
                                break;
                            case m_FSM_StateTimeout:
                                ev_SendPacketFsm_Idle_Uninitialized_StateTimeout(this, msg);
                                break;
                            default:
                                break;
                        }
                        break;
                    default:
                        break;
                }
                break;
            case st_SendPacketFsm_WaitRouteResponse:
                switch (box->data->code) {
                    case dt_SendPacketFsm_WaitingRouteResponse:
                        switch (type) {
                            case r_Rtable_Route:
                                ev_SendPacketFsm_Idle_WaitingAcks_Route(this, msg);
                                break;
                            case m_FSM_StateTimeout:
                                ev_SendPacketFsm_Idle_WaitingAcks_StateTimeout(this, msg);
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
        mp_SendPacketFsm_Initialize(this, msg);
    }

    return true;
}

Actor* new_SendPacketFsm() {
    Actor *actor = new_Actor(SendPacketFsm_receive, SendPacketFsm_beforeStop, new_fsm_box(NULL, NULL));

    return actor;
}