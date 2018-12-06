#include <string.h>
#include "../include/a_connector.h"
#include "../libs/miniakka/includes/standard_messages.h"
#include "../libs/miniakka/includes/actor_utils.h"
#include "../oscl/include/a_tpcio.h"
#include "../oscl/include/a_tcp_connection_creator.h"
#include "../include/a_gateway_pool.h"
#include "../include/a_gateway.h"

/** Коннтектор нативных сетевых соединений. Задачей актора является
  * продукция шлюзов из нативных сетевых соединений. После создания актора,
  * он находится в неинициализированном режиме. В данном режим он должен получить
  * некоторое сообщения из группы Connect... . В процессе обработки последнего
  * актор пробует установить соединение через слой OSCL. Если операция
  * завершается созданеим актора содениения, формируется новый шлюз,
  * добавляется в пул шлюзов, после чего актор переходит в режим ожанния. Если
  * соединение не получается установить, актор совершает повторную попытку соединения
  * через установленный временной промежуток. В режиме ожидания актор может
  * быть безопасно остановлен сообщением Stop. Так же в этом же режиме актор
  * ожидает сообщение терминации шлюза Stopped. При получении такового,
  * производит повторную попытку соединения с продукцией новго шлюза в замен
  * терминированного.
  */

typedef struct s_Connector {
    /** Референс пула шлюзов */
    ActorRef *gwPool;
    /** Референс таблицы маршрутизации */
    ActorRef *rTable;
    /** Референс роутера ресиверов */
    Router* receivers;
    /** Оригинальное сообщенние полученное при инициализации */
    ActorMessage *originalMessage;
    /** Задерка при повторном создании соединения из-за ошибка TCPIO */
    uint32_t retryMs;
    /** Режим работы коннектора:
    *  0 = TCP
    * -1 = ни инициализрован,
    * -2 =  ожадание соединения после терминации шлюза */
    int8_t mode;
} s_Connector;

// =====================================================================================================================

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/** Connect
 *
 * Инициализирует актор в качестве коннектора TCP уровня. Совершает звпроск к TCPIO для
 * создания нового соединения */

void del_m_Connector_ConnectTcp(b_Connector_ConnectTcp *b) {
    pfree(b->host);
}

ActorMessage* new_m_Connector_ConnectTcp(ActorRef *tcpIo, char *host, uint16_t post, uint32_t retryMs, uint16_t stackSize) {
    char *dup_host = pmalloc(strlen(host) + 1);
    memcpy(dup_host, host, strlen(host) + 1);
    b_Connector_ConnectTcp *body = pmalloc(sizeof(b_Connector_ConnectTcp));

    body->host = dup_host;
    body->port = post;
    body->retryMs = retryMs;
    body->tcpIo = AU_copyRef(tcpIo);

    return new_ActorMessage(m_Connector_ConnectTcp, body, stackSize, del_m_Connector_ConnectTcp);
}

void mp_Connector_ConnectTcp(Actor *this, b_Connector_ConnectTcp *b) {
    s_Connector *state = this->state;

    if (state->mode < 0) {
        state->originalMessage = new_m_Connector_ConnectTcp(b->tcpIo, b->host, b->port, b->retryMs, 32);
        state->retryMs = b->retryMs;
        state->mode = 1;
        b->tcpIo->tell(b->tcpIo, new_m_TcpIO_Connect(b->host, b->port, 32), this->context->self, this->context->system);
    } else {
        this->context->sender->tell(this->context->sender, new_r_Connector_AlreadyRun(32), this->context->self, this->context->system);
    }
};


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/** Resp - AlreadyConnected
 *
 * Ответ при попытке повторной инициализации коннектора */

ActorMessage* new_r_Connector_AlreadyRun(uint16_t stackSize) {
    return new_ActorMessage(r_Connector_AlreadyRun, NULL, stackSize, del_default);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/** TcpConnectionCreator_Connection
 *
 * NETWORK IO через которое проивзодится попытка установки соединения вернуло референс
 * соединения. Создает новый шлюз для этого референса и добавляет его в пул шлюзов */

void mp_Connector_TcpConnectionCreator_Connection(Actor *this, b_TcpConnectionCreator_Connection *body) {
    s_Connector *state = this->state;

    Actor *gw = new_Gateway(body->ref, state->rTable, this->context->self, state->receivers, NULL, false);
    ActorSystem *system = AU_system(this);
    ActorRef *gateway = system->actorOf(system, gw , NULL);
    state->gwPool->tell(state->gwPool, new_m_GatewayPool_Add(gateway, 32), this->context->self, this->context->system);
    AU_freeRef(gateway);
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/** TcpConnectionCreator - ConnectionFailed
 *
 * NETWORK IO через которое проивзодится попытка установки соединения вернуло ошибку создания
 * соединения. Переводит актор в режим -2 и отправляет первичное сообщение
 * инициализации самому себе через задержку */

void mp_Connector_TcpConnectionCreator_ConnectionFailed(Actor *this, b_TcpConnectionCreator_Connection *body) {
    s_Connector *state = this->state;
    state->mode = -2;


    ActorSystem *system = AU_system(this);
    ActorRef *self = this->context->self;
    char *key = system->scheduler->scheduleOnce(system->scheduler, NULL, 3000, self, state->originalMessage, self);
    pfree(key);
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/** Gateway - Stopped
 *
 * Сообщение терминации обслуживаемого шлюза (созданный ранее шлюз был терминирован по каким
 * либо внутренним причинам). Переводит актор в режим -2 и отправляет первичное сообщение
 * инициализации самому себе */

void mp_Connector_Gateway_Stopped(Actor *this, b_TcpConnectionCreator_Connection *body) {
    s_Connector *state = this->state;
    state->mode = -2;
    this->context->self->tell(this->context->self, state->originalMessage, this->context->self, this->context->system);
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void Connector_beforeStop(Actor *this) {
    //TODO нормально сделай, по типам как в акцепторе
    s_Connector *state = (s_Connector *) this->state;
    pfree(state->gwPool);
    pfree(state->rTable);
    pfree(state->originalMessage);
}

bool Connector_receive(Actor *this, uint16_t type, void *msg) {
    switch (type) {
        case m_Connector_ConnectTcp:
            mp_Connector_ConnectTcp(this, msg);
            break;
        case r_TcpConnectionCreator_Connection:
            mp_Connector_TcpConnectionCreator_Connection(this, msg);
            break;
        case r_TcpConnectionCreator_ConnectionFailed:
            mp_Connector_TcpConnectionCreator_ConnectionFailed(this, msg);
            break;
        case r_Gateway_Stopped:
            mp_Connector_Gateway_Stopped(this, msg);
            break;
        default: break;
    }

    return true;
}

Actor* new_Connector(ActorRef* gwPool, ActorRef* rTable, Router* receivers) {
    s_Connector *state = pmalloc(sizeof(s_Connector));
    state->gwPool = AU_copyRef(gwPool);
    state->rTable = AU_copyRef(rTable);
    state->receivers = AU_copyRouter(receivers);
    state->originalMessage = NULL;
    state->retryMs = 0;
    state->mode = -1;

    Actor *actor = new_Actor(Connector_receive, Connector_beforeStop, state);
    return actor;
}