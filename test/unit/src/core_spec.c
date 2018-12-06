#include <string.h>
#include "../include/core_spec.h"
#include "../../../libs/miniakka/includes/testprobe.h"
#include "../../../include/a_gateway_pool.h"
#include "../../../include/a_core.h"
#include "../../../include/a_gateway.h"
#include "../../../oscl/include/connection.h"
#include "../../../libs/cunit/include/asserts.h"
#include "../../../libs/miniakka/includes/actor_utils.h"
#include "../../../libs/miniakka/includes/standard_messages.h"
#include "../../../include/a_transaction_pool.h"
#include "../../../include/a_rtable.h"
#include "../../extractors/include/extractors.h"
#include "../../../include/a_send_packet_fsm.h"
#include "../../../oscl/include/time.h"

b_GatewayPool_RouteToAll* b_GatewayPool_RouteToAll_extr_0(b_GatewayPool_RouteToAll *body) {
    b_GatewayPool_RouteToAll *dup_body = pmalloc(sizeof(b_GatewayPool_RouteToAll));
    dup_body->message = AU_copyMsg(body->message, sizeof(b_Gateway_Send));

    return dup_body;
}

b_TransactionPool_PoolAndExec* b_TransactionPool_PoolAndExec_0(b_TransactionPool_PoolAndExec *body) {
    b_TransactionPool_PoolAndExec *dup_body = pmalloc(sizeof(b_TransactionPool_PoolAndExec));
    dup_body->transaction = AU_copyRef(body->transaction);

    return dup_body;
}

//-- Core must
//---- Send RouteToAll message to GatewayPool with Send inner message
void CoreSpec_t_0_0(ActorSystem *testSystem, ActorSystem *system) {
    printf("Test t_0_0 was started ... ");

    TestProbe *probe = new_TestProbe(testSystem, system,  "Probe");
    TestProbe *gwPool = new_TestProbe(testSystem, system,  "GwPool");
    TestProbe *trPool = new_TestProbe(testSystem, system,  "TrPool");
    TestProbe *rTable = new_LockedTestProbe(testSystem, system,  "Rtable");

    ActorRef *target = system->actorOf(system, new_Core(gwPool->ref, trPool->ref, rTable->ref, 99), NULL);

    probe->send(probe, target, new_m_Core_Hello(32));
    b_GatewayPool_RouteToAll *msg0 = gwPool->expectMsgType(gwPool, m_GatewayPool_RouteToAll, b_GatewayPool_RouteToAll_extr_0, 3000);

    assertTrue(msg0->message->type == m_Gateway_Send, "t_0_0-A");

    probe->destroy(probe);
    pfree(probe);
    gwPool->destroy(gwPool);
    pfree(gwPool);
    trPool->destroy(trPool);
    pfree(trPool);
    target->tell(target, new_m_Stop(32), NULL, system);
    AU_freeRef(target);
    pfree(msg0);

    printf("OK\n");
}

//---- After receive Preq message send PoolAndExec message to TransactionPool
void CoreSpec_t_1_0(ActorSystem *testSystem, ActorSystem *system) {
    printf("Test t_1_0 was started ... ");

    TestProbe *probe = new_TestProbe(testSystem, system,  "Probe");
    TestProbe *gwPool = new_TestProbe(testSystem, system,  "GwPool");
    TestProbe *trPool = new_TestProbe(testSystem, system,  "TrPool");
    TestProbe *rTable = new_LockedTestProbe(testSystem, system,  "Rtable");

    ActorRef *target = system->actorOf(system, new_Core(gwPool->ref, trPool->ref, rTable->ref, 99), NULL);

    probe->send(probe, target, new_m_Core_Preq(0, 1000, 0, 0, 32));
    b_TransactionPool_PoolAndExec *msg0 = trPool->expectMsgType(trPool, m_TransactionPool_PoolAndExec, b_TransactionPool_PoolAndExec_0, 3000);

    probe->destroy(probe);
    pfree(probe);
    gwPool->destroy(gwPool);
    pfree(gwPool);
    trPool->destroy(trPool);
    pfree(trPool);
    target->tell(target, new_m_Stop(32), NULL, system);
    AU_freeRef(target);
    AU_freeRef(msg0->transaction);
    pfree(msg0);

    printf("OK\n");
}

//---- After receive SendPacket message send FindRoute to the Rtable from fsm
void CoreSpec_t_2_0(ActorSystem *testSystem, ActorSystem *system) {
    printf("Test t_2_0 was started ... ");

    TestProbe *probe = new_TestProbe(testSystem, system,  "Probe");
    TestProbe *gwPool = new_LockedTestProbe(testSystem, system,  "GwPool");
    TestProbe *trPool = new_LockedTestProbe(testSystem, system,  "TrPool");
    TestProbe *rTable = new_TestProbe(testSystem, system,  "Rtable");

    ActorRef *target = system->actorOf(system, new_Core(gwPool->ref, trPool->ref, rTable->ref, 99), NULL);

    Packet *packet = Packet_createHelloPacket(100, 99);
    packet->dest = 299;

    probe->send(probe, target, new_m_Core_SendPacket(packet, 32));
    b_Rtable_FindRoute *msg0 = rTable->expectMsgType(rTable, m_Rtable_FindRoute, b_Rtable_FindRoute_extr ,3000);
    assertTrue(msg0->dest == 299, "t_0_0-A");


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

    printf("OK\n");
}

void run_core_spec(ActorSystem *testSystem, ActorSystem *system) {
    printf("-----start core_spec-----\n");
    CoreSpec_t_0_0(testSystem, system);
    CoreSpec_t_1_0(testSystem, system);
    CoreSpec_t_2_0(testSystem, system);
    printf("------end core_spec------\n\n");
}