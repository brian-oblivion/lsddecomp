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
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "Entity.h"
#include "DreamSys.h"
#include "StageMap.h"

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
    long *peerPos;
    GsCOORDINATE2 *coord;
    s32 dx;
    s32 dz;

    peerPos = NULL;
    if (peer->parent != 0) {
        peerPos = peer->coord2->workm.t;
    }
    coord = this->coord2;
    dx = coord->coord.t[0] - peerPos[0];
    if (dx < 0) {
        dx = ~dx + 1; /* MATCHING: -dx compiles differently */
    }
    dz = coord->coord.t[2] - peerPos[2];
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

/* PlacementGridVabSound.c's; SoundCueSet.h does not declare it. */
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
                if (Entity__IsNearTarget(this, this->coord2->coord.t, row->activeRange,
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
                near = Entity__IsNearTarget(this, this->coord2->coord.t, row->activeRange,
                                            row->nearTolerance);
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

/* ---- merged from Entity_b ---- */

/* Entity_b: the last of Entity's own methods, and fifteen of its MoodCue
 * handlers (include/Entity.h).
 *
 * The methods. Entity__Update runs the table's last three slots every tick:
 * Entity__UpdateTargetProximity (+0x178) latches targetReached through
 * setTargetReached once the player (`peer`) is within the mood row's
 * proximityRange, Entity__UpdateSoundCueStart (+0x17C) starts the sound cue
 * when the player comes within the row's cueRange, and
 * Entity__UpdateSoundCueStop (+0x180) stops it again when the player leaves
 * that range. Entity__NotifyIfTargetInRange and Entity__IsTargetInRange are
 * the range test Entity__UpdateDeactivationState makes on the row's
 * gEntityEventVideoTable entry; Get_vtable_Entity is the table's getter.
 *
 * The handlers. Each Entity__MoodCueNN is the `handler` of
 * gEntityMoodHandlerTable's row NN: rows 0, 1, 5 and 7 to 17 (rows 2 to 4
 * and 6 have none). An Entity whose moodIndex selects the row installs it as
 * its SoundCueSet callback, so ServiceSoundCueSet calls it once per tick
 * with the Entity and its cue set. A handler requests tones by filling the
 * set's slots (a VAB program of the cue's sound object, or SOUND_CUE_STOP),
 * moves, turns and scales the entity on moodTimer (the ticks since
 * startSoundCue), on the cue set's own `tick`, or on todFrame (the frame of
 * its TOD animation), and sends the dream an EntityEffect through
 * notifyParents. Which dream object owns each row is not established.
 *
 * The literals are left unnamed where they are one handler's tuning: tick
 * counts, distances in world units, TOD frame numbers, VAB program numbers,
 * and the `state` values other than 0 and ENTITY_STATE_DONE, which are each
 * handler's own phases (same convention as Entity_c to Entity_g).
 */

s32 Entity__UpdateTargetProximity(Entity *this) {
    EntityMoodRow *row;
    long *pos;
    s32 dist;

    row = &gEntityMoodTable[this->moodIndex];
    if (this->active != 0) {
        if (this->targetReached == 0) {
            pos = this->coord2->coord.t;
            dist = row->proximityRange;
            if (dist < 0) {
                dist = ~dist + 1; /* MATCHING: -dist compiles differently */
            }
            if (Entity__IsNearTarget(this, pos, dist, row->nearTolerance) != 0) {
                this->methods->setTargetReached(this, 1);
            }
        }
        if (row->proximityRange < 0) {
            SceneNode__FaceTarget((SceneNode *)this, (SceneNode *)this->peer, 1, 0, 0);
        }
    }
    return this->targetReached;
}

s32 Entity__UpdateSoundCueStart(Entity *this) {
    EntityMoodRow *row;
    long *pos;
    s32 dist;

    if (this->active != 0 && this->soundCueActive == 0 && this->state != ENTITY_STATE_DONE) {
        row = &gEntityMoodTable[this->moodIndex];
        if (row->cueRange != 0) {
            pos = this->coord2->coord.t;
            dist = row->cueRange;
            if (dist < 0) {
                dist = ~dist + 1; /* MATCHING: -dist compiles differently */
            }
            if (Entity__IsNearTarget(this, pos, dist, row->nearTolerance) != 0) {
                this->methods->startSoundCue(this);
            }
        }
    }
    return this->soundCueActive;
}

/* For a row with no link stage (a negative gEntityLinkStageTable entry) and
 * an event video, sends ENTITY_EFFECT_LINK_STAGE while the player is within
 * the video entry times 512 world units (Entity__IsTargetInRange). `unused`
 * is Entity.h's declared second parameter; the one caller,
 * Entity__UpdateDeactivationState, passes 0. */
void Entity__NotifyIfTargetInRange(Entity *this, s32 unused) {
    if (gEntityLinkStageTable[this->moodIndex * 16] < 0 &&
        gEntityEventVideoTable[this->moodIndex * 16] != 0 &&
        Entity__IsTargetInRange(this, gEntityEventVideoTable[this->moodIndex * 16] << 9)) {
        this->methods->notifyParents(this, ENTITY_EFFECT_LINK_STAGE);
    }
}

/* 1 when the player is within 512 world units of this entity's height and
 * distanceToPeer (|dx| + |dz|) is below `range`. */
s32 Entity__IsTargetInRange(Entity *this, s32 range) {
    TodActor *other;
    s32 oy, ty;

    other = this->peer;
    oy = other->coord2->coord.t[1];
    ty = this->coord2->coord.t[1];
    /* MATCHING: two ifs and a goto; one || with plain returns compiles differently */
    if (oy + 512 < ty) {
        goto fail;
    }
    if (ty < oy - 512) {
        goto fail;
    }
    if (this->methods->distanceToPeer(this, other) < range) {
        return 1;
    }
fail:
    return 0;
}

s32 Entity__UpdateSoundCueStop(Entity *this) {
    EntityMoodRow *row;
    long *pos;
    s32 dist;

    if (this->active != 0 && this->soundCueActive != 0) {
        row = &gEntityMoodTable[this->moodIndex];
        dist = row->cueRange;
        if (dist < 0) {
            dist = ~dist + 1; /* MATCHING: -dist compiles differently */
            pos = this->coord2->coord.t;
            if (Entity__IsNearTarget(this, pos, dist, row->nearTolerance) == 0) {
                this->methods->stopSoundCue(this);
            }
        }
    }
    return this->soundCueActive;
}

EntityMethods *Get_vtable_Entity(void) {
    return &gEntityMethods;
}

void Entity__MoodCue00(Entity *this, SoundCueSet *out) {
    if (out->tick == 0) {
        if (((DreamSys *)this->peer)->methods->getDreamColor((DreamSys *)this->peer) == DREAM_COLOR_PINK) {
            this->state = 100;
        }
    }
    out->attenuation = this->methods->getProximityRatio(this);
    if (this->state == 0) {
        if (out->tick % 10 == 0) {
            out->slots[0].program = 5;
            out->slots[0].octave = -2;
        }
        if (this->moodTimer == 2400) {
            this->moodTimer = -1; /* Entity__TickSoundCue counts it back to 0: the cycle restarts */
        } else if (this->moodTimer < 1200) {
            this->methods->moveLocalZ(this, 50, 0);
        } else {
            this->methods->moveLocalZ(this, -50, 0);
        }
    } else if (this->moodTimer < 250) {
        if (out->tick % 10 == 0) {
            out->slots[0].program = 5;
            out->slots[0].octave = -2;
        }
        if (this->moodTimer < 100) {
            this->methods->moveLocalZ(this, 50, 0);
        } else if (this->moodTimer < 250) {
            this->methods->addTranslation(this, TRANSLATE_Y_PLUS64_Z_MINUS64);
        }
    } else if (this->moodTimer == 250) {
        this->methods->stopTod(this);
        out->slots[0].program = SOUND_CUE_STOP;
    } else if (this->moodTimer >= 261 && this->moodTimer < 568) {
        this->methods->moveLocalZ(this, -50, 0);
        this->methods->updateRotation(this, 1, ROTATION_YAW_MINUS120);
    } else if (this->moodTimer >= 569) {
        this->methods->updateRotation(this, 1, ROTATION_X50_YMINUS120_Z30);
    }
}

void Entity__MoodCue01(Entity *this, SoundCueSet *out) {
    out->attenuation = 0;
    if (out->tick == 0) {
        out->slots[0].program = 20;
        out->slots[1].program = 20;
        out->slots[2].program = 20;
        ((DreamSys *)this->peer)->methods->clearTickCallbacks((DreamSys *)this->peer, 1);
    }
    SceneNode__FaceTarget((SceneNode *)this, (SceneNode *)this->peer, 1, 0, 0);
    this->methods->moveLocalZ(this, -90, 0);
    if (this->moodTimer == 30) {
        this->methods->notifyParents(this, ENTITY_EFFECT_LINK_STAGE);
    }
}

void Entity__MoodCue05(Entity *this, SoundCueSet *out) {
    out->attenuation = this->methods->getProximityRatio(this);
    if (out->tick == 0) {
        out->slots[0].program = 23;
    }
}

void Entity__MoodCue07(Entity *this, SoundCueSet *out) {
    out->attenuation = this->methods->getProximityRatio(this);
    if (this->todFrame == this->todFrameCount / 2) {
        out->slots[0].program = 7;
        out->slots[0].octave = -2;
        out->slots[1].program = 3;
        out->slots[1].octave = -2;
    }
    if (out->tick % 90 < 3) {
        out->slots[2].program = 6;
        out->slots[2].octave = -1;
    }
    if (this->moodTimer >= 121) {
        this->methods->updateRotation(this, 0, ROTATION_YAW_PLUS2);
        this->methods->moveLocalZ(this, -320, 0);
    } else if (this->moodTimer >= 56 || Entity__IsNearTarget(this, this->coord2->coord.t, 1, 1) != 0) {
        this->methods->addTranslation(this, TRANSLATE_Y_MINUS64);
    } else if (this->moodTimer >= 10) {
        SceneNode__FaceTarget((SceneNode *)this, (SceneNode *)this->peer, 1, 0, 0);
        this->methods->moveLocalZ(this, -256, 0);
    } else {
        SceneNode__FaceTarget((SceneNode *)this, (SceneNode *)this->peer, 1, 0, 0);
    }
}

void Entity__MoodCue08(Entity *this) {
    this->methods->updateScale(this, 1, SCALE_DOUBLE);
    this->methods->addTranslation(this, TRANSLATE_Y_MINUS64);
}

void Entity__MoodCue09(Entity *this, SoundCueSet *out) {
    out->attenuation = this->methods->getProximityRatio(this);
    if (out->tick % (this->todFrameCount / 2) == 0) {
        out->slots[0].program = 10;
    }
    this->methods->moveLocalZ(this, -30, 0);
}

void Entity__MoodCue10(Entity *this, SoundCueSet *out) {
    out->attenuation = 0;
    if (out->tick == 0) {
        out->slots[0].program = 11;
        out->slots[1].program = 11;
        out->slots[2].program = 11;
    }
    this->methods->moveLocalZ(this, -30, 0);
}

void Entity__MoodCue11(Entity *this, SoundCueSet *out) {
    Ratio16 *turn;

    this->lastOffsetValue = -20;
    out->attenuation = this->methods->getProximityRatio(this);
    turn = NULL;
    if (out->tick % (this->todFrameCount / 2) == 0) {
        out->slots[0].program = 10;
        out->slots[0].octave = 1;
    }
    if (this->state == 11) {
        if (this->moodTimer == 2700) {
            turn = ROTATION_YAW_MINUS90;
        }
        if (this->moodTimer == 3180) {
            turn = ROTATION_YAW_PLUS90;
        }
        if (this->moodTimer == 3600) {
            turn = ROTATION_YAW_MINUS90;
        }
        if (this->moodTimer >= 3421 && this->moodTimer < 3541) {
            if (((DreamSys *)this->peer)->methods->getLinkCommandFlag((DreamSys *)this->peer) != 0) {
                this->moodTimer = 0;
                this->state = 13;
            }
        }
    } else if (this->state == 12) {
        if (this->moodTimer == 1980) {
            turn = ROTATION_YAW_MINUS90;
        }
    } else if (this->state == 13) {
        this->lastOffsetValue = -120;
        SceneNode__FaceTarget((SceneNode *)this, (SceneNode *)this->peer, 1, 0, 0);
        this->methods->updateScale(this, 1, SCALE_HALF);
        if (this->methods->distanceToPeer(this, this->peer) < 1024) {
            this->methods->notifyParents(this, ENTITY_EFFECT_EVENT_VIDEO);
        }
    }
    if (this->moodTimer == 1560) {
        if ((rand() & 1) != 0) {
            turn = ROTATION_YAW_PLUS90;
            this->state = 11;
        } else {
            turn = ROTATION_YAW_MINUS90;
            this->state = 12;
        }
    }
    if (turn != NULL) {
        this->methods->updateRotation(this, 0, turn);
    }
    this->methods->moveLocalZOrFindLink(this, this->lastOffsetValue, 0);
    if (this->state != 12) {
        if (this->linkTarget != 0) {
            this->methods->moveLocalY(this, -200, 0);
        }
    }
}

void Entity__MoodCue12(Entity *this) {
    s32 y;
    s32 dist;
    s32 timer;

    if (this->moodTimer == 0) {
        if ((rand() & 1) == 0) {
            this->state = 11;
        }
    }
    y = this->coord2->coord.t[1];
    if (y < 2000) {
        SceneNode__FaceTarget((SceneNode *)this, (SceneNode *)this->peer, 1, 0, 0);
    }
    if (this->state == 11) {
        dist = this->methods->distanceToPeer(this, this->peer);
        if (dist < 2560) {
            this->grid->methods->startScaleRamp(this->grid, 1, 1);
            this->moodTimer = 1;
            this->state = 12;
        }
    } else if (this->state == 12) {
        /* Counted here as well as by Entity__TickSoundCue: two per tick. */
        timer = this->moodTimer;
        this->moodTimer = timer + 1;
        if (timer == 300) {
            this->methods->notifyParents(this, ENTITY_EFFECT_END_DREAM);
        }
    }
}

void Entity__MoodCue13(Entity *this, SoundCueSet *out) {
    out->attenuation = this->methods->getProximityRatio(this);
    if (out->tick == 0) {
        out->slots[0].program = 12;
        this->state++;
    } else if (out->tick >= this->todFrameCount - 1) {
        out->tick = -1; /* the cue's tick restarts every todFrameCount ticks */
    }
    if (this->state == 36) {
        if (rand() % 3 == 0) {
            this->methods->notifyParents(this, ENTITY_EFFECT_EVENT_VIDEO);
        }
    }
}

void Entity__MoodCue14(Entity *this, SoundCueSet *out) {
    out->attenuation = this->methods->getProximityRatio(this);
    if (this->todFrame == 10) {
        out->slots[0].program = 13;
    }
    this->methods->moveLocalZ(this, -10, 0);
}

void Entity__MoodCue15(Entity *this, SoundCueSet *out) {
    if (this->moodTimer == 0) {
        out->attenuation = 0;
        out->slots[0].program = 15;
    }
}

void Entity__MoodCue16(Entity *this) {
    Ratio16 *turn;
    s32 roll;

    if (this->moodTimer == 0) {
        if ((rand() & 1) != 0) {
            this->state = 11;
        }
    }
    if (this->state == 0) {
        if (this->moodTimer < 64) {
            this->methods->moveLocalZ(this, -90, 0);
        } else if (this->moodTimer == 64) {
            roll = rand() & 1;
            turn = ROTATION_YAW_MINUS90;
            if (roll != 0) {
                turn = ROTATION_YAW_PLUS90;
            }
            this->methods->updateRotation(this, 0, turn);
            this->methods->addTranslation(this, TRANSLATE_Y_PLUS256);
        } else {
            this->methods->moveLocalZOrFindLink(this, -374, (void *)(rand() % 2));
        }
    } else if (this->state == 11) {
        if (this->moodTimer % 5 == 0) {
            this->methods->updateRotation(this, 0, ROTATION_YAW_PLUS90);
        }
        this->methods->moveLocalZ(this, -2048, 0);
        this->methods->setDisplay(this, (rand() % 7) == 0);
    }
}

void Entity__MoodCue17(Entity *this) {
    this->methods->updateScale(this, 1, SCALE_HALF);
}
