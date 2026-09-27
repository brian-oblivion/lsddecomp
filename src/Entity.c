/* Entity: the class's construction and per-tick logic (include/Entity.h).
 * Entity_b holds its last three slots and the table getter, and Entity_b to
 * Entity_g the MoodCue handlers.
 *
 * An Entity is a TodActor driven by one row of gEntityMoodTable, chosen by
 * New_Entity's moodIndex. This file holds:
 *  - construction and teardown: New_Entity, Entity__Entity, Entity__Reset
 *    (fog for unlockKind 1 to 9, tick callback B, start inactive),
 *    Entity__Finalize, and Entity__GetOrCreateFadeBox, the screen fade some
 *    handlers run;
 *  - attaching: Entity__AttachToParent activates the entity at once when its
 *    row has no activation condition, and then starts its sound cue when the
 *    row has no cue range; Entity__DetachFromParent deactivates it;
 *  - the tick: Entity__Update runs the activation, deactivation, sound-cue and
 *    proximity slots, then TodActor's update. Entity__UpdateActivationState and
 *    Entity__UpdateDeactivationState test the row's activateKind and
 *    deactivateKind against Entity__IsNearTarget (the player's projected
 *    position against this entity's, in ENTITY_RANGE_UNITs);
 *  - link commands: Entity__NotifyLinkStage passes them to TodActor's
 *    handler and, on event 4, sends the row's EntityEffect to its parents;
 *    Entity__OnGridCellLinkCommand deactivates on event 4;
 *  - the sound cue: Entity__StartSoundCue installs the row's handler on
 *    soundCueSet, Entity__TickSoundCue services it and counts moodTimer,
 *    Entity__StopSoundCue flushes it;
 *  - the getters the dream reads when an Entity sends an effect: the row
 *    itself, its unlock score, link stage and event video, and
 *    Entity__GetProximityRatio, the sound attenuation step for the player's
 *    distance.
 */
#include "common.h"
#include "Entity.h"
#include "DreamSys.h"

Entity *New_Entity(s32 moodIndex, void *desc, void *sound) {
    Entity *obj;

    obj = BMemPMgrAlloc(sizeof(Entity));
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

    cached = this->fadeBox; /* MATCHING: `cached`, `boxMethods` and `attachOffset` are each load-bearing */
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
    if (kind >= 1 && kind <= 9) {
        this->methods->setLightMode(this, 1); /* fog on (GsFOG) */
    }
    this->methods->selectTickCallback(this, TICK_CALLBACK_B);
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
    if (gEntityMoodTable[this->moodIndex].activateKind != 0) {
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

/* A row with no link stage (0 or below) drops events 2 to 8 and sends no
 * effect. */
void Entity__NotifyLinkStage(Entity *this, void *sender, s32 event) {
    s32 linkStage;

    linkStage = gEntityMoodTable[this->moodIndex].linkStage;
    if (event >= 2 && event <= 8) {
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
    /* MATCHING: the effect reuses `event`; a local of its own compiles differently */
    if (linkStage != ENTITY_LINK_STAGE_END_DREAM) {
        event = ENTITY_EFFECT_LINK_STAGE;
    } else if (gEntityMoodTable[this->moodIndex].eventVideo != 0) {
        event = ENTITY_EFFECT_EVENT_VIDEO;
    } else {
        event = ENTITY_EFFECT_END_DREAM;
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

/* 1 when `pos` (plus unlockKind * 1024 in y for an unlockKind of -9 to -1)
 * lies within `tolerance` of the point `range` ahead of the player
 * (DreamSys__ProjectPointAtDistance). Both are in ENTITY_RANGE_UNITs; a
 * negative tolerance -n means ENTITY_RANGE_UNIT / n. */
s32 Entity__IsNearTarget(Entity *this, void *pos, s32 range, s32 tolerance) {
    LongVec3 point;
    s32 kind;

    point = *(LongVec3 *)pos;
    kind = (u8)gEntityMoodTable[this->moodIndex].unlockKind;
    if ((s8)kind >= -9 && (s8)kind < 0) {
        point.y += (s8)kind * 1024;
    }
    if (tolerance < 0) {
        tolerance = ENTITY_RANGE_UNIT / (~tolerance + 1); /* MATCHING: -tolerance compiles differently */
    } else {
        tolerance <<= ENTITY_RANGE_SHIFT;
    }
    return ((DreamSys *)this->peer)
        ->methods->projectPointAtDistance((DreamSys *)this->peer, 0, range << ENTITY_RANGE_SHIFT,
                                          (s32 *)&point, tolerance);
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
        dx = ~dx + 1; /* MATCHING: -dx compiles differently */
    }
    dz = coord->tz - peerPos[2];
    return (dz >= 0) ? (dx + dz) : (dx - dz);
}

/* -1 without a peer or beyond the row's proximityThreshold; otherwise the
 * player's distance in steps of threshold / attenuationSteps, 0 nearest. */
s32 Entity__GetProximityRatio(Entity *this) {
    s32 result;
    s32 threshold;

    do { /* MATCHING: a bare if compiles one word differently */
        if (this->peer == NULL) {
            return -1;
        }
    } while (0);
    result = this->methods->distanceToPeer(this, this->peer);
    threshold = gEntityMoodTable[this->moodIndex].proximityThreshold << ENTITY_RANGE_SHIFT;
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

/* The stage index a linkStage names: n - 1 for a positive n, ~n for a
 * negative one (so -1 is stage 0). */
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
        this->methods->notifyParents(this, ENTITY_EFFECT_LOG_MOOD);
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

    if (this->active == 0 && this->state != ENTITY_STATE_DONE) {
        row = &gEntityMoodTable[this->moodIndex];
        doActivate = 0;
        if (row->activateKind != ENTITY_ACTIVATE_AT_ATTACH) {
            if (row->activateKind == ENTITY_ACTIVATE_RANDOM) {
                goto randCheck;
            }
            if (row->activeRange != 0) {
                if (Entity__IsNearTarget(this, &this->coord2->tx, row->activeRange,
                                         row->nearTolerance) != 0) {
                    if (row->activateKind == ENTITY_ACTIVATE_NEAR) {
                        doActivate = 1;
                    } else if (row->activateKind == ENTITY_ACTIVATE_NEAR_RANDOM) {
                        goto randCheck;
                    }
                } else if (row->activateKind == ENTITY_ACTIVATE_FAR) {
                    doActivate = 1;
                }
            }
        }
        goto merge; /* MATCHING: randCheck placed after this block, reached by goto */

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

    if (this->active != 0) {
        row = &gEntityMoodTable[this->moodIndex];
        doDeactivate = 0;
        Entity__NotifyIfTargetInRange(this, 0);
        if (row->deactivateKind != ENTITY_DEACTIVATE_NONE &&
            row->deactivateKind != ENTITY_DEACTIVATE_NONE_ALT) {
            if (row->deactivateKind >= ENTITY_DEACTIVATE_TIMED) {
                if (this->tick == row->deactivateKind * 15) {
                    doDeactivate = 1;
                }
            } else if (row->activeRange != 0) {
                near = Entity__IsNearTarget(this, &this->coord2->tx, row->activeRange, row->nearTolerance);
                if (near != 0) {
                    if (row->deactivateKind == ENTITY_DEACTIVATE_NEAR) {
                        doDeactivate = 1;
                    }
                } else if (row->deactivateKind == ENTITY_DEACTIVATE_FAR) {
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
