#include "../include/a_heart.h"
#include "../oscl/include/malloc.h"
#include "../libs/miniakka/includes/standard_messages.h"

ActorMessage* new_r_Heart_HeartBeat(uint16_t stackSize) {
    return new_ActorMessage(r_Heart_HeartBeat, NULL, stackSize, del_default);
}