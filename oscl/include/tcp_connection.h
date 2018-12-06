//
// Created by serbis on 06.07.18.
//

#ifndef MNVP_DRIVER_TCP_CONNECTION_H
#define MNVP_DRIVER_TCP_CONNECTION_H

#include "../../libs/miniakka/includes/messages.h"
#include "../../libs/miniakka/includes/actor.h"
#include "../include/connection.h"
#include "rec_thread.h"


Actor* new_TcpConnectionActor(RecThreadArgs *tArgs);

#endif //MNVP_DRIVER_TCP_CONNECTION_H
