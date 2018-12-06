#include "../includes/simple_fsm.h"
#include "../../../oscl/include/data.h"
#include "../includes/standard_messages.h"
#include "../includes/actor_utils.h"

/* -----------------------------------Messages creation section---------------------------------------- */

//MESSAGE ------> Message0
void del_m_SimpleFsm_Message0(b_SimpleFsm_Message0 *b) {
    pfree(b->str1);
}

ActorMessage* new_m_SimpleFsm_Message0(char *str1, uint16_t stackSize) {
    b_SimpleFsm_Message0 *body = pmalloc(sizeof(b_SimpleFsm_Message0));
    body->str1 = strcpy2(str1);

    return new_ActorMessage(m_SimpleFsm_Message0, body, stackSize, del_m_SimpleFsm_Message0);
}

//MESSAGE ------> Message1
void del_m_SimpleFsm_Message1(b_SimpleFsm_Message1 *b) {
    pfree(b->str2);
}

ActorMessage* new_m_SimpleFsm_Message1(char *str2, uint16_t stackSize) {
    b_SimpleFsm_Message1 *body = pmalloc(sizeof(b_SimpleFsm_Message1));
    body->str2 = strcpy2(str2);

    return new_ActorMessage(m_SimpleFsm_Message1, body, stackSize, del_m_SimpleFsm_Message1);
}

//MESSAGE ------> Message2
ActorMessage* new_m_SimpleFsm_Message2(uint16_t stackSize) {
    return new_ActorMessage(m_SimpleFsm_Message2, NULL, stackSize, del_default);
}

/* -------------------------------------Data creation section------------------------------------------ */

//DATA --------> idle
void del_dtb_SimpleFsm_idle(dtb_SimpleFsm_idle *body) {
    if (body->str0 != NULL)
        pfree(body->str0);
}

fsm_data* new_dtb_SimpleFsm_idle(char *str0) {
    dtb_SimpleFsm_idle *data = pmalloc(sizeof(dtb_SimpleFsm_idle));
    data->str0 = strcpy2(str0);

    return new_fsm_data(dt_SimpleFsm_idle, data, del_dtb_SimpleFsm_idle);
}

//DATA --------> one
void del_dtb_SimpleFsm_one(dtb_SimpleFsm_one *body) {
    if (body->str0 != NULL)
        pfree(body->str0);
    if (body->str1 != NULL)
        pfree(body->str1);
}

fsm_data* new_dtb_SimpleFsm_one(char *str0, char *str1) {
    dtb_SimpleFsm_one* data = pmalloc(sizeof(dtb_SimpleFsm_one));
    data->str0 = strcpy2(str0);
    data->str1 = strcpy2(str1);

    return new_fsm_data(dt_SimpleFsm_one, data, del_dtb_SimpleFsm_one);
}

//DATA --------> two
void del_dtb_SimpleFsm_two(dtb_SimpleFsm_two *body) {
    if (body->str0 != NULL)
        pfree(body->str0);
    if (body->str1 != NULL)
        pfree(body->str1);
    if (body->str2 != NULL)
        pfree(body->str2);
}

fsm_data* new_dtb_SimpleFsm_two(char *str0, char *str1, char *str2) {
    dtb_SimpleFsm_two* data = pmalloc(sizeof(dtb_SimpleFsm_two));
    data->str0 = strcpy2(str0);
    data->str1 = strcpy2(str1);
    data->str2 = strcpy2(str2);

    return new_fsm_data(dt_SimpleFsm_two, data, del_dtb_SimpleFsm_two);
}

/* --------------------------------------States logic section------------------------------------------ */

//STATE --------> idle / DATA ----------> idle

void ev_SimpleFsm_idle_idle_Message0(Actor *this, b_SimpleFsm_Message0 *b) {
    dtb_SimpleFsm_idle *data = AU_fsm_data(this);
    printf("Message0 in state = Idle, date = Idle\n");
    FSM_toState(this, new_dtb_SimpleFsm_one(data->str0, b->str1), st_SimpleFsm_one, 3000, 32);
}

void ev_SimpleFsm_idle_idle_StateTimeout(Actor *this, b_FSM_StateTimeout *b) {
    printf("StateTimeout in state = Idle, date = Idle. Fsm stopped\n");
    FSM_stop(this, 32);
}

//STATE --------> one / DATA ----------> one
void ev_SimpleFsm_idle_idle_Message1(Actor *this, b_SimpleFsm_Message1 *b) {
    dtb_SimpleFsm_one *data = AU_fsm_data(this);
    printf("Message1 in state = One, date = One\n");
    FSM_toState(this, new_dtb_SimpleFsm_two(data->str0, data->str1, b->str2), st_SimpleFsm_two, 3000, 32);
}

void ev_SimpleFsm_one_one_StateTimeout(Actor *this, b_FSM_StateTimeout *b) {
    printf("StateTimeout in state = One, date = One. Fsm stopped\n");
    FSM_stop(this, 32);
}

//STATE --------> two / DATA ----------> two
void ev_SimpleFsm_idle_idle_Message2(Actor *this, b_SimpleFsm_Message0 *b) {
    dtb_SimpleFsm_two *data = AU_fsm_data(this);
    printf("Message2 in state = Two, date = Two. str0 = %s, str1 = %s, str2 = %s. Fsm stopped\n", data->str0, data->str1, data->str2);
    FSM_stop(this, 32);
}

void ev_SimpleFsm_two_two_StateTimeout(Actor *this, b_FSM_StateTimeout *b) {
    printf("StateTimeout in state = Two, date = Two. Fsm stopped\n");
    FSM_stop(this, 32);
}

/* ----------------------------------------Initialize message-------------------------------------------- */

void del_m_SimpleFsm_Initialize(b_SimpleFsm_Initialize *b) {
    pfree(b->str0);
}

ActorMessage* new_m_SimpleFsm_Initialize(char *str0, uint16_t stackSize) {
    b_SimpleFsm_Initialize *body = pmalloc(sizeof(b_SimpleFsm_Initialize));
    body->str0 = strcpy2(str0);

    return new_ActorMessage(m_Initialize, body, stackSize, del_m_SimpleFsm_Initialize);
}

void mp_SimpleFsm_Initialize(Actor *this, b_SimpleFsm_Initialize* b) {
    printf("Fsm initialization processed\n");
    FSM_toState(this, new_dtb_SimpleFsm_idle(b->str0), st_SimpleFsm_idle, 3000, 32);
}

void SimpleFsm_beforeStop(Actor *this) {

}

/* -------------------------------------------Actor section---------------------------------------------- */

bool SimpleFsm_receive(Actor *this, uint16_t type, void *msg) {
    if (type != m_Initialize) {
        fsm_box *box = this->state;

        switch (box->state->code) {
            case st_SimpleFsm_idle:
                switch (box->data->code) {
                    case dt_SimpleFsm_idle:
                        switch (type) {
                            case m_SimpleFsm_Message0:
                                ev_SimpleFsm_idle_idle_Message0(this, msg);
                                break;
                            case m_FSM_StateTimeout:
                                ev_SimpleFsm_idle_idle_StateTimeout(this, msg);
                                break;
                            default:
                                break;
                        }
                        break;
                    default:
                        break;
                }
                break;
            case st_SimpleFsm_one:
                switch (box->data->code) {
                    case dt_SimpleFsm_one:
                        switch (type) {
                            case m_SimpleFsm_Message1:
                                ev_SimpleFsm_idle_idle_Message1(this, msg);
                                break;
                            case m_FSM_StateTimeout:
                                ev_SimpleFsm_one_one_StateTimeout(this, msg);
                                break;
                            default:
                                break;
                        }
                        break;
                    default:
                        break;
                }
                break;
            case st_SimpleFsm_two:
                switch (box->data->code) {
                    case dt_SimpleFsm_two:
                        switch (type) {
                            case m_SimpleFsm_Message2:
                                ev_SimpleFsm_idle_idle_Message2(this, msg);
                                break;
                            case m_FSM_StateTimeout:
                                ev_SimpleFsm_two_two_StateTimeout(this, msg);
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
        mp_SimpleFsm_Initialize(this, msg);
    }

    return true;
}

Actor* new_SimpleFsm() {
    fsm_box *box = new_fsm_box(NULL, NULL);
    Actor *actor = new_Actor(SimpleFsm_receive, SimpleFsm_beforeStop, box);

    return actor;
}