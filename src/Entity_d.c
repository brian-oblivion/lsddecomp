/* Entity_d: nineteen of Entity's MoodCue handlers and the helper two of
 * them share.
 *
 * Each Entity__MoodCueNN is the `handler` of gEntityMoodHandlerTable's row
 * NN (include/Entity.h): rows 39 to 52, 55 to 58 and 115. An Entity whose
 * moodIndex selects the row installs it as its SoundCueSet callback, so
 * ServiceSoundCueSet calls it once per tick with the Entity and its cue
 * set. A handler requests tones by filling the set's slots (a VAB program
 * of the cue's sound object, or SOUND_CUE_STOP), moves and turns the
 * entity (or the player, its `peer`) on moodTimer, the ticks since
 * startSoundCue, or on todFrame, the frame of its TOD animation, and sends
 * the dream an EntityEffect through notifyParents. Entity__MoodCue45 is
 * empty: its row has no per-tick effect.
 *
 * Entity__RollScaleOrDelayedDrift is not a row: Entity__MoodCue43 and
 * Entity__MoodCue44 call it first thing every tick.
 *
 * The literals are left unnamed where they are one handler's tuning: tick
 * counts, distances in world units, TOD frame numbers, VAB program numbers,
 * and the `state` values other than 0 and ENTITY_STATE_DONE, which are each
 * handler's own phases.
 */
#include "common.h"
#include "Entity.h"
#include "DreamSys.h"

/* The z den of a scale template, three Ratio16s {1/1, 1/1, 1/zDenom} that
 * end here (the range splat labels SCALE_X3 runs on into its first ten
 * bytes). Entity__MoodCue41 writes the den and passes the template. */
extern s16 sScaleTemplateZDenom;

/* The Ratio16[3] rotations (degrees) and scales the handlers pass to
 * updateRotation and updateScale, and the offset they pass to
 * addTranslation. */
extern Ratio16 ROTATION_YAW_PLUS9[];
extern Ratio16 ROTATION_YAW_MINUS9[];
extern Ratio16 SCALE_X3[];
extern LongVec3 TRANSLATE_Z_MINUS256[];
extern Ratio16 SCALE_Y2[];
extern Ratio16 SCALE_SIX[];
extern Ratio16 ROTATION_YAW_PLUS180[];
extern Ratio16 ROTATION_YAW_MINUS90[];
extern Ratio16 ROTATION_YAW_PLUS90[];
extern Ratio16 SCALE_Y4[];
extern Ratio16 ROTATION_ZMINUS90[];

/* Defined after Entity__MoodCue43, which calls it. */
void Entity__RollScaleOrDelayedDrift(Entity *this);

void Entity__MoodCue39(Entity *this, SoundCueSet *out) {
    s32 dayYearPhase;

    if (this->moodTimer == 0) {
        dayYearPhase =
            ((DreamSys *)this->peer)->methods->getCurrentDayAndYear((DreamSys *)this->peer, 0) % 3;
        if (dayYearPhase == 0) {
            if (rand() % 3 != 0) {
                goto skipScaleBump;
            }
        } else if (dayYearPhase != 2) {
            goto skipScaleBump;
        }
        this->methods->updateScale(this, 1, SCALE_Y4);
    }
skipScaleBump:
    if (out->tick % 22 == 0) {
        out->attenuation = this->methods->getProximityRatio(this);
        out->slots[0].program = 2;
    }
    if (rand() % 12 == 0) {
        this->methods->stopTod(this);
    } else if (rand() % 6 == 0) {
        ((EntityPlayTodFn)this->methods->playTod)(this);
    }
}

void Entity__MoodCue40(Entity *this, SoundCueSet *out) {
    Ratio16 *table = NULL;

    if (out->tick % 7 == 0) {
        out->attenuation = this->methods->getProximityRatio(this);
        out->slots[0].program = 3;
        out->slots[0].vol = 64;
        out->slots[0].endVol = 64;
    }
    if (this->moodTimer == 200) {
        table = ROTATION_YAW_MINUS90;
    } else if (this->moodTimer == 400) {
        table = ROTATION_YAW_PLUS180;
    } else if (this->moodTimer == 600) {
        table = ROTATION_YAW_PLUS90;
    } else if (this->moodTimer == 800) {
        table = ROTATION_YAW_PLUS180;
        this->moodTimer = -1;
    }
    if (table != NULL) {
        this->methods->updateRotation(this, 0, table);
    }
    this->methods->moveLocalZOrFindLink(this, -30, 0);
    if (this->linkTarget != 0) {
        this->methods->moveLocalY(this, -200, 0);
    }
}

void Entity__MoodCue41(Entity *this, SoundCueSet *out) {
    s32 roll;
    s16 *zDenom;

    if (this->moodTimer == 0) {
        this->state = rand() % 5 + 10;
    }
    if (this->state < 14 || this->moodTimer < 320) {
        Entity__StepYawInWindowsThenDeactivate(this, out, 3000, 500, -256);
        return;
    }
    if (this->state == 14) {
        if ((this->moodTimer & 3) == 0) {
            roll = rand();
            zDenom = &sScaleTemplateZDenom;
            *zDenom = roll % 32 + 1;
            /* Back from the den to the start of its template. MATCHING:
             * retail relocates against sScaleTemplateZDenom, not SCALE_X3. */
            this->methods->updateScale(this, 1, (Ratio16 *)(zDenom + 1) - 3);
        }
    }
}

void Entity__MoodCue42(Entity *this, SoundCueSet *out) {
    s32 divisor;

    if (this->moodTimer < 20) {
        this->methods->stopTod(this);
        this->methods->moveLocalZ(this, -30, 0);
    } else if (this->moodTimer == 20) {
        ((EntityPlayTodFn)this->methods->playTod)(this);
        out->attenuation = 0;
        out->slots[0].program = 5;
    } else {
        divisor = this->todFrameCount * 3 + 20;
        if (this->moodTimer % divisor == 0) {
            this->methods->stopTod(this);
            out->slots[0].program = SOUND_CUE_STOP;
        }
    }
}

void Entity__MoodCue43(Entity *this, SoundCueSet *out) {
    s32 dx;
    s32 rotPick;
    Ratio16 *table;

    Entity__RollScaleOrDelayedDrift(this);
    out->attenuation = this->methods->getProximityRatio(this);
    if (this->todFrame == 0 || this->todFrame == 15) {
        out->slots[0].program = 18;
        out->slots[1].program = 18;
    }
    if (this->moodTimer >= 321) {
        dx = (rand() & 1) ? -60 : 60;
        this->methods->moveLocalX(this, dx, 0);
        rotPick = rand();
        table = ROTATION_YAW_PLUS9;
        if ((rotPick & 3) != 0) {
            table = ROTATION_YAW_MINUS9;
        }
        this->methods->updateRotation(this, 0, table);
    }
}

void Entity__MoodCue44(Entity *this, SoundCueSet *out) {
    s32 dx;
    s32 dxPick;
    s32 rotPick;
    Ratio16 *table;

    Entity__RollScaleOrDelayedDrift(this);
    out->attenuation = this->methods->getProximityRatio(this);
    if (this->todFrame == 7 || this->todFrame == 22) {
        out->slots[0].program = 3;
    }
    if (this->moodTimer >= 300 && this->moodTimer < 320) {
        this->methods->moveLocalZ(this, -60, 0);
    } else if (this->moodTimer >= 321 && this->moodTimer < 340) {
        this->methods->updateRotation(this, 0, ROTATION_YAW_MINUS9);
    } else if (this->moodTimer >= 321) {
        dxPick = rand();
        dx = -128;
        if ((dxPick & 1) != 0) {
            dx = 128;
        }
        this->methods->moveLocalX(this, dx, (void *)1);
        rotPick = rand();
        table = ROTATION_YAW_PLUS9;
        if ((rotPick & 3) != 0) {
            table = ROTATION_YAW_MINUS9;
        }
        this->methods->updateRotation(this, 0, table);
    }
}

/* At the cue's start, rolls 0..9: 8 or 9 stretches the entity by
 * SCALE_X3, 5 to 7 arms a drift (state 10) that adds TRANSLATE_Z_MINUS256
 * every tick from tick 201 on. */
void Entity__RollScaleOrDelayedDrift(Entity *this) {
    s32 roll;

    if (this->moodTimer == 0) {
        roll = rand() % 10;
        if (roll >= 8) {
            this->methods->updateScale(this, 1, SCALE_X3);
        } else if (roll >= 5) {
            this->state = 10;
        }
    }
    if (this->state == 10 && this->moodTimer >= 201) {
        this->methods->addTranslation(this, TRANSLATE_Z_MINUS256);
    }
}

/* Row 45 has no per-tick effect. */
void Entity__MoodCue45(void) {}

void Entity__MoodCue46(Entity *this, SoundCueSet *out) {
    s32 dayYearPhase;

    if (this->moodTimer == 0) {
        dayYearPhase =
            ((DreamSys *)this->peer)->methods->getCurrentDayAndYear((DreamSys *)this->peer, 0) % 3;
        if (dayYearPhase == 0) {
            if (rand() % 3 != 0) {
                goto skipScaleBump;
            }
        } else if (dayYearPhase != 1) {
            goto skipScaleBump;
        }
        this->methods->updateScale(this, 1, SCALE_SIX);
    }
skipScaleBump:
    if (out->tick == 0) {
        out->attenuation = 0;
        out->slots[0].program = 18;
    }
    SceneNode__FaceTarget((SceneNode *)this, (SceneNode *)this->peer, 1, 0, 0);
}

void Entity__MoodCue47(Entity *this) {
    if (this->targetReached == 0) {
        return;
    }
    if (this->state == 0) {
        this->state = 12;
        this->moodTimer = 0;
        return;
    }
    if (this->state == 12) {
        if (this->moodTimer < 30) {
            if (((DreamSys *)this->peer)->methods->getLinkCommandFlag((DreamSys *)this->peer) != 0) {
                ((DreamSys *)this->peer)->methods->clearTickCallbacks((DreamSys *)this->peer, false);
                this->moodTimer = 0;
                this->state = 11;
            }
        } else {
            this->methods->notifyParents(this, ENTITY_EFFECT_EVENT_VIDEO);
            this->state = 10;
        }
    } else if (this->state == 11) {
        if (this->moodTimer == 100) {
            this->methods->notifyParents(this, ENTITY_EFFECT_END_DREAM);
        } else {
            ((DreamSys *)this->peer)->methods->moveLocalY((DreamSys *)this->peer, -100, 0);
        }
    }
}

void Entity__MoodCue48(Entity *this, SoundCueSet *out) {
    s32 dy;
    EntityMethods *methods;

    if (this->todFrame == 38) {
        SceneNode__FaceTarget((SceneNode *)this, (SceneNode *)this->peer, 1, 0, 0);
        out->attenuation = this->methods->getProximityRatio(this);
        out->slots[0].program = 6;
    }
    methods = this->methods;
    dy = (this->moodTimer % 10 < 5) ? -30 : 30;
    methods->moveLocalY(this, dy, 0);
    this->methods->moveLocalZ(this, -30, (void *)1);
}

void Entity__MoodCue49(Entity *this, SoundCueSet *out) {
    DreamSysMethods *peerMethods;
    void *translation;

    if (out->tick == 6) {
        out->attenuation = 0;
        out->slots[0].program = 4;
        out->slots[1].program = 4;
        out->slots[2].program = 4;
    }
    if (this->targetReached != 0) {
        if (this->state == 0) {
            this->state = 10;
            this->moodTimer = 0;
        } else if (this->state == 10) {
            if (this->moodTimer == 10) {
                this->methods->notifyParents(this, ENTITY_EFFECT_LINK_STAGE);
            } else if (((DreamSys *)this->peer)->methods->getLinkCommandFlag((DreamSys *)this->peer) != 0) {
                peerMethods = ((DreamSys *)this->peer)->methods;
                translation = this->parent ? this->coord2->unk38 : NULL;
                peerMethods->setTranslation((DreamSys *)this->peer, translation);
                ((DreamSys *)this->peer)->methods->updateRotation((DreamSys *)this->peer, 1, ROTATION_YAW_MINUS90);
                ((DreamSys *)this->peer)->methods->clearTickCallbacks((DreamSys *)this->peer, false);
                this->moodTimer = 0;
                this->state = 11;
            }
        } else if (this->state == 11) {
            peerMethods = ((DreamSys *)this->peer)->methods;
            translation = this->parent ? this->coord2->unk38 : NULL;
            peerMethods->setTranslation((DreamSys *)this->peer, translation);
            if (this->moodTimer == 100) {
                this->methods->notifyParents(this, ENTITY_EFFECT_LINK_STAGE);
            }
        }
    }
    this->methods->moveLocalZ(this, -256, 0);
}

void Entity__MoodCue50(Entity *this, SoundCueSet *out) {
    if (this->moodTimer < this->todFrameCount * 5) {
        if (this->todFrame == 15 || this->todFrame == 70) {
            out->attenuation = 0;
            out->slots[0].program = 7;
            out->slots[1].program = 7;
            out->slots[2].program = 7;
        }
    } else {
        this->methods->deactivate(this);
        this->state = ENTITY_STATE_DONE;
    }
}

void Entity__MoodCue51(Entity *this, SoundCueSet *out) {
    Ratio16 *table;

    if (this->moodTimer == 0 && rand() % 5 == 0 && this->state == 0) {
        this->methods->updateScale(this, 1, SCALE_SIX);
        this->methods->moveLocalY(this, 800, 0);
        this->state = 11;
    }
    table = NULL;
    if (out->tick % 5 == 0) {
        out->attenuation = this->methods->getProximityRatio(this);
        out->slots[0].program = 8;
    }
    if (this->moodTimer == 90) {
        table = ROTATION_YAW_MINUS90;
    } else if (this->moodTimer == 160) {
        table = ROTATION_YAW_PLUS90;
    } else if (this->moodTimer == 220) {
        if (rand() & 1) {
            table = ROTATION_YAW_PLUS180;
        }
    }
    if (table != NULL) {
        this->methods->updateRotation(this, 0, table);
    }
    this->methods->moveLocalZ(this, -80, (void *)1);
}

void Entity__MoodCue52(Entity *this, SoundCueSet *out) {
    out->attenuation = this->methods->getProximityRatio(this);
    if (this->moodTimer < 188) {
        if (this->moodTimer == 84) {
            this->methods->updateRotation(this, 0, ROTATION_YAW_PLUS180);
        }
        if (out->tick % 20 == 0) {
            out->slots[0].program = 9;
        }
    } else if (this->moodTimer < 200) {
        this->methods->updateRotation(this, 0, ROTATION_YAW_PLUS9);
    } else {
        this->methods->deactivate(this);
        out->slots[1].program = 30;
        this->state = ENTITY_STATE_DONE;
    }
    this->methods->moveLocalZOrFindLink(this, -512, 0);
}

void Entity__MoodCue55(Entity *this, SoundCueSet *out) {
    s32 frame = this->todFrame;

    if (this->moodTimer == 0) {
        if (rand() % 3 == 0) {
            this->methods->updateScale(this, 1, SCALE_Y2);
        }
    }
    out->attenuation = this->methods->getProximityRatio(this);
    if (frame >= 32) {
        frame -= 32;
    }
    if (frame == 9 || frame == 17 || frame == 23) {
        out->slots[0].program = 19;
    }
    if (frame == 23) {
        out->slots[1].program = 19;
    }
}

void Entity__MoodCue56(Entity *this) {
    if (this->moodTimer == 0) {
        this->methods->moveLocalY(this, -200, 0);
    }
}

void Entity__MoodCue57(Entity *this, SoundCueSet *out) {
    s32 frame;

    if (this->moodTimer == 0) {
        this->state = rand() % 3;
        if (this->state == 0) {
            this->methods->stopTod(this);
            this->methods->moveLocalY(this, 6144, 0);
        }
    }
    out->attenuation = this->methods->getProximityRatio(this);
    if (this->state != 0) {
        frame = this->todFrame;
        if (frame < 30) {
            out->slots[0].program = 12;
            out->slots[0].octave = -1;
        } else if (frame == 30) {
            out->slots[0].program = SOUND_CUE_STOP;
        } else if (frame == 35) {
            out->slots[2].program = 22;
            out->slots[2].octave = -2;
        } else if (frame == 48) {
            if (Entity__IsNearTarget(this, &this->coord2->tx, 15, 10)) {
                if (Entity__GetOrCreateFadeBox(this, NULL, NULL, (void *)10, 0) != NULL) {
                    this->unk100->methods->startFadeDown(this->unk100, (BasicClass *)this->ticker, 4, 0);
                }
                if (rand() & 1) {
                    this->methods->notifyParents(this, ENTITY_EFFECT_EVENT_VIDEO);
                }
            }
        } else if (frame == 59) {
            this->methods->deactivate(this);
            this->state = ENTITY_STATE_DONE;
        }
        return;
    }
    out->slots[0].program = 12;
    out->slots[0].octave = -1;
    this->methods->moveLocalZ(this, -512, 0);
    if (this->moodTimer >= 128 && this->moodTimer < 322) {
        this->methods->moveLocalY(this, -128, 0);
    } else if (this->moodTimer == 322) {
        ((EntityPlayTodFn)this->methods->playTod)(this);
        this->state = ENTITY_STATE_DONE;
    }
}

void Entity__MoodCue58(Entity *this, SoundCueSet *out) {
    if (this->moodTimer == 0) {
        out->attenuation = 0;
        out->slots[0].program = 12;
        if (((DreamSys *)this->peer)->methods->getDreamColor((DreamSys *)this->peer) ==
            DREAM_COLOR_YELLOW) {
            this->state = 11;
        } else if (rand() % 3 == 0) {
            this->state = 12;
        }
    }
    if (out->tick % 100 == 0) {
        out->attenuation = this->methods->getProximityRatio(this);
        out->slots[0].program = 12;
        out->slots[0].octave = -1;
    }
    if (this->state == 11) {
        if (this->methods->distanceToPeer(this, this->peer) < 1024) {
            ((DreamSys *)this->peer)->methods->clearTickCallbacks((DreamSys *)this->peer, false);
            this->state = 13;
            this->moodTimer = 0;
        }
    } else if (this->state == 12) {
        if (this->methods->distanceToPeer(this, this->peer) < 1024) {
            this->methods->stopTod(this);
            this->state = 14;
            this->moodTimer = 0;
        }
    }
    if (this->state == 13) {
        if (this->moodTimer < 50) {
            ((DreamSys *)this->peer)->methods->moveLocalY((DreamSys *)this->peer, -20, 0);
        } else if (this->moodTimer < 500) {
            ((DreamSys *)this->peer)
                ->methods->moveLocalX((DreamSys *)this->peer, (this->moodTimer % 40 < 20) ? -5 : 5, 0);
        } else if (this->moodTimer == 500) {
            this->methods->notifyParents(this, ENTITY_EFFECT_END_DREAM);
        }
    }
    if (this->state == 14) {
        if (this->moodTimer < 10) {
            this->methods->moveLocalY(this, 200, 0);
            return;
        }
        if (this->moodTimer == 10) {
            out->slots[0].program = 18;
            out->attenuation = 0;
            out->slots[1].program = 3;
            this->methods->updateRotation(this, 1, ROTATION_ZMINUS90);
            this->methods->moveLocalX(this, 2400, 0);
            this->methods->moveLocalY(this, 1500, 0);
            this->parts[1]->methods->setDisplay(this->parts[1], 0);
            this->state = ENTITY_STATE_DONE;
        }
    }
}

void Entity__MoodCue115(Entity *this, SoundCueSet *out) {
    if (this->state == 0) {
        if (Entity__IsTargetInRange(this, 2048) != 0) {
            this->state = 11;
            SceneNode__FaceTarget((SceneNode *)this, (SceneNode *)this->peer, 1, 0, 0);
            SceneNode__FaceTarget((SceneNode *)this->peer, (SceneNode *)this, 1, 1, 0);
            this->methods->activate(this);
            this->methods->startSoundCue(this);
            ((DreamSys *)this->peer)->methods->clearTickCallbacks((DreamSys *)this->peer, true);
            out->attenuation = 0;
            out->slots[0].program = 12;
            this->moodTimer = 0;
        }
    }
    if (this->state == 0) {
        this->methods->deactivate(this);
        this->methods->stopSoundCue(this);
        goto tail;
    }
    if (out->tick % 100 == 0) {
        out->attenuation = this->methods->getProximityRatio(this);
        out->slots[0].program = 12;
        out->slots[0].octave = -1;
    }
    if (this->moodTimer < 3) {
        this->methods->moveLocalY(this, 150, 0);
    } else if (this->moodTimer < 7) {
        this->methods->moveLocalY(this, (this->moodTimer & 1) ? -50 : 50, 0);
    } else if (this->moodTimer == 100) {
        if (rand() & 1) {
            this->state = 12;
            this->methods->stopTod(this);
        }
    } else if (this->moodTimer == 240) {
        ((DreamSys *)this->peer)
            ->methods->setTickCallbacks((DreamSys *)this->peer, MOVE_CALLBACK_TICK_MOVE,
                                        LOOK_CALLBACK_STEP_LOOK);
    }
    if (this->state == 12) {
        if (this->moodTimer < 130) {
            this->methods->moveLocalY(this, 10, 0);
        } else if (this->moodTimer < 160) {
            this->methods->moveLocalZ(this, -30, 0);
        } else if (this->moodTimer < 301) {
            /* nothing */
        } else {
            SceneNode__FaceTarget((SceneNode *)this, (SceneNode *)this->peer, 1, 0, 0);
            this->methods->moveLocalZ(this, -30, 0);
        }
    }
tail:
    if (this->methods->distanceToPeer(this, this->peer) < 512) {
        this->methods->deactivate(this);
        this->methods->notifyParents(this, ENTITY_EFFECT_LINK_STAGE);
    }
}
