#include "../include/receiver_spec.h"
#include "../../../libs/miniakka/includes/testprobe.h"
#include "../../../include/a_receiver.h"
#include "../../../include/a_rtable.h"
#include "../../../libs/cunit/include/asserts.h"
#include "../../../include/a_gateway.h"
#include "../../../libs/miniakka/includes/actor_utils.h"
#include "../../../libs/miniakka/includes/standard_messages.h"
#include "../../../include/a_gateway_pool.h"
#include "../../../include/transactions/transaction_commands.h"
#include "../../../include/a_transaction_pool.h"

b_Rtable_UpdateRoute* b_Rtable_UpdateRoute_extr(b_Rtable_UpdateRoute *body) {
    b_Rtable_UpdateRoute *dup_body = pmalloc(sizeof(b_Rtable_UpdateRoute));

    dup_body->dest = body->dest;
    dup_body->gateway = body->gateway;
    dup_body->distance = body->distance;
    dup_body->gwl = body->gwl;

    return dup_body;
}

b_GatewayPool_RouteByLabel* b_Gateway_RouteByLabel_extr(b_GatewayPool_RouteByLabel *body) {
    b_GatewayPool_RouteByLabel *dup_body = pmalloc(sizeof(b_GatewayPool_RouteByLabel));

    dup_body->gwLabel = body->gwLabel;
    dup_body->message = AU_copyMsg(body->message, NULL);

    return dup_body;
}

Packet* b_Gateway_RouteByLabel_Send_extr(b_GatewayPool_RouteByLabel *body) {
    Packet *packet = ((b_Gateway_Send*) body->message->body)->packet;

    return copy_Packet(packet);
}

//Экстрактирует пакет из сообщения Gateway_SendData с вложенного в сообщение GatewayPool_RouteToAll
Packet* b_GatewayPool_RouteToAll_SendData_Packet_extr_2(b_GatewayPool_RouteToAll *body) {
    Packet *packet = ((b_Gateway_Send*) body->message->body)->packet;
    Packet *cp = copy_Packet(packet);
    //AU_freeMsg(body->message, true);

    return cp;
}

Packet* b_TransactionPool_Route_ReceivePacket_extr_1(b_TransactionPool_Route *body) {
    return copy_Packet(body->packet);
}

//-- Receiver must
//---- After receive incoming hello packet should create new route entry and set gateway network address
void ReceiverSpec_t_0_0(ActorSystem *testSystem, ActorSystem *system) {
    printf("Test t_0_0 was started ... ");

    TestProbe *probe = new_TestProbe(testSystem, system,  "Probe");
    TestProbe *gwPool = new_TestProbe(testSystem, system,  "GwPool");
    TestProbe *rTable = new_TestProbe(testSystem, system,  "Rtable");
    TestProbe *trPool = new_TestProbe(testSystem, system,  "trPool");

    ActorRef *target = system->actorOf(system, new_Receiver(gwPool->ref, rTable->ref, trPool->ref, 1), NULL);

    Packet *packet = Packet_createHelloPacket(111, 999);

    probe->send(probe, target, new_m_Receiver_Receive(packet, 100, 0, 32));
    b_GatewayPool_RouteByLabel *msg1 = gwPool->expectMsgType(gwPool, m_GatewayPool_RouteByLabel, b_Gateway_RouteByLabel_extr, 3000);
    b_Rtable_UpdateRoute *msg0 = rTable->expectMsgType(rTable, m_Rtable_UpdateRoute, b_Rtable_UpdateRoute_extr, 3000);

    assertTrue(msg1->message->type == m_Gateway_SetNetworkAddress, "t_0_0-D");
    assertTrue(msg0->dest == 999, "t_0_0-A");
    assertTrue(msg0->gateway == 999, "t_0_0-B");
    assertTrue(msg0->distance == 1, "t_0_0-C");
    assertTrue(msg0->gwl == 100, "t_0_0-D0");


    probe->destroy(probe);
    pfree(probe);
    gwPool->destroy(gwPool);
    pfree(gwPool);
    trPool->destroy(trPool);
    pfree(trPool);
    rTable->destroy(rTable);
    pfree(rTable);
    target->tell(target, new_m_Stop(32), NULL, system);
    AU_freeRef(target);
    free_Packet(packet);
    pfree(msg0);
    AU_freeMsg(msg1->message, true);
    pfree(msg1);

    printf("OK\n");
}

//---- After receive incoming transit prep packet should resend it to all gateways with decremented ttl
void ReceiverSpec_t_1_0(ActorSystem *testSystem, ActorSystem *system) {
    printf("Test t_1_0 was started ... ");

    TestProbe *probe = new_TestProbe(testSystem, system,  "Probe");
    TestProbe *gwPool = new_TestProbe(testSystem, system,  "GwPool");
    TestProbe *rTable = new_TestProbe(testSystem, system,  "Rtable");
    TestProbe *trPool = new_TestProbe(testSystem, system,  "trPool");

    ActorRef *target = system->actorOf(system, new_Receiver(gwPool->ref, rTable->ref, trPool->ref, 99), NULL);

    Packet *inPacket = Packet_createPreqPacket(999, 100, 200, 8);

    probe->send(probe, target, new_m_Receiver_Receive(inPacket, 128, 300, 32));

    b_Rtable_UpdateRoute *msg0 = rTable->expectMsgType(rTable, m_Rtable_UpdateRoute, b_Rtable_UpdateRoute_extr, 3000);

    assertTrue(msg0->dest == 100, "t_1_0-A");
    assertTrue(msg0->gateway == 300, "t_1_0-B");
    assertTrue(msg0->distance == 8, "t_1_0-C");
    assertTrue(msg0->gwl == 128, "t_1-0-D0");

    Packet *packet = gwPool->expectMsgType(gwPool, m_GatewayPool_RouteToAll, b_GatewayPool_RouteToAll_SendData_Packet_extr_2, 10000);
    assertTrue(packet->msgId == 999, "t_1_0-D");
    assertTrue(packet->type == PACKET_PREQ, "t_1_0-E");
    assertTrue(packet->source == 100, "t_1_0-F");
    assertTrue(packet->dest == 200, "t_1-G");
    assertTrue(packet->ttl == 7, "t_1_0-H");

    probe->destroy(probe);
    pfree(probe);
    gwPool->destroy(gwPool);
    pfree(gwPool);
    trPool->destroy(trPool);
    pfree(trPool);
    rTable->destroy(rTable);
    pfree(rTable);
    target->tell(target, new_m_Stop(32), NULL, system);
    AU_freeRef(target);
    pfree(msg0);
    free_Packet(packet);
    free_Packet(inPacket);

    printf("OK\n");

}

//---- After receive incoming target preq packet should send to packet source ack modification of the packet
void ReceiverSpec_t_2_0(ActorSystem *testSystem, ActorSystem *system) {
    printf("Test t_2_0 was started ... ");

    TestProbe *probe = new_TestProbe(testSystem, system,  "Probe");
    TestProbe *gwPool = new_TestProbe(testSystem, system,  "GwPool");
    TestProbe *rTable = new_TestProbe(testSystem, system,  "Rtable");
    TestProbe *trPool = new_TestProbe(testSystem, system,  "trPool");

    ActorRef *target = system->actorOf(system, new_Receiver(gwPool->ref, rTable->ref, trPool->ref, 200), NULL);

    Packet *inPacket = Packet_createPreqPacket(9999, 100, 200, 8);

    probe->send(probe, target, new_m_Receiver_Receive(inPacket, 128, 300, 32));

    b_Rtable_UpdateRoute *msg0 = rTable->expectMsgType(rTable, m_Rtable_UpdateRoute, b_Rtable_UpdateRoute_extr, 3000);

    assertTrue(msg0->dest == 100, "t_2_0-A");
    assertTrue(msg0->gateway == 300, "t_2_0-B");
    assertTrue(msg0->distance == 8, "t_2_0-C");
    assertTrue(msg0->gwl == 128, "t_1-0-D0");

    Packet *outPacket = gwPool->expectMsgType(gwPool, m_GatewayPool_RouteByLabel, b_Gateway_RouteByLabel_Send_extr, 10000);
    Packet *ackPacket = Packet_ackPreqPacket(inPacket, 8);

    assertTrue(outPacket->msgId == ackPacket->msgId, "t_2_0-D");
    assertTrue(outPacket->type == ackPacket->type, "t_2_0-E");
    assertTrue(outPacket->source == ackPacket->source, "t_2_0-F");
    assertTrue(outPacket->dest == ackPacket->dest, "t_2-G");
    assertTrue(outPacket->ttl == ackPacket->ttl, "t_2_0-H");

    probe->destroy(probe);
    pfree(probe);
    gwPool->destroy(gwPool);
    pfree(gwPool);
    trPool->destroy(trPool);
    pfree(trPool);
    rTable->destroy(rTable);
    pfree(rTable);
    target->tell(target, new_m_Stop(32), NULL, system);
    AU_freeRef(target);
    pfree(msg0);
    free_Packet(inPacket);
    free_Packet(outPacket);
    free_Packet(ackPacket);

    printf("OK\n");

}

//---- After receive incoming target preq_ack packet should send it with Route message to the transaction pool
void ReceiverSpec_t_3_0(ActorSystem *testSystem, ActorSystem *system) {
    printf("Test t_3_0 was started ... ");

    TestProbe *probe = new_TestProbe(testSystem, system,  "Probe");
    TestProbe *gwPool = new_TestProbe(testSystem, system,  "GwPool");
    TestProbe *rTable = new_TestProbe(testSystem, system,  "Rtable");
    TestProbe *trPool = new_TestProbe(testSystem, system,  "trPool");

    ActorRef *target = system->actorOf(system, new_Receiver(gwPool->ref, rTable->ref, trPool->ref, 100), NULL);

    Packet *p = Packet_createPreqPacket(9999, 100, 200, 8);
    Packet *inPacket = Packet_ackPreqPacket(p, 8);

    probe->send(probe, target, new_m_Receiver_Receive(inPacket, 128, 300, 32));

    b_Rtable_UpdateRoute *msg0 = rTable->expectMsgType(rTable, m_Rtable_UpdateRoute, b_Rtable_UpdateRoute_extr, 3000);

    assertTrue(msg0->dest == 200, "t_3_0-A");
    assertTrue(msg0->gateway == 300, "t_3_0-B");
    assertTrue(msg0->distance == 8, "t_3_0-C");
    assertTrue(msg0->gwl == 128, "t_1-0-D0");

    Packet *packet = trPool->expectMsgType(trPool, m_TransactionPool_Route, b_TransactionPool_Route_ReceivePacket_extr_1, 10000);

    assertTrue(inPacket->msgId == packet->msgId, "t_3_0-D");
    assertTrue(inPacket->type == packet->type, "t_3_0-E");
    assertTrue(inPacket->source == packet->source, "t_3_0-F");
    assertTrue(inPacket->dest == packet->dest, "t_3-G");
    assertTrue(inPacket->ttl == packet->ttl, "t_3_0-H");

    probe->destroy(probe);
    pfree(probe);
    gwPool->destroy(gwPool);
    pfree(gwPool);
    trPool->destroy(trPool);
    pfree(trPool);
    rTable->destroy(rTable);
    pfree(rTable);
    target->tell(target, new_m_Stop(32), NULL, system);
    AU_freeRef(target);
    pfree(msg0);
    free_Packet(inPacket);
    free_Packet(packet);
    free_Packet(p);

    printf("OK\n");

}

void run_receiver_spec(ActorSystem *testSystem, ActorSystem *system) {
    printf("-----start receiver_spec-----\n");
    ReceiverSpec_t_0_0(testSystem, system); //LEAK FREE
    ReceiverSpec_t_1_0(testSystem, system); //LEAK FREE
    ReceiverSpec_t_2_0(testSystem, system); //LEAK FREE
    ReceiverSpec_t_3_0(testSystem, system); //LEAK FREE
    printf("------end receiver_spec------\n\n");
}