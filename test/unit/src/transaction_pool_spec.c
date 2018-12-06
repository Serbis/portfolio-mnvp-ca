#include <string.h>
#include "../include/transaction_pool_spec.h"
#include "../../../libs/miniakka/includes/testprobe.h"
#include "../../../include/a_transaction_pool.h"
#include "../../../include/transactions/transaction_commands.h"
#include "../../../libs/miniakka/includes/standard_messages.h"
#include "../../../libs/miniakka/includes/actor_utils.h"
#include "../../../libs/cunit/include/asserts.h"

b_Transaction_Exec* b_Transaction_Exec_extr(b_Transaction_Exec *body) {
    b_Transaction_Exec *dup_body = pmalloc(sizeof(b_Transaction_Exec));
    dup_body->tid = body->tid;

    return dup_body;
}

b_TransactionPool_Transaction* b_TransactionPool_Transaction_extr(b_TransactionPool_Transaction *body) {
    b_TransactionPool_Transaction *dup_body = pmalloc(sizeof(b_TransactionPool_Transaction));
    dup_body->ref = AU_copyRef(body->ref);

    return dup_body;
}

b_Transaction_ReceivePacket* b_Transaction_ReceivePacket_extr(b_Transaction_ReceivePacket *body) {
    b_Transaction_ReceivePacket *dup_body = pmalloc(sizeof(b_Transaction_ReceivePacket));
    dup_body->packet = copy_Packet(body->packet);

    return dup_body;
}



//-- TransactionPool must
//---- Add and execute the transaction
void TransactionPoolSpec_t_0_0(ActorSystem *testSystem, ActorSystem *system) {
    printf("Test t_0_0 was started ... ");

    TestProbe *probe = new_TestProbe(testSystem, system,  "Probe");
    TestProbe *transaction = new_TestProbe(testSystem, system,  "Transaction");

    ActorRef *target = system->actorOf(system, new_TransactionPool(), NULL);

    probe->send(probe, target, new_m_TransactionPool_PoolAndExec(transaction->ref, 32));
    b_Transaction_Exec *msg0 = transaction->expectMsgType(transaction, m_Transaction_Exec, b_Transaction_Exec_extr, 3000);

    probe->destroy(probe);
    pfree(probe);
    transaction->destroy(transaction);
    pfree(transaction);
    target->tell(target, new_m_Stop(32), NULL, system);
    AU_freeRef(target);
    pfree(msg0);

    printf("OK\n");
}

//---- Return transaction reference by Get message
void TransactionPoolSpec_t_1_0(ActorSystem *testSystem, ActorSystem *system) {
    printf("Test t_1_0 was started ... ");

    TestProbe *probe = new_TestProbe(testSystem, system,  "Probe");
    TestProbe *transaction = new_TestProbe(testSystem, system,  "Transaction");

    ActorRef *target = system->actorOf(system, new_TransactionPool(), NULL);

    probe->send(probe, target, new_m_TransactionPool_PoolAndExec(transaction->ref, 32));
    b_Transaction_Exec *msg0 = transaction->expectMsgType(transaction, m_Transaction_Exec, b_Transaction_Exec_extr, 3000);
    probe->send(probe, target, new_m_TransactionPool_Get(msg0->tid, 32));
    b_TransactionPool_Transaction *msg1 = probe->expectMsgType(probe, r_TransactionPool_Transaction, b_TransactionPool_Transaction_extr, 3000);
    assertTrue(strcmp(msg1->ref->actorName, transaction->ref->actorName) == 0, "t_1_0-A");

    probe->destroy(probe);
    pfree(probe);
    transaction->destroy(transaction);
    pfree(transaction);
    target->tell(target, new_m_Stop(32), NULL, system);
    AU_freeRef(target);
    pfree(msg0);
    AU_freeRef(msg1->ref);
    pfree(msg1);

    printf("OK\n");
}

//---- Route a message to the transaction
void TransactionPoolSpec_t_2_0(ActorSystem *testSystem, ActorSystem *system) {
    printf("Test t_2_0 was started ... ");

    TestProbe *probe = new_TestProbe(testSystem, system, "Probe");
    TestProbe *transaction = new_TestProbe(testSystem, system, "Transaction");

    ActorRef *target = system->actorOf(system, new_TransactionPool(), NULL);

    probe->send(probe, target, new_m_TransactionPool_PoolAndExec(transaction->ref, 32));
    b_Transaction_Exec *msg0 = transaction->expectMsgType(transaction, m_Transaction_Exec, b_Transaction_Exec_extr, 300000);
    Packet *packet = Packet_createHelloPacket(msg0->tid, 0);
    probe->send(probe, target, new_m_TransactionPool_Route(packet, 32));
    b_Transaction_ReceivePacket *msg1 = transaction->expectMsgType(transaction, m_Transaction_ReceivePacket, b_Transaction_ReceivePacket_extr, 300000);
    Packet *recPacket = msg1->packet;
    assertTrue(packet->msgId == recPacket->msgId, "t_2_0-A");
    assertTrue(packet->ttl == recPacket->ttl, "t_2_0-B");
    assertTrue(packet->startTtl == recPacket->startTtl, "t_2_0-C");
    assertTrue(packet->type == recPacket->type, "t_2_0-D");
    assertTrue(packet->source == recPacket->source, "t_2_0-E");
    assertTrue(packet->dest == recPacket->dest, "t_2_0-F");

    probe->destroy(probe);
    pfree(probe);
    transaction->destroy(transaction);
    pfree(transaction);
    target->tell(target, new_m_Stop(32), NULL, system);
    AU_freeRef(target);
    pfree(msg0);
    pfree(msg1);
    free_Packet(packet);
    free_Packet(recPacket);

    printf("OK\n");
}

//---- Remove transaction from the pool by TransactionFinished message
void TransactionPoolSpec_t_3_0(ActorSystem *testSystem, ActorSystem *system) {
    printf("Test t_3_0 was started ... ");

    TestProbe *probe = new_TestProbe(testSystem, system, "Probe");
    TestProbe *transaction = new_TestProbe(testSystem, system, "Transaction");

    ActorRef *target = system->actorOf(system, new_TransactionPool(), NULL);

    probe->send(probe, target, new_m_TransactionPool_PoolAndExec(transaction->ref, 32));
    b_Transaction_Exec *msg0 = transaction->expectMsgType(transaction, m_Transaction_Exec, b_Transaction_Exec_extr, 3000);
    transaction->send(transaction, target, new_r_Transaction_TransactionFinished(msg0->tid, 32));
    Packet *packet = Packet_createHelloPacket(msg0->tid, 0);
    probe->send(probe, target, new_m_TransactionPool_Route(packet, 32));
    probe->send(probe, target, new_m_TransactionPool_Get(msg0->tid, 32));
    b_TransactionPool_Transaction *msg1 = probe->expectMsgType(probe, r_TransactionPool_Transaction, b_TransactionPool_Transaction_extr, 3000);
    assertTrue(msg1->ref == NULL, "t_3_0-A");

    probe->destroy(probe);
    pfree(probe);
    transaction->destroy(transaction);
    pfree(transaction);
    target->tell(target, new_m_Stop(32), NULL, system);
    AU_freeRef(target);
    pfree(msg0);
    pfree(msg1);
    free_Packet(packet);

    printf("OK\n");
}



void run_transaction_pool_spec(ActorSystem *testSystem, ActorSystem *system) {
    printf("-----start transaction_pool_spec-----\n");
    TransactionPoolSpec_t_0_0(testSystem, system); // LEAK FREE
    TransactionPoolSpec_t_1_0(testSystem, system); // LEAK FREE
    TransactionPoolSpec_t_2_0(testSystem, system); // LEAK FREE
    TransactionPoolSpec_t_3_0(testSystem, system); // LEAK FREE
    printf("------end transaction_pool_spec------\n\n");
}