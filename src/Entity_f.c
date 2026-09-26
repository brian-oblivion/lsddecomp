/* Entity_f -- one of the Entity class's split units (Entity_c..Entity_g
 * hold its 97-function remainder after Entity/Entity_b), 0x800634A8..
 * 0x80064618, fully matched.
 *
 * 15 of the 17 functions are `gEntityMoodHandlerTable` callbacks
 * (Entity.h), `Entity__MoodCueNN` for the row whose `handler` word holds
 * their address: rows 82-96, consecutive and in address order here, read
 * from disk/SLPS_015.56 (base 0x80089EB0, stride 0x10). `Entity__MoodCue93`
 * also occupies row 107, named for its lower row as in Entity_c/_e/_g.
 * ServiceSoundCueSet calls each once per tick with the entity's SoundCueSet
 * (`SoundCueSet`); they request tones and step the entity's pose
 * and TOD animation on moodTimer / TOD-frame thresholds.
 *
 * The other two, `SetCueTones7_7_7` and `SetCueTones18_3_3`, are private
 * helpers of Entity__MoodCue85/86 that take only the SoundCueSet and write
 * a fixed three-voice tone request into it.
 *
 * Entity derives from Class65650 (code_55dd4.h), whose TOD fields and slots
 * it inherits at the same offsets: this round renamed the two only this unit
 * touches (`todFramePtr` +0x88, `applyTodFrame` +0x134) and proposed the
 * shared ones (setTod/playTod/stopTod, todIndex/todFrame, companion2 --
 * now Actor's `ticker` -- and moodDuration -> todFrameCount) in
 * Entity__MoodCue93.md; the head applied them by type scope at merge.
 */
#include "common.h"
#include "Entity.h"
#include "DreamSys.h"

/* Data tables reached with a raw pointer by this unit's mood-dispatch
 * handlers -- same convention as Entity_d.c/Entity_c.c's own SCALE_Y2/
 * SCALE_SIX/etc externs (separate local view per translation unit, not
 * shared via the header). */
extern u8 ROTATION_YAW_PLUS9[];
extern u8 ROTATION_YAW_PLUS180[];
extern u8 ROTATION_ZPLUS1[];
extern u8 ROTATION_ZMINUS9[];
extern LongVec3 TRANSLATE_Y_MINUS256[];
extern u8 D_80089DE4[];
extern u8 SCALE_SIX[];

/* Forward declarations: both are defined later in this file (in ROM
 * order), but Entity__MoodCue85 and Entity__MoodCue86 call them before their own
 * definitions appear -- same convention as Entity_d.c's own forward calls. */
void SetCueTones7_7_7(SoundCueSet *out);
void SetCueTones18_3_3(SoundCueSet *out);

void Entity__MoodCue82(Entity *this, SoundCueSet *out) {
    if (this->state == 0 && this->moodTimer == 0) {
        if (rand() % 3 != 0) {
            this->state = (rand() & 1) ? 0xB : 0xC;
        } else {
            this->methods->stopSoundCue(this);
            this->methods->moveLocalZ(this, -0x5000, 0);
            rand();
        }
    }
    if (this->state == 0xC) {
        SceneNode__FaceTarget((SceneNode *)this, (SceneNode *)this->peer, 1, 0, 0);
        if (this->moodTimer == 0x14) {
            out->slots[0].program = 0x12;
            out->attenuation = 0;
            out->slots[1].program = 3;
            ((DreamSys *)this->peer)->methods->clearTickCallbacks((DreamSys *)this->peer, 1);
        }
        if (this->moodTimer >= 0x15) {
            this->methods->moveLocalZ(this, -0x28, 0);
        }
        if (this->moodTimer == 0x28) {
            this->methods->notifyParents(this, 0xA);
        }
    } else if (this->state == 0xB) {
        this->methods->stopTod(this);
        if (this->methods->distanceToPeer(this, this->peer) < 0x200) {
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
        out->slots[0].program = 0xC;
        out->slots[0].octave = 2;
    }
    if (this->moodTimer == this->todFrameCount) {
        out->slots[0].program = -2;
        this->methods->stopSoundCue(this);
        this->state = 1;
    }
}

void Entity__MoodCue84(Entity *this, SoundCueSet *out) {
    out->attenuation = this->methods->getProximityRatio(this);
    if (this->todFrame < 0x28) {
        out->slots[0].program = 0xC;
        out->slots[0].octave = -2;
        out->slots[2].program = 5;
        out->slots[2].octave = -1;
        return;
    }
    if (this->todFrame == 0x28) {
        out->slots[0].program = -2;
        out->slots[2].program = -2;
        return;
    }
    if (this->todFrame == 0x2D) {
        out->slots[1].program = 0x12;
        out->slots[1].octave = 1;
        return;
    }
    if (this->todFrame == 0x40) {
        out->slots[0].program = 7;
        return;
    }
    if (this->todFrame == 0x59) {
        this->methods->stopSoundCue(this);
        this->state = 1;
    }
}

void Entity__MoodCue85(Entity *this, SoundCueSet *out) {
    if (this->state == 0) {
        if (this->todFrame == 5) {
            SetCueTones7_7_7(out);
        }
        if (this->moodTimer == this->todFrameCount) {
            this->methods->stopTod(this);
            this->state = 0xA;
            this->moodTimer = -1;
        }
    } else if (this->state == 0xA) {
        if (this->moodTimer < 0xA) {
            this->methods->updateRotation(this, 0, ROTATION_YAW_PLUS9);
            if (((DreamSys *)this->peer)->methods->getLinkCommandFlag((DreamSys *)this->peer) != 0) {
                SetCueTones7_7_7(out);
                this->state = 0xC;
                this->moodTimer = -1;
            }
        } else {
            ((DreamSys *)this->peer)->methods->clearTickCallbacks((DreamSys *)this->peer, 1);
            this->state = 0xB;
            this->moodTimer = -1;
        }
    } else if (this->state == 0xB) {
        SceneNode__FaceTarget((SceneNode *)this, (SceneNode *)this->peer, 1, 0, 0);
        if (this->moodTimer < 0x1E) {
            this->methods->moveLocalZ(this, -0xA, 0);
        } else {
            SetCueTones7_7_7(out);
            if (Entity__GetOrCreateFadeBox(this, NULL, NULL, (void *)0x1E, 0) != NULL) {
                this->unk100->methods->startFadeDown(this->unk100, (BasicClass *)this->ticker, 7, 0);
            }
            this->state = 0xD;
            this->moodTimer = -1;
        }
    } else if (this->state == 0xD) {
        if (this->moodTimer < 0x5A) {
            if (this->moodTimer == 0x1E) {
                if (Entity__GetOrCreateFadeBox(this, NULL, NULL, (void *)0xA, 0) != NULL) {
                    this->unk100->methods->startFadeUp(this->unk100, (BasicClass *)this->ticker, 0, 0);
                }
            }
            ((DreamSys *)this->peer)->methods->updateRotation((DreamSys *)this->peer, 0, ROTATION_ZPLUS1);
        } else {
            SetCueTones18_3_3(out);
            ((DreamSys *)this->peer)->methods->updateRotation((DreamSys *)this->peer, 1, ROTATION_YAW_PLUS180);
            this->methods->notifyParents(this, (rand() % 5 != 0) ? 0xA : 0xC);
            this->state = 0xE;
        }
    } else if (this->state == 0xC) {
        if (this->moodTimer < 0xA) {
            this->methods->updateRotation(this, 0, ROTATION_ZMINUS9);
        } else {
            SetCueTones18_3_3(out);
            this->methods->stopSoundCue(this);
            this->state = 1;
        }
    }
}

void Entity__MoodCue86(Entity *this, SoundCueSet *out) {
    if (this->moodTimer < 0xA) {
        this->methods->stopTod(this);
    } else if (this->moodTimer == 0xA) {
        ((EntityPlayTodFn)this->methods->playTod)(this);
    }
    if (this->todFrame == 0xA) {
        SetCueTones18_3_3(out);
    }
    if (this->moodTimer == this->todFrameCount + 0xA) {
        this->methods->stopSoundCue(this);
        this->state = 1;
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
    out->slots[0].program = 0x12;
    out->attenuation = 0;
    out->slots[1].program = 3;
    out->slots[2].program = 3;
}

void Entity__MoodCue87(Entity *this, SoundCueSet *out) {
    out->attenuation = this->methods->getProximityRatio(this);
    if (out->tick == 0) {
        out->slots[0].program = 0x12;
    }
    if (out->tick >= this->todFrameCount - 1) {
        out->tick = -1;
    }
}

void Entity__MoodCue88(Entity *this, SoundCueSet *out) {
    out->attenuation = this->methods->getProximityRatio(this);
    if (out->tick == this->todFrameCount / 2) {
        out->slots[0].program = 0x12;
    }
    if (out->tick >= this->todFrameCount - 1) {
        out->tick = -1;
    }
}

void Entity__MoodCue89(Entity *this, SoundCueSet *out) {
    if (this->moodTimer == 0x14) {
        out->slots[0].program = 0x12;
        out->attenuation = 0;
        out->slots[1].program = 3;
        return;
    }
    if (this->moodTimer == this->todFrameCount) {
        this->methods->stopSoundCue(this);
        this->state = 1;
        if (rand() & 1) {
            this->methods->notifyParents(this, 0xB);
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
            this->unk100->methods->startFadeDown(this->unk100, (BasicClass *)this->ticker, 0, 0);
        }
    } else {
        if (this->todFrame == 0) {
            do {
                this->todFramePtr = this->methods->applyTodFrame(this, this->todFramePtr, 0);
                this->todFrame += 1;
            } while (this->todFrame < 0x18);
        }
    }
    if (this->todFrame >= 0x19) {
        this->methods->moveLocalZ(this, -0x14, 0);
        ((DreamSys *)this->peer)->methods->clearTickCallbacks((DreamSys *)this->peer, 1);
    }
    if (this->moodTimer == 0x32) {
        this->methods->notifyParents(this, 0xA);
    } else if (this->moodTimer == 0xC) {
        out->attenuation = 0;
        out->slots[0].program = 0x15;
    }
    this->methods->updateScale(this, 1, D_80089DE4);
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
            } while (this->todFrame < 0x18);
        }
    } else {
        if (this->todFrame == 0) {
            out->attenuation = 0;
            out->slots[0].program = 0x16;
        } else if (this->todFrame == this->todFrameCount - 1) {
            out->attenuation = 0;
            out->slots[1].program = 0x12;
            this->methods->notifyParents(this, 0xA);
        }
    }
    this->methods->updateScale(this, 1, D_80089DE4);
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
        this->methods->moveLocalZ(this, -0x80, (void *)1);
    }
}

void Entity__MoodCue94(Entity *this, SoundCueSet *out) {
    if (this->moodTimer == 0) {
        if (((DreamSys *)this->peer)->methods->getDreamColor((DreamSys *)this->peer) != 7) {
            this->state = 0xB;
        }
    }
    out->attenuation = this->methods->getProximityRatio(this);
    if (out->tick % 10 == 0) {
        out->slots[0].program = 0xE;
    }
    if (this->moodTimer == this->todFrameCount) {
        this->methods->setTod(this, 1);
        if (this->state != 0) {
            if ((rand() & 1) == 0) {
                this->methods->updateScale(this, 1, SCALE_SIX);
                this->methods->moveLocalY(this, 0x800, 0);
            }
        }
        if (rand() % 3 == 0) {
            this->methods->updateRotation(this, 0, ROTATION_YAW_PLUS180);
        }
    }
    if (this->todIndex != 0) {
        this->methods->moveLocalZ(this, -0x80, (void *)1);
    }
}

void Entity__MoodCue95(Entity *this, SoundCueSet *out) {
    if (this->moodTimer == 0) {
        this->methods->setTod(this, 3);
    } else if (this->moodTimer == this->todFrameCount) {
        this->methods->setTod(this, 1);
    }
    if (this->todIndex == 1) {
        this->methods->moveLocalZ(this, -0x80, 0);
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
        out->slots[0].program = 0x16;
        out->slots[0].octave = 2;
        out->slots[0].vol = 0x40;
        out->slots[0].endVol = 0x20;
    }
}
