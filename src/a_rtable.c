#include "../include/a_rtable.h"
#include "../libs/miniakka/includes/standard_messages.h"

/** Таблица маршрутиации узла. В задачи данного актора входит создание маршрутов, удаление маршрутов и
  * поиск маршрутов по заданным условиям */

/** Определение записи таблицы маршрутизации
 *
 * @param dest сетевой адрес целевого узла
 * @param gateway шлюз через который достижим целевой узел
 * @param distance дистанция в прыжках до целевого узла
 */
typedef struct RouteEntry {
    uint32_t dest;
    uint32_t gateway;
    uint16_t gwl;
    uint16_t distance;
} RouteEntry;

typedef struct s_Rtable {
    /** Списрок реализующи таблицу маршрутизаци */
    List *table;
} s_Rtable;


// =====================================================================================================================

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/** RemoveAllRoutesByGateway
  *
  * Удаляет все маршруты с заданным сетевым адресом шлюза
  *
  * @param gateway сетевой адрес шлюза
  */

ActorMessage* new_m_Rtable_RemoveAllRoutesByGateway(uint32_t gwAdr, uint16_t stackSize) {
ActorMessage *msg = pmalloc(sizeof(ActorMessage));

b_Rtable_RemoveAllRoutesByGateway *body = pmalloc(sizeof(b_Rtable_RemoveAllRoutesByGateway));
body->gwAdr = gwAdr;

msg->type = m_Rtable_RemoveAllRoutesByGateway;
msg->body = body;
msg->stackSize = stackSize;
msg->destructor =  del_default;

return msg;
}

void mp_Rtable_RemoveAllRoutesByGateway(Actor *this, b_Rtable_RemoveAllRoutesByGateway *b) {
s_Rtable *state = this->state;

ListIterator *iterator = state->table->iterator(state->table);

while(iterator->hasNext(iterator)) {
    RouteEntry *entry = iterator->next(iterator);
    if (entry->gateway == b->gwAdr)
        iterator->remove(iterator);
}

pfree(iterator);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/** UpdateRoute
  *
  * Обновляет/создает новый маршрут
  *
  * @param dest сетевой адрес цели
  * @param gateway сетевой адрес шлюза
  * @param distance дистанция до цели
  */

ActorMessage* new_m_Rtable_UpdateRoute(uint32_t dest, uint32_t gateway, uint16_t gwl, uint16_t distance, uint16_t stackSize) {
    b_Rtable_UpdateRoute *body = pmalloc(sizeof(b_Rtable_UpdateRoute));
    body->dest = dest;
    body->gateway = gateway;
    body->distance = distance;
    body->gwl = gwl;

    return new_ActorMessage(m_Rtable_UpdateRoute, body, stackSize, del_default);
}

void mp_Rtable_UpdateRoute(Actor *this, b_Rtable_UpdateRoute *b) {
    if (b->gwl == 0)
        return;

    s_Rtable *state = this->state;

    ListIterator *iterator = state->table->iterator(state->table);
    RouteEntry *entry = NULL;

    while(iterator->hasNext(iterator)) {
        RouteEntry *locEntry = iterator->next(iterator);
        if (locEntry->dest == b->dest && locEntry->gateway == b->gateway && locEntry->distance == b->distance)
            entry = locEntry;
    }

    if (entry == NULL) {
        entry = pmalloc(sizeof(RouteEntry));
        entry->dest = b->dest;
        entry->gateway = b->gateway;
        entry->distance = b->distance;
        entry->gwl = b->gwl;

        state->table->prepend(state->table, entry);
    }

    pfree(iterator);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/** RemoveAllRoutesByDest
  *
  * Удаляет все маршруты с заданным сетевом адресом цели
  *
  * @param dest сетевой адрес цели
  */

ActorMessage* new_m_Rtable_RemoveAllRoutesByDest(uint32_t dest, uint16_t stackSize) {
    b_Rtable_RemoveAllRoutesByDest *body = pmalloc(sizeof(b_Rtable_RemoveAllRoutesByDest));
    body->dest = dest;

    return new_ActorMessage(m_Rtable_RemoveAllRoutesByDest, body, stackSize, del_default);
}

void mp_Rtable_RemoveAllRoutesByDest(Actor *this, b_Rtable_RemoveAllRoutesByDest *b) {
    s_Rtable *state = this->state;

    ListIterator *iterator = state->table->iterator(state->table);

    while(iterator->hasNext(iterator)) {
        RouteEntry *entry = iterator->next(iterator);
        if (entry->dest == b->dest)
            iterator->remove(iterator);
    }

    pfree(iterator);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/** FindRoute
  *
  * Возращает наиболее оптимальный маршрут до цели
  *
  * @param dest сетевой адрес цели
  */
ActorMessage* new_m_Rtable_FindRoute(uint32_t dest, uint16_t stackSize) {
    b_Rtable_FindRoute *body = pmalloc(sizeof(b_Rtable_FindRoute));
    body->dest = dest;

    return new_ActorMessage(m_Rtable_FindRoute, body, stackSize, del_default);
}

void mp_Rtable_FindRoute(Actor *this, b_Rtable_FindRoute *b) {
    s_Rtable *state = this->state;

    uint16_t min = 65535;
    ListIterator *iterator = state->table->iterator(state->table);
    RouteEntry *entry = NULL;

    while(iterator->hasNext(iterator)) {
        RouteEntry *locEntry = iterator->next(iterator);
        if (locEntry->dest == b->dest && locEntry->distance < min) {
            entry = locEntry;
            min = locEntry->distance;
        }
    }

    ActorRef *sender = this->context->sender;
    if (entry != NULL) {
        sender->tell(sender, new_r_Rtable_Route(entry->gateway, entry->gwl, entry->distance, 32), this->context->sender, this->context->system);
    } else {
        sender->tell(sender, new_r_Rtable_Route(0, 0, 0, 32), this->context->sender, this->context->system);
    }

    pfree(iterator);

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/** Resp - Route
  *
  * Определение маршрута возвращаемого в качестве ответа на запрос поиска маршрута
  *
  * @param gateway сетевой адрес шлюза по которому достижима цель
  * @param distance дистанция в прыжках до цели
  */

ActorMessage* new_r_Rtable_Route(uint32_t gateway, uint16_t gwl, uint16_t dist, uint16_t stackSize) {
    b_Rtable_Route *body = pmalloc(sizeof(b_Rtable_Route));
    body->gateway = gateway;
    body->dist = dist;
    body->gwl = gwl;

    return new_ActorMessage(r_Rtable_Route, body, stackSize, del_default);
}


void Rtable_beforeStop(Actor *this) {
    s_Rtable *state = (s_Rtable *) this->state;
    //TODO освобождение коллекции
    pfree(state->table);
}

bool Rtable_receive(Actor *this, uint16_t type, void *msg) {
    switch (type) {
        case m_Rtable_RemoveAllRoutesByGateway:
            mp_Rtable_RemoveAllRoutesByGateway(this, msg);
            break;
        case m_Rtable_RemoveAllRoutesByDest:
            mp_Rtable_RemoveAllRoutesByDest(this, msg);
            break;
        case m_Rtable_UpdateRoute:
            mp_Rtable_UpdateRoute(this, msg);
            break;
        case m_Rtable_FindRoute:
            mp_Rtable_FindRoute(this, msg);
            break;
        default: break;
    }

    return true;
}

Actor* new_Rtable() {
    s_Rtable *state = pmalloc(sizeof(s_Rtable));
    state->table = new_List();

    return new_Actor(Rtable_receive, Rtable_beforeStop, state);
}