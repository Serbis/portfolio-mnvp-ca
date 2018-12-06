#include "../../include/transactions/transaction_commands.h"
#include "../../libs/miniakka/includes/actor_utils.h"
#include "../../libs/miniakka/includes/standard_messages.h"

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

ActorMessage* new_m_Transaction_Exec(uint32_t tid, uint16_t stackSize) {
    b_Transaction_Exec *body = pmalloc(sizeof(b_Transaction_Exec));
    body->tid = tid;

    return new_ActorMessage(m_Transaction_Exec, body, stackSize, del_default);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

ActorMessage* new_r_Transaction_TransactionFinished(uint32_t tid, uint16_t stackSize) {
    b_Transaction_TransactionFinished *body = pmalloc(sizeof(b_Transaction_TransactionFinished));
    body->tid = tid;

    return new_ActorMessage(r_Transaction_TransactionFinished, body, stackSize, del_default);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void del_Transaction_ReceivePacket(b_Transaction_ReceivePacket *b) {
    free_Packet(b->packet);
}

ActorMessage* new_m_Transaction_ReceivePacket(Packet *packet, uint16_t stackSize) {
    b_Transaction_ReceivePacket *body = pmalloc(sizeof(b_Transaction_ReceivePacket));
    body->packet = copy_Packet(packet);

    return new_ActorMessage(m_Transaction_ReceivePacket, body, stackSize, del_Transaction_ReceivePacket);
}