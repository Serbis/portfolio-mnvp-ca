#include <string.h>
#include "../include/gateway_pool_spec.h"
#include "../../../libs/miniakka/includes/testprobe.h"
#include "../../../include/a_gateway_pool.h"
#include "../../../include/a_gateway.h"
#include "../../../libs/miniakka/includes/actor_utils.h"
#include "../../../libs/cunit/include/asserts.h"
#include "../../../libs/miniakka/includes/standard_messages.h"

b_Gateway_SetLabel* b_Gateway_SetLabel_extr(b_Gateway_SetLabel *body) {
    b_Gateway_SetLabel *dup_body = pmalloc(sizeof(b_Gateway_SetLabel));
    dup_body->label = body->label;

    return dup_body;
}

b_GatewayPool_Gateway* b_GatewayPool_Gateway_extr(b_GatewayPool_Gateway *body) {
    b_GatewayPool_Gateway *dup_body = pmalloc(sizeof(b_GatewayPool_Gateway));
    if (body->ref != NULL)
        dup_body->ref = AU_copyRef(body->ref);
    else
        dup_body->ref = NULL;

    return dup_body;
}

//-- GatewayPool must
//---- Add/Get/Remove refs From/To pool
void GatewayPoolSpec_t_0_0(ActorSystem *testSystem, ActorSystem *system) {
    printf("Test t_0_0 was started ... ");

    TestProbe *probe = new_TestProbe(testSystem, system,  "Probe");
    TestProbe *gw0 = new_TestProbe(testSystem, system,  "Gw0");
    TestProbe *gw1 = new_TestProbe(testSystem, system,  "Gw1");
    TestProbe *gw2 = new_TestProbe(testSystem, system,  "Gw2");
    TestProbe *gw3 = new_TestProbe(testSystem, system,  "Gw3");
    TestProbe *gw4 = new_TestProbe(testSystem, system,  "Gw4");

    ActorRef *target = system->actorOf(system, new_GatewayPool(), NULL);

    probe->send(probe, target, new_m_GatewayPool_Add(gw0->ref, 32));
    b_Gateway_SetLabel *label0 = gw0->expectMsgType(gw0, m_Gateway_SetLabel, b_Gateway_SetLabel_extr, 3000);
    probe->send(probe, target, new_m_GatewayPool_Add(gw1->ref, 32));
    b_Gateway_SetLabel *label1 = gw1->expectMsgType(gw1, m_Gateway_SetLabel, b_Gateway_SetLabel_extr, 3000);
    probe->send(probe, target, new_m_GatewayPool_Add(gw2->ref, 32));
    b_Gateway_SetLabel *label2 = gw2->expectMsgType(gw2, m_Gateway_SetLabel, b_Gateway_SetLabel_extr, 3000);
    probe->send(probe, target, new_m_GatewayPool_Add(gw3->ref, 32));
    b_Gateway_SetLabel *label3 = gw3->expectMsgType(gw3, m_Gateway_SetLabel, b_Gateway_SetLabel_extr, 3000);
    probe->send(probe, target, new_m_GatewayPool_Add(gw4->ref, 32));
    b_Gateway_SetLabel *label4 = gw4->expectMsgType(gw4, m_Gateway_SetLabel, b_Gateway_SetLabel_extr, 3000);

    probe->send(probe, target, new_m_GatewayPool_GetByLabel(label4->label, 32));
    b_GatewayPool_Gateway *retGw4 = probe->expectMsgType(probe, r_GatewayPool_Gateway, b_GatewayPool_Gateway_extr, 3000);
    probe->send(probe, target, new_m_GatewayPool_GetByLabel(label2->label, 32));
    b_GatewayPool_Gateway *retGw2 = probe->expectMsgType(probe, r_GatewayPool_Gateway, b_GatewayPool_Gateway_extr, 3000);
    probe->send(probe, target, new_m_GatewayPool_GetByLabel(label0->label, 32));
    b_GatewayPool_Gateway *retGw0 = probe->expectMsgType(probe, r_GatewayPool_Gateway, b_GatewayPool_Gateway_extr, 3000);
    probe->send(probe, target, new_m_GatewayPool_GetByLabel(label1->label, 32));
    b_GatewayPool_Gateway *retGw1 = probe->expectMsgType(probe, r_GatewayPool_Gateway, b_GatewayPool_Gateway_extr, 3000);
    probe->send(probe, target, new_m_GatewayPool_GetByLabel(label3->label, 32));
    b_GatewayPool_Gateway *retGw3 = probe->expectMsgType(probe, r_GatewayPool_Gateway, b_GatewayPool_Gateway_extr, 3000);

    assertTrue(strcmp(retGw0->ref->actorName, gw0->name) == 0, "t_0_0-A");
    assertTrue(strcmp(retGw1->ref->actorName, gw1->name) == 0, "t_0_0-B");
    assertTrue(strcmp(retGw2->ref->actorName, gw2->name) == 0, "t_0_0-C");
    assertTrue(strcmp(retGw3->ref->actorName, gw3->name) == 0, "t_0_0-D");
    assertTrue(strcmp(retGw4->ref->actorName, gw4->name) == 0, "t_0_0-E");

    probe->send(probe, target, new_m_GatewayPool_RemoveByLabel(label4->label, 32));
    probe->send(probe, target, new_m_GatewayPool_GetByLabel(label4->label, 32));
    b_GatewayPool_Gateway *nullGw4 = probe->expectMsgType(probe, r_GatewayPool_Gateway, b_GatewayPool_Gateway_extr, 3000);
    probe->send(probe, target, new_m_GatewayPool_RemoveByLabel(label2->label, 32));
    probe->send(probe, target, new_m_GatewayPool_GetByLabel(label2->label, 32));
    b_GatewayPool_Gateway *nullGw2 = probe->expectMsgType(probe, r_GatewayPool_Gateway, b_GatewayPool_Gateway_extr, 3000);
    probe->send(probe, target, new_m_GatewayPool_RemoveByLabel(label0->label, 32));
    probe->send(probe, target, new_m_GatewayPool_GetByLabel(label0->label, 32));
    b_GatewayPool_Gateway *nullGw0 = probe->expectMsgType(probe, r_GatewayPool_Gateway, b_GatewayPool_Gateway_extr, 3000);
    probe->send(probe, target, new_m_GatewayPool_RemoveByLabel(label1->label, 32));
    probe->send(probe, target, new_m_GatewayPool_GetByLabel(label1->label, 32));
    b_GatewayPool_Gateway *nullGw1 = probe->expectMsgType(probe, r_GatewayPool_Gateway, b_GatewayPool_Gateway_extr, 3000);
    probe->send(probe, target, new_m_GatewayPool_RemoveByLabel(label3->label, 32));
    probe->send(probe, target, new_m_GatewayPool_GetByLabel(label3->label, 32));
    b_GatewayPool_Gateway *nullGw3 = probe->expectMsgType(probe, r_GatewayPool_Gateway, b_GatewayPool_Gateway_extr, 3000);

    assertTrue(nullGw0->ref == NULL, "t_0_0-F");
    assertTrue(nullGw1->ref == NULL, "t_0_0-G");
    assertTrue(nullGw2->ref == NULL, "t_0_0-H");
    assertTrue(nullGw3->ref == NULL, "t_0_0-I");
    assertTrue(nullGw4->ref == NULL, "t_0_0-J");

    probe->destroy(probe);
    pfree(probe);
    gw0->destroy(gw0);
    pfree(gw0);
    gw1->destroy(gw1);
    pfree(gw1);
    gw2->destroy(gw2);
    pfree(gw2);
    gw3->destroy(gw3);
    pfree(gw3);
    gw4->destroy(gw4);
    pfree(gw4);
    target->tell(target, new_m_Stop(32), NULL, system);
    AU_freeRef(target);
    pfree(label0);
    pfree(label1);
    pfree(label2);
    pfree(label3);
    pfree(label4);
    AU_freeRef(retGw0->ref);
    pfree(retGw0);
    AU_freeRef(retGw1->ref);
    pfree(retGw1);
    AU_freeRef(retGw2->ref);
    pfree(retGw2);
    AU_freeRef(retGw3->ref);
    pfree(retGw3);
    AU_freeRef(retGw4->ref);
    pfree(retGw4);
    pfree(nullGw0);
    pfree(nullGw1);
    pfree(nullGw2);
    pfree(nullGw3);
    pfree(nullGw4);

    printf("OK\n");
}

//---- Route message to all gateways by message RouteToAll
void GatewayPoolSpec_t_1_0(ActorSystem *testSystem, ActorSystem *system) {
    printf("Test t_1_0 was started ... ");

    TestProbe *probe = new_TestProbe(testSystem, system,  "Probe");
    TestProbe *gw0 = new_TestProbe(testSystem, system,  "Gw0");
    TestProbe *gw1 = new_TestProbe(testSystem, system,  "Gw1");
    TestProbe *gw2 = new_TestProbe(testSystem, system,  "Gw2");

    ActorRef *target = system->actorOf(system, new_GatewayPool(), NULL);

    probe->send(probe, target, new_m_GatewayPool_Add(gw0->ref, 32));
    gw0->expectMsgType(gw0, m_Gateway_SetLabel, empty_extr, 30000);
    probe->send(probe, target, new_m_GatewayPool_Add(gw1->ref, 32));
    gw1->expectMsgType(gw1, m_Gateway_SetLabel, empty_extr, 30000);
    probe->send(probe, target, new_m_GatewayPool_Add(gw2->ref, 32));
    gw2->expectMsgType(gw2, m_Gateway_SetLabel, empty_extr, 30000);

    probe->send(probe, target, new_m_GatewayPool_RouteToAll(new_m_Gateway_SetLabel(999, 32), 32));

    b_Gateway_SetLabel *msg2 = gw2->expectMsgType(gw2, m_Gateway_SetLabel, b_Gateway_SetLabel_extr, 3000);
    b_Gateway_SetLabel *msg1 = gw1->expectMsgType(gw1, m_Gateway_SetLabel, b_Gateway_SetLabel_extr, 3000);
    b_Gateway_SetLabel *msg0 = gw0->expectMsgType(gw0, m_Gateway_SetLabel, b_Gateway_SetLabel_extr, 3000);

    assertTrue(msg0->label == 999, "t_1_0-A");
    assertTrue(msg1->label == 999, "t_1_0-B");
    assertTrue(msg2->label == 999, "t_1_0-C");

    probe->destroy(probe);
    pfree(probe);
    gw0->destroy(gw0);
    pfree(gw0);
    gw1->destroy(gw1);
    pfree(gw1);
    gw2->destroy(gw2);
    pfree(gw2);
    target->tell(target, new_m_Stop(32), NULL, system);
    AU_freeRef(target);
    pfree(msg0);
    pfree(msg1);
    pfree(msg2);

    printf("OK\n");
}

//---- Route message to target gateway by message RouteByLabel
void GatewayPoolSpec_t_2_0(ActorSystem *testSystem, ActorSystem *system) {
    printf("Test t_2_0 was started ... ");

    TestProbe *probe = new_TestProbe(testSystem, system,  "Probe");
    TestProbe *gw0 = new_TestProbe(testSystem, system,  "Gw0");
    TestProbe *gw1 = new_TestProbe(testSystem, system,  "Gw1");
    TestProbe *gw2 = new_TestProbe(testSystem, system,  "Gw2");

    ActorRef *target = system->actorOf(system, new_GatewayPool(), NULL);

    probe->send(probe, target, new_m_GatewayPool_Add(gw0->ref, 32));
    b_Gateway_SetLabel *label0 = gw0->expectMsgType(gw0, m_Gateway_SetLabel, b_Gateway_SetLabel_extr, 30000);
    probe->send(probe, target, new_m_GatewayPool_Add(gw1->ref, 32));
    b_Gateway_SetLabel *label1 = gw1->expectMsgType(gw1, m_Gateway_SetLabel, b_Gateway_SetLabel_extr, 30000);
    probe->send(probe, target, new_m_GatewayPool_Add(gw2->ref, 32));
    b_Gateway_SetLabel *label2 = gw2->expectMsgType(gw2, m_Gateway_SetLabel, b_Gateway_SetLabel_extr, 30000);

    probe->send(probe, target, new_m_GatewayPool_RouteByLabel(label1->label, new_m_Gateway_SetLabel(999, 32), 32));

    b_Gateway_SetLabel *msg0 = gw1->expectMsgType(gw1, m_Gateway_SetLabel, b_Gateway_SetLabel_extr, 3000);

    assertTrue(msg0->label == 999, "t_1_0-A");

    probe->destroy(probe);
    pfree(probe);
    gw0->destroy(gw0);
    pfree(gw0);
    gw1->destroy(gw1);
    pfree(gw1);
    gw2->destroy(gw2);
    pfree(gw2);
    target->tell(target, new_m_Stop(32), NULL, system);
    AU_freeRef(target);
    pfree(label0);
    pfree(label1);
    pfree(label2);
    pfree(msg0);


    printf("OK\n");
}

void run_gateway_pool_spec(ActorSystem *testSystem, ActorSystem *system) {
    printf("-----start gateway_pool_spec-----\n");
    GatewayPoolSpec_t_0_0(testSystem, system);
    GatewayPoolSpec_t_1_0(testSystem, system);
    GatewayPoolSpec_t_2_0(testSystem, system);
    printf("------end gateway_pool_spec------\n\n");
}