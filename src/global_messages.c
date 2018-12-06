#include "../include/global_messages.h"
#include "../oscl/include/malloc.h"
#include "../libs/miniakka/includes/standard_messages.h"

ActorMessage* new_m_MnvpGlobal_HeartBeat(uint16_t stackSize) {
    ActorMessage *msg = pmalloc(sizeof(ActorMessage));

    msg->type = m_MnvpGlobal_HeartBeat;
    msg->body = NULL;
    msg->stackSize = stackSize;
    msg->destructor = del_default;

    return msg;
}