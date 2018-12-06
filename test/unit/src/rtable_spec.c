#include "../include/rtable_spec.h"
#include "../../../libs/miniakka/includes/testprobe.h"
#include "../../../include/a_rtable.h"
#include "../../../libs/cunit/include/asserts.h"
#include "../../../libs/miniakka/includes/standard_messages.h"
#include "../../../libs/miniakka/includes/actor_utils.h"

b_Rtable_Route* b_Rtable_Route_extr(b_Rtable_Route *body) {
    b_Rtable_Route *dup_body = pmalloc(sizeof(b_Rtable_Route));

    dup_body->gateway = body->gateway;
    dup_body->dist = body->dist;
    dup_body->gwl = body->gwl;

    return dup_body;
}

//-- Rtable must
//---- Create route
void RtableSpec_t_0_0(ActorSystem *testSystem, ActorSystem *system) {
    printf("Test t_0_0 was started ... ");

    TestProbe *probe = new_TestProbe(testSystem, system,  "Probe");

    ActorRef *target = system->actorOf(system, new_Rtable(), NULL);

    probe->send(probe, target, new_m_Rtable_UpdateRoute(10, 20, 1, 30, 32));
    probe->send(probe, target, new_m_Rtable_UpdateRoute(11, 21, 1, 31, 32));
    probe->send(probe, target, new_m_Rtable_FindRoute(11, 32));
    b_Rtable_Route *msg0 = probe->expectMsgType(probe, r_Rtable_Route, b_Rtable_Route_extr, 3000);
    assertTrue(msg0->gateway == 21, "t_0_0-A");
    assertTrue(msg0->gwl == 1, "t_0_0-B");
    assertTrue(msg0->dist == 31, "t_0_0-C");

    probe->destroy(probe);
    pfree(probe);
    target->tell(target, new_m_Stop(32), NULL, system);
    AU_freeRef(target);
    pfree(msg0);

    printf("OK\n");
}

//---- Remove all routes by gateway
void RtableSpec_t_1_0(ActorSystem *testSystem, ActorSystem *system) {
    printf("Test t_1_0 was started ... ");

    TestProbe *probe = new_TestProbe(testSystem, system,  "Probe");

    ActorRef *target = system->actorOf(system, new_Rtable(), NULL);

    probe->send(probe, target, new_m_Rtable_UpdateRoute(10, 20, 1, 30, 32));
    probe->send(probe, target, new_m_Rtable_UpdateRoute(11, 21, 1, 31, 32));
    probe->send(probe, target, new_m_Rtable_UpdateRoute(18, 21, 1, 10, 32));
    probe->send(probe, target, new_m_Rtable_UpdateRoute(43, 15, 1, 12, 32));
    probe->send(probe, target, new_m_Rtable_RemoveAllRoutesByGateway(21, 32));
    probe->send(probe, target, new_m_Rtable_FindRoute(11, 32));
    b_Rtable_Route *msg0 = probe->expectMsgType(probe, r_Rtable_Route, b_Rtable_Route_extr, 3000);
    assertTrue(msg0->gateway == 0, "t_1_0-A");
    assertTrue(msg0->gwl == 0, "t_1_0-B");
    assertTrue(msg0->dist == 0, "t_1_0-C");

    probe->destroy(probe);
    pfree(probe);
    target->tell(target, new_m_Stop(32), NULL, system);
    AU_freeRef(target);
    pfree(msg0);

    printf("OK\n");
}

//---- Remove all routes by destination
void RtableSpec_t_2_0(ActorSystem *testSystem, ActorSystem *system) {
    printf("Test t_2_0 was started ... ");

    TestProbe *probe = new_TestProbe(testSystem, system,  "Probe");

    ActorRef *target = system->actorOf(system, new_Rtable(), NULL);

    probe->send(probe, target, new_m_Rtable_UpdateRoute(10, 20, 1, 30, 32));
    probe->send(probe, target, new_m_Rtable_UpdateRoute(11, 21, 1, 31, 32));
    probe->send(probe, target, new_m_Rtable_UpdateRoute(11, 31, 1, 10, 32));
    probe->send(probe, target, new_m_Rtable_UpdateRoute(43, 15, 1, 12, 32));
    probe->send(probe, target, new_m_Rtable_RemoveAllRoutesByDest(11, 32));
    probe->send(probe, target, new_m_Rtable_FindRoute(11, 32));
    b_Rtable_Route *msg0 = probe->expectMsgType(probe, r_Rtable_Route, b_Rtable_Route_extr, 3000);
    assertTrue(msg0->gateway == 0, "t_2_0-A");
    assertTrue(msg0->gwl == 0, "t_2_0-B");
    assertTrue(msg0->dist == 0, "t_2_0-C");

    probe->destroy(probe);
    pfree(probe);
    target->tell(target, new_m_Stop(32), NULL, system);
    AU_freeRef(target);
    pfree(msg0);

    printf("OK\n");
}

//---- Find a route based on distance (with 0 if route not found)
void RtableSpec_t_3_0(ActorSystem *testSystem, ActorSystem *system) {
    printf("Test t_3_0 was started ... ");

    TestProbe *probe = new_TestProbe(testSystem, system,  "Probe");

    ActorRef *target = system->actorOf(system, new_Rtable(), NULL);

    probe->send(probe, target, new_m_Rtable_UpdateRoute(10, 20, 1, 30, 32));
    probe->send(probe, target, new_m_Rtable_UpdateRoute(11, 21, 1, 30, 32));
    probe->send(probe, target, new_m_Rtable_UpdateRoute(11, 22, 1, 10, 32));
    probe->send(probe, target, new_m_Rtable_UpdateRoute(11, 23, 1, 20, 32));
    probe->send(probe, target, new_m_Rtable_UpdateRoute(43, 15, 1, 12, 32));
    probe->send(probe, target, new_m_Rtable_FindRoute(11, 32));
    b_Rtable_Route *msg0 = probe->expectMsgType(probe, r_Rtable_Route, b_Rtable_Route_extr, 3000);
    assertTrue(msg0->gateway == 22, "t_3_0-A");
    assertTrue(msg0->gwl == 1, "t_3_0-B");
    assertTrue(msg0->dist == 10, "t_3_0-C");

    probe->destroy(probe);
    pfree(probe);
    target->tell(target, new_m_Stop(32), NULL, system);
    AU_freeRef(target);
    pfree(msg0);

    printf("OK\n");
}


void run_rtable_spec(ActorSystem *testSystem, ActorSystem *system) {
    printf("-----start rtable_spec-----\n");
    RtableSpec_t_0_0(testSystem, system);
    //RtableSpec_t_1_0(testSystem, system);
    //RtableSpec_t_2_0(testSystem, system);
    //RtableSpec_t_3_0(testSystem, system);
    printf("------end rtable_spec------\n\n");
}