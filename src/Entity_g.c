#include "common.h"
#include "Entity.h"

/* Data rows this unit's mood-dispatch handlers pass through to a vtable
 * call as an opaque argument -- never dereferenced here, so an opaque byte
 * array is enough to form &D_8008xxxx correctly. Real element type/count
 * unknown. Same per-unit local-declaration convention as Entity_b.c/
 * Entity_c.c/Entity_e.c (each unit keeps its own extern, not shared). */
extern u8 D_80089CAC[];
extern u8 D_80089E14[];
extern u8 D_80089DE4[];
extern u8 SCALE_HALF[];
extern u8 SCALE_EIGHTH[];
extern u8 SCALE_QUARTER[];
extern u8 D_80089CB8[];
extern u8 D_80089E2C[];
extern u8 SCALE_SIX[];
extern u8 D_80089E44[];
extern u8 ROTATION_ZPLUS4[];
extern u8 ROTATION_YAW_PLUS1[];
extern u8 ROTATION_YAW_MINUS9[];
extern u8 ROTATION_YAW_PLUS9[];
extern u8 SCALE_THIRTY_SECOND[];

/* Entity__AdvanceWobbleAndDeactivate, this unit's own function, is called by Entity__MoodCue111
 * (earlier in ROM order) before its own definition below -- forward
 * declaration, same convention CLAUDE.md documents for calling into a
 * still-INCLUDE_ASM function. Already known cross-unit from Entity_d.c's
 * own extern (Entity__MoodCue40's caller there), reproduced here matching. */
extern void Entity__AdvanceWobbleAndDeactivate(Entity *this, EntityMoodHandlerArg *out, s32 arg2, s32 arg3, s32 arg4);

void Entity__MoodCue98(Entity *this, EntityMoodHandlerArg *out) {
    if (this->unkF4 != 0) {
        if (Entity__GetOrCreateUnk100(this, NULL, 0, 0xA, 0) != 0) {
            this->unk100->methods->slotD4(this->unk100, this->unk50, 7, 0);
            this->methods->deactivate(this);
            this->target->methods->slot21C(this->target);
        }
    }
    this->methods->slotC4(this, -0x1E, 1);
}

void Entity__MoodCue102(Entity *this, EntityMoodHandlerArg *out) {
    void *a2;

    if (this->moodTimer == 0) {
        this->methods->slotCC(this, -0x200, 0);
    }
    out->unk10 = this->methods->getProximityRatio(this);
    if (this->unk84 == this->unk80 / 2) {
        out->unk1C = 7;
        out->unk20 = -2;
        out->unk30 = 3;
        out->unk34 = -2;
    }
    if (this->moodTimer >= 0x33) {
        this->methods->updateRotation(this, 0, D_80089CAC);
    }
    if (this->moodTimer >= 0x30D) {
        Class6B5CC__FaceTarget(this, this->target, 1, 0, 0);
        if (this->moodTimer >= 0x790) {
            a2 = D_80089E14;
        } else if (this->moodTimer >= 0x78B) {
            a2 = D_80089DE4;
        } else if (this->moodTimer >= 0x786) {
            a2 = SCALE_HALF;
        } else if (this->moodTimer >= 0x781) {
            a2 = SCALE_QUARTER;
        } else {
            a2 = SCALE_EIGHTH;
        }
        this->methods->updateScale(this, 1, a2);
        if (this->moodTimer < 0x7D0) {
            this->methods->slotC4(this, -0x40, 0);
        } else {
            this->moodState = 1;
        }
    } else {
        this->methods->slotC4(this, -0x100, 0);
    }
    if (this->unkF4 != 0 && this->moodState == 0) {
        this->moodState = 0xC;
        this->target->methods->slot130(this->target, 1);
        this->methods->notifyParents(this, 0xA);
    }
    if (this->moodState == 0xC) {
        this->target->methods->slotC4(this->target, 0x100, 0);
    }
}

void Entity__MoodCue103(Entity *this, EntityMoodHandlerArg *out) {
    if (this->moodTimer == 0x2BC) {
        if (rand() % 3 == 0) {
            this->moodState = 0xB;
        }
    }
    if (this->moodState == 0xB) {
        if (this->moodTimer < 0x3FC) {
            this->methods->updateRotation(this, 0, D_80089CB8);
            this->methods->slotCC(this, 0x1E, 0);
        }
        if (this->moodTimer == 0x3A2) {
            this->methods->notifyParents(this, 0xA);
        }
    } else if (this->moodTimer == 0x64 || this->moodTimer == 0x320) {
        if (rand() % 5 == 0) {
            this->unk4C->methods->slot138(this->unk4C, 4, 0);
        }
    }
    this->methods->slotC4(this, -0x1E, 0);
}

void Entity__MoodCue104(Entity *this, EntityMoodHandlerArg *out) {
    this->methods->updateScale(this, 1, SCALE_QUARTER);
    if ((u32)(this->moodTimer - 0xC9) < 0x63) {
        this->methods->slotCC(this, -0x20, 0);
    }
}

void Entity__MoodCue105(Entity *this, EntityMoodHandlerArg *out) {
    this->methods->slot60(this, rand() % 20 == 0);
}

void Entity__MoodCue106(Entity *this, EntityMoodHandlerArg *out) {
    if (this->moodTimer == 0) {
        if ((rand() & 1) == 0) {
            this->moodState = 0xB;
        }
    }
    if (this->moodState == 0xB) {
        void *fn;

        if (this->moodTimer == 0) {
            this->methods->slot130(this);
        }
        fn = this->methods->updateScale;
        ((void (*)(Entity *, s32, void *))fn)(this, 1, D_80089E2C);
        return;
    }
    if (this->moodTimer == 0) {
        this->methods->slot128(this, 1);
    }
    this->methods->slotC8(this, (this->moodTimer % 20 < 10) ? 0x20 : -0x20, 0);
}

void Entity__MoodCue108(Entity *this, EntityMoodHandlerArg *out) {
    Entity__MoodCue71(this, out);
    this->methods->updateScale(this, 1, SCALE_SIX);
}

void Entity__MoodCue109(Entity *this, EntityMoodHandlerArg *out) {
    this->methods->updateScale(this, 1, SCALE_HALF);
    this->methods->slotC4(this, -0xA, 0);
}

void Entity__MoodCue110(Entity *this, EntityMoodHandlerArg *out) {
    this->methods->updateScale(this, 1, D_80089E44);
    this->methods->slot130(this);
    if (this->moodState == 0) {
        if (this->methods->slot144(this, this->target) < 0x800) {
            this->moodState = 0xA;
            this->moodTimer = 0;
        }
    }
    if (this->moodState == 0xA) {
        if (this->moodTimer < 0x2D) {
            this->methods->updateRotation(this, 0, ROTATION_ZPLUS4);
        }
        if (this->moodTimer >= 0x1F5) {
            this->moodState = 0;
        }
    }
}

void Entity__MoodCue111(Entity *this, EntityMoodHandlerArg *out) {
    if (this->moodTimer == 0) {
        if (this->target->methods->slot200(this->target) == 5) {
            this->moodState = 0xB;
        }
    }
    if (this->moodState != 0 && this->moodTimer >= 0x870) {
        if ((u32)(this->moodTimer - 0x870) < 0x191) {
            if (this->moodTimer == 0x870) {
                this->methods->slot130(this);
                out->unk1C = -2;
                out->unk30 = -2;
                out->unk44 = -2;
                return;
            }
            if ((u32)(this->moodTimer - 0x9F6) < 0xA) {
                out->unk1C = 5;
                out->unk20 = -2;
                return;
            }
            if (this->moodTimer == 0xA00) {
                this->methods->slot12C(this);
                out->unk4 = 1;
                return;
            }
            return;
        }
        if (this->moodTimer < 0xA03) {
            return;
        }
        if (this->moodTimer >= 0xAF1) {
            this->methods->slotCC(this, -0x20, 0);
        }
        Entity__AdvanceWobbleAndDeactivate(this, out, 0x1E1, 0xFA0, -0x3C);
    } else {
        Entity__AdvanceWobbleAndDeactivate(this, out, 0x1E1, 0x884, -0x3C);
    }
}

void Entity__AdvanceWobbleAndDeactivate(Entity *this, EntityMoodHandlerArg *out, s32 arg2, s32 arg3, s32 arg4) {
    s32 unkFC;

    out->unk10 = 0;
    if (out->unk4 == 6) {
        out->unk1C = 4;
        out->unk30 = 4;
        out->unk44 = 4;
    }
    unkFC = this->moodTimer;
    if (unkFC < arg2) {
        goto L18;
    }
    if (!(arg2 + 0x5B < unkFC)) {
        goto L50;
    }
L18:
    if (unkFC < arg2 + 0x155) {
        goto L34;
    }
    if (!(arg2 + 0x1B1 < unkFC)) {
        goto L50;
    }
L34:
    if (unkFC < arg2 + 0x2BA) {
        goto L74;
    }
    if (arg2 + 0x317 < unkFC) {
        goto L74;
    }
L50:
    this->methods->updateRotation(this, 0, ROTATION_YAW_PLUS1);
L74:
    this->methods->slotC4(this, arg4, 0);
    if (this->moodTimer == arg3) {
        this->methods->deactivate(this);
        this->moodState = 1;
    }
}

void Entity__MoodCue113(Entity *this, EntityMoodHandlerArg *out) {
    Entity__MoodCue51(this, out);
}

void Entity__MoodCue114(Entity *this, EntityMoodHandlerArg *out) {
    void *a2;

    if (rand() % 3 == 0) {
        return;
    }
    if (rand() % 3 != 0) {
        a2 = ROTATION_YAW_PLUS9;
    } else {
        a2 = ROTATION_YAW_MINUS9;
    }
    this->methods->updateRotation(this, 0, a2);
}

void Entity__MoodCue117(Entity *this, EntityMoodHandlerArg *out) {
    this->methods->updateScale(this, 1, SCALE_SIX);
}

void Entity__MoodCue118(Entity *this, EntityMoodHandlerArg *out) {
    this->methods->updateScale(this, 1, SCALE_SIX);
}

void Entity__MoodCue121(Entity *this, EntityMoodHandlerArg *out) {
    this->methods->updateScale(this, 1, SCALE_QUARTER);
}

void Entity__MoodCue123(Entity *this, EntityMoodHandlerArg *out) {
    if (this->moodTimer == 0) {
        if (rand() % 5 == 0) {
            this->moodState = 0xB;
        }
    }
    this->methods->slot130(this);
    this->methods->slotC4(this, 0x64, 0);
    if (this->moodTimer == 0x3E8) {
        this->methods->stopSoundCue(this);
        this->moodState = 1;
    }
    if (this->moodState == 0xB) {
        if (this->moodTimer >= 0x12D) {
            this->target->methods->slot94(this->target, 0, 2);
            this->target->methods->slot94(this->target, 0, 7);
        }
    }
}

void Entity__MoodCue125(Entity *this, EntityMoodHandlerArg *out) {
    if (this->moodTimer == 0) {
        if ((rand() & 3) == 0) {
            goto trigger;
        }
    }
    if (this->moodTimer != 0xE10) {
        goto merge;
    }
trigger:
    this->methods->deactivate(this);
    this->moodState = 1;
merge:
    this->methods->updateScale(this, 1, D_80089E44);
    out->unk10 = this->methods->getProximityRatio(this);
    if (out->unk4 % (this->unk80 / 2) == 0) {
        out->unk1C = 0xA;
        out->unk20 = 1;
    }
    this->methods->slotC4(this, -0xA, 0);
}

void Entity__MoodCue128(Entity *this, EntityMoodHandlerArg *out) {
    Class6B5CC__FaceTarget(this, this->target, 1, 0, 0);
    this->methods->updateScale(this, 1, SCALE_THIRTY_SECOND);
    this->methods->slotC4(this, -0x1E, 1);
}

void Entity__MoodCue129(Entity *this, EntityMoodHandlerArg *out) {
    if (this->moodTimer == 0) {
        this->moodState = rand() % 2 + 0xA;
    }
    this->methods->slot130(this);
    if (this->moodTimer >= 0xC9) {
        Class6B5CC__FaceTarget(this, this->target, 1, 0, 0);
        if (this->moodState == 0xA) {
            this->methods->slotD0(this, -0x200, 0);
        }
    }
}
