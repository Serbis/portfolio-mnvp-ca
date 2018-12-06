#include "../includes/fsm.h"
#include "../../../oscl/include/malloc.h"
#include "../includes/standard_messages.h"
#include "../includes/actor_system.h"
#include "../includes/actor_utils.h"

fsm_box* new_fsm_box(fsm_data* data, fsm_state* state) {
    fsm_box *box = pmalloc(sizeof(fsm_box));
    box->data = data;
    box->state = state;

    return box;
}

fsm_state* new_fsm_state(uint8_t code, char* timer) {
    fsm_state *state = pmalloc(sizeof(fsm_state));
    state->code = code;
    state->timer = timer;

    return state;
}

fsm_data* new_fsm_data(uint8_t code, void* data, void (*destructor)(void*)) {
    fsm_data *dt = pmalloc(sizeof(fsm_data));
    dt->code = code;
    dt->data = data;
    dt->destrutor = destructor;

    return dt;
}

ActorMessage* new_m_FSM_StateTimeout(uint8_t stateCode, uint16_t stackSize) {
    b_FSM_StateTimeout *body = pmalloc(sizeof(b_FSM_StateTimeout));
    body->stateCode = stateCode;

    return new_ActorMessage(m_FSM_StateTimeout, body, stackSize, del_default);
}

void FSM_toState(Actor *actor, fsm_data *data, uint8_t state, uint32_t stateTimeout, uint16_t stateTimeoutStack) {
    fsm_box *box = (fsm_box*) actor->state;
    ActorSystem *system = AU_system(actor);
    ActorRef *self = actor->context->self;

    if (box->state != NULL) {
        system->scheduler->cancel(system->scheduler, box->state->timer); //Отменить таймер таймаута состояния
        pfree(box->state->timer); //Освободить ключ таймера
        pfree(box->state); //Освободить память текущего состояния

    }

    if (box->data != NULL) {
        box->data->destrutor(box->data->data); //Вызвать деструктор текущих данных]
        pfree(box->data->data);
        pfree(box->data); //Освободить память текущих данных
    }

    box->data = data; //Установить новое состояние в качестве текущего
    fsm_state *newState = new_fsm_state(state, system->scheduler->scheduleOnce(system->scheduler, NULL, stateTimeout, self, new_m_FSM_StateTimeout(state, stateTimeoutStack), self));
    box->state = newState; //Установить новые данные в качестве текущих
}

void FSM_stay(Actor *actor, uint32_t stateTimeout, uint16_t stateTimeoutStack) {
    fsm_box *box = (fsm_box*) actor->state;
    ActorSystem *system = AU_system(actor);
    ActorRef *self = actor->context->self;

    if (box->state != NULL) {
        system->scheduler->cancel(system->scheduler, box->state->timer); //Отменить таймер таймаута состояния
        pfree(box->state->timer); //Освободить ключ таймера
        box->state->timer = system->scheduler->scheduleOnce(system->scheduler, NULL, stateTimeout, self, new_m_FSM_StateTimeout(box->state->code, stateTimeoutStack), self);
    }
}

void FSM_stop(Actor *actor, uint16_t stackSize) {
    fsm_box *box = (fsm_box*) actor->state;
    ActorSystem *system = AU_system(actor);
    ActorRef *self = actor->context->self;

    if (box->state != NULL) {
        system->scheduler->cancel(system->scheduler, box->state->timer); //Отменить таймер таймаута состояния
        pfree(box->state->timer); //Освободить ключ таймера
        pfree(box->state); //Освободить память текущего состояния
        box->state = NULL;
    }

    if (box->data != NULL) {
        box->data->destrutor(box->data->data); //Вызвать деструктор текущих данных
        pfree(box->data->data);
        pfree(box->data); //Освободить память текущих данных
        box->data = NULL;
    }

    self->tell(self, new_m_Stop(stackSize), self, system);
}