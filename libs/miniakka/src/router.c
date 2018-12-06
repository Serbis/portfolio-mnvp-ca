#include "../includes/router.h"
#include "../includes/actor_system.h"

void Router_routeRoundRobin(Router *this, ActorMessage *msg, ActorRef *sender, ActorSystem *system) {
    RoundRobinRoutingLogic *logic = (RoundRobinRoutingLogic*) this->routingLogic;
    ActorRef *routee = (ActorRef*) this->routees->get(this->routees, logic->nextRoutee);
    routee->tell(routee, msg, sender, system);
    if (logic->nextRoutee == this->routees->size - 1)
        logic->nextRoutee = 0;
    else
        logic->nextRoutee = (uint8_t) (logic->nextRoutee + 1);
}


void Router_route(Router *this, ActorMessage *msg, ActorRef *sender, ActorSystem *system) {
    MutexLock(this->mutex);
    switch (this->routingLogicType) {
        case ROUND_ROBIN_LOGIC:
            Router_routeRoundRobin(this, msg, sender, system);
            break;
        default:break;
    }
    MutexUnlock(this->mutex);
}

RoundRobinRoutingLogic* new_RoundRobinRoutingLogic() {
    RoundRobinRoutingLogic *logic = pmalloc(sizeof(logic));
    logic->nextRoutee = 0;

    return logic;
}

Router* new_Router(uint8_t routingLogicType, void *routingLogic, List *routees) {
    Router *router = pmalloc(sizeof(Router));
    router->routingLogicType = routingLogicType;
    router->routingLogic = routingLogic;
    router->routees = routees;
    router->mutex = NewMutex();
    router->route = Router_route;

    return router;
}