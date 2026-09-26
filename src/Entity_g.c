/* One of the Entity class's split units (Entity_c through Entity_g cover
 * its 97-function remainder after Entity/Entity_b), 0x80064618..0x800655D4,
 * directly following Entity_e (Entity_f is a separate, not-yet-named,
 * interleaved unit -- its own address range is not contiguous with this
 * one's).
 *
 * 19 of this unit's 20 functions are `gEntityMoodHandlerTable` callbacks
 * (Entity.h), named `Entity__MoodCueNN` for the row they occupy -- rows
 * 98, 102-106, 108-111, 113-114, 117-118, 121, 123, 125, 128-129,
 * confirmed by reading disk/SLPS_015.56 directly (base 0x80089EB0 +
 * 0x10*row is the row's own `handler` word, checked against each
 * candidate function's address; row order does not track code address,
 * same finding as Entity_d/Entity_e, rounds 76-77). `Entity__MoodCue123`
 * (0x80065238) also occupies row 126 of the same table (identical
 * `handler` word, different data0/data1/data2) -- one function shared by
 * two mood-row configurations, named for its lower row.
 *
 * The 20th function, `Entity__StepYawInWindowsThenDeactivate` (formerly
 * `func_80064FBC`), is not itself a table row -- it is a shared per-tick
 * helper called directly (`jal`) by two different row handlers:
 * `Entity__MoodCue111` (this unit, twice) and `Entity__MoodCue40`
 * (Entity_d.c, cross-unit, its first caller from outside that unit).
 *
 * Five rotation/scale data constants named this round, decoded from
 * disk/SLPS_015.56 against the existing ROTATION_YAW_PLUS2/
 * ROTATION_YAW_MINUS120/ROTATION_ZPLUS9/SCALE_HALF/SCALE_SIX tables
 * (rotation/scale: four s16 {num,den} pairs for X/Y(yaw)/Z/W, W never
 * reflected in the name per the ROTATION_YAW_MINUS120/SCALE_HALF/SCALE_SIX
 * precedent): ROTATION_YAW_PLUS1, ROTATION_ZPLUS4, SCALE_EIGHTH,
 * SCALE_QUARTER, SCALE_THIRTY_SECOND. Six more data constants
 * (D_80089CAC, D_80089CB8, D_80089DE4, D_80089E14, D_80089E2C, D_80089E44)
 * were decoded but left unnamed -- either a non-whole-degree rotation
 * (no precedent for naming those) or a non-uniform-axis or non-unit-
 * fraction scale (no precedent either). See each function's match
 * report's `## Naming` / `## Data constant(s) ... unnamed` sections for
 * the per-constant evidence. `ROTATION_YAW_PLUS1` and `SCALE_QUARTER`
 * turned out to also be referenced from `src/Entity_c.c` (pre-existing,
 * not new to this round) -- each unit keeps its own local `extern`, per
 * this project's per-unit-local-view convention; the global rename
 * updated both units' externs uniformly.
 */
#include "common.h"
#include "Entity.h"
#include "DreamSys.h"
#include "Class866E8.h"

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

void Entity__MoodCue98(Entity *this, SoundCueSet *out) {
    if (this->targetReached != 0) {
        if (Entity__GetOrCreateFadeBox(this, NULL, 0, 0xA, 0) != 0) {
            this->unk100->methods->startFadeDown(this->unk100, (BasicClass *)this->ticker, 7, 0);
            this->methods->deactivate(this);
            ((DreamSys *)this->peer)->methods->resetFlashbackList((DreamSys *)this->peer);
        }
    }
    this->methods->moveLocalZ(this, -0x1E, (void *)1);
}

void Entity__MoodCue102(Entity *this, SoundCueSet *out) {
    void *a2;

    if (this->moodTimer == 0) {
        this->methods->moveLocalY(this, -0x200, 0);
    }
    out->attenuation = this->methods->getProximityRatio(this);
    if (this->todFrame == this->todFrameCount / 2) {
        out->slots[0].program = 7;
        out->slots[0].octave = -2;
        out->slots[1].program = 3;
        out->slots[1].octave = -2;
    }
    if (this->moodTimer >= 0x33) {
        this->methods->updateRotation(this, 0, D_80089CAC);
    }
    if (this->moodTimer >= 0x30D) {
        SceneNode__FaceTarget((SceneNode *)this, (SceneNode *)this->peer, 1, 0, 0);
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
            this->methods->moveLocalZ(this, -0x40, 0);
        } else {
            this->state = 1;
        }
    } else {
        this->methods->moveLocalZ(this, -0x100, 0);
    }
    if (this->targetReached != 0 && this->state == 0) {
        this->state = 0xC;
        ((DreamSys *)this->peer)->methods->clearTickCallbacks((DreamSys *)this->peer, 1);
        this->methods->notifyParents(this, 0xA);
    }
    if (this->state == 0xC) {
        ((DreamSys *)this->peer)->methods->moveLocalZ((DreamSys *)this->peer, 0x100, 0);
    }
}

void Entity__MoodCue103(Entity *this, SoundCueSet *out) {
    if (this->moodTimer == 0x2BC) {
        if (rand() % 3 == 0) {
            this->state = 0xB;
        }
    }
    if (this->state == 0xB) {
        if (this->moodTimer < 0x3FC) {
            this->methods->updateRotation(this, 0, D_80089CB8);
            this->methods->moveLocalY(this, 0x1E, 0);
        }
        if (this->moodTimer == 0x3A2) {
            this->methods->notifyParents(this, 0xA);
        }
    } else if (this->moodTimer == 0x64 || this->moodTimer == 0x320) {
        if (rand() % 5 == 0) {
            this->grid->methods->configureRateEntry(this->grid, 4, 0);
        }
    }
    this->methods->moveLocalZ(this, -0x1E, 0);
}

void Entity__MoodCue104(Entity *this, SoundCueSet *out) {
    this->methods->updateScale(this, 1, SCALE_QUARTER);
    if ((u32)(this->moodTimer - 0xC9) < 0x63) {
        this->methods->moveLocalY(this, -0x20, 0);
    }
}

void Entity__MoodCue105(Entity *this, SoundCueSet *out) {
    this->methods->setDisplay(this, rand() % 20 == 0);
}

void Entity__MoodCue106(Entity *this, SoundCueSet *out) {
    if (this->moodTimer == 0) {
        if ((rand() & 1) == 0) {
            this->state = 0xB;
        }
    }
    if (this->state == 0xB) {
        void *fn;

        if (this->moodTimer == 0) {
            this->methods->stopTod(this);
        }
        fn = this->methods->updateScale;
        ((void (*)(Entity *, s32, void *))fn)(this, 1, D_80089E2C);
        return;
    }
    if (this->moodTimer == 0) {
        this->methods->setTod(this, 1);
    }
    this->methods->moveLocalX(this, (this->moodTimer % 20 < 10) ? 0x20 : -0x20, 0);
}

void Entity__MoodCue108(Entity *this, SoundCueSet *out) {
    Entity__MoodCue71(this, out);
    this->methods->updateScale(this, 1, SCALE_SIX);
}

void Entity__MoodCue109(Entity *this, SoundCueSet *out) {
    this->methods->updateScale(this, 1, SCALE_HALF);
    this->methods->moveLocalZ(this, -0xA, 0);
}

void Entity__MoodCue110(Entity *this, SoundCueSet *out) {
    this->methods->updateScale(this, 1, D_80089E44);
    this->methods->stopTod(this);
    if (this->state == 0) {
        if (this->methods->distanceToPeer(this, this->peer) < 0x800) {
            this->state = 0xA;
            this->moodTimer = 0;
        }
    }
    if (this->state == 0xA) {
        if (this->moodTimer < 0x2D) {
            this->methods->updateRotation(this, 0, ROTATION_ZPLUS4);
        }
        if (this->moodTimer >= 0x1F5) {
            this->state = 0;
        }
    }
}

void Entity__MoodCue111(Entity *this, SoundCueSet *out) {
    if (this->moodTimer == 0) {
        if (((DreamSys *)this->peer)->methods->getDreamColor((DreamSys *)this->peer) == 5) {
            this->state = 0xB;
        }
    }
    if (this->state != 0 && this->moodTimer >= 0x870) {
        if ((u32)(this->moodTimer - 0x870) < 0x191) {
            if (this->moodTimer == 0x870) {
                this->methods->stopTod(this);
                out->slots[0].program = -2;
                out->slots[1].program = -2;
                out->slots[2].program = -2;
                return;
            }
            if ((u32)(this->moodTimer - 0x9F6) < 0xA) {
                out->slots[0].program = 5;
                out->slots[0].octave = -2;
                return;
            }
            if (this->moodTimer == 0xA00) {
                ((EntityPlayTodFn)this->methods->playTod)(this);
                out->tick = 1;
                return;
            }
            return;
        }
        if (this->moodTimer < 0xA03) {
            return;
        }
        if (this->moodTimer >= 0xAF1) {
            this->methods->moveLocalY(this, -0x20, 0);
        }
        Entity__StepYawInWindowsThenDeactivate(this, out, 0x1E1, 0xFA0, -0x3C);
    } else {
        Entity__StepYawInWindowsThenDeactivate(this, out, 0x1E1, 0x884, -0x3C);
    }
}

void Entity__StepYawInWindowsThenDeactivate(Entity *this, SoundCueSet *out, s32 arg2, s32 arg3, s32 arg4) {
    s32 timer;

    out->attenuation = 0;
    if (out->tick == 6) {
        out->slots[0].program = 4;
        out->slots[1].program = 4;
        out->slots[2].program = 4;
    }
    timer = this->moodTimer;
    if (timer < arg2) {
        goto L18;
    }
    if (!(arg2 + 0x5B < timer)) {
        goto L50;
    }
L18:
    if (timer < arg2 + 0x155) {
        goto L34;
    }
    if (!(arg2 + 0x1B1 < timer)) {
        goto L50;
    }
L34:
    if (timer < arg2 + 0x2BA) {
        goto L74;
    }
    if (arg2 + 0x317 < timer) {
        goto L74;
    }
L50:
    this->methods->updateRotation(this, 0, ROTATION_YAW_PLUS1);
L74:
    this->methods->moveLocalZ(this, arg4, 0);
    if (this->moodTimer == arg3) {
        this->methods->deactivate(this);
        this->state = 1;
    }
}

void Entity__MoodCue113(Entity *this, SoundCueSet *out) {
    Entity__MoodCue51(this, out);
}

void Entity__MoodCue114(Entity *this, SoundCueSet *out) {
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

void Entity__MoodCue117(Entity *this, SoundCueSet *out) {
    this->methods->updateScale(this, 1, SCALE_SIX);
}

void Entity__MoodCue118(Entity *this, SoundCueSet *out) {
    this->methods->updateScale(this, 1, SCALE_SIX);
}

void Entity__MoodCue121(Entity *this, SoundCueSet *out) {
    this->methods->updateScale(this, 1, SCALE_QUARTER);
}

void Entity__MoodCue123(Entity *this, SoundCueSet *out) {
    if (this->moodTimer == 0) {
        if (rand() % 5 == 0) {
            this->state = 0xB;
        }
    }
    this->methods->stopTod(this);
    this->methods->moveLocalZ(this, 0x64, 0);
    if (this->moodTimer == 0x3E8) {
        this->methods->stopSoundCue(this);
        this->state = 1;
    }
    if (this->state == 0xB) {
        if (this->moodTimer >= 0x12D) {
            ((DreamSys *)this->peer)->methods->onPadEvent((DreamSys *)this->peer, 0, 2);
            ((DreamSys *)this->peer)->methods->onPadEvent((DreamSys *)this->peer, 0, 7);
        }
    }
}

void Entity__MoodCue125(Entity *this, SoundCueSet *out) {
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
    this->state = 1;
merge:
    this->methods->updateScale(this, 1, D_80089E44);
    out->attenuation = this->methods->getProximityRatio(this);
    if (out->tick % (this->todFrameCount / 2) == 0) {
        out->slots[0].program = 0xA;
        out->slots[0].octave = 1;
    }
    this->methods->moveLocalZ(this, -0xA, 0);
}

void Entity__MoodCue128(Entity *this, SoundCueSet *out) {
    SceneNode__FaceTarget((SceneNode *)this, (SceneNode *)this->peer, 1, 0, 0);
    this->methods->updateScale(this, 1, SCALE_THIRTY_SECOND);
    this->methods->moveLocalZ(this, -0x1E, (void *)1);
}

void Entity__MoodCue129(Entity *this, SoundCueSet *out) {
    if (this->moodTimer == 0) {
        this->state = rand() % 2 + 0xA;
    }
    this->methods->stopTod(this);
    if (this->moodTimer >= 0xC9) {
        SceneNode__FaceTarget((SceneNode *)this, (SceneNode *)this->peer, 1, 0, 0);
        if (this->state == 0xA) {
            this->methods->moveLocalZOrFindLink(this, -0x200, 0);
        }
    }
}
