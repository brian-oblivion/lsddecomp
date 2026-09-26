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
 * Fields and slots are the unified Entity's (include/Entity.h, track 4
 * round 88): the inherited ones carry Class65650's, Actor's and
 * SceneNode's names (`state`, `linkTarget`, `peer`, moveLocalZ/X/Y,
 * moveLocalZOrFindLink, ...), Entity's own are named for their occupants.
 */
#include "common.h"
#include "Entity.h"
#include "DreamSys.h"

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
extern LongVec3 D_80089DB4[];
extern LongVec3 TRANSLATE_Y_MINUS64[];
extern LongVec3 D_80089D9C[];
extern LongVec3 D_80089D48[];
extern LongVec3 D_80089D60[];

void Entity__MoodCue19(Entity *this, SoundCueSet *out) {
    out->attenuation = this->methods->getProximityRatio(this);
    if (out->tick % 10 == 0) {
        out->slots[0].program = 0x11;
    }
    this->methods->moveLocalZ(this, -0x100, 0);
}

void Entity__MoodCue20(Entity *this, SoundCueSet *out) {
    if (this->moodTimer == 0 && rand() % 7 == 0) {
        this->methods->updateScale(this, 1, SCALE_Y2);
    }
    if ((out->tick & 3) == 0) {
        out->attenuation = this->methods->getProximityRatio(this);
        out->slots[0].program = 0x1C;
    }
    this->methods->moveLocalZ(this, -0x64, 0);
}

void Entity__MoodCue21(Entity *this, SoundCueSet *out) {
    s32 half;
    s32 rem;

    out->attenuation = this->methods->getProximityRatio(this);
    half = this->todFrameCount / 2;
    rem = out->tick % half;
    if (rem == 0) {
        out->slots[0].program = 0xA;
    } else if (rem == 3) {
        out->slots[1].program = 0xD;
    }
    this->methods->moveLocalZ(this, -0x1E, (void *)1);
}

void Entity__MoodCue22(Entity *this) {
    SceneNode__FaceTarget((SceneNode *)this, (SceneNode *)this->peer, 1, 0, 0);
}

void Entity__MoodCue23(Entity *this) {
    s32 arg1;

    SceneNode__FaceTarget((SceneNode *)this, (SceneNode *)this->peer, 1, 0, 0);

    if (this->targetReached != 0) {
        if (this->moodTimer >= 0x41) {
            this->moodTimer = 0;
        }
        if (this->moodTimer >= 7) {
            this->methods->addTranslation(this, TRANSLATE_Y_MINUS64);
            this->methods->moveLocalZ(this, 0xA, 0);
        } else {
            this->methods->addTranslation(this, D_80089D9C);
        }
    } else if (this->moodTimer == 0) {
        this->methods->addTranslation(this, D_80089D48);
    } else if (this->moodTimer < 0x41) {
        this->methods->addTranslation(this, D_80089D60);
    } else if (this->moodTimer < 0x47) {
        this->methods->addTranslation(this, TRANSLATE_Y_MINUS64);
        this->methods->moveLocalZ(this, -0x1E, 0);
    } else {
        EntityMethods *methods = this->methods;

        if (this->moodTimer < 0x100) {
            arg1 = -this->moodTimer - 0x41;
        } else {
            arg1 = 0xFF;
        }
        methods->moveLocalZ(this, arg1, 0);
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
    this->methods->moveLocalZ(this, -0x200, 0);
}

void Entity__MoodCue25(Entity *this, SoundCueSet *out) {
    out->attenuation = 0;
    if (this->moodTimer < 0x64) {
        if (out->tick % 3 == 0) {
            out->slots[0].program = 0x16;
            out->slots[0].octave = 1;
        }
        this->methods->moveLocalY(this, -0x40, 0);
    } else if (this->moodTimer < 0x12C) {
        out->slots[0].program = 0xC;
        out->slots[0].octave = -1;
        out->slots[1].program = 0xC;
        out->slots[1].octave = -1;
        out->slots[2].program = 0xC;
        out->slots[2].octave = -1;
        this->methods->moveLocalY(this, -0x100, 0);
    } else {
        this->methods->updateRotation(this, 0, D_80089C58);
        this->methods->moveLocalY(this, -0x200, 0);
    }
}

void Entity__MoodCue26(Entity *this, SoundCueSet *out) {
    s32 v1;
    s32 arg1;
    void (**moveZOrFindLink)(Entity *self, s32 val, void *notify);

    /* The do/while(0) wrapper is a no-op scoping device, load-bearing for
     * register allocation only -- see the match report. Without it GCC
     * swaps which callee-saved register holds `this` vs `out` for the
     * whole function. */
    do {
        if (out->tick % this->todFrameCount == 0) {
            out->attenuation = this->methods->getProximityRatio(this);
            out->slots[0].program = 0x1A;
            /* Keeps the `li` of v1 = 0x6E below the out->slots[0].program store; without it
             * GCC schedules it above the out->attenuation store, right after the call. */
            __asm__("");
            v1 = 0x6E;
            goto compare;
        }
    } while (0);
    v1 = 0x6E;
compare:
    moveZOrFindLink = &this->methods->moveLocalZOrFindLink;
    arg1 = -0x180;
    if (this->moodTimer == v1) {
        arg1 = -0x2D00;
    }
    (*moveZOrFindLink)(this, arg1, 0);
}

void Entity__MoodCue27(Entity *this, SoundCueSet *out) {
    if (out->tick % 70 == 0) {
        out->attenuation = 0;
        out->slots[0].program = 0x1B;
    }
    this->methods->moveLocalZ(this, -0x80, 0);
    if (this->moodTimer < 0x64) {
        this->methods->moveLocalY(this, 0x20, 0);
    } else if (this->moodTimer >= 0x12D) {
        this->methods->moveLocalY(this, -0x20, 0);
    }
}

void Entity__MoodCue119(Entity *this) {
    this->methods->updateScale(this, 1, SCALE_SIX);
}

void Entity__MoodCue29(Entity *this) {
    if (this->moodTimer == 0) {
        if (((DreamSys *)this->peer)->methods->getDreamColor((DreamSys *)this->peer) == 7) {
            this->methods->updateScale(this, 1, D_80089E74);
            this->methods->moveLocalY(this, -0x7800, 0);
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
            this->state = 0xB;
        } else {
            this->state = 0xC;
        }
    }

    if (this->state == 0xC) {
        this->methods->updateScale(this, 1, SCALE_DOUBLE);
        this->methods->moveLocalY(this, -0x1E, 0);
    } else {
        SceneNode__FaceTarget((SceneNode *)this, (SceneNode *)this->peer, 1, 0, 0);
        if (this->state == 0xB) {
            this->methods->moveLocalZ(this, -0x64, 0);
            if ((u32)(this->moodTimer - 0x55) < 0x1E) {
                this->methods->moveLocalY(this, 0x50, 0);
            } else if (this->moodTimer == 0x78) {
                this->state = 0xD;
            }
        } else if (this->state == 0xD) {
            this->methods->setTranslation(this, (LongVec3 *)&((DreamSys *)this->peer)->coord2->tx);
            this->methods->addTranslation(this, D_80089DB4);
        }
    }
}

void Entity__MoodCue31(Entity *this, SoundCueSet *out) {
    SceneNode__FaceTarget((SceneNode *)this, (SceneNode *)this->peer, 1, 0, 0);
    ((DreamSys *)this->peer)->methods->clearTickCallbacks((DreamSys *)this->peer, 1);
    if ((out->tick % 10) < 3) {
        out->attenuation = 0;
        out->slots[0].program = 0xD;
        out->slots[1].program = 0xD;
        out->slots[2].program = 0xD;
    }
    if (this->moodTimer == this->todFrameCount) {
        this->methods->stopTod(this);
        this->methods->notifyParents(this, 0xA);
    }
}

void Entity__MoodCue32(Entity *this) {
    this->methods->moveLocalZ(this, -0x1E, 0);
}

void Entity__MoodCue33(Entity *this, SoundCueSet *out) {
    if (this->targetReached != 0) {
        this->methods->updateScale(this, 1, SCALE_QUARTER);
    } else if (out->tick % 30 == 0) {
        out->attenuation = 0;
        out->slots[0].program = 3;
    }
    this->methods->moveLocalZOrFindLink(this, -0x1E, 0);
    if (this->linkTarget != 0) {
        this->methods->moveLocalY(this, -0xC8, 0);
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
    methods->moveLocalZ(this, arg1, (void *)1);
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
        arg1a = -0x40;
    } else {
        arg1a = 0x40;
    }
    (*moveY)(this, arg1a, 0);

    moveX = &this->methods->moveLocalX;
    if (this->moodTimer % 12 < 6) {
        arg1b = -0x40;
    } else {
        arg1b = 0x40;
    }
    (*moveX)(this, arg1b, 0);

    moveZ = &this->methods->moveLocalZ;
    if (this->moodTimer % 64 < 0x20) {
        arg1c = -0x80;
    } else {
        arg1c = 0x80;
    }
    (*moveZ)(this, arg1c, 0);

    if (rem500 < 0x20) {
        this->methods->addTranslation(this, TRANSLATE_Y_MINUS64);
    } else if (rem500 < 0x40) {
        this->methods->addTranslation(this, D_80089D60);
    }
}

void Entity__MoodCue36(Entity *this) {
    u8 *arg2;
    s32 roll;

    if (this->state == 0) {
        roll = rand();
        arg2 = SCALE_SIX;
        if ((roll & 1) != 0) {
            arg2 = SCALE_DOUBLE;
        }
        this->methods->updateScale(this, 1, arg2);
        this->state = 0xB;
    }
    SceneNode__FaceTarget((SceneNode *)this, (SceneNode *)this->peer, 1, 0, 0);
    if (this->methods->distanceToPeer(this, this->peer) < 0x7000) {
        this->methods->moveLocalZ(this, 0x100, 0);
    }
}

void Entity__MoodCue37(Entity *this) {
    this->methods->moveLocalY(this, -0x5A, 0);
}

void Entity__MoodCue38(Entity *this, SoundCueSet *out) {
    if (out->tick % 120 == 0) {
        out->attenuation = this->methods->getProximityRatio(this);
        out->slots[0].program = 1;
    }
}
