#include "../includes/standard_messages.h"
#include "../../../oscl/include/malloc.h"

ActorMessage* new_m_Stop(uint16_t stackSize) {
    return new_ActorMessage(m_Stop, NULL, stackSize, del_default);
}

void del_default(void *b) { ; }

void* new_DefaultBody() {
    return NULL;
}