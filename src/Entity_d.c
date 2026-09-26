/* Second 20-function slice of the Entity class's 97-function remainder,
 * 0x5077C..0x52290 (Entity_c is the first slice, Entity_e the third).
 *
 * 19 of the 20 are gEntityMoodHandlerTable callbacks (Entity.h), named
 * Entity__MoodCueNN for the row they occupy -- rows 39-52 and 55-58 are
 * consecutive with Entity_c's own tail, row 115 (Entity__MoodCue115, this
 * unit's last function) is not, confirming row order tracks moodIndex
 * assignment, not code address. The names were confirmed by reading
 * disk/SLPS_015.56 directly rather than trusting address proximity:
 * gEntityMoodHandlerTable's own base plus a fixed per-row stride locates
 * each row's `handler` word (the arithmetic and addresses are in each
 * function's docs/match-reports/Entity__MoodCueNN.md), and it was checked
 * against every candidate function's own address. The one
 * exception, `Entity__RollScaleOrDelayedDrift`, is not itself a table row -- it is a
 * private helper Entity__MoodCue43/44 both call directly (`jal`, not
 * through any vtable or table), tier C because its own purpose beyond
 * "sometimes bump scale, sometimes queue a delayed addTranslation" is not
 * established.
 *
 * `Entity__MoodCue45` is a real, matched, genuinely empty function (`{}`,
 * `jr $ra; nop` after splat's own frame elision) -- row 45 of the table is
 * a legitimate "this mood has no per-tick cue effect" entry, not an
 * unfinished stub.
 */
#include "common.h"
#include "Entity.h"
#include "DreamSys.h"

extern s16 sScaleTemplateZDenom;

/* Data tables reached with a raw pointer by this unit's mood-dispatch
 * handlers -- same convention as Entity_c.c's own SCALE_Y2/SCALE_SIX/etc
 * externs (separate local view per translation unit, not shared via the
 * header). */
extern u8 ROTATION_YAW_PLUS9[];
extern u8 ROTATION_YAW_MINUS9[];
extern u8 SCALE_X3[];
extern LongVec3 TRANSLATE_Z_MINUS256[];
extern u8 SCALE_Y2[];
extern u8 SCALE_SIX[];
extern u8 ROTATION_YAW_PLUS180[];
extern u8 ROTATION_YAW_MINUS90[];
extern u8 ROTATION_YAW_PLUS90[];
extern u8 SCALE_Y4[];
extern u8 ROTATION_ZMINUS90[];

/* Forward declaration: Entity__RollScaleOrDelayedDrift is defined later in this file (higher
 * ROM address) but Entity__MoodCue43, at a lower address, calls it directly. */
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
    void *table = NULL;

    if (out->tick % 7 == 0) {
        out->attenuation = this->methods->getProximityRatio(this);
        out->slots[0].program = 3;
        out->slots[0].vol = 0x40;
        out->slots[0].endVol = 0x40;
    }
    if (this->moodTimer == 0xC8) {
        table = ROTATION_YAW_MINUS90;
    } else if (this->moodTimer == 0x190) {
        table = ROTATION_YAW_PLUS180;
    } else if (this->moodTimer == 0x258) {
        table = ROTATION_YAW_PLUS90;
    } else if (this->moodTimer == 0x320) {
        table = ROTATION_YAW_PLUS180;
        this->moodTimer = -1;
    }
    if (table != NULL) {
        this->methods->updateRotation(this, 0, table);
    }
    this->methods->moveLocalZOrFindLink(this, -0x1E, 0);
    if (this->linkTarget != 0) {
        this->methods->moveLocalY(this, -0xC8, 0);
    }
}

void Entity__MoodCue41(Entity *this, SoundCueSet *out) {
    s32 rv;
    s16 *tablePtr;

    if (this->moodTimer == 0) {
        this->state = rand() % 5 + 0xA;
    }
    if (this->state < 0xE || this->moodTimer < 0x140) {
        Entity__StepYawInWindowsThenDeactivate(this, out, 0xBB8, 0x1F4, -0x100);
        return;
    }
    if (this->state == 0xE) {
        if ((this->moodTimer & 3) == 0) {
            rv = rand();
            tablePtr = &sScaleTemplateZDenom;
            *tablePtr = rv % 32 + 1;
            /* sScaleTemplateZDenom sits directly after SCALE_X3 in rodata;
             * this reaches 0xA bytes back into SCALE_X3's tail to reuse it
             * as the fixed part of a template, with the just-randomized
             * denominator as its final field (see
             * docs/match-reports/Entity__MoodCue41.md). */
            this->methods->updateScale(this, 1, (u8 *)tablePtr - 0xA);
        }
    }
}

void Entity__MoodCue42(Entity *this, SoundCueSet *out) {
    s32 divisor;

    if (this->moodTimer < 0x14) {
        this->methods->stopTod(this);
        this->methods->moveLocalZ(this, -0x1E, 0);
    } else if (this->moodTimer == 0x14) {
        ((EntityPlayTodFn)this->methods->playTod)(this);
        out->attenuation = 0;
        out->slots[0].program = 5;
    } else {
        divisor = this->todFrameCount * 3 + 0x14;
        if (this->moodTimer % divisor == 0) {
            this->methods->stopTod(this);
            out->slots[0].program = -2;
        }
    }
}

void Entity__MoodCue43(Entity *this, SoundCueSet *out) {
    s32 dx;
    s32 rotPick;
    u8 *table;

    Entity__RollScaleOrDelayedDrift(this);
    out->attenuation = this->methods->getProximityRatio(this);
    if (this->todFrame == 0 || this->todFrame == 0xF) {
        out->slots[0].program = 0x12;
        out->slots[1].program = 0x12;
    }
    if (this->moodTimer >= 0x141) {
        dx = (rand() & 1) ? -0x3C : 0x3C;
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
    u8 *table;

    Entity__RollScaleOrDelayedDrift(this);
    out->attenuation = this->methods->getProximityRatio(this);
    if (this->todFrame == 7 || this->todFrame == 0x16) {
        out->slots[0].program = 3;
    }
    if ((u32)(this->moodTimer - 0x12C) < 0x14) {
        this->methods->moveLocalZ(this, -0x3C, 0);
    } else if ((u32)(this->moodTimer - 0x141) < 0x13) {
        this->methods->updateRotation(this, 0, ROTATION_YAW_MINUS9);
    } else if (this->moodTimer >= 0x141) {
        dxPick = rand();
        dx = -0x80;
        if ((dxPick & 1) != 0) {
            dx = 0x80;
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

void Entity__RollScaleOrDelayedDrift(Entity *this) {
    s32 r;

    if (this->moodTimer == 0) {
        r = rand() % 10;
        if (r >= 8) {
            this->methods->updateScale(this, 1, SCALE_X3);
        } else if (r >= 5) {
            this->state = 0xA;
        }
    }
    if (this->state == 0xA && this->moodTimer >= 0xC9) {
        this->methods->addTranslation(this, TRANSLATE_Z_MINUS256);
    }
}

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
        out->slots[0].program = 0x12;
    }
    SceneNode__FaceTarget((SceneNode *)this, (SceneNode *)this->peer, 1, 0, 0);
}

void Entity__MoodCue47(Entity *this) {
    if (this->targetReached == 0) {
        return;
    }
    if (this->state == 0) {
        this->state = 0xC;
        this->moodTimer = 0;
        return;
    }
    if (this->state == 0xC) {
        if (this->moodTimer < 0x1E) {
            if (((DreamSys *)this->peer)->methods->getLinkCommandFlag((DreamSys *)this->peer) != 0) {
                ((DreamSys *)this->peer)->methods->clearTickCallbacks((DreamSys *)this->peer, 0);
                this->moodTimer = 0;
                this->state = 0xB;
            }
        } else {
            this->methods->notifyParents(this, 0xB);
            this->state = 0xA;
        }
    } else if (this->state == 0xB) {
        if (this->moodTimer == 0x64) {
            this->methods->notifyParents(this, 0xC);
        } else {
            ((DreamSys *)this->peer)->methods->moveLocalY((DreamSys *)this->peer, -0x64, 0);
        }
    }
}

void Entity__MoodCue48(Entity *this, SoundCueSet *out) {
    s32 dy;
    EntityMethods *methods;

    if (this->todFrame == 0x26) {
        SceneNode__FaceTarget((SceneNode *)this, (SceneNode *)this->peer, 1, 0, 0);
        out->attenuation = this->methods->getProximityRatio(this);
        out->slots[0].program = 6;
    }
    methods = this->methods;
    dy = (this->moodTimer % 10 < 5) ? -0x1E : 0x1E;
    methods->moveLocalY(this, dy, 0);
    this->methods->moveLocalZ(this, -0x1E, (void *)1);
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
            this->state = 0xA;
            this->moodTimer = 0;
        } else if (this->state == 0xA) {
            if (this->moodTimer == 0xA) {
                this->methods->notifyParents(this, 0xA);
            } else if (((DreamSys *)this->peer)->methods->getLinkCommandFlag((DreamSys *)this->peer) != 0) {
                peerMethods = ((DreamSys *)this->peer)->methods;
                translation = this->parent ? this->coord2->unk38 : NULL;
                peerMethods->setTranslation((DreamSys *)this->peer, translation);
                ((DreamSys *)this->peer)->methods->updateRotation((DreamSys *)this->peer, 1, ROTATION_YAW_MINUS90);
                ((DreamSys *)this->peer)->methods->clearTickCallbacks((DreamSys *)this->peer, 0);
                this->moodTimer = 0;
                this->state = 0xB;
            }
        } else if (this->state == 0xB) {
            peerMethods = ((DreamSys *)this->peer)->methods;
            translation = this->parent ? this->coord2->unk38 : NULL;
            peerMethods->setTranslation((DreamSys *)this->peer, translation);
            if (this->moodTimer == 0x64) {
                this->methods->notifyParents(this, 0xA);
            }
        }
    }
    this->methods->moveLocalZ(this, -0x100, 0);
}

void Entity__MoodCue50(Entity *this, SoundCueSet *out) {
    if (this->moodTimer < this->todFrameCount * 5) {
        if (this->todFrame == 0xF || this->todFrame == 0x46) {
            out->attenuation = 0;
            out->slots[0].program = 7;
            out->slots[1].program = 7;
            out->slots[2].program = 7;
        }
    } else {
        this->methods->deactivate(this);
        this->state = 1;
    }
}

void Entity__MoodCue51(Entity *this, SoundCueSet *out) {
    void *table;

    if (this->moodTimer == 0 && rand() % 5 == 0 && this->state == 0) {
        this->methods->updateScale(this, 1, SCALE_SIX);
        this->methods->moveLocalY(this, 0x320, 0);
        this->state = 0xB;
    }
    table = NULL;
    if (out->tick % 5 == 0) {
        out->attenuation = this->methods->getProximityRatio(this);
        out->slots[0].program = 8;
    }
    if (this->moodTimer == 0x5A) {
        table = ROTATION_YAW_MINUS90;
    } else if (this->moodTimer == 0xA0) {
        table = ROTATION_YAW_PLUS90;
    } else if (this->moodTimer == 0xDC) {
        if (rand() & 1) {
            table = ROTATION_YAW_PLUS180;
        }
    }
    if (table != NULL) {
        this->methods->updateRotation(this, 0, table);
    }
    this->methods->moveLocalZ(this, -0x50, (void *)1);
}

void Entity__MoodCue52(Entity *this, SoundCueSet *out) {
    out->attenuation = this->methods->getProximityRatio(this);
    if (this->moodTimer < 0xBC) {
        if (this->moodTimer == 0x54) {
            this->methods->updateRotation(this, 0, ROTATION_YAW_PLUS180);
        }
        if (out->tick % 20 == 0) {
            out->slots[0].program = 9;
        }
    } else if (this->moodTimer < 0xC8) {
        this->methods->updateRotation(this, 0, ROTATION_YAW_PLUS9);
    } else {
        this->methods->deactivate(this);
        out->slots[1].program = 0x1E;
        this->state = 1;
    }
    this->methods->moveLocalZOrFindLink(this, -0x200, 0);
}

void Entity__MoodCue55(Entity *this, SoundCueSet *out) {
    s32 mood = this->todFrame;

    if (this->moodTimer == 0) {
        if (rand() % 3 == 0) {
            this->methods->updateScale(this, 1, SCALE_Y2);
        }
    }
    out->attenuation = this->methods->getProximityRatio(this);
    if (mood >= 0x20) {
        mood -= 0x20;
    }
    if (mood == 9 || mood == 0x11 || mood == 0x17) {
        out->slots[0].program = 0x13;
    }
    if (mood == 0x17) {
        out->slots[1].program = 0x13;
    }
}

void Entity__MoodCue56(Entity *this) {
    if (this->moodTimer == 0) {
        this->methods->moveLocalY(this, -0xC8, 0);
    }
}

void Entity__MoodCue57(Entity *this, SoundCueSet *out) {
    s32 mood;

    if (this->moodTimer == 0) {
        this->state = rand() % 3;
        if (this->state == 0) {
            this->methods->stopTod(this);
            this->methods->moveLocalY(this, 0x1800, 0);
        }
    }
    out->attenuation = this->methods->getProximityRatio(this);
    if (this->state != 0) {
        mood = this->todFrame;
        if (mood < 0x1E) {
            out->slots[0].program = 0xC;
            out->slots[0].octave = -1;
        } else if (mood == 0x1E) {
            out->slots[0].program = -2;
        } else if (mood == 0x23) {
            out->slots[2].program = 0x16;
            out->slots[2].octave = -2;
        } else if (mood == 0x30) {
            if (Entity__IsNearTarget(this, &this->coord2->tx, 0xF, 0xA)) {
                if (Entity__GetOrCreateUnk100(this, NULL, NULL, (void *)0xA, 0) != NULL) {
                    this->unk100->methods->startFadeDown(this->unk100, (BasicClass *)this->ticker, 4, 0);
                }
                if (rand() & 1) {
                    this->methods->notifyParents(this, 0xB);
                }
            }
        } else if (mood == 0x3B) {
            this->methods->deactivate(this);
            this->state = 1;
        }
        return;
    }
    out->slots[0].program = 0xC;
    out->slots[0].octave = -1;
    this->methods->moveLocalZ(this, -0x200, 0);
    if ((u32)(this->moodTimer - 0x80) < 0xC2) {
        this->methods->moveLocalY(this, -0x80, 0);
    } else if (this->moodTimer == 0x142) {
        ((EntityPlayTodFn)this->methods->playTod)(this);
        this->state = 1;
    }
}

void Entity__MoodCue58(Entity *this, SoundCueSet *out) {
    if (this->moodTimer == 0) {
        out->attenuation = 0;
        out->slots[0].program = 0xC;
        if (((DreamSys *)this->peer)->methods->getDreamColor((DreamSys *)this->peer) == 6) {
            this->state = 0xB;
        } else if (rand() % 3 == 0) {
            this->state = 0xC;
        }
    }
    if (out->tick % 100 == 0) {
        out->attenuation = this->methods->getProximityRatio(this);
        out->slots[0].program = 0xC;
        out->slots[0].octave = -1;
    }
    if (this->state == 0xB) {
        if (this->methods->distanceToPeer(this, this->peer) < 0x400) {
            ((DreamSys *)this->peer)->methods->clearTickCallbacks((DreamSys *)this->peer, 0);
            this->state = 0xD;
            this->moodTimer = 0;
        }
    } else if (this->state == 0xC) {
        if (this->methods->distanceToPeer(this, this->peer) < 0x400) {
            this->methods->stopTod(this);
            this->state = 0xE;
            this->moodTimer = 0;
        }
    }
    if (this->state == 0xD) {
        if (this->moodTimer < 0x32) {
            ((DreamSys *)this->peer)->methods->moveLocalY((DreamSys *)this->peer, -0x14, 0);
        } else if (this->moodTimer < 0x1F4) {
            ((DreamSys *)this->peer)
                ->methods->moveLocalX((DreamSys *)this->peer, (this->moodTimer % 40 < 0x14) ? -5 : 5, 0);
        } else if (this->moodTimer == 0x1F4) {
            this->methods->notifyParents(this, 0xC);
        }
    }
    if (this->state == 0xE) {
        if (this->moodTimer < 0xA) {
            this->methods->moveLocalY(this, 0xC8, 0);
            return;
        }
        if (this->moodTimer == 0xA) {
            out->slots[0].program = 0x12;
            out->attenuation = 0;
            out->slots[1].program = 3;
            this->methods->updateRotation(this, 1, ROTATION_ZMINUS90);
            this->methods->moveLocalX(this, 0x960, 0);
            this->methods->moveLocalY(this, 0x5DC, 0);
            this->parts[1]->methods->setDisplay(this->parts[1], 0);
            this->state = 1;
        }
    }
}

void Entity__MoodCue115(Entity *this, SoundCueSet *out) {
    if (this->state == 0) {
        if (Entity__IsTargetInRange(this, 0x800) != 0) {
            this->state = 0xB;
            SceneNode__FaceTarget((SceneNode *)this, (SceneNode *)this->peer, 1, 0, 0);
            SceneNode__FaceTarget((SceneNode *)this->peer, (SceneNode *)this, 1, 1, 0);
            this->methods->activate(this);
            this->methods->startSoundCue(this);
            ((DreamSys *)this->peer)->methods->clearTickCallbacks((DreamSys *)this->peer, 1);
            out->attenuation = 0;
            out->slots[0].program = 0xC;
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
        out->slots[0].program = 0xC;
        out->slots[0].octave = -1;
    }
    if (this->moodTimer < 3) {
        this->methods->moveLocalY(this, 0x96, 0);
    } else if (this->moodTimer < 7) {
        this->methods->moveLocalY(this, (this->moodTimer & 1) ? -0x32 : 0x32, 0);
    } else if (this->moodTimer == 0x64) {
        if (rand() & 1) {
            this->state = 0xC;
            this->methods->stopTod(this);
        }
    } else if (this->moodTimer == 0xF0) {
        ((DreamSys *)this->peer)->methods->setTickCallbacks((DreamSys *)this->peer, 1, 1);
    }
    if (this->state == 0xC) {
        if (this->moodTimer < 0x82) {
            this->methods->moveLocalY(this, 0xA, 0);
        } else if (this->moodTimer < 0xA0) {
            this->methods->moveLocalZ(this, -0x1E, 0);
        } else if (this->moodTimer < 0x12D) {
            /* nothing */
        } else {
            SceneNode__FaceTarget((SceneNode *)this, (SceneNode *)this->peer, 1, 0, 0);
            this->methods->moveLocalZ(this, -0x1E, 0);
        }
    }
tail:
    if (this->methods->distanceToPeer(this, this->peer) < 0x200) {
        this->methods->deactivate(this);
        this->methods->notifyParents(this, 0xA);
    }
}
