#include <string.h>
#include <zconf.h>
#include "../include/core_hello_spec.h"
#include "../../../libs/miniakka/includes/testprobe.h"
#include "../../../include/a_gateway_pool.h"
#include "../../../include/a_gateway.h"
#include "../../../include/a_core.h"
#include "../../../oscl/include/connection.h"
#include "../../../libs/cunit/include/asserts.h"
#include "../../../libs/miniakka/includes/standard_messages.h"
#include "../../../libs/miniakka/includes/actor_utils.h"
#include "../../../oscl/include/utils.h"
#include "../../extractors/include/extractors.h"


//-- Core after receive hello message must
//---- Send correct binary datagram with hello packet to each exist connection under each gateway
void CoreHelloSpec_t_0_0(ActorSystem *testSystem, ActorSystem *system) {
    printf("Test t_0_0 was started ... ");

    TestProbe *con0 = new_TestProbe(testSystem, system, "Con0");
    TestProbe *con1 = new_TestProbe(testSystem, system, "Con1");
    TestProbe *con2 = new_TestProbe(testSystem, system, "Con2");
    TestProbe *con3 = new_TestProbe(testSystem, system, "Con3");
    TestProbe *con4 = new_TestProbe(testSystem, system, "Con4");

    TestProbe *rTable = new_LockedTestProbe(testSystem, system, "Rtable");
    TestProbe *receiver = new_LockedTestProbe(testSystem, system, "Receiver");
    TestProbe *trPool = new_LockedTestProbe(testSystem, system, "Receiver");

    List *rt = new_List();
    rt->prepend(rt, receiver->ref);
    RoundRobinRoutingLogic *logic = new_RoundRobinRoutingLogic();
    Router *receivers = new_Router(ROUND_ROBIN_LOGIC, logic, rt);

    ActorRef *gw0 = system->actorOf(system, new_Gateway(con0->ref, rTable->ref, NULL, receivers, 0, false), NULL);
    ActorRef *gw1 = system->actorOf(system, new_Gateway(con1->ref, rTable->ref, NULL, receivers, 0, false), NULL);
    ActorRef *gw2 = system->actorOf(system, new_Gateway(con2->ref, rTable->ref, NULL, receivers, 0, false), NULL);
    ActorRef *gw3 = system->actorOf(system, new_Gateway(con3->ref, rTable->ref, NULL, receivers, 0, false), NULL);
    ActorRef *gw4 = system->actorOf(system, new_Gateway(con4->ref, rTable->ref, NULL, receivers, 0, false), NULL);

    ActorRef *gwPool = system->actorOf(system, new_GatewayPool(), NULL);
    gwPool->tell(gwPool, new_m_GatewayPool_Add(gw0, 32), NULL, system);
    gwPool->tell(gwPool, new_m_GatewayPool_Add(gw1, 32), NULL, system);
    gwPool->tell(gwPool, new_m_GatewayPool_Add(gw2, 32), NULL, system);
    gwPool->tell(gwPool, new_m_GatewayPool_Add(gw3, 32), NULL, system);
    gwPool->tell(gwPool, new_m_GatewayPool_Add(gw4, 32), NULL, system);



    ActorRef *target = system->actorOf(system, new_Core(gwPool, trPool->ref, rTable->ref, 99), NULL);

    target->tell(target, new_m_Core_Hello(32), NULL, system);

    b_Connection_SendData *bin0 = con0->expectMsgType(con0, m_Connection_SendData, b_Connection_SendData_extr , 300000);
    b_Connection_SendData *bin1 = con1->expectMsgType(con1, m_Connection_SendData, b_Connection_SendData_extr , 300000);
    b_Connection_SendData *bin2 = con2->expectMsgType(con2, m_Connection_SendData, b_Connection_SendData_extr , 300000);
    b_Connection_SendData *bin3 = con3->expectMsgType(con3, m_Connection_SendData, b_Connection_SendData_extr , 300000);
    b_Connection_SendData *bin4 = con4->expectMsgType(con4, m_Connection_SendData, b_Connection_SendData_extr , 300000);

    Packet *helloPacket = Packet_createHelloPacket(888, 99);
    uint16_t *helloSize = pmalloc(2);
    uint8_t *helloBin = Packet_toBinary(helloPacket, helloSize);

    assertTrue(bin0->size == *helloSize, "t_0_0-A");
    assertTrue(bin1->size == *helloSize, "t_0_0-B");
    assertTrue(bin2->size == *helloSize, "t_0_0-C");
    assertTrue(bin3->size == *helloSize, "t_0_0-D");
    assertTrue(bin4->size == *helloSize, "t_0_0-E");


    for (int i = 0; i < *helloSize; i++) {
        if (i != 5 && i != 6 && i != 7 && i != 8) {
            assertTrue(bin0->data[i] == helloBin[i], "t_0_0-F");
            assertTrue(bin1->data[i] == helloBin[i], "t_0_0-G");
            assertTrue(bin2->data[i] == helloBin[i], "t_0_0-H");
            assertTrue(bin3->data[i] == helloBin[i], "t_0_0-I");
            assertTrue(bin4->data[i] == helloBin[i], "t_0_0-J");
        }
    }

    con0->destroy(con0);
    pfree(con0);
    con1->destroy(con1);
    pfree(con1);
    con2->destroy(con2);
    pfree(con2);
    con3->destroy(con3);
    pfree(con3);
    con4->destroy(con4);
    pfree(con4);
    rTable->destroy(rTable);
    pfree(rTable);
    receiver->destroy(receiver);
    pfree(receiver);
    trPool->destroy(trPool);
    pfree(trPool);

    //Освобождение роутера

    /*gw0->tell(gw0, new_m_Stop(32), NULL);
    AU_freeRef(gw0);
    gw1->tell(gw1, new_m_Stop(32), NULL);
    AU_freeRef(gw1);
    gw2->tell(gw2, new_m_Stop(32), NULL);
    AU_freeRef(gw2);
    gw3->tell(gw3, new_m_Stop(32), NULL);
    AU_freeRef(gw3);
    gw4->tell(gw4, new_m_Stop(32), NULL);
    AU_freeRef(gw4);
    gwPool->tell(gwPool, new_m_Stop(32), NULL);
    AU_freeRef(gwPool);
    target->tell(target, new_m_Stop(32), NULL);
    AU_freeRef(target);
    free_Packet(helloPacket);
    pfree(helloSize);
    pfree(helloBin);
    pfree(bin0->data);
    pfree(bin0);
    pfree(bin1->data);
    pfree(bin1);
    pfree(bin2->data);
    pfree(bin2);
    pfree(bin3->data);
    pfree(bin3);
    pfree(bin4->data);
    pfree(bin4);*/


    printf("OK\n");
}

void run_core_hello_spec(ActorSystem *testSystem, ActorSystem *system) {
    printf("-----start core_hello_spec-----\n");
    CoreHelloSpec_t_0_0(testSystem, system);
    printf("------end core_hello_spec------\n\n");
}