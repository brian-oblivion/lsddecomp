/* Third slice of the Entity block -- 20 functions, 0x4F754..0x5077C. The
 * remainder is `Entity_d` and is still a monolithic asm segment.
 *
 * The whole 97-function remainder this came out of has zero `jlabel`s and
 * zero `jr $t2`, so no slice of it needs a rodata slot attached and none of
 * it is a BIOS trampoline. Entity/Entity_b's `include/Entity.h` is already
 * heavily typed and these functions are the same class family -- extend that
 * header rather than starting a new one.
 *
 * Named, round 78: all 20 functions are `gEntityMoodHandlerTable`
 * mood-dispatch callbacks, `Entity__MoodCueNN` where NN is the table row
 * (`asm/data/79528.data.s`, base 0x80089EB0, stride 0x10; row = (slot
 * address - base) / 0x10) -- same family and naming convention as
 * Entity_b/_d/_e/_g. Row order does not track code address, so this unit's
 * rows (19-27, 29-38, plus 119) are not contiguous with each other or with
 * source order; `Entity__MoodCue119` sits far from its neighbours by address
 * alone, confirmed against the table rather than assumed from proximity
 * (Entity_d/round 76's lesson). `Entity__MoodCue30` additionally occupies
 * row 122 with the same handler and different data words -- one function
 * shared by two distinct mood-row configurations, named for its lower row
 * (same precedent as `Entity__MoodCue81`, Entity_e).
 *
 * Every field and vtable slot this unit's functions touch (`moodTimer`,
 * `moodState`, `target`, `todFrameCount`, `unk28`, `targetReached`, the `out->unkNN`
 * `EntityMoodHandlerArg` members, `slotC4`/`slotC8`/`slotCC`/`slotD0`,
 * `stopTod`/`distanceToRegion`/`slot200`) is shared with at least one sibling
 * Entity_x unit, so the runner renamed none of it; the head applied three
 * by type scope in round 78 (`unk80` -> `moodDuration`, `unkF4` ->
 * `targetReached`, `slot144` -> `distanceToRegion`; evidence in
 * `Entity__MoodCue21.md`, `Entity__UpdateTargetProximity.md`,
 * `Entity__MoodCue11.md`). Round 79 corrected `moodDuration` to
 * `todFrameCount` (Entity inherits it from Class65650) and renamed
 * `slot130` to `stopTod`; see `Entity__MoodCue93.md`.
 */
#include "common.h"
#include "Entity.h"

extern u8 SCALE_SIX[];
extern u8 SCALE_Y2[];
extern u8 SCALE_DOUBLE[];
extern u8 SCALE_QUARTER[];
extern u8 ROTATION_YAW_PLUS2[];
extern u8 ROTATION_YAW_PLUS9[];
extern u8 ROTATION_YAW_MINUS9[];
extern u8 D_80089C58[];
extern u8 D_80089E74[];
extern u8 ROTATION_YAW_PLUS1[];
extern u8 D_80089DB4[];
extern u8 TRANSLATE_Y_MINUS64[];
extern u8 D_80089D9C[];
extern u8 D_80089D48[];
extern u8 D_80089D60[];

void Entity__MoodCue19(Entity *this, EntityMoodHandlerArg *out) {
    out->unk10 = this->methods->getProximityRatio(this);
    if (out->unk4 % 10 == 0) {
        out->unk1C = 0x11;
    }
    this->methods->slotC4(this, -0x100, 0);
}

void Entity__MoodCue20(Entity *this, EntityMoodHandlerArg *out) {
    if (this->moodTimer == 0 && rand() % 7 == 0) {
        this->methods->updateScale(this, 1, SCALE_Y2);
    }
    if ((out->unk4 & 3) == 0) {
        out->unk10 = this->methods->getProximityRatio(this);
        out->unk1C = 0x1C;
    }
    this->methods->slotC4(this, -0x64, 0);
}

void Entity__MoodCue21(Entity *this, EntityMoodHandlerArg *out) {
    s32 half;
    s32 rem;

    out->unk10 = this->methods->getProximityRatio(this);
    half = this->todFrameCount / 2;
    rem = out->unk4 % half;
    if (rem == 0) {
        out->unk1C = 0xA;
    } else if (rem == 3) {
        out->unk30 = 0xD;
    }
    this->methods->slotC4(this, -0x1E, 1);
}

void Entity__MoodCue22(Entity *this) {
    Class6B5CC__FaceTarget((Class6B5CC *)this, (Class6B5CC *)this->target, 1, 0, 0);
}

void Entity__MoodCue23(Entity *this) {
    s32 arg1;

    Class6B5CC__FaceTarget((Class6B5CC *)this, (Class6B5CC *)this->target, 1, 0, 0);

    if (this->targetReached != 0) {
        if (this->moodTimer >= 0x41) {
            this->moodTimer = 0;
        }
        if (this->moodTimer >= 7) {
            this->methods->addVec14(this, TRANSLATE_Y_MINUS64);
            this->methods->slotC4(this, 0xA, 0);
        } else {
            this->methods->addVec14(this, D_80089D9C);
        }
    } else if (this->moodTimer == 0) {
        this->methods->addVec14(this, D_80089D48);
    } else if (this->moodTimer < 0x41) {
        this->methods->addVec14(this, D_80089D60);
    } else if (this->moodTimer < 0x47) {
        this->methods->addVec14(this, TRANSLATE_Y_MINUS64);
        this->methods->slotC4(this, -0x1E, 0);
    } else {
        EntityMethods *methods = this->methods;

        if (this->moodTimer < 0x100) {
            arg1 = -this->moodTimer - 0x41;
        } else {
            arg1 = 0xFF;
        }
        methods->slotC4(this, arg1, 0);
    }
}

void Entity__MoodCue24(Entity *this, EntityMoodHandlerArg *out) {
    if (out->unk4 % 15 == 0) {
        out->unk10 = this->methods->getProximityRatio(this);
        out->unk1C = 7;
        out->unk20 = -2;
    }
    this->methods->updateScale(this, 1, SCALE_DOUBLE);
    this->methods->updateRotation(this, 0, ROTATION_YAW_PLUS2);
    this->methods->slotC4(this, -0x200, 0);
}

void Entity__MoodCue25(Entity *this, EntityMoodHandlerArg *out) {
    out->unk10 = 0;
    if (this->moodTimer < 0x64) {
        if (out->unk4 % 3 == 0) {
            out->unk1C = 0x16;
            out->unk20 = 1;
        }
        this->methods->slotCC(this, -0x40, 0);
    } else if (this->moodTimer < 0x12C) {
        out->unk1C = 0xC;
        out->unk20 = -1;
        out->unk30 = 0xC;
        out->unk34 = -1;
        out->unk44 = 0xC;
        out->unk48 = -1;
        this->methods->slotCC(this, -0x100, 0);
    } else {
        this->methods->updateRotation(this, 0, D_80089C58);
        this->methods->slotCC(this, -0x200, 0);
    }
}

void Entity__MoodCue26(Entity *this, EntityMoodHandlerArg *out) {
    s32 v1;
    s32 arg1;
    void (**slotD0)(Entity *self, s32 arg1, s32 arg2);

    /* The do/while(0) wrapper is a no-op scoping device, load-bearing for
     * register allocation only -- see the match report. Without it GCC
     * swaps which callee-saved register holds `this` vs `out` for the
     * whole function. */
    do {
        if (out->unk4 % this->todFrameCount == 0) {
            out->unk10 = this->methods->getProximityRatio(this);
            out->unk1C = 0x1A;
            __asm__("");
            v1 = 0x6E;
            goto compare;
        }
    } while (0);
    v1 = 0x6E;
compare:
    slotD0 = &this->methods->slotD0;
    arg1 = -0x180;
    if (this->moodTimer == v1) {
        arg1 = -0x2D00;
    }
    (*slotD0)(this, arg1, 0);
}

void Entity__MoodCue27(Entity *this, EntityMoodHandlerArg *out) {
    if (out->unk4 % 70 == 0) {
        out->unk10 = 0;
        out->unk1C = 0x1B;
    }
    this->methods->slotC4(this, -0x80, 0);
    if (this->moodTimer < 0x64) {
        this->methods->slotCC(this, 0x20, 0);
    } else if (this->moodTimer >= 0x12D) {
        this->methods->slotCC(this, -0x20, 0);
    }
}

s32 Entity__MoodCue119(Entity *this) {
    return this->methods->updateScale(this, 1, SCALE_SIX);
}

void Entity__MoodCue29(Entity *this) {
    if (this->moodTimer == 0) {
        if (this->target->methods->slot200(this->target) == 7) {
            this->methods->updateScale(this, 1, D_80089E74);
            this->methods->slotCC(this, -0x7800, 0);
        }
        this->moodState = rand() % 5;
    }
    if (this->moodState == 0) {
        this->methods->updateRotation(this, 0, ROTATION_YAW_PLUS1);
    }
}

void Entity__MoodCue30(Entity *this) {
    if (this->moodState == 0) {
        if (this->target->methods->slot200(this->target) == 1) {
            this->moodState = 0xB;
        } else {
            this->moodState = 0xC;
        }
    }

    if (this->moodState == 0xC) {
        this->methods->updateScale(this, 1, SCALE_DOUBLE);
        this->methods->slotCC(this, -0x1E, 0);
    } else {
        Class6B5CC__FaceTarget((Class6B5CC *)this, (Class6B5CC *)this->target, 1, 0, 0);
        if (this->moodState == 0xB) {
            this->methods->slotC4(this, -0x64, 0);
            if ((u32)(this->moodTimer - 0x55) < 0x1E) {
                this->methods->slotCC(this, 0x50, 0);
            } else if (this->moodTimer == 0x78) {
                this->moodState = 0xD;
            }
        } else if (this->moodState == 0xD) {
            this->methods->setVec14(this, &this->target->unk14->x);
            this->methods->addVec14(this, D_80089DB4);
        }
    }
}

void Entity__MoodCue31(Entity *this, EntityMoodHandlerArg *out) {
    Class6B5CC__FaceTarget((Class6B5CC *)this, (Class6B5CC *)this->target, 1, 0, 0);
    this->target->methods->slot130(this->target, 1);
    if ((out->unk4 % 10) < 3) {
        out->unk10 = 0;
        out->unk1C = 0xD;
        out->unk30 = 0xD;
        out->unk44 = 0xD;
    }
    if (this->moodTimer == this->todFrameCount) {
        this->methods->stopTod(this);
        this->methods->notifyParents(this, 0xA);
    }
}

void Entity__MoodCue32(Entity *this) {
    this->methods->slotC4(this, -0x1E, 0);
}

void Entity__MoodCue33(Entity *this, EntityMoodHandlerArg *out) {
    if (this->targetReached != 0) {
        this->methods->updateScale(this, 1, SCALE_QUARTER);
    } else if (out->unk4 % 30 == 0) {
        out->unk10 = 0;
        out->unk1C = 3;
    }
    this->methods->slotD0(this, -0x1E, 0);
    if (this->unk28 != 0) {
        this->methods->slotCC(this, -0xC8, 0);
    }
}

void Entity__MoodCue34(Entity *this) {
    EntityMethods *methods;
    s32 arg1;

    if ((u32)(this->moodTimer - 0x190) < 0xA) {
        this->methods->updateRotation(this, 0, ROTATION_YAW_PLUS9);
    } else if ((u32)(this->moodTimer - 0x2BC) < 0xA) {
        this->methods->updateRotation(this, 0, ROTATION_YAW_MINUS9);
    } else if ((u32)(this->moodTimer - 0x33E) < 0x4) {
        this->methods->updateRotation(this, 0, ROTATION_YAW_MINUS9);
    } else if (this->moodTimer >= 0x353) {
        this->methods->deactivate(this);
    }
    methods = this->methods;
    arg1 = -0x200;
    if (this->moodTimer < 0x320) {
        arg1 = -0x3C;
    }
    methods->slotC4(this, arg1, 1);
}

void Entity__MoodCue35(Entity *this) {
    s32 rem500;
    s32 arg1a;
    s32 arg1b;
    s32 arg1c;
    void (**slotCC)(Entity *self, s32 arg1, s32 arg2);
    void (**slotC8)(Entity *self, s32 arg1, s32 arg2);
    void (**slotC4)(Entity *self, s32 arg1, s32 arg2);

    rem500 = this->moodTimer % 500;

    slotCC = &this->methods->slotCC;
    if (this->moodTimer % 6 < 3) {
        arg1a = -0x40;
    } else {
        arg1a = 0x40;
    }
    (*slotCC)(this, arg1a, 0);

    slotC8 = &this->methods->slotC8;
    if (this->moodTimer % 12 < 6) {
        arg1b = -0x40;
    } else {
        arg1b = 0x40;
    }
    (*slotC8)(this, arg1b, 0);

    slotC4 = &this->methods->slotC4;
    if (this->moodTimer % 64 < 0x20) {
        arg1c = -0x80;
    } else {
        arg1c = 0x80;
    }
    (*slotC4)(this, arg1c, 0);

    if (rem500 < 0x20) {
        this->methods->addVec14(this, TRANSLATE_Y_MINUS64);
    } else if (rem500 < 0x40) {
        this->methods->addVec14(this, D_80089D60);
    }
}

void Entity__MoodCue36(Entity *this) {
    u8 *arg2;
    s32 roll;

    if (this->moodState == 0) {
        roll = rand();
        arg2 = SCALE_SIX;
        if ((roll & 1) != 0) {
            arg2 = SCALE_DOUBLE;
        }
        this->methods->updateScale(this, 1, arg2);
        this->moodState = 0xB;
    }
    Class6B5CC__FaceTarget((Class6B5CC *)this, (Class6B5CC *)this->target, 1, 0, 0);
    if (this->methods->distanceToRegion(this, this->target) < 0x7000) {
        this->methods->slotC4(this, 0x100, 0);
    }
}

void Entity__MoodCue37(Entity *this) {
    this->methods->slotCC(this, -0x5A, 0);
}

void Entity__MoodCue38(Entity *this, EntityMoodHandlerArg *out) {
    if (out->unk4 % 120 == 0) {
        out->unk10 = this->methods->getProximityRatio(this);
        out->unk1C = 1;
    }
}
