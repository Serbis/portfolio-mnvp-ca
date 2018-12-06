#include "../includes/simple_actor.h"
#include "../../../oscl/include/malloc.h"
#include "../includes/actor_ref.h"
#include "../includes/standard_messages.h"
#include "../../../oscl/include/time.h"
#include "../includes/actor_system.h"
#include "../includes/actor_utils.h"
#include <stdio.h>

#define DoWorkMsgType 6548

typedef struct s_SimpleActor {
    char *str;
    int8_t mode;
    int8_t r;
} s_SimpleActor;

ActorMessage* new_DoWorkMsg(uint16_t workNumber) {
    ActorMessage *msg = pmalloc(sizeof(ActorMessage));
    uint16_t *body = pmalloc(2);
    *body = workNumber;

    msg->type = DoWorkMsgType;
    msg->body = body;
    msg->stackSize = 32;
    msg->destructor = del_default;

    return msg;
}

void mp_SimpleActor_DoWorkMsg(Actor* this, void *body) {
    printf("I am a simple actor\n");
    ActorSystem *system = AU_system(this);
    ActorRef *self = this->context->self;
    char *key = system->scheduler->scheduleOnce(system->scheduler, "A", 1000, self, new_DoWorkMsg(0), self);
}

void SimpleActor_beforeStop(Actor *this) {
    s_SimpleActor *state = (s_SimpleActor *) this->state;
    pfree(state->str);
}

bool SimpleActor_receive(Actor *this, uint16_t type, void *msg) {
    s_SimpleActor *state = (s_SimpleActor *) this->state;
    switch (type) {
        case DoWorkMsgType:
            mp_SimpleActor_DoWorkMsg(this, msg);
            //while (true) {
            //    printf("I am a simple actor %d in sleep - %d\n", state->r, *((int*) msg));
                //DelayMillis(1000);
            //}
            //usleep(300000);

            break;

        default:break;
    }

    true;
}

Actor* new_SimpleActor(int8_t r) {
    ActorRef *dup_gwPool = pmalloc(sizeof(ActorRef));
    //memcpy(dup_gwPool, gwPool, sizeof(ActorRef));

    s_SimpleActor *state = pmalloc(sizeof(s_SimpleActor));
    char *str = pmalloc(4);
    *str = (char) "abc";
    state->str = str;
    state->r = r;



    Actor *actor = new_Actor(SimpleActor_receive, SimpleActor_beforeStop, state);
    return actor;
}