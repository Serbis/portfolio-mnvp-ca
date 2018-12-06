#include <zconf.h>
#include <string.h>
#include "../includes/actor_ref.h"
#include "../includes/actor_cell.h"
#include "../../../test/unit/include/acceptor_spec.h"
#include "../includes/actor_utils.h"

void LocalActorRef_tell(ActorRef* this, ActorMessage* msg, ActorRef *sender, ActorSystem *system) {
    MutexLock(system->mutex);
    bool actorIsAlive = MAP_contain(this->actorName, system->actors);

    if (actorIsAlive) {
        ActorCell *cell = (ActorCell *) this->actorCell;
        MutexLock(cell->mutexTop);

        if (!cell->stopped) {
            if (sender != NULL)
                msg->sender = AU_copyRef(sender);
            else
                msg->sender = NULL;
            ((ActorCell *) this->actorCell)->sendMessage(this->actorCell, msg);
        }

        MutexUnlock(cell->mutexTop);
    } else {
        AU_freeMsg(msg, true);
    }
    MutexUnlock(system->mutex);
}


ActorRef* new_LocalActorRef(void* actor, void* dispatcher, char *actorName) {
    ActorRef *ref = pmalloc(sizeof(ActorRef));
    if (actor == NULL || dispatcher == NULL) {
        return ref;
    } else {
        size_t actorNameLen = strlen(actorName) + 1;
        char *dup_actorName = pmalloc(actorNameLen);
        memcpy(dup_actorName, actorName, actorNameLen);

        ref->actorCell = new_ActorCell(actor, dispatcher);
        ref->actorName = dup_actorName;
        ref->tell =  LocalActorRef_tell;
    }

    return ref;
}