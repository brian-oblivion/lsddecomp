/* Third 20-function slice of the Entity class's 97-function remainder,
 * following Entity_c and Entity_d (Entity_d's own header comment names the
 * split). Every function in this unit is a `gEntityMoodHandlerTable`
 * callback (Entity.h), named `Entity__MoodCueNN` for the row it occupies --
 * rows 59, 61-62, 64-71, 73-81; row order does not track code address (see
 * each function's match report for the row derivation). Rows 60, 63 and 72
 * have a NULL handler word in the table -- those mood indices legitimately
 * dispatch no per-tick cue callback at all, not a gap in this unit's queue.
 *
 * `Entity__MoodCue81` also occupies a second row of the same table, under
 * different tuning data; `Entity__MoodCue71` is also called cross-unit, from
 * Entity_g (see each function's own match report).
 *
 * Rotation/scale tables are three s16 {num,den} pairs for X/Y(yaw)/Z;
 * translate tables are three consecutive s32 for X/Y/Z. `sMoodCue78TransitionDone`
 * is a one-shot s32 flag local to Entity__MoodCue78's own state machine,
 * referenced nowhere else in `src/`. See each function's match report's
 * `## Naming` section for the per-constant evidence.
 */
#include "common.h"
#include "Entity.h"
#include "DreamSys.h"
#include "Class866E8.h"
#include "Viewport.h"

void Entity__MoodCue59(Entity *this, SoundCueSet *out) {
    if (this->moodTimer == 0 && rand() % 10 == 0) {
        this->state = 0xC;
    }
    if (out->tick % 10 == 0) {
        out->attenuation = this->methods->getProximityRatio(this);
        out->slots[0].program = 0xC;
        out->slots[0].octave = -1;
    }
    if (this->moodTimer == 0) {
        if (rand() & 1) {
            this->methods->moveLocalY(this, 0x800, 0);
        }
    }
    this->methods->moveLocalZ(this, -0x80, 0);
    if (this->state == 0xC && this->moodTimer == 0x12C) {
        this->grid->methods->configureRateEntry(this->grid, 1, 1);
    }
}

void Entity__MoodCue61(Entity *this, SoundCueSet *out) {
    if (this->todFrame == 0x1E) {
        out->slots[0].program = 0x12;
        out->attenuation = 0;
        out->slots[0].octave = -1;
    }
}

void Entity__MoodCue62(Entity *this, SoundCueSet *out) {
    if (this->todFrame % 30 == 0) {
        out->slots[0].program = 3;
        out->attenuation = 0;
        out->slots[0].octave = -2;
    }
    if (this->moodTimer >= 0x65) {
        SceneNode__FaceTarget((SceneNode *)this, (SceneNode *)this->peer, 1, 0, 0);
    }
    this->methods->moveLocalZ(this, -5, 0);
    if (this->moodTimer == 0x12C && this->methods->distanceToPeer(this, this->peer) < 0x1000) {
        ((DreamSys *)this->peer)->methods->clearTickCallbacks((DreamSys *)this->peer, 0);
    } else if (this->moodTimer == 0x1F4) {
        ((DreamSys *)this->peer)->methods->setTickCallbacks((DreamSys *)this->peer, 1, 1);
    }
    if (this->state == 0) {
        if (this->methods->distanceToPeer(this, this->peer) < 0x400) {
            if (rand() & 1) {
                out->slots[1].program = 6;
                out->attenuation = 0;
                out->slots[1].octave = -1;
                if (rand() & 1) {
                    this->grid->methods->configureRateEntry(this->grid, -1, 0);
                }
                this->moodTimer = 0;
                this->state = 0xA;
            } else {
                this->state = 0xB;
            }
        }
    }
    if (this->state == 0xA && this->moodTimer == 0x46) {
        this->methods->notifyParents(this, (rand() & 1) ? 0xC : 0xB);
    }
}

void Entity__MoodCue64(Entity *this, SoundCueSet *out) {
    s32 r = out->tick % 300;

    out->attenuation = this->methods->getProximityRatio(this);
    if (r < 0x14) {
        out->slots[0].program = 5;
        out->slots[0].octave = -2;
    } else if (r == 0x16) {
        out->slots[0].program = -2;
    }
    this->methods->moveLocalZ(this, -0xA, 0);
}

void Entity__MoodCue65(Entity *this, SoundCueSet *out) {
    if (this->moodTimer == 0) {
        if (rand() % 3 == 0) {
            this->methods->updateScale(this, 1, SCALE_HALF);
            this->methods->moveLocalY(this, -0x12C, 0);
            this->methods->updateRotation(this, 1, ROTATION_YAW_PLUS90);
            this->state = 0xB;
        }
    }
    if (this->state == 0xB) {
        if (this->moodTimer == 0x7D0) {
            this->methods->updateRotation(this, 0, ROTATION_YAW_MINUS90);
        }
        this->methods->moveLocalZ(this, -0x14, 0);
    }
}

void Entity__MoodCue66(Entity *this, SoundCueSet *out) {
    out->attenuation = this->methods->getProximityRatio(this);
    if (out->tick % 30 == 0) {
        out->slots[0].program = 0xD;
    }
}

void Entity__MoodCue67(Entity *this, SoundCueSet *out) {
    if (this->moodTimer == 0) {
        if (rand() % 3 == 0) {
            this->state = 0xB;
        }
    }
    if (this->state == 0xB) {
        if (this->moodTimer == 0x1F6) {
            this->methods->moveLocalY(this, 0x800, 0);
            SceneNode__FaceTarget((SceneNode *)this, (SceneNode *)this->peer, 1, 0, 0);
        }
        if (this->moodTimer >= 0x1F5) {
            this->methods->moveLocalZ(this, -0x200, 0);
        }
    }
}

void Entity__MoodCue68(Entity *this, SoundCueSet *out) {
    out->attenuation = this->methods->getProximityRatio(this);
    if (out->tick == 0) {
        this->lastOffsetValue = (rand() & 1) ? -0x176 : -0xC0;
    }
    if (this->state == 0) {
        if (out->tick % 10 == 0) {
            out->slots[0].program = 0x1C;
        }
        if (out->tick % 20 == 0) {
            out->slots[1].program = 0x17;
            out->slots[1].octave = -1;
            out->slots[2].program = 0x17;
            out->slots[2].octave = -1;
        } else if (out->tick % 20 == 0xE) {
            out->slots[1].program = -2;
            out->slots[2].program = -2;
        }
        if ((this->moodTimer & 1) == 0) {
            if (((DreamSys *)this->peer)->methods->getLinkCommandFlag((DreamSys *)this->peer) != 0) {
                this->moodTimer = -1;
                this->state = 0xA;
                out->slots[0].program = 0x12;
            }
        }
        SceneNode__FaceTarget((SceneNode *)this, (SceneNode *)this->peer, 1, 0, 0);
        this->methods->moveLocalZ(this, this->lastOffsetValue, (void *)1);
    } else if (this->state == 0xA) {
        if (this->moodTimer < 8) {
            this->methods->updateRotation(this, 0, ROTATION_ZPLUS9);
            this->methods->addTranslation(this, TRANSLATE_Y_PLUS8);
        } else {
            u32 r;

            out->slots[0].program = 0x12;
            out->slots[1].program = 3;
            this->methods->stopSoundCue(this);
            r = rand() & 1;
            this->state = r < 1;
        }
    }
}

void Entity__MoodCue69(Entity *this, SoundCueSet *out) {
    out->attenuation = this->methods->getProximityRatio(this);
    if (this->todFrame == this->todFrameCount - 1) {
        out->slots[0].program = 0x19;
        out->slots[0].octave = -2;
    }
    if (out->tick % 4 == 0) {
        out->slots[1].program = 0x15;
        out->slots[1].octave = -1;
    }
    if (out->tick % 200 == 0) {
        out->slots[2].program = 0xD;
        out->slots[2].octave = 1;
    }
}

void Entity__MoodCue70(Entity *this, SoundCueSet *out) {
    if (this->moodTimer == 0) {
        if (rand() & 1) {
            this->state = 0xB;
        }
    }
    if (this->moodTimer == 0x12C) {
        this->methods->updateRotation(this, 0, ROTATION_YAW_PLUS180);
    }
    if (this->moodTimer < 0x258) {
        this->methods->moveLocalZ(this, this->state == 0 ? -0x100 : 0x100, 0);
    }
}

void Entity__MoodCue71(Entity *this, SoundCueSet *out) {
    s32 r;

    out->attenuation = this->methods->getProximityRatio(this);
    if (out->tick == 0) {
        out->slots[0].program = 0;
        r = rand() % 3;
        this->methods->moveLocalX(this, r * 51200, 0);
    }
    if (this->moodTimer >= 0x961) {
        SceneNode__FaceTarget((SceneNode *)this, (SceneNode *)this->peer, 1, 0, 0);
    }
    this->methods->moveLocalZ(this, -0x1E, 0);
}

void Entity__MoodCue73(Entity *this, SoundCueSet *out) {
    out->attenuation = 0;
    if (out->tick == 0) {
        ((DreamSys *)this->peer)->methods->updateRotation((DreamSys *)this->peer, 1, ROTATION_YAW_PLUS90);
        ((DreamSys *)this->peer)->methods->clearTickCallbacks((DreamSys *)this->peer, 1);
        out->slots[0].program = 0x19;
        out->slots[1].program = 0x19;
        out->slots[2].program = 0x19;
    } else if (out->tick == 0x14) {
        out->slots[1].program = 0xD;
    }
    if (this->moodTimer == this->todFrameCount - 1) {
        this->methods->deactivate(this);
    }
}

extern u8 D_8008AC1C[];

void Entity__MoodCue74(Entity *this, SoundCueSet *out) {
    if (this->moodTimer == 0) {
        ((DreamSys *)this->peer)
            ->viewport->methods->setClearColor(((DreamSys *)this->peer)->viewport,
                                               (ViewportRgb *)D_8008AC1C);
        this->state = rand() % 3;
        if (((DreamSys *)this->peer)->coord2->tz < 0x262) {
            this->state = 0;
        }
    }
    if (this->state != 0) {
        if (this->todFrameCount / 2 < this->moodTimer) {
            ((DreamSys *)this->peer)->methods->moveLocalZ((DreamSys *)this->peer, 0x80, 0);
        }
        if (this->moodTimer == this->todFrameCount - 0x1E) {
            this->methods->notifyParents(this, 0xA);
        }
    } else {
        if ((u32)(this->moodTimer - 0x14) < 0x64) {
            ((DreamSys *)this->peer)
                ->methods->moveLocalZ((DreamSys *)this->peer, -((this->moodTimer - 0x13) * 0x20),
                                      (void *)1);
            if (this->moodTimer == 0x55) {
                ((DreamSys *)this->peer)->methods->setTickCallbacks((DreamSys *)this->peer, 1, 1);
            }
        }
    }
}

void Entity__MoodCue75(Entity *this, SoundCueSet *out) {
    if (out->tick == 0) {
        out->attenuation = 0;
        out->slots[0].program = 0x19;
        out->slots[1].program = 0x19;
        out->slots[2].program = 0x19;
        SceneNode__FaceTarget((SceneNode *)this, (SceneNode *)this->peer, 1, 0, 0);
    }
    if (this->moodTimer == this->todFrameCount) {
        this->methods->stopTod(this);
        this->methods->notifyParents(this, 0xA);
    }
}

extern u8 SCALE_MINUS_SIXTY_FOURTH[];

void Entity__MoodCue76(Entity *this, SoundCueSet *out) {
    void (*fn)(Entity *self, s32 set, void *table);
    void *table;

    if (this->targetReached != 0) {
        ((EntityPlayTodFn)this->methods->playTod)(this);
        if (this->todFrame == this->todFrameCount - 1) {
            this->methods->stopTod(this);
            fn = (void (*)(Entity *, s32, void *))this->methods->updateScale;
            table = SCALE_MINUS_SIXTY_FOURTH;
            fn(this, 0, table);
        }
    } else {
        this->methods->stopTod(this);
        fn = this->methods->updateRotation;
        table = ROTATION_YAW_PLUS9;
        fn(this, 0, table);
    }
}

void Entity__MoodCue77(Entity *this, SoundCueSet *out) {
    out->attenuation = this->methods->getProximityRatio(this);
    if (out->tick % 5 == 0) {
        out->slots[0].program = 0x11;
        out->slots[0].octave = -2;
    }
    if (this->todIndex == 0) {
        if (this->moodTimer == this->todFrameCount) {
            this->methods->setTod(this, 1);
            if (rand() & 1) {
                this->state = 0xB;
            }
        }
        return;
    }
    if (this->state == 0) {
        if (this->moodTimer == 0x3C || this->moodTimer == 0xD4 || this->moodTimer == 0x122 ||
            this->moodTimer == 0x140) {
            this->methods->updateRotation(this, 0, ROTATION_YAW_PLUS90);
        }
        if (this->moodTimer == 0x18E) {
            this->methods->updateRotation(this, 0, ROTATION_YAW_MINUS90);
        }
        this->methods->moveLocalZOrFindLink(this, -0x32, 0);
        return;
    }
    if (this->moodTimer == 0x3C || this->moodTimer == 0x8C) {
        this->methods->updateRotation(this, 0, ROTATION_YAW_PLUS90);
    }
    if (this->moodTimer < 0xAE) {
        this->methods->moveLocalZOrFindLink(this, -0x32, 0);
    }
    if (this->moodTimer == 0xAE) {
        this->methods->stopSoundCue(this);
        this->state = 1;
    }
}

extern u8 D_80089E14[];
extern s32 sMoodCue78TransitionDone;

void Entity__MoodCue78(Entity *this, SoundCueSet *out) {
    s32 tmp;
    void *table;

    if (out->tick == 0) {
        sMoodCue78TransitionDone = 0;
        tmp = rand() % 3;
        if (tmp == 1) {
            this->state = 0xB;
        }
        if (tmp == 2) {
            this->state = 0xC;
        }
    }

    out->attenuation = this->methods->getProximityRatio(this);

    if (this->todPlaying != 0 && (out->tick & 3) == 0) {
        out->slots[0].program = 0x1C;
    }

    if (this->moodTimer == this->todFrameCount - 1) {
        this->moodTimer = -1;
    } else {
        if (this->moodTimer >= this->todFrameCount / 2 + this->todFrameCount / 4) {
            ((EntityPlayTodFn)this->methods->playTod)(this);
        } else if (this->moodTimer >= this->todFrameCount / 2) {
            if (this->moodTimer == this->todFrameCount / 2) {
                out->slots[0].program = 0x10;
            }
            this->methods->moveLocalZ(this, 0x6E, 0);
        } else if (this->moodTimer < this->todFrameCount / 4) {
            /* nothing */
        } else {
            this->methods->stopTod(this);
            this->methods->moveLocalZ(this, -0x6E, 0);
        }
    }

    if (this->state == 0xB && out->tick == 0x1FE) {
        this->methods->moveLocalY(this, -0x17C, 0);
        this->methods->updateRotation(this, 0, ROTATION_XPLUS90);
        this->methods->stopSoundCue(this);
        this->state = 1;
        sMoodCue78TransitionDone = 1;
    } else if (this->state >= 0xC && out->tick >= 0x14A && (out->tick % 60) == 30) {
        tmp = 0;
        if (rand() & 1) {
            table = SCALE_Y2;
            tmp = (this->state == 0xC) ? 0x190 : 0;
            this->state = 0xD;
        } else {
            table = D_80089E14;
            if (this->state == 0xD) {
                tmp = -0x190;
            }
            this->state = 0xC;
        }
        this->methods->updateScale(this, 1, table);
        this->methods->moveLocalY(this, tmp, 0);
    }

    if (out->tick == 0x208 && sMoodCue78TransitionDone != 0) {
        this->methods->stopSoundCue(this);
        this->state = 1;
    }
}

void Entity__MoodCue79(Entity *this, SoundCueSet *out) {
    out->attenuation = this->methods->getProximityRatio(this);
    if (out->tick % 10 == 0) {
        out->slots[0].program = 0x19;
        out->slots[0].octave = 2;
    }
    this->methods->updateRotation(this, 0, ROTATION_YAW_PLUS2);
    if (this->state == 0 && this->targetReached != 0) {
        this->methods->notifyParents(this, 0xB);
        this->state = 0xB;
    }
}

void Entity__MoodCue80(Entity *this, SoundCueSet *out) {
    if (this->moodTimer < this->todFrameCount) {
        if (this->todFrame != 0) {
            if (this->todFrame == 0x14) {
                out->attenuation = 0;
                out->slots[0].program = 0x10;
            }
        }
    } else {
        this->methods->stopTod(this);
        this->methods->addTranslation(this, TRANSLATE_Y_MINUS512);
    }
    SceneNode__FaceTarget((SceneNode *)this, (SceneNode *)this->peer, 1, 0, 0);
}

extern u8 D_80089E08[];

void Entity__MoodCue81(Entity *this, SoundCueSet *out) {
    s32 mod;
    s32 mood;

    if (this->moodTimer == 0 && (rand() & 1)) {
        this->state = rand() % 3;
    }
    if (this->state != 0 && this->targetReached != 0) {
        if (out->tick >= 0x3D) {
            out->tick = 0;
        }
        if (out->tick % 20 == 0) {
            out->slots[0].program = 0x17;
            out->attenuation = 0;
            out->slots[0].octave = -1;
            out->slots[2].program = out->slots[1].program = 0x17;
            out->slots[1].octave = -1;
            out->slots[2].octave = -2;
        } else if (out->tick % 20 == 0xE) {
            out->slots[0].program = -2;
            out->slots[1].program = -2;
            out->slots[2].program = -2;
        }
        SceneNode__FaceTarget((SceneNode *)this, (SceneNode *)this->peer, 1, 0, 0);
        mood = this->state;
        if (mood == 1) {
            this->methods->updateScale(this, 0, D_80089E08);
            mod = -0x176;
            if (this->methods->distanceToPeer(this, this->peer) < 0x200) {
                this->methods->deactivate(this);
                this->state = mood;
            }
        } else {
            ((DreamSys *)this->peer)->methods->clearTickCallbacks((DreamSys *)this->peer, 1);
            SceneNode__FaceTarget((SceneNode *)this->peer, (SceneNode *)this, 1, 1, 0);
            if (this->state == 2) {
                if (this->methods->distanceToPeer(this, this->peer) < 0x960) {
                    this->state = 0xB;
                    this->methods->notifyParents(this, 0xC);
                }
                mod = -0x60;
            } else {
                mod = 0;
            }
        }
    } else {
        out->attenuation = this->methods->getProximityRatio(this);
        if (out->tick % 22 == 0) {
            out->slots[0].program = 0x1C;
        }
        if (this->moodTimer >= 0x1F5) {
            SceneNode__FaceTarget((SceneNode *)this, (SceneNode *)this->peer, 1, 0, 0);
        }
        if (this->methods->distanceToPeer(this, this->peer) < 0x800) {
            ((DreamSys *)this->peer)->methods->moveLocalZ((DreamSys *)this->peer, -0x800, 0);
        }
        mod = -0x14;
    }
    this->methods->moveLocalZOrFindLink(this, mod, (void *)1);
    if (this->linkTarget != 0) {
        this->methods->moveLocalY(this, -0xC8, 0);
    }
}
