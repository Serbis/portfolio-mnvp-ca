#include <stdlib.h>
#include <string.h>
#include "../include/gateway_spec.h"
#include "../../../include/a_gateway.h"
#include "../../../libs/miniakka/includes/testprobe.h"
#include "../../../libs/miniakka/includes/standard_messages.h"
#include "../../../oscl/include/connection.h"
#include "../../../include/a_receiver.h"
#include "../../../include/a_rtable.h"
#include "../../../include/global_messages.h"
#include "../../../include/a_heart.h"
#include "../../../libs/cunit/include/asserts.h"
#include "../../../libs/miniakka/includes/actor_utils.h"
#include "../../extractors/include/extractors.h"

void ffail(char *reason) {
    printf("FAILED -> %s", reason);
    exit(1);
}

b_Receiver_Receive* b_Receiver_Receive_extr(b_Receiver_Receive *body) {
    b_Receiver_Receive *dup_body = pmalloc(sizeof(b_Receiver_Receive));
    Packet *dup_packet = copy_Packet(body->packet);
    dup_body->packet = dup_packet;
    dup_body->gwNetAdr = body->gwNetAdr;
    dup_body->gwLabel = body->gwLabel;

    return dup_body;
}



b_Rtable_RemoveAllRoutesByGateway* b_Rtable_RemoveByGateway_extr(b_Rtable_RemoveAllRoutesByGateway *body) {
    b_Rtable_RemoveAllRoutesByGateway *dup_body = pmalloc(sizeof(b_Rtable_RemoveAllRoutesByGateway));
    dup_body->gwAdr = body->gwAdr;

    return dup_body;
}

b_Gateway_NetworkAddress* b_Gateway_NetworkAddress_extr(b_Gateway_NetworkAddress *body) {
    b_Gateway_NetworkAddress *dup_body = pmalloc(sizeof(b_Gateway_NetworkAddress));
    dup_body->netAdr = body->netAdr;

    return dup_body;
}

//-- Gateway must
//---- Construct package from a small parts of a data and send it to the receiver
//     (This test must be run twice, for to eliminate the possibility of inconsistent states actor after assembly of
//     the first packet)
void GatewaySpec_t_0_0(ActorSystem *testSystem, ActorSystem *system) {
    printf("Test t_0_0 was started ... ");

    TestProbe *connection = new_TestProbe(testSystem, system,  "Connection");
    TestProbe *receiver = new_TestProbe(testSystem, system,  "Router");
    TestProbe *rTable = new_TestProbe(testSystem, system,  "Rtable");
    List *rt = new_List();
    rt->prepend(rt, receiver->ref);
    RoundRobinRoutingLogic *logic = new_RoundRobinRoutingLogic();
    Router *receivers = new_Router(ROUND_ROBIN_LOGIC, logic, rt);

    ActorRef *target = system->actorOf(system, new_Gateway(connection->ref, rTable->ref, NULL, receivers, "tConnection", true), NULL);

    Packet *tPacket = Packet_createHelloPacket(3, 1);
    uint16_t *size = pmalloc(2);
    uint8_t *bin = Packet_toBinary(tPacket, size);


    connection->send(connection, target, new_r_Connection_Data(bin, 5, 0));
    connection->send(connection, target, new_r_Connection_Data(bin + 5, 5, 0));
    connection->send(connection, target, new_r_Connection_Data(bin + 10, 5, 0));
    connection->send(connection, target, new_r_Connection_Data(bin + 15, 5, 0));
    connection->send(connection, target, new_r_Connection_Data(bin + 20, 5, 0));
    connection->send(connection, target, new_r_Connection_Data(bin + 25, 4, 0));

    b_Receiver_Receive *msg = receiver->expectMsgType(receiver, m_Receiver_Receive,  b_Receiver_Receive_extr, 300000);
    if (msg->packet->msgId != 3) ffail("Bad packet received 1\n");



    connection->send(connection, target, new_r_Connection_Data(bin, 3, 0));
    connection->send(connection, target, new_r_Connection_Data(bin + 3, 3, 0));
    connection->send(connection, target, new_r_Connection_Data(bin + 6, 3, 0));
    connection->send(connection, target, new_r_Connection_Data(bin + 9, 3, 0));
    connection->send(connection, target, new_r_Connection_Data(bin + 12, 3, 0));
    connection->send(connection, target, new_r_Connection_Data(bin + 15, 3, 0));
    connection->send(connection, target, new_r_Connection_Data(bin + 18, 3, 0));
    connection->send(connection, target, new_r_Connection_Data(bin + 21, 3, 0));
    connection->send(connection, target, new_r_Connection_Data(bin + 24, 3, 0));
    connection->send(connection, target, new_r_Connection_Data(bin + 27, 2, 0));
    target->tell(target, new_m_Stop(0), NULL, system);


    b_Receiver_Receive *msg1 = receiver->expectMsgType(receiver, m_Receiver_Receive,  b_Receiver_Receive_extr, 300000);
    if (msg1->packet->msgId != 3) ffail("0_0 -> Bad packet received 2\n");


    connection->destroy(connection);
    pfree(connection);
    receiver->destroy(receiver);
    pfree(receiver);
    rTable->destroy(rTable);
    pfree(rTable);

    printf("OK\n");

}

//---- Send data to the connection
void GatewaySpec_t_1_0(ActorSystem *testSystem, ActorSystem *system) {
    printf("Test t_1_0 was started ... ");

    TestProbe *probe = new_TestProbe(testSystem, system,  "Probe");
    TestProbe *connection = new_TestProbe(testSystem, system,  "Connection");
    TestProbe *receiver = new_TestProbe(testSystem, system,  "Router");
    TestProbe *rTable = new_TestProbe(testSystem, system,  "Rtable");

    List *rt = new_List();
    rt->prepend(rt, receiver->ref);
    RoundRobinRoutingLogic *logic = new_RoundRobinRoutingLogic();
    Router *receivers = new_Router(ROUND_ROBIN_LOGIC, logic, rt);

    ActorRef *target = system->actorOf(system, new_Gateway(connection->ref, rTable->ref, NULL, receivers, 0, true), NULL);

    Packet *tPacket = Packet_createHelloPacket(3, 1);
    uint16_t *size = pmalloc(2);
    uint8_t *bin = Packet_toBinary(tPacket, size);

    probe->send(probe, target, new_m_Gateway_Send(tPacket, 32));
    b_Connection_SendData *msg = connection->expectMsgType(connection, m_Connection_SendData,  b_Connection_SendData_extr, 3000);

    if (msg->size != *size) ffail("1_0 size is not equal\n");

    for (int i = 0; i < *size; i++) {
        if (msg->data[i] != bin[i]) ffail("1_0 data not equal \n");
    }

    probe->destroy(probe);
    pfree(probe);
    connection->destroy(connection);
    pfree(connection);
    receiver->destroy(receiver);
    pfree(receiver);
    rTable->destroy(rTable);
    pfree(rTable);

    printf("OK\n");

    //probe->destroy(probe);
    //connection->destroy(connection);
    //receiver->destroy(receiver);
}

//---- After receive ConnectionClosed messages from Connection actor
//     1. Stop Connection actor
//     2. Remove all entry with self from rtable
//     3. Notify connector thats gw is stopped

void GatewaySpec_t_2_0(ActorSystem *testSystem, ActorSystem *system) {
    printf("Test t_2_0 was started ... ");

    TestProbe *connection = new_LockedTestProbe(testSystem, system,  "Connection");
    TestProbe *receiver = new_LockedTestProbe(testSystem, system,  "Router");
    TestProbe *rTable = new_TestProbe(testSystem, system,  "Rtable");
    TestProbe *connector = new_TestProbe(testSystem, system,  "Connector");

    List *rt = new_List();
    rt->prepend(rt, receiver->ref);
    RoundRobinRoutingLogic *logic = new_RoundRobinRoutingLogic();
    Router *receivers = new_Router(ROUND_ROBIN_LOGIC, logic, rt);

    ActorRef *target = system->actorOf(system, new_Gateway(connection->ref, rTable->ref, connector->ref, receivers, 0, true), NULL);
    connection->send(connection, target, new_r_Connection_ConnectionClosed(32));
    rTable->expectMsgType(rTable, m_Rtable_RemoveAllRoutesByGateway, b_Rtable_RemoveByGateway_extr, 3000);
    connector->expectMsgType(connector, r_Gateway_Stopped, empty_extr, 3000);

    connection->destroy(connection);
    pfree(connection);
    receiver->destroy(receiver);
    pfree(receiver);
    rTable->destroy(rTable);
    pfree(rTable);
    connector->destroy(connector);
    pfree(connector);

    printf("OK\n");
}

//---- If when processed heartbeat detects that lastIncomingActivity less than allowed -
//     1. Stop Connection actor
//     2. Remove all entry with self from rtable
//     3. Notify connector thats gw is stopped
void GatewaySpec_t_3_0(ActorSystem *testSystem, ActorSystem *system) {
    printf("Test t_3_0 was started ... ");

    TestProbe *heart = new_TestProbe(testSystem, system,  "Heart");
    TestProbe *connection = new_TestProbe(testSystem, system,  "Connection");
    TestProbe *receiver = new_TestProbe(testSystem, system,  "Router");
    TestProbe *rTable = new_TestProbe(testSystem, system,  "Rtable");
    TestProbe *connector = new_TestProbe(testSystem, system,  "Connector");

    List *rt = new_List();
    rt->prepend(rt, receiver->ref);
    RoundRobinRoutingLogic *logic = new_RoundRobinRoutingLogic();
    Router *receivers = new_Router(ROUND_ROBIN_LOGIC, logic, rt);

    ActorRef *target = system->actorOf(system, new_Gateway(connection->ref, rTable->ref, connector->ref, receivers, 0, true), NULL);
    heart->send(heart, target, new_r_Heart_HeartBeat(32));
    rTable->expectMsgType(rTable, m_Rtable_RemoveAllRoutesByGateway, b_Rtable_RemoveByGateway_extr, 3000);
    connector->expectMsgType(connector, r_Gateway_Stopped, empty_extr, 3000);

    /*heart->destroy(heart);
    pfree(heart);
    connection->destroy(connection);
    pfree(connection);
    receiver->destroy(receiver);
    pfree(receiver);
    rTable->destroy(rTable);
    pfree(rTable);
    connector->destroy(connector);
    pfree(connector);*/

    printf("OK\n");
}

//---- Set/Get gateway label through relevant messages
void GatewaySpec_t_4_0(ActorSystem *testSystem, ActorSystem *system) {
    printf("Test t_4_0 was started ... ");

    TestProbe *probe = new_TestProbe(testSystem, system,  "Probe");
    TestProbe *connection = new_TestProbe(testSystem, system,  "Connection");
    TestProbe *receiver = new_TestProbe(testSystem, system,  "Router");
    TestProbe *rTable = new_TestProbe(testSystem, system,  "Rtable");
    TestProbe *connector = new_TestProbe(testSystem, system,  "Connector");

    List *rt = new_List();
    rt->prepend(rt, receiver->ref);
    RoundRobinRoutingLogic *logic = new_RoundRobinRoutingLogic();
    Router *receivers = new_Router(ROUND_ROBIN_LOGIC, logic, rt);

    ActorRef *target = system->actorOf(system, new_Gateway(connection->ref, rTable->ref, connector->ref, receivers, 0, true), NULL);

    probe->send(probe, target, new_m_Gateway_SetLabel(888, 32));
    probe->send(probe, target, new_m_Gateway_GetLabel(32));

    b_Gateway_Label *msg0 = probe->expectMsgType(probe, r_Gateway_Label, b_Gateway_Label_extr, 3000);
    assertTrue(msg0->label == 888, "t_4_0-A");

    probe->destroy(probe);
    pfree(probe);
    connection->destroy(connection);
    pfree(connection);
    receiver->destroy(receiver);
    pfree(receiver);
    rTable->destroy(rTable);
    pfree(rTable);
    connector->destroy(connector);
    pfree(connector);

    printf("OK\n");
}


//---- Set/Get network address through relevant messages
void GatewaySpec_t_5_0(ActorSystem *testSystem, ActorSystem *system) {
    printf("Test t_5_0 was started ... ");

    TestProbe *probe = new_TestProbe(testSystem, system,  "Probe");
    TestProbe *connection = new_TestProbe(testSystem, system,  "Connection");
    TestProbe *receiver = new_TestProbe(testSystem, system,  "Router");
    TestProbe *rTable = new_TestProbe(testSystem, system,  "Rtable");
    TestProbe *connector = new_TestProbe(testSystem, system,  "Connector");

    List *rt = new_List();
    rt->prepend(rt, receiver->ref);
    RoundRobinRoutingLogic *logic = new_RoundRobinRoutingLogic();
    Router *receivers = new_Router(ROUND_ROBIN_LOGIC, logic, rt);

    ActorRef *target = system->actorOf(system, new_Gateway(connection->ref, rTable->ref, connector->ref, receivers, 0, true), NULL);

    probe->send(probe, target, new_m_Gateway_SetNetworkAddress(999, 32));
    probe->send(probe, target, new_m_Gateway_GetNetworkAddress(32));

    b_Gateway_NetworkAddress *msg0 = probe->expectMsgType(probe, r_Gateway_NetworkAddress, b_Gateway_NetworkAddress_extr, 3000);
    assertTrue(msg0->netAdr == 999, "t_5_0-A");

    probe->destroy(probe);
    pfree(probe);
    connection->destroy(connection);
    pfree(connection);
    receiver->destroy(receiver);
    pfree(receiver);
    rTable->destroy(rTable);
    pfree(rTable);
    connector->destroy(connector);
    pfree(connector);

    printf("OK\n");
}

void run_gateway_spec(ActorSystem *testSystem, ActorSystem *system) {
    //TODO в этом тесте есть очень серьезный баг относящийся к логике работы пробок. В шлюзах есть логика отправки сообщений stop. Данное сообщение в тесте прилетает пробке, в следствии чего, она останавливается и при попытке потом сделать ее дестрой, по понятным причинам происходит SEGFAULT
    printf("------start gateway_spec------\n");
    GatewaySpec_t_0_0(testSystem, system);
    GatewaySpec_t_1_0(testSystem, system);
    //GatewaySpec_t_2_0(testSystem, system);
    //GatewaySpec_t_3_0(testSystem, system);
    GatewaySpec_t_4_0(testSystem, system);
    GatewaySpec_t_5_0(testSystem, system);
    printf("-------end gateway_spec-------\n\n");
}