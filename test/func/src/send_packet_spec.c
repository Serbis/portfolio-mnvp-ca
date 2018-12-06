#include "../include/send_packet_spec.h"
#include "../../../libs/miniakka/includes/testprobe.h"
#include "../../../include/a_gateway_pool.h"
#include "../../../include/a_rtable.h"
#include "../../../libs/miniakka/includes/router.h"
#include "../../../include/a_gateway.h"
#include "../../../include/a_core.h"
#include "../../../oscl/include/time.h"
#include "../../extractors/include/extractors.h"
#include "../../../oscl/include/connection.h"
#include "../../../libs/cunit/include/asserts.h"
#include "../../../libs/miniakka/includes/standard_messages.h"
#include "../../../libs/miniakka/includes/actor_utils.h"

//-- Send mechanics must
//---- Send packet to the expected connection
void SendPacketSpec_t_0_0(ActorSystem *testSystem, ActorSystem *system) {
    printf("Test t_0_0 was started ... ");

    //---- Этап концигурирования

    TestProbe *probe = new_TestProbe(testSystem, system, "Probe");
    TestProbe *probe2 = new_TestProbe(testSystem, system, "Probe2");
    TestProbe *con0 = new_TestProbe(testSystem, system, "Con0");
    TestProbe *con1 = new_TestProbe(testSystem, system, "Con1");
    TestProbe *con2 = new_TestProbe(testSystem, system, "Con2");
    TestProbe *con3 = new_TestProbe(testSystem, system, "Con3");
    TestProbe *con4 = new_TestProbe(testSystem, system, "Con4");
    TestProbe *receiver = new_TestProbe(testSystem, system, "Receiver");
    TestProbe *trPool = new_TestProbe(testSystem, system, "RtPool");

    ActorRef *gwPool = system->actorOf(system, new_GatewayPool(), NULL);
    ActorRef *rTable = system->actorOf(system, new_Rtable(), NULL);

    List *rt = new_List();
    rt->prepend(rt, receiver->ref);
    RoundRobinRoutingLogic *logic = new_RoundRobinRoutingLogic();
    Router *receivers = new_Router(ROUND_ROBIN_LOGIC, logic, rt);

    ActorRef *gw0 = system->actorOf(system, new_Gateway(con0->ref, rTable, NULL, receivers, 0, false), NULL);
    ActorRef *gw1 = system->actorOf(system, new_Gateway(con1->ref, rTable, NULL, receivers, 0, false), NULL);
    ActorRef *gw2 = system->actorOf(system, new_Gateway(con2->ref, rTable, NULL, receivers, 0, false), NULL);
    ActorRef *gw3 = system->actorOf(system, new_Gateway(con3->ref, rTable, NULL, receivers, 0, false), NULL);
    ActorRef *gw4 = system->actorOf(system, new_Gateway(con4->ref, rTable, NULL, receivers, 0, false), NULL);

    gw0->tell(gw0, new_m_Gateway_SetNetworkAddress(1, 32), NULL, system);
    gw2->tell(gw2, new_m_Gateway_SetNetworkAddress(2, 32), NULL, system);
    gw4->tell(gw4, new_m_Gateway_SetNetworkAddress(3, 32), NULL, system);

    gw0->tell(gw0, new_m_Gateway_SetLabel(10, 32), NULL, system);
    gw2->tell(gw2, new_m_Gateway_SetLabel(20, 32), NULL, system);
    gw4->tell(gw4, new_m_Gateway_SetLabel(30, 32), NULL, system);

    gwPool->tell(gwPool, new_m_GatewayPool_Add(gw0, 32), NULL, system);
    gwPool->tell(gwPool, new_m_GatewayPool_Add(gw1, 32), NULL, system);
    gwPool->tell(gwPool, new_m_GatewayPool_Add(gw2, 32), NULL, system);
    gwPool->tell(gwPool, new_m_GatewayPool_Add(gw3, 32), NULL, system);
    gwPool->tell(gwPool, new_m_GatewayPool_Add(gw4, 32), NULL, system);


    ActorRef *target = system->actorOf(system, new_Core(gwPool, trPool->ref, rTable, 99), NULL);

    DelayMillis(1000); //ВНИМАНЕИЕ ТАЙМИНГ - за это время все ранее отправленные сообщения должны дойти до целевых акторов

    probe2->send(probe2, gw2, new_m_Gateway_GetLabel(32));
    b_Gateway_Label *lMsg = probe2->expectMsgType(probe2, r_Gateway_Label, b_Gateway_Label_extr, 5000);

    rTable->tell(rTable, new_m_Rtable_UpdateRoute(299, 3,  10, 5, 32), NULL, system);
    rTable->tell(rTable, new_m_Rtable_UpdateRoute(199, 4,  lMsg->label, 6, 32), NULL, system);
    rTable->tell(rTable, new_m_Rtable_UpdateRoute(399, 5,  30, 7, 32), NULL, system);

    DelayMillis(1000); //ВНИМАНЕИЕ ТАЙМИНГ - за это время все ранее отправленные сообщения должны дойти до целевых акторов

    //---- Начало исполнительное логики

    //---- Отправка запроса к core (1)

    Packet *packet = Packet_createHelloPacket(100,99);
    packet->dest = 199;
    uint16_t *packetSize = pmalloc(2);
    uint8_t *packetBin = Packet_toBinary(packet, packetSize);

    probe->send(probe, target, new_m_Core_SendPacket(packet, 32));

    //---- Результатом действия [1] должна стать отправка пакета в нужное соединение с послудиюм ответом отправителю
    //     запроса об успешном выполненеии операции

    b_Connection_SendData *bin = con2->expectMsgType(con2, m_Connection_SendData, b_Connection_SendData_extr, 3000);

    for (int i = 0; i < *packetSize; i++) {
        assertTrue(bin->data[i] == packetBin[i], "t_0_0-A");
    }

    b_SendPacketFsm_SendResult *msg0 = probe->expectMsgType(probe, r_SendPacketFsm_SendResult, b_SendPacketFsm_SendResult_extr, 3000);
    assertTrue(msg0->code == 0, "t_0_0-C");

    //---- Блок освобождения памяти

    probe->destroy(probe);
    pfree(probe);
    probe2->destroy(probe2);
    pfree(probe2);
    //con0->destroy(con0);
    //pfree(con0);
    //con1->destroy(con1);          НЕ МОГУ ОСТАНОВИТЬ ПРОБКИ ИЗ-ЗА ОШИБКИ ДОХОДА СООБЩЕНИЯ ОТ ОСТАНОВЛЕННОГО ШЛЮЗА
    //pfree(con1);
    //con2->destroy(con2);
    //pfree(con2);
    //con3->destroy(con3);
    //pfree(con3);
    //con4->destroy(con4);
    //pfree(con4);
    receiver->destroy(receiver);
    pfree(receiver);
    trPool->destroy(trPool);
    pfree(trPool);
    gwPool->tell(gwPool, new_m_Stop(32), NULL, system);
    AU_freeRef(gwPool);
    rTable->tell(rTable, new_m_Stop(32), NULL, system);
    AU_freeRef(rTable);
    //Освобождение роутера
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
    target->tell(target, new_m_Stop(32), NULL, system);
    AU_freeRef(target);
    pfree(lMsg);
    free_Packet(packet);
    pfree(bin->data);
    pfree(bin);
    pfree(msg0);

    printf("OK\n");
}

void run_send_packet_spec(ActorSystem *testSystem, ActorSystem *system) {
    printf("-----start send_packet_spec-----\n");
    SendPacketSpec_t_0_0(testSystem, system);
    printf("------end send_packet_spec------\n\n");
}