/* Entity_g: twenty of Entity's MoodCue handlers and the helper one of them
 * shares.
 *
 * Each Entity__MoodCueNN is the `handler` of gEntityMoodHandlerTable's row
 * NN (include/Entity.h): rows 98, 102 to 106, 108 to 111, 113, 114, 117,
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
 * Entity__MoodCue111 (this unit, twice) and Entity__MoodCue40 (Entity_d.c,
 * its only caller from outside this unit).
 *
 * The literals are left unnamed where they are one handler's tuning: tick
 * counts, distances in world units, TOD frame numbers, VAB program
 * numbers, and the `state` values other than 0 and ENTITY_STATE_DONE,
 * which are each handler's own phases. The named motion templates
 * (ROTATION_YAW_PLUS1, ROTATION_YAW_PLUS4, SCALE_EIGHTH, SCALE_QUARTER,
 * SCALE_THIRTY_SECOND, SCALE_UNIT) are declared once in include/Entity.h.
 * Five more data constants (ROTATION_YAW_MINUS_THIRD, ROTATION_YAW_MINUS_HALF, SCALE_X_FOUR_FIFTHS_Y_SIX_FIFTHS, SCALE_X_EIGHTH_Y2_Z_EIGHTH,
 * SCALE_TWO_FIFTHS) are left unnamed for lack of a naming precedent -- a
 * non-whole-degree rotation or a non-uniform/non-unit-fraction scale; see
 * each function's match report (`## Naming` / `## Data constant(s) ...
 * unnamed`) for the per-constant evidence.
 */
#include "common.h"
#include "Entity.h"
#include "DreamSys.h"
#include "StageMap.h"

void Entity__MoodCue98(Entity *this, SoundCueSet *out) {
    if (this->targetReached != 0) {
        if (Entity__GetOrCreateFadeBox(this, NULL, 0, 10, 0) != 0) {
            this->fadeBox->methods->startFadeDown(this->fadeBox, (BasicClass *)this->ticker, 7, 0);
            this->methods->deactivate(this);
            ((DreamSys *)this->peer)->methods->resetFlashbackList((DreamSys *)this->peer);
        }
    }
    this->methods->moveLocalZ(this, -30, (void *)1);
}

void Entity__MoodCue102(Entity *this, SoundCueSet *out) {
    void *a2;

    if (this->moodTimer == 0) {
        this->methods->moveLocalY(this, -512, 0);
    }
    out->attenuation = this->methods->getProximityRatio(this);
    if (this->todFrame == this->todFrameCount / 2) {
        out->slots[0].program = 7;
        out->slots[0].octave = -2;
        out->slots[1].program = 3;
        out->slots[1].octave = -2;
    }
    if (this->moodTimer >= 51) {
        this->methods->updateRotation(this, 0, ROTATION_YAW_MINUS_THIRD);
    }
    if (this->moodTimer >= 781) {
        SceneNode__FaceTarget((SceneNode *)this, (SceneNode *)this->peer, 1, 0, 0);
        if (this->moodTimer >= 1936) {
            a2 = SCALE_UNIT;
        } else if (this->moodTimer >= 1931) {
            a2 = SCALE_X_FOUR_FIFTHS_Y_SIX_FIFTHS;
        } else if (this->moodTimer >= 1926) {
            a2 = SCALE_HALF;
        } else if (this->moodTimer >= 1921) {
            a2 = SCALE_QUARTER;
        } else {
            a2 = SCALE_EIGHTH;
        }
        this->methods->updateScale(this, 1, a2);
        if (this->moodTimer < 2000) {
            this->methods->moveLocalZ(this, -64, 0);
        } else {
            this->state = 1;
        }
    } else {
        this->methods->moveLocalZ(this, -256, 0);
    }
    if (this->targetReached != 0 && this->state == 0) {
        this->state = 12;
        ((DreamSys *)this->peer)->methods->clearTickCallbacks((DreamSys *)this->peer, 1);
        this->methods->notifyParents(this, ENTITY_EFFECT_LINK_STAGE);
    }
    if (this->state == 12) {
        ((DreamSys *)this->peer)->methods->moveLocalZ((DreamSys *)this->peer, 256, 0);
    }
}

void Entity__MoodCue103(Entity *this, SoundCueSet *out) {
    if (this->moodTimer == 700) {
        if (rand() % 3 == 0) {
            this->state = 11;
        }
    }
    if (this->state == 11) {
        if (this->moodTimer < 1020) {
            this->methods->updateRotation(this, 0, ROTATION_YAW_MINUS_HALF);
            this->methods->moveLocalY(this, 30, 0);
        }
        if (this->moodTimer == 930) {
            this->methods->notifyParents(this, ENTITY_EFFECT_LINK_STAGE);
        }
    } else if (this->moodTimer == 100 || this->moodTimer == 800) {
        if (rand() % 5 == 0) {
            this->grid->methods->startScaleRamp(this->grid, 4, 0);
        }
    }
    this->methods->moveLocalZ(this, -30, 0);
}

void Entity__MoodCue104(Entity *this, SoundCueSet *out) {
    this->methods->updateScale(this, 1, SCALE_QUARTER);
    if ((u32)(this->moodTimer - 201) < 99) {
        this->methods->moveLocalY(this, -32, 0);
    }
}

void Entity__MoodCue105(Entity *this, SoundCueSet *out) {
    this->methods->setDisplay(this, rand() % 20 == 0);
}

void Entity__MoodCue106(Entity *this, SoundCueSet *out) {
    if (this->moodTimer == 0) {
        if ((rand() & 1) == 0) {
            this->state = 11;
        }
    }
    if (this->state == 11) {
        void *fn;

        if (this->moodTimer == 0) {
            this->methods->stopTod(this);
        }
        fn = this->methods->updateScale;
        ((void (*)(Entity *, s32, void *))fn)(this, 1, SCALE_X_EIGHTH_Y2_Z_EIGHTH);
        return;
    }
    if (this->moodTimer == 0) {
        this->methods->setTod(this, 1);
    }
    this->methods->moveLocalX(this, (this->moodTimer % 20 < 10) ? 32 : -32, 0);
}

void Entity__MoodCue108(Entity *this, SoundCueSet *out) {
    Entity__MoodCue71(this, out);
    this->methods->updateScale(this, 1, SCALE_SIX);
}

void Entity__MoodCue109(Entity *this, SoundCueSet *out) {
    this->methods->updateScale(this, 1, SCALE_HALF);
    this->methods->moveLocalZ(this, -10, 0);
}

void Entity__MoodCue110(Entity *this, SoundCueSet *out) {
    this->methods->updateScale(this, 1, SCALE_TWO_FIFTHS);
    this->methods->stopTod(this);
    if (this->state == 0) {
        if (this->methods->distanceToPeer(this, this->peer) < 2048) {
            this->state = 10;
            this->moodTimer = 0;
        }
    }
    if (this->state == 10) {
        if (this->moodTimer < 45) {
            this->methods->updateRotation(this, 0, ROTATION_YAW_PLUS4);
        }
        if (this->moodTimer >= 501) {
            this->state = 0;
        }
    }
}

void Entity__MoodCue111(Entity *this, SoundCueSet *out) {
    if (this->moodTimer == 0) {
        if (((DreamSys *)this->peer)->methods->getDreamColor((DreamSys *)this->peer) == 5) {
            this->state = 11;
        }
    }
    if (this->state != 0 && this->moodTimer >= 2160) {
        if ((u32)(this->moodTimer - 2160) < 401) {
            if (this->moodTimer == 2160) {
                this->methods->stopTod(this);
                out->slots[0].program = -2;
                out->slots[1].program = -2;
                out->slots[2].program = -2;
                return;
            }
            if ((u32)(this->moodTimer - 2550) < 10) {
                out->slots[0].program = 5;
                out->slots[0].octave = -2;
                return;
            }
            if (this->moodTimer == 2560) {
                ((EntityPlayTodFn)this->methods->playTod)(this);
                out->tick = 1;
                return;
            }
            return;
        }
        if (this->moodTimer < 2563) {
            return;
        }
        if (this->moodTimer >= 2801) {
            this->methods->moveLocalY(this, -32, 0);
        }
        Entity__StepYawInWindowsThenDeactivate(this, out, 481, 4000, -60);
    } else {
        Entity__StepYawInWindowsThenDeactivate(this, out, 481, 2180, -60);
    }
}

void Entity__StepYawInWindowsThenDeactivate(Entity *this, SoundCueSet *out, s32 windowStart,
                                            s32 deactivateTimer, s32 zStep) {
    s32 timer;

    out->attenuation = 0;
    if (out->tick == 6) {
        out->slots[0].program = 4;
        out->slots[1].program = 4;
        out->slots[2].program = 4;
    }
    timer = this->moodTimer;
    if (timer < windowStart) {
        goto L18;
    }
    if (!(windowStart + 91 < timer)) {
        goto L50;
    }
L18:
    if (timer < windowStart + 341) {
        goto L34;
    }
    if (!(windowStart + 433 < timer)) {
        goto L50;
    }
L34:
    if (timer < windowStart + 698) {
        goto L74;
    }
    if (windowStart + 791 < timer) {
        goto L74;
    }
L50:
    this->methods->updateRotation(this, 0, ROTATION_YAW_PLUS1);
L74:
    this->methods->moveLocalZ(this, zStep, 0);
    if (this->moodTimer == deactivateTimer) {
        this->methods->deactivate(this);
        this->state = ENTITY_STATE_DONE;
    }
}

void Entity__MoodCue113(Entity *this, SoundCueSet *out) {
    Entity__MoodCue51(this, out);
}

void Entity__MoodCue114(Entity *this, SoundCueSet *out) {
    Ratio16 *a2;

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
            this->state = 11;
        }
    }
    this->methods->stopTod(this);
    this->methods->moveLocalZ(this, 100, 0);
    if (this->moodTimer == 1000) {
        this->methods->stopSoundCue(this);
        this->state = ENTITY_STATE_DONE;
    }
    if (this->state == 11) {
        if (this->moodTimer >= 301) {
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
    if (this->moodTimer != 3600) {
        goto merge;
    }
trigger:
    this->methods->deactivate(this);
    this->state = ENTITY_STATE_DONE;
merge:
    this->methods->updateScale(this, 1, SCALE_TWO_FIFTHS);
    out->attenuation = this->methods->getProximityRatio(this);
    if (out->tick % (this->todFrameCount / 2) == 0) {
        out->slots[0].program = 10;
        out->slots[0].octave = 1;
    }
    this->methods->moveLocalZ(this, -10, 0);
}

void Entity__MoodCue128(Entity *this, SoundCueSet *out) {
    SceneNode__FaceTarget((SceneNode *)this, (SceneNode *)this->peer, 1, 0, 0);
    this->methods->updateScale(this, 1, SCALE_THIRTY_SECOND);
    this->methods->moveLocalZ(this, -30, (void *)1);
}

void Entity__MoodCue129(Entity *this, SoundCueSet *out) {
    if (this->moodTimer == 0) {
        this->state = rand() % 2 + 10;
    }
    this->methods->stopTod(this);
    if (this->moodTimer >= 201) {
        SceneNode__FaceTarget((SceneNode *)this, (SceneNode *)this->peer, 1, 0, 0);
        if (this->state == 10) {
            this->methods->moveLocalZOrFindLink(this, -512, 0);
        }
    }
}
