#include "../include/preq_transaction_spec.h"
#include "../../../libs/miniakka/includes/testprobe.h"
#include "../../../include/transactions/a_preq_transaction.h"
#include "../../../include/transactions/transaction_commands.h"
#include "../../../include/a_gateway_pool.h"
#include "../../../libs/miniakka/includes/actor_utils.h"
#include "../../../include/a_gateway.h"
#include "../../../libs/cunit/include/asserts.h"
#include "../../../libs/miniakka/includes/standard_messages.h"

//Экстрактирует пакет из сообщения Gateway_SendData с вложенного в сообщение GatewayPool_RouteToAll
Packet* b_GatewayPool_RouteToAll_SendData_Packet_extr_1(b_GatewayPool_RouteToAll *body) {
    Packet *packet = ((b_Gateway_Send*) body->message->body)->packet;
    Packet *cp = copy_Packet(packet);
    //AU_freeMsg(body->message, true);

    return cp;
}

b_Transaction_TransactionFinished* b_Transaction_TransactionFinished_extr(b_Transaction_TransactionFinished *body) {
    b_Transaction_TransactionFinished *dup_body = pmalloc(sizeof(b_Transaction_TransactionFinished));
    dup_body->tid = body->tid;

    return dup_body;
}

//-- PreqTransaction must
//---- Return Found by min ack count received
void PreqTransactionSpec_t_0_0(ActorSystem *testSystem, ActorSystem *system) {
    printf("Test t_0_0 was started ... ");

    TestProbe *gwPool = new_TestProbe(testSystem, system,  "gwPool");
    TestProbe *recipient = new_TestProbe(testSystem, system,  "recipient");
    TestProbe *trPool = new_TestProbe(testSystem, system,  "trPool");

    ActorRef *target = system->actorOf(system, new_PreqTransaction(), NULL);
    target->tell(target, new_m_PreqTransaction_Initialize(999, gwPool->ref, recipient->ref, trPool->ref, 99, 100000, 3, 8, 32), NULL, system);
    target->tell(target, new_m_Transaction_Exec(9999, 32), NULL, system);

    Packet *packet = gwPool->expectMsgType(gwPool, m_GatewayPool_RouteToAll, b_GatewayPool_RouteToAll_SendData_Packet_extr_1, 10000);
    assertTrue(packet->msgId == 9999, "t_0_0-B");
    assertTrue(packet->type == PACKET_PREQ, "t_0_0-C");
    assertTrue(packet->source == 99, "t_0_0-D");
    assertTrue(packet->dest == 999, "t_0_0-E");
    assertTrue(packet->ttl == 8, "t_0_0-F");

    Packet *ackPacket1 = Packet_ackPreqPacket(packet, 3);
    Packet *ackPacket2 = Packet_ackPreqPacket(packet, 4);
    Packet *ackPacket3 = Packet_ackPreqPacket(packet, 5);

    target->tell(target, new_m_Transaction_ReceivePacket(ackPacket1, 32), NULL, system);
    target->tell(target, new_m_Transaction_ReceivePacket(ackPacket2, 32), NULL, system);
    target->tell(target, new_m_Transaction_ReceivePacket(ackPacket3, 32), NULL, system);


    //printf("a\n");


    recipient->expectMsgType(recipient, r_PreqTransaction_Found, empty_extr, 10000);
    //printf("b\n");
    b_Transaction_TransactionFinished *msg1 = trPool->expectMsgType(trPool, r_Transaction_TransactionFinished, b_Transaction_TransactionFinished_extr, 10000);
   // printf("c\n");
    assertTrue(msg1->tid == 9999, "t_0_0-G");

    gwPool->destroy(gwPool);
    pfree(gwPool);
    recipient->destroy(recipient);
    pfree(recipient);
    trPool->destroy(trPool);
    pfree(trPool);
    target->tell(target, new_m_Stop(32), NULL, system);
    AU_freeRef(target);
    free_Packet(packet);
    free_Packet(ackPacket1);
    free_Packet(ackPacket2);
    free_Packet(ackPacket3);
    pfree(msg1);

    printf("OK\n");
}

//---- Return Found by transaction timeout if acks was received
void PreqTransactionSpec_t_1_0(ActorSystem *testSystem, ActorSystem *system) {
    printf("Test t_1_0 was started ... ");

    TestProbe *gwPool = new_TestProbe(testSystem, system,  "gwPool");
    TestProbe *recipient = new_TestProbe(testSystem, system,  "recipient");
    TestProbe *trPool = new_TestProbe(testSystem, system,  "trPool");

    ActorRef *target = system->actorOf(system, new_PreqTransaction(), NULL);
    target->tell(target, new_m_PreqTransaction_Initialize(999, gwPool->ref, recipient->ref, trPool->ref, 99, 1000, 3, 8, 32), NULL, system);
    target->tell(target, new_m_Transaction_Exec(9999, 32), NULL, system);

    Packet *packet = gwPool->expectMsgType(gwPool, m_GatewayPool_RouteToAll, b_GatewayPool_RouteToAll_SendData_Packet_extr_1, 5000);
    assertTrue(packet->msgId == 9999, "t_1_0-B");
    assertTrue(packet->type == PACKET_PREQ, "t_1_0-C");
    assertTrue(packet->source == 99, "t_1_0-D");
    assertTrue(packet->dest == 999, "t_1_0-E");
    assertTrue(packet->ttl == 8, "t_1_0-F");

    Packet *ackPacket1 = Packet_ackPreqPacket(packet, 3);

    target->tell(target, new_m_Transaction_ReceivePacket(ackPacket1, 32), NULL, system);

    recipient->expectMsgType(recipient, r_PreqTransaction_Found, empty_extr, 120000);
    b_Transaction_TransactionFinished *msg1 = trPool->expectMsgType(trPool, r_Transaction_TransactionFinished, b_Transaction_TransactionFinished_extr, 5000);
    assertTrue(msg1->tid == 9999, "t_1_0-G");

    gwPool->destroy(gwPool);
    pfree(gwPool);
    recipient->destroy(recipient);
    pfree(recipient);
    trPool->destroy(trPool);
    pfree(trPool);
    target->tell(target, new_m_Stop(32), NULL, system);
    AU_freeRef(target);
    free_Packet(ackPacket1);
    free_Packet(packet);
    pfree(msg1);

    printf("OK\n");
}

//---- Return NotFound if transaction timeout was achieved and no ack was received
void PreqTransactionSpec_t_2_0(ActorSystem *testSystem, ActorSystem *system) {
    printf("Test t_2_0 was started ... ");

    TestProbe *gwPool = new_TestProbe(testSystem, system,  "gwPool");
    TestProbe *recipient = new_TestProbe(testSystem, system,  "recipient");
    TestProbe *trPool = new_TestProbe(testSystem, system,  "trPool");

    ActorRef *target = system->actorOf(system, new_PreqTransaction(), NULL);
    target->tell(target, new_m_PreqTransaction_Initialize(999, gwPool->ref, recipient->ref, trPool->ref, 99, 1000, 3, 8, 32), NULL, system);
    target->tell(target, new_m_Transaction_Exec(9999, 32), NULL, system);

    Packet *packet = gwPool->expectMsgType(gwPool, m_GatewayPool_RouteToAll, b_GatewayPool_RouteToAll_SendData_Packet_extr_1, 3000);

    recipient->expectMsgType(recipient, r_PreqTransaction_NotFound, empty_extr, 3000);
    b_Transaction_TransactionFinished *msg1 = trPool->expectMsgType(trPool, r_Transaction_TransactionFinished, b_Transaction_TransactionFinished_extr, 3000);
    assertTrue(msg1->tid == 9999, "t_2_0-B");

    gwPool->destroy(gwPool);
    pfree(gwPool);
    recipient->destroy(recipient);
    pfree(recipient);
    trPool->destroy(trPool);
    pfree(trPool);
    target->tell(target, new_m_Stop(32), NULL, system);
    AU_freeRef(target);
    free_Packet(packet);
    pfree(msg1);

    printf("OK\n");
}

//---- Return NotFound if transaction timeout was achieved and no ack was received
void PreqTransactionSpec_t_3_0(ActorSystem *testSystem, ActorSystem *system) {
    printf("Test t_3_0 was started ... ");

    TestProbe *gwPool = new_TestProbe(testSystem, system,  "gwPool");
    TestProbe *recipient = new_TestProbe(testSystem, system,  "recipient");
    TestProbe *trPool = new_TestProbe(testSystem, system,  "trPool");

    ActorRef *target = system->actorOf(system, new_PreqTransaction(), NULL);
    target->tell(target, new_m_PreqTransaction_Initialize(999, gwPool->ref, recipient->ref, trPool->ref, 99, 1000, 1, 8, 32), NULL, system);
    target->tell(target, new_m_Transaction_Exec(9999, 32), NULL, system);

    Packet *packet = gwPool->expectMsgType(gwPool, m_GatewayPool_RouteToAll, b_GatewayPool_RouteToAll_SendData_Packet_extr_1, 10000);
    assertTrue(packet->msgId == 9999, "t_3_0-B");
    assertTrue(packet->type == PACKET_PREQ, "t_3_0-C");
    assertTrue(packet->source == 99, "t_3_0-D");
    assertTrue(packet->dest == 999, "t_30-E");
    assertTrue(packet->ttl == 8, "t_3_0-F");

    Packet *helloPacket1 = Packet_createHelloPacket(0, 0);
    Packet *helloPacket2 = Packet_createHelloPacket(0, 0);
    Packet *ackPacket1 = Packet_ackPreqPacket(packet, 3);

    target->tell(target, new_m_Transaction_ReceivePacket(helloPacket1, 32), NULL, system);
    target->tell(target, new_m_Transaction_ReceivePacket(helloPacket2, 32), NULL, system);
    target->tell(target, new_m_Transaction_ReceivePacket(ackPacket1, 32), NULL, system);

    recipient->expectMsgType(recipient, r_PreqTransaction_Found, empty_extr, 10000);
    b_Transaction_TransactionFinished *msg1 = trPool->expectMsgType(trPool, r_Transaction_TransactionFinished, b_Transaction_TransactionFinished_extr, 10000);
    assertTrue(msg1->tid == 9999, "t_3_0-G");

    gwPool->destroy(gwPool);
    pfree(gwPool);
    recipient->destroy(recipient);
    pfree(recipient);
    trPool->destroy(trPool);
    pfree(trPool);
    target->tell(target, new_m_Stop(32), NULL, system);
    AU_freeRef(target);
    free_Packet(packet);
    free_Packet(ackPacket1);
    pfree(msg1);

    printf("OK\n");
}

void run_preq_transaction_spec(ActorSystem *testSystem, ActorSystem *system) {
    printf("------start preq_transaction_spec------\n");
    PreqTransactionSpec_t_0_0(testSystem, system); //NO LEAKS
    PreqTransactionSpec_t_1_0(testSystem, system);
    PreqTransactionSpec_t_2_0(testSystem, system);
    PreqTransactionSpec_t_3_0(testSystem, system);
    printf("-------end preq_transaction_spec-------\n\n");
}