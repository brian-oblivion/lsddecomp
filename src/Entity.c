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
#include "Viewport.h"

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

/* ---- merged from Entity_c ---- */

/* Third slice of the Entity block -- 20 functions, 0x4F754..0x5077C. The
 * remainder is `Entity_d` and is still a monolithic asm segment.
 *
 * The whole 97-function remainder this came out of has zero `jlabel`s and
 * zero `jr $t2`, so no slice of it needs a rodata slot attached and none of
 * it is a BIOS trampoline. Entity/Entity's `include/Entity.h` is already
 * heavily typed and these functions are the same class family -- extend that
 * header rather than starting a new one.
 *
 * All 20 functions are `gEntityMoodHandlerTable` mood-dispatch callbacks,
 * `Entity__MoodCueNN` where NN is the table row (`asm/data/79528.data.s`,
 * stride 0x10) -- same family and naming convention as Entity/_d/_e/_g.
 * Row order does not track code address, so this unit's rows (19-27, 29-38,
 * plus 119) are not contiguous with each other or with source order;
 * `Entity__MoodCue119` sits far from its neighbours by address alone,
 * confirmed against the table rather than assumed from proximity.
 * `Entity__MoodCue30` additionally occupies row 122 with the same handler
 * and different data words -- one function shared by two distinct mood-row
 * configurations, named for its lower row (same precedent as
 * `Entity__MoodCue81`, Entity_e).
 *
 * Fields and slots are the unified Entity's (include/Entity.h): the
 * inherited ones carry TodActor's, Actor's and SceneNode's names (`state`,
 * `linkTarget`, `peer`, moveLocalZ/X/Y, moveLocalZOrFindLink, ...), Entity's
 * own are named for their occupants.
 *
 * The literals are left unnamed where they are one handler's tuning: tick
 * counts, distances in world units, TOD frame numbers, VAB program numbers,
 * and the `state` values other than 0 and ENTITY_STATE_DONE, which are each
 * handler's own phases (same convention as Entity_d/Entity_e).
 */

void Entity__MoodCue19(Entity *this, SoundCueSet *out) {
    out->attenuation = this->methods->getProximityRatio(this);
    if (out->tick % 10 == 0) {
        out->slots[0].program = 17;
    }
    this->methods->moveLocalZ(this, -256, 0);
}

void Entity__MoodCue20(Entity *this, SoundCueSet *out) {
    if (this->moodTimer == 0 && rand() % 7 == 0) {
        this->methods->updateScale(this, 1, SCALE_Y2);
    }
    if ((out->tick & 3) == 0) {
        out->attenuation = this->methods->getProximityRatio(this);
        out->slots[0].program = 28;
    }
    this->methods->moveLocalZ(this, -100, 0);
}

void Entity__MoodCue21(Entity *this, SoundCueSet *out) {
    s32 half;
    s32 rem;

    out->attenuation = this->methods->getProximityRatio(this);
    half = this->todFrameCount / 2;
    rem = out->tick % half;
    if (rem == 0) {
        out->slots[0].program = 10;
    } else if (rem == 3) {
        out->slots[1].program = 13;
    }
    this->methods->moveLocalZ(this, -30, (void *)1);
}

void Entity__MoodCue22(Entity *this) {
    SceneNode__FaceTarget((SceneNode *)this, (SceneNode *)this->peer, 1, 0, 0);
}

void Entity__MoodCue23(Entity *this) {
    s32 zDelta;

    SceneNode__FaceTarget((SceneNode *)this, (SceneNode *)this->peer, 1, 0, 0);

    if (this->targetReached != 0) {
        if (this->moodTimer >= 65) {
            this->moodTimer = 0;
        }
        if (this->moodTimer >= 7) {
            this->methods->addTranslation(this, TRANSLATE_Y_MINUS64);
            this->methods->moveLocalZ(this, 10, 0);
        } else {
            this->methods->addTranslation(this, TRANSLATE_X_MINUS64);
        }
    } else if (this->moodTimer == 0) {
        this->methods->addTranslation(this, TRANSLATE_Y_MINUS4096);
    } else if (this->moodTimer < 65) {
        this->methods->addTranslation(this, TRANSLATE_Y_PLUS64);
    } else if (this->moodTimer < 71) {
        this->methods->addTranslation(this, TRANSLATE_Y_MINUS64);
        this->methods->moveLocalZ(this, -30, 0);
    } else {
        EntityMethods *methods = this->methods;

        if (this->moodTimer < 256) {
            zDelta = -this->moodTimer - 65;
        } else {
            zDelta = 255;
        }
        methods->moveLocalZ(this, zDelta, 0);
    }
}

void Entity__MoodCue24(Entity *this, SoundCueSet *out) {
    if (out->tick % 15 == 0) {
        out->attenuation = this->methods->getProximityRatio(this);
        out->slots[0].program = 7;
        out->slots[0].octave = -2;
    }
    this->methods->updateScale(this, 1, SCALE_DOUBLE);
    this->methods->updateRotation(this, 0, ROTATION_YAW_PLUS2);
    this->methods->moveLocalZ(this, -512, 0);
}

void Entity__MoodCue25(Entity *this, SoundCueSet *out) {
    out->attenuation = 0;
    if (this->moodTimer < 100) {
        if (out->tick % 3 == 0) {
            out->slots[0].program = 22;
            out->slots[0].octave = 1;
        }
        this->methods->moveLocalY(this, -64, 0);
    } else if (this->moodTimer < 300) {
        out->slots[0].program = 12;
        out->slots[0].octave = -1;
        out->slots[1].program = 12;
        out->slots[1].octave = -1;
        out->slots[2].program = 12;
        out->slots[2].octave = -1;
        this->methods->moveLocalY(this, -256, 0);
    } else {
        this->methods->updateRotation(this, 0, ROTATION_XPLUS_EIGHTH);
        this->methods->moveLocalY(this, -512, 0);
    }
}

void Entity__MoodCue26(Entity *this, SoundCueSet *out) {
    s32 v1;
    s32 zDelta;
    void (**moveZOrFindLink)(Entity *self, s32 val, void *notify);

    /* The do/while(0) wrapper is a no-op scoping device, load-bearing for
     * register allocation only -- see the match report. Without it GCC
     * swaps which callee-saved register holds `this` vs `out` for the
     * whole function. */
    do {
        if (out->tick % this->todFrameCount == 0) {
            out->attenuation = this->methods->getProximityRatio(this);
            out->slots[0].program = 26;
            /* Keeps the `li` of v1 = 110 below the out->slots[0].program store; without it
             * GCC schedules it above the out->attenuation store, right after the call. */
            __asm__("");
            v1 = 110;
            goto compare;
        }
    } while (0);
    v1 = 110;
compare:
    moveZOrFindLink = &this->methods->moveLocalZOrFindLink;
    zDelta = -384;
    if (this->moodTimer == v1) {
        zDelta = -11520;
    }
    (*moveZOrFindLink)(this, zDelta, 0);
}

void Entity__MoodCue27(Entity *this, SoundCueSet *out) {
    if (out->tick % 70 == 0) {
        out->attenuation = 0;
        out->slots[0].program = 27;
    }
    this->methods->moveLocalZ(this, -128, 0);
    if (this->moodTimer < 100) {
        this->methods->moveLocalY(this, 32, 0);
    } else if (this->moodTimer >= 301) {
        this->methods->moveLocalY(this, -32, 0);
    }
}

void Entity__MoodCue119(Entity *this) {
    this->methods->updateScale(this, 1, SCALE_SIX);
}

void Entity__MoodCue29(Entity *this) {
    if (this->moodTimer == 0) {
        if (((DreamSys *)this->peer)->methods->getDreamColor((DreamSys *)this->peer) == DREAM_COLOR_WHITE) {
            this->methods->updateScale(this, 1, SCALE_TRIPLE);
            this->methods->moveLocalY(this, -30720, 0);
        }
        this->state = rand() % 5;
    }
    if (this->state == 0) {
        this->methods->updateRotation(this, 0, ROTATION_YAW_PLUS1);
    }
}

void Entity__MoodCue30(Entity *this) {
    if (this->state == 0) {
        if (((DreamSys *)this->peer)->methods->getDreamColor((DreamSys *)this->peer) == 1) {
            this->state = 11;
        } else {
            this->state = 12;
        }
    }

    if (this->state == 12) {
        this->methods->updateScale(this, 1, SCALE_DOUBLE);
        this->methods->moveLocalY(this, -30, 0);
    } else {
        SceneNode__FaceTarget((SceneNode *)this, (SceneNode *)this->peer, 1, 0, 0);
        if (this->state == 11) {
            this->methods->moveLocalZ(this, -100, 0);
            if ((u32)(this->moodTimer - 85) < 30) {
                this->methods->moveLocalY(this, 80, 0);
            } else if (this->moodTimer == 120) {
                this->state = 13;
            }
        } else if (this->state == 13) {
            this->methods->setTranslation(this, (LongVec3 *)((DreamSys *)this->peer)->coord2->coord.t);
            this->methods->addTranslation(this, TRANSLATE_Y_MINUS1500_Z_PLUS1024);
        }
    }
}

void Entity__MoodCue31(Entity *this, SoundCueSet *out) {
    SceneNode__FaceTarget((SceneNode *)this, (SceneNode *)this->peer, 1, 0, 0);
    ((DreamSys *)this->peer)->methods->clearTickCallbacks((DreamSys *)this->peer, 1);
    if ((out->tick % 10) < 3) {
        out->attenuation = 0;
        out->slots[0].program = 13;
        out->slots[1].program = 13;
        out->slots[2].program = 13;
    }
    if (this->moodTimer == this->todFrameCount) {
        this->methods->stopTod(this);
        this->methods->notifyParents(this, ENTITY_EFFECT_LINK_STAGE);
    }
}

void Entity__MoodCue32(Entity *this) {
    this->methods->moveLocalZ(this, -30, 0);
}

void Entity__MoodCue33(Entity *this, SoundCueSet *out) {
    if (this->targetReached != 0) {
        this->methods->updateScale(this, 1, SCALE_QUARTER);
    } else if (out->tick % 30 == 0) {
        out->attenuation = 0;
        out->slots[0].program = 3;
    }
    this->methods->moveLocalZOrFindLink(this, -30, 0);
    if (this->linkTarget != 0) {
        this->methods->moveLocalY(this, -200, 0);
    }
}

void Entity__MoodCue34(Entity *this) {
    EntityMethods *methods;
    s32 zDelta;

    if ((u32)(this->moodTimer - 400) < 10) {
        this->methods->updateRotation(this, 0, ROTATION_YAW_PLUS9);
    } else if ((u32)(this->moodTimer - 700) < 10) {
        this->methods->updateRotation(this, 0, ROTATION_YAW_MINUS9);
    } else if ((u32)(this->moodTimer - 830) < 4) {
        this->methods->updateRotation(this, 0, ROTATION_YAW_MINUS9);
    } else if (this->moodTimer >= 851) {
        this->methods->deactivate(this);
    }
    methods = this->methods;
    zDelta = -512;
    if (this->moodTimer < 800) {
        zDelta = -60;
    }
    methods->moveLocalZ(this, zDelta, (void *)1);
}

void Entity__MoodCue35(Entity *this) {
    s32 rem500;
    s32 arg1a;
    s32 arg1b;
    s32 arg1c;
    void (**moveY)(Entity *self, s32 val, void *notify);
    void (**moveX)(Entity *self, s32 val, void *notify);
    void (**moveZ)(Entity *self, s32 val, void *notify);

    rem500 = this->moodTimer % 500;

    moveY = &this->methods->moveLocalY;
    if (this->moodTimer % 6 < 3) {
        arg1a = -64;
    } else {
        arg1a = 64;
    }
    (*moveY)(this, arg1a, 0);

    moveX = &this->methods->moveLocalX;
    if (this->moodTimer % 12 < 6) {
        arg1b = -64;
    } else {
        arg1b = 64;
    }
    (*moveX)(this, arg1b, 0);

    moveZ = &this->methods->moveLocalZ;
    if (this->moodTimer % 64 < 32) {
        arg1c = -128;
    } else {
        arg1c = 128;
    }
    (*moveZ)(this, arg1c, 0);

    if (rem500 < 32) {
        this->methods->addTranslation(this, TRANSLATE_Y_MINUS64);
    } else if (rem500 < 64) {
        this->methods->addTranslation(this, TRANSLATE_Y_PLUS64);
    }
}

void Entity__MoodCue36(Entity *this) {
    Ratio16 *scaleTemplate;
    s32 roll;

    if (this->state == 0) {
        roll = rand();
        scaleTemplate = SCALE_SIX;
        if ((roll & 1) != 0) {
            scaleTemplate = SCALE_DOUBLE;
        }
        this->methods->updateScale(this, 1, scaleTemplate);
        this->state = 11;
    }
    SceneNode__FaceTarget((SceneNode *)this, (SceneNode *)this->peer, 1, 0, 0);
    if (this->methods->distanceToPeer(this, this->peer) < 28672) {
        this->methods->moveLocalZ(this, 256, 0);
    }
}

void Entity__MoodCue37(Entity *this) {
    this->methods->moveLocalY(this, -90, 0);
}

void Entity__MoodCue38(Entity *this, SoundCueSet *out) {
    if (out->tick % 120 == 0) {
        out->attenuation = this->methods->getProximityRatio(this);
        out->slots[0].program = 1;
    }
}

/* ---- merged from Entity_d ---- */

/* Entity_d: nineteen of Entity's MoodCue handlers and the helper two of
 * them share.
 *
 * Each Entity__MoodCueNN is the `handler` of gEntityMoodHandlerTable's row
 * NN (include/Entity.h): rows 39 to 52, 55 to 58 and 115. An Entity whose
 * moodIndex selects the row installs it as its SoundCueSet callback, so
 * ServiceSoundCueSet calls it once per tick with the Entity and its cue
 * set. A handler requests tones by filling the set's slots (a VAB program
 * of the cue's sound object, or SOUND_CUE_STOP), moves and turns the
 * entity (or the player, its `peer`) on moodTimer, the ticks since
 * startSoundCue, or on todFrame, the frame of its TOD animation, and sends
 * the dream an EntityEffect through notifyParents. Entity__MoodCue45 is
 * empty: its row has no per-tick effect.
 *
 * Entity__RollScaleOrDelayedDrift is not a row: Entity__MoodCue43 and
 * Entity__MoodCue44 call it first thing every tick.
 *
 * The literals are left unnamed where they are one handler's tuning: tick
 * counts, distances in world units, TOD frame numbers, VAB program numbers,
 * and the `state` values other than 0 and ENTITY_STATE_DONE, which are each
 * handler's own phases.
 */

/* The z den of a scale template, three Ratio16s {1/1, 1/1, 1/zDenom} that
 * end here (the range splat labels SCALE_X3 runs on into its first ten
 * bytes). Entity__MoodCue41 writes the den and passes the template. */
extern s16 sScaleTemplateZDenom;

/* Defined after Entity__MoodCue43, which calls it. */
void Entity__RollScaleOrDelayedDrift(Entity *this);

void Entity__MoodCue39(Entity *this, SoundCueSet *out) {
    s32 dayYearPhase;

    if (this->moodTimer == 0) {
        dayYearPhase =
            ((DreamSys *)this->peer)->methods->getCurrentDayAndYear((DreamSys *)this->peer, 0) % 3;
        if (dayYearPhase == 0) {
            if (rand() % 3 != 0) {
                goto skipScaleBump;
            }
        } else if (dayYearPhase != 2) {
            goto skipScaleBump;
        }
        this->methods->updateScale(this, 1, SCALE_Y4);
    }
skipScaleBump:
    if (out->tick % 22 == 0) {
        out->attenuation = this->methods->getProximityRatio(this);
        out->slots[0].program = 2;
    }
    if (rand() % 12 == 0) {
        this->methods->stopTod(this);
    } else if (rand() % 6 == 0) {
        ((EntityPlayTodFn)this->methods->playTod)(this);
    }
}

void Entity__MoodCue40(Entity *this, SoundCueSet *out) {
    Ratio16 *table = NULL;

    if (out->tick % 7 == 0) {
        out->attenuation = this->methods->getProximityRatio(this);
        out->slots[0].program = 3;
        out->slots[0].vol = 64;
        out->slots[0].endVol = 64;
    }
    if (this->moodTimer == 200) {
        table = ROTATION_YAW_MINUS90;
    } else if (this->moodTimer == 400) {
        table = ROTATION_YAW_PLUS180;
    } else if (this->moodTimer == 600) {
        table = ROTATION_YAW_PLUS90;
    } else if (this->moodTimer == 800) {
        table = ROTATION_YAW_PLUS180;
        this->moodTimer = -1;
    }
    if (table != NULL) {
        this->methods->updateRotation(this, 0, table);
    }
    this->methods->moveLocalZOrFindLink(this, -30, 0);
    if (this->linkTarget != 0) {
        this->methods->moveLocalY(this, -200, 0);
    }
}

void Entity__MoodCue41(Entity *this, SoundCueSet *out) {
    s32 roll;
    s16 *zDenom;

    if (this->moodTimer == 0) {
        this->state = rand() % 5 + 10;
    }
    if (this->state < 14 || this->moodTimer < 320) {
        Entity__StepYawInWindowsThenDeactivate(this, out, 3000, 500, -256);
        return;
    }
    if (this->state == 14) {
        if ((this->moodTimer & 3) == 0) {
            roll = rand();
            zDenom = &sScaleTemplateZDenom;
            *zDenom = roll % 32 + 1;
            /* Back from the den to the start of its template. MATCHING:
             * retail relocates against sScaleTemplateZDenom, not SCALE_X3. */
            this->methods->updateScale(this, 1, (Ratio16 *)(zDenom + 1) - 3);
        }
    }
}

void Entity__MoodCue42(Entity *this, SoundCueSet *out) {
    s32 divisor;

    if (this->moodTimer < 20) {
        this->methods->stopTod(this);
        this->methods->moveLocalZ(this, -30, 0);
    } else if (this->moodTimer == 20) {
        ((EntityPlayTodFn)this->methods->playTod)(this);
        out->attenuation = 0;
        out->slots[0].program = 5;
    } else {
        divisor = this->todFrameCount * 3 + 20;
        if (this->moodTimer % divisor == 0) {
            this->methods->stopTod(this);
            out->slots[0].program = SOUND_CUE_STOP;
        }
    }
}

void Entity__MoodCue43(Entity *this, SoundCueSet *out) {
    s32 dx;
    s32 rotPick;
    Ratio16 *table;

    Entity__RollScaleOrDelayedDrift(this);
    out->attenuation = this->methods->getProximityRatio(this);
    if (this->todFrame == 0 || this->todFrame == 15) {
        out->slots[0].program = 18;
        out->slots[1].program = 18;
    }
    if (this->moodTimer >= 321) {
        dx = (rand() & 1) ? -60 : 60;
        this->methods->moveLocalX(this, dx, 0);
        rotPick = rand();
        table = ROTATION_YAW_PLUS9;
        if ((rotPick & 3) != 0) {
            table = ROTATION_YAW_MINUS9;
        }
        this->methods->updateRotation(this, 0, table);
    }
}

void Entity__MoodCue44(Entity *this, SoundCueSet *out) {
    s32 dx;
    s32 dxPick;
    s32 rotPick;
    Ratio16 *table;

    Entity__RollScaleOrDelayedDrift(this);
    out->attenuation = this->methods->getProximityRatio(this);
    if (this->todFrame == 7 || this->todFrame == 22) {
        out->slots[0].program = 3;
    }
    if (this->moodTimer >= 300 && this->moodTimer < 320) {
        this->methods->moveLocalZ(this, -60, 0);
    } else if (this->moodTimer >= 321 && this->moodTimer < 340) {
        this->methods->updateRotation(this, 0, ROTATION_YAW_MINUS9);
    } else if (this->moodTimer >= 321) {
        dxPick = rand();
        dx = -128;
        if ((dxPick & 1) != 0) {
            dx = 128;
        }
        this->methods->moveLocalX(this, dx, (void *)1);
        rotPick = rand();
        table = ROTATION_YAW_PLUS9;
        if ((rotPick & 3) != 0) {
            table = ROTATION_YAW_MINUS9;
        }
        this->methods->updateRotation(this, 0, table);
    }
}

/* At the cue's start, rolls 0..9: 8 or 9 stretches the entity by
 * SCALE_X3, 5 to 7 arms a drift (state 10) that adds TRANSLATE_Z_MINUS256
 * every tick from tick 201 on. */
void Entity__RollScaleOrDelayedDrift(Entity *this) {
    s32 roll;

    if (this->moodTimer == 0) {
        roll = rand() % 10;
        if (roll >= 8) {
            this->methods->updateScale(this, 1, SCALE_X3);
        } else if (roll >= 5) {
            this->state = 10;
        }
    }
    if (this->state == 10 && this->moodTimer >= 201) {
        this->methods->addTranslation(this, TRANSLATE_Z_MINUS256);
    }
}

/* Row 45 has no per-tick effect. */
void Entity__MoodCue45(void) {}

void Entity__MoodCue46(Entity *this, SoundCueSet *out) {
    s32 dayYearPhase;

    if (this->moodTimer == 0) {
        dayYearPhase =
            ((DreamSys *)this->peer)->methods->getCurrentDayAndYear((DreamSys *)this->peer, 0) % 3;
        if (dayYearPhase == 0) {
            if (rand() % 3 != 0) {
                goto skipScaleBump;
            }
        } else if (dayYearPhase != 1) {
            goto skipScaleBump;
        }
        this->methods->updateScale(this, 1, SCALE_SIX);
    }
skipScaleBump:
    if (out->tick == 0) {
        out->attenuation = 0;
        out->slots[0].program = 18;
    }
    SceneNode__FaceTarget((SceneNode *)this, (SceneNode *)this->peer, 1, 0, 0);
}

void Entity__MoodCue47(Entity *this) {
    if (this->targetReached == 0) {
        return;
    }
    if (this->state == 0) {
        this->state = 12;
        this->moodTimer = 0;
        return;
    }
    if (this->state == 12) {
        if (this->moodTimer < 30) {
            if (((DreamSys *)this->peer)->methods->getLinkCommandFlag((DreamSys *)this->peer) != 0) {
                ((DreamSys *)this->peer)->methods->clearTickCallbacks((DreamSys *)this->peer, false);
                this->moodTimer = 0;
                this->state = 11;
            }
        } else {
            this->methods->notifyParents(this, ENTITY_EFFECT_EVENT_VIDEO);
            this->state = 10;
        }
    } else if (this->state == 11) {
        if (this->moodTimer == 100) {
            this->methods->notifyParents(this, ENTITY_EFFECT_END_DREAM);
        } else {
            ((DreamSys *)this->peer)->methods->moveLocalY((DreamSys *)this->peer, -100, 0);
        }
    }
}

void Entity__MoodCue48(Entity *this, SoundCueSet *out) {
    s32 dy;
    EntityMethods *methods;

    if (this->todFrame == 38) {
        SceneNode__FaceTarget((SceneNode *)this, (SceneNode *)this->peer, 1, 0, 0);
        out->attenuation = this->methods->getProximityRatio(this);
        out->slots[0].program = 6;
    }
    methods = this->methods;
    dy = (this->moodTimer % 10 < 5) ? -30 : 30;
    methods->moveLocalY(this, dy, 0);
    this->methods->moveLocalZ(this, -30, (void *)1);
}

void Entity__MoodCue49(Entity *this, SoundCueSet *out) {
    DreamSysMethods *peerMethods;
    void *translation;

    if (out->tick == 6) {
        out->attenuation = 0;
        out->slots[0].program = 4;
        out->slots[1].program = 4;
        out->slots[2].program = 4;
    }
    if (this->targetReached != 0) {
        if (this->state == 0) {
            this->state = 10;
            this->moodTimer = 0;
        } else if (this->state == 10) {
            if (this->moodTimer == 10) {
                this->methods->notifyParents(this, ENTITY_EFFECT_LINK_STAGE);
            } else if (((DreamSys *)this->peer)->methods->getLinkCommandFlag((DreamSys *)this->peer) != 0) {
                peerMethods = ((DreamSys *)this->peer)->methods;
                translation = this->parent ? this->coord2->workm.t : NULL;
                peerMethods->setTranslation((DreamSys *)this->peer, translation);
                ((DreamSys *)this->peer)->methods->updateRotation((DreamSys *)this->peer, 1, ROTATION_YAW_MINUS90);
                ((DreamSys *)this->peer)->methods->clearTickCallbacks((DreamSys *)this->peer, false);
                this->moodTimer = 0;
                this->state = 11;
            }
        } else if (this->state == 11) {
            peerMethods = ((DreamSys *)this->peer)->methods;
            translation = this->parent ? this->coord2->workm.t : NULL;
            peerMethods->setTranslation((DreamSys *)this->peer, translation);
            if (this->moodTimer == 100) {
                this->methods->notifyParents(this, ENTITY_EFFECT_LINK_STAGE);
            }
        }
    }
    this->methods->moveLocalZ(this, -256, 0);
}

void Entity__MoodCue50(Entity *this, SoundCueSet *out) {
    if (this->moodTimer < this->todFrameCount * 5) {
        if (this->todFrame == 15 || this->todFrame == 70) {
            out->attenuation = 0;
            out->slots[0].program = 7;
            out->slots[1].program = 7;
            out->slots[2].program = 7;
        }
    } else {
        this->methods->deactivate(this);
        this->state = ENTITY_STATE_DONE;
    }
}

void Entity__MoodCue51(Entity *this, SoundCueSet *out) {
    Ratio16 *table;

    if (this->moodTimer == 0 && rand() % 5 == 0 && this->state == 0) {
        this->methods->updateScale(this, 1, SCALE_SIX);
        this->methods->moveLocalY(this, 800, 0);
        this->state = 11;
    }
    table = NULL;
    if (out->tick % 5 == 0) {
        out->attenuation = this->methods->getProximityRatio(this);
        out->slots[0].program = 8;
    }
    if (this->moodTimer == 90) {
        table = ROTATION_YAW_MINUS90;
    } else if (this->moodTimer == 160) {
        table = ROTATION_YAW_PLUS90;
    } else if (this->moodTimer == 220) {
        if (rand() & 1) {
            table = ROTATION_YAW_PLUS180;
        }
    }
    if (table != NULL) {
        this->methods->updateRotation(this, 0, table);
    }
    this->methods->moveLocalZ(this, -80, (void *)1);
}

void Entity__MoodCue52(Entity *this, SoundCueSet *out) {
    out->attenuation = this->methods->getProximityRatio(this);
    if (this->moodTimer < 188) {
        if (this->moodTimer == 84) {
            this->methods->updateRotation(this, 0, ROTATION_YAW_PLUS180);
        }
        if (out->tick % 20 == 0) {
            out->slots[0].program = 9;
        }
    } else if (this->moodTimer < 200) {
        this->methods->updateRotation(this, 0, ROTATION_YAW_PLUS9);
    } else {
        this->methods->deactivate(this);
        out->slots[1].program = 30;
        this->state = ENTITY_STATE_DONE;
    }
    this->methods->moveLocalZOrFindLink(this, -512, 0);
}

void Entity__MoodCue55(Entity *this, SoundCueSet *out) {
    s32 frame = this->todFrame;

    if (this->moodTimer == 0) {
        if (rand() % 3 == 0) {
            this->methods->updateScale(this, 1, SCALE_Y2);
        }
    }
    out->attenuation = this->methods->getProximityRatio(this);
    if (frame >= 32) {
        frame -= 32;
    }
    if (frame == 9 || frame == 17 || frame == 23) {
        out->slots[0].program = 19;
    }
    if (frame == 23) {
        out->slots[1].program = 19;
    }
}

void Entity__MoodCue56(Entity *this) {
    if (this->moodTimer == 0) {
        this->methods->moveLocalY(this, -200, 0);
    }
}

void Entity__MoodCue57(Entity *this, SoundCueSet *out) {
    s32 frame;

    if (this->moodTimer == 0) {
        this->state = rand() % 3;
        if (this->state == 0) {
            this->methods->stopTod(this);
            this->methods->moveLocalY(this, 6144, 0);
        }
    }
    out->attenuation = this->methods->getProximityRatio(this);
    if (this->state != 0) {
        frame = this->todFrame;
        if (frame < 30) {
            out->slots[0].program = 12;
            out->slots[0].octave = -1;
        } else if (frame == 30) {
            out->slots[0].program = SOUND_CUE_STOP;
        } else if (frame == 35) {
            out->slots[2].program = 22;
            out->slots[2].octave = -2;
        } else if (frame == 48) {
            if (Entity__IsNearTarget(this, this->coord2->coord.t, 15, 10)) {
                if (Entity__GetOrCreateFadeBox(this, NULL, NULL, (void *)10, 0) != NULL) {
                    this->fadeBox->methods->startFadeDown(this->fadeBox, (BasicClass *)this->ticker,
                                                          4, 0);
                }
                if (rand() & 1) {
                    this->methods->notifyParents(this, ENTITY_EFFECT_EVENT_VIDEO);
                }
            }
        } else if (frame == 59) {
            this->methods->deactivate(this);
            this->state = ENTITY_STATE_DONE;
        }
        return;
    }
    out->slots[0].program = 12;
    out->slots[0].octave = -1;
    this->methods->moveLocalZ(this, -512, 0);
    if (this->moodTimer >= 128 && this->moodTimer < 322) {
        this->methods->moveLocalY(this, -128, 0);
    } else if (this->moodTimer == 322) {
        ((EntityPlayTodFn)this->methods->playTod)(this);
        this->state = ENTITY_STATE_DONE;
    }
}

void Entity__MoodCue58(Entity *this, SoundCueSet *out) {
    if (this->moodTimer == 0) {
        out->attenuation = 0;
        out->slots[0].program = 12;
        if (((DreamSys *)this->peer)->methods->getDreamColor((DreamSys *)this->peer) ==
            DREAM_COLOR_YELLOW) {
            this->state = 11;
        } else if (rand() % 3 == 0) {
            this->state = 12;
        }
    }
    if (out->tick % 100 == 0) {
        out->attenuation = this->methods->getProximityRatio(this);
        out->slots[0].program = 12;
        out->slots[0].octave = -1;
    }
    if (this->state == 11) {
        if (this->methods->distanceToPeer(this, this->peer) < 1024) {
            ((DreamSys *)this->peer)->methods->clearTickCallbacks((DreamSys *)this->peer, false);
            this->state = 13;
            this->moodTimer = 0;
        }
    } else if (this->state == 12) {
        if (this->methods->distanceToPeer(this, this->peer) < 1024) {
            this->methods->stopTod(this);
            this->state = 14;
            this->moodTimer = 0;
        }
    }
    if (this->state == 13) {
        if (this->moodTimer < 50) {
            ((DreamSys *)this->peer)->methods->moveLocalY((DreamSys *)this->peer, -20, 0);
        } else if (this->moodTimer < 500) {
            ((DreamSys *)this->peer)
                ->methods->moveLocalX((DreamSys *)this->peer, (this->moodTimer % 40 < 20) ? -5 : 5, 0);
        } else if (this->moodTimer == 500) {
            this->methods->notifyParents(this, ENTITY_EFFECT_END_DREAM);
        }
    }
    if (this->state == 14) {
        if (this->moodTimer < 10) {
            this->methods->moveLocalY(this, 200, 0);
            return;
        }
        if (this->moodTimer == 10) {
            out->slots[0].program = 18;
            out->attenuation = 0;
            out->slots[1].program = 3;
            this->methods->updateRotation(this, 1, ROTATION_ZMINUS90);
            this->methods->moveLocalX(this, 2400, 0);
            this->methods->moveLocalY(this, 1500, 0);
            this->parts[1]->methods->setDisplay(this->parts[1], 0);
            this->state = ENTITY_STATE_DONE;
        }
    }
}

void Entity__MoodCue115(Entity *this, SoundCueSet *out) {
    if (this->state == 0) {
        if (Entity__IsTargetInRange(this, 2048) != 0) {
            this->state = 11;
            SceneNode__FaceTarget((SceneNode *)this, (SceneNode *)this->peer, 1, 0, 0);
            SceneNode__FaceTarget((SceneNode *)this->peer, (SceneNode *)this, 1, 1, 0);
            this->methods->activate(this);
            this->methods->startSoundCue(this);
            ((DreamSys *)this->peer)->methods->clearTickCallbacks((DreamSys *)this->peer, true);
            out->attenuation = 0;
            out->slots[0].program = 12;
            this->moodTimer = 0;
        }
    }
    if (this->state == 0) {
        this->methods->deactivate(this);
        this->methods->stopSoundCue(this);
        goto tail;
    }
    if (out->tick % 100 == 0) {
        out->attenuation = this->methods->getProximityRatio(this);
        out->slots[0].program = 12;
        out->slots[0].octave = -1;
    }
    if (this->moodTimer < 3) {
        this->methods->moveLocalY(this, 150, 0);
    } else if (this->moodTimer < 7) {
        this->methods->moveLocalY(this, (this->moodTimer & 1) ? -50 : 50, 0);
    } else if (this->moodTimer == 100) {
        if (rand() & 1) {
            this->state = 12;
            this->methods->stopTod(this);
        }
    } else if (this->moodTimer == 240) {
        ((DreamSys *)this->peer)
            ->methods->setTickCallbacks((DreamSys *)this->peer, MOVE_CALLBACK_TICK_MOVE,
                                        LOOK_CALLBACK_STEP_LOOK);
    }
    if (this->state == 12) {
        if (this->moodTimer < 130) {
            this->methods->moveLocalY(this, 10, 0);
        } else if (this->moodTimer < 160) {
            this->methods->moveLocalZ(this, -30, 0);
        } else if (this->moodTimer < 301) {
            /* nothing */
        } else {
            SceneNode__FaceTarget((SceneNode *)this, (SceneNode *)this->peer, 1, 0, 0);
            this->methods->moveLocalZ(this, -30, 0);
        }
    }
tail:
    if (this->methods->distanceToPeer(this, this->peer) < 512) {
        this->methods->deactivate(this);
        this->methods->notifyParents(this, ENTITY_EFFECT_LINK_STAGE);
    }
}

/* ---- merged from Entity_e ---- */

/* Entity_e: twenty of Entity's MoodCue handlers.
 *
 * Each Entity__MoodCueNN is the `handler` of gEntityMoodHandlerTable's row
 * NN (include/Entity.h): rows 59, 61, 62, 64 to 71 and 73 to 81, and
 * Entity__MoodCue81 is row 120's handler too (the row's data words
 * differ). Rows 60, 63 and 72 have no handler. An Entity whose moodIndex
 * selects the row installs it as its SoundCueSet callback, so
 * ServiceSoundCueSet calls it once per tick with the Entity and its cue
 * set. A handler requests tones by filling the set's slots (a VAB program
 * of the cue's sound object, or SOUND_CUE_STOP), moves and turns the
 * entity (or the player, its `peer`) on moodTimer, the ticks since
 * startSoundCue, on the cue set's own `tick`, or on todFrame, the frame of
 * its TOD animation, and sends the dream an EntityEffect through
 * notifyParents. Entity__MoodCue108 (Entity_g) runs Entity__MoodCue71 and
 * then sets its scale to SCALE_SIX.
 *
 * The literals are left unnamed where they are one handler's tuning: tick
 * counts, distances in world units, TOD frame numbers, VAB program numbers,
 * and the `state` values other than 0 and ENTITY_STATE_DONE, which are each
 * handler's own phases.
 */

void Entity__MoodCue59(Entity *this, SoundCueSet *out) {
    if (this->moodTimer == 0 && rand() % 10 == 0) {
        this->state = 12;
    }
    if (out->tick % 10 == 0) {
        out->attenuation = this->methods->getProximityRatio(this);
        out->slots[0].program = 12;
        out->slots[0].octave = -1;
    }
    if (this->moodTimer == 0) {
        if (rand() & 1) {
            this->methods->moveLocalY(this, 2048, 0);
        }
    }
    this->methods->moveLocalZ(this, -128, 0);
    if (this->state == 12 && this->moodTimer == 300) {
        this->grid->methods->startScaleRamp(this->grid, 1, 1);
    }
}

void Entity__MoodCue61(Entity *this, SoundCueSet *out) {
    if (this->todFrame == 30) {
        out->slots[0].program = 18;
        out->attenuation = 0;
        out->slots[0].octave = -1;
    }
}

void Entity__MoodCue62(Entity *this, SoundCueSet *out) {
    if (this->todFrame % 30 == 0) {
        out->slots[0].program = 3;
        out->attenuation = 0;
        out->slots[0].octave = -2;
    }
    if (this->moodTimer >= 101) {
        SceneNode__FaceTarget((SceneNode *)this, (SceneNode *)this->peer, 1, 0, 0);
    }
    this->methods->moveLocalZ(this, -5, 0);
    if (this->moodTimer == 300 && this->methods->distanceToPeer(this, this->peer) < 4096) {
        ((DreamSys *)this->peer)->methods->clearTickCallbacks((DreamSys *)this->peer, false);
    } else if (this->moodTimer == 500) {
        ((DreamSys *)this->peer)
            ->methods->setTickCallbacks((DreamSys *)this->peer, MOVE_CALLBACK_TICK_MOVE,
                                        LOOK_CALLBACK_STEP_LOOK);
    }
    if (this->state == 0) {
        if (this->methods->distanceToPeer(this, this->peer) < 1024) {
            if (rand() & 1) {
                out->slots[1].program = 6;
                out->attenuation = 0;
                out->slots[1].octave = -1;
                if (rand() & 1) {
                    this->grid->methods->startScaleRamp(this->grid, -1, 0);
                }
                this->moodTimer = 0;
                this->state = 10;
            } else {
                this->state = 11;
            }
        }
    }
    if (this->state == 10 && this->moodTimer == 70) {
        this->methods->notifyParents(this, (rand() & 1) ? ENTITY_EFFECT_END_DREAM
                                                        : ENTITY_EFFECT_EVENT_VIDEO);
    }
}

void Entity__MoodCue64(Entity *this, SoundCueSet *out) {
    s32 phase = out->tick % 300;

    out->attenuation = this->methods->getProximityRatio(this);
    if (phase < 20) {
        out->slots[0].program = 5;
        out->slots[0].octave = -2;
    } else if (phase == 22) {
        out->slots[0].program = SOUND_CUE_STOP;
    }
    this->methods->moveLocalZ(this, -10, 0);
}

void Entity__MoodCue65(Entity *this, SoundCueSet *out) {
    if (this->moodTimer == 0) {
        if (rand() % 3 == 0) {
            this->methods->updateScale(this, 1, SCALE_HALF);
            this->methods->moveLocalY(this, -300, 0);
            this->methods->updateRotation(this, 1, ROTATION_YAW_PLUS90);
            this->state = 11;
        }
    }
    if (this->state == 11) {
        if (this->moodTimer == 2000) {
            this->methods->updateRotation(this, 0, ROTATION_YAW_MINUS90);
        }
        this->methods->moveLocalZ(this, -20, 0);
    }
}

void Entity__MoodCue66(Entity *this, SoundCueSet *out) {
    out->attenuation = this->methods->getProximityRatio(this);
    if (out->tick % 30 == 0) {
        out->slots[0].program = 13;
    }
}

void Entity__MoodCue67(Entity *this, SoundCueSet *out) {
    if (this->moodTimer == 0) {
        if (rand() % 3 == 0) {
            this->state = 11;
        }
    }
    if (this->state == 11) {
        if (this->moodTimer == 502) {
            this->methods->moveLocalY(this, 2048, 0);
            SceneNode__FaceTarget((SceneNode *)this, (SceneNode *)this->peer, 1, 0, 0);
        }
        if (this->moodTimer >= 501) {
            this->methods->moveLocalZ(this, -512, 0);
        }
    }
}

void Entity__MoodCue68(Entity *this, SoundCueSet *out) {
    out->attenuation = this->methods->getProximityRatio(this);
    if (out->tick == 0) {
        this->lastOffsetValue = (rand() & 1) ? -374 : -192;
    }
    if (this->state == 0) {
        if (out->tick % 10 == 0) {
            out->slots[0].program = 28;
        }
        if (out->tick % 20 == 0) {
            out->slots[1].program = 23;
            out->slots[1].octave = -1;
            out->slots[2].program = 23;
            out->slots[2].octave = -1;
        } else if (out->tick % 20 == 14) {
            out->slots[1].program = SOUND_CUE_STOP;
            out->slots[2].program = SOUND_CUE_STOP;
        }
        if ((this->moodTimer & 1) == 0) {
            if (((DreamSys *)this->peer)->methods->getLinkCommandFlag((DreamSys *)this->peer) != 0) {
                this->moodTimer = -1;
                this->state = 10;
                out->slots[0].program = 18;
            }
        }
        SceneNode__FaceTarget((SceneNode *)this, (SceneNode *)this->peer, 1, 0, 0);
        this->methods->moveLocalZ(this, this->lastOffsetValue, (void *)1);
    } else if (this->state == 10) {
        if (this->moodTimer < 8) {
            this->methods->updateRotation(this, 0, ROTATION_ZPLUS9);
            this->methods->addTranslation(this, TRANSLATE_Y_PLUS8);
        } else {
            u32 coin;

            out->slots[0].program = 18;
            out->slots[1].program = 3;
            this->methods->stopSoundCue(this);
            coin = rand() & 1;
            /* Half the time ENTITY_STATE_DONE, else back to phase 0. */
            this->state = coin < 1;
        }
    }
}

void Entity__MoodCue69(Entity *this, SoundCueSet *out) {
    out->attenuation = this->methods->getProximityRatio(this);
    if (this->todFrame == this->todFrameCount - 1) {
        out->slots[0].program = 25;
        out->slots[0].octave = -2;
    }
    if (out->tick % 4 == 0) {
        out->slots[1].program = 21;
        out->slots[1].octave = -1;
    }
    if (out->tick % 200 == 0) {
        out->slots[2].program = 13;
        out->slots[2].octave = 1;
    }
}

void Entity__MoodCue70(Entity *this, SoundCueSet *out) {
    if (this->moodTimer == 0) {
        if (rand() & 1) {
            this->state = 11;
        }
    }
    if (this->moodTimer == 300) {
        this->methods->updateRotation(this, 0, ROTATION_YAW_PLUS180);
    }
    if (this->moodTimer < 600) {
        this->methods->moveLocalZ(this, this->state == 0 ? -256 : 256, 0);
    }
}

void Entity__MoodCue71(Entity *this, SoundCueSet *out) {
    s32 lane;

    out->attenuation = this->methods->getProximityRatio(this);
    if (out->tick == 0) {
        out->slots[0].program = 0;
        lane = rand() % 3;
        this->methods->moveLocalX(this, lane * 51200, 0);
    }
    if (this->moodTimer >= 2401) {
        SceneNode__FaceTarget((SceneNode *)this, (SceneNode *)this->peer, 1, 0, 0);
    }
    this->methods->moveLocalZ(this, -30, 0);
}

void Entity__MoodCue73(Entity *this, SoundCueSet *out) {
    out->attenuation = 0;
    if (out->tick == 0) {
        ((DreamSys *)this->peer)->methods->updateRotation((DreamSys *)this->peer, 1, ROTATION_YAW_PLUS90);
        ((DreamSys *)this->peer)->methods->clearTickCallbacks((DreamSys *)this->peer, true);
        out->slots[0].program = 25;
        out->slots[1].program = 25;
        out->slots[2].program = 25;
    } else if (out->tick == 20) {
        out->slots[1].program = 13;
    }
    if (this->moodTimer == this->todFrameCount - 1) {
        this->methods->deactivate(this);
    }
}

/* {0, 100, 190}: the clear colour Entity__MoodCue74 gives the peer's viewport. */
extern ViewportRgb sMoodCue74ClearColor;

void Entity__MoodCue74(Entity *this, SoundCueSet *out) {
    if (this->moodTimer == 0) {
        ((DreamSys *)this->peer)
            ->viewport->methods->setClearColor(((DreamSys *)this->peer)->viewport, &sMoodCue74ClearColor);
        this->state = rand() % 3;
        if (((DreamSys *)this->peer)->coord2->coord.t[2] < 610) {
            this->state = 0;
        }
    }
    if (this->state != 0) {
        if (this->todFrameCount / 2 < this->moodTimer) {
            ((DreamSys *)this->peer)->methods->moveLocalZ((DreamSys *)this->peer, 128, 0);
        }
        if (this->moodTimer == this->todFrameCount - 30) {
            this->methods->notifyParents(this, ENTITY_EFFECT_LINK_STAGE);
        }
    } else {
        if (this->moodTimer >= 20 && this->moodTimer < 120) {
            ((DreamSys *)this->peer)
                ->methods->moveLocalZ((DreamSys *)this->peer, -((this->moodTimer - 19) * 32), (void *)1);
            if (this->moodTimer == 85) {
                ((DreamSys *)this->peer)
                    ->methods->setTickCallbacks((DreamSys *)this->peer, MOVE_CALLBACK_TICK_MOVE,
                                                LOOK_CALLBACK_STEP_LOOK);
            }
        }
    }
}

void Entity__MoodCue75(Entity *this, SoundCueSet *out) {
    if (out->tick == 0) {
        out->attenuation = 0;
        out->slots[0].program = 25;
        out->slots[1].program = 25;
        out->slots[2].program = 25;
        SceneNode__FaceTarget((SceneNode *)this, (SceneNode *)this->peer, 1, 0, 0);
    }
    if (this->moodTimer == this->todFrameCount) {
        this->methods->stopTod(this);
        this->methods->notifyParents(this, ENTITY_EFFECT_LINK_STAGE);
    }
}

void Entity__MoodCue76(Entity *this, SoundCueSet *out) {
    if (this->targetReached != 0) {
        ((EntityPlayTodFn)this->methods->playTod)(this);
        if (this->todFrame == this->todFrameCount - 1) {
            this->methods->stopTod(this);
            this->methods->updateScale(this, 0, SCALE_MINUS_SIXTY_FOURTH);
        }
    } else {
        this->methods->stopTod(this);
        this->methods->updateRotation(this, 0, ROTATION_YAW_PLUS9);
    }
}

void Entity__MoodCue77(Entity *this, SoundCueSet *out) {
    out->attenuation = this->methods->getProximityRatio(this);
    if (out->tick % 5 == 0) {
        out->slots[0].program = 17;
        out->slots[0].octave = -2;
    }
    if (this->todIndex == 0) {
        if (this->moodTimer == this->todFrameCount) {
            this->methods->setTod(this, 1);
            if (rand() & 1) {
                this->state = 11;
            }
        }
        return;
    }
    if (this->state == 0) {
        if (this->moodTimer == 60 || this->moodTimer == 212 || this->moodTimer == 290 ||
            this->moodTimer == 320) {
            this->methods->updateRotation(this, 0, ROTATION_YAW_PLUS90);
        }
        if (this->moodTimer == 398) {
            this->methods->updateRotation(this, 0, ROTATION_YAW_MINUS90);
        }
        this->methods->moveLocalZOrFindLink(this, -50, 0);
        return;
    }
    if (this->moodTimer == 60 || this->moodTimer == 140) {
        this->methods->updateRotation(this, 0, ROTATION_YAW_PLUS90);
    }
    if (this->moodTimer < 174) {
        this->methods->moveLocalZOrFindLink(this, -50, 0);
    }
    if (this->moodTimer == 174) {
        this->methods->stopSoundCue(this);
        this->state = ENTITY_STATE_DONE;
    }
}

/* Entity__MoodCue78's: cleared on the cue's first tick, set when its phase
 * 11 ends the cue at tick 510; at tick 520 a set flag ends it again. */
extern s32 sMoodCue78TransitionDone;

void Entity__MoodCue78(Entity *this, SoundCueSet *out) {
    /* MATCHING: one local for the roll and then the y move; two allocate
     * differently. */
    s32 rollOrDy;
    void *table;

    if (out->tick == 0) {
        sMoodCue78TransitionDone = 0;
        rollOrDy = rand() % 3;
        if (rollOrDy == 1) {
            this->state = 11;
        }
        if (rollOrDy == 2) {
            this->state = 12;
        }
    }

    out->attenuation = this->methods->getProximityRatio(this);

    if (this->todPlaying != 0 && (out->tick & 3) == 0) {
        out->slots[0].program = 28;
    }

    if (this->moodTimer == this->todFrameCount - 1) {
        this->moodTimer = -1;
    } else {
        if (this->moodTimer >= this->todFrameCount / 2 + this->todFrameCount / 4) {
            ((EntityPlayTodFn)this->methods->playTod)(this);
        } else if (this->moodTimer >= this->todFrameCount / 2) {
            if (this->moodTimer == this->todFrameCount / 2) {
                out->slots[0].program = 16;
            }
            this->methods->moveLocalZ(this, 110, 0);
        } else if (this->moodTimer < this->todFrameCount / 4) {
            /* nothing */
        } else {
            this->methods->stopTod(this);
            this->methods->moveLocalZ(this, -110, 0);
        }
    }

    if (this->state == 11 && out->tick == 510) {
        this->methods->moveLocalY(this, -380, 0);
        this->methods->updateRotation(this, 0, ROTATION_XPLUS90);
        this->methods->stopSoundCue(this);
        this->state = ENTITY_STATE_DONE;
        sMoodCue78TransitionDone = 1;
    } else if (this->state >= 12 && out->tick >= 330 && (out->tick % 60) == 30) {
        rollOrDy = 0;
        if (rand() & 1) {
            table = SCALE_Y2;
            rollOrDy = (this->state == 12) ? 400 : 0;
            this->state = 13;
        } else {
            table = SCALE_UNIT;
            if (this->state == 13) {
                rollOrDy = -400;
            }
            this->state = 12;
        }
        this->methods->updateScale(this, 1, table);
        this->methods->moveLocalY(this, rollOrDy, 0);
    }

    if (out->tick == 520 && sMoodCue78TransitionDone != 0) {
        this->methods->stopSoundCue(this);
        this->state = ENTITY_STATE_DONE;
    }
}

void Entity__MoodCue79(Entity *this, SoundCueSet *out) {
    out->attenuation = this->methods->getProximityRatio(this);
    if (out->tick % 10 == 0) {
        out->slots[0].program = 25;
        out->slots[0].octave = 2;
    }
    this->methods->updateRotation(this, 0, ROTATION_YAW_PLUS2);
    if (this->state == 0 && this->targetReached != 0) {
        this->methods->notifyParents(this, ENTITY_EFFECT_EVENT_VIDEO);
        this->state = 11;
    }
}

void Entity__MoodCue80(Entity *this, SoundCueSet *out) {
    if (this->moodTimer < this->todFrameCount) {
        if (this->todFrame != 0) {
            if (this->todFrame == 20) {
                out->attenuation = 0;
                out->slots[0].program = 16;
            }
        }
    } else {
        this->methods->stopTod(this);
        this->methods->addTranslation(this, TRANSLATE_Y_MINUS512);
    }
    SceneNode__FaceTarget((SceneNode *)this, (SceneNode *)this->peer, 1, 0, 0);
}

void Entity__MoodCue81(Entity *this, SoundCueSet *out) {
    s32 dz;
    s32 state;

    if (this->moodTimer == 0 && (rand() & 1)) {
        this->state = rand() % 3;
    }
    if (this->state != 0 && this->targetReached != 0) {
        if (out->tick >= 61) {
            out->tick = 0;
        }
        if (out->tick % 20 == 0) {
            out->slots[0].program = 23;
            out->attenuation = 0;
            out->slots[0].octave = -1;
            out->slots[2].program = out->slots[1].program = 23;
            out->slots[1].octave = -1;
            out->slots[2].octave = -2;
        } else if (out->tick % 20 == 14) {
            out->slots[0].program = SOUND_CUE_STOP;
            out->slots[1].program = SOUND_CUE_STOP;
            out->slots[2].program = SOUND_CUE_STOP;
        }
        SceneNode__FaceTarget((SceneNode *)this, (SceneNode *)this->peer, 1, 0, 0);
        state = this->state;
        if (state == 1) {
            this->methods->updateScale(this, 0, SCALE_EIGHT_SEVENTHS);
            dz = -374;
            if (this->methods->distanceToPeer(this, this->peer) < 512) {
                this->methods->deactivate(this);
                this->state = state;
            }
        } else {
            ((DreamSys *)this->peer)->methods->clearTickCallbacks((DreamSys *)this->peer, true);
            SceneNode__FaceTarget((SceneNode *)this->peer, (SceneNode *)this, 1, 1, 0);
            if (this->state == 2) {
                if (this->methods->distanceToPeer(this, this->peer) < 2400) {
                    this->state = 11;
                    this->methods->notifyParents(this, ENTITY_EFFECT_END_DREAM);
                }
                dz = -96;
            } else {
                dz = 0;
            }
        }
    } else {
        out->attenuation = this->methods->getProximityRatio(this);
        if (out->tick % 22 == 0) {
            out->slots[0].program = 28;
        }
        if (this->moodTimer >= 501) {
            SceneNode__FaceTarget((SceneNode *)this, (SceneNode *)this->peer, 1, 0, 0);
        }
        if (this->methods->distanceToPeer(this, this->peer) < 2048) {
            ((DreamSys *)this->peer)->methods->moveLocalZ((DreamSys *)this->peer, -2048, 0);
        }
        dz = -20;
    }
    this->methods->moveLocalZOrFindLink(this, dz, (void *)1);
    if (this->linkTarget != 0) {
        this->methods->moveLocalY(this, -200, 0);
    }
}

/* ---- merged from Entity_f ---- */

/* Entity_f: fifteen of Entity's MoodCue handlers and the two tone setters
 * they share.
 *
 * Each Entity__MoodCueNN is the `handler` of gEntityMoodHandlerTable's row
 * NN (include/Entity.h): rows 82 to 96, and Entity__MoodCue93 is row 107's
 * handler too (same `handler` word, different data words), named for its
 * lower row. An Entity whose moodIndex selects the row installs it as its
 * SoundCueSet callback, so ServiceSoundCueSet calls it once per tick with
 * the Entity and its cue set. A handler requests tones by filling the
 * set's slots (a VAB program of the cue's sound object, or SOUND_CUE_STOP),
 * moves, turns and scales the entity (or the player, its `peer`) on
 * moodTimer, the ticks since startSoundCue, on the cue set's own `tick`, or
 * on todFrame, the frame of its TOD animation, and sends the dream an
 * EntityEffect through notifyParents. Entity__MoodCue91 and
 * Entity__MoodCue92 skip their TOD animation ahead to frame 24 by stepping
 * applyTodFrame, which returns the next frame's pointer.
 *
 * SetCueTones7_7_7 and SetCueTones18_3_3 are not rows: they write a fixed
 * three-voice request into the set, unattenuated (programs 7, 7, 7 at
 * octave -2, and 18, 3, 3), for Entity__MoodCue85 and
 * Entity__MoodCue86. Entity__MoodCue82 and Entity__MoodCue89 write the
 * 18, 3 request inline.
 *
 * The literals are left unnamed where they are one handler's tuning: tick
 * counts, distances in world units, TOD frame numbers, VAB program numbers,
 * and the `state` values other than 0 and ENTITY_STATE_DONE, which are each
 * handler's own phases. The motion templates (ROTATION_*, SCALE_*,
 * TRANSLATE_*) are named by value and declared once in include/Entity.h.
 */

/* Defined after Entity__MoodCue86, which calls them. */
void SetCueTones7_7_7(SoundCueSet *out);
void SetCueTones18_3_3(SoundCueSet *out);

void Entity__MoodCue82(Entity *this, SoundCueSet *out) {
    if (this->state == 0 && this->moodTimer == 0) {
        if (rand() % 3 != 0) {
            this->state = (rand() & 1) ? 11 : 12;
        } else {
            this->methods->stopSoundCue(this);
            this->methods->moveLocalZ(this, -20480, 0);
            rand();
        }
    }
    if (this->state == 12) {
        SceneNode__FaceTarget((SceneNode *)this, (SceneNode *)this->peer, 1, 0, 0);
        if (this->moodTimer == 20) {
            out->slots[0].program = 18;
            out->attenuation = 0;
            out->slots[1].program = 3;
            ((DreamSys *)this->peer)->methods->clearTickCallbacks((DreamSys *)this->peer, 1);
        }
        if (this->moodTimer >= 21) {
            this->methods->moveLocalZ(this, -40, 0);
        }
        if (this->moodTimer == 40) {
            this->methods->notifyParents(this, ENTITY_EFFECT_LINK_STAGE);
        }
    } else if (this->state == 11) {
        this->methods->stopTod(this);
        if (this->methods->distanceToPeer(this, this->peer) < 512) {
            if (rand() % 3 != 0) {
                this->methods->deactivate(this);
            } else {
                this->methods->stopSoundCue(this);
            }
        }
    }
}

void Entity__MoodCue83(Entity *this, SoundCueSet *out) {
    if (this->todFrame % 15 == 0) {
        out->attenuation = 0;
        out->slots[0].program = 12;
        out->slots[0].octave = 2;
    }
    if (this->moodTimer == this->todFrameCount) {
        out->slots[0].program = SOUND_CUE_STOP;
        this->methods->stopSoundCue(this);
        this->state = ENTITY_STATE_DONE;
    }
}

void Entity__MoodCue84(Entity *this, SoundCueSet *out) {
    out->attenuation = this->methods->getProximityRatio(this);
    if (this->todFrame < 40) {
        out->slots[0].program = 12;
        out->slots[0].octave = -2;
        out->slots[2].program = 5;
        out->slots[2].octave = -1;
        return;
    }
    if (this->todFrame == 40) {
        out->slots[0].program = SOUND_CUE_STOP;
        out->slots[2].program = SOUND_CUE_STOP;
        return;
    }
    if (this->todFrame == 45) {
        out->slots[1].program = 18;
        out->slots[1].octave = 1;
        return;
    }
    if (this->todFrame == 64) {
        out->slots[0].program = 7;
        return;
    }
    if (this->todFrame == 89) {
        this->methods->stopSoundCue(this);
        this->state = ENTITY_STATE_DONE;
    }
}

void Entity__MoodCue85(Entity *this, SoundCueSet *out) {
    if (this->state == 0) {
        if (this->todFrame == 5) {
            SetCueTones7_7_7(out);
        }
        if (this->moodTimer == this->todFrameCount) {
            this->methods->stopTod(this);
            this->state = 10;
            this->moodTimer = -1;
        }
    } else if (this->state == 10) {
        if (this->moodTimer < 10) {
            this->methods->updateRotation(this, 0, ROTATION_YAW_PLUS9);
            if (((DreamSys *)this->peer)->methods->getLinkCommandFlag((DreamSys *)this->peer) != 0) {
                SetCueTones7_7_7(out);
                this->state = 12;
                this->moodTimer = -1;
            }
        } else {
            ((DreamSys *)this->peer)->methods->clearTickCallbacks((DreamSys *)this->peer, 1);
            this->state = 11;
            this->moodTimer = -1;
        }
    } else if (this->state == 11) {
        SceneNode__FaceTarget((SceneNode *)this, (SceneNode *)this->peer, 1, 0, 0);
        if (this->moodTimer < 30) {
            this->methods->moveLocalZ(this, -10, 0);
        } else {
            SetCueTones7_7_7(out);
            if (Entity__GetOrCreateFadeBox(this, NULL, NULL, (void *)30, 0) != NULL) {
                this->fadeBox->methods->startFadeDown(this->fadeBox, (BasicClass *)this->ticker, 7, 0);
            }
            this->state = 13;
            this->moodTimer = -1;
        }
    } else if (this->state == 13) {
        if (this->moodTimer < 90) {
            if (this->moodTimer == 30) {
                if (Entity__GetOrCreateFadeBox(this, NULL, NULL, (void *)10, 0) != NULL) {
                    this->fadeBox->methods->startFadeUp(this->fadeBox, (BasicClass *)this->ticker, 0, 0);
                }
            }
            ((DreamSys *)this->peer)->methods->updateRotation((DreamSys *)this->peer, 0, ROTATION_ZPLUS1);
        } else {
            SetCueTones18_3_3(out);
            ((DreamSys *)this->peer)->methods->updateRotation((DreamSys *)this->peer, 1, ROTATION_YAW_PLUS180);
            this->methods->notifyParents(this, (rand() % 5 != 0) ? ENTITY_EFFECT_LINK_STAGE
                                                                 : ENTITY_EFFECT_END_DREAM);
            this->state = 14;
        }
    } else if (this->state == 12) {
        if (this->moodTimer < 10) {
            this->methods->updateRotation(this, 0, ROTATION_ZMINUS9);
        } else {
            SetCueTones18_3_3(out);
            this->methods->stopSoundCue(this);
            this->state = ENTITY_STATE_DONE;
        }
    }
}

void Entity__MoodCue86(Entity *this, SoundCueSet *out) {
    if (this->moodTimer < 10) {
        this->methods->stopTod(this);
    } else if (this->moodTimer == 10) {
        ((EntityPlayTodFn)this->methods->playTod)(this);
    }
    if (this->todFrame == 10) {
        SetCueTones18_3_3(out);
    }
    if (this->moodTimer == this->todFrameCount + 10) {
        this->methods->stopSoundCue(this);
        this->state = ENTITY_STATE_DONE;
    }
}

void SetCueTones7_7_7(SoundCueSet *out) {
    out->attenuation = 0;
    out->slots[0].program = 7;
    out->slots[0].octave = -2;
    out->slots[1].program = 7;
    out->slots[1].octave = -2;
    out->slots[2].program = 7;
    out->slots[2].octave = -2;
}

void SetCueTones18_3_3(SoundCueSet *out) {
    out->slots[0].program = 18;
    out->attenuation = 0;
    out->slots[1].program = 3;
    out->slots[2].program = 3;
}

void Entity__MoodCue87(Entity *this, SoundCueSet *out) {
    out->attenuation = this->methods->getProximityRatio(this);
    if (out->tick == 0) {
        out->slots[0].program = 18;
    }
    if (out->tick >= this->todFrameCount - 1) {
        out->tick = -1;
    }
}

void Entity__MoodCue88(Entity *this, SoundCueSet *out) {
    out->attenuation = this->methods->getProximityRatio(this);
    if (out->tick == this->todFrameCount / 2) {
        out->slots[0].program = 18;
    }
    if (out->tick >= this->todFrameCount - 1) {
        out->tick = -1;
    }
}

void Entity__MoodCue89(Entity *this, SoundCueSet *out) {
    if (this->moodTimer == 20) {
        out->slots[0].program = 18;
        out->attenuation = 0;
        out->slots[1].program = 3;
        return;
    }
    if (this->moodTimer == this->todFrameCount) {
        this->methods->stopSoundCue(this);
        this->state = ENTITY_STATE_DONE;
        if (rand() & 1) {
            this->methods->notifyParents(this, ENTITY_EFFECT_EVENT_VIDEO);
        }
    }
}

void Entity__MoodCue90(Entity *this, SoundCueSet *out) {
    out->attenuation = this->methods->getProximityRatio(this);
    if (out->tick == 0) {
        out->slots[0].program = (rand() & 1) ? 2 : 1;
        out->slots[0].octave = 2;
    }
}

void Entity__MoodCue91(Entity *this, SoundCueSet *out) {
    if (this->moodTimer == 0) {
        if (Entity__GetOrCreateFadeBox(this, NULL, NULL, (void *)5, 0) != NULL) {
            if (rand() & 1) {
                this->methods->addTranslation(this, TRANSLATE_Y_MINUS256);
            }
            this->fadeBox->methods->startFadeDown(this->fadeBox, (BasicClass *)this->ticker, 0, 0);
        }
    } else {
        if (this->todFrame == 0) {
            do {
                this->todFramePtr = this->methods->applyTodFrame(this, this->todFramePtr, 0);
                this->todFrame += 1;
            } while (this->todFrame < 24);
        }
    }
    if (this->todFrame >= 25) {
        this->methods->moveLocalZ(this, -20, 0);
        ((DreamSys *)this->peer)->methods->clearTickCallbacks((DreamSys *)this->peer, 1);
    }
    if (this->moodTimer == 50) {
        this->methods->notifyParents(this, ENTITY_EFFECT_LINK_STAGE);
    } else if (this->moodTimer == 12) {
        out->attenuation = 0;
        out->slots[0].program = 21;
    }
    this->methods->updateScale(this, 1, SCALE_X_FOUR_FIFTHS_Y_SIX_FIFTHS);
}

void Entity__MoodCue92(Entity *this, SoundCueSet *out) {
    if (this->todIndex == 0) {
        if (this->targetReached != 0) {
            SceneNode__FaceTarget((SceneNode *)this, (SceneNode *)this->peer, 1, 0, 0);
            this->methods->setTod(this, 1);
            ((DreamSys *)this->peer)->methods->clearTickCallbacks((DreamSys *)this->peer, 1);
        } else if (this->todFrame == 0) {
            do {
                this->todFramePtr = this->methods->applyTodFrame(this, this->todFramePtr, 0);
                this->todFrame += 1;
            } while (this->todFrame < 24);
        }
    } else {
        if (this->todFrame == 0) {
            out->attenuation = 0;
            out->slots[0].program = 22;
        } else if (this->todFrame == this->todFrameCount - 1) {
            out->attenuation = 0;
            out->slots[1].program = 18;
            this->methods->notifyParents(this, ENTITY_EFFECT_LINK_STAGE);
        }
    }
    this->methods->updateScale(this, 1, SCALE_X_FOUR_FIFTHS_Y_SIX_FIFTHS);
}

void Entity__MoodCue93(Entity *this, SoundCueSet *out) {
    out->attenuation = this->methods->getProximityRatio(this);
    if (out->tick % 10 == 0) {
        out->slots[0].program = 3;
    }
    if (this->moodTimer == this->todFrameCount) {
        this->methods->setTod(this, 1);
    }
    if (this->todIndex == 1) {
        this->methods->moveLocalZ(this, -128, (void *)1);
    }
}

void Entity__MoodCue94(Entity *this, SoundCueSet *out) {
    if (this->moodTimer == 0) {
        if (((DreamSys *)this->peer)->methods->getDreamColor((DreamSys *)this->peer) != DREAM_COLOR_WHITE) {
            this->state = 11;
        }
    }
    out->attenuation = this->methods->getProximityRatio(this);
    if (out->tick % 10 == 0) {
        out->slots[0].program = 14;
    }
    if (this->moodTimer == this->todFrameCount) {
        this->methods->setTod(this, 1);
        if (this->state != 0) {
            if ((rand() & 1) == 0) {
                this->methods->updateScale(this, 1, SCALE_SIX);
                this->methods->moveLocalY(this, 2048, 0);
            }
        }
        if (rand() % 3 == 0) {
            this->methods->updateRotation(this, 0, ROTATION_YAW_PLUS180);
        }
    }
    if (this->todIndex != 0) {
        this->methods->moveLocalZ(this, -128, (void *)1);
    }
}

void Entity__MoodCue95(Entity *this, SoundCueSet *out) {
    if (this->moodTimer == 0) {
        this->methods->setTod(this, 3);
    } else if (this->moodTimer == this->todFrameCount) {
        this->methods->setTod(this, 1);
    }
    if (this->todIndex == 1) {
        this->methods->moveLocalZ(this, -128, 0);
    }
}

void Entity__MoodCue96(Entity *this, SoundCueSet *out) {
    if (this->moodTimer == 0) {
        this->methods->setTod(this, rand() % 4);
        return;
    }
    if (this->moodTimer % this->todFrameCount == 0) {
        this->methods->setTod(this, rand() % 4);
        out->attenuation = this->methods->getProximityRatio(this);
        out->slots[0].program = 22;
        out->slots[0].octave = 2;
        out->slots[0].vol = 64;
        out->slots[0].endVol = 32;
    }
}
