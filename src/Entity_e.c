/* Entity_e: twenty of Entity's MoodCue handlers.
 *
 * Each Entity__MoodCueNN is the `handler` of gEntityMoodHandlerTable's row
 * NN (include/Entity.h): rows 59, 61, 62, 64 to 71 and 73 to 81, and
 * Entity__MoodCue81 is row 120's handler too (the row's data words
 * differ). Rows 60, 63 and 72 have no handler. An Entity whose moodIndex
 * selects the row installs it as its SoundCueSet callback, so
 * ServiceSoundCueSet calls it once per tick with the Entity and its cue
 * set. A handler requests tones by filling the set's slots (a VAB program
 * of the cue's sound object, or SOUND_CUE_STOP), moves and turns the
 * entity (or the player, its `peer`) on moodTimer, the ticks since
 * startSoundCue, on the cue set's own `tick`, or on todFrame, the frame of
 * its TOD animation, and sends the dream an EntityEffect through
 * notifyParents. Entity__MoodCue108 (Entity_g) runs Entity__MoodCue71 and
 * then sets its scale to SCALE_SIX.
 *
 * The literals are left unnamed where they are one handler's tuning: tick
 * counts, distances in world units, TOD frame numbers, VAB program numbers,
 * and the `state` values other than 0 and ENTITY_STATE_DONE, which are each
 * handler's own phases.
 */
#include "common.h"
#include "Entity.h"
#include "DreamSys.h"
#include "Class866E8.h"
#include "Viewport.h"

void Entity__MoodCue59(Entity *this, SoundCueSet *out) {
    if (this->moodTimer == 0 && rand() % 10 == 0) {
        this->state = 12;
    }
    if (out->tick % 10 == 0) {
        out->attenuation = this->methods->getProximityRatio(this);
        out->slots[0].program = 12;
        out->slots[0].octave = -1;
    }
    if (this->moodTimer == 0) {
        if (rand() & 1) {
            this->methods->moveLocalY(this, 2048, 0);
        }
    }
    this->methods->moveLocalZ(this, -128, 0);
    if (this->state == 12 && this->moodTimer == 300) {
        this->grid->methods->configureRateEntry(this->grid, 1, 1);
    }
}

void Entity__MoodCue61(Entity *this, SoundCueSet *out) {
    if (this->todFrame == 30) {
        out->slots[0].program = 18;
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
    if (this->moodTimer >= 101) {
        SceneNode__FaceTarget((SceneNode *)this, (SceneNode *)this->peer, 1, 0, 0);
    }
    this->methods->moveLocalZ(this, -5, 0);
    if (this->moodTimer == 300 && this->methods->distanceToPeer(this, this->peer) < 4096) {
        ((DreamSys *)this->peer)->methods->clearTickCallbacks((DreamSys *)this->peer, false);
    } else if (this->moodTimer == 500) {
        ((DreamSys *)this->peer)
            ->methods->setTickCallbacks((DreamSys *)this->peer, MOVE_CALLBACK_TICK_MOVE,
                                        LOOK_CALLBACK_STEP_LOOK);
    }
    if (this->state == 0) {
        if (this->methods->distanceToPeer(this, this->peer) < 1024) {
            if (rand() & 1) {
                out->slots[1].program = 6;
                out->attenuation = 0;
                out->slots[1].octave = -1;
                if (rand() & 1) {
                    this->grid->methods->configureRateEntry(this->grid, -1, 0);
                }
                this->moodTimer = 0;
                this->state = 10;
            } else {
                this->state = 11;
            }
        }
    }
    if (this->state == 10 && this->moodTimer == 70) {
        this->methods->notifyParents(this, (rand() & 1) ? ENTITY_EFFECT_END_DREAM
                                                        : ENTITY_EFFECT_EVENT_VIDEO);
    }
}

void Entity__MoodCue64(Entity *this, SoundCueSet *out) {
    s32 phase = out->tick % 300;

    out->attenuation = this->methods->getProximityRatio(this);
    if (phase < 20) {
        out->slots[0].program = 5;
        out->slots[0].octave = -2;
    } else if (phase == 22) {
        out->slots[0].program = SOUND_CUE_STOP;
    }
    this->methods->moveLocalZ(this, -10, 0);
}

void Entity__MoodCue65(Entity *this, SoundCueSet *out) {
    if (this->moodTimer == 0) {
        if (rand() % 3 == 0) {
            this->methods->updateScale(this, 1, SCALE_HALF);
            this->methods->moveLocalY(this, -300, 0);
            this->methods->updateRotation(this, 1, ROTATION_YAW_PLUS90);
            this->state = 11;
        }
    }
    if (this->state == 11) {
        if (this->moodTimer == 2000) {
            this->methods->updateRotation(this, 0, ROTATION_YAW_MINUS90);
        }
        this->methods->moveLocalZ(this, -20, 0);
    }
}

void Entity__MoodCue66(Entity *this, SoundCueSet *out) {
    out->attenuation = this->methods->getProximityRatio(this);
    if (out->tick % 30 == 0) {
        out->slots[0].program = 13;
    }
}

void Entity__MoodCue67(Entity *this, SoundCueSet *out) {
    if (this->moodTimer == 0) {
        if (rand() % 3 == 0) {
            this->state = 11;
        }
    }
    if (this->state == 11) {
        if (this->moodTimer == 502) {
            this->methods->moveLocalY(this, 2048, 0);
            SceneNode__FaceTarget((SceneNode *)this, (SceneNode *)this->peer, 1, 0, 0);
        }
        if (this->moodTimer >= 501) {
            this->methods->moveLocalZ(this, -512, 0);
        }
    }
}

void Entity__MoodCue68(Entity *this, SoundCueSet *out) {
    out->attenuation = this->methods->getProximityRatio(this);
    if (out->tick == 0) {
        this->lastOffsetValue = (rand() & 1) ? -374 : -192;
    }
    if (this->state == 0) {
        if (out->tick % 10 == 0) {
            out->slots[0].program = 28;
        }
        if (out->tick % 20 == 0) {
            out->slots[1].program = 23;
            out->slots[1].octave = -1;
            out->slots[2].program = 23;
            out->slots[2].octave = -1;
        } else if (out->tick % 20 == 14) {
            out->slots[1].program = SOUND_CUE_STOP;
            out->slots[2].program = SOUND_CUE_STOP;
        }
        if ((this->moodTimer & 1) == 0) {
            if (((DreamSys *)this->peer)->methods->getLinkCommandFlag((DreamSys *)this->peer) != 0) {
                this->moodTimer = -1;
                this->state = 10;
                out->slots[0].program = 18;
            }
        }
        SceneNode__FaceTarget((SceneNode *)this, (SceneNode *)this->peer, 1, 0, 0);
        this->methods->moveLocalZ(this, this->lastOffsetValue, (void *)1);
    } else if (this->state == 10) {
        if (this->moodTimer < 8) {
            this->methods->updateRotation(this, 0, ROTATION_ZPLUS9);
            this->methods->addTranslation(this, TRANSLATE_Y_PLUS8);
        } else {
            u32 coin;

            out->slots[0].program = 18;
            out->slots[1].program = 3;
            this->methods->stopSoundCue(this);
            coin = rand() & 1;
            /* Half the time ENTITY_STATE_DONE, else back to phase 0. */
            this->state = coin < 1;
        }
    }
}

void Entity__MoodCue69(Entity *this, SoundCueSet *out) {
    out->attenuation = this->methods->getProximityRatio(this);
    if (this->todFrame == this->todFrameCount - 1) {
        out->slots[0].program = 25;
        out->slots[0].octave = -2;
    }
    if (out->tick % 4 == 0) {
        out->slots[1].program = 21;
        out->slots[1].octave = -1;
    }
    if (out->tick % 200 == 0) {
        out->slots[2].program = 13;
        out->slots[2].octave = 1;
    }
}

void Entity__MoodCue70(Entity *this, SoundCueSet *out) {
    if (this->moodTimer == 0) {
        if (rand() & 1) {
            this->state = 11;
        }
    }
    if (this->moodTimer == 300) {
        this->methods->updateRotation(this, 0, ROTATION_YAW_PLUS180);
    }
    if (this->moodTimer < 600) {
        this->methods->moveLocalZ(this, this->state == 0 ? -256 : 256, 0);
    }
}

void Entity__MoodCue71(Entity *this, SoundCueSet *out) {
    s32 lane;

    out->attenuation = this->methods->getProximityRatio(this);
    if (out->tick == 0) {
        out->slots[0].program = 0;
        lane = rand() % 3;
        this->methods->moveLocalX(this, lane * 51200, 0);
    }
    if (this->moodTimer >= 2401) {
        SceneNode__FaceTarget((SceneNode *)this, (SceneNode *)this->peer, 1, 0, 0);
    }
    this->methods->moveLocalZ(this, -30, 0);
}

void Entity__MoodCue73(Entity *this, SoundCueSet *out) {
    out->attenuation = 0;
    if (out->tick == 0) {
        ((DreamSys *)this->peer)->methods->updateRotation((DreamSys *)this->peer, 1, ROTATION_YAW_PLUS90);
        ((DreamSys *)this->peer)->methods->clearTickCallbacks((DreamSys *)this->peer, true);
        out->slots[0].program = 25;
        out->slots[1].program = 25;
        out->slots[2].program = 25;
    } else if (out->tick == 20) {
        out->slots[1].program = 13;
    }
    if (this->moodTimer == this->todFrameCount - 1) {
        this->methods->deactivate(this);
    }
}

/* {0, 100, 190}: the clear colour Entity__MoodCue74 gives the peer's viewport. */
extern ViewportRgb sMoodCue74ClearColor;

void Entity__MoodCue74(Entity *this, SoundCueSet *out) {
    if (this->moodTimer == 0) {
        ((DreamSys *)this->peer)
            ->viewport->methods->setClearColor(((DreamSys *)this->peer)->viewport, &sMoodCue74ClearColor);
        this->state = rand() % 3;
        if (((DreamSys *)this->peer)->coord2->tz < 610) {
            this->state = 0;
        }
    }
    if (this->state != 0) {
        if (this->todFrameCount / 2 < this->moodTimer) {
            ((DreamSys *)this->peer)->methods->moveLocalZ((DreamSys *)this->peer, 128, 0);
        }
        if (this->moodTimer == this->todFrameCount - 30) {
            this->methods->notifyParents(this, ENTITY_EFFECT_LINK_STAGE);
        }
    } else {
        if (this->moodTimer >= 20 && this->moodTimer < 120) {
            ((DreamSys *)this->peer)
                ->methods->moveLocalZ((DreamSys *)this->peer, -((this->moodTimer - 19) * 32), (void *)1);
            if (this->moodTimer == 85) {
                ((DreamSys *)this->peer)
                    ->methods->setTickCallbacks((DreamSys *)this->peer, MOVE_CALLBACK_TICK_MOVE,
                                                LOOK_CALLBACK_STEP_LOOK);
            }
        }
    }
}

void Entity__MoodCue75(Entity *this, SoundCueSet *out) {
    if (out->tick == 0) {
        out->attenuation = 0;
        out->slots[0].program = 25;
        out->slots[1].program = 25;
        out->slots[2].program = 25;
        SceneNode__FaceTarget((SceneNode *)this, (SceneNode *)this->peer, 1, 0, 0);
    }
    if (this->moodTimer == this->todFrameCount) {
        this->methods->stopTod(this);
        this->methods->notifyParents(this, ENTITY_EFFECT_LINK_STAGE);
    }
}

void Entity__MoodCue76(Entity *this, SoundCueSet *out) {
    if (this->targetReached != 0) {
        ((EntityPlayTodFn)this->methods->playTod)(this);
        if (this->todFrame == this->todFrameCount - 1) {
            this->methods->stopTod(this);
            this->methods->updateScale(this, 0, SCALE_MINUS_SIXTY_FOURTH);
        }
    } else {
        this->methods->stopTod(this);
        this->methods->updateRotation(this, 0, ROTATION_YAW_PLUS9);
    }
}

void Entity__MoodCue77(Entity *this, SoundCueSet *out) {
    out->attenuation = this->methods->getProximityRatio(this);
    if (out->tick % 5 == 0) {
        out->slots[0].program = 17;
        out->slots[0].octave = -2;
    }
    if (this->todIndex == 0) {
        if (this->moodTimer == this->todFrameCount) {
            this->methods->setTod(this, 1);
            if (rand() & 1) {
                this->state = 11;
            }
        }
        return;
    }
    if (this->state == 0) {
        if (this->moodTimer == 60 || this->moodTimer == 212 || this->moodTimer == 290 ||
            this->moodTimer == 320) {
            this->methods->updateRotation(this, 0, ROTATION_YAW_PLUS90);
        }
        if (this->moodTimer == 398) {
            this->methods->updateRotation(this, 0, ROTATION_YAW_MINUS90);
        }
        this->methods->moveLocalZOrFindLink(this, -50, 0);
        return;
    }
    if (this->moodTimer == 60 || this->moodTimer == 140) {
        this->methods->updateRotation(this, 0, ROTATION_YAW_PLUS90);
    }
    if (this->moodTimer < 174) {
        this->methods->moveLocalZOrFindLink(this, -50, 0);
    }
    if (this->moodTimer == 174) {
        this->methods->stopSoundCue(this);
        this->state = ENTITY_STATE_DONE;
    }
}

/* {1/1, 1/1, 1/1}: the identity scale, a copy in .data of the values
 * code_d294.h's SCALE_ONE holds in .rodata. Entity_g passes it too. */
extern u8 D_80089E14[];
/* Entity__MoodCue78's: cleared on the cue's first tick, set when its phase
 * 11 ends the cue at tick 510; at tick 520 a set flag ends it again. */
extern s32 sMoodCue78TransitionDone;

void Entity__MoodCue78(Entity *this, SoundCueSet *out) {
    /* MATCHING: one local for the roll and then the y move; two allocate
     * differently. */
    s32 rollOrDy;
    void *table;

    if (out->tick == 0) {
        sMoodCue78TransitionDone = 0;
        rollOrDy = rand() % 3;
        if (rollOrDy == 1) {
            this->state = 11;
        }
        if (rollOrDy == 2) {
            this->state = 12;
        }
    }

    out->attenuation = this->methods->getProximityRatio(this);

    if (this->todPlaying != 0 && (out->tick & 3) == 0) {
        out->slots[0].program = 28;
    }

    if (this->moodTimer == this->todFrameCount - 1) {
        this->moodTimer = -1;
    } else {
        if (this->moodTimer >= this->todFrameCount / 2 + this->todFrameCount / 4) {
            ((EntityPlayTodFn)this->methods->playTod)(this);
        } else if (this->moodTimer >= this->todFrameCount / 2) {
            if (this->moodTimer == this->todFrameCount / 2) {
                out->slots[0].program = 16;
            }
            this->methods->moveLocalZ(this, 110, 0);
        } else if (this->moodTimer < this->todFrameCount / 4) {
            /* nothing */
        } else {
            this->methods->stopTod(this);
            this->methods->moveLocalZ(this, -110, 0);
        }
    }

    if (this->state == 11 && out->tick == 510) {
        this->methods->moveLocalY(this, -380, 0);
        this->methods->updateRotation(this, 0, ROTATION_XPLUS90);
        this->methods->stopSoundCue(this);
        this->state = ENTITY_STATE_DONE;
        sMoodCue78TransitionDone = 1;
    } else if (this->state >= 12 && out->tick >= 330 && (out->tick % 60) == 30) {
        rollOrDy = 0;
        if (rand() & 1) {
            table = SCALE_Y2;
            rollOrDy = (this->state == 12) ? 400 : 0;
            this->state = 13;
        } else {
            table = D_80089E14;
            if (this->state == 13) {
                rollOrDy = -400;
            }
            this->state = 12;
        }
        this->methods->updateScale(this, 1, table);
        this->methods->moveLocalY(this, rollOrDy, 0);
    }

    if (out->tick == 520 && sMoodCue78TransitionDone != 0) {
        this->methods->stopSoundCue(this);
        this->state = ENTITY_STATE_DONE;
    }
}

void Entity__MoodCue79(Entity *this, SoundCueSet *out) {
    out->attenuation = this->methods->getProximityRatio(this);
    if (out->tick % 10 == 0) {
        out->slots[0].program = 25;
        out->slots[0].octave = 2;
    }
    this->methods->updateRotation(this, 0, ROTATION_YAW_PLUS2);
    if (this->state == 0 && this->targetReached != 0) {
        this->methods->notifyParents(this, ENTITY_EFFECT_EVENT_VIDEO);
        this->state = 11;
    }
}

void Entity__MoodCue80(Entity *this, SoundCueSet *out) {
    if (this->moodTimer < this->todFrameCount) {
        if (this->todFrame != 0) {
            if (this->todFrame == 20) {
                out->attenuation = 0;
                out->slots[0].program = 16;
            }
        }
    } else {
        this->methods->stopTod(this);
        this->methods->addTranslation(this, TRANSLATE_Y_MINUS512);
    }
    SceneNode__FaceTarget((SceneNode *)this, (SceneNode *)this->peer, 1, 0, 0);
}

void Entity__MoodCue81(Entity *this, SoundCueSet *out) {
    s32 dz;
    s32 state;

    if (this->moodTimer == 0 && (rand() & 1)) {
        this->state = rand() % 3;
    }
    if (this->state != 0 && this->targetReached != 0) {
        if (out->tick >= 61) {
            out->tick = 0;
        }
        if (out->tick % 20 == 0) {
            out->slots[0].program = 23;
            out->attenuation = 0;
            out->slots[0].octave = -1;
            out->slots[2].program = out->slots[1].program = 23;
            out->slots[1].octave = -1;
            out->slots[2].octave = -2;
        } else if (out->tick % 20 == 14) {
            out->slots[0].program = SOUND_CUE_STOP;
            out->slots[1].program = SOUND_CUE_STOP;
            out->slots[2].program = SOUND_CUE_STOP;
        }
        SceneNode__FaceTarget((SceneNode *)this, (SceneNode *)this->peer, 1, 0, 0);
        state = this->state;
        if (state == 1) {
            this->methods->updateScale(this, 0, SCALE_EIGHT_SEVENTHS);
            dz = -374;
            if (this->methods->distanceToPeer(this, this->peer) < 512) {
                this->methods->deactivate(this);
                this->state = state;
            }
        } else {
            ((DreamSys *)this->peer)->methods->clearTickCallbacks((DreamSys *)this->peer, true);
            SceneNode__FaceTarget((SceneNode *)this->peer, (SceneNode *)this, 1, 1, 0);
            if (this->state == 2) {
                if (this->methods->distanceToPeer(this, this->peer) < 2400) {
                    this->state = 11;
                    this->methods->notifyParents(this, ENTITY_EFFECT_END_DREAM);
                }
                dz = -96;
            } else {
                dz = 0;
            }
        }
    } else {
        out->attenuation = this->methods->getProximityRatio(this);
        if (out->tick % 22 == 0) {
            out->slots[0].program = 28;
        }
        if (this->moodTimer >= 501) {
            SceneNode__FaceTarget((SceneNode *)this, (SceneNode *)this->peer, 1, 0, 0);
        }
        if (this->methods->distanceToPeer(this, this->peer) < 2048) {
            ((DreamSys *)this->peer)->methods->moveLocalZ((DreamSys *)this->peer, -2048, 0);
        }
        dz = -20;
    }
    this->methods->moveLocalZOrFindLink(this, dz, (void *)1);
    if (this->linkTarget != 0) {
        this->methods->moveLocalY(this, -200, 0);
    }
}
