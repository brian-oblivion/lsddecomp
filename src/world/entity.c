/* Entity -- the class whole (include/entity.h): its methods, its table
 * getter GetEntityMethods, and the MoodCue handlers of its mood rows.
 *
 * An Entity is a TodActor driven by one row of sEntityMoodTable, chosen by
 * New_Entity's moodIndex. The first section holds:
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
#include <rand.h>
#include "entity.h"
#include "dream_sys.h"
#include "StageMap.h"
#include "Viewport.h"
#include "bmem_pmgr.h"

/* The size and attach offset Entity__GetOrCreateFadeBox substitutes when its
 * `size`/`offset` arguments are NULL: {320, 240} and {-100, -100}, what
 * Viewport gives its FadeBox (fade_box.h). */
extern s32 sEntityFadeBoxDefaultSize[2];
extern s32 sEntityFadeBoxDefaultOffset[2];

extern EntityMoodRow sEntityMoodTable[];
extern s8 sEntityLinkStageTable[];  /* the linkStage column (Entity) */
extern s8 sEntityEventVideoTable[]; /* the eventVideo column (Entity) */

/* The motion templates (.data, in address order):
 * the constant triples the MoodCue handlers in src/world/entity.c pass to
 * updateRotation (+0x044) and updateScale (+0x048) -- three Ratio16s
 * (include/scene_node.h), degrees or scale factors, {x, y, z} -- and to
 * addTranslation (+0x0BC), three s32 deltas. Named by value. The slots take
 * the table untyped, so the element type is the reader's (SceneNode__Update-
 * Rotation/UpdateScale), not the callers'. sTranslateYMinus64's label also
 * holds a second triple, (0, -0x20, 0); sScaleX3's z den is entity.c's
 * sScaleTemplateZDenom. */
extern Ratio16 sRotationXPlusEighth[];
extern Ratio16 sRotationYawPlus9[];
extern Ratio16 sRotationYawMinus9[];
extern Ratio16 sRotationYawPlus180[];
extern Ratio16 sRotationYawPlus90[];
extern Ratio16 sRotationYawMinus90[];
extern Ratio16 sRotationYawPlus2[];
extern Ratio16 sRotationYawMinusThird[];
extern Ratio16 sRotationYawMinusHalf[];
extern Ratio16 sRotationZPlus9[];
extern Ratio16 sRotationZPlus1[];
extern Ratio16 sRotationZMinus9[];
extern Ratio16 sRotationYawMinus120[];
extern Ratio16 sRotationX50YMinus120Z30[];
extern Ratio16 sRotationYawPlus4[];
extern Ratio16 sRotationXPlus90[];
extern Ratio16 sRotationYawPlus1[];
extern Ratio16 sRotationZMinus90[];
extern LongVec3 sTranslateYPlus256[];
extern LongVec3 sTranslateYMinus4096[];
extern LongVec3 sTranslateYMinus512[];
extern LongVec3 sTranslateYPlus64[];
extern LongVec3 sTranslateYPlus8[];
extern LongVec3 sTranslateYMinus64[];
extern LongVec3 sTranslateYMinus256[];
extern LongVec3 sTranslateXMinus64[];
extern LongVec3 sTranslateYPlus64ZMinus64[];
extern LongVec3 sTranslateYMinus1500ZPlus1024[];
extern LongVec3 sTranslateZMinus256[];
extern Ratio16 sScaleQuarter[];
extern Ratio16 sScaleHalf[];
extern Ratio16 sScaleXFourFifthsYSixFifths[]; /* {4/5, 6/5, 5/5} */
extern Ratio16 sScaleDouble[];
extern Ratio16 sScaleMinusSixtyFourth[];
extern Ratio16 sScaleEightSevenths[];
extern Ratio16 sScaleUnit[]; /* {1/1, 1/1, 1/1}, a .data copy of scene_node.h's sSceneNodeScaleOne */
extern Ratio16 sScaleEighth[];
extern Ratio16 sScaleXEighthY2ZEighth[];
extern Ratio16 sScaleSix[];
extern Ratio16 sScaleTwoFifths[];
extern Ratio16 sScaleY2[];
extern Ratio16 sScaleY4[];
extern Ratio16 sScaleTriple[];
extern Ratio16 sScaleThirtySecond[];
extern Ratio16 sScaleX3[];

Entity *New_Entity(s32 moodIndex, void *desc, void *sound) {
    Entity *obj;

    obj = BMemPMgrAlloc(sizeof(Entity));
    if (obj == NULL) {
        return NULL;
    }
    if (GetEntityMethods()->ctor(obj, moodIndex, desc, sound) == NULL) {
        BMemPMgrFree(obj);
        return NULL;
    }
    return obj;
}

Entity *Entity__Entity(Entity *self, s32 moodIndex, void *desc, void *sound) {
    if (GetTodActorMethods()->ctor((TodActor *)self, desc, sound) != NULL) {
        self->methods = GetEntityMethods();
        self->moodIndex = moodIndex;
        self->soundCueSet.tag = 0;
        self->fadeBox = NULL;
        self->ownedObject = NULL;
        self->methods->reset(self);
        return self;
    }
    return NULL;
}

FadeBox *Entity__GetOrCreateFadeBox(Entity *self, void *size, void *offset, void *step, s32 pri) {
    FadeBox *cached;
    FadeBox *box;
    FadeBoxMethods *boxMethods;
    void *attachOffset;

    cached = self->fadeBox; /* MATCHING: `cached`, `boxMethods` and `attachOffset` are each load-bearing */
    if (cached == NULL) {
        if (size == NULL) {
            size = sEntityFadeBoxDefaultSize;
        }
        box = New_FadeBox(size, 0, pri);
        if (box == NULL) {
            return NULL;
        }
        self->fadeBox = box;
    } else {
        box = cached;
    }
    box->methods->detachFromParent(box);
    boxMethods = box->methods;
    attachOffset = offset;
    if (attachOffset == NULL) {
        attachOffset = sEntityFadeBoxDefaultOffset;
    }
    boxMethods->attachToParent(box, (SceneNode *)self, attachOffset);
    box->methods->setStep(box, (s32)step);
    return box;
}

void Entity__Finalize(Entity *self) {
    if (self->fadeBox != NULL) {
        self->fadeBox->methods->release(self->fadeBox);
    }
    if (self->ownedObject != NULL) {
        self->ownedObject->methods->release(self->ownedObject);
    }
    GetTodActorMethods()->finalize((TodActor *)self);
}

void Entity__Reset(Entity *self) {
    s32 kind;

    kind = (u8)sEntityMoodTable[self->moodIndex].unlockKind;
    if (kind >= 1 && kind <= 9) {
        self->methods->setLightMode(self, 1); /* fog on (GsFOG) */
    }
    self->methods->selectTickCallback(self, TICK_CALLBACK_B);
    self->methods->deactivate(self);
}

void Entity__AttachToParent(Entity *self, TodActor *peer, void *companion, struct StageMap *parent,
                            void *offset) {
    if (self->parent != 0) {
        return;
    }
    ((TodActorAttachToParentFn)GetTodActorMethods()->attachToParent)((TodActor *)self, peer,
                                                                     companion, parent, offset);
    self->grid = parent;
    if (sEntityMoodTable[self->moodIndex].activateKind != 0) {
        return;
    }
    self->methods->activate(self);
    if (sEntityMoodTable[self->moodIndex].cueRange != 0) {
        return;
    }
    self->methods->startSoundCue(self);
}

void Entity__DetachFromParent(Entity *self) {
    if (self->parent != 0) {
        self->methods->deactivate(self);
        GetTodActorMethods()->detachFromParent((TodActor *)self);
        self->grid = NULL;
    }
}

void Entity__Update(Entity *self, void *sender, s32 event) {
    if (self->methods->updateActivationState(self) != 0) {
        self->methods->updateDeactivationState(self);
    }
    if (self->methods->updateSoundCueStart(self) != 0) {
        self->methods->updateSoundCueStop(self);
    }
    self->methods->updateTargetProximity(self);
    GetTodActorMethods()->update((TodActor *)self, sender, event);
}

/* A row with no link stage (0 or below) drops events 2 to 8 and sends no
 * effect. */
void Entity__NotifyLinkStage(Entity *self, void *sender, s32 event) {
    s32 linkStage;

    linkStage = sEntityMoodTable[self->moodIndex].linkStage;
    if (event >= SCENENODE_EVENT_HULL_FIRST && event <= ACTOR_EVENT_MOVED_Y) {
        if (linkStage <= 0) {
            return;
        }
    }
    GetTodActorMethods()->onActorLinkCommand((TodActor *)self, sender, event);
    if (event != SCENENODE_EVENT_LINKED) {
        return;
    }
    if (linkStage <= 0) {
        return;
    }
    /* MATCHING: the effect reuses `event`; a local of its own compiles differently */
    if (linkStage != ENTITY_LINK_STAGE_END_DREAM) {
        event = ENTITY_EFFECT_LINK_STAGE;
    } else if (sEntityMoodTable[self->moodIndex].eventVideo != 0) {
        event = ENTITY_EFFECT_EVENT_VIDEO;
    } else {
        event = ENTITY_EFFECT_END_DREAM;
    }
    self->methods->notifyParents(self, event);
}

void Entity__OnGridCellLinkCommand(Entity *self, void *sender, s32 event) {
    GetTodActorMethods()->onGridCellLinkCommand((TodActor *)self, sender, event);
    if (event == SCENENODE_EVENT_LINKED) {
        self->methods->deactivate(self);
    }
}

void Entity__TickSoundCue(Entity *self) {
    ServiceSoundCueSet(self->sound, &self->soundCueSet);
    self->moodTimer++;
}

/* 1 when `pos` (plus unlockKind * 1024 in y for an unlockKind of -9 to -1)
 * lies within `tolerance` of the point `range` ahead of the player
 * (DreamSys__ProjectPointAtDistance). Both are in ENTITY_RANGE_UNITs; a
 * negative tolerance -n means ENTITY_RANGE_UNIT / n. */
s32 Entity__IsNearTarget(Entity *self, void *pos, s32 range, s32 tolerance) {
    LongVec3 point;
    s32 kind;

    point = *(LongVec3 *)pos;
    kind = (u8)sEntityMoodTable[self->moodIndex].unlockKind;
    if ((s8)kind >= -9 && (s8)kind < 0) {
        point.y += (s8)kind * 1024;
    }
    if (tolerance < 0) {
        tolerance = ENTITY_RANGE_UNIT / (~tolerance + 1); /* MATCHING: -tolerance compiles differently */
    } else {
        tolerance <<= ENTITY_RANGE_SHIFT;
    }
    return ((DreamSys *)self->peer)
        ->methods->projectPointAtDistance((DreamSys *)self->peer, 0, range << ENTITY_RANGE_SHIFT,
                                          (s32 *)&point, tolerance);
}

s32 Entity__DistanceToPeer(Entity *self, TodActor *peer) {
    long *peerPos;
    GsCOORDINATE2 *coord;
    s32 dx;
    s32 dz;

    peerPos = NULL;
    if (peer->parent != 0) {
        peerPos = peer->coord2->workm.t;
    }
    coord = self->coord2;
    dx = coord->coord.t[0] - peerPos[0];
    if (dx < 0) {
        dx = ~dx + 1; /* MATCHING: -dx compiles differently */
    }
    dz = coord->coord.t[2] - peerPos[2];
    return (dz >= 0) ? (dx + dz) : (dx - dz);
}

/* -1 without a peer or beyond the row's proximityThreshold; otherwise the
 * player's distance in steps of threshold / attenuationSteps, 0 nearest. */
s32 Entity__GetProximityRatio(Entity *self) {
    s32 result;
    s32 threshold;

    do { /* MATCHING: a bare if compiles one word differently */
        if (self->peer == NULL) {
            return -1;
        }
    } while (0);
    result = self->methods->distanceToPeer(self, self->peer);
    threshold = sEntityMoodTable[self->moodIndex].proximityThreshold << ENTITY_RANGE_SHIFT;
    if (threshold < result) {
        return -1;
    }
    return result / (threshold / self->soundCueSet.attenuationSteps);
}

EntityMoodRow *Entity__GetMoodEffect(Entity *self) {
    return &sEntityMoodTable[self->moodIndex];
}

s32 Entity__GetUnlockEffect(Entity *self) {
    return sEntityMoodTable[self->moodIndex].unlockKind * 1000;
}

/* The stage index a linkStage names: n - 1 for a positive n, ~n for a
 * negative one (so -1 is stage 0). */
s32 Entity__GetLinkStage(Entity *self) {
    s32 linkStage = sEntityMoodTable[self->moodIndex].linkStage;

    if (linkStage < 0) {
        return ~linkStage;
    }
    return linkStage - 1;
}

s32 Entity__GetEventVideo(Entity *self) {
    return sEntityMoodTable[self->moodIndex].eventVideo - 1;
}

void Entity__Activate(Entity *self) {
    self->methods->setDisplay(self, 1);
    self->active = 1;
    self->tick = 0;
}

void Entity__Deactivate(Entity *self) {
    self->methods->setDisplay(self, 0);
    self->methods->stopSoundCue(self);
    self->methods->setTargetReached(self, 0);
    self->active = 0;
}

void Entity__SetTargetReached(Entity *self, s32 reached) {
    if (reached != 0) {
        self->methods->notifyParents(self, ENTITY_EFFECT_LOG_MOOD);
    }
    self->targetReached = reached;
}

void Entity__StartSoundCue(Entity *self) {
    InitSoundCueSet(self->sound, &self->soundCueSet, self->moodIndex + 1, self,
                    sEntityMoodTable[self->moodIndex].handler);
    ((EntityPlayTodFn)self->methods->playTod)(self);
    self->methods->enableTickCallback(self);
    self->moodTimer = 0;
    self->soundCueActive = 1;
}

void Entity__StopSoundCue(Entity *self) {
    FlushSoundCueSet(self->sound, &self->soundCueSet);
    self->methods->stopTod(self);
    self->methods->disableTickCallback(self);
    self->soundCueActive = 0;
}

s32 Entity__UpdateActivationState(Entity *self) {
    EntityMoodRow *row;
    s32 doActivate;

    if (self->active == 0 && self->state != ENTITY_STATE_DONE) {
        row = &sEntityMoodTable[self->moodIndex];
        doActivate = 0;
        if (row->activateKind != ENTITY_ACTIVATE_AT_ATTACH) {
            if (row->activateKind == ENTITY_ACTIVATE_RANDOM) {
                goto randCheck;
            }
            if (row->activeRange != 0) {
                if (Entity__IsNearTarget(self, self->coord2->coord.t, row->activeRange,
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
            self->methods->activate(self);
        }
    }
    return self->active;
}

s32 Entity__UpdateDeactivationState(Entity *self) {
    EntityMoodRow *row;
    s32 doDeactivate;
    s32 near;

    if (self->active != 0) {
        row = &sEntityMoodTable[self->moodIndex];
        doDeactivate = 0;
        Entity__NotifyIfTargetInRange(self, 0);
        if (row->deactivateKind != ENTITY_DEACTIVATE_NONE &&
            row->deactivateKind != ENTITY_DEACTIVATE_NONE_ALT) {
            if (row->deactivateKind >= ENTITY_DEACTIVATE_TIMED) {
                if (self->tick == row->deactivateKind * 15) {
                    doDeactivate = 1;
                }
            } else if (row->activeRange != 0) {
                near = Entity__IsNearTarget(self, self->coord2->coord.t, row->activeRange,
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
            self->methods->deactivate(self);
        }
    }
    return self->active;
}

/* ---- Entity's last methods; MoodCue handlers, rows 0 to 17 -------------
 *
 * The methods. Entity__Update runs the table's last three slots every tick:
 * Entity__UpdateTargetProximity (+0x178) latches targetReached through
 * setTargetReached once the player (`peer`) is within the mood row's
 * proximityRange, Entity__UpdateSoundCueStart (+0x17C) starts the sound cue
 * when the player comes within the row's cueRange, and
 * Entity__UpdateSoundCueStop (+0x180) stops it again when the player leaves
 * that range. Entity__NotifyIfTargetInRange and Entity__IsTargetInRange are
 * the range test Entity__UpdateDeactivationState makes on the row's
 * sEntityEventVideoTable entry; GetEntityMethods is the table's getter.
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
 * handler's own phases (the convention of every handler section below).
 */

s32 Entity__UpdateTargetProximity(Entity *self) {
    EntityMoodRow *row;
    long *pos;
    s32 dist;

    row = &sEntityMoodTable[self->moodIndex];
    if (self->active != 0) {
        if (self->targetReached == 0) {
            pos = self->coord2->coord.t;
            dist = row->proximityRange;
            if (dist < 0) {
                dist = ~dist + 1; /* MATCHING: -dist compiles differently */
            }
            if (Entity__IsNearTarget(self, pos, dist, row->nearTolerance) != 0) {
                self->methods->setTargetReached(self, 1);
            }
        }
        if (row->proximityRange < 0) {
            SceneNode__FaceTarget((SceneNode *)self, (SceneNode *)self->peer, 1, 0, 0);
        }
    }
    return self->targetReached;
}

s32 Entity__UpdateSoundCueStart(Entity *self) {
    EntityMoodRow *row;
    long *pos;
    s32 dist;

    if (self->active != 0 && self->soundCueActive == 0 && self->state != ENTITY_STATE_DONE) {
        row = &sEntityMoodTable[self->moodIndex];
        if (row->cueRange != 0) {
            pos = self->coord2->coord.t;
            dist = row->cueRange;
            if (dist < 0) {
                dist = ~dist + 1; /* MATCHING: -dist compiles differently */
            }
            if (Entity__IsNearTarget(self, pos, dist, row->nearTolerance) != 0) {
                self->methods->startSoundCue(self);
            }
        }
    }
    return self->soundCueActive;
}

/* For a row with no link stage (a negative sEntityLinkStageTable entry) and
 * an event video, sends ENTITY_EFFECT_LINK_STAGE while the player is within
 * the video entry times 512 world units (Entity__IsTargetInRange). `unused`
 * is entity.h's declared second parameter; the one caller,
 * Entity__UpdateDeactivationState, passes 0. */
void Entity__NotifyIfTargetInRange(Entity *self, s32 unused) {
    if (sEntityLinkStageTable[self->moodIndex * 16] < 0 &&
        sEntityEventVideoTable[self->moodIndex * 16] != 0 &&
        Entity__IsTargetInRange(self, sEntityEventVideoTable[self->moodIndex * 16] << 9)) {
        self->methods->notifyParents(self, ENTITY_EFFECT_LINK_STAGE);
    }
}

/* 1 when the player is within 512 world units of this entity's height and
 * distanceToPeer (|dx| + |dz|) is below `range`. */
s32 Entity__IsTargetInRange(Entity *self, s32 range) {
    TodActor *other;
    s32 oy, ty;

    other = self->peer;
    oy = other->coord2->coord.t[1];
    ty = self->coord2->coord.t[1];
    /* MATCHING: two ifs and a goto; one || with plain returns compiles differently */
    if (oy + 512 < ty) {
        goto fail;
    }
    if (ty < oy - 512) {
        goto fail;
    }
    if (self->methods->distanceToPeer(self, other) < range) {
        return 1;
    }
fail:
    return 0;
}

s32 Entity__UpdateSoundCueStop(Entity *self) {
    EntityMoodRow *row;
    long *pos;
    s32 dist;

    if (self->active != 0 && self->soundCueActive != 0) {
        row = &sEntityMoodTable[self->moodIndex];
        dist = row->cueRange;
        if (dist < 0) {
            dist = ~dist + 1; /* MATCHING: -dist compiles differently */
            pos = self->coord2->coord.t;
            if (Entity__IsNearTarget(self, pos, dist, row->nearTolerance) == 0) {
                self->methods->stopSoundCue(self);
            }
        }
    }
    return self->soundCueActive;
}

EntityMethods *GetEntityMethods(void) {
    return &gEntityMethods;
}

void Entity__MoodCue00(Entity *self, SoundCueSet *out) {
    if (out->tick == 0) {
        if (((DreamSys *)self->peer)->methods->getDreamColor((DreamSys *)self->peer) == DREAM_COLOR_PINK) {
            self->state = 100;
        }
    }
    out->attenuation = self->methods->getProximityRatio(self);
    if (self->state == 0) {
        if (out->tick % 10 == 0) {
            out->slots[0].program = 5;
            out->slots[0].octave = -2;
        }
        if (self->moodTimer == 2400) {
            self->moodTimer = -1; /* Entity__TickSoundCue counts it back to 0: the cycle restarts */
        } else if (self->moodTimer < 1200) {
            self->methods->moveLocalZ(self, 50, 0);
        } else {
            self->methods->moveLocalZ(self, -50, 0);
        }
    } else if (self->moodTimer < 250) {
        if (out->tick % 10 == 0) {
            out->slots[0].program = 5;
            out->slots[0].octave = -2;
        }
        if (self->moodTimer < 100) {
            self->methods->moveLocalZ(self, 50, 0);
        } else if (self->moodTimer < 250) {
            self->methods->addTranslation(self, sTranslateYPlus64ZMinus64);
        }
    } else if (self->moodTimer == 250) {
        self->methods->stopTod(self);
        out->slots[0].program = SOUND_CUE_STOP;
    } else if (self->moodTimer >= 261 && self->moodTimer < 568) {
        self->methods->moveLocalZ(self, -50, 0);
        self->methods->updateRotation(self, 1, sRotationYawMinus120);
    } else if (self->moodTimer >= 569) {
        self->methods->updateRotation(self, 1, sRotationX50YMinus120Z30);
    }
}

void Entity__MoodCue01(Entity *self, SoundCueSet *out) {
    out->attenuation = 0;
    if (out->tick == 0) {
        out->slots[0].program = 20;
        out->slots[1].program = 20;
        out->slots[2].program = 20;
        ((DreamSys *)self->peer)->methods->clearTickCallbacks((DreamSys *)self->peer, 1);
    }
    SceneNode__FaceTarget((SceneNode *)self, (SceneNode *)self->peer, 1, 0, 0);
    self->methods->moveLocalZ(self, -90, 0);
    if (self->moodTimer == 30) {
        self->methods->notifyParents(self, ENTITY_EFFECT_LINK_STAGE);
    }
}

void Entity__MoodCue05(Entity *self, SoundCueSet *out) {
    out->attenuation = self->methods->getProximityRatio(self);
    if (out->tick == 0) {
        out->slots[0].program = 23;
    }
}

void Entity__MoodCue07(Entity *self, SoundCueSet *out) {
    out->attenuation = self->methods->getProximityRatio(self);
    if (self->todFrame == self->todFrameCount / 2) {
        out->slots[0].program = 7;
        out->slots[0].octave = -2;
        out->slots[1].program = 3;
        out->slots[1].octave = -2;
    }
    if (out->tick % 90 < 3) {
        out->slots[2].program = 6;
        out->slots[2].octave = -1;
    }
    if (self->moodTimer >= 121) {
        self->methods->updateRotation(self, 0, sRotationYawPlus2);
        self->methods->moveLocalZ(self, -320, 0);
    } else if (self->moodTimer >= 56 || Entity__IsNearTarget(self, self->coord2->coord.t, 1, 1) != 0) {
        self->methods->addTranslation(self, sTranslateYMinus64);
    } else if (self->moodTimer >= 10) {
        SceneNode__FaceTarget((SceneNode *)self, (SceneNode *)self->peer, 1, 0, 0);
        self->methods->moveLocalZ(self, -256, 0);
    } else {
        SceneNode__FaceTarget((SceneNode *)self, (SceneNode *)self->peer, 1, 0, 0);
    }
}

void Entity__MoodCue08(Entity *self) {
    self->methods->updateScale(self, 1, sScaleDouble);
    self->methods->addTranslation(self, sTranslateYMinus64);
}

void Entity__MoodCue09(Entity *self, SoundCueSet *out) {
    out->attenuation = self->methods->getProximityRatio(self);
    if (out->tick % (self->todFrameCount / 2) == 0) {
        out->slots[0].program = 10;
    }
    self->methods->moveLocalZ(self, -30, 0);
}

void Entity__MoodCue10(Entity *self, SoundCueSet *out) {
    out->attenuation = 0;
    if (out->tick == 0) {
        out->slots[0].program = 11;
        out->slots[1].program = 11;
        out->slots[2].program = 11;
    }
    self->methods->moveLocalZ(self, -30, 0);
}

void Entity__MoodCue11(Entity *self, SoundCueSet *out) {
    Ratio16 *turn;

    self->lastOffsetValue = -20;
    out->attenuation = self->methods->getProximityRatio(self);
    turn = NULL;
    if (out->tick % (self->todFrameCount / 2) == 0) {
        out->slots[0].program = 10;
        out->slots[0].octave = 1;
    }
    if (self->state == 11) {
        if (self->moodTimer == 2700) {
            turn = sRotationYawMinus90;
        }
        if (self->moodTimer == 3180) {
            turn = sRotationYawPlus90;
        }
        if (self->moodTimer == 3600) {
            turn = sRotationYawMinus90;
        }
        if (self->moodTimer >= 3421 && self->moodTimer < 3541) {
            if (((DreamSys *)self->peer)->methods->getLinkCommandFlag((DreamSys *)self->peer) != 0) {
                self->moodTimer = 0;
                self->state = 13;
            }
        }
    } else if (self->state == 12) {
        if (self->moodTimer == 1980) {
            turn = sRotationYawMinus90;
        }
    } else if (self->state == 13) {
        self->lastOffsetValue = -120;
        SceneNode__FaceTarget((SceneNode *)self, (SceneNode *)self->peer, 1, 0, 0);
        self->methods->updateScale(self, 1, sScaleHalf);
        if (self->methods->distanceToPeer(self, self->peer) < 1024) {
            self->methods->notifyParents(self, ENTITY_EFFECT_EVENT_VIDEO);
        }
    }
    if (self->moodTimer == 1560) {
        if ((rand() & 1) != 0) {
            turn = sRotationYawPlus90;
            self->state = 11;
        } else {
            turn = sRotationYawMinus90;
            self->state = 12;
        }
    }
    if (turn != NULL) {
        self->methods->updateRotation(self, 0, turn);
    }
    self->methods->moveLocalZOrFindLink(self, self->lastOffsetValue, 0);
    if (self->state != 12) {
        if (self->linkTarget != 0) {
            self->methods->moveLocalY(self, -200, 0);
        }
    }
}

void Entity__MoodCue12(Entity *self) {
    s32 y;
    s32 dist;
    s32 timer;

    if (self->moodTimer == 0) {
        if ((rand() & 1) == 0) {
            self->state = 11;
        }
    }
    y = self->coord2->coord.t[1];
    if (y < 2000) {
        SceneNode__FaceTarget((SceneNode *)self, (SceneNode *)self->peer, 1, 0, 0);
    }
    if (self->state == 11) {
        dist = self->methods->distanceToPeer(self, self->peer);
        if (dist < 2560) {
            self->grid->methods->startScaleRamp(self->grid, 1, 1);
            self->moodTimer = 1;
            self->state = 12;
        }
    } else if (self->state == 12) {
        /* Counted here as well as by Entity__TickSoundCue: two per tick. */
        timer = self->moodTimer;
        self->moodTimer = timer + 1;
        if (timer == 300) {
            self->methods->notifyParents(self, ENTITY_EFFECT_END_DREAM);
        }
    }
}

void Entity__MoodCue13(Entity *self, SoundCueSet *out) {
    out->attenuation = self->methods->getProximityRatio(self);
    if (out->tick == 0) {
        out->slots[0].program = 12;
        self->state++;
    } else if (out->tick >= self->todFrameCount - 1) {
        out->tick = -1; /* the cue's tick restarts every todFrameCount ticks */
    }
    if (self->state == 36) {
        if (rand() % 3 == 0) {
            self->methods->notifyParents(self, ENTITY_EFFECT_EVENT_VIDEO);
        }
    }
}

void Entity__MoodCue14(Entity *self, SoundCueSet *out) {
    out->attenuation = self->methods->getProximityRatio(self);
    if (self->todFrame == 10) {
        out->slots[0].program = 13;
    }
    self->methods->moveLocalZ(self, -10, 0);
}

void Entity__MoodCue15(Entity *self, SoundCueSet *out) {
    if (self->moodTimer == 0) {
        out->attenuation = 0;
        out->slots[0].program = 15;
    }
}

void Entity__MoodCue16(Entity *self) {
    Ratio16 *turn;
    s32 roll;

    if (self->moodTimer == 0) {
        if ((rand() & 1) != 0) {
            self->state = 11;
        }
    }
    if (self->state == 0) {
        if (self->moodTimer < 64) {
            self->methods->moveLocalZ(self, -90, 0);
        } else if (self->moodTimer == 64) {
            roll = rand() & 1;
            turn = sRotationYawMinus90;
            if (roll != 0) {
                turn = sRotationYawPlus90;
            }
            self->methods->updateRotation(self, 0, turn);
            self->methods->addTranslation(self, sTranslateYPlus256);
        } else {
            self->methods->moveLocalZOrFindLink(self, -374, (void *)(rand() % 2));
        }
    } else if (self->state == 11) {
        if (self->moodTimer % 5 == 0) {
            self->methods->updateRotation(self, 0, sRotationYawPlus90);
        }
        self->methods->moveLocalZ(self, -2048, 0);
        self->methods->setDisplay(self, (rand() % 7) == 0);
    }
}

void Entity__MoodCue17(Entity *self) {
    self->methods->updateScale(self, 1, sScaleHalf);
}

/* ---- MoodCue handlers, rows 19 to 38 and 119 ---------------------------
 *
 * All 20 functions are `gEntityMoodHandlerTable` mood-dispatch callbacks,
 * `Entity__MoodCueNN` where NN is the table row (`asm/data/79528.data.s`,
 * stride 0x10), as in every handler section.
 * Row order does not track code address, so this section's rows (19-27, 29-38,
 * plus 119) are not contiguous with each other or with source order;
 * `Entity__MoodCue119` sits far from its neighbours by address alone,
 * confirmed against the table rather than assumed from proximity.
 * `Entity__MoodCue30` additionally occupies row 122 with the same handler
 * and different data words -- one function shared by two distinct mood-row
 * configurations, named for its lower row (same precedent as
 * `Entity__MoodCue81`, below).
 *
 * Fields and slots are the unified Entity's (include/entity.h): the
 * inherited ones carry TodActor's, Actor's and SceneNode's names (`state`,
 * `linkTarget`, `peer`, moveLocalZ/X/Y, moveLocalZOrFindLink, ...), Entity's
 * own are named for their occupants.
 *
 * The literals are left unnamed where they are one handler's tuning: tick
 * counts, distances in world units, TOD frame numbers, VAB program numbers,
 * and the `state` values other than 0 and ENTITY_STATE_DONE, which are each
 * handler's own phases.
 */

void Entity__MoodCue19(Entity *self, SoundCueSet *out) {
    out->attenuation = self->methods->getProximityRatio(self);
    if (out->tick % 10 == 0) {
        out->slots[0].program = 17;
    }
    self->methods->moveLocalZ(self, -256, 0);
}

void Entity__MoodCue20(Entity *self, SoundCueSet *out) {
    if (self->moodTimer == 0 && rand() % 7 == 0) {
        self->methods->updateScale(self, 1, sScaleY2);
    }
    if ((out->tick & 3) == 0) {
        out->attenuation = self->methods->getProximityRatio(self);
        out->slots[0].program = 28;
    }
    self->methods->moveLocalZ(self, -100, 0);
}

void Entity__MoodCue21(Entity *self, SoundCueSet *out) {
    s32 half;
    s32 rem;

    out->attenuation = self->methods->getProximityRatio(self);
    half = self->todFrameCount / 2;
    rem = out->tick % half;
    if (rem == 0) {
        out->slots[0].program = 10;
    } else if (rem == 3) {
        out->slots[1].program = 13;
    }
    self->methods->moveLocalZ(self, -30, (void *)1);
}

void Entity__MoodCue22(Entity *self) {
    SceneNode__FaceTarget((SceneNode *)self, (SceneNode *)self->peer, 1, 0, 0);
}

void Entity__MoodCue23(Entity *self) {
    s32 zDelta;

    SceneNode__FaceTarget((SceneNode *)self, (SceneNode *)self->peer, 1, 0, 0);

    if (self->targetReached != 0) {
        if (self->moodTimer >= 65) {
            self->moodTimer = 0;
        }
        if (self->moodTimer >= 7) {
            self->methods->addTranslation(self, sTranslateYMinus64);
            self->methods->moveLocalZ(self, 10, 0);
        } else {
            self->methods->addTranslation(self, sTranslateXMinus64);
        }
    } else if (self->moodTimer == 0) {
        self->methods->addTranslation(self, sTranslateYMinus4096);
    } else if (self->moodTimer < 65) {
        self->methods->addTranslation(self, sTranslateYPlus64);
    } else if (self->moodTimer < 71) {
        self->methods->addTranslation(self, sTranslateYMinus64);
        self->methods->moveLocalZ(self, -30, 0);
    } else {
        EntityMethods *methods = self->methods;

        if (self->moodTimer < 256) {
            zDelta = -self->moodTimer - 65;
        } else {
            zDelta = 255;
        }
        methods->moveLocalZ(self, zDelta, 0);
    }
}

void Entity__MoodCue24(Entity *self, SoundCueSet *out) {
    if (out->tick % 15 == 0) {
        out->attenuation = self->methods->getProximityRatio(self);
        out->slots[0].program = 7;
        out->slots[0].octave = -2;
    }
    self->methods->updateScale(self, 1, sScaleDouble);
    self->methods->updateRotation(self, 0, sRotationYawPlus2);
    self->methods->moveLocalZ(self, -512, 0);
}

void Entity__MoodCue25(Entity *self, SoundCueSet *out) {
    out->attenuation = 0;
    if (self->moodTimer < 100) {
        if (out->tick % 3 == 0) {
            out->slots[0].program = 22;
            out->slots[0].octave = 1;
        }
        self->methods->moveLocalY(self, -64, 0);
    } else if (self->moodTimer < 300) {
        out->slots[0].program = 12;
        out->slots[0].octave = -1;
        out->slots[1].program = 12;
        out->slots[1].octave = -1;
        out->slots[2].program = 12;
        out->slots[2].octave = -1;
        self->methods->moveLocalY(self, -256, 0);
    } else {
        self->methods->updateRotation(self, 0, sRotationXPlusEighth);
        self->methods->moveLocalY(self, -512, 0);
    }
}

void Entity__MoodCue26(Entity *self, SoundCueSet *out) {
    s32 v1;
    s32 zDelta;
    void (**moveZOrFindLink)(Entity *self, s32 val, void *notify);

    /* The do/while(0) wrapper is a no-op scoping device, load-bearing for
     * register allocation only -- see the match report. Without it GCC
     * swaps which callee-saved register holds `self` vs `out` for the
     * whole function. */
    do {
        if (out->tick % self->todFrameCount == 0) {
            out->attenuation = self->methods->getProximityRatio(self);
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
    moveZOrFindLink = &self->methods->moveLocalZOrFindLink;
    zDelta = -384;
    if (self->moodTimer == v1) {
        zDelta = -11520;
    }
    (*moveZOrFindLink)(self, zDelta, 0);
}

void Entity__MoodCue27(Entity *self, SoundCueSet *out) {
    if (out->tick % 70 == 0) {
        out->attenuation = 0;
        out->slots[0].program = 27;
    }
    self->methods->moveLocalZ(self, -128, 0);
    if (self->moodTimer < 100) {
        self->methods->moveLocalY(self, 32, 0);
    } else if (self->moodTimer >= 301) {
        self->methods->moveLocalY(self, -32, 0);
    }
}

void Entity__MoodCue119(Entity *self) {
    self->methods->updateScale(self, 1, sScaleSix);
}

void Entity__MoodCue29(Entity *self) {
    if (self->moodTimer == 0) {
        if (((DreamSys *)self->peer)->methods->getDreamColor((DreamSys *)self->peer) == DREAM_COLOR_WHITE) {
            self->methods->updateScale(self, 1, sScaleTriple);
            self->methods->moveLocalY(self, -30720, 0);
        }
        self->state = rand() % 5;
    }
    if (self->state == 0) {
        self->methods->updateRotation(self, 0, sRotationYawPlus1);
    }
}

void Entity__MoodCue30(Entity *self) {
    if (self->state == 0) {
        if (((DreamSys *)self->peer)->methods->getDreamColor((DreamSys *)self->peer) == 1) {
            self->state = 11;
        } else {
            self->state = 12;
        }
    }

    if (self->state == 12) {
        self->methods->updateScale(self, 1, sScaleDouble);
        self->methods->moveLocalY(self, -30, 0);
    } else {
        SceneNode__FaceTarget((SceneNode *)self, (SceneNode *)self->peer, 1, 0, 0);
        if (self->state == 11) {
            self->methods->moveLocalZ(self, -100, 0);
            if ((u32)(self->moodTimer - 85) < 30) {
                self->methods->moveLocalY(self, 80, 0);
            } else if (self->moodTimer == 120) {
                self->state = 13;
            }
        } else if (self->state == 13) {
            self->methods->setTranslation(self, (LongVec3 *)((DreamSys *)self->peer)->coord2->coord.t);
            self->methods->addTranslation(self, sTranslateYMinus1500ZPlus1024);
        }
    }
}

void Entity__MoodCue31(Entity *self, SoundCueSet *out) {
    SceneNode__FaceTarget((SceneNode *)self, (SceneNode *)self->peer, 1, 0, 0);
    ((DreamSys *)self->peer)->methods->clearTickCallbacks((DreamSys *)self->peer, 1);
    if ((out->tick % 10) < 3) {
        out->attenuation = 0;
        out->slots[0].program = 13;
        out->slots[1].program = 13;
        out->slots[2].program = 13;
    }
    if (self->moodTimer == self->todFrameCount) {
        self->methods->stopTod(self);
        self->methods->notifyParents(self, ENTITY_EFFECT_LINK_STAGE);
    }
}

void Entity__MoodCue32(Entity *self) {
    self->methods->moveLocalZ(self, -30, 0);
}

void Entity__MoodCue33(Entity *self, SoundCueSet *out) {
    if (self->targetReached != 0) {
        self->methods->updateScale(self, 1, sScaleQuarter);
    } else if (out->tick % 30 == 0) {
        out->attenuation = 0;
        out->slots[0].program = 3;
    }
    self->methods->moveLocalZOrFindLink(self, -30, 0);
    if (self->linkTarget != 0) {
        self->methods->moveLocalY(self, -200, 0);
    }
}

void Entity__MoodCue34(Entity *self) {
    EntityMethods *methods;
    s32 zDelta;

    if ((u32)(self->moodTimer - 400) < 10) {
        self->methods->updateRotation(self, 0, sRotationYawPlus9);
    } else if ((u32)(self->moodTimer - 700) < 10) {
        self->methods->updateRotation(self, 0, sRotationYawMinus9);
    } else if ((u32)(self->moodTimer - 830) < 4) {
        self->methods->updateRotation(self, 0, sRotationYawMinus9);
    } else if (self->moodTimer >= 851) {
        self->methods->deactivate(self);
    }
    methods = self->methods;
    zDelta = -512;
    if (self->moodTimer < 800) {
        zDelta = -60;
    }
    methods->moveLocalZ(self, zDelta, (void *)1);
}

void Entity__MoodCue35(Entity *self) {
    s32 rem500;
    s32 arg1a;
    s32 arg1b;
    s32 arg1c;
    void (**moveY)(Entity *self, s32 val, void *notify);
    void (**moveX)(Entity *self, s32 val, void *notify);
    void (**moveZ)(Entity *self, s32 val, void *notify);

    rem500 = self->moodTimer % 500;

    moveY = &self->methods->moveLocalY;
    if (self->moodTimer % 6 < 3) {
        arg1a = -64;
    } else {
        arg1a = 64;
    }
    (*moveY)(self, arg1a, 0);

    moveX = &self->methods->moveLocalX;
    if (self->moodTimer % 12 < 6) {
        arg1b = -64;
    } else {
        arg1b = 64;
    }
    (*moveX)(self, arg1b, 0);

    moveZ = &self->methods->moveLocalZ;
    if (self->moodTimer % 64 < 32) {
        arg1c = -128;
    } else {
        arg1c = 128;
    }
    (*moveZ)(self, arg1c, 0);

    if (rem500 < 32) {
        self->methods->addTranslation(self, sTranslateYMinus64);
    } else if (rem500 < 64) {
        self->methods->addTranslation(self, sTranslateYPlus64);
    }
}

void Entity__MoodCue36(Entity *self) {
    Ratio16 *scaleTemplate;
    s32 roll;

    if (self->state == 0) {
        roll = rand();
        scaleTemplate = sScaleSix;
        if ((roll & 1) != 0) {
            scaleTemplate = sScaleDouble;
        }
        self->methods->updateScale(self, 1, scaleTemplate);
        self->state = 11;
    }
    SceneNode__FaceTarget((SceneNode *)self, (SceneNode *)self->peer, 1, 0, 0);
    if (self->methods->distanceToPeer(self, self->peer) < 28672) {
        self->methods->moveLocalZ(self, 256, 0);
    }
}

void Entity__MoodCue37(Entity *self) {
    self->methods->moveLocalY(self, -90, 0);
}

void Entity__MoodCue38(Entity *self, SoundCueSet *out) {
    if (out->tick % 120 == 0) {
        out->attenuation = self->methods->getProximityRatio(self);
        out->slots[0].program = 1;
    }
}

/* ---- MoodCue handlers, rows 39 to 58 and 115 ---------------------------
 *
 * Nineteen of Entity's MoodCue handlers and the helper two of them share.
 *
 * Each Entity__MoodCueNN is the `handler` of gEntityMoodHandlerTable's row
 * NN (include/entity.h): rows 39 to 52, 55 to 58 and 115. An Entity whose
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
 * end here (the range splat labels sScaleX3 runs on into its first ten
 * bytes). Entity__MoodCue41 writes the den and passes the template. */
extern s16 sScaleTemplateZDenom;

/* Defined after Entity__MoodCue43, which calls it. */
void Entity__RollScaleOrDelayedDrift(Entity *self);

void Entity__MoodCue39(Entity *self, SoundCueSet *out) {
    s32 dayYearPhase;

    if (self->moodTimer == 0) {
        dayYearPhase =
            ((DreamSys *)self->peer)->methods->getCurrentDayAndYear((DreamSys *)self->peer, 0) % 3;
        if (dayYearPhase == 0) {
            if (rand() % 3 != 0) {
                goto skipScaleBump;
            }
        } else if (dayYearPhase != 2) {
            goto skipScaleBump;
        }
        self->methods->updateScale(self, 1, sScaleY4);
    }
skipScaleBump:
    if (out->tick % 22 == 0) {
        out->attenuation = self->methods->getProximityRatio(self);
        out->slots[0].program = 2;
    }
    if (rand() % 12 == 0) {
        self->methods->stopTod(self);
    } else if (rand() % 6 == 0) {
        ((EntityPlayTodFn)self->methods->playTod)(self);
    }
}

void Entity__MoodCue40(Entity *self, SoundCueSet *out) {
    Ratio16 *table = NULL;

    if (out->tick % 7 == 0) {
        out->attenuation = self->methods->getProximityRatio(self);
        out->slots[0].program = 3;
        out->slots[0].vol = 64;
        out->slots[0].endVol = 64;
    }
    if (self->moodTimer == 200) {
        table = sRotationYawMinus90;
    } else if (self->moodTimer == 400) {
        table = sRotationYawPlus180;
    } else if (self->moodTimer == 600) {
        table = sRotationYawPlus90;
    } else if (self->moodTimer == 800) {
        table = sRotationYawPlus180;
        self->moodTimer = -1;
    }
    if (table != NULL) {
        self->methods->updateRotation(self, 0, table);
    }
    self->methods->moveLocalZOrFindLink(self, -30, 0);
    if (self->linkTarget != 0) {
        self->methods->moveLocalY(self, -200, 0);
    }
}

void Entity__MoodCue41(Entity *self, SoundCueSet *out) {
    s32 roll;
    s16 *zDenom;

    if (self->moodTimer == 0) {
        self->state = rand() % 5 + 10;
    }
    if (self->state < 14 || self->moodTimer < 320) {
        Entity__StepYawInWindowsThenDeactivate(self, out, 3000, 500, -256);
        return;
    }
    if (self->state == 14) {
        if ((self->moodTimer & 3) == 0) {
            roll = rand();
            zDenom = &sScaleTemplateZDenom;
            *zDenom = roll % 32 + 1;
            /* Back from the den to the start of its template. MATCHING:
             * retail relocates against sScaleTemplateZDenom, not sScaleX3. */
            self->methods->updateScale(self, 1, (Ratio16 *)(zDenom + 1) - 3);
        }
    }
}

void Entity__MoodCue42(Entity *self, SoundCueSet *out) {
    s32 divisor;

    if (self->moodTimer < 20) {
        self->methods->stopTod(self);
        self->methods->moveLocalZ(self, -30, 0);
    } else if (self->moodTimer == 20) {
        ((EntityPlayTodFn)self->methods->playTod)(self);
        out->attenuation = 0;
        out->slots[0].program = 5;
    } else {
        divisor = self->todFrameCount * 3 + 20;
        if (self->moodTimer % divisor == 0) {
            self->methods->stopTod(self);
            out->slots[0].program = SOUND_CUE_STOP;
        }
    }
}

void Entity__MoodCue43(Entity *self, SoundCueSet *out) {
    s32 dx;
    s32 rotPick;
    Ratio16 *table;

    Entity__RollScaleOrDelayedDrift(self);
    out->attenuation = self->methods->getProximityRatio(self);
    if (self->todFrame == 0 || self->todFrame == 15) {
        out->slots[0].program = 18;
        out->slots[1].program = 18;
    }
    if (self->moodTimer >= 321) {
        dx = (rand() & 1) ? -60 : 60;
        self->methods->moveLocalX(self, dx, 0);
        rotPick = rand();
        table = sRotationYawPlus9;
        if ((rotPick & 3) != 0) {
            table = sRotationYawMinus9;
        }
        self->methods->updateRotation(self, 0, table);
    }
}

void Entity__MoodCue44(Entity *self, SoundCueSet *out) {
    s32 dx;
    s32 dxPick;
    s32 rotPick;
    Ratio16 *table;

    Entity__RollScaleOrDelayedDrift(self);
    out->attenuation = self->methods->getProximityRatio(self);
    if (self->todFrame == 7 || self->todFrame == 22) {
        out->slots[0].program = 3;
    }
    if (self->moodTimer >= 300 && self->moodTimer < 320) {
        self->methods->moveLocalZ(self, -60, 0);
    } else if (self->moodTimer >= 321 && self->moodTimer < 340) {
        self->methods->updateRotation(self, 0, sRotationYawMinus9);
    } else if (self->moodTimer >= 321) {
        dxPick = rand();
        dx = -128;
        if ((dxPick & 1) != 0) {
            dx = 128;
        }
        self->methods->moveLocalX(self, dx, (void *)1);
        rotPick = rand();
        table = sRotationYawPlus9;
        if ((rotPick & 3) != 0) {
            table = sRotationYawMinus9;
        }
        self->methods->updateRotation(self, 0, table);
    }
}

/* At the cue's start, rolls 0..9: 8 or 9 stretches the entity by
 * sScaleX3, 5 to 7 arms a drift (state 10) that adds sTranslateZMinus256
 * every tick from tick 201 on. */
void Entity__RollScaleOrDelayedDrift(Entity *self) {
    s32 roll;

    if (self->moodTimer == 0) {
        roll = rand() % 10;
        if (roll >= 8) {
            self->methods->updateScale(self, 1, sScaleX3);
        } else if (roll >= 5) {
            self->state = 10;
        }
    }
    if (self->state == 10 && self->moodTimer >= 201) {
        self->methods->addTranslation(self, sTranslateZMinus256);
    }
}

/* Row 45 has no per-tick effect. */
void Entity__MoodCue45(void) {}

void Entity__MoodCue46(Entity *self, SoundCueSet *out) {
    s32 dayYearPhase;

    if (self->moodTimer == 0) {
        dayYearPhase =
            ((DreamSys *)self->peer)->methods->getCurrentDayAndYear((DreamSys *)self->peer, 0) % 3;
        if (dayYearPhase == 0) {
            if (rand() % 3 != 0) {
                goto skipScaleBump;
            }
        } else if (dayYearPhase != 1) {
            goto skipScaleBump;
        }
        self->methods->updateScale(self, 1, sScaleSix);
    }
skipScaleBump:
    if (out->tick == 0) {
        out->attenuation = 0;
        out->slots[0].program = 18;
    }
    SceneNode__FaceTarget((SceneNode *)self, (SceneNode *)self->peer, 1, 0, 0);
}

void Entity__MoodCue47(Entity *self) {
    if (self->targetReached == 0) {
        return;
    }
    if (self->state == 0) {
        self->state = 12;
        self->moodTimer = 0;
        return;
    }
    if (self->state == 12) {
        if (self->moodTimer < 30) {
            if (((DreamSys *)self->peer)->methods->getLinkCommandFlag((DreamSys *)self->peer) != 0) {
                ((DreamSys *)self->peer)->methods->clearTickCallbacks((DreamSys *)self->peer, false);
                self->moodTimer = 0;
                self->state = 11;
            }
        } else {
            self->methods->notifyParents(self, ENTITY_EFFECT_EVENT_VIDEO);
            self->state = 10;
        }
    } else if (self->state == 11) {
        if (self->moodTimer == 100) {
            self->methods->notifyParents(self, ENTITY_EFFECT_END_DREAM);
        } else {
            ((DreamSys *)self->peer)->methods->moveLocalY((DreamSys *)self->peer, -100, 0);
        }
    }
}

void Entity__MoodCue48(Entity *self, SoundCueSet *out) {
    s32 dy;
    EntityMethods *methods;

    if (self->todFrame == 38) {
        SceneNode__FaceTarget((SceneNode *)self, (SceneNode *)self->peer, 1, 0, 0);
        out->attenuation = self->methods->getProximityRatio(self);
        out->slots[0].program = 6;
    }
    methods = self->methods;
    dy = (self->moodTimer % 10 < 5) ? -30 : 30;
    methods->moveLocalY(self, dy, 0);
    self->methods->moveLocalZ(self, -30, (void *)1);
}

void Entity__MoodCue49(Entity *self, SoundCueSet *out) {
    DreamSysMethods *peerMethods;
    void *translation;

    if (out->tick == 6) {
        out->attenuation = 0;
        out->slots[0].program = 4;
        out->slots[1].program = 4;
        out->slots[2].program = 4;
    }
    if (self->targetReached != 0) {
        if (self->state == 0) {
            self->state = 10;
            self->moodTimer = 0;
        } else if (self->state == 10) {
            if (self->moodTimer == 10) {
                self->methods->notifyParents(self, ENTITY_EFFECT_LINK_STAGE);
            } else if (((DreamSys *)self->peer)->methods->getLinkCommandFlag((DreamSys *)self->peer) != 0) {
                peerMethods = ((DreamSys *)self->peer)->methods;
                translation = self->parent ? self->coord2->workm.t : NULL;
                peerMethods->setTranslation((DreamSys *)self->peer, translation);
                ((DreamSys *)self->peer)->methods->updateRotation((DreamSys *)self->peer, 1, sRotationYawMinus90);
                ((DreamSys *)self->peer)->methods->clearTickCallbacks((DreamSys *)self->peer, false);
                self->moodTimer = 0;
                self->state = 11;
            }
        } else if (self->state == 11) {
            peerMethods = ((DreamSys *)self->peer)->methods;
            translation = self->parent ? self->coord2->workm.t : NULL;
            peerMethods->setTranslation((DreamSys *)self->peer, translation);
            if (self->moodTimer == 100) {
                self->methods->notifyParents(self, ENTITY_EFFECT_LINK_STAGE);
            }
        }
    }
    self->methods->moveLocalZ(self, -256, 0);
}

void Entity__MoodCue50(Entity *self, SoundCueSet *out) {
    if (self->moodTimer < self->todFrameCount * 5) {
        if (self->todFrame == 15 || self->todFrame == 70) {
            out->attenuation = 0;
            out->slots[0].program = 7;
            out->slots[1].program = 7;
            out->slots[2].program = 7;
        }
    } else {
        self->methods->deactivate(self);
        self->state = ENTITY_STATE_DONE;
    }
}

void Entity__MoodCue51(Entity *self, SoundCueSet *out) {
    Ratio16 *table;

    if (self->moodTimer == 0 && rand() % 5 == 0 && self->state == 0) {
        self->methods->updateScale(self, 1, sScaleSix);
        self->methods->moveLocalY(self, 800, 0);
        self->state = 11;
    }
    table = NULL;
    if (out->tick % 5 == 0) {
        out->attenuation = self->methods->getProximityRatio(self);
        out->slots[0].program = 8;
    }
    if (self->moodTimer == 90) {
        table = sRotationYawMinus90;
    } else if (self->moodTimer == 160) {
        table = sRotationYawPlus90;
    } else if (self->moodTimer == 220) {
        if (rand() & 1) {
            table = sRotationYawPlus180;
        }
    }
    if (table != NULL) {
        self->methods->updateRotation(self, 0, table);
    }
    self->methods->moveLocalZ(self, -80, (void *)1);
}

void Entity__MoodCue52(Entity *self, SoundCueSet *out) {
    out->attenuation = self->methods->getProximityRatio(self);
    if (self->moodTimer < 188) {
        if (self->moodTimer == 84) {
            self->methods->updateRotation(self, 0, sRotationYawPlus180);
        }
        if (out->tick % 20 == 0) {
            out->slots[0].program = 9;
        }
    } else if (self->moodTimer < 200) {
        self->methods->updateRotation(self, 0, sRotationYawPlus9);
    } else {
        self->methods->deactivate(self);
        out->slots[1].program = 30;
        self->state = ENTITY_STATE_DONE;
    }
    self->methods->moveLocalZOrFindLink(self, -512, 0);
}

void Entity__MoodCue55(Entity *self, SoundCueSet *out) {
    s32 frame = self->todFrame;

    if (self->moodTimer == 0) {
        if (rand() % 3 == 0) {
            self->methods->updateScale(self, 1, sScaleY2);
        }
    }
    out->attenuation = self->methods->getProximityRatio(self);
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

void Entity__MoodCue56(Entity *self) {
    if (self->moodTimer == 0) {
        self->methods->moveLocalY(self, -200, 0);
    }
}

void Entity__MoodCue57(Entity *self, SoundCueSet *out) {
    s32 frame;

    if (self->moodTimer == 0) {
        self->state = rand() % 3;
        if (self->state == 0) {
            self->methods->stopTod(self);
            self->methods->moveLocalY(self, 6144, 0);
        }
    }
    out->attenuation = self->methods->getProximityRatio(self);
    if (self->state != 0) {
        frame = self->todFrame;
        if (frame < 30) {
            out->slots[0].program = 12;
            out->slots[0].octave = -1;
        } else if (frame == 30) {
            out->slots[0].program = SOUND_CUE_STOP;
        } else if (frame == 35) {
            out->slots[2].program = 22;
            out->slots[2].octave = -2;
        } else if (frame == 48) {
            if (Entity__IsNearTarget(self, self->coord2->coord.t, 15, 10)) {
                if (Entity__GetOrCreateFadeBox(self, NULL, NULL, (void *)10, 0) != NULL) {
                    self->fadeBox->methods->startFadeDown(self->fadeBox, (BasicClass *)self->ticker,
                                                          4, 0);
                }
                if (rand() & 1) {
                    self->methods->notifyParents(self, ENTITY_EFFECT_EVENT_VIDEO);
                }
            }
        } else if (frame == 59) {
            self->methods->deactivate(self);
            self->state = ENTITY_STATE_DONE;
        }
        return;
    }
    out->slots[0].program = 12;
    out->slots[0].octave = -1;
    self->methods->moveLocalZ(self, -512, 0);
    if (self->moodTimer >= 128 && self->moodTimer < 322) {
        self->methods->moveLocalY(self, -128, 0);
    } else if (self->moodTimer == 322) {
        ((EntityPlayTodFn)self->methods->playTod)(self);
        self->state = ENTITY_STATE_DONE;
    }
}

void Entity__MoodCue58(Entity *self, SoundCueSet *out) {
    if (self->moodTimer == 0) {
        out->attenuation = 0;
        out->slots[0].program = 12;
        if (((DreamSys *)self->peer)->methods->getDreamColor((DreamSys *)self->peer) ==
            DREAM_COLOR_YELLOW) {
            self->state = 11;
        } else if (rand() % 3 == 0) {
            self->state = 12;
        }
    }
    if (out->tick % 100 == 0) {
        out->attenuation = self->methods->getProximityRatio(self);
        out->slots[0].program = 12;
        out->slots[0].octave = -1;
    }
    if (self->state == 11) {
        if (self->methods->distanceToPeer(self, self->peer) < 1024) {
            ((DreamSys *)self->peer)->methods->clearTickCallbacks((DreamSys *)self->peer, false);
            self->state = 13;
            self->moodTimer = 0;
        }
    } else if (self->state == 12) {
        if (self->methods->distanceToPeer(self, self->peer) < 1024) {
            self->methods->stopTod(self);
            self->state = 14;
            self->moodTimer = 0;
        }
    }
    if (self->state == 13) {
        if (self->moodTimer < 50) {
            ((DreamSys *)self->peer)->methods->moveLocalY((DreamSys *)self->peer, -20, 0);
        } else if (self->moodTimer < 500) {
            ((DreamSys *)self->peer)
                ->methods->moveLocalX((DreamSys *)self->peer, (self->moodTimer % 40 < 20) ? -5 : 5, 0);
        } else if (self->moodTimer == 500) {
            self->methods->notifyParents(self, ENTITY_EFFECT_END_DREAM);
        }
    }
    if (self->state == 14) {
        if (self->moodTimer < 10) {
            self->methods->moveLocalY(self, 200, 0);
            return;
        }
        if (self->moodTimer == 10) {
            out->slots[0].program = 18;
            out->attenuation = 0;
            out->slots[1].program = 3;
            self->methods->updateRotation(self, 1, sRotationZMinus90);
            self->methods->moveLocalX(self, 2400, 0);
            self->methods->moveLocalY(self, 1500, 0);
            self->parts[1]->methods->setDisplay(self->parts[1], 0);
            self->state = ENTITY_STATE_DONE;
        }
    }
}

void Entity__MoodCue115(Entity *self, SoundCueSet *out) {
    if (self->state == 0) {
        if (Entity__IsTargetInRange(self, 2048) != 0) {
            self->state = 11;
            SceneNode__FaceTarget((SceneNode *)self, (SceneNode *)self->peer, 1, 0, 0);
            SceneNode__FaceTarget((SceneNode *)self->peer, (SceneNode *)self, 1, 1, 0);
            self->methods->activate(self);
            self->methods->startSoundCue(self);
            ((DreamSys *)self->peer)->methods->clearTickCallbacks((DreamSys *)self->peer, true);
            out->attenuation = 0;
            out->slots[0].program = 12;
            self->moodTimer = 0;
        }
    }
    if (self->state == 0) {
        self->methods->deactivate(self);
        self->methods->stopSoundCue(self);
        goto tail;
    }
    if (out->tick % 100 == 0) {
        out->attenuation = self->methods->getProximityRatio(self);
        out->slots[0].program = 12;
        out->slots[0].octave = -1;
    }
    if (self->moodTimer < 3) {
        self->methods->moveLocalY(self, 150, 0);
    } else if (self->moodTimer < 7) {
        self->methods->moveLocalY(self, (self->moodTimer & 1) ? -50 : 50, 0);
    } else if (self->moodTimer == 100) {
        if (rand() & 1) {
            self->state = 12;
            self->methods->stopTod(self);
        }
    } else if (self->moodTimer == 240) {
        ((DreamSys *)self->peer)
            ->methods->setTickCallbacks((DreamSys *)self->peer, MOVE_CALLBACK_TICK_MOVE,
                                        LOOK_CALLBACK_STEP_LOOK);
    }
    if (self->state == 12) {
        if (self->moodTimer < 130) {
            self->methods->moveLocalY(self, 10, 0);
        } else if (self->moodTimer < 160) {
            self->methods->moveLocalZ(self, -30, 0);
        } else if (self->moodTimer < 301) {
            /* nothing */
        } else {
            SceneNode__FaceTarget((SceneNode *)self, (SceneNode *)self->peer, 1, 0, 0);
            self->methods->moveLocalZ(self, -30, 0);
        }
    }
tail:
    if (self->methods->distanceToPeer(self, self->peer) < 512) {
        self->methods->deactivate(self);
        self->methods->notifyParents(self, ENTITY_EFFECT_LINK_STAGE);
    }
}

/* ---- MoodCue handlers, rows 59 to 81 ------------------------------------
 *
 * Twenty of Entity's MoodCue handlers.
 *
 * Each Entity__MoodCueNN is the `handler` of gEntityMoodHandlerTable's row
 * NN (include/entity.h): rows 59, 61, 62, 64 to 71 and 73 to 81, and
 * Entity__MoodCue81 is row 120's handler too (the row's data words
 * differ). Rows 60, 63 and 72 have no handler. An Entity whose moodIndex
 * selects the row installs it as its SoundCueSet callback, so
 * ServiceSoundCueSet calls it once per tick with the Entity and its cue
 * set. A handler requests tones by filling the set's slots (a VAB program
 * of the cue's sound object, or SOUND_CUE_STOP), moves and turns the
 * entity (or the player, its `peer`) on moodTimer, the ticks since
 * startSoundCue, on the cue set's own `tick`, or on todFrame, the frame of
 * its TOD animation, and sends the dream an EntityEffect through
 * notifyParents. Entity__MoodCue108 (below) runs Entity__MoodCue71 and
 * then sets its scale to sScaleSix.
 *
 * The literals are left unnamed where they are one handler's tuning: tick
 * counts, distances in world units, TOD frame numbers, VAB program numbers,
 * and the `state` values other than 0 and ENTITY_STATE_DONE, which are each
 * handler's own phases.
 */

void Entity__MoodCue59(Entity *self, SoundCueSet *out) {
    if (self->moodTimer == 0 && rand() % 10 == 0) {
        self->state = 12;
    }
    if (out->tick % 10 == 0) {
        out->attenuation = self->methods->getProximityRatio(self);
        out->slots[0].program = 12;
        out->slots[0].octave = -1;
    }
    if (self->moodTimer == 0) {
        if (rand() & 1) {
            self->methods->moveLocalY(self, 2048, 0);
        }
    }
    self->methods->moveLocalZ(self, -128, 0);
    if (self->state == 12 && self->moodTimer == 300) {
        self->grid->methods->startScaleRamp(self->grid, 1, 1);
    }
}

void Entity__MoodCue61(Entity *self, SoundCueSet *out) {
    if (self->todFrame == 30) {
        out->slots[0].program = 18;
        out->attenuation = 0;
        out->slots[0].octave = -1;
    }
}

void Entity__MoodCue62(Entity *self, SoundCueSet *out) {
    if (self->todFrame % 30 == 0) {
        out->slots[0].program = 3;
        out->attenuation = 0;
        out->slots[0].octave = -2;
    }
    if (self->moodTimer >= 101) {
        SceneNode__FaceTarget((SceneNode *)self, (SceneNode *)self->peer, 1, 0, 0);
    }
    self->methods->moveLocalZ(self, -5, 0);
    if (self->moodTimer == 300 && self->methods->distanceToPeer(self, self->peer) < 4096) {
        ((DreamSys *)self->peer)->methods->clearTickCallbacks((DreamSys *)self->peer, false);
    } else if (self->moodTimer == 500) {
        ((DreamSys *)self->peer)
            ->methods->setTickCallbacks((DreamSys *)self->peer, MOVE_CALLBACK_TICK_MOVE,
                                        LOOK_CALLBACK_STEP_LOOK);
    }
    if (self->state == 0) {
        if (self->methods->distanceToPeer(self, self->peer) < 1024) {
            if (rand() & 1) {
                out->slots[1].program = 6;
                out->attenuation = 0;
                out->slots[1].octave = -1;
                if (rand() & 1) {
                    self->grid->methods->startScaleRamp(self->grid, -1, 0);
                }
                self->moodTimer = 0;
                self->state = 10;
            } else {
                self->state = 11;
            }
        }
    }
    if (self->state == 10 && self->moodTimer == 70) {
        self->methods->notifyParents(self, (rand() & 1) ? ENTITY_EFFECT_END_DREAM
                                                        : ENTITY_EFFECT_EVENT_VIDEO);
    }
}

void Entity__MoodCue64(Entity *self, SoundCueSet *out) {
    s32 phase = out->tick % 300;

    out->attenuation = self->methods->getProximityRatio(self);
    if (phase < 20) {
        out->slots[0].program = 5;
        out->slots[0].octave = -2;
    } else if (phase == 22) {
        out->slots[0].program = SOUND_CUE_STOP;
    }
    self->methods->moveLocalZ(self, -10, 0);
}

void Entity__MoodCue65(Entity *self, SoundCueSet *out) {
    if (self->moodTimer == 0) {
        if (rand() % 3 == 0) {
            self->methods->updateScale(self, 1, sScaleHalf);
            self->methods->moveLocalY(self, -300, 0);
            self->methods->updateRotation(self, 1, sRotationYawPlus90);
            self->state = 11;
        }
    }
    if (self->state == 11) {
        if (self->moodTimer == 2000) {
            self->methods->updateRotation(self, 0, sRotationYawMinus90);
        }
        self->methods->moveLocalZ(self, -20, 0);
    }
}

void Entity__MoodCue66(Entity *self, SoundCueSet *out) {
    out->attenuation = self->methods->getProximityRatio(self);
    if (out->tick % 30 == 0) {
        out->slots[0].program = 13;
    }
}

void Entity__MoodCue67(Entity *self, SoundCueSet *out) {
    if (self->moodTimer == 0) {
        if (rand() % 3 == 0) {
            self->state = 11;
        }
    }
    if (self->state == 11) {
        if (self->moodTimer == 502) {
            self->methods->moveLocalY(self, 2048, 0);
            SceneNode__FaceTarget((SceneNode *)self, (SceneNode *)self->peer, 1, 0, 0);
        }
        if (self->moodTimer >= 501) {
            self->methods->moveLocalZ(self, -512, 0);
        }
    }
}

void Entity__MoodCue68(Entity *self, SoundCueSet *out) {
    out->attenuation = self->methods->getProximityRatio(self);
    if (out->tick == 0) {
        self->lastOffsetValue = (rand() & 1) ? -374 : -192;
    }
    if (self->state == 0) {
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
        if ((self->moodTimer & 1) == 0) {
            if (((DreamSys *)self->peer)->methods->getLinkCommandFlag((DreamSys *)self->peer) != 0) {
                self->moodTimer = -1;
                self->state = 10;
                out->slots[0].program = 18;
            }
        }
        SceneNode__FaceTarget((SceneNode *)self, (SceneNode *)self->peer, 1, 0, 0);
        self->methods->moveLocalZ(self, self->lastOffsetValue, (void *)1);
    } else if (self->state == 10) {
        if (self->moodTimer < 8) {
            self->methods->updateRotation(self, 0, sRotationZPlus9);
            self->methods->addTranslation(self, sTranslateYPlus8);
        } else {
            u32 coin;

            out->slots[0].program = 18;
            out->slots[1].program = 3;
            self->methods->stopSoundCue(self);
            coin = rand() & 1;
            /* Half the time ENTITY_STATE_DONE, else back to phase 0. */
            self->state = coin < 1;
        }
    }
}

void Entity__MoodCue69(Entity *self, SoundCueSet *out) {
    out->attenuation = self->methods->getProximityRatio(self);
    if (self->todFrame == self->todFrameCount - 1) {
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

void Entity__MoodCue70(Entity *self, SoundCueSet *out) {
    if (self->moodTimer == 0) {
        if (rand() & 1) {
            self->state = 11;
        }
    }
    if (self->moodTimer == 300) {
        self->methods->updateRotation(self, 0, sRotationYawPlus180);
    }
    if (self->moodTimer < 600) {
        self->methods->moveLocalZ(self, self->state == 0 ? -256 : 256, 0);
    }
}

void Entity__MoodCue71(Entity *self, SoundCueSet *out) {
    s32 lane;

    out->attenuation = self->methods->getProximityRatio(self);
    if (out->tick == 0) {
        out->slots[0].program = 0;
        lane = rand() % 3;
        self->methods->moveLocalX(self, lane * 51200, 0);
    }
    if (self->moodTimer >= 2401) {
        SceneNode__FaceTarget((SceneNode *)self, (SceneNode *)self->peer, 1, 0, 0);
    }
    self->methods->moveLocalZ(self, -30, 0);
}

void Entity__MoodCue73(Entity *self, SoundCueSet *out) {
    out->attenuation = 0;
    if (out->tick == 0) {
        ((DreamSys *)self->peer)->methods->updateRotation((DreamSys *)self->peer, 1, sRotationYawPlus90);
        ((DreamSys *)self->peer)->methods->clearTickCallbacks((DreamSys *)self->peer, true);
        out->slots[0].program = 25;
        out->slots[1].program = 25;
        out->slots[2].program = 25;
    } else if (out->tick == 20) {
        out->slots[1].program = 13;
    }
    if (self->moodTimer == self->todFrameCount - 1) {
        self->methods->deactivate(self);
    }
}

/* {0, 100, 190}: the clear colour Entity__MoodCue74 gives the peer's viewport. */
extern ColorRgb sMoodCue74ClearColor;

void Entity__MoodCue74(Entity *self, SoundCueSet *out) {
    if (self->moodTimer == 0) {
        ((DreamSys *)self->peer)
            ->viewport->methods->setClearColor(((DreamSys *)self->peer)->viewport, &sMoodCue74ClearColor);
        self->state = rand() % 3;
        if (((DreamSys *)self->peer)->coord2->coord.t[2] < 610) {
            self->state = 0;
        }
    }
    if (self->state != 0) {
        if (self->todFrameCount / 2 < self->moodTimer) {
            ((DreamSys *)self->peer)->methods->moveLocalZ((DreamSys *)self->peer, 128, 0);
        }
        if (self->moodTimer == self->todFrameCount - 30) {
            self->methods->notifyParents(self, ENTITY_EFFECT_LINK_STAGE);
        }
    } else {
        if (self->moodTimer >= 20 && self->moodTimer < 120) {
            ((DreamSys *)self->peer)
                ->methods->moveLocalZ((DreamSys *)self->peer, -((self->moodTimer - 19) * 32), (void *)1);
            if (self->moodTimer == 85) {
                ((DreamSys *)self->peer)
                    ->methods->setTickCallbacks((DreamSys *)self->peer, MOVE_CALLBACK_TICK_MOVE,
                                                LOOK_CALLBACK_STEP_LOOK);
            }
        }
    }
}

void Entity__MoodCue75(Entity *self, SoundCueSet *out) {
    if (out->tick == 0) {
        out->attenuation = 0;
        out->slots[0].program = 25;
        out->slots[1].program = 25;
        out->slots[2].program = 25;
        SceneNode__FaceTarget((SceneNode *)self, (SceneNode *)self->peer, 1, 0, 0);
    }
    if (self->moodTimer == self->todFrameCount) {
        self->methods->stopTod(self);
        self->methods->notifyParents(self, ENTITY_EFFECT_LINK_STAGE);
    }
}

void Entity__MoodCue76(Entity *self, SoundCueSet *out) {
    if (self->targetReached != 0) {
        ((EntityPlayTodFn)self->methods->playTod)(self);
        if (self->todFrame == self->todFrameCount - 1) {
            self->methods->stopTod(self);
            self->methods->updateScale(self, 0, sScaleMinusSixtyFourth);
        }
    } else {
        self->methods->stopTod(self);
        self->methods->updateRotation(self, 0, sRotationYawPlus9);
    }
}

void Entity__MoodCue77(Entity *self, SoundCueSet *out) {
    out->attenuation = self->methods->getProximityRatio(self);
    if (out->tick % 5 == 0) {
        out->slots[0].program = 17;
        out->slots[0].octave = -2;
    }
    if (self->todIndex == 0) {
        if (self->moodTimer == self->todFrameCount) {
            self->methods->setTod(self, 1);
            if (rand() & 1) {
                self->state = 11;
            }
        }
        return;
    }
    if (self->state == 0) {
        if (self->moodTimer == 60 || self->moodTimer == 212 || self->moodTimer == 290 ||
            self->moodTimer == 320) {
            self->methods->updateRotation(self, 0, sRotationYawPlus90);
        }
        if (self->moodTimer == 398) {
            self->methods->updateRotation(self, 0, sRotationYawMinus90);
        }
        self->methods->moveLocalZOrFindLink(self, -50, 0);
        return;
    }
    if (self->moodTimer == 60 || self->moodTimer == 140) {
        self->methods->updateRotation(self, 0, sRotationYawPlus90);
    }
    if (self->moodTimer < 174) {
        self->methods->moveLocalZOrFindLink(self, -50, 0);
    }
    if (self->moodTimer == 174) {
        self->methods->stopSoundCue(self);
        self->state = ENTITY_STATE_DONE;
    }
}

/* Entity__MoodCue78's: cleared on the cue's first tick, set when its phase
 * 11 ends the cue at tick 510; at tick 520 a set flag ends it again. */
extern s32 sMoodCue78TransitionDone;

void Entity__MoodCue78(Entity *self, SoundCueSet *out) {
    /* MATCHING: one local for the roll and then the y move; two allocate
     * differently. */
    s32 rollOrDy;
    void *table;

    if (out->tick == 0) {
        sMoodCue78TransitionDone = 0;
        rollOrDy = rand() % 3;
        if (rollOrDy == 1) {
            self->state = 11;
        }
        if (rollOrDy == 2) {
            self->state = 12;
        }
    }

    out->attenuation = self->methods->getProximityRatio(self);

    if (self->todPlaying != 0 && (out->tick & 3) == 0) {
        out->slots[0].program = 28;
    }

    if (self->moodTimer == self->todFrameCount - 1) {
        self->moodTimer = -1;
    } else {
        if (self->moodTimer >= self->todFrameCount / 2 + self->todFrameCount / 4) {
            ((EntityPlayTodFn)self->methods->playTod)(self);
        } else if (self->moodTimer >= self->todFrameCount / 2) {
            if (self->moodTimer == self->todFrameCount / 2) {
                out->slots[0].program = 16;
            }
            self->methods->moveLocalZ(self, 110, 0);
        } else if (self->moodTimer < self->todFrameCount / 4) {
            /* nothing */
        } else {
            self->methods->stopTod(self);
            self->methods->moveLocalZ(self, -110, 0);
        }
    }

    if (self->state == 11 && out->tick == 510) {
        self->methods->moveLocalY(self, -380, 0);
        self->methods->updateRotation(self, 0, sRotationXPlus90);
        self->methods->stopSoundCue(self);
        self->state = ENTITY_STATE_DONE;
        sMoodCue78TransitionDone = 1;
    } else if (self->state >= 12 && out->tick >= 330 && (out->tick % 60) == 30) {
        rollOrDy = 0;
        if (rand() & 1) {
            table = sScaleY2;
            rollOrDy = (self->state == 12) ? 400 : 0;
            self->state = 13;
        } else {
            table = sScaleUnit;
            if (self->state == 13) {
                rollOrDy = -400;
            }
            self->state = 12;
        }
        self->methods->updateScale(self, 1, table);
        self->methods->moveLocalY(self, rollOrDy, 0);
    }

    if (out->tick == 520 && sMoodCue78TransitionDone != 0) {
        self->methods->stopSoundCue(self);
        self->state = ENTITY_STATE_DONE;
    }
}

void Entity__MoodCue79(Entity *self, SoundCueSet *out) {
    out->attenuation = self->methods->getProximityRatio(self);
    if (out->tick % 10 == 0) {
        out->slots[0].program = 25;
        out->slots[0].octave = 2;
    }
    self->methods->updateRotation(self, 0, sRotationYawPlus2);
    if (self->state == 0 && self->targetReached != 0) {
        self->methods->notifyParents(self, ENTITY_EFFECT_EVENT_VIDEO);
        self->state = 11;
    }
}

void Entity__MoodCue80(Entity *self, SoundCueSet *out) {
    if (self->moodTimer < self->todFrameCount) {
        if (self->todFrame != 0) {
            if (self->todFrame == 20) {
                out->attenuation = 0;
                out->slots[0].program = 16;
            }
        }
    } else {
        self->methods->stopTod(self);
        self->methods->addTranslation(self, sTranslateYMinus512);
    }
    SceneNode__FaceTarget((SceneNode *)self, (SceneNode *)self->peer, 1, 0, 0);
}

void Entity__MoodCue81(Entity *self, SoundCueSet *out) {
    s32 dz;
    s32 state;

    if (self->moodTimer == 0 && (rand() & 1)) {
        self->state = rand() % 3;
    }
    if (self->state != 0 && self->targetReached != 0) {
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
        SceneNode__FaceTarget((SceneNode *)self, (SceneNode *)self->peer, 1, 0, 0);
        state = self->state;
        if (state == 1) {
            self->methods->updateScale(self, 0, sScaleEightSevenths);
            dz = -374;
            if (self->methods->distanceToPeer(self, self->peer) < 512) {
                self->methods->deactivate(self);
                self->state = state;
            }
        } else {
            ((DreamSys *)self->peer)->methods->clearTickCallbacks((DreamSys *)self->peer, true);
            SceneNode__FaceTarget((SceneNode *)self->peer, (SceneNode *)self, 1, 1, 0);
            if (self->state == 2) {
                if (self->methods->distanceToPeer(self, self->peer) < 2400) {
                    self->state = 11;
                    self->methods->notifyParents(self, ENTITY_EFFECT_END_DREAM);
                }
                dz = -96;
            } else {
                dz = 0;
            }
        }
    } else {
        out->attenuation = self->methods->getProximityRatio(self);
        if (out->tick % 22 == 0) {
            out->slots[0].program = 28;
        }
        if (self->moodTimer >= 501) {
            SceneNode__FaceTarget((SceneNode *)self, (SceneNode *)self->peer, 1, 0, 0);
        }
        if (self->methods->distanceToPeer(self, self->peer) < 2048) {
            ((DreamSys *)self->peer)->methods->moveLocalZ((DreamSys *)self->peer, -2048, 0);
        }
        dz = -20;
    }
    self->methods->moveLocalZOrFindLink(self, dz, (void *)1);
    if (self->linkTarget != 0) {
        self->methods->moveLocalY(self, -200, 0);
    }
}

/* ---- MoodCue handlers, rows 82 to 96 ------------------------------------
 *
 * Fifteen of Entity's MoodCue handlers and the two tone setters they share.
 *
 * Each Entity__MoodCueNN is the `handler` of gEntityMoodHandlerTable's row
 * NN (include/entity.h): rows 82 to 96, and Entity__MoodCue93 is row 107's
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
 * TRANSLATE_*) are named by value and declared once in include/entity.h.
 */

/* Defined after Entity__MoodCue86, which calls them. */
void SetCueTones7_7_7(SoundCueSet *out);
void SetCueTones18_3_3(SoundCueSet *out);

void Entity__MoodCue82(Entity *self, SoundCueSet *out) {
    if (self->state == 0 && self->moodTimer == 0) {
        if (rand() % 3 != 0) {
            self->state = (rand() & 1) ? 11 : 12;
        } else {
            self->methods->stopSoundCue(self);
            self->methods->moveLocalZ(self, -20480, 0);
            rand();
        }
    }
    if (self->state == 12) {
        SceneNode__FaceTarget((SceneNode *)self, (SceneNode *)self->peer, 1, 0, 0);
        if (self->moodTimer == 20) {
            out->slots[0].program = 18;
            out->attenuation = 0;
            out->slots[1].program = 3;
            ((DreamSys *)self->peer)->methods->clearTickCallbacks((DreamSys *)self->peer, 1);
        }
        if (self->moodTimer >= 21) {
            self->methods->moveLocalZ(self, -40, 0);
        }
        if (self->moodTimer == 40) {
            self->methods->notifyParents(self, ENTITY_EFFECT_LINK_STAGE);
        }
    } else if (self->state == 11) {
        self->methods->stopTod(self);
        if (self->methods->distanceToPeer(self, self->peer) < 512) {
            if (rand() % 3 != 0) {
                self->methods->deactivate(self);
            } else {
                self->methods->stopSoundCue(self);
            }
        }
    }
}

void Entity__MoodCue83(Entity *self, SoundCueSet *out) {
    if (self->todFrame % 15 == 0) {
        out->attenuation = 0;
        out->slots[0].program = 12;
        out->slots[0].octave = 2;
    }
    if (self->moodTimer == self->todFrameCount) {
        out->slots[0].program = SOUND_CUE_STOP;
        self->methods->stopSoundCue(self);
        self->state = ENTITY_STATE_DONE;
    }
}

void Entity__MoodCue84(Entity *self, SoundCueSet *out) {
    out->attenuation = self->methods->getProximityRatio(self);
    if (self->todFrame < 40) {
        out->slots[0].program = 12;
        out->slots[0].octave = -2;
        out->slots[2].program = 5;
        out->slots[2].octave = -1;
        return;
    }
    if (self->todFrame == 40) {
        out->slots[0].program = SOUND_CUE_STOP;
        out->slots[2].program = SOUND_CUE_STOP;
        return;
    }
    if (self->todFrame == 45) {
        out->slots[1].program = 18;
        out->slots[1].octave = 1;
        return;
    }
    if (self->todFrame == 64) {
        out->slots[0].program = 7;
        return;
    }
    if (self->todFrame == 89) {
        self->methods->stopSoundCue(self);
        self->state = ENTITY_STATE_DONE;
    }
}

void Entity__MoodCue85(Entity *self, SoundCueSet *out) {
    if (self->state == 0) {
        if (self->todFrame == 5) {
            SetCueTones7_7_7(out);
        }
        if (self->moodTimer == self->todFrameCount) {
            self->methods->stopTod(self);
            self->state = 10;
            self->moodTimer = -1;
        }
    } else if (self->state == 10) {
        if (self->moodTimer < 10) {
            self->methods->updateRotation(self, 0, sRotationYawPlus9);
            if (((DreamSys *)self->peer)->methods->getLinkCommandFlag((DreamSys *)self->peer) != 0) {
                SetCueTones7_7_7(out);
                self->state = 12;
                self->moodTimer = -1;
            }
        } else {
            ((DreamSys *)self->peer)->methods->clearTickCallbacks((DreamSys *)self->peer, 1);
            self->state = 11;
            self->moodTimer = -1;
        }
    } else if (self->state == 11) {
        SceneNode__FaceTarget((SceneNode *)self, (SceneNode *)self->peer, 1, 0, 0);
        if (self->moodTimer < 30) {
            self->methods->moveLocalZ(self, -10, 0);
        } else {
            SetCueTones7_7_7(out);
            if (Entity__GetOrCreateFadeBox(self, NULL, NULL, (void *)30, 0) != NULL) {
                self->fadeBox->methods->startFadeDown(self->fadeBox, (BasicClass *)self->ticker, 7, 0);
            }
            self->state = 13;
            self->moodTimer = -1;
        }
    } else if (self->state == 13) {
        if (self->moodTimer < 90) {
            if (self->moodTimer == 30) {
                if (Entity__GetOrCreateFadeBox(self, NULL, NULL, (void *)10, 0) != NULL) {
                    self->fadeBox->methods->startFadeUp(self->fadeBox, (BasicClass *)self->ticker, 0, 0);
                }
            }
            ((DreamSys *)self->peer)->methods->updateRotation((DreamSys *)self->peer, 0, sRotationZPlus1);
        } else {
            SetCueTones18_3_3(out);
            ((DreamSys *)self->peer)->methods->updateRotation((DreamSys *)self->peer, 1, sRotationYawPlus180);
            self->methods->notifyParents(self, (rand() % 5 != 0) ? ENTITY_EFFECT_LINK_STAGE
                                                                 : ENTITY_EFFECT_END_DREAM);
            self->state = 14;
        }
    } else if (self->state == 12) {
        if (self->moodTimer < 10) {
            self->methods->updateRotation(self, 0, sRotationZMinus9);
        } else {
            SetCueTones18_3_3(out);
            self->methods->stopSoundCue(self);
            self->state = ENTITY_STATE_DONE;
        }
    }
}

void Entity__MoodCue86(Entity *self, SoundCueSet *out) {
    if (self->moodTimer < 10) {
        self->methods->stopTod(self);
    } else if (self->moodTimer == 10) {
        ((EntityPlayTodFn)self->methods->playTod)(self);
    }
    if (self->todFrame == 10) {
        SetCueTones18_3_3(out);
    }
    if (self->moodTimer == self->todFrameCount + 10) {
        self->methods->stopSoundCue(self);
        self->state = ENTITY_STATE_DONE;
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

void Entity__MoodCue87(Entity *self, SoundCueSet *out) {
    out->attenuation = self->methods->getProximityRatio(self);
    if (out->tick == 0) {
        out->slots[0].program = 18;
    }
    if (out->tick >= self->todFrameCount - 1) {
        out->tick = -1;
    }
}

void Entity__MoodCue88(Entity *self, SoundCueSet *out) {
    out->attenuation = self->methods->getProximityRatio(self);
    if (out->tick == self->todFrameCount / 2) {
        out->slots[0].program = 18;
    }
    if (out->tick >= self->todFrameCount - 1) {
        out->tick = -1;
    }
}

void Entity__MoodCue89(Entity *self, SoundCueSet *out) {
    if (self->moodTimer == 20) {
        out->slots[0].program = 18;
        out->attenuation = 0;
        out->slots[1].program = 3;
        return;
    }
    if (self->moodTimer == self->todFrameCount) {
        self->methods->stopSoundCue(self);
        self->state = ENTITY_STATE_DONE;
        if (rand() & 1) {
            self->methods->notifyParents(self, ENTITY_EFFECT_EVENT_VIDEO);
        }
    }
}

void Entity__MoodCue90(Entity *self, SoundCueSet *out) {
    out->attenuation = self->methods->getProximityRatio(self);
    if (out->tick == 0) {
        out->slots[0].program = (rand() & 1) ? 2 : 1;
        out->slots[0].octave = 2;
    }
}

void Entity__MoodCue91(Entity *self, SoundCueSet *out) {
    if (self->moodTimer == 0) {
        if (Entity__GetOrCreateFadeBox(self, NULL, NULL, (void *)5, 0) != NULL) {
            if (rand() & 1) {
                self->methods->addTranslation(self, sTranslateYMinus256);
            }
            self->fadeBox->methods->startFadeDown(self->fadeBox, (BasicClass *)self->ticker, 0, 0);
        }
    } else {
        if (self->todFrame == 0) {
            do {
                self->todFramePtr = self->methods->applyTodFrame(self, self->todFramePtr, 0);
                self->todFrame += 1;
            } while (self->todFrame < 24);
        }
    }
    if (self->todFrame >= 25) {
        self->methods->moveLocalZ(self, -20, 0);
        ((DreamSys *)self->peer)->methods->clearTickCallbacks((DreamSys *)self->peer, 1);
    }
    if (self->moodTimer == 50) {
        self->methods->notifyParents(self, ENTITY_EFFECT_LINK_STAGE);
    } else if (self->moodTimer == 12) {
        out->attenuation = 0;
        out->slots[0].program = 21;
    }
    self->methods->updateScale(self, 1, sScaleXFourFifthsYSixFifths);
}

void Entity__MoodCue92(Entity *self, SoundCueSet *out) {
    if (self->todIndex == 0) {
        if (self->targetReached != 0) {
            SceneNode__FaceTarget((SceneNode *)self, (SceneNode *)self->peer, 1, 0, 0);
            self->methods->setTod(self, 1);
            ((DreamSys *)self->peer)->methods->clearTickCallbacks((DreamSys *)self->peer, 1);
        } else if (self->todFrame == 0) {
            do {
                self->todFramePtr = self->methods->applyTodFrame(self, self->todFramePtr, 0);
                self->todFrame += 1;
            } while (self->todFrame < 24);
        }
    } else {
        if (self->todFrame == 0) {
            out->attenuation = 0;
            out->slots[0].program = 22;
        } else if (self->todFrame == self->todFrameCount - 1) {
            out->attenuation = 0;
            out->slots[1].program = 18;
            self->methods->notifyParents(self, ENTITY_EFFECT_LINK_STAGE);
        }
    }
    self->methods->updateScale(self, 1, sScaleXFourFifthsYSixFifths);
}

void Entity__MoodCue93(Entity *self, SoundCueSet *out) {
    out->attenuation = self->methods->getProximityRatio(self);
    if (out->tick % 10 == 0) {
        out->slots[0].program = 3;
    }
    if (self->moodTimer == self->todFrameCount) {
        self->methods->setTod(self, 1);
    }
    if (self->todIndex == 1) {
        self->methods->moveLocalZ(self, -128, (void *)1);
    }
}

void Entity__MoodCue94(Entity *self, SoundCueSet *out) {
    if (self->moodTimer == 0) {
        if (((DreamSys *)self->peer)->methods->getDreamColor((DreamSys *)self->peer) != DREAM_COLOR_WHITE) {
            self->state = 11;
        }
    }
    out->attenuation = self->methods->getProximityRatio(self);
    if (out->tick % 10 == 0) {
        out->slots[0].program = 14;
    }
    if (self->moodTimer == self->todFrameCount) {
        self->methods->setTod(self, 1);
        if (self->state != 0) {
            if ((rand() & 1) == 0) {
                self->methods->updateScale(self, 1, sScaleSix);
                self->methods->moveLocalY(self, 2048, 0);
            }
        }
        if (rand() % 3 == 0) {
            self->methods->updateRotation(self, 0, sRotationYawPlus180);
        }
    }
    if (self->todIndex != 0) {
        self->methods->moveLocalZ(self, -128, (void *)1);
    }
}

void Entity__MoodCue95(Entity *self, SoundCueSet *out) {
    if (self->moodTimer == 0) {
        self->methods->setTod(self, 3);
    } else if (self->moodTimer == self->todFrameCount) {
        self->methods->setTod(self, 1);
    }
    if (self->todIndex == 1) {
        self->methods->moveLocalZ(self, -128, 0);
    }
}

void Entity__MoodCue96(Entity *self, SoundCueSet *out) {
    if (self->moodTimer == 0) {
        self->methods->setTod(self, rand() % 4);
        return;
    }
    if (self->moodTimer % self->todFrameCount == 0) {
        self->methods->setTod(self, rand() % 4);
        out->attenuation = self->methods->getProximityRatio(self);
        out->slots[0].program = 22;
        out->slots[0].octave = 2;
        out->slots[0].vol = 64;
        out->slots[0].endVol = 32;
    }
}

/* ---- MoodCue handlers, rows 98 to 129 -----------------------------------
 *
 * Nineteen of Entity's MoodCue handlers and a per-tick helper one of them
 * shares with a handler above.
 *
 * Each Entity__MoodCueNN is the `handler` of gEntityMoodHandlerTable's row
 * NN (include/entity.h): rows 98, 102 to 106, 108 to 111, 113, 114, 117,
 * 118, 121, 123, 125, 128 and 129. Entity__MoodCue123 is also row 126's
 * handler (same `handler` word, different data words), named for its lower
 * row. An Entity whose moodIndex selects the row installs it as its
 * SoundCueSet callback, so ServiceSoundCueSet calls it once per tick with
 * the Entity and its cue set. A handler requests tones by filling the
 * set's slots (a VAB program of the cue's sound object, or SOUND_CUE_STOP),
 * moves and turns the entity (or the player, its `peer`) on moodTimer, the
 * ticks since startSoundCue, or on todFrame, the frame of its TOD
 * animation, and sends the dream an EntityEffect through notifyParents.
 *
 * Entity__StepYawInWindowsThenDeactivate is not a row: it is a shared
 * per-tick helper called directly by two different row handlers,
 * Entity__MoodCue111 (this section, twice) and Entity__MoodCue40 (rows 39
 * to 58, its only caller outside this section).
 *
 * The literals are left unnamed where they are one handler's tuning: tick
 * counts, distances in world units, TOD frame numbers, VAB program
 * numbers, and the `state` values other than 0 and ENTITY_STATE_DONE,
 * which are each handler's own phases. The motion templates the handlers
 * pass to updateRotation and updateScale (ROTATION_*, SCALE_*) are named
 * by value and declared once in include/entity.h.
 */

void Entity__MoodCue98(Entity *self, SoundCueSet *out) {
    if (self->targetReached != 0) {
        if (Entity__GetOrCreateFadeBox(self, NULL, 0, 10, 0) != 0) {
            self->fadeBox->methods->startFadeDown(self->fadeBox, (BasicClass *)self->ticker, 7, 0);
            self->methods->deactivate(self);
            ((DreamSys *)self->peer)->methods->resetFlashbackList((DreamSys *)self->peer);
        }
    }
    self->methods->moveLocalZ(self, -30, (void *)1);
}

void Entity__MoodCue102(Entity *self, SoundCueSet *out) {
    Ratio16 *scale;

    if (self->moodTimer == 0) {
        self->methods->moveLocalY(self, -512, 0);
    }
    out->attenuation = self->methods->getProximityRatio(self);
    if (self->todFrame == self->todFrameCount / 2) {
        out->slots[0].program = 7;
        out->slots[0].octave = -2;
        out->slots[1].program = 3;
        out->slots[1].octave = -2;
    }
    if (self->moodTimer >= 51) {
        self->methods->updateRotation(self, 0, sRotationYawMinusThird);
    }
    if (self->moodTimer >= 781) {
        SceneNode__FaceTarget((SceneNode *)self, (SceneNode *)self->peer, 1, 0, 0);
        if (self->moodTimer >= 1936) {
            scale = sScaleUnit;
        } else if (self->moodTimer >= 1931) {
            scale = sScaleXFourFifthsYSixFifths;
        } else if (self->moodTimer >= 1926) {
            scale = sScaleHalf;
        } else if (self->moodTimer >= 1921) {
            scale = sScaleQuarter;
        } else {
            scale = sScaleEighth;
        }
        self->methods->updateScale(self, 1, scale);
        if (self->moodTimer < 2000) {
            self->methods->moveLocalZ(self, -64, 0);
        } else {
            self->state = ENTITY_STATE_DONE;
        }
    } else {
        self->methods->moveLocalZ(self, -256, 0);
    }
    if (self->targetReached != 0 && self->state == 0) {
        self->state = 12;
        ((DreamSys *)self->peer)->methods->clearTickCallbacks((DreamSys *)self->peer, 1);
        self->methods->notifyParents(self, ENTITY_EFFECT_LINK_STAGE);
    }
    if (self->state == 12) {
        ((DreamSys *)self->peer)->methods->moveLocalZ((DreamSys *)self->peer, 256, 0);
    }
}

void Entity__MoodCue103(Entity *self, SoundCueSet *out) {
    if (self->moodTimer == 700) {
        if (rand() % 3 == 0) {
            self->state = 11;
        }
    }
    if (self->state == 11) {
        if (self->moodTimer < 1020) {
            self->methods->updateRotation(self, 0, sRotationYawMinusHalf);
            self->methods->moveLocalY(self, 30, 0);
        }
        if (self->moodTimer == 930) {
            self->methods->notifyParents(self, ENTITY_EFFECT_LINK_STAGE);
        }
    } else if (self->moodTimer == 100 || self->moodTimer == 800) {
        if (rand() % 5 == 0) {
            self->grid->methods->startScaleRamp(self->grid, 4, 0);
        }
    }
    self->methods->moveLocalZ(self, -30, 0);
}

void Entity__MoodCue104(Entity *self, SoundCueSet *out) {
    self->methods->updateScale(self, 1, sScaleQuarter);
    if (self->moodTimer >= 201 && self->moodTimer < 300) {
        self->methods->moveLocalY(self, -32, 0);
    }
}

void Entity__MoodCue105(Entity *self, SoundCueSet *out) {
    self->methods->setDisplay(self, rand() % 20 == 0);
}

void Entity__MoodCue106(Entity *self, SoundCueSet *out) {
    if (self->moodTimer == 0) {
        if ((rand() & 1) == 0) {
            self->state = 11;
        }
    }
    if (self->state == 11) {
        if (self->moodTimer == 0) {
            self->methods->stopTod(self);
        }
        self->methods->updateScale(self, 1, sScaleXEighthY2ZEighth);
        return;
    }
    if (self->moodTimer == 0) {
        self->methods->setTod(self, 1);
    }
    self->methods->moveLocalX(self, (self->moodTimer % 20 < 10) ? 32 : -32, 0);
}

void Entity__MoodCue108(Entity *self, SoundCueSet *out) {
    Entity__MoodCue71(self, out);
    self->methods->updateScale(self, 1, sScaleSix);
}

void Entity__MoodCue109(Entity *self, SoundCueSet *out) {
    self->methods->updateScale(self, 1, sScaleHalf);
    self->methods->moveLocalZ(self, -10, 0);
}

void Entity__MoodCue110(Entity *self, SoundCueSet *out) {
    self->methods->updateScale(self, 1, sScaleTwoFifths);
    self->methods->stopTod(self);
    if (self->state == 0) {
        if (self->methods->distanceToPeer(self, self->peer) < 2048) {
            self->state = 10;
            self->moodTimer = 0;
        }
    }
    if (self->state == 10) {
        if (self->moodTimer < 45) {
            self->methods->updateRotation(self, 0, sRotationYawPlus4);
        }
        if (self->moodTimer >= 501) {
            self->state = 0;
        }
    }
}

void Entity__MoodCue111(Entity *self, SoundCueSet *out) {
    if (self->moodTimer == 0) {
        if (((DreamSys *)self->peer)->methods->getDreamColor((DreamSys *)self->peer) == DREAM_COLOR_PINK) {
            self->state = 11;
        }
    }
    if (self->state != 0 && self->moodTimer >= 2160) {
        /* MATCHING: the repeated `>= 2160` is retail's second range test. */
        if (self->moodTimer >= 2160 && self->moodTimer <= 2560) {
            if (self->moodTimer == 2160) {
                self->methods->stopTod(self);
                out->slots[0].program = SOUND_CUE_STOP;
                out->slots[1].program = SOUND_CUE_STOP;
                out->slots[2].program = SOUND_CUE_STOP;
                return;
            }
            if (self->moodTimer >= 2550 && self->moodTimer < 2560) {
                out->slots[0].program = 5;
                out->slots[0].octave = -2;
                return;
            }
            if (self->moodTimer == 2560) {
                ((EntityPlayTodFn)self->methods->playTod)(self);
                out->tick = 1;
                return;
            }
            return;
        }
        if (self->moodTimer < 2563) {
            return;
        }
        if (self->moodTimer >= 2801) {
            self->methods->moveLocalY(self, -32, 0);
        }
        Entity__StepYawInWindowsThenDeactivate(self, out, 481, 4000, -60);
    } else {
        Entity__StepYawInWindowsThenDeactivate(self, out, 481, 2180, -60);
    }
}

void Entity__StepYawInWindowsThenDeactivate(Entity *self, SoundCueSet *out, s32 windowStart,
                                            s32 deactivateTimer, s32 zStep) {
    s32 timer;

    out->attenuation = 0;
    if (out->tick == 6) {
        out->slots[0].program = 4;
        out->slots[1].program = 4;
        out->slots[2].program = 4;
    }
    timer = self->moodTimer;
    if ((timer >= windowStart && timer <= windowStart + 91) ||
        (timer >= windowStart + 341 && timer <= windowStart + 433) ||
        (timer >= windowStart + 698 && timer <= windowStart + 791)) {
        self->methods->updateRotation(self, 0, sRotationYawPlus1);
    }
    self->methods->moveLocalZ(self, zStep, 0);
    if (self->moodTimer == deactivateTimer) {
        self->methods->deactivate(self);
        self->state = ENTITY_STATE_DONE;
    }
}

void Entity__MoodCue113(Entity *self, SoundCueSet *out) {
    Entity__MoodCue51(self, out);
}

void Entity__MoodCue114(Entity *self, SoundCueSet *out) {
    Ratio16 *rotation;

    if (rand() % 3 == 0) {
        return;
    }
    if (rand() % 3 != 0) {
        rotation = sRotationYawPlus9;
    } else {
        rotation = sRotationYawMinus9;
    }
    self->methods->updateRotation(self, 0, rotation);
}

void Entity__MoodCue117(Entity *self, SoundCueSet *out) {
    self->methods->updateScale(self, 1, sScaleSix);
}

void Entity__MoodCue118(Entity *self, SoundCueSet *out) {
    self->methods->updateScale(self, 1, sScaleSix);
}

void Entity__MoodCue121(Entity *self, SoundCueSet *out) {
    self->methods->updateScale(self, 1, sScaleQuarter);
}

void Entity__MoodCue123(Entity *self, SoundCueSet *out) {
    if (self->moodTimer == 0) {
        if (rand() % 5 == 0) {
            self->state = 11;
        }
    }
    self->methods->stopTod(self);
    self->methods->moveLocalZ(self, 100, 0);
    if (self->moodTimer == 1000) {
        self->methods->stopSoundCue(self);
        self->state = ENTITY_STATE_DONE;
    }
    if (self->state == 11) {
        if (self->moodTimer >= 301) {
            /* DreamSys__OnPadEvent's events 2 and 7: walk forward, then run. */
            ((DreamSys *)self->peer)->methods->onPadEvent((DreamSys *)self->peer, 0, 2);
            ((DreamSys *)self->peer)->methods->onPadEvent((DreamSys *)self->peer, 0, 7);
        }
    }
}

void Entity__MoodCue125(Entity *self, SoundCueSet *out) {
    if ((self->moodTimer == 0 && (rand() & 3) == 0) || self->moodTimer == 3600) {
        self->methods->deactivate(self);
        self->state = ENTITY_STATE_DONE;
    }
    self->methods->updateScale(self, 1, sScaleTwoFifths);
    out->attenuation = self->methods->getProximityRatio(self);
    if (out->tick % (self->todFrameCount / 2) == 0) {
        out->slots[0].program = 10;
        out->slots[0].octave = 1;
    }
    self->methods->moveLocalZ(self, -10, 0);
}

void Entity__MoodCue128(Entity *self, SoundCueSet *out) {
    SceneNode__FaceTarget((SceneNode *)self, (SceneNode *)self->peer, 1, 0, 0);
    self->methods->updateScale(self, 1, sScaleThirtySecond);
    self->methods->moveLocalZ(self, -30, (void *)1);
}

void Entity__MoodCue129(Entity *self, SoundCueSet *out) {
    if (self->moodTimer == 0) {
        self->state = rand() % 2 + 10;
    }
    self->methods->stopTod(self);
    if (self->moodTimer >= 201) {
        SceneNode__FaceTarget((SceneNode *)self, (SceneNode *)self->peer, 1, 0, 0);
        if (self->state == 10) {
            self->methods->moveLocalZOrFindLink(self, -512, 0);
        }
    }
}
