#include "../include/a_gateway_pool.h"
#include <stdlib.h>
#include "../libs/miniakka/includes/actor_utils.h"
#include "../libs/miniakka/includes/standard_messages.h"
#include "../include/a_gateway.h"
#include "../oscl/include/data.h"
#include "../libs/collections/includes/map2.h"

/** Пул шлюзов mnvp сети. Предназначен для проведения процедуры лукапинга целвого шлюза
  * по метке или сетевому адресу. Каждый пакет входящий в драйвер, получает метку шлюза
  * из которого он пришел.Используя эту метку, через данный пул, можно произвести определение
  * актора шлюза который собрал некоторый пакет. Так же пул поддерживает прямую
  * трансляцию запросов к шлюзу (режим роутинга сообщения).
  */

typedef struct s_GatewayPool {
    /** Пул референсов шлюзов */
    Map *pool;
} s_GatewayPool;

// =====================================================================================================================

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/** Add
 *
 * Добавляет референс шлюза в пул. Генерирует случайную метку, устраняя возможность
 * коллизии с существующими метками и нулвеого значения. Указыват шлюзу установить
 * сгенерированную метку в качестве внутренней метки. */

void del_m_GatewayPool_Add(b_GatewayPool_Add *b) {
    AU_freeRef(b->ref);
}

ActorMessage* new_m_GatewayPool_Add(ActorRef *ref, uint16_t stackSize) {
    ActorRef *dup_ref = AU_copyRef(ref);
    b_GatewayPool_Add *body = pmalloc(sizeof(b_GatewayPool_Add));
    body->ref = dup_ref;

    return new_ActorMessage(m_GatewayPool_Add, body, stackSize, del_m_GatewayPool_Add);
}

void mp_GatewayPool_Add(Actor *this, b_GatewayPool_Add *b) {
    s_GatewayPool *state = this->state;

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

    ActorRef *dup_ref = AU_copyRef(b->ref);
    dup_ref->tell(dup_ref, new_m_Gateway_SetLabel(label, 32), this->context->self, this->context->system); //Пока сообщение недошло, у шлюза лейбл 0, он не будет пытаться себя удалить, пока до него не дойдет это сообщение
    MAP_add(key, dup_ref, state->pool);
    pfree(key);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/** GetByLabel
 *
 * Возвращает источнику некоторый референс шлюза по его метке */

ActorMessage* new_m_GatewayPool_GetByLabel(uint32_t label, uint16_t stackSize) {
    b_GatewayPool_GetByLabel *body = pmalloc(sizeof(b_GatewayPool_GetByLabel));
    body->label = label;

    return new_ActorMessage(m_GatewayPool_GetByLabel, body, stackSize, del_default);
}

void mp_GatewayPool_GetByLabel(Actor *this, b_GatewayPool_GetByLabel *b) {
    s_GatewayPool *state = this->state;

    char *key = itoa(b->label);
    ActorRef *gwRef = MAP_get(key, state->pool);
    pfree(key);
    this->context->sender->tell(this->context->sender, new_r_GatewayPool_Gateway(gwRef, 32), this->context->self, this->context->system);
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/** RemoveByLabel
 *
 * Удаляет референс шлюза из пула по его метке */

ActorMessage* new_m_GatewayPool_RemoveByLabel(uint32_t label, uint16_t stackSize) {
    b_GatewayPool_RemoveByLabel *body = pmalloc(sizeof(b_GatewayPool_RemoveByLabel));
    body->label = label;

    return new_ActorMessage(m_GatewayPool_RemoveByLabel, body, stackSize, del_default);
}

void mp_GatewayPool_RemoveByLabel(Actor *this, b_GatewayPool_RemoveByLabel *b) {
    s_GatewayPool *state = this->state;

    char *key = itoa(b->label);
    ActorRef *gwRef = MAP_remove(key, state->pool);
    if (gwRef != NULL)
        AU_freeRef(gwRef);
    pfree(key);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/** RouteToAll
  *
  * Производит роутинг сообщения всем имеющимся в пуле шлюзам
  *
  * @param message сообщение для роутинга
  * @param sender отправитель сообщения
  */

void del_GatewayPool_RouteToAll(b_GatewayPool_RouteToAll *b) {
    AU_freeMsg(b->message, true);
}

ActorMessage* copyInnerMessage(ActorMessage *msg) {
    if (msg->type == m_Gateway_Send) {
        b_Gateway_Send *body = msg->body;
        b_Gateway_Send *dup_body = pmalloc(sizeof(b_Gateway_Send));
        dup_body->packet = copy_Packet(body->packet);

        return AU_copyMsg(msg, dup_body);
    } else if (msg->type == m_Gateway_SetLabel) {
        b_Gateway_SetLabel *body = msg->body;
        b_Gateway_SetLabel *dup_body = pmalloc(sizeof(b_Gateway_SetLabel));
        dup_body->label = body->label;

        return AU_copyMsg(msg, dup_body);
    } else if (msg->type == m_Gateway_SetNetworkAddress) {
        b_Gateway_SetNetworkAddress *body = msg->body;
        b_Gateway_SetNetworkAddress *dup_body = pmalloc(sizeof(b_Gateway_SetNetworkAddress));
        dup_body->netAdr = body->netAdr;

        return AU_copyMsg(msg, dup_body);
    }

    return NULL;
}

ActorMessage* new_m_GatewayPool_RouteToAll(ActorMessage *message, uint16_t stackSize) {
    b_GatewayPool_RouteToAll *body = pmalloc(sizeof(b_GatewayPool_RouteToAll));
    body->message = copyInnerMessage(message);

    return new_ActorMessage(m_GatewayPool_RouteToAll, body, stackSize, del_GatewayPool_RouteToAll);
}

void mp_GatewayPool_RouteToAll(Actor *this, b_GatewayPool_RouteToAll *b) {
    s_GatewayPool *state = this->state;


    ListIterator *iterator = state->pool->inner->iterator(state->pool->inner);

    while (iterator->hasNext(iterator)) {
        MapItem *item = iterator->next(iterator);
        ActorRef *gwRef = (ActorRef*) item->value;
        gwRef->tell(gwRef, copyInnerMessage(b->message), this->context->sender, this->context->system);
    }

    pfree(iterator);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/** RouteByLabel
  *
  * Производит роутинг сообщения к шлюзу с указанной меткой
  *
  * @param gwLabel метка целевого
  * @param message сообщение для роутинга
  * @param sender отправитель сообщения
  */

void del_GatewayPool_RouteByLabel(b_GatewayPool_RouteByLabel *b) {
    AU_freeMsg(b->message, true);
}

ActorMessage* new_m_GatewayPool_RouteByLabel(uint16_t gwLabel, ActorMessage *message, uint16_t stackSize) {
    b_GatewayPool_RouteByLabel *body = pmalloc(sizeof(b_GatewayPool_RouteByLabel));
    body->gwLabel = gwLabel;
    body->message = copyInnerMessage(message);

    return new_ActorMessage(m_GatewayPool_RouteByLabel, body, stackSize, del_GatewayPool_RouteByLabel);
}

void mp_GatewayPool_RouteByLabel(Actor *this, b_GatewayPool_RouteByLabel *b) {
    s_GatewayPool *state = this->state;


    ListIterator *iterator = state->pool->inner->iterator(state->pool->inner);

    while (iterator->hasNext(iterator)) {
        MapItem *item = iterator->next(iterator);
        if (atoi(item->key) == b->gwLabel) {
            ActorRef *gwRef = (ActorRef *) item->value;
            gwRef->tell(gwRef, copyInnerMessage(b->message), this->context->sender, this->context->system);
            break;
        }
    }

    pfree(iterator);
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/** Resp - Gateway
 *
 * Ответ по сообщениям get. Создержит рефернс шлюза или None если
 * по каким-то причинам он не обнуружен в пуле. */

void del__GatewayPool_Gateway(b_GatewayPool_Gateway *b) {
    if (b->ref != NULL)
        AU_freeRef(b->ref);
}

ActorMessage* new_r_GatewayPool_Gateway(ActorRef *ref, uint16_t stackSize) {
    b_GatewayPool_Gateway *body = pmalloc(sizeof(b_GatewayPool_Gateway));
    if (ref != NULL)
        body->ref = AU_copyRef(ref);
    else
        body->ref = NULL;

    return new_ActorMessage(r_GatewayPool_Gateway, body, stackSize, del__GatewayPool_Gateway);
}


void GatewayPool_beforeStop(Actor *this) {
    s_GatewayPool *state = (s_GatewayPool *) this->state;
    //TODO освобождение памяти коллекции
    pfree(state->pool);
}

bool GatewayPool_receive(Actor *this, uint16_t type, void *msg) {
    switch (type) {
        case m_GatewayPool_Add:
            mp_GatewayPool_Add(this, msg);
            break;
        case m_GatewayPool_GetByLabel:
            mp_GatewayPool_GetByLabel(this, msg);
            break;
        case m_GatewayPool_RemoveByLabel:
            mp_GatewayPool_RemoveByLabel(this, msg);
            break;
        case m_GatewayPool_RouteToAll:
            mp_GatewayPool_RouteToAll(this, msg);
            break;
        case m_GatewayPool_RouteByLabel:
            mp_GatewayPool_RouteByLabel(this, msg);
            break;
        default: break;
    }

    return true;
}

Actor* new_GatewayPool() {
    s_GatewayPool *state = pmalloc(sizeof(s_GatewayPool));

    state->pool = MAP_new();

    return new_Actor(GatewayPool_receive, GatewayPool_beforeStop, state);
}