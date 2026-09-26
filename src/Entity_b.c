/* Entity_b -- second slice of the Entity class (include/Entity.h, table
 * ENTITY_METHODS; `tools/classtable.py ENTITY_METHODS`).
 *
 *  - ENTITY_METHODS' last three slots, run every tick by Entity__Update:
 *    Entity__UpdateTargetProximity (+0x178, raises targetReached via setTargetReached once
 *    the target is within the mood row's proximityRange) and
 *    Entity__UpdateSoundCueStart/Stop (+0x17C/+0x180, start the sound cue
 *    when the target enters the row's cueRange, stop it when it leaves);
 *    two range helpers; the vtable accessor Get_vtable_Entity.
 *  - Entity__MoodCue00..17: gEntityMoodHandlerTable's callbacks, NN = the
 *    row. Entity__StartSoundCue gives the row's callback to InitSoundCueSet,
 *    and ServiceSoundCueSet calls it once per tick as callback(owner, set): it
 *    picks tones for the set's three voices and moves/rotates/scales the
 *    entity on moodTimer thresholds. Which dream object owns each row is
 *    not established.
 */
#include "common.h"
#include "Entity.h"
#include "DreamSys.h"
#include "Class866E8.h"

/* Constant transform triples passed to updateRotation (slot +0x44,
 * func_8001CEB4: three {s16 num, s16 den} ratios in degrees), updateScale
 * (+0x48, func_8001D008: three ratios) and addTranslation (+0xBC,
 * Actor__AddTranslation: three s32 deltas, so Vec3_d294), named by value.
 * Only the address of the rotation and scale ones is taken here, so a byte
 * array is enough; the RotationRatios type in DreamSys.h is their real
 * shape.
 * TRANSLATE_Y_MINUS64's label also holds a second triple, (0, -0x20, 0). */
extern u8 SCALE_HALF[];
extern u8 SCALE_DOUBLE[];
extern Vec3_d294 TRANSLATE_Y_MINUS64[];
extern u8 ROTATION_YAW_PLUS2[];
extern u8 ROTATION_YAW_MINUS90[];
extern u8 ROTATION_YAW_PLUS90[];
extern Vec3_d294 TRANSLATE_Y_PLUS256[];
extern Vec3_d294 TRANSLATE_Y_PLUS64_Z_MINUS64[];
extern u8 ROTATION_YAW_MINUS120[];
extern u8 ROTATION_X50_YMINUS120_Z30[];

s32 Entity__UpdateTargetProximity(Entity *this) {
    EntityMoodRow *row;
    s32 *xptr;
    s32 dist;

    row = &gEntityMoodTable[this->moodIndex];
    if (this->active != 0) {
        if (this->targetReached == 0) {
            xptr = &this->coord2->tx;
            dist = row->proximityRange;
            if (dist < 0) {
                dist = ~dist + 1;
            }
            if (Entity__IsNearTarget(this, xptr, dist, row->unk9) != 0) {
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
    s32 *xptr;
    s32 dist;

    if (this->active != 0 && this->soundCueActive == 0 && this->state != 1) {
        row = &gEntityMoodTable[this->moodIndex];
        if (row->cueRange != 0) {
            xptr = &this->coord2->tx;
            dist = row->cueRange;
            if (dist < 0) {
                dist = ~dist + 1;
            }
            if (Entity__IsNearTarget(this, xptr, dist, row->unk9) != 0) {
                this->methods->startSoundCue(this);
            }
        }
    }
    return this->soundCueActive;
}

/* arg1 is unused here; the canonical declaration in include/Entity.h has it
 * and its one caller, Entity__UpdateDeactivationState, passes 0. Do not drop
 * it -- `conflicting types`. */
void Entity__NotifyIfTargetInRange(Entity *this, s32 arg1) {
    if (gEntityLinkStageTable[this->moodIndex * 0x10] < 0 &&
        gEntityEventVideoTable[this->moodIndex * 0x10] != 0 &&
        Entity__IsTargetInRange(this, gEntityEventVideoTable[this->moodIndex * 0x10] << 9)) {
        this->methods->notifyParents(this, 0xA);
    }
}

s32 Entity__IsTargetInRange(Entity *this, s32 range) {
    Class65650 *other;
    s32 oy, ty;

    other = this->peer;
    oy = other->coord2->ty;
    ty = this->coord2->ty;
    if (oy + 0x200 < ty) {
        goto fail;
    }
    if (ty < oy - 0x200) {
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
    s32 *xptr;
    s32 dist;

    if (this->active != 0 && this->soundCueActive != 0) {
        row = &gEntityMoodTable[this->moodIndex];
        dist = row->cueRange;
        if (dist < 0) {
            dist = ~dist + 1;
            xptr = &this->coord2->tx;
            if (Entity__IsNearTarget(this, xptr, dist, row->unk9) == 0) {
                this->methods->stopSoundCue(this);
            }
        }
    }
    return this->soundCueActive;
}

EntityMethods *Get_vtable_Entity(void) {
    return &ENTITY_METHODS;
}

void Entity__MoodCue00(Entity *this, EntityMoodHandlerArg *out) {
    if (out->unk4 == 0) {
        if (((DreamSys *)this->peer)->methods->getDreamColor((DreamSys *)this->peer) == 5) {
            this->state = 0x64;
        }
    }
    out->unk10 = this->methods->getProximityRatio(this);
    if (this->state == 0) {
        if (out->unk4 % 10 == 0) {
            out->unk1C = 5;
            out->unk20 = -2;
        }
        if (this->moodTimer == 0x960) {
            this->moodTimer = -1;
        } else if (this->moodTimer < 0x4B0) {
            this->methods->moveLocalZ(this, 0x32, 0);
        } else {
            this->methods->moveLocalZ(this, -0x32, 0);
        }
    } else if (this->moodTimer < 0xFA) {
        if (out->unk4 % 10 == 0) {
            out->unk1C = 5;
            out->unk20 = -2;
        }
        if (this->moodTimer < 0x64) {
            this->methods->moveLocalZ(this, 0x32, 0);
        } else if (this->moodTimer < 0xFA) {
            this->methods->addTranslation(this, TRANSLATE_Y_PLUS64_Z_MINUS64);
        }
    } else if (this->moodTimer == 0xFA) {
        this->methods->stopTod(this);
        out->unk1C = -2;
    } else if (this->moodTimer >= 0x105 && this->moodTimer < 0x238) {
        this->methods->moveLocalZ(this, -0x32, 0);
        this->methods->updateRotation(this, 1, ROTATION_YAW_MINUS120);
    } else if (this->moodTimer >= 0x239) {
        this->methods->updateRotation(this, 1, ROTATION_X50_YMINUS120_Z30);
    }
}

void Entity__MoodCue01(Entity *this, EntityMoodHandlerArg *out) {
    out->unk10 = 0;
    if (out->unk4 == 0) {
        out->unk1C = 0x14;
        out->unk30 = 0x14;
        out->unk44 = 0x14;
        ((DreamSys *)this->peer)->methods->clearTickCallbacks((DreamSys *)this->peer, 1);
    }
    SceneNode__FaceTarget((SceneNode *)this, (SceneNode *)this->peer, 1, 0, 0);
    this->methods->moveLocalZ(this, -0x5A, 0);
    if (this->moodTimer == 0x1E) {
        this->methods->notifyParents(this, 0xA);
    }
}

void Entity__MoodCue05(Entity *this, EntityMoodHandlerArg *out) {
    out->unk10 = this->methods->getProximityRatio(this);
    if (out->unk4 == 0) {
        out->unk1C = 0x17;
    }
}

void Entity__MoodCue07(Entity *this, EntityMoodHandlerArg *out) {
    out->unk10 = this->methods->getProximityRatio(this);
    if (this->todFrame == this->todFrameCount / 2) {
        out->unk1C = 0x7;
        out->unk20 = -0x2;
        out->unk30 = 0x3;
        out->unk34 = -0x2;
    }
    if (out->unk4 % 90 < 3) {
        out->unk44 = 0x6;
        out->unk48 = -0x1;
    }
    if (this->moodTimer >= 0x79) {
        this->methods->updateRotation(this, 0, ROTATION_YAW_PLUS2);
        this->methods->moveLocalZ(this, -0x140, 0);
    } else if (this->moodTimer >= 0x38 || Entity__IsNearTarget(this, &this->coord2->tx, 1, 1) != 0) {
        this->methods->addTranslation(this, TRANSLATE_Y_MINUS64);
    } else if (this->moodTimer >= 0xA) {
        SceneNode__FaceTarget((SceneNode *)this, (SceneNode *)this->peer, 1, 0, 0);
        this->methods->moveLocalZ(this, -0x100, 0);
    } else {
        SceneNode__FaceTarget((SceneNode *)this, (SceneNode *)this->peer, 1, 0, 0);
    }
}

void Entity__MoodCue08(Entity *this) {
    this->methods->updateScale(this, 1, SCALE_DOUBLE);
    this->methods->addTranslation(this, TRANSLATE_Y_MINUS64);
}

void Entity__MoodCue09(Entity *this, EntityMoodHandlerArg *out) {
    out->unk10 = this->methods->getProximityRatio(this);
    if (out->unk4 % (this->todFrameCount / 2) == 0) {
        out->unk1C = 0xA;
    }
    this->methods->moveLocalZ(this, -0x1E, 0);
}

void Entity__MoodCue10(Entity *this, EntityMoodHandlerArg *out) {
    out->unk10 = 0;
    if (out->unk4 == 0) {
        out->unk1C = 0xB;
        out->unk30 = 0xB;
        out->unk44 = 0xB;
    }
    this->methods->moveLocalZ(this, -0x1E, 0);
}

void Entity__MoodCue11(Entity *this, EntityMoodHandlerArg *out) {
    u8 *row;

    this->lastOffsetValue = -0x14;
    out->unk10 = this->methods->getProximityRatio(this);
    row = 0;
    if (out->unk4 % (this->todFrameCount / 2) == 0) {
        out->unk1C = 0xA;
        out->unk20 = 1;
    }
    if (this->state == 0xB) {
        if (this->moodTimer == 0xA8C) {
            row = ROTATION_YAW_MINUS90;
        }
        if (this->moodTimer == 0xC6C) {
            row = ROTATION_YAW_PLUS90;
        }
        if (this->moodTimer == 0xE10) {
            row = ROTATION_YAW_MINUS90;
        }
        if ((u32)(this->moodTimer - 0xD5D) < 0x78) {
            if (((DreamSys *)this->peer)->methods->getLinkCommandFlag((DreamSys *)this->peer) != 0) {
                this->moodTimer = 0;
                this->state = 0xD;
            }
        }
    } else if (this->state == 0xC) {
        if (this->moodTimer == 0x7BC) {
            row = ROTATION_YAW_MINUS90;
        }
    } else if (this->state == 0xD) {
        this->lastOffsetValue = -0x78;
        SceneNode__FaceTarget((SceneNode *)this, (SceneNode *)this->peer, 1, 0, 0);
        this->methods->updateScale(this, 1, SCALE_HALF);
        if (this->methods->distanceToPeer(this, this->peer) < 0x400) {
            this->methods->notifyParents(this, 0xB);
        }
    }
    if (this->moodTimer == 0x618) {
        if ((rand() & 1) != 0) {
            row = ROTATION_YAW_PLUS90;
            this->state = 0xB;
        } else {
            row = ROTATION_YAW_MINUS90;
            this->state = 0xC;
        }
    }
    if (row != 0) {
        this->methods->updateRotation(this, 0, row);
    }
    this->methods->moveLocalZOrFindLink(this, this->lastOffsetValue, 0);
    if (this->state != 0xC) {
        if (this->linkTarget != 0) {
            this->methods->moveLocalY(this, -0xC8, 0);
        }
    }
}

void Entity__MoodCue12(Entity *this) {
    s32 y;
    s32 result;
    s32 oldFC;

    if (this->moodTimer == 0) {
        if ((rand() & 1) == 0) {
            this->state = 0xB;
        }
    }
    y = this->coord2->ty;
    if (y < 0x7D0) {
        SceneNode__FaceTarget((SceneNode *)this, (SceneNode *)this->peer, 1, 0, 0);
    }
    if (this->state == 0xB) {
        result = this->methods->distanceToPeer(this, this->peer);
        if (result < 0xA00) {
            this->grid->methods->configureRateEntry(this->grid, 1, 1);
            this->moodTimer = 1;
            this->state = 0xC;
        }
    } else if (this->state == 0xC) {
        oldFC = this->moodTimer;
        this->moodTimer = oldFC + 1;
        if (oldFC == 0x12C) {
            this->methods->notifyParents(this, 0xC);
        }
    }
}

void Entity__MoodCue13(Entity *this, EntityMoodHandlerArg *out) {
    out->unk10 = this->methods->getProximityRatio(this);
    if (out->unk4 == 0) {
        out->unk1C = 0xC;
        this->state++;
    } else if (out->unk4 >= this->todFrameCount - 1) {
        out->unk4 = -1;
    }
    if (this->state == 0x24) {
        if (rand() % 3 == 0) {
            this->methods->notifyParents(this, 0xB);
        }
    }
}

void Entity__MoodCue14(Entity *this, EntityMoodHandlerArg *out) {
    out->unk10 = this->methods->getProximityRatio(this);
    if (this->todFrame == 0xA) {
        out->unk1C = 0xD;
    }
    this->methods->moveLocalZ(this, -0xA, 0);
}

void Entity__MoodCue15(Entity *this, EntityMoodHandlerArg *out) {
    if (this->moodTimer == 0) {
        out->unk10 = 0;
        out->unk1C = 0xF;
    }
}

void Entity__MoodCue16(Entity *this) {
    u8 *arg2;
    s32 roll;

    if (this->moodTimer == 0) {
        if ((rand() & 1) != 0) {
            this->state = 0xB;
        }
    }
    if (this->state == 0) {
        if (this->moodTimer < 0x40) {
            this->methods->moveLocalZ(this, -0x5A, 0);
        } else if (this->moodTimer == 0x40) {
            roll = rand() & 1;
            arg2 = ROTATION_YAW_MINUS90;
            if (roll != 0) {
                arg2 = ROTATION_YAW_PLUS90;
            }
            this->methods->updateRotation(this, 0, arg2);
            this->methods->addTranslation(this, TRANSLATE_Y_PLUS256);
        } else {
            this->methods->moveLocalZOrFindLink(this, -0x176, (void *)(rand() % 2));
        }
    } else if (this->state == 0xB) {
        if (this->moodTimer % 5 == 0) {
            this->methods->updateRotation(this, 0, ROTATION_YAW_PLUS90);
        }
        this->methods->moveLocalZ(this, -0x800, 0);
        this->methods->setDisplay(this, (rand() % 7) == 0);
    }
}

void Entity__MoodCue17(Entity *this) {
    this->methods->updateScale(this, 1, SCALE_HALF);
}
