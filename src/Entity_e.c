/* Third 20-function slice of the Entity class's 97-function remainder,
 * following Entity_c and Entity_d (Entity_d's own header comment names the
 * split). Every function in this unit is a `gEntityMoodHandlerTable`
 * callback (Entity.h), named `Entity__MoodCueNN` for the row it occupies --
 * rows 59, 61-62, 64-71, 73-81, confirmed by reading disk/SLPS_015.56
 * directly (base 0x80089EB0 + 0x10*row is the row's own `handler` word,
 * checked against each candidate function's address; row order does not
 * track code address, same finding as Entity_d/round 76). Rows 60, 63 and
 * 72 have a NULL handler word in the table -- those mood indices legitimately
 * dispatch no per-tick cue callback at all, not a gap in this unit's queue.
 *
 * `Entity__MoodCue81` (0x80063144) also occupies row 120 of the same table
 * (identical `handler` word, different data0/data1/data2) -- one function
 * shared by two mood-row configurations, named for its lower row.
 * `Entity__MoodCue71` (0x80062570) was already cross-unit called (Entity_g's
 * `Entity__MoodCue108` forwards its own args straight through) before this round;
 * its `include/Entity.h` extern is updated by this rename.
 *
 * Four rotation/translate data constants named this round, decoded from
 * disk/SLPS_015.56 against the existing ROTATION_YAW_PLUS90/ROTATION_ZMINUS90/
 * TRANSLATE_Y_PLUS256/SCALE_HALF tables (rotation/scale: four s16 {num,den}
 * pairs for X/Y(yaw)/Z/W; translate: three consecutive s32 for X/Y/Z):
 * ROTATION_ZPLUS9, ROTATION_XPLUS90, TRANSLATE_Y_PLUS8, TRANSLATE_Y_MINUS512.
 * `sMoodCue78TransitionDone` (formerly D_8008ACCC) is a one-shot s32 flag
 * local to Entity__MoodCue78's own state machine, referenced nowhere else in
 * `src/`. See each function's match report's `## Naming` section for the
 * per-constant evidence.
 */
#include "common.h"
#include "Entity.h"

void Entity__MoodCue59(Entity *this, EntityMoodHandlerArg *out) {
    if (this->moodTimer == 0 && rand() % 10 == 0) {
        this->moodState = 0xC;
    }
    if (out->unk4 % 10 == 0) {
        out->unk10 = this->methods->getProximityRatio(this);
        out->unk1C = 0xC;
        out->unk20 = -1;
    }
    if (this->moodTimer == 0) {
        if (rand() & 1) {
            this->methods->slotCC(this, 0x800, 0);
        }
    }
    this->methods->slotC4(this, -0x80, 0);
    if (this->moodState == 0xC && this->moodTimer == 0x12C) {
        this->unk4C->methods->slot138(this->unk4C, 1, 1);
    }
}

void Entity__MoodCue61(Entity *this, EntityMoodHandlerArg *out) {
    if (this->todFrame == 0x1E) {
        out->unk1C = 0x12;
        out->unk10 = 0;
        out->unk20 = -1;
    }
}

void Entity__MoodCue62(Entity *this, EntityMoodHandlerArg *out) {
    if (this->todFrame % 30 == 0) {
        out->unk1C = 3;
        out->unk10 = 0;
        out->unk20 = -2;
    }
    if (this->moodTimer >= 0x65) {
        Class6B5CC__FaceTarget((Class6B5CC *)this, (Class6B5CC *)this->target, 1, 0, 0);
    }
    this->methods->slotC4(this, -5, 0);
    if (this->moodTimer == 0x12C && this->methods->distanceToRegion(this, this->target) < 0x1000) {
        this->target->methods->slot130(this->target, 0);
    } else if (this->moodTimer == 0x1F4) {
        this->target->methods->slot134(this->target, 1, 1);
    }
    if (this->moodState == 0) {
        if (this->methods->distanceToRegion(this, this->target) < 0x400) {
            if (rand() & 1) {
                out->unk30 = 6;
                out->unk10 = 0;
                out->unk34 = -1;
                if (rand() & 1) {
                    this->unk4C->methods->slot138(this->unk4C, -1, 0);
                }
                this->moodTimer = 0;
                this->moodState = 0xA;
            } else {
                this->moodState = 0xB;
            }
        }
    }
    if (this->moodState == 0xA && this->moodTimer == 0x46) {
        this->methods->notifyParents(this, (rand() & 1) ? 0xC : 0xB);
    }
}

void Entity__MoodCue64(Entity *this, EntityMoodHandlerArg *out) {
    s32 r = out->unk4 % 300;

    out->unk10 = this->methods->getProximityRatio(this);
    if (r < 0x14) {
        out->unk1C = 5;
        out->unk20 = -2;
    } else if (r == 0x16) {
        out->unk1C = -2;
    }
    this->methods->slotC4(this, -0xA, 0);
}

extern u8 SCALE_HALF[];
extern u8 ROTATION_YAW_PLUS90[];
extern u8 ROTATION_YAW_MINUS90[];

void Entity__MoodCue65(Entity *this, EntityMoodHandlerArg *out) {
    if (this->moodTimer == 0) {
        if (rand() % 3 == 0) {
            this->methods->updateScale(this, 1, SCALE_HALF);
            this->methods->slotCC(this, -0x12C, 0);
            this->methods->updateRotation(this, 1, ROTATION_YAW_PLUS90);
            this->moodState = 0xB;
        }
    }
    if (this->moodState == 0xB) {
        if (this->moodTimer == 0x7D0) {
            this->methods->updateRotation(this, 0, ROTATION_YAW_MINUS90);
        }
        this->methods->slotC4(this, -0x14, 0);
    }
}

void Entity__MoodCue66(Entity *this, EntityMoodHandlerArg *out) {
    out->unk10 = this->methods->getProximityRatio(this);
    if (out->unk4 % 30 == 0) {
        out->unk1C = 0xD;
    }
}

void Entity__MoodCue67(Entity *this, EntityMoodHandlerArg *out) {
    if (this->moodTimer == 0) {
        if (rand() % 3 == 0) {
            this->moodState = 0xB;
        }
    }
    if (this->moodState == 0xB) {
        if (this->moodTimer == 0x1F6) {
            this->methods->slotCC(this, 0x800, 0);
            Class6B5CC__FaceTarget((Class6B5CC *)this, (Class6B5CC *)this->target, 1, 0, 0);
        }
        if (this->moodTimer >= 0x1F5) {
            this->methods->slotC4(this, -0x200, 0);
        }
    }
}

extern u8 ROTATION_ZPLUS9[];
extern u8 TRANSLATE_Y_PLUS8[];

void Entity__MoodCue68(Entity *this, EntityMoodHandlerArg *out) {
    out->unk10 = this->methods->getProximityRatio(this);
    if (out->unk4 == 0) {
        this->unk48 = (rand() & 1) ? -0x176 : -0xC0;
    }
    if (this->moodState == 0) {
        if (out->unk4 % 10 == 0) {
            out->unk1C = 0x1C;
        }
        if (out->unk4 % 20 == 0) {
            out->unk30 = 0x17;
            out->unk34 = -1;
            out->unk44 = 0x17;
            out->unk48 = -1;
        } else if (out->unk4 % 20 == 0xE) {
            out->unk30 = -2;
            out->unk44 = -2;
        }
        if ((this->moodTimer & 1) == 0) {
            if (this->target->methods->slot100(this->target) != 0) {
                this->moodTimer = -1;
                this->moodState = 0xA;
                out->unk1C = 0x12;
            }
        }
        Class6B5CC__FaceTarget((Class6B5CC *)this, (Class6B5CC *)this->target, 1, 0, 0);
        this->methods->slotC4(this, this->unk48, 1);
    } else if (this->moodState == 0xA) {
        if (this->moodTimer < 8) {
            this->methods->updateRotation(this, 0, ROTATION_ZPLUS9);
            this->methods->addVec14(this, TRANSLATE_Y_PLUS8);
        } else {
            u32 r;

            out->unk1C = 0x12;
            out->unk30 = 3;
            this->methods->stopSoundCue(this);
            r = rand() & 1;
            this->moodState = r < 1;
        }
    }
}

void Entity__MoodCue69(Entity *this, EntityMoodHandlerArg *out) {
    out->unk10 = this->methods->getProximityRatio(this);
    if (this->todFrame == this->todFrameCount - 1) {
        out->unk1C = 0x19;
        out->unk20 = -2;
    }
    if (out->unk4 % 4 == 0) {
        out->unk30 = 0x15;
        out->unk34 = -1;
    }
    if (out->unk4 % 200 == 0) {
        out->unk44 = 0xD;
        out->unk48 = 1;
    }
}

/* Data tables reached with a raw pointer by this unit's mood-dispatch
 * handlers -- same convention as Entity_c.c/Entity_d.c's own separate
 * per-unit externs, not shared via the header. */
extern u8 ROTATION_YAW_PLUS180[];

void Entity__MoodCue70(Entity *this, EntityMoodHandlerArg *out) {
    if (this->moodTimer == 0) {
        if (rand() & 1) {
            this->moodState = 0xB;
        }
    }
    if (this->moodTimer == 0x12C) {
        this->methods->updateRotation(this, 0, ROTATION_YAW_PLUS180);
    }
    if (this->moodTimer < 0x258) {
        this->methods->slotC4(this, this->moodState == 0 ? -0x100 : 0x100, 0);
    }
}

void Entity__MoodCue71(Entity *this, EntityMoodHandlerArg *out) {
    s32 r;

    out->unk10 = this->methods->getProximityRatio(this);
    if (out->unk4 == 0) {
        out->unk1C = 0;
        r = rand() % 3;
        this->methods->slotC8(this, r * 51200, 0);
    }
    if (this->moodTimer >= 0x961) {
        Class6B5CC__FaceTarget((Class6B5CC *)this, (Class6B5CC *)this->target, 1, 0, 0);
    }
    this->methods->slotC4(this, -0x1E, 0);
}

extern u8 ROTATION_YAW_PLUS90[];

void Entity__MoodCue73(Entity *this, EntityMoodHandlerArg *out) {
    out->unk10 = 0;
    if (out->unk4 == 0) {
        this->target->methods->slot44(this->target, 1, ROTATION_YAW_PLUS90);
        this->target->methods->slot130(this->target, 1);
        out->unk1C = 0x19;
        out->unk30 = 0x19;
        out->unk44 = 0x19;
    } else if (out->unk4 == 0x14) {
        out->unk30 = 0xD;
    }
    if (this->moodTimer == this->todFrameCount - 1) {
        this->methods->deactivate(this);
    }
}

extern u8 D_8008AC1C[];

void Entity__MoodCue74(Entity *this, EntityMoodHandlerArg *out) {
    if (this->moodTimer == 0) {
        this->target->unk5C->methods->slot64(this->target->unk5C, D_8008AC1C);
        this->moodState = rand() % 3;
        if (this->target->unk14->z < 0x262) {
            this->moodState = 0;
        }
    }
    if (this->moodState != 0) {
        if (this->todFrameCount / 2 < this->moodTimer) {
            this->target->methods->slotC4(this->target, 0x80, 0);
        }
        if (this->moodTimer == this->todFrameCount - 0x1E) {
            this->methods->notifyParents(this, 0xA);
        }
    } else {
        if ((u32)(this->moodTimer - 0x14) < 0x64) {
            this->target->methods->slotC4(this->target, -((this->moodTimer - 0x13) * 0x20), 1);
            if (this->moodTimer == 0x55) {
                this->target->methods->slot134(this->target, 1, 1);
            }
        }
    }
}

void Entity__MoodCue75(Entity *this, EntityMoodHandlerArg *out) {
    if (out->unk4 == 0) {
        out->unk10 = 0;
        out->unk1C = 0x19;
        out->unk30 = 0x19;
        out->unk44 = 0x19;
        Class6B5CC__FaceTarget((Class6B5CC *)this, (Class6B5CC *)this->target, 1, 0, 0);
    }
    if (this->moodTimer == this->todFrameCount) {
        this->methods->stopTod(this);
        this->methods->notifyParents(this, 0xA);
    }
}

extern u8 D_80089DFC[];
extern u8 ROTATION_YAW_PLUS9[];

void Entity__MoodCue76(Entity *this, EntityMoodHandlerArg *out) {
    void (*fn)(Entity *self, s32 arg1, void *arg2);
    void *table;

    if (this->targetReached != 0) {
        this->methods->playTod(this);
        if (this->todFrame == this->todFrameCount - 1) {
            this->methods->stopTod(this);
            fn = (void (*)(Entity *, s32, void *))this->methods->updateScale;
            table = D_80089DFC;
            fn(this, 0, table);
        }
    } else {
        this->methods->stopTod(this);
        fn = this->methods->updateRotation;
        table = ROTATION_YAW_PLUS9;
        fn(this, 0, table);
    }
}

void Entity__MoodCue77(Entity *this, EntityMoodHandlerArg *out) {
    out->unk10 = this->methods->getProximityRatio(this);
    if (out->unk4 % 5 == 0) {
        out->unk1C = 0x11;
        out->unk20 = -2;
    }
    if (this->todIndex == 0) {
        if (this->moodTimer == this->todFrameCount) {
            this->methods->setTod(this, 1);
            if (rand() & 1) {
                this->moodState = 0xB;
            }
        }
        return;
    }
    if (this->moodState == 0) {
        if (this->moodTimer == 0x3C || this->moodTimer == 0xD4 || this->moodTimer == 0x122 || this->moodTimer == 0x140) {
            this->methods->updateRotation(this, 0, ROTATION_YAW_PLUS90);
        }
        if (this->moodTimer == 0x18E) {
            this->methods->updateRotation(this, 0, ROTATION_YAW_MINUS90);
        }
        this->methods->slotD0(this, -0x32, 0);
        return;
    }
    if (this->moodTimer == 0x3C || this->moodTimer == 0x8C) {
        this->methods->updateRotation(this, 0, ROTATION_YAW_PLUS90);
    }
    if (this->moodTimer < 0xAE) {
        this->methods->slotD0(this, -0x32, 0);
    }
    if (this->moodTimer == 0xAE) {
        this->methods->stopSoundCue(this);
        this->moodState = 1;
    }
}

extern u8 ROTATION_XPLUS90[];
extern u8 SCALE_Y2[];
extern u8 D_80089E14[];
extern s32 sMoodCue78TransitionDone;

void Entity__MoodCue78(Entity *this, EntityMoodHandlerArg *out) {
    s32 tmp;
    void *table;

    if (out->unk4 == 0) {
        sMoodCue78TransitionDone = 0;
        tmp = rand() % 3;
        if (tmp == 1) {
            this->moodState = 0xB;
        }
        if (tmp == 2) {
            this->moodState = 0xC;
        }
    }

    out->unk10 = this->methods->getProximityRatio(this);

    if (this->unk90 != 0 && (out->unk4 & 3) == 0) {
        out->unk1C = 0x1C;
    }

    if (this->moodTimer == this->todFrameCount - 1) {
        this->moodTimer = -1;
    } else {
        if (this->moodTimer >= this->todFrameCount / 2 + this->todFrameCount / 4) {
            this->methods->playTod(this);
        } else if (this->moodTimer >= this->todFrameCount / 2) {
            if (this->moodTimer == this->todFrameCount / 2) {
                out->unk1C = 0x10;
            }
            this->methods->slotC4(this, 0x6E, 0);
        } else if (this->moodTimer < this->todFrameCount / 4) {
            /* nothing */
        } else {
            this->methods->stopTod(this);
            this->methods->slotC4(this, -0x6E, 0);
        }
    }

    if (this->moodState == 0xB && out->unk4 == 0x1FE) {
        this->methods->slotCC(this, -0x17C, 0);
        this->methods->updateRotation(this, 0, ROTATION_XPLUS90);
        this->methods->stopSoundCue(this);
        this->moodState = 1;
        sMoodCue78TransitionDone = 1;
    } else if (this->moodState >= 0xC && out->unk4 >= 0x14A && (out->unk4 % 60) == 30) {
        tmp = 0;
        if (rand() & 1) {
            table = SCALE_Y2;
            tmp = (this->moodState == 0xC) ? 0x190 : 0;
            this->moodState = 0xD;
        } else {
            table = D_80089E14;
            if (this->moodState == 0xD) {
                tmp = -0x190;
            }
            this->moodState = 0xC;
        }
        this->methods->updateScale(this, 1, table);
        this->methods->slotCC(this, tmp, 0);
    }

    if (out->unk4 == 0x208 && sMoodCue78TransitionDone != 0) {
        this->methods->stopSoundCue(this);
        this->moodState = 1;
    }
}


extern u8 ROTATION_YAW_PLUS2[];

void Entity__MoodCue79(Entity *this, EntityMoodHandlerArg *out) {
    out->unk10 = this->methods->getProximityRatio(this);
    if (out->unk4 % 10 == 0) {
        out->unk1C = 0x19;
        out->unk20 = 2;
    }
    this->methods->updateRotation(this, 0, ROTATION_YAW_PLUS2);
    if (this->moodState == 0 && this->targetReached != 0) {
        this->methods->notifyParents(this, 0xB);
        this->moodState = 0xB;
    }
}

extern u8 TRANSLATE_Y_MINUS512[];

void Entity__MoodCue80(Entity *this, EntityMoodHandlerArg *out) {
    if (this->moodTimer < this->todFrameCount) {
        if (this->todFrame != 0) {
            if (this->todFrame == 0x14) {
                out->unk10 = 0;
                out->unk1C = 0x10;
            }
        }
    } else {
        this->methods->stopTod(this);
        this->methods->addVec14(this, TRANSLATE_Y_MINUS512);
    }
    Class6B5CC__FaceTarget((Class6B5CC *)this, (Class6B5CC *)this->target, 1, 0, 0);
}

extern u8 D_80089E08[];

void Entity__MoodCue81(Entity *this, EntityMoodHandlerArg *out) {
    s32 mod;
    s32 mood;

    if (this->moodTimer == 0 && (rand() & 1)) {
        this->moodState = rand() % 3;
    }
    if (this->moodState != 0 && this->targetReached != 0) {
        if (out->unk4 >= 0x3D) {
            out->unk4 = 0;
        }
        if (out->unk4 % 20 == 0) {
            out->unk1C = 0x17;
            out->unk10 = 0;
            out->unk20 = -1;
            out->unk44 = out->unk30 = 0x17;
            out->unk34 = -1;
            out->unk48 = -2;
        } else if (out->unk4 % 20 == 0xE) {
            out->unk1C = -2;
            out->unk30 = -2;
            out->unk44 = -2;
        }
        Class6B5CC__FaceTarget((Class6B5CC *)this, (Class6B5CC *)this->target, 1, 0, 0);
        mood = this->moodState;
        if (mood == 1) {
            this->methods->updateScale(this, 0, D_80089E08);
            mod = -0x176;
            if (this->methods->distanceToRegion(this, this->target) < 0x200) {
                this->methods->deactivate(this);
                this->moodState = mood;
            }
        } else {
            this->target->methods->slot130(this->target, 1);
            Class6B5CC__FaceTarget((Class6B5CC *)this->target, (Class6B5CC *)this, 1, 1, 0);
            if (this->moodState == 2) {
                if (this->methods->distanceToRegion(this, this->target) < 0x960) {
                    this->moodState = 0xB;
                    this->methods->notifyParents(this, 0xC);
                }
                mod = -0x60;
            } else {
                mod = 0;
            }
        }
    } else {
        out->unk10 = this->methods->getProximityRatio(this);
        if (out->unk4 % 22 == 0) {
            out->unk1C = 0x1C;
        }
        if (this->moodTimer >= 0x1F5) {
            Class6B5CC__FaceTarget((Class6B5CC *)this, (Class6B5CC *)this->target, 1, 0, 0);
        }
        if (this->methods->distanceToRegion(this, this->target) < 0x800) {
            this->target->methods->slotC4(this->target, -0x800, 0);
        }
        mod = -0x14;
    }
    this->methods->slotD0(this, mod, 1);
    if (this->unk28 != 0) {
        this->methods->slotCC(this, -0xC8, 0);
    }
}

