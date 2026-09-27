/* Entity_f: fifteen of Entity's MoodCue handlers and the two tone setters
 * they share.
 *
 * Each Entity__MoodCueNN is the `handler` of gEntityMoodHandlerTable's row
 * NN (include/Entity.h): rows 82 to 96, and Entity__MoodCue93 is row 107's
 * handler too (same `handler` word, different data words), named for its
 * lower row. An Entity whose moodIndex selects the row installs it as its
 * SoundCueSet callback, so ServiceSoundCueSet calls it once per tick with
 * the Entity and its cue set. A handler requests tones by filling the
 * set's slots (a VAB program of the cue's sound object, or SOUND_CUE_STOP),
 * moves, turns and scales the entity (or the player, its `peer`) on
 * moodTimer, the ticks since startSoundCue, on the cue set's own `tick`, or
 * on todFrame, the frame of its TOD animation, and sends the dream an
 * EntityEffect through notifyParents. Entity__MoodCue91 and
 * Entity__MoodCue92 skip their TOD animation ahead to frame 24 by stepping
 * applyTodFrame, which returns the next frame's pointer.
 *
 * SetCueTones7_7_7 and SetCueTones18_3_3 are not rows: they write a fixed
 * three-voice request into the set, unattenuated (programs 7, 7, 7 at
 * octave -2, and 18, 3, 3), for Entity__MoodCue85 and
 * Entity__MoodCue86. Entity__MoodCue82 and Entity__MoodCue89 write the
 * 18, 3 request inline.
 *
 * The literals are left unnamed where they are one handler's tuning: tick
 * counts, distances in world units, TOD frame numbers, VAB program numbers,
 * and the `state` values other than 0 and ENTITY_STATE_DONE, which are each
 * handler's own phases. The motion templates (ROTATION_*, SCALE_*,
 * TRANSLATE_*) are named by value and declared once in include/Entity.h.
 */
#include "common.h"
#include "Entity.h"
#include "DreamSys.h"

/* Defined after Entity__MoodCue86, which calls them. */
void SetCueTones7_7_7(SoundCueSet *out);
void SetCueTones18_3_3(SoundCueSet *out);

void Entity__MoodCue82(Entity *this, SoundCueSet *out) {
    if (this->state == 0 && this->moodTimer == 0) {
        if (rand() % 3 != 0) {
            this->state = (rand() & 1) ? 11 : 12;
        } else {
            this->methods->stopSoundCue(this);
            this->methods->moveLocalZ(this, -20480, 0);
            rand();
        }
    }
    if (this->state == 12) {
        SceneNode__FaceTarget((SceneNode *)this, (SceneNode *)this->peer, 1, 0, 0);
        if (this->moodTimer == 20) {
            out->slots[0].program = 18;
            out->attenuation = 0;
            out->slots[1].program = 3;
            ((DreamSys *)this->peer)->methods->clearTickCallbacks((DreamSys *)this->peer, 1);
        }
        if (this->moodTimer >= 21) {
            this->methods->moveLocalZ(this, -40, 0);
        }
        if (this->moodTimer == 40) {
            this->methods->notifyParents(this, ENTITY_EFFECT_LINK_STAGE);
        }
    } else if (this->state == 11) {
        this->methods->stopTod(this);
        if (this->methods->distanceToPeer(this, this->peer) < 512) {
            if (rand() % 3 != 0) {
                this->methods->deactivate(this);
            } else {
                this->methods->stopSoundCue(this);
            }
        }
    }
}

void Entity__MoodCue83(Entity *this, SoundCueSet *out) {
    if (this->todFrame % 15 == 0) {
        out->attenuation = 0;
        out->slots[0].program = 12;
        out->slots[0].octave = 2;
    }
    if (this->moodTimer == this->todFrameCount) {
        out->slots[0].program = SOUND_CUE_STOP;
        this->methods->stopSoundCue(this);
        this->state = ENTITY_STATE_DONE;
    }
}

void Entity__MoodCue84(Entity *this, SoundCueSet *out) {
    out->attenuation = this->methods->getProximityRatio(this);
    if (this->todFrame < 40) {
        out->slots[0].program = 12;
        out->slots[0].octave = -2;
        out->slots[2].program = 5;
        out->slots[2].octave = -1;
        return;
    }
    if (this->todFrame == 40) {
        out->slots[0].program = SOUND_CUE_STOP;
        out->slots[2].program = SOUND_CUE_STOP;
        return;
    }
    if (this->todFrame == 45) {
        out->slots[1].program = 18;
        out->slots[1].octave = 1;
        return;
    }
    if (this->todFrame == 64) {
        out->slots[0].program = 7;
        return;
    }
    if (this->todFrame == 89) {
        this->methods->stopSoundCue(this);
        this->state = ENTITY_STATE_DONE;
    }
}

void Entity__MoodCue85(Entity *this, SoundCueSet *out) {
    if (this->state == 0) {
        if (this->todFrame == 5) {
            SetCueTones7_7_7(out);
        }
        if (this->moodTimer == this->todFrameCount) {
            this->methods->stopTod(this);
            this->state = 10;
            this->moodTimer = -1;
        }
    } else if (this->state == 10) {
        if (this->moodTimer < 10) {
            this->methods->updateRotation(this, 0, ROTATION_YAW_PLUS9);
            if (((DreamSys *)this->peer)->methods->getLinkCommandFlag((DreamSys *)this->peer) != 0) {
                SetCueTones7_7_7(out);
                this->state = 12;
                this->moodTimer = -1;
            }
        } else {
            ((DreamSys *)this->peer)->methods->clearTickCallbacks((DreamSys *)this->peer, 1);
            this->state = 11;
            this->moodTimer = -1;
        }
    } else if (this->state == 11) {
        SceneNode__FaceTarget((SceneNode *)this, (SceneNode *)this->peer, 1, 0, 0);
        if (this->moodTimer < 30) {
            this->methods->moveLocalZ(this, -10, 0);
        } else {
            SetCueTones7_7_7(out);
            if (Entity__GetOrCreateFadeBox(this, NULL, NULL, (void *)30, 0) != NULL) {
                this->fadeBox->methods->startFadeDown(this->fadeBox, (BasicClass *)this->ticker, 7, 0);
            }
            this->state = 13;
            this->moodTimer = -1;
        }
    } else if (this->state == 13) {
        if (this->moodTimer < 90) {
            if (this->moodTimer == 30) {
                if (Entity__GetOrCreateFadeBox(this, NULL, NULL, (void *)10, 0) != NULL) {
                    this->fadeBox->methods->startFadeUp(this->fadeBox, (BasicClass *)this->ticker, 0, 0);
                }
            }
            ((DreamSys *)this->peer)->methods->updateRotation((DreamSys *)this->peer, 0, ROTATION_ZPLUS1);
        } else {
            SetCueTones18_3_3(out);
            ((DreamSys *)this->peer)->methods->updateRotation((DreamSys *)this->peer, 1, ROTATION_YAW_PLUS180);
            this->methods->notifyParents(this, (rand() % 5 != 0) ? ENTITY_EFFECT_LINK_STAGE
                                                                 : ENTITY_EFFECT_END_DREAM);
            this->state = 14;
        }
    } else if (this->state == 12) {
        if (this->moodTimer < 10) {
            this->methods->updateRotation(this, 0, ROTATION_ZMINUS9);
        } else {
            SetCueTones18_3_3(out);
            this->methods->stopSoundCue(this);
            this->state = ENTITY_STATE_DONE;
        }
    }
}

void Entity__MoodCue86(Entity *this, SoundCueSet *out) {
    if (this->moodTimer < 10) {
        this->methods->stopTod(this);
    } else if (this->moodTimer == 10) {
        ((EntityPlayTodFn)this->methods->playTod)(this);
    }
    if (this->todFrame == 10) {
        SetCueTones18_3_3(out);
    }
    if (this->moodTimer == this->todFrameCount + 10) {
        this->methods->stopSoundCue(this);
        this->state = ENTITY_STATE_DONE;
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

void Entity__MoodCue87(Entity *this, SoundCueSet *out) {
    out->attenuation = this->methods->getProximityRatio(this);
    if (out->tick == 0) {
        out->slots[0].program = 18;
    }
    if (out->tick >= this->todFrameCount - 1) {
        out->tick = -1;
    }
}

void Entity__MoodCue88(Entity *this, SoundCueSet *out) {
    out->attenuation = this->methods->getProximityRatio(this);
    if (out->tick == this->todFrameCount / 2) {
        out->slots[0].program = 18;
    }
    if (out->tick >= this->todFrameCount - 1) {
        out->tick = -1;
    }
}

void Entity__MoodCue89(Entity *this, SoundCueSet *out) {
    if (this->moodTimer == 20) {
        out->slots[0].program = 18;
        out->attenuation = 0;
        out->slots[1].program = 3;
        return;
    }
    if (this->moodTimer == this->todFrameCount) {
        this->methods->stopSoundCue(this);
        this->state = ENTITY_STATE_DONE;
        if (rand() & 1) {
            this->methods->notifyParents(this, ENTITY_EFFECT_EVENT_VIDEO);
        }
    }
}

void Entity__MoodCue90(Entity *this, SoundCueSet *out) {
    out->attenuation = this->methods->getProximityRatio(this);
    if (out->tick == 0) {
        out->slots[0].program = (rand() & 1) ? 2 : 1;
        out->slots[0].octave = 2;
    }
}

void Entity__MoodCue91(Entity *this, SoundCueSet *out) {
    if (this->moodTimer == 0) {
        if (Entity__GetOrCreateFadeBox(this, NULL, NULL, (void *)5, 0) != NULL) {
            if (rand() & 1) {
                this->methods->addTranslation(this, TRANSLATE_Y_MINUS256);
            }
            this->fadeBox->methods->startFadeDown(this->fadeBox, (BasicClass *)this->ticker, 0, 0);
        }
    } else {
        if (this->todFrame == 0) {
            do {
                this->todFramePtr = this->methods->applyTodFrame(this, this->todFramePtr, 0);
                this->todFrame += 1;
            } while (this->todFrame < 24);
        }
    }
    if (this->todFrame >= 25) {
        this->methods->moveLocalZ(this, -20, 0);
        ((DreamSys *)this->peer)->methods->clearTickCallbacks((DreamSys *)this->peer, 1);
    }
    if (this->moodTimer == 50) {
        this->methods->notifyParents(this, ENTITY_EFFECT_LINK_STAGE);
    } else if (this->moodTimer == 12) {
        out->attenuation = 0;
        out->slots[0].program = 21;
    }
    this->methods->updateScale(this, 1, SCALE_X_FOUR_FIFTHS_Y_SIX_FIFTHS);
}

void Entity__MoodCue92(Entity *this, SoundCueSet *out) {
    if (this->todIndex == 0) {
        if (this->targetReached != 0) {
            SceneNode__FaceTarget((SceneNode *)this, (SceneNode *)this->peer, 1, 0, 0);
            this->methods->setTod(this, 1);
            ((DreamSys *)this->peer)->methods->clearTickCallbacks((DreamSys *)this->peer, 1);
        } else if (this->todFrame == 0) {
            do {
                this->todFramePtr = this->methods->applyTodFrame(this, this->todFramePtr, 0);
                this->todFrame += 1;
            } while (this->todFrame < 24);
        }
    } else {
        if (this->todFrame == 0) {
            out->attenuation = 0;
            out->slots[0].program = 22;
        } else if (this->todFrame == this->todFrameCount - 1) {
            out->attenuation = 0;
            out->slots[1].program = 18;
            this->methods->notifyParents(this, ENTITY_EFFECT_LINK_STAGE);
        }
    }
    this->methods->updateScale(this, 1, SCALE_X_FOUR_FIFTHS_Y_SIX_FIFTHS);
}

void Entity__MoodCue93(Entity *this, SoundCueSet *out) {
    out->attenuation = this->methods->getProximityRatio(this);
    if (out->tick % 10 == 0) {
        out->slots[0].program = 3;
    }
    if (this->moodTimer == this->todFrameCount) {
        this->methods->setTod(this, 1);
    }
    if (this->todIndex == 1) {
        this->methods->moveLocalZ(this, -128, (void *)1);
    }
}

void Entity__MoodCue94(Entity *this, SoundCueSet *out) {
    if (this->moodTimer == 0) {
        if (((DreamSys *)this->peer)->methods->getDreamColor((DreamSys *)this->peer) != DREAM_COLOR_WHITE) {
            this->state = 11;
        }
    }
    out->attenuation = this->methods->getProximityRatio(this);
    if (out->tick % 10 == 0) {
        out->slots[0].program = 14;
    }
    if (this->moodTimer == this->todFrameCount) {
        this->methods->setTod(this, 1);
        if (this->state != 0) {
            if ((rand() & 1) == 0) {
                this->methods->updateScale(this, 1, SCALE_SIX);
                this->methods->moveLocalY(this, 2048, 0);
            }
        }
        if (rand() % 3 == 0) {
            this->methods->updateRotation(this, 0, ROTATION_YAW_PLUS180);
        }
    }
    if (this->todIndex != 0) {
        this->methods->moveLocalZ(this, -128, (void *)1);
    }
}

void Entity__MoodCue95(Entity *this, SoundCueSet *out) {
    if (this->moodTimer == 0) {
        this->methods->setTod(this, 3);
    } else if (this->moodTimer == this->todFrameCount) {
        this->methods->setTod(this, 1);
    }
    if (this->todIndex == 1) {
        this->methods->moveLocalZ(this, -128, 0);
    }
}

void Entity__MoodCue96(Entity *this, SoundCueSet *out) {
    if (this->moodTimer == 0) {
        this->methods->setTod(this, rand() % 4);
        return;
    }
    if (this->moodTimer % this->todFrameCount == 0) {
        this->methods->setTod(this, rand() % 4);
        out->attenuation = this->methods->getProximityRatio(this);
        out->slots[0].program = 22;
        out->slots[0].octave = 2;
        out->slots[0].vol = 64;
        out->slots[0].endVol = 32;
    }
}
