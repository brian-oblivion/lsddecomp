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
 * TodActor `arg2`/soundCueSet pair), the active-flag toggle
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
 * The overrides of TodActor's slots are named for their slots
 * (Entity__Finalize, Reset, AttachToParent, DetachFromParent,
 * OnGridCellLinkCommand; track 4, round 88).
 */
#include "common.h"
#include "Entity.h"
#include "DreamSys.h"

Entity *New_Entity(s32 moodIndex, void *desc, void *sound) {
    Entity *obj;

    obj = BMemPMgrAlloc(0x108);
    if (obj == NULL) {
        return NULL;
    }
    if (Get_vtable_Entity()->ctor(obj, moodIndex, desc, sound) == NULL) {
        BMemPMgrFree(obj);
        return NULL;
    }
    return obj;
}

Entity *Entity__Entity(Entity *this, s32 moodIndex, void *desc, void *sound) {
    if (GetTodActorMethods()->ctor((TodActor *)this, desc, sound) != NULL) {
        this->methods = Get_vtable_Entity();
        this->moodIndex = moodIndex;
        this->soundCueSet.tag = 0;
        this->fadeBox = NULL;
        this->unk104 = NULL;
        this->methods->reset(this);
        return this;
    }
    return NULL;
}

FadeBox *Entity__GetOrCreateFadeBox(Entity *this, void *size, void *offset, void *step, s32 pri) {
    FadeBox *cached;
    FadeBox *box;
    FadeBoxMethods *boxMethods;
    void *attachOffset;

    cached = this->fadeBox;
    if (cached == NULL) {
        if (size == NULL) {
            size = gEntityFadeBoxDefaultSize;
        }
        box = New_FadeBox(size, 0, pri);
        if (box == NULL) {
            return NULL;
        }
        this->fadeBox = box;
    } else {
        box = cached;
    }
    box->methods->detachFromParent(box);
    boxMethods = box->methods;
    attachOffset = offset;
    if (attachOffset == NULL) {
        attachOffset = gEntityFadeBoxDefaultOffset;
    }
    boxMethods->attachToParent(box, (SceneNode *)this, attachOffset);
    box->methods->setStep(box, (s32)step);
    return box;
}

void Entity__Finalize(Entity *this) {
    if (this->fadeBox != NULL) {
        this->fadeBox->methods->release(this->fadeBox);
    }
    if (this->unk104 != NULL) {
        this->unk104->methods->release(this->unk104);
    }
    GetTodActorMethods()->finalize((TodActor *)this);
}

void Entity__Reset(Entity *this) {
    s32 kind;

    kind = (u8)gEntityMoodTable[this->moodIndex].unlockKind;
    if ((u32)(kind - 1) < 9) {
        this->methods->setLightMode(this, 1);
    }
    this->methods->selectTickCallback(this, 0x42);
    this->methods->deactivate(this);
}

void Entity__AttachToParent(Entity *this, TodActor *peer, void *companion, struct StageMap *parent,
                            void *offset) {
    if (this->parent != 0) {
        return;
    }
    ((TodActorAttachToParentFn)GetTodActorMethods()->attachToParent)((TodActor *)this, peer,
                                                                     companion, parent, offset);
    this->grid = parent;
    if (gEntityMoodTable[this->moodIndex].detachKind != 0) {
        return;
    }
    this->methods->activate(this);
    if (gEntityMoodTable[this->moodIndex].cueRange != 0) {
        return;
    }
    this->methods->startSoundCue(this);
}

void Entity__DetachFromParent(Entity *this) {
    if (this->parent != 0) {
        this->methods->deactivate(this);
        GetTodActorMethods()->detachFromParent((TodActor *)this);
        this->grid = NULL;
    }
}

void Entity__Update(Entity *this, void *sender, s32 event) {
    if (this->methods->updateActivationState(this) != 0) {
        this->methods->updateDeactivationState(this);
    }
    if (this->methods->updateSoundCueStart(this) != 0) {
        this->methods->updateSoundCueStop(this);
    }
    this->methods->updateTargetProximity(this);
    GetTodActorMethods()->update((TodActor *)this, sender, event);
}

void Entity__NotifyLinkStage(Entity *this, void *sender, s32 event) {
    s32 linkStage;

    linkStage = gEntityMoodTable[this->moodIndex].linkStage;
    if ((u32)(event - 2) < 7) {
        if (linkStage <= 0) {
            return;
        }
    }
    GetTodActorMethods()->onActorLinkCommand((TodActor *)this, sender, event);
    if (event != 4) {
        return;
    }
    if (linkStage <= 0) {
        return;
    }
    if (linkStage != 0x7F) {
        event = 0xA;
    } else if (gEntityMoodTable[this->moodIndex].eventVideo != 0) {
        event = 0xB;
    } else {
        event = 0xC;
    }
    this->methods->notifyParents(this, event);
}

void Entity__OnGridCellLinkCommand(Entity *this, void *sender, s32 event) {
    GetTodActorMethods()->onGridCellLinkCommand((TodActor *)this, sender, event);
    if (event == 4) {
        this->methods->deactivate(this);
    }
}

void Entity__TickSoundCue(Entity *this) {
    ServiceSoundCueSet(this->sound, &this->soundCueSet);
    this->moodTimer++;
}

s32 Entity__IsNearTarget(Entity *this, void *pos, s32 range, s32 tolerance) {
    LongVec3 point;
    s32 kind;

    point = *(LongVec3 *)pos;
    kind = (u8)gEntityMoodTable[this->moodIndex].unlockKind;
    if ((u32)((kind + 9) & 0xFF) < 9) {
        point.y += (s8)kind * 1024;
    }
    if (tolerance < 0) {
        tolerance = 0x800 / (~tolerance + 1);
    } else {
        tolerance <<= 11;
    }
    return ((DreamSys *)this->peer)
        ->methods->projectPointAtDistance((DreamSys *)this->peer, 0, range << 11, (s32 *)&point,
                                          tolerance);
}

s32 Entity__DistanceToPeer(Entity *this, TodActor *peer) {
    s32 *peerPos;
    SceneNodeSub14 *coord;
    s32 dx;
    s32 dz;

    peerPos = NULL;
    if (peer->parent != 0) {
        peerPos = peer->coord2->unk38;
    }
    coord = this->coord2;
    dx = coord->tx - peerPos[0];
    if (dx < 0) {
        dx = ~dx + 1;
    }
    dz = coord->tz - peerPos[2];
    return (dz >= 0) ? (dx + dz) : (dx - dz);
}

s32 Entity__GetProximityRatio(Entity *this) {
    s32 result;
    s32 threshold;

    do {
        if (this->peer == NULL) {
            return -1;
        }
    } while (0);
    result = this->methods->distanceToPeer(this, this->peer);
    threshold = gEntityMoodTable[this->moodIndex].proximityThreshold << 11;
    if (threshold < result) {
        return -1;
    }
    return result / (threshold / this->soundCueSet.attenuationSteps);
}

EntityMoodRow *Entity__GetMoodEffect(Entity *this) {
    return &gEntityMoodTable[this->moodIndex];
}

s32 Entity__GetUnlockEffect(Entity *this) {
    return gEntityMoodTable[this->moodIndex].unlockKind * 1000;
}

s32 Entity__GetLinkStage(Entity *this) {
    s32 linkStage = gEntityMoodTable[this->moodIndex].linkStage;

    if (linkStage < 0) {
        return ~linkStage;
    }
    return linkStage - 1;
}

s32 Entity__GetEventVideo(Entity *this) {
    return gEntityMoodTable[this->moodIndex].eventVideo - 1;
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

void Entity__SetTargetReached(Entity *this, s32 reached) {
    if (reached != 0) {
        this->methods->notifyParents(this, 9);
    }
    this->targetReached = reached;
}

/* code_179d8_e.c's; SoundCueSet.h does not declare it. */
extern s32 InitSoundCueSet(struct VabStreamObj *sound, SoundCueSet *set, s32 tag, void *owner,
                           SoundCueCallbackFn callback);

void Entity__StartSoundCue(Entity *this) {
    InitSoundCueSet(this->sound, &this->soundCueSet, this->moodIndex + 1, this,
                    gEntityMoodTable[this->moodIndex].handler);
    ((EntityPlayTodFn)this->methods->playTod)(this);
    this->methods->enableTickCallback(this);
    this->moodTimer = 0;
    this->soundCueActive = 1;
}

void Entity__StopSoundCue(Entity *this) {
    FlushSoundCueSet(this->sound, &this->soundCueSet);
    this->methods->stopTod(this);
    this->methods->disableTickCallback(this);
    this->soundCueActive = 0;
}

s32 Entity__UpdateActivationState(Entity *this) {
    EntityMoodRow *row;
    s32 doActivate;

    if (this->active == 0 && this->state != 1) {
        row = &gEntityMoodTable[this->moodIndex];
        doActivate = 0;
        if (row->detachKind != 0) {
            if (row->detachKind == 4) {
                goto randCheck;
            }
            if (row->activeRange != 0) {
                if (Entity__IsNearTarget(this, &this->coord2->tx, row->activeRange,
                                         row->nearTolerance) != 0) {
                    if (row->detachKind == 1) {
                        doActivate = 1;
                    } else if (row->detachKind == 3) {
                        goto randCheck;
                    }
                } else if (row->detachKind == 2) {
                    doActivate = 1;
                }
            }
        }
        goto merge;

    randCheck:
        if ((rand() & 0x7F) == 0) {
            doActivate = 1;
        }

    merge:
        if (doActivate) {
            this->methods->activate(this);
        }
    }
    return this->active;
}

s32 Entity__UpdateDeactivationState(Entity *this) {
    EntityMoodRow *row;
    s32 doDeactivate;
    s32 near;
    s32 tick;

    if (this->active != 0) {
        row = &gEntityMoodTable[this->moodIndex];
        doDeactivate = 0;
        Entity__NotifyIfTargetInRange(this, 0);
        if (row->linkKind != 0 && row->linkKind != 3) {
            if (row->linkKind >= 10) {
                tick = this->tick;
                if ((tick ^ (row->linkKind * 15)) == 0) {
                    doDeactivate = 1;
                }
            } else if (row->activeRange != 0) {
                near = Entity__IsNearTarget(this, &this->coord2->tx, row->activeRange, row->nearTolerance);
                if (near != 0) {
                    if (row->linkKind == 1) {
                        doDeactivate = 1;
                    }
                } else if (row->linkKind == 2) {
                    doDeactivate = 1;
                }
            }
        }
        if (doDeactivate) {
            this->methods->deactivate(this);
        }
    }
    return this->active;
}
