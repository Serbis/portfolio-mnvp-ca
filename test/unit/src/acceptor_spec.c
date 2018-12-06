#include <string.h>
#include <stdlib.h>
#include "../include/acceptor_spec.h"
#include "../../../libs/miniakka/includes/testprobe.h"
#include "../../../include/a_acceptor.h"
#include "../../../oscl/include/a_tpcio.h"
#include "../../../oscl/include/a_tcp_connection_creator.h"
#include "../../../include/a_gateway_pool.h"
#include "../../../libs/miniakka/includes/standard_messages.h"
#include "../../../oscl/include/tcp_connection.h"

void *marker = (void *) 100;

void fail(char *reason) {
    printf("FAILED -> %s", reason);
    exit(1);
}

b_TcpIo_BindAndAccept* b_BindAndAccept_extr(b_TcpIo_BindAndAccept *body) {
    b_TcpIo_BindAndAccept *dup_body = pmalloc(sizeof(b_TcpIo_BindAndAccept));
    char *dup_address = pmalloc(strlen(body->address) + 1);
    memcpy(dup_address, body->address, strlen(body->address) + 1);
    dup_body->address = dup_address;
    dup_body->port = body->port;

    return dup_body;
}

//-- Acceptor actor must
//---- After receive AcceptTcp message must send to tcpio BindAndAccept message, after that, must create new gateway
//     and send it ref to the gateway pool if Connection was received from tcpio
void AcceptorSpec_t_0_0(ActorSystem *testSystem, ActorSystem *system) {
    printf("Test t_0_0 was started ... ");
    TestProbe *probe = new_TestProbe(testSystem, system, "MainProbe");
    TestProbe *gwPool = new_TestProbe(testSystem, system, "GwPool");
    TestProbe *tcpIo = new_TestProbe(testSystem, system, "TcpIo");
    ActorRef *target = system->actorOf(system, new_Acceptor(gwPool->ref), NULL);

    probe->send(probe, target, new_m_Acceptor_AcceptTcp(tcpIo->ref, "127.0.0.1", 5000, 0));
    b_TcpIo_BindAndAccept *msg = tcpIo->expectMsgType(tcpIo, m_TcpIO_BindAndAccept, (void *(*)(void *)) b_BindAndAccept_extr, 3000);
    if (strcmp(msg->address, "127.0.0.1") != 0) fail("compare error = msg->host");
    tcpIo->reply(tcpIo, new_r_TcpConnectionCreator_Connection(new_LocalActorRef(NULL, NULL, "x"), 0));

    //gwPool->expectMsgType(gwPool, m_GatewayPool_Add, NULL, 3000);
    //tcpIo->expectMsgType(tcpIo, m_TcpIO_BindAndAccept, NULL, 3000);

    /*probe->destroy(probe);
    pfree(probe);
    gwPool->destroy(gwPool);
    pfree(gwPool);
    pfree(msg->host);
    tcpIo->destroy(tcpIo);
    pfree(tcpIo);
    target->tell(target, new_m_Stop(0), NULL);
    pfree(target);*/


    printf("OK\n");
}

//---- After receive AcceptTcp message must send to tcpio BindAndAccept message, after that, must resend BindAndAccept
//     to tcpio after 3 second sleep timeout, if early BindFailed message was received from tcpio
void AcceptorSpec_t_0_1(ActorSystem *testSystem, ActorSystem *system) {
    marker = (void *) 100;
    printf("Test t_0_0 was started ... \n");
    TestProbe *probe = new_TestProbe(testSystem, system, "MainProbe");
    TestProbe *gwPool = new_TestProbe(testSystem, system, "GwPool");
    TestProbe *tcpIo = new_TestProbe(testSystem, system, "TcpIo");
    ActorRef *target = system->actorOf(system, new_Acceptor(gwPool->ref), NULL);

    probe->send(probe, target, new_m_Acceptor_AcceptTcp(tcpIo->ref, "127.0.0.1", 5000, 0));
    tcpIo->expectMsgType(tcpIo, m_TcpIO_BindAndAccept, NULL, 5000);
    tcpIo->reply(tcpIo, new_r_TcpConnectionCreator_BindFailed("test", 0));
    //probe->expectMsgType(probe, m_TcpIO_BindAndAccept, NULL, 5000);

    //probe->destroy(probe);
    //pfree(probe);
   // gwPool->destroy(gwPool);
    //pfree(gwPool);
    //tcpIo->destroy(tcpIo);
    //pfree(tcpIo);
    target->tell(target, new_m_Stop(0), NULL, system);
    //target->destroy(target);
    //pfree(target);

    printf("OK\n");
}

void run_acceptor_spec(ActorSystem *testSystem, ActorSystem *system) {
    printf("-----start acceptor_spec-----\n");
    AcceptorSpec_t_0_0(testSystem, system);
    //AcceptorSpec_t_0_1(testSystem, system); TODO что не так с этим тестом?
    printf("------end acceptor_spec------\n\n");
}