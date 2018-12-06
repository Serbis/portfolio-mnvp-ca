//
// Created by serbis on 07.07.18.
//

#include "actor_system.h"
#include "actor_cell.h"
#include "router.h"

#ifndef MNVP_DRIVER_ACTOR_UTILS_H
#define MNVP_DRIVER_ACTOR_UTILS_H

ActorSystem* AU_system(Actor* actor);
void* AU_fsm_data(Actor* actor);
void AU_destroyActor(ActorCell *cell, bool fromMailBox);
ActorRef* AU_copyRef(ActorRef *ref);
Router* AU_copyRouter(Router *router);
void AU_freeRef(ActorRef *ref);
void AU_freeMsg(ActorMessage *msg, bool destruct);
ActorMessage* AU_copyMsg(ActorMessage* msg, void* body);

#endif //MNVP_DRIVER_ACTOR_UTILS_H
