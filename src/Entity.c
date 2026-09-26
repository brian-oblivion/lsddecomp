/* The Entity class -- fully matched, no INCLUDE_ASM left (round 56 was a
 * track 3 naming pass, not matching work). This is the first 25 of a
 * 142-function block split at Entity__UpdateTargetProximity; the rest (Entity_b through
 * Entity_g, all sharing include/Entity.h) hold the mood-dispatch handler
 * tables and the per-frame behaviour those handlers run.
 *
 * This unit itself covers Entity's own construction/destruction
 * (New_Entity/Entity__Entity/Entity__Finalize), its per-tick dispatcher
 * (Entity__Update, chaining through most of EntityMethods), the sound-cue
 * lifecycle (Entity__StartSoundCue/TickSoundCue/StopSoundCue on the
 * Class65650 `arg2`/soundCueSet pair), the active-flag toggle
 * (Entity__Activate/Deactivate and the two functions that decide whether to
 * fire them, Entity__UpdateActivationState/DeactivationState), and four
 * small getters over the moodIndex-selected per-mood tables
 * (Entity__GetMoodEffect/GetUnlockEffect/GetLinkStage/GetEventVideo).
 *
 * Entity's own vtable is gEntityMethods (asm/data/79528.data.s), reached via
 * Get_vtable_Entity (Entity_b.c); `tools/classtable.py gEntityMethods` is
 * the ground truth for which function occupies which slot, including the
 * several self-referential slots this unit's own functions dispatch back
 * into (activate/deactivate/getProximityRatio/startSoundCue/stopSoundCue).
 * The overrides of Class65650's slots are named for their slots
 * (Entity__Finalize, Reset, AttachToParent, DetachFromParent,
 * OnClass86AA0LinkCommand; track 4, round 88).
 */
#include "common.h"
#include "Entity.h"
#include "DreamSys.h"

Entity *New_Entity(s32 moodIndex, void *desc, void *arg2) {
    Entity *obj;

    obj = BMemPMgrAlloc(0x108);
    if (obj == NULL) {
        return NULL;
    }
    if (Get_vtable_Entity()->ctor(obj, moodIndex, desc, arg2) == NULL) {
        BMemPMgrFree(obj);
        return NULL;
    }
    return obj;
}

Entity *Entity__Entity(Entity *this, s32 moodIndex, void *desc, void *arg2) {
    if (Get_vtable_Class65650()->ctor((Class65650 *)this, desc, arg2) != NULL) {
        this->methods = Get_vtable_Entity();
        this->moodIndex = moodIndex;
        this->soundCueSet.tag = 0;
        this->unk100 = NULL;
        this->unk104 = NULL;
        this->methods->reset(this);
        return this;
    }
    return NULL;
}

Class6E99C *Entity__GetOrCreateUnk100(Entity *this, void *name, void *arg2, void *arg3, s32 arg4) {
    Class6E99C *cached;
    Class6E99C *sub;
    Class6E99CMethods *m;
    void *dispatchArg2;

    cached = this->unk100;
    if (cached == NULL) {
        if (name == NULL) {
            name = gEntityDefaultPos;
        }
        sub = New_Class6E99C(name, 0, arg4);
        if (sub == NULL) {
            return NULL;
        }
        this->unk100 = sub;
    } else {
        sub = cached;
    }
    sub->methods->detachFromParent(sub);
    m = sub->methods;
    dispatchArg2 = arg2;
    if (dispatchArg2 == NULL) {
        dispatchArg2 = gEntityDefaultOffset;
    }
    m->attachToParent(sub, (Class6B5CC *)this, dispatchArg2);
    sub->methods->setStep(sub, (s32)arg3);
    return sub;
}

void Entity__Finalize(Entity *this) {
    if (this->unk100 != NULL) {
        this->unk100->methods->release(this->unk100);
    }
    if (this->unk104 != NULL) {
        this->unk104->methods->release(this->unk104);
    }
    Get_vtable_Class65650()->finalize((Class65650 *)this);
}

void Entity__Reset(Entity *this) {
    s32 kind;

    kind = ((u8 *)gEntityUnlockKindTable)[this->moodIndex * 0x10];
    if ((u32)(kind - 1) < 9) {
        this->methods->setLightMode(this, 1);
    }
    this->methods->selectTickCallback(this, 0x42);
    this->methods->deactivate(this);
}

void Entity__AttachToParent(Entity *this, Class65650 *peer, void *companion,
                            struct Class866E8 *parent, void *offset) {
    if (this->parent != 0) {
        return;
    }
    ((Class65650AttachToParentFn)Get_vtable_Class65650()->attachToParent)((Class65650 *)this, peer,
                                                                          companion, parent, offset);
    this->grid = parent;
    if (D_80089EA7[this->moodIndex * 0x10] != 0) {
        return;
    }
    this->methods->activate(this);
    if (D_80089EAF[this->moodIndex * 0x10] != 0) {
        return;
    }
    this->methods->startSoundCue(this);
}

void Entity__DetachFromParent(Entity *this) {
    if (this->parent != 0) {
        this->methods->deactivate(this);
        Get_vtable_Class65650()->detachFromParent((Class65650 *)this);
        this->grid = NULL;
    }
}

void Entity__Update(Entity *this, void *a1, s32 a2) {
    if (this->methods->updateActivationState(this) != 0) {
        this->methods->updateDeactivationState(this);
    }
    if (this->methods->updateSoundCueStart(this) != 0) {
        this->methods->updateSoundCueStop(this);
    }
    this->methods->updateTargetProximity(this);
    Get_vtable_Class65650()->update((Class65650 *)this, a1, a2);
}

void Entity__NotifyLinkStage(Entity *this, void *arg1, s32 arg2) {
    s32 linkStage;

    linkStage = gEntityLinkStageTable[this->moodIndex * 0x10];
    if ((u32)(arg2 - 2) < 7) {
        if (linkStage <= 0) {
            return;
        }
    }
    Get_vtable_Class65650()->onActorLinkCommand((Class65650 *)this, arg1, arg2);
    if (arg2 != 4) {
        return;
    }
    if (linkStage <= 0) {
        return;
    }
    if (linkStage != 0x7F) {
        arg2 = 0xA;
    } else if (gEntityEventVideoTable[this->moodIndex * 0x10] != 0) {
        arg2 = 0xB;
    } else {
        arg2 = 0xC;
    }
    this->methods->notifyParents(this, arg2);
}

void Entity__OnClass86AA0LinkCommand(Entity *this, void *a1, s32 a2) {
    Get_vtable_Class65650()->onClass86AA0LinkCommand((Class65650 *)this, a1, a2);
    if (a2 == 4) {
        this->methods->deactivate(this);
    }
}

void Entity__TickSoundCue(Entity *this) {
    ServiceSoundCueSet(this->arg2, &this->soundCueSet);
    this->moodTimer++;
}

typedef struct EntityVec3 EntityVec3;

struct EntityVec3 {
    s32 x;
    s32 y;
    s32 z;
};

s32 Entity__IsNearTarget(Entity *this, void *pos, s32 arg2, s32 arg3) {
    EntityVec3 local;
    s32 kind;

    local = *(EntityVec3 *)pos;
    kind = ((u8 *)gEntityUnlockKindTable)[this->moodIndex * 0x10];
    if ((u32)((kind + 9) & 0xFF) < 9) {
        local.y += (s8)kind * 1024;
    }
    if (arg3 < 0) {
        arg3 = 0x800 / (~arg3 + 1);
    } else {
        arg3 <<= 11;
    }
    return ((DreamSys *)this->peer)
        ->methods->projectPointAtDistance((DreamSys *)this->peer, 0, arg2 << 11, (s32 *)&local, arg3);
}

s32 Entity__DistanceToPeer(Entity *this, Class65650 *peer) {
    s32 *world;
    Class6B5CCSub14 *pos;
    s32 dx;
    s32 dz;

    world = NULL;
    if (peer->parent != 0) {
        world = peer->coord2->unk38;
    }
    pos = this->coord2;
    dx = pos->tx - world[0];
    if (dx < 0) {
        dx = ~dx + 1;
    }
    dz = pos->tz - world[2];
    return (dz >= 0) ? (dx + dz) : (dx - dz);
}

s32 Entity__GetProximityRatio(Entity *this) {
    s32 result;
    Entity *self;
    s32 threshold;

    do {
        if (this->peer == NULL) {
            return -1;
        }
    } while (0);
    self = this;
    result = this->methods->distanceToPeer(this, self->peer);
    threshold = gEntityProximityThresholdTable[self->moodIndex * 0x10] << 11;
    if (threshold < result) {
        return -1;
    }
    return result / (threshold / self->soundCueSet.unk14);
}

EntityMoodRow *Entity__GetMoodEffect(Entity *this) {
    return &gEntityMoodTable[this->moodIndex];
}

s32 Entity__GetUnlockEffect(Entity *this) {
    return gEntityUnlockKindTable[this->moodIndex * 0x10] * 1000;
}

s32 Entity__GetLinkStage(Entity *this) {
    s32 linkStage = gEntityLinkStageTable[this->moodIndex * 0x10];

    if (linkStage < 0) {
        return ~linkStage;
    }
    return linkStage - 1;
}

s32 Entity__GetEventVideo(Entity *this) {
    return gEntityEventVideoTable[this->moodIndex * 0x10] - 1;
}

void Entity__Activate(Entity *this) {
    this->methods->setDisplay(this, 1);
    this->active = 1;
    this->tick = 0;
}

void Entity__Deactivate(Entity *this) {
    this->methods->setDisplay(this, 0);
    this->methods->stopSoundCue(this);
    this->methods->setTargetReached(this, 0);
    this->active = 0;
}

void Entity__SetTargetReached(Entity *this, s32 arg1) {
    if (arg1 != 0) {
        this->methods->notifyParents(this, 9);
    }
    this->targetReached = arg1;
}

typedef struct EntityMoodHandlerRow EntityMoodHandlerRow;

struct EntityMoodHandlerRow {
    void *handler; /* +0x00 */
    u8 pad04[0x10 - 0x04];
};

extern EntityMoodHandlerRow gEntityMoodHandlerTable[];
extern void InitSoundCueSet(void *sound, void *set, s32 tag, Entity *owner, void *callback);

void Entity__StartSoundCue(Entity *this) {
    InitSoundCueSet(this->arg2, &this->soundCueSet, this->moodIndex + 1, this,
                    gEntityMoodHandlerTable[this->moodIndex].handler);
    ((EntityPlayTodFn)this->methods->playTod)(this);
    this->methods->enableTickCallback(this);
    this->moodTimer = 0;
    this->soundCueActive = 1;
}

void Entity__StopSoundCue(Entity *this) {
    FlushSoundCueSet(this->arg2, &this->soundCueSet);
    this->methods->stopTod(this);
    this->methods->disableTickCallback(this);
    this->soundCueActive = 0;
}

s32 Entity__UpdateActivationState(Entity *this) {
    EntityMoodRow *row;
    s32 doDetach;

    if (this->active == 0 && this->state != 1) {
        row = &gEntityMoodTable[this->moodIndex];
        doDetach = 0;
        if (row->detachKind != 0) {
            if (row->detachKind == 4) {
                goto randCheck;
            }
            if (row->unk5 != 0) {
                if (Entity__IsNearTarget(this, &this->coord2->tx, row->unk5, row->unk9) != 0) {
                    if (row->detachKind == 1) {
                        doDetach = 1;
                    } else if (row->detachKind == 3) {
                        goto randCheck;
                    }
                } else if (row->detachKind == 2) {
                    doDetach = 1;
                }
            }
        }
        goto merge;

    randCheck:
        if ((rand() & 0x7F) == 0) {
            doDetach = 1;
        }

    merge:
        if (doDetach) {
            this->methods->activate(this);
        }
    }
    return this->active;
}

s32 Entity__UpdateDeactivationState(Entity *this) {
    EntityMoodRow *row;
    s32 doDetach;
    s32 dist;
    s32 scaled;

    if (this->active != 0) {
        row = &gEntityMoodTable[this->moodIndex];
        doDetach = 0;
        Entity__NotifyIfTargetInRange(this, 0);
        if (row->linkKind != 0 && row->linkKind != 3) {
            if (row->linkKind >= 10) {
                scaled = this->tick;
                if ((scaled ^ (row->linkKind * 15)) == 0) {
                    doDetach = 1;
                }
            } else if (row->unk5 != 0) {
                dist = Entity__IsNearTarget(this, &this->coord2->tx, row->unk5, row->unk9);
                if (dist != 0) {
                    if (row->linkKind == 1) {
                        doDetach = 1;
                    }
                } else if (row->linkKind == 2) {
                    doDetach = 1;
                }
            }
        }
        if (doDetach) {
            this->methods->deactivate(this);
        }
    }
    return this->active;
}
