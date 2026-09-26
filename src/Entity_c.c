/* Third slice of the Entity block -- 20 functions, 0x4F754..0x5077C. The
 * remainder is `Entity_d` and is still a monolithic asm segment.
 *
 * The whole 97-function remainder this came out of has zero `jlabel`s and
 * zero `jr $t2`, so no slice of it needs a rodata slot attached and none of
 * it is a BIOS trampoline. Entity/Entity_b's `include/Entity.h` is already
 * heavily typed and these functions are the same class family -- extend that
 * header rather than starting a new one.
 *
 * All 20 functions are `gEntityMoodHandlerTable` mood-dispatch callbacks,
 * `Entity__MoodCueNN` where NN is the table row (`asm/data/79528.data.s`,
 * stride 0x10) -- same family and naming convention as Entity_b/_d/_e/_g.
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
#include "common.h"
#include "Entity.h"
#include "DreamSys.h"

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
            this->methods->setTranslation(this, (LongVec3 *)&((DreamSys *)this->peer)->coord2->tx);
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
