#include "../include/send_packet_fsm_spec.h"
#include "../../../libs/miniakka/includes/testprobe.h"
#include "../../../include/a_send_packet_fsm.h"
#include "../../../include/a_rtable.h"
#include "../../../libs/cunit/include/asserts.h"
#include "../../../include/a_gateway_pool.h"
#include "../../../include/a_gateway.h"
#include "../../../libs/miniakka/includes/standard_messages.h"
#include "../../../libs/miniakka/includes/actor_utils.h"
#include "../../extractors/include/extractors.h"

Packet* b_Gateway_RouteByLabel_Send_extr1(b_GatewayPool_RouteByLabel *body) {
    Packet *packet = ((b_Gateway_Send*) body->message->body)->packet;

    return copy_Packet(packet);
}


//-- SendPacketFsm
//---- After receive Exec message send packet to expected gateway and return result 0
void SendPacketFsmSpec_t_0_0(ActorSystem *testSystem, ActorSystem *system) {
    printf("Test t_0_0 was started ... ");

    TestProbe *probe = new_TestProbe(testSystem, system,  "Probe");
    TestProbe *gwPool = new_TestProbe(testSystem, system,  "GwPool");
    TestProbe *rTable = new_TestProbe(testSystem, system,  "Rtable");

    ActorRef *target = system->actorOf(system, new_SendPacketFsm(), NULL);
    target->tell(target, new_m_SendPacketFsm_Initialize(rTable->ref, gwPool->ref, false, 32), NULL, system);

    Packet *packet = Packet_createHelloPacket(100, 99);
    packet->dest = 199;

    probe->send(probe, target, new_m_SendPacketFsm_Exec(packet, 32));
    b_Rtable_FindRoute *msg0 = rTable->expectMsgType(rTable, m_Rtable_FindRoute, b_Rtable_FindRoute_extr, 3000);
    assertTrue(msg0->dest == 199, "t_0_0-A");
    rTable->reply(rTable, new_r_Rtable_Route(888, 11, 5, 32));
    Packet *outPacket = gwPool->expectMsgType(gwPool, m_GatewayPool_RouteByLabel, b_Gateway_RouteByLabel_Send_extr1, 3000);
    assertTrue(compare_PacketHeader(packet, outPacket) == true, "t_0_0-B");

    b_SendPacketFsm_SendResult *msg1 = probe->expectMsgType(probe, r_SendPacketFsm_SendResult, b_SendPacketFsm_SendResult_extr, 3000);
    assertTrue(msg1->code == 0, "t_0_0-C");

    probe->destroy(probe);
    pfree(probe);
    gwPool->destroy(gwPool);
    pfree(gwPool);
    rTable->destroy(rTable);
    pfree(rTable);
    target->tell(target, new_m_Stop(32), NULL, system);
    AU_freeRef(target);
    pfree(msg0);
    pfree(msg1);
    free_Packet(packet);
    free_Packet(outPacket);

    printf("OK\n");
}

//---- After receive Exec message return result 1 if routing table response with route not found
void SendPacketFsmSpec_t_1_0(ActorSystem *testSystem, ActorSystem *system) {
    printf("Test t_1_0 was started ... ");

    TestProbe *probe = new_TestProbe(testSystem, system,  "Probe");
    TestProbe *gwPool = new_TestProbe(testSystem, system,  "GwPool");
    TestProbe *rTable = new_TestProbe(testSystem, system,  "Rtable");

    ActorRef *target = system->actorOf(system, new_SendPacketFsm(), NULL);
    target->tell(target, new_m_SendPacketFsm_Initialize(rTable->ref, gwPool->ref, false, 32), NULL, system);

    Packet *packet = Packet_createHelloPacket(100, 99);
    packet->dest = 199;

    probe->send(probe, target, new_m_SendPacketFsm_Exec(packet, 32));
    b_Rtable_FindRoute *msg0 = rTable->expectMsgType(rTable, m_Rtable_FindRoute, b_Rtable_FindRoute_extr, 3000);
    assertTrue(msg0->dest == 199, "t_0_0-A");
    rTable->reply(rTable, new_r_Rtable_Route(0, 0, 0, 32));

    b_SendPacketFsm_SendResult *msg1 = probe->expectMsgType(probe, r_SendPacketFsm_SendResult, b_SendPacketFsm_SendResult_extr, 3000);
    assertTrue(msg1->code == 1, "t_0_0-B");

    probe->destroy(probe);
    pfree(probe);
    gwPool->destroy(gwPool);
    pfree(gwPool);
    rTable->destroy(rTable);
    pfree(rTable);
    target->tell(target, new_m_Stop(32), NULL, system);
    AU_freeRef(target);
    pfree(msg0);
    pfree(msg1);
    free_Packet(packet);

    printf("OK\n");
}

//---- After receive Exec message return result 2 if routing table not respond with expected timeout
void SendPacketFsmSpec_t_2_0(ActorSystem *testSystem, ActorSystem *system) {
    printf("Test t_2_0 was started ... ");

    TestProbe *probe = new_TestProbe(testSystem, system,  "Probe");
    TestProbe *gwPool = new_TestProbe(testSystem, system,  "GwPool");
    TestProbe *rTable = new_TestProbe(testSystem, system,  "Rtable");

    ActorRef *target = system->actorOf(system, new_SendPacketFsm(), NULL);
    target->tell(target, new_m_SendPacketFsm_Initialize(rTable->ref, gwPool->ref, true, 32), NULL, system);

    Packet *packet = Packet_createHelloPacket(100, 99);
    packet->dest = 199;

    probe->send(probe, target, new_m_SendPacketFsm_Exec(packet, 32));
    b_Rtable_FindRoute *msg0 = rTable->expectMsgType(rTable, m_Rtable_FindRoute, b_Rtable_FindRoute_extr, 3000);
    assertTrue(msg0->dest == 199, "t_0_0-A");

    b_SendPacketFsm_SendResult *msg1 = probe->expectMsgType(probe, r_SendPacketFsm_SendResult, b_SendPacketFsm_SendResult_extr, 5000);
    assertTrue(msg1->code == 2, "t_0_0-B");

    probe->destroy(probe);
    pfree(probe);
    gwPool->destroy(gwPool);
    pfree(gwPool);
    rTable->destroy(rTable);
    pfree(rTable);
    target->tell(target, new_m_Stop(32), NULL, system);
    AU_freeRef(target);
    pfree(msg0);
    pfree(msg1);
    free_Packet(packet);

    printf("OK\n");
}



void run_send_packet_fsm_spec(ActorSystem *testSystem, ActorSystem *system) {
    printf("-----start send_packet_fsm_spec-----\n");
    SendPacketFsmSpec_t_0_0(testSystem, system); //LEAK FREE
    //SendPacketFsmSpec_t_1_0(testSystem, system); //LEAK FREE
    //SendPacketFsmSpec_t_2_0(testSystem, system); //LEAK FREE
    printf("------end send_packet_fsm_spec------\n\n");
}