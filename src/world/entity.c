/*
 * entity.c -- Entity (include/entity.h): its methods, in address order, then
 * the MoodCue handlers of its mood rows, in sections by row. The class doc in
 * include/entity.h says what an Entity is, how its mood row drives it and
 * what a MoodCue handler does.
 */
#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <rand.h>
#include "entity.h"
#include "dream_sys.h"
#include "pad.h"
#include "stage_map.h"
#include "viewport.h"
#include "bmem_pmgr.h"

/* MATCHING: playTod is called through EntityPlayTodFn (void); through the s32 slot, calls stop merging */
/* MATCHING: tod_actor.h's moveLocalZ/moveLocalY must return void, or the handlers' calls stop merging */

/* The size and attach offset Entity__GetOrCreateFadeBox substitutes when its
 * `size`/`offset` arguments are NULL: {320, 240} and {-100, -100}, what
 * Viewport gives its FadeBox (fade_box.h). */
extern s32 sEntityFadeBoxDefaultSize[2];
extern s32 sEntityFadeBoxDefaultOffset[2];

/* Defined at the end of this file, after the handlers it lists. */
extern EntityMoodRow sEntityMoodTable[ENTITY_MOOD_ROW_COUNT];

/* The motion templates (.data, in address order):
 * the constant triples the MoodCue handlers in src/world/entity.c pass to
 * updateRotation (+0x044) and updateScale (+0x048) -- three Ratio16s
 * (include/scene_node.h), degrees or scale factors, {x, y, z} -- and to
 * addTranslation (+0x0BC), three s32 deltas. Named by value. The slots take
 * the table untyped, so the element type is the reader's (SceneNode__Update-
 * Rotation/UpdateScale), not the callers'. sTranslateYMinus64's data also
 * holds a second triple, (0, -0x20, 0); sScaleX3's z den is
 * sScaleTemplateZDenom (below). */
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
                    (SoundCueCallbackFn)sEntityMoodTable[self->moodIndex].handler);
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
 * The table's last three slots (updateTargetProximity, updateSoundCueStart,
 * updateSoundCueStop), the range test updateDeactivationState makes
 * (NotifyIfTargetInRange, IsTargetInRange), GetEntityMethods, then the
 * handlers of rows 0, 1, 5 and 7 to 17 (rows 2 to 4 and 6 have none).
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

/* For a row with no link stage (a negative linkStage) and
 * an event video, sends ENTITY_EFFECT_LINK_STAGE while the player is within
 * the video entry times 512 world units (Entity__IsTargetInRange). `unused`
 * is entity.h's declared second parameter; the one caller,
 * Entity__UpdateDeactivationState, passes 0. */
void Entity__NotifyIfTargetInRange(Entity *self, s32 unused) {
    if (sEntityMoodTable[self->moodIndex].linkStage < 0 &&
        sEntityMoodTable[self->moodIndex].eventVideo != 0 &&
        Entity__IsTargetInRange(self, sEntityMoodTable[self->moodIndex].eventVideo << 9)) {
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

/* Row 0: sounds program 5 every tenth tick and paces, 1200 ticks forward and 1200 back, then again.
 * In a pink dream it instead walks forward, lifts off forward and up, goes still and silent at
 * moodTimer 250, backs away turned to -120 degrees and at last tilts over. */
void Entity__CuePaceOrLiftOffOnPink(Entity *self, SoundCueSet *out) {
    enum { PINK_LIFT_OFF = 100 };

    if (out->tick == 0) {
        if (((DreamSys *)self->peer)->methods->getDreamColor((DreamSys *)self->peer) == DREAM_COLOR_PINK) {
            self->state = PINK_LIFT_OFF;
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

/* Row 1: sounds program 20 on three voices and takes the dreamer's controls on its first tick,
 * faces the dreamer and charges at 90 a tick, and links to the row's stage at moodTimer 30. */
void Entity__CueHoldDreamerChargeThenLink(Entity *self, SoundCueSet *out) {
    out->attenuation = 0;
    if (out->tick == 0) {
        out->slots[0].program = 20;
        out->slots[1].program = 20;
        out->slots[2].program = 20;
        ((DreamSys *)self->peer)->methods->clearTickCallbacks((DreamSys *)self->peer, true);
    }
    SceneNode__FaceTarget((SceneNode *)self, (SceneNode *)self->peer, 1, 0, 0);
    self->methods->moveLocalZ(self, -90, 0);
    if (self->moodTimer == 30) {
        self->methods->notifyParents(self, ENTITY_EFFECT_LINK_STAGE);
    }
}

/* Row 5: sounds program 23 once, attenuated by distance. */
void Entity__CueTone23Once(Entity *self, SoundCueSet *out) {
    out->attenuation = self->methods->getProximityRatio(self);
    if (out->tick == 0) {
        out->slots[0].program = 23;
    }
}

/* Row 7: faces the dreamer, then from moodTimer 10 comes at it (256 a tick); from 56, or once it is
 * on the dreamer, rises; from 121 spirals away, turning 2 degrees and moving 320 a tick. Sounds
 * programs 7 and 3 at mid-animation and program 6 in bursts. */
void Entity__CueApproachRiseThenSpiralAway(Entity *self, SoundCueSet *out) {
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

/* Row 8: holds double size and rises 64 a tick. */
void Entity__CueDoubleSizeAndRise(Entity *self, SoundCueSet *out) {
    self->methods->updateScale(self, 1, sScaleDouble);
    self->methods->addTranslation(self, sTranslateYMinus64);
}

/* Row 9: sounds program 10 every half animation cycle and walks forward 30 a tick. */
void Entity__CueWalkToTodBeat(Entity *self, SoundCueSet *out) {
    out->attenuation = self->methods->getProximityRatio(self);
    if (out->tick % (self->todFrameCount / 2) == 0) {
        out->slots[0].program = 10;
    }
    self->methods->moveLocalZ(self, -30, 0);
}

/* Row 10: sounds program 11 on three voices once, unattenuated, and walks forward 30 a tick. */
void Entity__CueChordThenWalk(Entity *self, SoundCueSet *out) {
    out->attenuation = 0;
    if (out->tick == 0) {
        out->slots[0].program = 11;
        out->slots[1].program = 11;
        out->slots[2].program = 11;
    }
    self->methods->moveLocalZ(self, -30, 0);
}

/* Row 11: walks a route at 20 a tick, climbing what it meets, and at moodTimer 1560 turns left or
 * right at random; each branch turns again later. If the dreamer presses the link button between
 * moodTimer 3421 and 3540 on the right-hand branch, it shrinks to half size, faces the dreamer and
 * chases at 120 a tick, and within 1024 of it ends the dream into the row's video. */
void Entity__CueWanderThenChaseIfLinkPressed(Entity *self, SoundCueSet *out) {
    enum { WANDER_RIGHT = 11, WANDER_LEFT = 12, WANDER_CHASE = 13 };

    Ratio16 *turn;

    self->lastOffsetValue = -20;
    out->attenuation = self->methods->getProximityRatio(self);
    turn = NULL;
    if (out->tick % (self->todFrameCount / 2) == 0) {
        out->slots[0].program = 10;
        out->slots[0].octave = 1;
    }
    if (self->state == WANDER_RIGHT) {
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
                self->state = WANDER_CHASE;
            }
        }
    } else if (self->state == WANDER_LEFT) {
        if (self->moodTimer == 1980) {
            turn = sRotationYawMinus90;
        }
    } else if (self->state == WANDER_CHASE) {
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
            self->state = WANDER_RIGHT;
        } else {
            turn = sRotationYawMinus90;
            self->state = WANDER_LEFT;
        }
    }
    if (turn != NULL) {
        self->methods->updateRotation(self, 0, turn);
    }
    self->methods->moveLocalZOrFindLink(self, self->lastOffsetValue, 0);
    if (self->state != WANDER_LEFT) {
        if (self->linkTarget != 0) {
            self->methods->moveLocalY(self, -200, 0);
        }
    }
}

/* Row 12: faces the dreamer while it is low (y below 2000). Half the time, once the dreamer comes
 * within 2560 it starts the stage's scale ramp and 150 ticks later ends the dream. */
void Entity__CueStretchStageNearDreamerThenEndDream(Entity *self, SoundCueSet *out) {
    enum { STRETCH_ARMED = 11, STRETCH_COUNTDOWN = 12 };

    s32 y;
    s32 dist;
    s32 timer;

    if (self->moodTimer == 0) {
        if ((rand() & 1) == 0) {
            self->state = STRETCH_ARMED;
        }
    }
    y = self->coord2->coord.t[1];
    if (y < 2000) {
        SceneNode__FaceTarget((SceneNode *)self, (SceneNode *)self->peer, 1, 0, 0);
    }
    if (self->state == STRETCH_ARMED) {
        dist = self->methods->distanceToPeer(self, self->peer);
        if (dist < 2560) {
            self->grid->methods->startScaleRamp(self->grid, 1, 1);
            self->moodTimer = 1;
            self->state = STRETCH_COUNTDOWN;
        }
    } else if (self->state == STRETCH_COUNTDOWN) {
        /* Counted here as well as by Entity__TickSoundCue: two per tick. */
        timer = self->moodTimer;
        self->moodTimer = timer + 1;
        if (timer == 300) {
            self->methods->notifyParents(self, ENTITY_EFFECT_END_DREAM);
        }
    }
}

/* Row 13: sounds program 12 at the start of every animation loop and counts the loops in `state`;
 * from the 36th it ends the dream into the row's video (one tick in three). */
void Entity__CueVideoAfter36TodLoops(Entity *self, SoundCueSet *out) {
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

/* Row 14: sounds program 13 at animation frame 10 and creeps forward 10 a tick. */
void Entity__CueCreepWithTone13(Entity *self, SoundCueSet *out) {
    out->attenuation = self->methods->getProximityRatio(self);
    if (self->todFrame == 10) {
        out->slots[0].program = 13;
    }
    self->methods->moveLocalZ(self, -10, 0);
}

/* Row 15: sounds program 15 once, unattenuated, on the cue's first tick. */
void Entity__CueTone15Once(Entity *self, SoundCueSet *out) {
    if (self->moodTimer == 0) {
        out->attenuation = 0;
        out->slots[0].program = 15;
    }
}

/* Row 16: half the time walks forward 90 a tick for 64 ticks, turns 90 degrees left or right, drops
 * 256 and runs on at 374 a tick; the other half spins a quarter turn every fifth tick, dashes 2048
 * a tick and flickers (shown one tick in seven). */
void Entity__CueWalkAndTurnOrSpinFlickering(Entity *self, SoundCueSet *out) {
    enum { SPIN_FLICKERING = 11 };

    Ratio16 *turn;
    s32 roll;

    if (self->moodTimer == 0) {
        if ((rand() & 1) != 0) {
            self->state = SPIN_FLICKERING;
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
    } else if (self->state == SPIN_FLICKERING) {
        if (self->moodTimer % 5 == 0) {
            self->methods->updateRotation(self, 0, sRotationYawPlus90);
        }
        self->methods->moveLocalZ(self, -2048, 0);
        self->methods->setDisplay(self, (rand() % 7) == 0);
    }
}

/* Row 17: holds half size. */
void Entity__CueHalfSize(Entity *self, SoundCueSet *out) {
    self->methods->updateScale(self, 1, sScaleHalf);
}

/* ---- MoodCue handlers, rows 19 to 38 and 119 ---------------------------
 *
 * Rows 19 to 27, 29 to 38 and 119. Row order does not follow address order,
 * so Entity__CueSixfoldSizeRow119 sits here among rows 19 to 38.
 */

/* Row 19: sounds program 17 every tenth tick and runs forward 256 a tick. */
void Entity__CueRunWithTone17(Entity *self, SoundCueSet *out) {
    out->attenuation = self->methods->getProximityRatio(self);
    if (out->tick % 10 == 0) {
        out->slots[0].program = 17;
    }
    self->methods->moveLocalZ(self, -256, 0);
}

/* Row 20: one time in seven stretches to twice its height at the start; sounds program 28 every
 * fourth tick and walks forward 100 a tick. */
void Entity__CueWalkMaybeTall(Entity *self, SoundCueSet *out) {
    if (self->moodTimer == 0 && rand() % 7 == 0) {
        self->methods->updateScale(self, 1, sScaleY2);
    }
    if ((out->tick & 3) == 0) {
        out->attenuation = self->methods->getProximityRatio(self);
        out->slots[0].program = 28;
    }
    self->methods->moveLocalZ(self, -100, 0);
}

/* Row 21: sounds program 10 every half animation cycle and program 13 three ticks later, and walks
 * forward 30 a tick, reporting the move. */
void Entity__CueWalkToTodBeatTwoTones(Entity *self, SoundCueSet *out) {
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

/* Row 22: faces the dreamer. */
void Entity__CueFaceDreamer(Entity *self, SoundCueSet *out) {
    SceneNode__FaceTarget((SceneNode *)self, (SceneNode *)self->peer, 1, 0, 0);
}

/* Row 23: faces the dreamer throughout. Appears 4096 up and drops back down over 64 ticks, rises
 * and edges in briefly, then rushes forward faster and faster until moodTimer 256 and backs off
 * after. Once the dreamer has reached it, it drifts upward and sideways instead. */
void Entity__CueDropInRushDreamerThenDriftUp(Entity *self, SoundCueSet *out) {
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

/* Row 24: holds double size and circles fast, turning 2 degrees and moving 512 a tick; sounds
 * program 7 every 15 ticks. */
void Entity__CueDoubleSizeCircleFast(Entity *self, SoundCueSet *out) {
    if (out->tick % 15 == 0) {
        out->attenuation = self->methods->getProximityRatio(self);
        out->slots[0].program = 7;
        out->slots[0].octave = -2;
    }
    self->methods->updateScale(self, 1, sScaleDouble);
    self->methods->updateRotation(self, 0, sRotationYawPlus2);
    self->methods->moveLocalZ(self, -512, 0);
}

/* Row 25: rises at 64 a tick with program 22 for 100 ticks, then at 256 a tick with program 12 on
 * three voices until moodTimer 300, then at 512 a tick while pitching forward an eighth of a degree
 * a tick. */
void Entity__CueRiseFasterThenPitchUp(Entity *self, SoundCueSet *out) {
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

/* Row 26: runs forward 384 a tick, climbing what it meets, with one 11520 lunge at moodTimer 110;
 * sounds program 26 every animation cycle. */
void Entity__CueRunAndLunge(Entity *self, SoundCueSet *out) {
    s32 v1;
    s32 zDelta;
    void (**moveZOrFindLink)(Entity *self, s32 val, void *notify);

    do { /* MATCHING: without the do/while(0), `self` and `out` swap registers */
        if (out->tick % self->todFrameCount == 0) {
            out->attenuation = self->methods->getProximityRatio(self);
            out->slots[0].program = 26;
            __asm__(""); /* MATCHING: keeps v1 = 110 below the program store */
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

/* Row 27: walks forward 128 a tick; sinks 32 a tick for its first 100 ticks and climbs 32 a tick
 * from moodTimer 301. Sounds program 27 every 70 ticks. */
void Entity__CueWalkDipThenClimb(Entity *self, SoundCueSet *out) {
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

/* Row 119: holds six times its size. Rows 117 and 118 have handlers with the same body
 * (Entity__CueSixfoldSizeRow117, Entity__CueSixfoldSizeRow118). */
void Entity__CueSixfoldSizeRow119(Entity *self, SoundCueSet *out) {
    self->methods->updateScale(self, 1, sScaleSix);
}

/* Row 29: in a white dream, triples its size and rises 30720 at the start; one time in five turns 1
 * degree a tick. */
void Entity__CueTripleAloftOnWhiteMaybeTurn(Entity *self, SoundCueSet *out) {
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

/* Row 30's handler, and row 122's (its data words differ). In a blue dream it
 * faces the dreamer and comes at it, drops a little between moodTimer 85 and 114, and from 120
 * hangs 1500 above the dreamer, 1024 off in z; in any other dream it holds double size and rises 30
 * a tick. */
void Entity__CueHoverOverDreamerOnBlueElseRise(Entity *self, SoundCueSet *out) {
    enum { HOVER_APPROACH = 11, HOVER_RISE = 12, HOVER_FOLLOW = 13 };

    if (self->state == 0) {
        if (((DreamSys *)self->peer)->methods->getDreamColor((DreamSys *)self->peer) == DREAM_COLOR_BLUE) {
            self->state = HOVER_APPROACH;
        } else {
            self->state = HOVER_RISE;
        }
    }

    if (self->state == HOVER_RISE) {
        self->methods->updateScale(self, 1, sScaleDouble);
        self->methods->moveLocalY(self, -30, 0);
    } else {
        SceneNode__FaceTarget((SceneNode *)self, (SceneNode *)self->peer, 1, 0, 0);
        if (self->state == HOVER_APPROACH) {
            self->methods->moveLocalZ(self, -100, 0);
            if ((u32)(self->moodTimer - 85) < 30) {
                self->methods->moveLocalY(self, 80, 0);
            } else if (self->moodTimer == 120) {
                self->state = HOVER_FOLLOW;
            }
        } else if (self->state == HOVER_FOLLOW) {
            self->methods->setTranslation(self, (LongVec3 *)((DreamSys *)self->peer)->coord2->coord.t);
            self->methods->addTranslation(self, sTranslateYMinus1500ZPlus1024);
        }
    }
}

/* Row 31: faces the dreamer and holds its controls, sounds program 13 on three voices in bursts,
 * and when its animation has played once stops it and links to the row's stage. */
void Entity__CueHoldDreamerThenLinkAfterTod(Entity *self, SoundCueSet *out) {
    SceneNode__FaceTarget((SceneNode *)self, (SceneNode *)self->peer, 1, 0, 0);
    ((DreamSys *)self->peer)->methods->clearTickCallbacks((DreamSys *)self->peer, true);
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

/* Row 32: walks forward 30 a tick. */
void Entity__CueWalk(Entity *self, SoundCueSet *out) {
    self->methods->moveLocalZ(self, -30, 0);
}

/* Row 33: walks forward 30 a tick, climbing what it meets, and sounds program 3 every 30 ticks;
 * once the dreamer has reached it, it holds a quarter of its size. */
void Entity__CueWalkShrinkWhenReached(Entity *self, SoundCueSet *out) {
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

/* Row 34: walks forward 60 a tick, turning 90 degrees at moodTimer 400 and back at 700; from 800
 * dashes 512 a tick, turning a further 36 degrees, and is gone from 851. */
void Entity__CueWalkZigzagThenDashAway(Entity *self, SoundCueSet *out) {
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

/* Row 35: jitters back and forth on all three axes, and every 500 ticks bobs up and back down. */
void Entity__CueWobble(Entity *self, SoundCueSet *out) {
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

/* Row 36: at the start grows to six or two times its size at random; faces the dreamer and backs
 * away 256 a tick while it is within 28672. */
void Entity__CueGrowThenBackAwayFromDreamer(Entity *self, SoundCueSet *out) {
    enum { GROWN = 11 };

    Ratio16 *scaleTemplate;
    s32 roll;

    if (self->state == 0) {
        roll = rand();
        scaleTemplate = sScaleSix;
        if ((roll & 1) != 0) {
            scaleTemplate = sScaleDouble;
        }
        self->methods->updateScale(self, 1, scaleTemplate);
        self->state = GROWN;
    }
    SceneNode__FaceTarget((SceneNode *)self, (SceneNode *)self->peer, 1, 0, 0);
    if (self->methods->distanceToPeer(self, self->peer) < 28672) {
        self->methods->moveLocalZ(self, 256, 0);
    }
}

/* Row 37: rises 90 a tick. */
void Entity__CueRise(Entity *self, SoundCueSet *out) {
    self->methods->moveLocalY(self, -90, 0);
}

/* Row 38: sounds program 1 every 120 ticks. */
void Entity__CueTone1Slow(Entity *self, SoundCueSet *out) {
    if (out->tick % 120 == 0) {
        out->attenuation = self->methods->getProximityRatio(self);
        out->slots[0].program = 1;
    }
}

/* ---- MoodCue handlers, rows 39 to 58 and 115 ---------------------------
 *
 * Rows 39 to 52, 55 to 58 and 115. Entity__CueIdle is empty: its row has
 * no per-tick effect. Entity__RollScaleOrDelayedDrift is not a row:
 * Entity__CueShuffleSideways and Entity__CueStepTurnThenShuffle call it first thing every tick.
 */

/* The z den of a scale template, three Ratio16s {1/1, 1/1, 1/zDenom} that
 * end here (sScaleX3's data runs on into the template's first ten bytes).
 * Entity__CueRunOffOrStopAndJitterDepth writes the den and passes the template. */
extern s16 sScaleTemplateZDenom;

/* Defined after Entity__CueShuffleSideways, which calls it. */
void Entity__RollScaleOrDelayedDrift(Entity *self);

/* Row 39: at the start, on some days of the dream calendar (getCurrentDayAndYear), stretches to
 * four times its height; sounds program 2 every 22 ticks, and at random stops and restarts its
 * animation. */
void Entity__CueStutterTodMaybeTall(Entity *self, SoundCueSet *out) {
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

/* Row 40: walks forward 30 a tick, climbing what it meets, and turns every 200 ticks (-90, 180,
 * +90, 180 degrees), then round again; sounds program 3 every seventh tick. */
void Entity__CuePatrolTurning(Entity *self, SoundCueSet *out) {
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

/* Row 41: four times in five runs 256 a tick and is gone at moodTimer 500
 * (Entity__StepYawInWindowsThenDeactivate); one time in five it stops at 320 and from then on
 * squashes its depth to a random 1/1 to 1/32 every fourth tick. */
void Entity__CueRunOffOrStopAndJitterDepth(Entity *self, SoundCueSet *out) {
    enum { RUN_OFF = 10, JITTER_DEPTH = 14 };

    s32 roll;
    s16 *zDenom;

    if (self->moodTimer == 0) {
        self->state = rand() % 5 + RUN_OFF;
    }
    if (self->state < JITTER_DEPTH || self->moodTimer < 320) {
        Entity__StepYawInWindowsThenDeactivate(self, out, 3000, 500, -256);
        return;
    }
    if (self->state == JITTER_DEPTH) {
        if ((self->moodTimer & 3) == 0) {
            roll = rand();
            zDenom = &sScaleTemplateZDenom;
            *zDenom = roll % 32 + 1;
            /* Back from the den to the start of its template. */
            /* MATCHING: the address is formed from sScaleTemplateZDenom, not sScaleX3 */
            self->methods->updateScale(self, 1, (Ratio16 *)(zDenom + 1) - 3);
        }
    }
}

/* Row 42: slides forward 30 a tick, still, for 20 ticks; then plays its animation with program 5,
 * and stops it and the tone after three cycles. */
void Entity__CueSlideInThenAnimateOnce(Entity *self, SoundCueSet *out) {
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

/* Row 43: rolls a size or a drift at the start (Entity__RollScaleOrDelayedDrift); sounds program 18
 * on two voices at animation frames 0 and 15; from moodTimer 321 shuffles sideways 60 a tick at
 * random while twitching its heading. */
void Entity__CueShuffleSideways(Entity *self, SoundCueSet *out) {
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

/* Row 44: rolls a size or a drift at the start (Entity__RollScaleOrDelayedDrift); sounds program 3
 * at animation frames 7 and 22; steps forward from moodTimer 300 to 319, turns from 321 to 339,
 * then shuffles sideways 128 a tick at random, reporting the move, while twitching its heading. */
void Entity__CueStepTurnThenShuffle(Entity *self, SoundCueSet *out) {
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
 * sScaleX3, 5 to 7 arms a drift (DRIFT_ARMED) that adds sTranslateZMinus256
 * every tick from tick 201 on. */
void Entity__RollScaleOrDelayedDrift(Entity *self) {
    enum { DRIFT_ARMED = 10 };

    s32 roll;

    if (self->moodTimer == 0) {
        roll = rand() % 10;
        if (roll >= 8) {
            self->methods->updateScale(self, 1, sScaleX3);
        } else if (roll >= 5) {
            self->state = DRIFT_ARMED;
        }
    }
    if (self->state == DRIFT_ARMED && self->moodTimer >= 201) {
        self->methods->addTranslation(self, sTranslateZMinus256);
    }
}

/* Row 45: does nothing; the row has no per-tick effect. */
void Entity__CueIdle(Entity *self, SoundCueSet *out) {}

/* Row 46: at the start, on some days of the dream calendar (getCurrentDayAndYear), grows to six
 * times its size; sounds program 18 once and faces the dreamer. */
void Entity__CueWatchDreamerMaybeGiant(Entity *self, SoundCueSet *out) {
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

/* Row 47: once the dreamer has reached it, gives 30 ticks: if the link button is pressed, it takes
 * the dreamer's movement, lifts the dreamer 100 a tick and ends the dream at moodTimer 100; if not,
 * it ends the dream into the row's video. */
void Entity__CueLiftDreamerIfLinkPressedElseVideo(Entity *self, SoundCueSet *out) {
    enum { LIFT_VIDEO_SENT = 10, LIFT_CARRYING = 11, LIFT_LINK_WINDOW = 12 };

    if (self->targetReached == 0) {
        return;
    }
    if (self->state == 0) {
        self->state = LIFT_LINK_WINDOW;
        self->moodTimer = 0;
        return;
    }
    if (self->state == LIFT_LINK_WINDOW) {
        if (self->moodTimer < 30) {
            if (((DreamSys *)self->peer)->methods->getLinkCommandFlag((DreamSys *)self->peer) != 0) {
                ((DreamSys *)self->peer)->methods->clearTickCallbacks((DreamSys *)self->peer, false);
                self->moodTimer = 0;
                self->state = LIFT_CARRYING;
            }
        } else {
            self->methods->notifyParents(self, ENTITY_EFFECT_EVENT_VIDEO);
            self->state = LIFT_VIDEO_SENT;
        }
    } else if (self->state == LIFT_CARRYING) {
        if (self->moodTimer == 100) {
            self->methods->notifyParents(self, ENTITY_EFFECT_END_DREAM);
        } else {
            ((DreamSys *)self->peer)->methods->moveLocalY((DreamSys *)self->peer, -100, 0);
        }
    }
}

/* Row 48: bobs up and down 30 a tick and walks forward 30 a tick, reporting the move; at animation
 * frame 38 faces the dreamer and sounds program 6. */
void Entity__CueBobAndWalk(Entity *self, SoundCueSet *out) {
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

/* Row 49: sounds program 4 on three voices on tick 6 and runs forward 256 a tick. Once the dreamer
 * has reached it, it links to the row's stage 10 ticks later, unless the link button is pressed
 * first: then it turns the dreamer, takes its movement, carries it along at its own position and
 * links 100 ticks later. */
void Entity__CueRunCarryDreamerIfLinkPressed(Entity *self, SoundCueSet *out) {
    enum { CARRY_LINK_WINDOW = 10, CARRY_CARRYING = 11 };

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
            self->state = CARRY_LINK_WINDOW;
            self->moodTimer = 0;
        } else if (self->state == CARRY_LINK_WINDOW) {
            if (self->moodTimer == 10) {
                self->methods->notifyParents(self, ENTITY_EFFECT_LINK_STAGE);
            } else if (((DreamSys *)self->peer)->methods->getLinkCommandFlag((DreamSys *)self->peer) != 0) {
                peerMethods = ((DreamSys *)self->peer)->methods;
                translation = self->parent ? self->coord2->workm.t : NULL;
                peerMethods->setTranslation((DreamSys *)self->peer, translation);
                ((DreamSys *)self->peer)->methods->updateRotation((DreamSys *)self->peer, 1, sRotationYawMinus90);
                ((DreamSys *)self->peer)->methods->clearTickCallbacks((DreamSys *)self->peer, false);
                self->moodTimer = 0;
                self->state = CARRY_CARRYING;
            }
        } else if (self->state == CARRY_CARRYING) {
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

/* Row 50: sounds program 7 on three voices at animation frames 15 and 70 for five animation cycles,
 * then deactivates and is done. */
void Entity__CueChordFiveTodLoopsThenLeave(Entity *self, SoundCueSet *out) {
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

/* Row 51's handler, and row 113's through Entity__CueWalkWithTurnsMaybeGiantRow113 (entity.h). */
void Entity__CueWalkWithTurnsMaybeGiant(Entity *self, SoundCueSet *out) {
    enum { GIANT = 11 };

    Ratio16 *table;

    if (self->moodTimer == 0 && rand() % 5 == 0 && self->state == 0) {
        self->methods->updateScale(self, 1, sScaleSix);
        self->methods->moveLocalY(self, 800, 0);
        self->state = GIANT;
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

/* Row 52: runs forward 512 a tick, climbing what it meets, with program 9 every 20 ticks; turns
 * about at moodTimer 84, veers 9 degrees a tick from 188 to 199, and at 200 deactivates with
 * program 30 and is done. */
void Entity__CueRunTurnBackThenVanish(Entity *self, SoundCueSet *out) {
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

/* Row 55: one time in three stretches to twice its height at the start; sounds program 19 at frames
 * 9, 17 and 23 of every 32 (a second voice on 23). */
void Entity__CueTone19RhythmMaybeTall(Entity *self, SoundCueSet *out) {
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

/* Row 56: rises 200 on the cue's first tick. */
void Entity__CueLiftOnce(Entity *self, SoundCueSet *out) {
    if (self->moodTimer == 0) {
        self->methods->moveLocalY(self, -200, 0);
    }
}

/* Row 57: one time in three it starts still and 6144 underground, runs forward 512 a tick with
 * program 12, climbs out from moodTimer 128 to 321, and at 322 animates and is done. Otherwise it
 * plays its animation with tones; at frame 48, if the dreamer is near, it fades the screen down and
 * half the time ends the dream into the row's video; at frame 59 it deactivates and is done. */
void Entity__CueSurfaceRunningOrFadeOutNearDreamer(Entity *self, SoundCueSet *out) {
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

/* Row 58: sounds program 12 now and then. In a yellow dream, once the dreamer is within 1024, it
 * takes the dreamer's movement, lifts it for 50 ticks, sways it side to side and ends the dream at
 * moodTimer 500. Otherwise one time in three, once the dreamer is within 1024, it stops, sinks for
 * 10 ticks, then falls on its side with programs 18 and 3, hides its second part and is done. */
void Entity__CueLevitateDreamerOnYellowOrCollapse(Entity *self, SoundCueSet *out) {
    enum { LEVITATE_ARMED = 11, COLLAPSE_ARMED = 12, LEVITATING = 13, COLLAPSING = 14 };

    if (self->moodTimer == 0) {
        out->attenuation = 0;
        out->slots[0].program = 12;
        if (((DreamSys *)self->peer)->methods->getDreamColor((DreamSys *)self->peer) ==
            DREAM_COLOR_YELLOW) {
            self->state = LEVITATE_ARMED;
        } else if (rand() % 3 == 0) {
            self->state = COLLAPSE_ARMED;
        }
    }
    if (out->tick % 100 == 0) {
        out->attenuation = self->methods->getProximityRatio(self);
        out->slots[0].program = 12;
        out->slots[0].octave = -1;
    }
    if (self->state == LEVITATE_ARMED) {
        if (self->methods->distanceToPeer(self, self->peer) < 1024) {
            ((DreamSys *)self->peer)->methods->clearTickCallbacks((DreamSys *)self->peer, false);
            self->state = LEVITATING;
            self->moodTimer = 0;
        }
    } else if (self->state == COLLAPSE_ARMED) {
        if (self->methods->distanceToPeer(self, self->peer) < 1024) {
            self->methods->stopTod(self);
            self->state = COLLAPSING;
            self->moodTimer = 0;
        }
    }
    if (self->state == LEVITATING) {
        if (self->moodTimer < 50) {
            ((DreamSys *)self->peer)->methods->moveLocalY((DreamSys *)self->peer, -20, 0);
        } else if (self->moodTimer < 500) {
            ((DreamSys *)self->peer)
                ->methods->moveLocalX((DreamSys *)self->peer, (self->moodTimer % 40 < 20) ? -5 : 5, 0);
        } else if (self->moodTimer == 500) {
            self->methods->notifyParents(self, ENTITY_EFFECT_END_DREAM);
        }
    }
    if (self->state == COLLAPSING) {
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

/* Row 115: waits, inactive, for the dreamer to come within 2048; then it and the dreamer turn to
 * face each other, it activates, takes the dreamer's movement and look, and bounces. Half the time,
 * at moodTimer 100, it stops, sinks, steps forward and at last walks at the dreamer; at 240 the
 * dreamer's control is given back. Within 512 of the dreamer it deactivates and links to the row's
 * stage. */
void Entity__CueConfrontDreamerThenLinkOnTouch(Entity *self, SoundCueSet *out) {
    enum { CONFRONTING = 11, CONFRONT_ADVANCE = 12 };

    if (self->state == 0) {
        if (Entity__IsTargetInRange(self, 2048) != 0) {
            self->state = CONFRONTING;
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
            self->state = CONFRONT_ADVANCE;
            self->methods->stopTod(self);
        }
    } else if (self->moodTimer == 240) {
        ((DreamSys *)self->peer)
            ->methods->setTickCallbacks((DreamSys *)self->peer, MOVE_CALLBACK_TICK_MOVE,
                                        LOOK_CALLBACK_STEP_LOOK);
    }
    if (self->state == CONFRONT_ADVANCE) {
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
 * Rows 59, 61, 62, 64 to 71 and 73 to 81 (rows 60, 63 and 72 have none).
 * Entity__CueWalkInRandomLaneGiant (below) runs Entity__CueWalkInRandomLane and then sets its scale
 * to sScaleSix.
 */

/* Row 59: half the time sinks 2048 at the start; walks forward 128 a tick with program 12 every
 * tenth tick; one time in ten starts the stage's scale ramp at moodTimer 300. */
void Entity__CueWalkMaybeSunkMaybeStretchStage(Entity *self, SoundCueSet *out) {
    enum { STRETCH_ARMED = 12 };

    if (self->moodTimer == 0 && rand() % 10 == 0) {
        self->state = STRETCH_ARMED;
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
    if (self->state == STRETCH_ARMED && self->moodTimer == 300) {
        self->grid->methods->startScaleRamp(self->grid, 1, 1);
    }
}

/* Row 61: sounds program 18 at animation frame 30, unattenuated. */
void Entity__CueTone18AtFrame30(Entity *self, SoundCueSet *out) {
    if (self->todFrame == 30) {
        out->slots[0].program = 18;
        out->attenuation = 0;
        out->slots[0].octave = -1;
    }
}

/* Row 62: creeps forward 5 a tick, facing the dreamer from moodTimer 101, with program 3 every 30
 * animation frames; at 300, if the dreamer is within 4096, takes its movement, and at 500 gives it
 * back. Once the dreamer is within 1024, half the time it sounds program 6, maybe shrinks the stage
 * (its scale ramp), and 70 ticks later ends the dream, plainly or into the row's video. */
void Entity__CueCreepUpAndHoldDreamer(Entity *self, SoundCueSet *out) {
    enum { CREEP_ENDING = 10, CREEP_SPARED = 11 };

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
                self->state = CREEP_ENDING;
            } else {
                self->state = CREEP_SPARED;
            }
        }
    }
    if (self->state == CREEP_ENDING && self->moodTimer == 70) {
        self->methods->notifyParents(self, (rand() & 1) ? ENTITY_EFFECT_END_DREAM
                                                        : ENTITY_EFFECT_EVENT_VIDEO);
    }
}

/* Row 64: sounds program 5 for 20 ticks out of every 300 and creeps forward 10 a tick. */
void Entity__CueCreepWithTone5Bursts(Entity *self, SoundCueSet *out) {
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

/* Row 65: one time in three, at the start, shrinks to half size, rises 300 and turns 90 degrees; it
 * then walks forward 20 a tick and turns back at moodTimer 2000. */
void Entity__CueMaybeHalfSizeWalkAloft(Entity *self, SoundCueSet *out) {
    enum { ALOFT = 11 };

    if (self->moodTimer == 0) {
        if (rand() % 3 == 0) {
            self->methods->updateScale(self, 1, sScaleHalf);
            self->methods->moveLocalY(self, -300, 0);
            self->methods->updateRotation(self, 1, sRotationYawPlus90);
            self->state = ALOFT;
        }
    }
    if (self->state == ALOFT) {
        if (self->moodTimer == 2000) {
            self->methods->updateRotation(self, 0, sRotationYawMinus90);
        }
        self->methods->moveLocalZ(self, -20, 0);
    }
}

/* Row 66: sounds program 13 every 30 ticks. */
void Entity__CueTone13Every30(Entity *self, SoundCueSet *out) {
    out->attenuation = self->methods->getProximityRatio(self);
    if (out->tick % 30 == 0) {
        out->slots[0].program = 13;
    }
}

/* Row 67: one time in three runs forward 512 a tick from moodTimer 501, dropping 2048 and facing
 * the dreamer at 502. */
void Entity__CueMaybeDropAndRunAt500(Entity *self, SoundCueSet *out) {
    enum { RUNNER = 11 };

    if (self->moodTimer == 0) {
        if (rand() % 3 == 0) {
            self->state = RUNNER;
        }
    }
    if (self->state == RUNNER) {
        if (self->moodTimer == 502) {
            self->methods->moveLocalY(self, 2048, 0);
            SceneNode__FaceTarget((SceneNode *)self, (SceneNode *)self->peer, 1, 0, 0);
        }
        if (self->moodTimer >= 501) {
            self->methods->moveLocalZ(self, -512, 0);
        }
    }
}

/* Row 68: faces the dreamer and chases at 374 or 192 a tick (picked at the start), reporting the
 * move, with a pattern of programs 28 and 23. If the link button is pressed, it rolls over and
 * sinks for 8 ticks, ends its cue with programs 18 and 3, and half the time is done; otherwise it
 * chases again. */
void Entity__CueChaseDreamerTumbleIfLinkPressed(Entity *self, SoundCueSet *out) {
    enum { TUMBLING = 10 };

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
                self->state = TUMBLING;
                out->slots[0].program = 18;
            }
        }
        SceneNode__FaceTarget((SceneNode *)self, (SceneNode *)self->peer, 1, 0, 0);
        self->methods->moveLocalZ(self, self->lastOffsetValue, (void *)1);
    } else if (self->state == TUMBLING) {
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

/* Row 69: layers three tones: program 25 at the end of each animation cycle, program 21 every
 * fourth tick and program 13 every 200. */
void Entity__CueLayeredTones(Entity *self, SoundCueSet *out) {
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

/* Row 70: runs 256 a tick for 600 ticks, forward or (half the time) backward, turning about at
 * moodTimer 300. */
void Entity__CueRunOutAndBack(Entity *self, SoundCueSet *out) {
    enum { BACKWARD = 11 };

    if (self->moodTimer == 0) {
        if (rand() & 1) {
            self->state = BACKWARD;
        }
    }
    if (self->moodTimer == 300) {
        self->methods->updateRotation(self, 0, sRotationYawPlus180);
    }
    if (self->moodTimer < 600) {
        self->methods->moveLocalZ(self, self->state == 0 ? -256 : 256, 0);
    }
}

/* Row 71's handler, and row 108's through Entity__CueWalkInRandomLaneGiant (entity.h). */
void Entity__CueWalkInRandomLane(Entity *self, SoundCueSet *out) {
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

/* Row 73: on its first tick turns the dreamer 90 degrees, takes the dreamer's movement and look,
 * and sounds program 25 on three voices; deactivates when its animation has played once. */
void Entity__CueTurnAndHoldDreamer(Entity *self, SoundCueSet *out) {
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

/* {0, 100, 190}: the clear colour Entity__CueBlueSkyPushDreamer gives the peer's viewport. */
extern ColorRgb sCueBlueSkyClearColor;

/* Row 74: turns the sky blue (the viewport's clear colour) at the start. Two times in three, unless
 * the dreamer's z is below 610, it pushes the dreamer back 128 a tick from mid-animation and links
 * to the row's stage 30 ticks before the animation ends; otherwise it pushes the dreamer forward
 * faster and faster from moodTimer 20 to 119, giving its control back at 85. */
void Entity__CueBlueSkyPushDreamer(Entity *self, SoundCueSet *out) {
    if (self->moodTimer == 0) {
        ((DreamSys *)self->peer)
            ->viewport->methods->setClearColor(((DreamSys *)self->peer)->viewport, &sCueBlueSkyClearColor);
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

/* Row 75: faces the dreamer and sounds program 25 on three voices on its first tick; when its
 * animation has played once, stops it and links to the row's stage. */
void Entity__CueFaceDreamerThenLinkAfterTod(Entity *self, SoundCueSet *out) {
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

/* Row 76: until the dreamer reaches it, stands still turning 9 degrees a tick; then plays its
 * animation, and on its last frame stops and shrinks by 1/64. */
void Entity__CueSpinUntilReachedThenShrink(Entity *self, SoundCueSet *out) {
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

/* Row 77: plays its first animation once, then its second, walking a route at 50 a tick, climbing
 * what it meets, with quarter turns at set times; half the time the route is a short one that ends
 * the cue at moodTimer 174. Sounds program 17 every fifth tick. */
void Entity__CueWalkTurningRoute(Entity *self, SoundCueSet *out) {
    enum { SHORT_ROUTE = 11 };

    out->attenuation = self->methods->getProximityRatio(self);
    if (out->tick % 5 == 0) {
        out->slots[0].program = 17;
        out->slots[0].octave = -2;
    }
    if (self->todIndex == 0) {
        if (self->moodTimer == self->todFrameCount) {
            self->methods->setTod(self, 1);
            if (rand() & 1) {
                self->state = SHORT_ROUTE;
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

/* Entity__CueStepForwardAndBack's: cleared on the cue's first tick, set when its phase
 * 11 ends the cue at tick 510; at tick 520 a set flag ends it again. */
extern s32 sCueStepForwardAndBackDone;

/* Row 78: each animation cycle rests, steps forward 110 a tick, steps back 110 a tick, then
 * animates; sounds program 28 while animating. One time in three it rises 380, pitches up 90
 * degrees and ends at tick 510; one time in three, from tick 330, it switches between double and
 * normal height every 60 ticks. */
void Entity__CueStepForwardAndBack(Entity *self, SoundCueSet *out) {
    enum { PITCH_UP = 11, STRETCH_NORMAL = 12, STRETCH_TALL = 13 };

    s32 rollOrDy; /* MATCHING: one local for the roll and then the y move; two allocate differently */
    void *table;

    if (out->tick == 0) {
        sCueStepForwardAndBackDone = 0;
        rollOrDy = rand() % 3;
        if (rollOrDy == 1) {
            self->state = PITCH_UP;
        }
        if (rollOrDy == 2) {
            self->state = STRETCH_NORMAL;
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

    if (self->state == PITCH_UP && out->tick == 510) {
        self->methods->moveLocalY(self, -380, 0);
        self->methods->updateRotation(self, 0, sRotationXPlus90);
        self->methods->stopSoundCue(self);
        self->state = ENTITY_STATE_DONE;
        sCueStepForwardAndBackDone = 1;
    } else if (self->state >= STRETCH_NORMAL && out->tick >= 330 && (out->tick % 60) == 30) {
        rollOrDy = 0;
        if (rand() & 1) {
            table = sScaleY2;
            rollOrDy = (self->state == STRETCH_NORMAL) ? 400 : 0;
            self->state = STRETCH_TALL;
        } else {
            table = sScaleUnit;
            if (self->state == STRETCH_TALL) {
                rollOrDy = -400;
            }
            self->state = STRETCH_NORMAL;
        }
        self->methods->updateScale(self, 1, table);
        self->methods->moveLocalY(self, rollOrDy, 0);
    }

    if (out->tick == 520 && sCueStepForwardAndBackDone != 0) {
        self->methods->stopSoundCue(self);
        self->state = ENTITY_STATE_DONE;
    }
}

/* Row 79: turns 2 degrees a tick with program 25 every tenth tick; when the dreamer reaches it,
 * ends the dream into the row's video. */
void Entity__CueSpinVideoWhenReached(Entity *self, SoundCueSet *out) {
    enum { VIDEO_SENT = 11 };

    out->attenuation = self->methods->getProximityRatio(self);
    if (out->tick % 10 == 0) {
        out->slots[0].program = 25;
        out->slots[0].octave = 2;
    }
    self->methods->updateRotation(self, 0, sRotationYawPlus2);
    if (self->state == 0 && self->targetReached != 0) {
        self->methods->notifyParents(self, ENTITY_EFFECT_EVENT_VIDEO);
        self->state = VIDEO_SENT;
    }
}

/* Row 80: faces the dreamer; plays its animation once, with program 16 at frame 20, then stops and
 * shoots up 512 a tick. */
void Entity__CueAnimateThenShootUp(Entity *self, SoundCueSet *out) {
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

/* Row 81's handler, and row 120's (its data words differ). It walks forward 20 a tick, climbing
 * what it meets, with program 28, faces the dreamer from moodTimer 501, and moves the dreamer 2048
 * along its facing when it comes within 2048. One time in three, once the dreamer has reached it,
 * it instead faces the dreamer with a tone pattern and either grows and chases at 374 a tick until
 * within 512, or takes the dreamer's movement, turns the dreamer to face it, and approaches at 96 a
 * tick (or not at all), ending the dream within 2400. */
void Entity__CueWalkThenChaseOrHoldDreamer(Entity *self, SoundCueSet *out) {
    enum { HOLD_ENDED = 11 };

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
                    self->state = HOLD_ENDED;
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
 * Rows 82 to 96. Entity__CueFadeSkipTodThenLink and Entity__CueAwaitReachThenLinkAfterTod skip their TOD
 * animation ahead to frame 24 by stepping applyTodFrame, which returns the
 * next frame's pointer. SetCueTones7_7_7 and SetCueTones18_3_3 are not
 * rows: they write a fixed three-voice request into the set, unattenuated
 * (programs 7, 7, 7 at octave -2, and 18, 3, 3), for Entity__CueFadeAndTurnDreamerThenLink and
 * Entity__CuePauseThenAnimateOnce; Entity__CueJumpAheadHoldDreamerOrStand and Entity__CueAnimateOnceMaybeVideo write the 18, 3
 * request inline.
 */

/* Defined after Entity__CuePauseThenAnimateOnce, which calls them. */
void SetCueTones7_7_7(SoundCueSet *out);
void SetCueTones18_3_3(SoundCueSet *out);

/* Row 82: one time in three it ends its cue at once and jumps 20480 ahead. Otherwise, at random, it
 * either faces the dreamer, takes the dreamer's movement at moodTimer 20 with programs 18 and 3,
 * walks forward 40 a tick and links to the row's stage at 40; or stands still, and when the dreamer
 * comes within 512 deactivates (two times in three) or ends its cue. */
void Entity__CueJumpAheadHoldDreamerOrStand(Entity *self, SoundCueSet *out) {
    enum { STAND_GUARD = 11, HOLD_AND_LINK = 12 };

    if (self->state == 0 && self->moodTimer == 0) {
        if (rand() % 3 != 0) {
            self->state = (rand() & 1) ? STAND_GUARD : HOLD_AND_LINK;
        } else {
            self->methods->stopSoundCue(self);
            self->methods->moveLocalZ(self, -20480, 0);
            rand();
        }
    }
    if (self->state == HOLD_AND_LINK) {
        SceneNode__FaceTarget((SceneNode *)self, (SceneNode *)self->peer, 1, 0, 0);
        if (self->moodTimer == 20) {
            out->slots[0].program = 18;
            out->attenuation = 0;
            out->slots[1].program = 3;
            ((DreamSys *)self->peer)->methods->clearTickCallbacks((DreamSys *)self->peer, true);
        }
        if (self->moodTimer >= 21) {
            self->methods->moveLocalZ(self, -40, 0);
        }
        if (self->moodTimer == 40) {
            self->methods->notifyParents(self, ENTITY_EFFECT_LINK_STAGE);
        }
    } else if (self->state == STAND_GUARD) {
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

/* Row 83: sounds program 12 every 15 animation frames, unattenuated, and ends its cue when the
 * animation has played once. */
void Entity__CueTone12ForOneTod(Entity *self, SoundCueSet *out) {
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

/* Row 84: sounds a sequence of tones over its animation (programs 12 and 5 to frame 40, 18 at 45, 7
 * at 64) and ends its cue at frame 89. */
void Entity__CueToneSequenceOverTod(Entity *self, SoundCueSet *out) {
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

/* Row 85: plays its animation once with the 7, 7, 7 chord, then turns 9 degrees a tick for 10
 * ticks. If the link button is pressed then, it rolls back and ends its cue. If not, it takes the
 * dreamer's movement, faces the dreamer and edges in, fades the screen down and back up while
 * rolling the dreamer slowly, then turns the dreamer about and links to the row's stage (one time
 * in five it ends the dream instead). */
void Entity__CueFadeAndTurnDreamerThenLink(Entity *self, SoundCueSet *out) {
    enum {
        FADE_TURN_WINDOW = 10,
        FADE_APPROACH = 11,
        FADE_ROLL_BACK = 12,
        FADE_ROLL_DREAMER = 13,
        FADE_LINKED = 14
    };

    if (self->state == 0) {
        if (self->todFrame == 5) {
            SetCueTones7_7_7(out);
        }
        if (self->moodTimer == self->todFrameCount) {
            self->methods->stopTod(self);
            self->state = FADE_TURN_WINDOW;
            self->moodTimer = -1;
        }
    } else if (self->state == FADE_TURN_WINDOW) {
        if (self->moodTimer < 10) {
            self->methods->updateRotation(self, 0, sRotationYawPlus9);
            if (((DreamSys *)self->peer)->methods->getLinkCommandFlag((DreamSys *)self->peer) != 0) {
                SetCueTones7_7_7(out);
                self->state = FADE_ROLL_BACK;
                self->moodTimer = -1;
            }
        } else {
            ((DreamSys *)self->peer)->methods->clearTickCallbacks((DreamSys *)self->peer, true);
            self->state = FADE_APPROACH;
            self->moodTimer = -1;
        }
    } else if (self->state == FADE_APPROACH) {
        SceneNode__FaceTarget((SceneNode *)self, (SceneNode *)self->peer, 1, 0, 0);
        if (self->moodTimer < 30) {
            self->methods->moveLocalZ(self, -10, 0);
        } else {
            SetCueTones7_7_7(out);
            if (Entity__GetOrCreateFadeBox(self, NULL, NULL, (void *)30, 0) != NULL) {
                self->fadeBox->methods->startFadeDown(self->fadeBox, (BasicClass *)self->ticker, 7, 0);
            }
            self->state = FADE_ROLL_DREAMER;
            self->moodTimer = -1;
        }
    } else if (self->state == FADE_ROLL_DREAMER) {
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
            self->state = FADE_LINKED;
        }
    } else if (self->state == FADE_ROLL_BACK) {
        if (self->moodTimer < 10) {
            self->methods->updateRotation(self, 0, sRotationZMinus9);
        } else {
            SetCueTones18_3_3(out);
            self->methods->stopSoundCue(self);
            self->state = ENTITY_STATE_DONE;
        }
    }
}

/* Row 86: holds its animation still for 10 ticks, plays it once with the 18, 3, 3 chord at frame
 * 10, and ends its cue. */
void Entity__CuePauseThenAnimateOnce(Entity *self, SoundCueSet *out) {
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

/* Row 87: sounds program 18 at the start of every animation cycle. */
void Entity__CueTone18OnTodLoop(Entity *self, SoundCueSet *out) {
    out->attenuation = self->methods->getProximityRatio(self);
    if (out->tick == 0) {
        out->slots[0].program = 18;
    }
    if (out->tick >= self->todFrameCount - 1) {
        out->tick = -1;
    }
}

/* Row 88: sounds program 18 halfway through every animation cycle. */
void Entity__CueTone18MidTodLoop(Entity *self, SoundCueSet *out) {
    out->attenuation = self->methods->getProximityRatio(self);
    if (out->tick == self->todFrameCount / 2) {
        out->slots[0].program = 18;
    }
    if (out->tick >= self->todFrameCount - 1) {
        out->tick = -1;
    }
}

/* Row 89: sounds programs 18 and 3 at moodTimer 20; when its animation has played once, ends its
 * cue and half the time ends the dream into the row's video. */
void Entity__CueAnimateOnceMaybeVideo(Entity *self, SoundCueSet *out) {
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

/* Row 90: sounds program 1 or 2 (at random) once, two octaves up. */
void Entity__CueRandomHighToneOnce(Entity *self, SoundCueSet *out) {
    out->attenuation = self->methods->getProximityRatio(self);
    if (out->tick == 0) {
        out->slots[0].program = (rand() & 1) ? 2 : 1;
        out->slots[0].octave = 2;
    }
}

/* Row 91: holds a stretched size (4/5 wide, 6/5 tall). On the cue's first tick it starts the screen
 * fading down, half the time rising 256; it then skips its animation ahead to frame 24, and from
 * frame 25 walks forward 20 a tick and takes the dreamer's movement; program 21 at moodTimer 12,
 * and a link to the row's stage at 50. */
void Entity__CueFadeSkipTodThenLink(Entity *self, SoundCueSet *out) {
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
        ((DreamSys *)self->peer)->methods->clearTickCallbacks((DreamSys *)self->peer, true);
    }
    if (self->moodTimer == 50) {
        self->methods->notifyParents(self, ENTITY_EFFECT_LINK_STAGE);
    } else if (self->moodTimer == 12) {
        out->attenuation = 0;
        out->slots[0].program = 21;
    }
    self->methods->updateScale(self, 1, sScaleXFourFifthsYSixFifths);
}

/* Row 92: holds a stretched size (4/5 wide, 6/5 tall) and skips the first 24 frames of its first
 * animation on every loop until the dreamer reaches it; then it faces the dreamer, switches to its
 * second animation and takes the dreamer's movement, and at that animation's last frame sounds
 * program 18 and links to the row's stage. */
void Entity__CueAwaitReachThenLinkAfterTod(Entity *self, SoundCueSet *out) {
    if (self->todIndex == 0) {
        if (self->targetReached != 0) {
            SceneNode__FaceTarget((SceneNode *)self, (SceneNode *)self->peer, 1, 0, 0);
            self->methods->setTod(self, 1);
            ((DreamSys *)self->peer)->methods->clearTickCallbacks((DreamSys *)self->peer, true);
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

/* Row 93's handler, and row 107's (its data words differ). It sounds program 3 every tenth tick,
 * plays its first animation once, then switches to its second and runs forward 128 a tick,
 * reporting the move. */
void Entity__CueAnimateThenRun(Entity *self, SoundCueSet *out) {
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

/* Row 94: as Entity__CueAnimateThenRun with program 14, and when it switches animation it may grow
 * to six times its size and sink 2048 (half the time, and only outside a white dream) and may turn
 * about (one time in three). */
void Entity__CueAnimateThenRunMaybeGiantOrTurn(Entity *self, SoundCueSet *out) {
    enum { MAY_GROW = 11 };

    if (self->moodTimer == 0) {
        if (((DreamSys *)self->peer)->methods->getDreamColor((DreamSys *)self->peer) != DREAM_COLOR_WHITE) {
            self->state = MAY_GROW;
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

/* Row 95: plays its fourth animation once, then switches to its second and runs forward 128 a tick;
 * no tone. */
void Entity__CueSilentAnimateThenRun(Entity *self, SoundCueSet *out) {
    if (self->moodTimer == 0) {
        self->methods->setTod(self, 3);
    } else if (self->moodTimer == self->todFrameCount) {
        self->methods->setTod(self, 1);
    }
    if (self->todIndex == 1) {
        self->methods->moveLocalZ(self, -128, 0);
    }
}

/* Row 96: picks one of its four animations at random at the start and at the end of every cycle,
 * sounding program 22 two octaves up, fading, each time it picks. */
void Entity__CueRandomTodEachCycle(Entity *self, SoundCueSet *out) {
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
 * Rows 98, 102 to 106, 108 to 111, 113, 114, 117, 118, 121, 123, 125, 128
 * and 129. Entity__StepYawInWindowsThenDeactivate is not a row:
 * Entity__CueWanderPauseOnPink (twice) and Entity__CueRunOffOrStopAndJitterDepth call it.
 */

/* Row 98: walks forward 30 a tick, reporting the move; when the dreamer reaches it, it fades the
 * screen down, deactivates and clears the dreamer's flashback list. */
void Entity__CueWalkFadeAndResetFlashbacks(Entity *self, SoundCueSet *out) {
    if (self->targetReached != 0) {
        if (Entity__GetOrCreateFadeBox(self, NULL, 0, 10, 0) != 0) {
            self->fadeBox->methods->startFadeDown(self->fadeBox, (BasicClass *)self->ticker, 7, 0);
            self->methods->deactivate(self);
            ((DreamSys *)self->peer)->methods->resetFlashbackList((DreamSys *)self->peer);
        }
    }
    self->methods->moveLocalZ(self, -30, (void *)1);
}

/* Row 102: rises 512 at the start and runs forward 256 a tick, turning slowly from moodTimer 51;
 * from 781 it faces the dreamer at an eighth of its size, grows back to full size in steps from
 * 1921 to 1936, walks at 64 a tick and is done at 2000. When the dreamer reaches it, it takes the
 * dreamer's movement, links to the row's stage and pushes the dreamer back 256 a tick. */
void Entity__CueCircleThenRegrowFacingDreamer(Entity *self, SoundCueSet *out) {
    enum { PUSH_DREAMER = 12 };

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
        self->state = PUSH_DREAMER;
        ((DreamSys *)self->peer)->methods->clearTickCallbacks((DreamSys *)self->peer, true);
        self->methods->notifyParents(self, ENTITY_EFFECT_LINK_STAGE);
    }
    if (self->state == PUSH_DREAMER) {
        ((DreamSys *)self->peer)->methods->moveLocalZ((DreamSys *)self->peer, 256, 0);
    }
}

/* Row 103: walks forward 30 a tick. One time in three, from moodTimer 700 it spirals down, turning
 * and sinking until 1019, and links to the row's stage at 930; otherwise, at 100 and 800, one time
 * in five it starts the stage's scale ramp. */
void Entity__CueWalkMaybeSpiralDownToLink(Entity *self, SoundCueSet *out) {
    enum { SPIRAL_DOWN = 11 };

    if (self->moodTimer == 700) {
        if (rand() % 3 == 0) {
            self->state = SPIRAL_DOWN;
        }
    }
    if (self->state == SPIRAL_DOWN) {
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

/* Row 104: holds a quarter of its size and rises 32 a tick from moodTimer 201 to 299. */
void Entity__CueQuarterSizeRiseBriefly(Entity *self, SoundCueSet *out) {
    self->methods->updateScale(self, 1, sScaleQuarter);
    if (self->moodTimer >= 201 && self->moodTimer < 300) {
        self->methods->moveLocalY(self, -32, 0);
    }
}

/* Row 105: flickers, shown one tick in twenty. */
void Entity__CueFlicker(Entity *self, SoundCueSet *out) {
    self->methods->setDisplay(self, rand() % 20 == 0);
}

/* Row 106: half the time stands still, thin and tall (1/8 wide and deep, twice its height);
 * otherwise plays its second animation and sways side to side 32 a tick. */
void Entity__CueSwayOrStandThin(Entity *self, SoundCueSet *out) {
    enum { STAND_THIN = 11 };

    if (self->moodTimer == 0) {
        if ((rand() & 1) == 0) {
            self->state = STAND_THIN;
        }
    }
    if (self->state == STAND_THIN) {
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

/* Row 108: runs Entity__CueWalkInRandomLane, then holds six times its size. */
void Entity__CueWalkInRandomLaneGiant(Entity *self, SoundCueSet *out) {
    Entity__CueWalkInRandomLane(self, out);
    self->methods->updateScale(self, 1, sScaleSix);
}

/* Row 109: holds half size and creeps forward 10 a tick. */
void Entity__CueHalfSizeCreep(Entity *self, SoundCueSet *out) {
    self->methods->updateScale(self, 1, sScaleHalf);
    self->methods->moveLocalZ(self, -10, 0);
}

/* Row 110: stands still at two fifths of its size; when the dreamer comes within 2048 it turns
 * about (4 degrees a tick for 45 ticks), and 500 ticks later it can turn again. */
void Entity__CueTurnAroundWhenApproached(Entity *self, SoundCueSet *out) {
    enum { TURNING_ABOUT = 10 };

    self->methods->updateScale(self, 1, sScaleTwoFifths);
    self->methods->stopTod(self);
    if (self->state == 0) {
        if (self->methods->distanceToPeer(self, self->peer) < 2048) {
            self->state = TURNING_ABOUT;
            self->moodTimer = 0;
        }
    }
    if (self->state == TURNING_ABOUT) {
        if (self->moodTimer < 45) {
            self->methods->updateRotation(self, 0, sRotationYawPlus4);
        }
        if (self->moodTimer >= 501) {
            self->state = 0;
        }
    }
}

/* Row 111: walks forward 60 a tick with three slow turns and deactivates at moodTimer 2180
 * (Entity__StepYawInWindowsThenDeactivate). In a pink dream it instead goes still and silent at
 * 2160, sounds program 5 just before 2560 and resumes then, rises from 2801 and deactivates at
 * 4000. */
void Entity__CueWanderPauseOnPink(Entity *self, SoundCueSet *out) {
    enum { PINK_PAUSE = 11 };

    if (self->moodTimer == 0) {
        if (((DreamSys *)self->peer)->methods->getDreamColor((DreamSys *)self->peer) == DREAM_COLOR_PINK) {
            self->state = PINK_PAUSE;
        }
    }
    if (self->state != 0 && self->moodTimer >= 2160) {
        /* MATCHING: the repeated `>= 2160` test stays; it is the second range test */
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

/* Row 113: runs row 51's handler, Entity__CueWalkWithTurnsMaybeGiant, unchanged. */
void Entity__CueWalkWithTurnsMaybeGiantRow113(Entity *self, SoundCueSet *out) {
    Entity__CueWalkWithTurnsMaybeGiant(self, out);
}

/* Row 114: twitches its heading: on two ticks in three, turns 9 degrees one way (two times in
 * three) or the other. */
void Entity__CueTwitchYaw(Entity *self, SoundCueSet *out) {
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

/* Row 117: holds six times its size, as rows 118 and 119 do. */
void Entity__CueSixfoldSizeRow117(Entity *self, SoundCueSet *out) {
    self->methods->updateScale(self, 1, sScaleSix);
}

/* Row 118: holds six times its size, as rows 117 and 119 do. */
void Entity__CueSixfoldSizeRow118(Entity *self, SoundCueSet *out) {
    self->methods->updateScale(self, 1, sScaleSix);
}

/* Row 121: holds a quarter of its size. */
void Entity__CueQuarterSize(Entity *self, SoundCueSet *out) {
    self->methods->updateScale(self, 1, sScaleQuarter);
}

/* Row 123's handler, and row 126's (its data words differ). It holds its animation still and backs
 * away 100 a tick, and ends its cue at moodTimer 1000; one time in five, from 301 it also drives
 * the dreamer, sending it the pad events to walk forward and run. */
void Entity__CueBackAwayMaybeDriveDreamer(Entity *self, SoundCueSet *out) {
    enum { DRIVE_DREAMER = 11 };

    if (self->moodTimer == 0) {
        if (rand() % 5 == 0) {
            self->state = DRIVE_DREAMER;
        }
    }
    self->methods->stopTod(self);
    self->methods->moveLocalZ(self, 100, 0);
    if (self->moodTimer == 1000) {
        self->methods->stopSoundCue(self);
        self->state = ENTITY_STATE_DONE;
    }
    if (self->state == DRIVE_DREAMER) {
        if (self->moodTimer >= 301) {
            /* Up held walks forward; cross held with it runs. */
            ((DreamSys *)self->peer)->methods->onPadEvent((DreamSys *)self->peer, 0, PAD_EVENT_HELD + PAD_BUTTON_LUP);
            ((DreamSys *)self->peer)->methods->onPadEvent((DreamSys *)self->peer, 0, PAD_EVENT_HELD + PAD_BUTTON_RDOWN);
        }
    }
}

/* Row 125: holds two fifths of its size and creeps forward 10 a tick with program 10 every half
 * animation cycle; one time in four it deactivates at once, and it always does at moodTimer 3600.
 */
void Entity__CueSmallCreepMaybeVanish(Entity *self, SoundCueSet *out) {
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

/* Row 128: holds a thirty-second of its size, faces the dreamer and walks at it 30 a tick,
 * reporting the move. */
void Entity__CueTinyWalkTowardDreamer(Entity *self, SoundCueSet *out) {
    SceneNode__FaceTarget((SceneNode *)self, (SceneNode *)self->peer, 1, 0, 0);
    self->methods->updateScale(self, 1, sScaleThirtySecond);
    self->methods->moveLocalZ(self, -30, (void *)1);
}

/* Row 129: holds its animation still; from moodTimer 201 faces the dreamer, and half the time runs
 * at it 512 a tick, climbing what it meets. */
void Entity__CueStillThenFaceOrRunAtDreamer(Entity *self, SoundCueSet *out) {
    enum { RUN_AT_DREAMER = 10, FACE_ONLY = 11 };

    if (self->moodTimer == 0) {
        self->state = rand() % 2 + RUN_AT_DREAMER;
    }
    self->methods->stopTod(self);
    if (self->moodTimer >= 201) {
        SceneNode__FaceTarget((SceneNode *)self, (SceneNode *)self->peer, 1, 0, 0);
        if (self->state == RUN_AT_DREAMER) {
            self->methods->moveLocalZOrFindLink(self, -512, 0);
        }
    }
}

/* The mood table: row moodIndex is everything per-entity that is data, not
 * code (struct EntityMoodRow, include/entity.h, has each column's meaning).
 * act is an EntityActivateKind, dea an EntityDeactivateKind (10 and up: the
 * tick count / 15), lnk a stage (ENTITY_LINK_STAGE_END_DREAM is 127), vid a
 * video plus 1. A row with no handler has no sound cue script. */
/* clang-format off */
EntityMoodRow sEntityMoodTable[ENTITY_MOOD_ROW_COUNT] = {
    /*            unread   unlk  act  dea   aR   pR   lnk  vid  tol  thr  cue  handler */
    /*   0 */ {{  0,   2},   20,   0,   0,   0,   2,  -13,   4,   1,  10,   0, Entity__CuePaceOrLiftOffOnPink},
    /*   1 */ {{ -2,   5},   -1,   0,   0,   0,   1,   -2,   0,  -6,  10,   1, Entity__CueHoldDreamerChargeThenLink},
    /*   2 */ {{  0,   0}, -100,   0,   0,   0,   1,   -5,   1,   1,   0,   0, NULL},
    /*   3 */ {{  0,  -3}, -100,   0,   0,   0,   1,   -6,   1,   1,   0,   0, NULL},
    /*   4 */ {{  0,  -7}, -100,   0,   0,   0,   1,  -13,   1,   1,   0,   0, NULL},
    /*   5 */ {{ -4,   4},  -50,   0,   0,   0,   1,  -14,   1,  -2,  10,  -1, Entity__CueTone23Once},
    /*   6 */ {{  0,   0},  100,   0,   0,   0,   0,    0,   0,   3,   0,  -1, NULL},
    /*   7 */ {{ -5,   0},   20,   3, 180,   5,   5,    0,   0,   3,  30,   5, Entity__CueApproachRiseThenSpiralAway},
    /*   8 */ {{  3,   3},  100,   3, 180,   7,   7,    0,   0,   5,   0,   7, Entity__CueDoubleSizeAndRise},
    /*   9 */ {{ -3,  -1},   20,   1, 180,  12,   6,   -3,   3,   3,  30,  12, Entity__CueWalkToTodBeat},
    /*  10 */ {{ -7,   3},   50,   0, 180,   0,  10,    0,   0,   3,  60,   0, Entity__CueChordThenWalk},
    /*  11 */ {{  3,   0},    9,   0, 255,   0,   5,    8,   4,   3,  20,   0, Entity__CueWanderThenChaseIfLinkPressed},
    /*  12 */ {{ -3,  -5},    5,   0,   0,   0,   3,    0,   0,   2,   0,   0, Entity__CueStretchStageNearDreamerThenEndDream},
    /*  13 */ {{ -5,   5},  100,   0,   0,   0,   5,    0,   4,   3,  30,   5, Entity__CueVideoAfter36TodLoops},
    /*  14 */ {{  0,   1}, -100,   0, 120,   0,   5,    0,   0,   3,  20,   5, Entity__CueCreepWithTone13},
    /*  15 */ {{ -2,  -1},  -20,   0,   0,   0,   4,    0,   0,   3,  30,  -5, Entity__CueTone15Once},
    /*  16 */ {{ -3,   2},   50,   1,  24,   3,   3,    0,   0,   1,  30,   3, Entity__CueWalkAndTurnOrSpinFlickering},
    /*  17 */ {{  1,   0},  100,   0,   0,   0,   5,    0,   0,   3,   0,   0, Entity__CueHalfSize},
    /*  18 */ {{  0,   0}, -100,   0,   0,   0,   2,    0,   0,   1,   0,   0, NULL},
    /*  19 */ {{  2,   3},  -50,   1,  30,   7,   5,   -2,   2,   3,  30,   7, Entity__CueRunWithTone17},
    /*  20 */ {{  0,  -4},    8,   1, 180,  12,   5,   -2,   1,   3,  30,  12, Entity__CueWalkMaybeTall},
    /*  21 */ {{ -1,  -2},   20,   0, 255,   0,   6,   -4,   4,   3,  30,   0, Entity__CueWalkToTodBeatTwoTones},
    /*  22 */ {{  1,   4},  -20,   0,   0,   0,   2,   -5,   2,   1,   0,   1, Entity__CueFaceDreamer},
    /*  23 */ {{  5,   1},  -20,   1, 255,   8,   1,    0,   0,   1,   0,   8, Entity__CueDropInRushDreamerThenDriftUp},
    /*  24 */ {{  2,  -7},   20,   0,   0,   0,   5,    0,   0,   3,  30,   0, Entity__CueDoubleSizeCircleFast},
    /*  25 */ {{ -5,  -1},   50,   0, 180,   0,   3,    0,   0,   3,  30,   3, Entity__CueRiseFasterThenPitchUp},
    /*  26 */ {{ -3,   1},    7,   1,  60,  10,   7,    0,   0,   3,  30,  10, Entity__CueRunAndLunge},
    /*  27 */ {{  8,  -1}, -100,   1,  60,   8,   8,    0,   0,   5,  30,   8, Entity__CueWalkDipThenClimb},
    /*  28 */ {{  7,   1},   20,   0,   0,   0,  -2,  -11,   1,   1,   0,   0, NULL},
    /*  29 */ {{ -8,   1},  100,   0,   0,   0,  12,    0,   0,   5,   0,   0, Entity__CueTripleAloftOnWhiteMaybeTurn},
    /*  30 */ {{  0,  -1},  -50,   0, 180,   0,   5,    0,   0,   3,   0,   3, Entity__CueHoverOverDreamerOnBlueElseRise},
    /*  31 */ {{  2,  -5}, -100,   1, 120,   1,   1,   -3,   0,   1,  30,   1, Entity__CueHoldDreamerThenLinkAfterTod},
    /*  32 */ {{  0,   4},   50,   1,   0,  15,  15,    0,   0,   5,   0,  15, Entity__CueWalk},
    /*  33 */ {{  2,   8},    8,   1, 180,  12,   2,   -8,   1,   2,  30,  12, Entity__CueWalkShrinkWhenReached},
    /*  34 */ {{ -8,   4},  -50,   0,  60,   3,   3,  -14,   2,   1,   0,   3, Entity__CueWalkZigzagThenDashAway},
    /*  35 */ {{  6,   0},  -20,   0,   0,   0,   2,    0,   0,   1,   0,   0, Entity__CueWobble},
    /*  36 */ {{  0,   7},    7,   1,   2,  17,  12,    0,   0,   8,   0,  17, Entity__CueGrowThenBackAwayFromDreamer},
    /*  37 */ {{ -2,   6},  100,   1, 120,   6,   6,    0,   0,   5,   0,   6, Entity__CueRise},
    /*  38 */ {{ -5,   2}, -100,   1, 120,  10,  10,    0,   0,   3,  30,  10, Entity__CueTone1Slow},
    /*  39 */ {{ -1,  -1},   20,   1, 120,  20,  20,   -5,   1,   3,  20,  20, Entity__CueStutterTodMaybeTall},
    /*  40 */ {{  3,   3}, -100,   0, 180,   0,   4,  -14,   2,   3,  30,   0, Entity__CuePatrolTurning},
    /*  41 */ {{ -9,   5},  -50,   1,   0,  15,  15,  -10,   8,   3,  50,  15, Entity__CueRunOffOrStopAndJitterDepth},
    /*  42 */ {{  2,   1},    1,   0, 120,   0,   4,    0,   0,   4,  30,   4, Entity__CueSlideInThenAnimateOnce},
    /*  43 */ {{ -3,   0},   20,   1, 180,   6,   6,   -4,   2,   2,  10,   6, Entity__CueShuffleSideways},
    /*  44 */ {{  0,  -2},  -20,   1, 180,   6,   6,   -7,   2,   2,  10,   6, Entity__CueStepTurnThenShuffle},
    /*  45 */ {{  5,   3},  -20,   1,   0,   8,   6,    0,   0,   3,  10,   8, Entity__CueIdle},
    /*  46 */ {{  3,  -1},  -20,   1,   0,   2,   2,   -6,   2,   1,  20,   2, Entity__CueWatchDreamerMaybeGiant},
    /*  47 */ {{  9,   9},  -50,   0,   0,   0,   1,    0,   2,   1,  30,   0, Entity__CueLiftDreamerIfLinkPressedElseVideo},
    /*  48 */ {{ -4,  -4},   50,   1,  90,   1,   1,   -9,   1,   1,  30,   1, Entity__CueBobAndWalk},
    /*  49 */ {{ -2,  -6}, -100,   1,  30,   5,   1,   -3,   0,   1,  30,   5, Entity__CueRunCarryDreamerIfLinkPressed},
    /*  50 */ {{  9,   0},   20,   1,   0,   6,   6,    0,   0,   1,   0,   6, Entity__CueChordFiveTodLoopsThenLeave},
    /*  51 */ {{  1,  -1},  -50,   1,  30,   6,   6,   -5,   1,   3,  20,   6, Entity__CueWalkWithTurnsMaybeGiant},
    /*  52 */ {{ -3,   1},  -50,   1,  30,   8,   8,   -5,   1,   3,  30,   8, Entity__CueRunTurnBackThenVanish},
    /*  53 */ {{  0,   0},   50,   0,   0,   0,   4,    0,   0,   2,  30,   0, NULL},
    /*  54 */ {{  0,   0},  -50,   0,   0,   0,   4,    0,   0,   2,  30,   0, NULL},
    /*  55 */ {{  7,   0},  100,   1,   0,  15,   8,  -13,   2,   2,  20,  15, Entity__CueTone19RhythmMaybeTall},
    /*  56 */ {{  0,  -4}, -100,   0,   0,   0,   5,   -6,   2,   3,   0,   0, Entity__CueLiftOnce},
    /*  57 */ {{ -9,  -3},  -50,   1,   0,  15,  15,    0,   3,   3,  30,  15, Entity__CueSurfaceRunningOrFadeOutNearDreamer},
    /*  58 */ {{ -3,  -6},  -20,   1,   0,   1,   1,    0,   0,   1,  30,   1, Entity__CueLevitateDreamerOnYellowOrCollapse},
    /*  59 */ {{ -3,   1},  -50,   1, 180,  15,   8,    0,   0,   3,  30,  15, Entity__CueWalkMaybeSunkMaybeStretchStage},
    /*  60 */ {{  0,  -9}, -100,   0,   0,   0,   5,    0,   0,   3,   0,   5, NULL},
    /*  61 */ {{  5,  -1},   20,   1,   0,  10,   1,   -6,   3,   3,  30,  10, Entity__CueTone18AtFrame30},
    /*  62 */ {{ -4,  -5},   -1,   1,  60,   2,   2,    0,   5,   1,  10,   2, Entity__CueCreepUpAndHoldDreamer},
    /*  63 */ {{  0,   8},   20,   2,   1,   1,   3,    0,   0,   2,   0,   0, NULL},
    /*  64 */ {{  2,   1},   50,   0,   0,   0,  12,    0,   0,   5,  40,   0, Entity__CueCreepWithTone5Bursts},
    /*  65 */ {{  1,   0},  -20,   0, 255,   0,   3,   -3,   2,   1,   0,   0, Entity__CueMaybeHalfSizeWalkAloft},
    /*  66 */ {{ -4,  -1},   20,   0,   0,   0,   3,   -8,   1,   1,  20,   0, Entity__CueTone13Every30},
    /*  67 */ {{  0,   0},  -50,   0, 220,   0,   3,    0,   0,   1,   0,   0, Entity__CueMaybeDropAndRunAt500},
    /*  68 */ {{  0,  -1},   50,   1, 180,   5,   5,   12,   0,   1,  30,   3, Entity__CueChaseDreamerTumbleIfLinkPressed},
    /*  69 */ {{  0,   0},   -4,   0,   0,   0,   3,    0,   0,   3, 100,   3, Entity__CueLayeredTones},
    /*  70 */ {{  2,   2},   50,   0, 180,   0,  10,    0,   0,   6,  30,   0, Entity__CueRunOutAndBack},
    /*  71 */ {{  9,   0},  100,   1, 180,   6,   3,   -3,   1,   1,  10,   6, Entity__CueWalkInRandomLane},
    /*  72 */ {{  0,   0},    0,   0,   0,   0,   0,    0,   0,   3,  30,   0, NULL},
    /*  73 */ {{  4,   0},    9,   0,   0,   0,   1,    0,   0,   1,  30,  -1, Entity__CueTurnAndHoldDreamer},
    /*  74 */ {{  0,   1},  100,   0,   0,   0,   1,   -4,   0,   1,  30,   1, Entity__CueBlueSkyPushDreamer},
    /*  75 */ {{  0,   0},  100,   0,   0,   0,   3,   13,   0,   1,  10,   3, Entity__CueFaceDreamerThenLinkAfterTod},
    /*  76 */ {{  6,   2},  -20,   1,  60,  17,   3,   -5,   6,   2,   0,  17, Entity__CueSpinUntilReachedThenShrink},
    /*  77 */ {{  9,   0},  100,   0,  60,   0,   1,   -5,   1,  -2,  10,   1, Entity__CueWalkTurningRoute},
    /*  78 */ {{ -2,   3},   20,   0,   0,   0,   1,    0,   0,   1,  10,   0, Entity__CueStepForwardAndBack},
    /*  79 */ {{ -6,   7},   -6,   0,   0,   0,   2,    0,   1,   1,  20,   0, Entity__CueSpinVideoWhenReached},
    /*  80 */ {{  5,   5}, -100,   0, 120,   0,   2,   -3,   4,   1,  10,   1, Entity__CueAnimateThenShootUp},
    /*  81 */ {{ -3,   3},   50,   0, 180,   0,   1,  127,   0,   1,  30,   0, Entity__CueWalkThenChaseOrHoldDreamer},
    /*  82 */ {{  4,  -5},  -20,   0,   0,   0,   3,   -5,   0,   1,   0,   1, Entity__CueJumpAheadHoldDreamerOrStand},
    /*  83 */ {{  3,  -1},  -50,   1,   0,   1,   1,    0,   0,   3,  10,   1, Entity__CueTone12ForOneTod},
    /*  84 */ {{ -5,   5},  -50,   1, 120,   3,   3,    0,   3,   3,  30,   3, Entity__CueToneSequenceOverTod},
    /*  85 */ {{  3,   0},   20,   1,   0,   8,   1,  -10,   3,   1,   0,   1, Entity__CueFadeAndTurnDreamerThenLink},
    /*  86 */ {{  0,  -8}, -100,   1,   0,  10,   3,    0,   0,   1,   0,   3, Entity__CuePauseThenAnimateOnce},
    /*  87 */ {{  3,   0},  -50,   0,   0,   0,   3,    8,   0,   1,  20,   0, Entity__CueTone18OnTodLoop},
    /*  88 */ {{  0,   3},  -50,   0,   0,   0,   3,   11,   0,   1,  20,   0, Entity__CueTone18MidTodLoop},
    /*  89 */ {{  0,  -7},    0,   0,   0,   0,   2,    0,   5,  -4,  10,   1, Entity__CueAnimateOnceMaybeVideo},
    /*  90 */ {{  0,   5},   -1,   0,   0,   0,   1,   -9,   1,  -2,  10,  -1, Entity__CueRandomHighToneOnce},
    /*  91 */ {{  0,   1}, -100,   1,   0,   1,   1,   -6,   0,  -4,  10,   1, Entity__CueFadeSkipTodThenLink},
    /*  92 */ {{ -3,  -4},   -1,   0,   0,   0,   1,   -6,   2,  -8,  10,   0, Entity__CueAwaitReachThenLinkAfterTod},
    /*  93 */ {{  2,   8},   50,   0,  60,   0,   3,    0,   0,   1,  20,   3, Entity__CueAnimateThenRun},
    /*  94 */ {{  0,   5},  100,   0,  40,   8,   8,  -11,   3,   3,  30,   0, Entity__CueAnimateThenRunMaybeGiantOrTurn},
    /*  95 */ {{  4,   2}, -100,   1,  20,   3,   1,    0,   0,   2,  10,   1, Entity__CueSilentAnimateThenRun},
    /*  96 */ {{  2,   0}, -100,   0,   0,   0,   2,  -14,   2,   1,  20,   0, Entity__CueRandomTodEachCycle},
    /*  97 */ {{  0,   0},    0,   0,   0,   0,   0,    0,   0,   3,  30,   0, NULL},
    /*  98 */ {{-10, -10},   20,   0, 180,   0,   1,    0,   0,   1,   0,   0, Entity__CueWalkFadeAndResetFlashbacks},
    /*  99 */ {{ 10,  10},  -20,   0,   0,   0,  -2,    0,   0,   1,  30,  -1, NULL},
    /* 100 */ {{-10, -10},  100,   0,   0,   0,   0,    0,   0,   3,  30,   0, NULL},
    /* 101 */ {{  9,   9},  100,   1,   0,   0,   0,    0,   0,   3,  30,   0, NULL},
    /* 102 */ {{ -5,   0},   20,   3, 180,   5,   1,  -13,   0,   3,  30,   5, Entity__CueCircleThenRegrowFacingDreamer},
    /* 103 */ {{ -6,   7},  100,   1, 180,   5,   5,  -10,   0,   2,   0,   5, Entity__CueWalkMaybeSpiralDownToLink},
    /* 104 */ {{  2,   2},   -4,   1,   0,   2,   2,    0,   0,   1,   0,   2, Entity__CueQuarterSizeRiseBriefly},
    /* 105 */ {{  0,  -3}, -100,   0,  60,   0,   2,    0,   0,   1,   0,   0, Entity__CueFlicker},
    /* 106 */ {{  0,   5},   -1,   0,  60,   0,   1,    0,   0,  -2,  10,   1, Entity__CueSwayOrStandThin},
    /* 107 */ {{  0,   5},   -1,   0,  60,   0,   1,   -2,   1,  -2,  10,  -1, Entity__CueAnimateThenRun},
    /* 108 */ {{  9,   0},  100,   1, 180,   6,   3,   -3,   1,   1,  10,   6, Entity__CueWalkInRandomLaneGiant},
    /* 109 */ {{  3,   3},  100,   0, 180,   0,   7,   -2,   4,   5,   0,   0, Entity__CueHalfSizeCreep},
    /* 110 */ {{  3,   0},    9,   0, 120,   0,   5,  -13,   1,   3,   0,   0, Entity__CueTurnAroundWhenApproached},
    /* 111 */ {{ -9,   5},  -50,   1,   0,  15,  15,   -4,   8,   3,  50,  15, Entity__CueWanderPauseOnPink},
    /* 112 */ {{  8,   8},  100,   0, 180,   0,  15,   -6,   2,   3,  50,   0, NULL},
    /* 113 */ {{  1,  -1},  -50,   1,  30,   6,   6,   -5,   1,   3,  20,   6, Entity__CueWalkWithTurnsMaybeGiantRow113},
    /* 114 */ {{  1,   1},  -80,   1,  90,   6,   6,   -5,   1,   3,  20,   6, Entity__CueTwitchYaw},
    /* 115 */ {{ -3,  -6},  -20,   1,   0,   1,   1,   -6,   1,   1,  30,   1, Entity__CueConfrontDreamerThenLinkOnTouch},
    /* 116 */ {{ -3,  -6},  -20,   1,   0,   1,   1,   -6,   3,   1,  30,   1, NULL},
    /* 117 */ {{ -4,  -5},   -1,   1,  60,   8,   8,  -11,   8,   3,  10,   8, Entity__CueSixfoldSizeRow117},
    /* 118 */ {{  6,   0},  -20,   0,   0,   0,   2,   -3,   8,   1,   0,   0, Entity__CueSixfoldSizeRow118},
    /* 119 */ {{  7,   1},   20,   0,   0,   0, -10,    0,   0,   5,   0,   0, Entity__CueSixfoldSizeRow119},
    /* 120 */ {{ -6,   8},   20,   1, 120,   8,   1,    8,   0,   1,  30,   8, Entity__CueWalkThenChaseOrHoldDreamer},
    /* 121 */ {{  1,   1},   20,   1,  90,   8,   3,   -4,   1,   3,  30,   8, Entity__CueQuarterSize},
    /* 122 */ {{  0,  -1},  -50,   1,  90,   3,   5,    0,   0,   3,   0,   3, Entity__CueHoverOverDreamerOnBlueElseRise},
    /* 123 */ {{ -7,   6},    8,   0, 140,   0,   3,    0,   0,   3,   0,   3, Entity__CueBackAwayMaybeDriveDreamer},
    /* 124 */ {{  4,   4}, -100,   0, 120,   0,   5,    0,   0,   3,  20,   5, NULL},
    /* 125 */ {{ -5,   6},    9,   1, 255,   5,   5,  -14,   1,   3,  20,   5, Entity__CueSmallCreepMaybeVanish},
    /* 126 */ {{ -5,   4},    9,   0, 140,   0,   5,   -4,   3,   3,   0,   0, Entity__CueBackAwayMaybeDriveDreamer},
    /* 127 */ {{  5,   1},  -20,   0, 120,   0,   3,   -5,   1,   1,   0,   0, NULL},
    /* 128 */ {{ -4,  -4},   50,   1,   0,   1,   1,  -14,   1,   1,   0,   1, Entity__CueTinyWalkTowardDreamer},
    /* 129 */ {{  2,  -3},  -20,   0, 120,   0,   2,  -14,   1,   1,   0,   1, Entity__CueStillThenFaceOrRunAtDreamer},
};
/* clang-format on */
