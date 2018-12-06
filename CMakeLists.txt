cmake_minimum_required(VERSION 3.7)
project(mnvp_driver C)

set(CMAKE_C_STANDARD 99)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)

set(SOURCE_FILES
        main.c
        libs/miniakka/includes/executor.h
        libs/miniakka/src/executor.c
        libs/collections/includes/lbq.h
        libs/collections/src/lbq.c
        libs/collections/includes/rings.h
        libs/collections/src/rings.c
        oscl/include/malloc.h
        oscl/src/malloc.c
        libs/collections/includes/list.h
        libs/collections/src/list.c
        libs/collections/includes/colnode.h
        oscl/include/threads.h
        oscl/src/threads.c
        libs/miniakka/includes/actor.h
        libs/miniakka/includes/messages.h
        libs/miniakka/includes/actor_cell.h
        libs/miniakka/includes/mailbox.h
        libs/miniakka/includes/actor_system.h
        libs/miniakka/includes/actor_ref.h
        libs/miniakka/includes/dispatcher.h
        libs/miniakka/src/actor_system.c
        libs/miniakka/src/dispatcher.c
        libs/miniakka/src/messages.c
        libs/miniakka/includes/simple_actor.h
        libs/miniakka/src/actor_ref.c
        libs/miniakka/src/actor_cell.c
        libs/miniakka/src/simple_actor.c
        libs/miniakka/src/mailbox.c
        libs/miniakka/includes/actor_context.h
        libs/miniakka/src/actor_context.c
        libs/miniakka/includes/testprobe.h
        libs/miniakka/src/testprobe.c
        oscl/include/time.h
        oscl/src/time.c
        libs/miniakka/src/actor.c
        oscl/include/a_tpcio.h
        oscl/src/a_tcpio.c
        oscl/include/a_tcp_connection_creator.h
        oscl/src/a_tcp_connection_creator.c
        oscl/include/ba_therad.h
        oscl/src/ba_thread.c
        oscl/include/tcp_connection.h
        oscl/src/tcp_connection.c
        oscl/include/rec_thread.h
        oscl/src/rec_thread.c
        test/integra/include/tcpio_spec.h
        test/integra/src/tcpio_spec.c
        test/integra/include/test_utils.h
        test/integra/src/tcp_utils.c
        libs/miniakka/includes/actor_utils.h
        libs/miniakka/src/actor_utils.c
        oscl/include/connection.h
        oscl/src/connection.c
        include/a_acceptor.h
        include/a_gateway.h
        include/a_gateway_pool.h
        src/a_acceptor.c
        src/a_gateway.c
        src/a_gateway_pool.c
        test/unit/include/acceptor_spec.h
        test/unit/src/acceptor_spec.c
        libs/miniakka/includes/standard_messages.h
        libs/miniakka/src/standard_messages.c
        libs/miniakka/includes/timers.h
        libs/miniakka/includes/scheduler.h
        libs/miniakka/src/timers.c
        libs/miniakka/src/scheduler.c
        include/packet.h src/packet.c
        test/unit/include/gateway_spec.h
        test/unit/src/gateway_spec.c
        include/global_messages.h
        src/global_messages.c
        oscl/include/utils.h
        oscl/src/utils.c libs/miniakka/includes/router.h libs/miniakka/src/router.c include/a_receiver.h src/a_receiver.c include/a_rtable.h src/a_rtable.c include/a_connector.h src/a_connector.c include/a_heart.h src/a_haert.c test/unit/include/connector_spec.h libs/cunit/include/asserts.h libs/cunit/src/asserts.c test/unit/src/connector_spec.c libs/collections/includes/map.h libs/collections/src/map.c test/unit/include/gateway_pool_spec.h test/unit/src/gateway_pool_spec.c oscl/include/data.h oscl/src/data.c libs/collections/includes/map2.h libs/collections/src/map2.c test/unit/include/rtable_spec.h test/unit/src/rtable_spec.c include/a_core.h src/a_core.c test/unit/include/core_spec.h test/unit/src/core_spec.c test/func/include/core_hello_spec.h test/func/src/core_hello_spec.c test/unit/include/receiver_spec.h test/unit/src/receiver_spec.c test/func/include/recv_hello_spec.h test/func/src/recv_hello_spec.c include/a_transaction_pool.h src/a_transaction_pool.c include/transactions/transaction_commands.h src/transactions/transaction_commands.c test/unit/include/transaction_pool_spec.h test/unit/src/transaction_pool_spec.c libs/miniakka/includes/fsm.h libs/miniakka/src/fsm.c libs/miniakka/includes/simple_fsm.h libs/miniakka/src/simple_fsm.c include/transactions/a_preq_transaction.h src/transactions/a_preq_transaction.c test/unit/include/preq_transaction_spec.h test/unit/src/preq_transaction_spec.c test/func/include/preq_spec.h test/func/src/preq_spec.c include/a_send_packet_fsm.h src/a_send_packet_fsm.c test/unit/include/send_packet_fsm_spec.h test/unit/src/send_packet_fsm_spec.c test/extractors/include/extractors.h test/extractors/src/extractors.c test/func/include/send_packet_spec.h test/func/src/send_packet_spec.c)

add_executable(mnvp_driver ${SOURCE_FILES})

find_package(Threads)
target_link_libraries(mnvp_driver ${CMAKE_THREAD_LIBS_INIT})