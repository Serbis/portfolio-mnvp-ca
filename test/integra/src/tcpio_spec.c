#include <zconf.h>
#include <sys/socket.h>
#include <stdlib.h>
#include "../include/tcpio_spec.h"
#include "../../../libs/miniakka/includes/actor_system.h"
#include "../../../libs/miniakka/includes/testprobe.h"
#include "../../../oscl/include/a_tpcio.h"
#include "../include/test_utils.h"
#include "../../../oscl/include/a_tcp_connection_creator.h"
#include "../../../oscl/include/tcp_connection.h"



//-- tpcio must
//---- When accept correct BindAndAccept command must Create new TcpConnection
/*void t_0_0(ActorSystem *system) {
    TestProbe *probe = new_TestProbe(system, "PROBE_0_0/0");
    ActorRef *target = system->actorOf(system, new_TcpIOActor());
    probe->send(probe, target, new_m_TcpIO_BindAndAcceptMsg("127.0.0.1", 5000, 0));
    sleep(1);
    int socket = TestUtils_createTcpConnection("127.0.0.1", "5000");
    probe->expectMsgType(probe, r_TcpConnectionCreator_Connection, NULL, 3000);
    close(socket);

    printf("tcpio_spec [t_0_0] completed\n");
}*/

//---- When accept incorrect BindAndAccept command must return BindingError
/*void t_0_1(ActorSystem *system) {
    TestProbe *probe = new_TestProbe(system, "PROBE_0_1/0");
    ActorRef *target = system->actorOf(system, new_TcpIOActor());
    probe->send(probe, target, new_m_TcpIO_BindAndAcceptMsg("127.0.0.1", 1, 0));
    probe->expectMsgType(probe, r_TcpConnectionCreator_BindFailed, NULL, 3000);
    printf("tcpio_spec [t_0_1] completed\n");
}*/

//-- TcpConnection must
//---- Write data to connection
/*void t_1_0(ActorSystem *system) {
    TestProbe *probe = new_TestProbe(system, "PROBE_1_0/0");
    ActorRef *target = system->actorOf(system, new_TcpIOActor());
    probe->send(probe, target, new_m_TcpIO_BindAndAcceptMsg("127.0.0.1", 5000, 0));
    sleep(1);
    int socket = TestUtils_createTcpConnection("127.0.0.1", "5000");
    b_TcpConnectionCreator_Connection *b = probe->expectMsgType(probe, r_TcpConnectionCreator_Connection, sizeof(ActorRef), 3000);
    ActorRef *conn = b->ref;
    uint8_t *data = pmalloc(3);
    *data = 'a';
    *(data + 1) = 'b';
    *(data + 2) = 'c';

    conn->tell(conn, new_m_Connection_SendData(data, 3, 64), NULL);
    pfree(data);
    data = pmalloc(3);

    int numbytes;
    if ((numbytes = (uint16_t) recv(socket, data, 32000, 0)) == -1) {
        printf("t_1_0 connection closed\n");
        exit(1);
    }

    if (numbytes != 3) {
        printf("t_1_0 incorrect recv size\n");
        exit(1);
    }

    if (*data != 'a') {
        printf("t_1_0 data + 0 != 'a'\n");
        exit(1);
    }
    if (*(data + 1) != 'b') {
        printf("t_1_0 data + 1 != 'b'\n");
        exit(1);
    }
    if (*(data + 2) != 'c') {
        printf("t_1_0 data + 2 != 'c'\n");
        exit(1);
    }

    pfree(data);
    printf("tcpio_spec [t_1_0] completed\n");
}*/

//---- Read data from connection
/*void t_1_1(ActorSystem *system) {
    TestProbe *probe = new_TestProbe(system, "PROBE_1_1/0");
    TestProbe *dataHandler = new_TestProbe(system, "PROBE_1_1/1");
    ActorRef *target = system->actorOf(system, new_TcpIOActor());
    probe->send(probe, target, new_m_TcpIO_BindAndAcceptMsg("127.0.0.1", 5000, 0));
    sleep(1);
    int socket = TestUtils_createTcpConnection("127.0.0.1", "5000");
    b_TcpConnectionCreator_Connection *b = probe->expectMsgType(probe, r_TcpConnectionCreator_Connection, sizeof(ActorRef), 3000);
    ActorRef *conn = b->ref;
    conn->tell(conn, new_m_Connection_SetDataHandler(dataHandler->ref, 64), NULL);
    uint8_t *data = pmalloc(3);
    *data = 'a';
    *(data + 1) = 'b';
    *(data + 2) = 'c';
    send(socket, data, 3, 0);
    b_Connection_Data *rData = dataHandler->expectMsgType(dataHandler, r_Connection_Data, sizeof(b_Connection_Data), 3000);
    if (rData->size != 3) {
        printf("t_1_0 incorrect recv size\n");
        exit(1);
    }

    if (*rData->data != 'a') {
        printf("t_1_1 data + 0 != 'a'\n");
        exit(1);
    }
    if (*(rData->data + 1) != 'b') {
        printf("t_1_1 data + 1 != 'b'\n");
        exit(1);
    }
    if (*(rData->data + 2) != 'c') {
        printf("t_1_1 data + 2 != 'c'\n");
        exit(1);
    }

    pfree(data);
    pfree(rData);
    pfree(probe);
    pfree(dataHandler);
    pfree(target);
    close(socket);
    pfree(conn);

    printf("tcpio_spec [t_1_1] completed\n");
}*/

//---- Close connection by Close command and respond with ConnectionClosed
/*void t_1_2(ActorSystem *system) {
    TestProbe *probe = new_TestProbe(system, "PROBE_1_1/0");
    TestProbe *dataHandler = new_TestProbe(system, "PROBE_1_1/1");
    ActorRef *target = system->actorOf(system, new_TcpIOActor());
    probe->send(probe, target, new_m_TcpIO_BindAndAcceptMsg("127.0.0.1", 5000, 0));
    sleep(1);
    int socket = TestUtils_createTcpConnection("127.0.0.1", "5000");
    b_TcpConnectionCreator_Connection *b = probe->expectMsgType(probe, r_TcpConnectionCreator_Connection, sizeof(ActorRef), 3000);
    ActorRef *conn = b->ref;
    conn->tell(conn, new_m_Connection_SetDataHandler(dataHandler->ref, 64), NULL);
    conn->tell(conn, new_m_Connection_Close(64), NULL);
    dataHandler->expectMsgType(dataHandler, r_Connection_ConnectionClosed, 0, 3000);
    printf("tcpio_spec [t_1_2] completed\n");
}*/

//---- Send ConnectionClosed to handler if connection will be closed from network side
/*void t_1_3(ActorSystem *system) {
    TestProbe *probe = new_TestProbe(system, "PROBE_1_1/0");
    TestProbe *dataHandler = new_TestProbe(system, "PROBE_1_1/1");
    ActorRef *target = system->actorOf(system, new_TcpIOActor());
    probe->send(probe, target, new_m_TcpIO_BindAndAcceptMsg("127.0.0.1", 5000, 0));
    sleep(1);
    int socket = TestUtils_createTcpConnection("127.0.0.1", "5000");
    b_TcpConnectionCreator_Connection *b = probe->expectMsgType(probe, r_TcpConnectionCreator_Connection, sizeof(ActorRef), 3000);
    ActorRef *conn = b->ref;
    conn->tell(conn, new_m_Connection_SetDataHandler(dataHandler->ref, 64), NULL);
    close(socket);
    dataHandler->expectMsgType(dataHandler, r_Connection_ConnectionClosed, 0, 30000);
    printf("tcpio_spec [t_1_3] completed\n");
}*/

void run_tcpio_spec(ActorSystem *system) {
    //t_0_0(system);
    //t_0_1(system);
    //t_1_0(system);
    //t_1_1(system);
    //t_1_2(system);
   // t_1_3(system);
}