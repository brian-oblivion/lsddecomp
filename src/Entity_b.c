/* Entity_b -- second slice of the Entity class (include/Entity.h, table
 * gEntityMethods; `tools/classtable.py gEntityMethods`).
 *
 *  - gEntityMethods' last three slots, run every tick by Entity__Update:
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
#include "StageMap.h"

s32 Entity__UpdateTargetProximity(Entity *this) {
    EntityMoodRow *row;
    s32 *pos;
    s32 dist;

    row = &gEntityMoodTable[this->moodIndex];
    if (this->active != 0) {
        if (this->targetReached == 0) {
            pos = &this->coord2->tx;
            dist = row->proximityRange;
            if (dist < 0) {
                dist = ~dist + 1; /* MATCHING: -dist compiles differently */
            }
            if (Entity__IsNearTarget(this, pos, dist, row->unk9) != 0) {
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
    s32 *pos;
    s32 dist;

    if (this->active != 0 && this->soundCueActive == 0 && this->state != ENTITY_STATE_DONE) {
        row = &gEntityMoodTable[this->moodIndex];
        if (row->cueRange != 0) {
            pos = &this->coord2->tx;
            dist = row->cueRange;
            if (dist < 0) {
                dist = ~dist + 1; /* MATCHING: -dist compiles differently */
            }
            if (Entity__IsNearTarget(this, pos, dist, row->unk9) != 0) {
                this->methods->startSoundCue(this);
            }
        }
    }
    return this->soundCueActive;
}

/* arg1 is unused here; the canonical declaration in include/Entity.h has it
 * and its one caller, Entity__UpdateDeactivationState, passes 0. Do not drop
 * it -- `conflicting types`. */
void Entity__NotifyIfTargetInRange(Entity *this, s32 unused) {
    if (gEntityLinkStageTable[this->moodIndex * 16] < 0 &&
        gEntityEventVideoTable[this->moodIndex * 16] != 0 &&
        Entity__IsTargetInRange(this, gEntityEventVideoTable[this->moodIndex * 16] << 9)) {
        this->methods->notifyParents(this, ENTITY_EFFECT_LINK_STAGE);
    }
}

s32 Entity__IsTargetInRange(Entity *this, s32 range) {
    TodActor *other;
    s32 oy, ty;

    other = this->peer;
    oy = other->coord2->ty;
    ty = this->coord2->ty;
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
    s32 *pos;
    s32 dist;

    if (this->active != 0 && this->soundCueActive != 0) {
        row = &gEntityMoodTable[this->moodIndex];
        dist = row->cueRange;
        if (dist < 0) {
            dist = ~dist + 1; /* MATCHING: -dist compiles differently */
            pos = &this->coord2->tx;
            if (Entity__IsNearTarget(this, pos, dist, row->unk9) == 0) {
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
            this->moodTimer = -1;
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
    } else if (this->moodTimer >= 56 || Entity__IsNearTarget(this, &this->coord2->tx, 1, 1) != 0) {
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
    y = this->coord2->ty;
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
        out->tick = -1;
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
