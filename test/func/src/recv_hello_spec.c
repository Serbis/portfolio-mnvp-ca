#include <zconf.h>
#include "../include/recv_hello_spec.h"
#include "../../../libs/miniakka/includes/testprobe.h"
#include "../../../libs/miniakka/includes/router.h"
#include "../../../include/a_gateway.h"
#include "../../../include/a_gateway_pool.h"
#include "../../../include/a_core.h"
#include "../../../include/a_receiver.h"
#include "../../../oscl/include/connection.h"
#include "../../../include/a_rtable.h"
#include "../../../libs/cunit/include/asserts.h"
#include "../../../oscl/include/time.h"
#include "../../../libs/miniakka/includes/standard_messages.h"
#include "../../../libs/miniakka/includes/actor_utils.h"

b_Rtable_UpdateRoute* b_Rtable_UpdateRoute_extr1(b_Rtable_UpdateRoute *body) {
    b_Rtable_UpdateRoute *dup_body = pmalloc(sizeof(b_Rtable_UpdateRoute));

    dup_body->dest = body->dest;
    dup_body->gateway = body->gateway;
    dup_body->distance = body->distance;

    return dup_body;
}

b_Gateway_NetworkAddress* b_Gateway_NetworkAddress_extr1(b_Gateway_NetworkAddress *body) {
    b_Gateway_NetworkAddress *dup_body = pmalloc(sizeof(b_Gateway_NetworkAddress));
    dup_body->netAdr = body->netAdr;

    return dup_body;
}

//-- Node must
//---- Create route entry and set gateway network address after receive hello packet datagram
void RecvHelloSpec_t_0_0(ActorSystem *testSystem, ActorSystem *system) {
    printf("Test t_0_0 was started ... ");

    TestProbe *probe = new_TestProbe(testSystem, system, "Probe");
    TestProbe *con0 = new_TestProbe(testSystem, system, "Con0");
    TestProbe *con1 = new_TestProbe(testSystem, system, "Con1");
    TestProbe *con2 = new_TestProbe(testSystem, system, "Con2");
    TestProbe *con3 = new_TestProbe(testSystem, system, "Con3");
    TestProbe *con4 = new_TestProbe(testSystem, system, "Con4");

    TestProbe *rTable = new_TestProbe(testSystem, system, "Rtable");
    TestProbe *trPool = new_TestProbe(testSystem, system, "trPool");

    ActorRef *gwPool = system->actorOf(system, new_GatewayPool(), NULL);
    ActorRef *receiver = system->actorOf(system, new_Receiver(gwPool, rTable->ref, trPool->ref, 1), NULL);

    List *rt = new_List();
    rt->prepend(rt, receiver);
    RoundRobinRoutingLogic *logic = new_RoundRobinRoutingLogic();
    Router *receivers = new_Router(ROUND_ROBIN_LOGIC, logic, rt);

    ActorRef *gw0 = system->actorOf(system, new_Gateway(con0->ref, rTable->ref, NULL, receivers, 0, false), NULL);
    ActorRef *gw1 = system->actorOf(system, new_Gateway(con1->ref, rTable->ref, NULL, receivers, 0, false), NULL);
    ActorRef *gw2 = system->actorOf(system, new_Gateway(con2->ref, rTable->ref, NULL, receivers, 0, false), NULL);
    ActorRef *gw3 = system->actorOf(system, new_Gateway(con3->ref, rTable->ref, NULL, receivers, 0, false), NULL);
    ActorRef *gw4 = system->actorOf(system, new_Gateway(con4->ref, rTable->ref, NULL, receivers, 0, false), NULL);

    gwPool->tell(gwPool, new_m_GatewayPool_Add(gw0, 32), NULL, system);
    gwPool->tell(gwPool, new_m_GatewayPool_Add(gw1, 32), NULL, system);
    gwPool->tell(gwPool, new_m_GatewayPool_Add(gw2, 32), NULL, system);
    gwPool->tell(gwPool, new_m_GatewayPool_Add(gw3, 32), NULL, system);
    gwPool->tell(gwPool, new_m_GatewayPool_Add(gw4, 32), NULL, system);

    Packet *helloPacket = Packet_createHelloPacket(888, 99);
    uint16_t *helloSize = pmalloc(2);
    uint8_t *helloBin = Packet_toBinary(helloPacket, helloSize);

    DelayMillis(1000); //ВНИМАНЕИЕ ТАЙМИНГ - за это время все ранее отправленные сообщения должны дойти до целевых акторов

    gw2->tell(gw2, new_r_Connection_Data(helloBin, *helloSize, 32), NULL, system);

    b_Rtable_UpdateRoute *msg0 = rTable->expectMsgType(rTable, m_Rtable_UpdateRoute, b_Rtable_UpdateRoute_extr1, 3000);

    assertTrue(msg0->dest == 99, "t_0_0-A");
    assertTrue(msg0->gateway == 99, "t_0_0-B");
    assertTrue(msg0->distance == 1, "t_0_0-C");

    DelayMillis(1000); //ВНИМАНЕИЕ ТАЙМИНГ - за это время шлюз должен получить назначанеие сетевого адреса

    b_Gateway_NetworkAddress *msg1 = NULL;
    for (int i = 0; i < 10; i ++) {
        probe->send(probe, gw2, new_m_Gateway_GetNetworkAddress(32));
        msg1 = probe->expectMsgType(probe, r_Gateway_NetworkAddress, b_Gateway_NetworkAddress_extr1, 30000);
        if (msg1->netAdr == 99)
            break;
        else
            DelayMillis(1000);
    }

    assertTrue(msg1 != NULL, "t_0_0-D");
    assertTrue(msg1->netAdr == 99, "t_0_0-E");

    probe->destroy(probe);
    pfree(probe);
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
    //TODO освобождение роутера
    gw0->tell(gw0, new_m_Stop(32), NULL, system);
    AU_freeRef(gw0);
    gw1->tell(gw1, new_m_Stop(32), NULL, system);
    AU_freeRef(gw1);
    gw2->tell(gw2, new_m_Stop(32), NULL, system);
    AU_freeRef(gw2);
    gw3->tell(gw3, new_m_Stop(32), NULL, system);
    AU_freeRef(gw3);
    gw4->tell(gw4, new_m_Stop(32), NULL, system);
    AU_freeRef(gw4);
    free_Packet(helloPacket);
    pfree(helloSize);
    pfree(helloBin);
    pfree(msg0);
    pfree(msg1);

    printf("OK\n");
}

void run_recv_hello_spec(ActorSystem *testSystem, ActorSystem *system) {
    printf("-----start recv_hello_spec-----\n");
    RecvHelloSpec_t_0_0(testSystem, system);
    printf("------end recv_hello_spec------\n\n");
}