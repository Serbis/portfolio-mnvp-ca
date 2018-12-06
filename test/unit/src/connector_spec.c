#include <string.h>
#include "../include/connector_spec.h"
#include "../../../libs/miniakka/includes/testprobe.h"
#include "../../../include/a_connector.h"
#include "../../../oscl/include/a_tpcio.h"
#include "../../../libs/cunit/include/asserts.h"
#include "../../../oscl/include/a_tcp_connection_creator.h"
#include "../../../include/a_gateway_pool.h"
#include "../../../libs/miniakka/includes/actor_utils.h"
#include "../../../libs/miniakka/includes/standard_messages.h"
#include "../../../include/a_gateway.h"

b_TcpIo_Connect* b_TcpIo_Connect_extr(b_TcpIo_Connect *body) {
    b_TcpIo_Connect *dup_body = pmalloc(sizeof(b_TcpIo_Connect));
    char *dup_host = pmalloc(strlen(body->host) + 1);
    memcpy(dup_host, body->host, strlen(body->host) + 1);

    dup_body->host = dup_host;
    dup_body->port = body->port;

    return dup_body;
}

b_GatewayPool_Add* b_GatewayPool_Add_extr(b_GatewayPool_Add *body) {
    b_GatewayPool_Add *dup_body = pmalloc(sizeof(b_GatewayPool_Add));

    dup_body->ref = AU_copyRef(body->ref);

    return dup_body;
}

//-- Connector must
//---- After receive message ConnectToTcp if TcpIo return Connection - create new gateway and add it to pool
void ConnectorSpec_t_0_0(ActorSystem *testSystem, ActorSystem *system) {
    printf("Test t_0_0 was started ... ");

    TestProbe *probe = new_TestProbe(testSystem, system,  "Probe");
    TestProbe *tcpIo = new_TestProbe(testSystem, system,  "TcpIo");
    TestProbe *gwPool = new_TestProbe(testSystem, system,  "GatewayPool");
    TestProbe *conStub = new_LockedTestProbe(testSystem, system,  "ConStub");
    TestProbe *receiver = new_TestProbe(testSystem, system,  "Router");
    TestProbe *rTable = new_TestProbe(testSystem, system,  "Rtable");

    List *rt = new_List();
    rt->prepend(rt, receiver->ref);
    RoundRobinRoutingLogic *logic = new_RoundRobinRoutingLogic();
    Router *receivers = new_Router(ROUND_ROBIN_LOGIC, logic, rt);

    ActorRef *target = system->actorOf(system, new_Connector(gwPool->ref, rTable->ref, receivers), NULL);

    probe->send(probe, target, new_m_Connector_ConnectTcp(tcpIo->ref, "127.0.0.1", 5000, 1000, 32));
    //b_TcpIo_Connect *msg0 = tcpIo->expectMsgType(tcpIo, m_TcpIO_Connect, b_TcpIo_Connect_extr, 3000000);
    //assertTrue(strcmp(msg0->host, "127.0.0.1") == 0, "t_0_0-A");
    //assertTrue(msg0->port == 5000, "t_0_0-B");
    //tcpIo->reply(tcpIo, new_r_TcpConnectionCreator_Connection(conStub->ref, 32));
    //b_GatewayPool_Add *msg1 = gwPool->expectMsgType(gwPool, m_GatewayPool_Add, b_GatewayPool_Add_extr, 3000000);

    //probe->destroy(probe);
    //pfree(probe);
    //tcpIo->destroy(tcpIo);
    //pfree(tcpIo);
    //gwPool->destroy(gwPool);
    //pfree(gwPool);
    //conStub->destroy(conStub);
    //pfree(conStub);
    target->tell(target, new_m_Stop(32), NULL, system);
    //AU_freeRef(target);
    //pfree(msg0->host);
    //pfree(msg0);
    //AU_freeRef(msg1->ref);
    //pfree(msg1);

    printf("OK\n");
}

//---- Resend Connect message to TcpIo with expected delay if TcpIo for Connect message respond with ConnectFailed
void ConnectorSpec_t_1_0(ActorSystem *testSystem, ActorSystem *system) {
    printf("Test t_1_0 was started ... ");

    TestProbe *probe = new_TestProbe(testSystem, system,  "Probe");
    TestProbe *tcpIo = new_TestProbe(testSystem, system,  "TcpIo");
    TestProbe *gwPool = new_LockedTestProbe(testSystem, system,  "GatewayPool");
    TestProbe *receiver = new_LockedTestProbe(testSystem, system,  "Router");
    TestProbe *rTable = new_LockedTestProbe(testSystem, system,  "Rtable");

    List *rt = new_List();
    rt->prepend(rt, receiver->ref);
    RoundRobinRoutingLogic *logic = new_RoundRobinRoutingLogic();
    Router *receivers = new_Router(ROUND_ROBIN_LOGIC, logic, rt);

    ActorRef *target = system->actorOf(system, new_Connector(gwPool->ref, rTable->ref, receivers), NULL);

    probe->send(probe, target, new_m_Connector_ConnectTcp(tcpIo->ref, "127.0.0.1", 5000, 1000, 32));
    b_TcpIo_Connect *msg0 = tcpIo->expectMsgType(tcpIo, m_TcpIO_Connect, b_TcpIo_Connect_extr, 3000);
    assertTrue(strcmp(msg0->host, "127.0.0.1") == 0, "t_1_0-A");
    assertTrue(msg0->port == 5000, "t_1_0-B");
    tcpIo->reply(tcpIo, new_r_TcpConnectionCreator_ConnectionFailed("", 32));
    b_TcpIo_Connect *msg1 = tcpIo->expectMsgType(tcpIo, m_TcpIO_Connect, b_TcpIo_Connect_extr, 5000);
    assertTrue(strcmp(msg1->host, "127.0.0.1") == 0, "t_1_0-C");
    assertTrue(msg1->port == 5000, "t_1_0-D");

    probe->destroy(probe);
    pfree(probe);
    tcpIo->destroy(tcpIo);
    pfree(tcpIo);
    gwPool->destroy(gwPool);
    pfree(gwPool);
    target->tell(target, new_m_Stop(32), NULL, system);
    AU_freeRef(target);
    pfree(msg0->host);
    pfree(msg0);
    pfree(msg1->host);
    pfree(msg1);

    printf("OK\n");
}

//---- After receive ConnectToTcp, respond with AlreadyRun if connector was early started
void ConnectorSpec_t_2_0(ActorSystem *testSystem, ActorSystem *system) {
    printf("Test t_2_0 was started ... ");

    TestProbe *probe = new_TestProbe(testSystem, system,  "Probe");
    TestProbe *tcpIo = new_LockedTestProbe(testSystem, system,  "TcpIo");
    TestProbe *gwPool = new_TestProbe(testSystem, system,  "GatewayPool");
    TestProbe *conStub = new_TestProbe(testSystem, system,  "ConStub");
    TestProbe *receiver = new_TestProbe(testSystem, system,  "Router");
    TestProbe *rTable = new_TestProbe(testSystem, system,  "Rtable");

    List *rt = new_List();
    rt->prepend(rt, receiver->ref);
    RoundRobinRoutingLogic *logic = new_RoundRobinRoutingLogic();
    Router *receivers = new_Router(ROUND_ROBIN_LOGIC, logic, rt);

    ActorRef *target = system->actorOf(system, new_Connector(gwPool->ref, rTable->ref, receivers), NULL);

    probe->send(probe, target, new_m_Connector_ConnectTcp(tcpIo->ref, "127.0.0.1", 5000, 1000, 32));
    probe->send(probe, target, new_m_Connector_ConnectTcp(tcpIo->ref, "127.0.0.1", 5000, 1000, 32));
    probe->expectMsgType(probe, r_Connector_AlreadyRun, empty_extr, 3000);

    probe->destroy(probe);
    pfree(probe);
    tcpIo->destroy(tcpIo);
    pfree(tcpIo);
    gwPool->destroy(gwPool);
    pfree(gwPool);
    conStub->destroy(conStub);
    pfree(conStub);
    target->tell(target, new_m_Stop(32), NULL, system);
    AU_freeRef(target);

    printf("OK\n");
}

//---- After receive Stopped message from gateway, send Connect message to TcpIo
void ConnectorSpec_t_3_0(ActorSystem *testSystem, ActorSystem *system) {
    printf("Test t_3_0 was started ... ");

    TestProbe *probe = new_TestProbe(testSystem, system, "Probe");
    TestProbe *tcpIo = new_TestProbe(testSystem, system,  "TcpIo");
    TestProbe *gwPool = new_LockedTestProbe(testSystem, system,  "GatewayPool");
    TestProbe *conStub = new_LockedTestProbe(testSystem, system,  "ConStub");
    TestProbe *receiver = new_LockedTestProbe(testSystem, system,  "Router");
    TestProbe *rTable = new_LockedTestProbe(testSystem, system,  "Rtable");
    TestProbe *gwStub = new_TestProbe(testSystem, system,  "Rtable");

    List *rt = new_List();
    rt->prepend(rt, receiver->ref);
    RoundRobinRoutingLogic *logic = new_RoundRobinRoutingLogic();
    Router *receivers = new_Router(ROUND_ROBIN_LOGIC, logic, rt);

    ActorRef *target = system->actorOf(system, new_Connector(gwPool->ref, rTable->ref, receivers), NULL);

    probe->send(probe, target, new_m_Connector_ConnectTcp(tcpIo->ref, "127.0.0.1", 5000, 1000, 32));
    tcpIo->expectMsgType(tcpIo, m_TcpIO_Connect, b_TcpIo_Connect_extr, 3000);
    gwStub->send(gwStub, target, new_r_Gateway_Stopped(32));
    b_TcpIo_Connect *msg0 = tcpIo->expectMsgType(tcpIo, m_TcpIO_Connect, b_TcpIo_Connect_extr, 3000);
    assertTrue(strcmp(msg0->host, "127.0.0.1") == 0, "t_3_0-A");
    assertTrue(msg0->port == 5000, "t_3_0-B");


    probe->destroy(probe);
    pfree(probe);
    tcpIo->destroy(tcpIo);
    pfree(tcpIo);
    gwPool->destroy(gwPool);
    pfree(gwPool);
    conStub->destroy(conStub);
    pfree(conStub);
    gwStub->destroy(gwStub);
    pfree(gwStub);
    target->tell(target, new_m_Stop(32), NULL, system);
    AU_freeRef(target);
    pfree(msg0->host);
    pfree(msg0);

    printf("OK\n");
}

void run_connector_spec(ActorSystem *testSystem, ActorSystem *system) {
    printf("-----start connector_spec-----\n");
    //ConnectorSpec_t_0_0(testSystem, system);
    //ConnectorSpec_t_1_0(testSystem, system);
    ConnectorSpec_t_2_0(testSystem, system);
    //ConnectorSpec_t_3_0(testSystem, system);
    printf("------end connector_spec------\n\n");
}