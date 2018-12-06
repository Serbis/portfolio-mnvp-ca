#include <string.h>
#include "../include/preq_spec.h"
#include "../../../libs/miniakka/includes/testprobe.h"
#include "../../../libs/miniakka/includes/router.h"
#include "../../../include/a_gateway.h"
#include "../../../include/a_gateway_pool.h"
#include "../../../include/a_core.h"
#include "../../../oscl/include/connection.h"
#include "../../../libs/cunit/include/asserts.h"
#include "../../../include/a_transaction_pool.h"
#include "../../../oscl/include/time.h"
#include "../../../include/transactions/a_preq_transaction.h"
#include "../../../libs/miniakka/includes/actor_utils.h"
#include "../../../libs/miniakka/includes/standard_messages.h"
#include "../../../include/a_receiver.h"
#include "../../../include/a_rtable.h"

b_Connection_SendData* b_Connection_SendData_extr2(b_Connection_SendData *body) {
    b_Connection_SendData *dup_body = pmalloc(sizeof(b_Connection_SendData));
    uint8_t *dup_data = pmalloc(body->size);
    memcpy(dup_data, body->data, body->size);

    dup_body->size = body->size;
    dup_body->data = dup_data;

    return dup_body;
}

b_Rtable_UpdateRoute* b_Rtable_UpdateRoute_extr2(b_Rtable_UpdateRoute *body) {
    b_Rtable_UpdateRoute *dup_body = pmalloc(sizeof(b_Rtable_UpdateRoute));

    dup_body->dest = body->dest;
    dup_body->gateway = body->gateway;
    dup_body->distance = body->distance;

    return dup_body;
}

void PreqSpec_t_0_0(ActorSystem *testSystem, ActorSystem *system) {
    printf("Test t_0_0 was started ... ");

    //---- Этап концигурирования

    TestProbe *probe = new_TestProbe(testSystem, system, "Probe");
    TestProbe *con0 = new_TestProbe(testSystem, system, "Con0");
    TestProbe *con1 = new_TestProbe(testSystem, system, "Con1");
    TestProbe *con2 = new_TestProbe(testSystem, system, "Con2");
    TestProbe *con3 = new_TestProbe(testSystem, system, "Con3");
    TestProbe *con4 = new_TestProbe(testSystem, system, "Con4");
    TestProbe *rTable = new_LockedTestProbe(testSystem, system, "Rtable");

    ActorRef *gwPool = system->actorOf(system, new_GatewayPool(), NULL);

    ActorRef *trPool = system->actorOf(system, new_TransactionPool(), NULL);

    ActorRef *receiver = system->actorOf(system, new_Receiver(gwPool, rTable->ref, trPool, 99), NULL);

    List *rt = new_List();
    rt->prepend(rt, receiver);
    RoundRobinRoutingLogic *logic = new_RoundRobinRoutingLogic();
    Router *receivers = new_Router(ROUND_ROBIN_LOGIC, logic, rt);

    ActorRef *gw0 = system->actorOf(system, new_Gateway(con0->ref, rTable->ref, NULL, receivers, 0, false), NULL);
    ActorRef *gw1 = system->actorOf(system, new_Gateway(con1->ref, rTable->ref, NULL, receivers, 0, false), NULL);
    ActorRef *gw2 = system->actorOf(system, new_Gateway(con2->ref, rTable->ref, NULL, receivers, 0, false), NULL);
    ActorRef *gw3 = system->actorOf(system, new_Gateway(con3->ref, rTable->ref, NULL, receivers, 0, false), NULL);
    ActorRef *gw4 = system->actorOf(system, new_Gateway(con4->ref, rTable->ref, NULL, receivers, 0, false), NULL);

    gw0->tell(gw0, new_m_Gateway_SetNetworkAddress(1, 32), NULL, system);
    gw2->tell(gw2, new_m_Gateway_SetNetworkAddress(2, 32), NULL, system);
    gw4->tell(gw4, new_m_Gateway_SetNetworkAddress(3, 32), NULL, system);

    gwPool->tell(gwPool, new_m_GatewayPool_Add(gw0, 32), NULL, system);
    gwPool->tell(gwPool, new_m_GatewayPool_Add(gw1, 32), NULL, system);
    gwPool->tell(gwPool, new_m_GatewayPool_Add(gw2, 32), NULL, system);
    gwPool->tell(gwPool, new_m_GatewayPool_Add(gw3, 32), NULL, system);
    gwPool->tell(gwPool, new_m_GatewayPool_Add(gw4, 32), NULL, system);


    ActorRef *target = system->actorOf(system, new_Core(gwPool, trPool, rTable->ref, 99), NULL);

    DelayMillis(1000); //ВНИМАНЕИЕ ТАЙМИНГ - за это время все ранее отправленные сообщения должны дойти до целевых акторов

    //---- Начало исполнительное логики

    //---- Отправка запроса к core (1)

    probe->send(probe, target, new_m_Core_Preq(199, 10000, 3, 8, 32));

    DelayMillis(1000); //ВНИМАНЕИЕ ТАЙМИНГ - за это время все ранее отправленные сообщения должны дойти до целевых акторов

    //---- Результатом действия [1] должна стать широковещаетльная отправке PREQ пакетов по сеоидениням, цель
    //     блока - убедиться в коннектности отылаемых пакетов и узнать номер транзакции

    b_Connection_SendData *bin4 = con4->expectMsgType(con4, m_Connection_SendData, b_Connection_SendData_extr2 , 5000);
    b_Connection_SendData *bin3 = con3->expectMsgType(con3, m_Connection_SendData, b_Connection_SendData_extr2 , 5000);
    b_Connection_SendData *bin2 = con2->expectMsgType(con2, m_Connection_SendData, b_Connection_SendData_extr2 , 5000);
    b_Connection_SendData *bin1 = con1->expectMsgType(con1, m_Connection_SendData, b_Connection_SendData_extr2 , 5000);
    b_Connection_SendData *bin0 = con0->expectMsgType(con0, m_Connection_SendData, b_Connection_SendData_extr2 , 5000);

    Packet *compPreqPacket = Packet_createPreqPacket(0, 99, 199, 8);
    uint16_t *compPreqSize = pmalloc(2);
    uint8_t *compPreqBin = Packet_toBinary(compPreqPacket, compPreqSize);

    assertTrue(bin0->size == *compPreqSize, "t_0_0-A");
    assertTrue(bin1->size == *compPreqSize, "t_0_0-B");
    assertTrue(bin2->size == *compPreqSize, "t_0_0-C");
    assertTrue(bin3->size == *compPreqSize, "t_0_0-D");
    assertTrue(bin4->size == *compPreqSize, "t_0_0-E");


    for (int i = 0; i < *compPreqSize; i++) {
        if (i != 5 && i != 6 && i != 7 && i != 8) {
            assertTrue(bin0->data[i] == compPreqBin[i], "t_0_0-F");
            assertTrue(bin1->data[i] == compPreqBin[i], "t_0_0-G");
            assertTrue(bin2->data[i] == compPreqBin[i], "t_0_0-H");
            assertTrue(bin3->data[i] == compPreqBin[i], "t_0_0-I");
            assertTrue(bin4->data[i] == compPreqBin[i], "t_0_0-J");
        }
    }

    Packet *p = pmalloc(sizeof(Packet));
    p->body = NULL;
    Packet_parsePacketHeader(p, bin0->data, 28);
    uint32_t tid = p->msgId;

    //---- Имитация приходязих ответов от найденных узлов. Блок проверяет, что все эти маршруты
    //     заносятся в таблицу маршрутизации

    compPreqPacket->msgId = tid;
    Packet *ackMod1 = Packet_ackPreqPacket(compPreqPacket, 3);
    Packet *ackMod2 = Packet_ackPreqPacket(compPreqPacket, 6);
    Packet *ackMod3 = Packet_ackPreqPacket(compPreqPacket, 9);

    uint16_t *ackMod1Size = pmalloc(2);
    uint8_t *ackMod1Bin = Packet_toBinary(ackMod1, ackMod1Size);
    uint16_t *ackMod2Size = pmalloc(2);
    uint8_t *ackMod2Bin = Packet_toBinary(ackMod2, ackMod2Size);
    uint16_t *ackMod3Size = pmalloc(2);
    uint8_t *ackMod3Bin = Packet_toBinary(ackMod3, ackMod3Size);

    gw0->tell(gw0, new_r_Connection_Data(ackMod1Bin, *ackMod1Size, 32), NULL, system);
    gw2->tell(gw2, new_r_Connection_Data(ackMod2Bin, *ackMod2Size, 32), NULL, system);
    gw4->tell(gw4, new_r_Connection_Data(ackMod3Bin, *ackMod3Size, 32), NULL, system);


    //rTable.expectMsgAllOf(
    //  Rtable.Commands.UpdateRoute(199, 1, 8),
    //  Rtable.Commands.UpdateRoute(199, 2, 8),
    //  Rtable.Commands.UpdateRoute(199, 3, 8)
    //)

    //---- Получаем ответ FOUND от Core

    probe->expectMsgType(probe, r_PreqTransaction_Found, empty_extr, 5000);

    //---- Блок освобождения памяти

    probe->destroy(probe);
    pfree(probe);
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
    rTable->destroy(rTable);
    pfree(rTable);
    gwPool->tell(gwPool, new_m_Stop(32), NULL, system);
    AU_freeRef(gwPool);
    trPool->tell(trPool, new_m_Stop(32), NULL, system);
    AU_freeRef(trPool);
    receiver->tell(receiver, new_m_Stop(32), NULL, system);
    AU_freeRef(receiver);
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
    pfree(bin0->data);
    pfree(bin0);
    pfree(bin1->data);
    pfree(bin1);
    pfree(bin2->data);
    pfree(bin2);
    pfree(bin3->data);
    pfree(bin3);
    pfree(bin4->data);
    pfree(bin4);
    free_Packet(compPreqPacket);
    pfree(compPreqSize);
    pfree(compPreqBin);
    free_Packet(p);
    free_Packet(ackMod1);
    free_Packet(ackMod2);
    free_Packet(ackMod3);
    pfree(ackMod1Size);
    pfree(ackMod1Bin);
    pfree(ackMod2Size);
    pfree(ackMod2Bin);
    pfree(ackMod3);
    pfree(ackMod2Bin);

    printf("OK\n");
}

void PreqSpec_t_1_0(ActorSystem *testSystem, ActorSystem *system) {
    printf("Test t_1_0 was started ... ");

    //---- Этап концигурирования

    TestProbe *con0 = new_TestProbe(testSystem, system, "Con0");
    TestProbe *con1 = new_TestProbe(testSystem, system, "Con1");
    TestProbe *con2 = new_TestProbe(testSystem, system, "Con2");
    TestProbe *con3 = new_TestProbe(testSystem, system, "Con3");
    TestProbe *con4 = new_TestProbe(testSystem, system, "Con4");
    TestProbe *rTable = new_TestProbe(testSystem, system, "Rtable");

    ActorRef *gwPool = system->actorOf(system, new_GatewayPool(), NULL);

    ActorRef *trPool = system->actorOf(system, new_TransactionPool(), NULL);

    ActorRef *receiver = system->actorOf(system, new_Receiver(gwPool, rTable->ref, trPool, 199), NULL);

    List *rt = new_List();
    rt->prepend(rt, receiver);
    RoundRobinRoutingLogic *logic = new_RoundRobinRoutingLogic();
    Router *receivers = new_Router(ROUND_ROBIN_LOGIC, logic, rt);

    ActorRef *gw0 = system->actorOf(system, new_Gateway(con0->ref, rTable->ref, NULL, receivers, 0, false), NULL);
    ActorRef *gw1 = system->actorOf(system, new_Gateway(con1->ref, rTable->ref, NULL, receivers, 0, false), NULL);
    ActorRef *gw2 = system->actorOf(system, new_Gateway(con2->ref, rTable->ref, NULL, receivers, 0, false), NULL);
    ActorRef *gw3 = system->actorOf(system, new_Gateway(con3->ref, rTable->ref, NULL, receivers, 0, false), NULL);
    ActorRef *gw4 = system->actorOf(system, new_Gateway(con4->ref, rTable->ref, NULL, receivers, 0, false), NULL);

    gw0->tell(gw0, new_m_Gateway_SetNetworkAddress(1, 32), NULL, system);
    gw2->tell(gw2, new_m_Gateway_SetNetworkAddress(2, 32), NULL, system);
    gw4->tell(gw4, new_m_Gateway_SetNetworkAddress(3, 32), NULL, system);

    gwPool->tell(gwPool, new_m_GatewayPool_Add(gw0, 32), NULL, system);
    gwPool->tell(gwPool, new_m_GatewayPool_Add(gw1, 32), NULL, system);
    gwPool->tell(gwPool, new_m_GatewayPool_Add(gw2, 32), NULL, system);
    gwPool->tell(gwPool, new_m_GatewayPool_Add(gw3, 32), NULL, system);
    gwPool->tell(gwPool, new_m_GatewayPool_Add(gw4, 32), NULL, system);

    DelayMillis(1000); //ВНИМАНЕИЕ ТАЙМИНГ - за это время все ранее отправленные сообщения должны дойти до целевых акторов

    //---- Начало исполнительное логики

    //---- Получение PREQ пакета одним из соединений (1)

    Packet *inPacket = Packet_createPreqPacket(100, 99, 199, 8);
    inPacket->ttl = 5;
    uint16_t *inSize = pmalloc(2);
    uint8_t *inBin = Packet_toBinary(inPacket, inSize);

    gw0->tell(gw0, new_r_Connection_Data(inBin, *inSize, 32), NULL, system);

    DelayMillis(1000); //ВНИМАНЕИЕ ТАЙМИНГ - за это время все ранее отправленные сообщения должны дойти до целевых акторов

    //---- Результатом действия [1] должно стать создание новой маршрутоной записи и отылка preq_ack пакет
    //     в тот же шлюз и которого пришела наутральная версия

    b_Rtable_UpdateRoute *msg0 = rTable->expectMsgType(rTable, m_Rtable_UpdateRoute, b_Rtable_UpdateRoute_extr2, 5000);

    assertTrue(msg0->dest == 99, "t_1_0-A");
    assertTrue(msg0->gateway == 1, "t_1_0-B");
    assertTrue(msg0->distance == 5, "t_1_0-C");

    b_Connection_SendData *bin0 = con0->expectMsgType(con0, m_Connection_SendData, b_Connection_SendData_extr2 , 5000);

    Packet *compAckPreqPacket = Packet_ackPreqPacket(inPacket, 5);
    uint16_t *compAckPreqSize = pmalloc(2);
    uint8_t *compAckPreqBin = Packet_toBinary(compAckPreqPacket, compAckPreqSize);

    assertTrue(bin0->size == *compAckPreqSize, "t_1_0-D");

    for (int i = 0; i < *inSize; i++) {
        assertTrue(bin0->data[i] == compAckPreqBin[i], "t_1_0-F");
    }

    //---- Блок освобождения памяти


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
    rTable->destroy(rTable);
    pfree(rTable);
    gwPool->tell(gwPool, new_m_Stop(32), NULL, system);
    AU_freeRef(gwPool);
    trPool->tell(trPool, new_m_Stop(32), NULL, system);
    AU_freeRef(trPool);
    receiver->tell(receiver, new_m_Stop(32), NULL, system);
    AU_freeRef(receiver);
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
    pfree(bin0->data);
    pfree(bin0);
    free_Packet(inPacket);
    pfree(inSize);
    pfree(inBin);
    free_Packet(compAckPreqPacket);
    pfree(compAckPreqSize);
    pfree(compAckPreqBin);
    pfree(msg0);

    printf("OK\n");
}

void PreqSpec_t_2_0(ActorSystem *testSystem, ActorSystem *system) {
    printf("Test t_2_0 was started ... ");

    //---- Этап концигурирования

    TestProbe *con0 = new_TestProbe(testSystem, system, "Con0");
    TestProbe *con1 = new_TestProbe(testSystem, system, "Con1");
    TestProbe *con2 = new_TestProbe(testSystem, system, "Con2");
    TestProbe *con3 = new_TestProbe(testSystem, system, "Con3");
    TestProbe *con4 = new_TestProbe(testSystem, system, "Con4");
    TestProbe *rTable = new_TestProbe(testSystem, system, "Rtable");

    ActorRef *gwPool = system->actorOf(system, new_GatewayPool(), NULL);

    ActorRef *trPool = system->actorOf(system, new_TransactionPool(), NULL);

    ActorRef *receiver = system->actorOf(system, new_Receiver(gwPool, rTable->ref, trPool, 111), NULL);

    List *rt = new_List();
    rt->prepend(rt, receiver);
    RoundRobinRoutingLogic *logic = new_RoundRobinRoutingLogic();
    Router *receivers = new_Router(ROUND_ROBIN_LOGIC, logic, rt);

    ActorRef *gw0 = system->actorOf(system, new_Gateway(con0->ref, rTable->ref, NULL, receivers, 0, false), NULL);
    ActorRef *gw1 = system->actorOf(system, new_Gateway(con1->ref, rTable->ref, NULL, receivers, 0, false), NULL);
    ActorRef *gw2 = system->actorOf(system, new_Gateway(con2->ref, rTable->ref, NULL, receivers, 0, false), NULL);
    ActorRef *gw3 = system->actorOf(system, new_Gateway(con3->ref, rTable->ref, NULL, receivers, 0, false), NULL);
    ActorRef *gw4 = system->actorOf(system, new_Gateway(con4->ref, rTable->ref, NULL, receivers, 0, false), NULL);

    gw0->tell(gw0, new_m_Gateway_SetNetworkAddress(1, 32), NULL, system);
    gw2->tell(gw2, new_m_Gateway_SetNetworkAddress(2, 32), NULL, system);
    gw4->tell(gw4, new_m_Gateway_SetNetworkAddress(3, 32), NULL, system);

    gwPool->tell(gwPool, new_m_GatewayPool_Add(gw0, 32), NULL, system);
    gwPool->tell(gwPool, new_m_GatewayPool_Add(gw1, 32), NULL, system);
    gwPool->tell(gwPool, new_m_GatewayPool_Add(gw2, 32), NULL, system);
    gwPool->tell(gwPool, new_m_GatewayPool_Add(gw3, 32), NULL, system);
    gwPool->tell(gwPool, new_m_GatewayPool_Add(gw4, 32), NULL, system);


    ActorRef *target = system->actorOf(system, new_Core(gwPool, trPool, rTable->ref, 99), NULL);

    DelayMillis(1000); //ВНИМАНЕИЕ ТАЙМИНГ - за это время все ранее отправленные сообщения должны дойти до целевых акторов

    //---- Начало исполнительное логики

    //---- Получение PREQ пакета одним из соединений (1)

    Packet *inPacket = Packet_createPreqPacket(100, 99, 199, 8);
    inPacket->ttl = 4;
    uint16_t *inSize = pmalloc(2);
    uint8_t *inBin = Packet_toBinary(inPacket, inSize);

    gw0->tell(gw0, new_r_Connection_Data(inBin, *inSize, 32), NULL, system);

    DelayMillis(1000); //ВНИМАНЕИЕ ТАЙМИНГ - за это время все ранее отправленные сообщения должны дойти до целевых акторов

    //---- Результатом действия [1] должна стать широковещаетльная отправке PREQ пакетов с декрементированным ttl
    //     по сеоидениням а так же создание маршрутной записи

    b_Rtable_UpdateRoute *msg0 = rTable->expectMsgType(rTable, m_Rtable_UpdateRoute, b_Rtable_UpdateRoute_extr2, 5000);

    assertTrue(msg0->dest == 99, "t_2_0-A");
    assertTrue(msg0->gateway == 1, "t_2_0-B");
    assertTrue(msg0->distance == 4, "t_2_0-C");

    b_Connection_SendData *bin4 = con4->expectMsgType(con4, m_Connection_SendData, b_Connection_SendData_extr2 , 5000);
    b_Connection_SendData *bin3 = con3->expectMsgType(con3, m_Connection_SendData, b_Connection_SendData_extr2 , 5000);
    b_Connection_SendData *bin2 = con2->expectMsgType(con2, m_Connection_SendData, b_Connection_SendData_extr2 , 5000);
    b_Connection_SendData *bin1 = con1->expectMsgType(con1, m_Connection_SendData, b_Connection_SendData_extr2 , 5000);
    b_Connection_SendData *bin0 = con0->expectMsgType(con0, m_Connection_SendData, b_Connection_SendData_extr2 , 5000);

    Packet *compPreqPacket = Packet_createPreqPacket(100, 99, 199, 8);
    compPreqPacket->ttl = 3;
    uint16_t *compPreqSize = pmalloc(2);
    uint8_t *compPreqBin = Packet_toBinary(compPreqPacket, compPreqSize);

    assertTrue(bin0->size == *compPreqSize, "t_2_0-A");
    assertTrue(bin1->size == *compPreqSize, "t_2_0-B");
    assertTrue(bin2->size == *compPreqSize, "t_2_0-C");
    assertTrue(bin3->size == *compPreqSize, "t_2_0-D");
    assertTrue(bin4->size == *compPreqSize, "t_2_0-E");


    for (int i = 0; i < *compPreqSize; i++) {
        if (i != 5 && i != 6 && i != 7 && i != 8) {
            assertTrue(bin0->data[i] == compPreqBin[i], "t_2_0-F");
            assertTrue(bin1->data[i] == compPreqBin[i], "t_2_0-G");
            assertTrue(bin2->data[i] == compPreqBin[i], "t_2_0-H");
            assertTrue(bin3->data[i] == compPreqBin[i], "t_2_0-I");
            assertTrue(bin4->data[i] == compPreqBin[i], "t_2_0-J");
        }
    }

    //---- Блок освобождения памяти


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
    rTable->destroy(rTable);
    pfree(rTable);
    gwPool->tell(gwPool, new_m_Stop(32), NULL, system);
    AU_freeRef(gwPool);
    trPool->tell(trPool, new_m_Stop(32), NULL, system);
    AU_freeRef(trPool);
    receiver->tell(receiver, new_m_Stop(32), NULL, system);
    AU_freeRef(receiver);
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
    pfree(bin0->data);
    pfree(bin0);
    pfree(bin1->data);
    pfree(bin1);
    pfree(bin2->data);
    pfree(bin2);
    pfree(bin3->data);
    pfree(bin3);
    pfree(bin4->data);
    pfree(bin4);
    free_Packet(compPreqPacket);
    pfree(compPreqSize);
    pfree(compPreqBin);
    pfree(msg0);

    printf("OK\n");
}

void run_preq_spec(ActorSystem *testSystem, ActorSystem *system) {
    printf("-----start preq_spec-----\n");
    PreqSpec_t_0_0(testSystem, system);
    PreqSpec_t_1_0(testSystem, system);
    PreqSpec_t_2_0(testSystem, system);
    printf("------end preq_spec------\n\n");
}