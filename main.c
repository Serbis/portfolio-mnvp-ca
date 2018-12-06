#include <stdio.h>
#include <stdint.h>
#include <zconf.h>
#include "oscl/include/malloc.h"
#include "libs/miniakka/includes/executor.h"
#include "libs/miniakka/includes/dispatcher.h"
#include "libs/miniakka/includes/actor_system.h"
#include "oscl/include/time.h"
#include "oscl/include/a_tpcio.h"
#include "test/unit/include/acceptor_spec.h"
#include "libs/miniakka/includes/simple_actor.h"
#include "libs/miniakka/includes/standard_messages.h"
#include "test/unit/include/gateway_spec.h"
#include "include/packet.h"
#include "oscl/include/utils.h"
#include "libs/miniakka/includes/router.h"
#include "test/unit/include/connector_spec.h"
#include "libs/collections/includes/map.h"
#include "test/unit/include/gateway_pool_spec.h"
#include "libs/collections/includes/map2.h"
#include "test/unit/include/rtable_spec.h"
#include "test/unit/include/core_spec.h"
#include "test/func/include/core_hello_spec.h"
#include "test/unit/include/receiver_spec.h"
#include "test/func/include/recv_hello_spec.h"
#include "libs/miniakka/includes/actor_utils.h"
#include "test/unit/include/transaction_pool_spec.h"
#include "libs/miniakka/includes/simple_fsm.h"
#include "test/unit/include/preq_transaction_spec.h"
#include "test/func/include/preq_spec.h"
#include "test/unit/include/send_packet_fsm_spec.h"
#include "test/func/include/send_packet_spec.h"
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main() {
    uint16_t *tVector = pmalloc(5 * 2);
    tVector[0] = 128;
    tVector[1] = 256;
    tVector[2] = 512;
    tVector[3] = 1024;
    tVector[4] = 2048;


    Executor *executor = EXECUTOR_New(50, tVector, 1, "ex1-");
    Dispatcher* dispatcher = new_Dispatcher(executor);
    ActorSystem* system = new_ActorSystem(dispatcher);

    uint16_t *tVector2 = pmalloc(10 * 2);
    tVector2[0] = 128;
    tVector2[1] = 256;
    tVector2[2] = 512;
    tVector2[3] = 1024;
    tVector2[4] = 2048;
    tVector2[5] = 128;
    tVector2[6] = 256;
    tVector2[7] = 512;
    tVector2[8] = 1024;
    tVector2[9] = 2048;

    Executor *executor2 = EXECUTOR_New(50, tVector2, 10, "ex-2");
    Dispatcher* dispatcher2 = new_Dispatcher(executor2);
    ActorSystem* testSystem = new_ActorSystem(dispatcher2);


    for (int i = 0; i < 1; i++) {
        //run_acceptor_spec(system); //НЕ ТРОГАЙ ЭТОТ ТЕСТ, ОН БИТЫЙ!!!

        run_gateway_spec(testSystem, system); //Этот тест частичный, см. комментарий в тесте
        run_connector_spec(testSystem, system);
        run_gateway_pool_spec(testSystem, system);
        run_rtable_spec(testSystem, system);
        run_core_spec(testSystem, system);
        run_receiver_spec(testSystem, system);
        run_transaction_pool_spec(testSystem, system);
        run_preq_transaction_spec(testSystem, system);
        run_send_packet_fsm_spec(testSystem, system);

        run_core_hello_spec(testSystem, system);
        run_recv_hello_spec(testSystem, system);
        run_preq_spec(testSystem, system);
        run_send_packet_spec(testSystem, system);

        printf("------------------------------%d\n", i);
    }

    sleep(5);


    pfree(tVector);
    pfree(tVector2);

    return 0;
}