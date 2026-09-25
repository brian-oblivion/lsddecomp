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
 * (`EntityMoodHandlerArg`); they request tones and step the entity's pose
 * and TOD animation on moodTimer / TOD-frame thresholds.
 *
 * The other two, `SetCueTones7_7_7` and `SetCueTones18_3_3`, are private
 * helpers of Entity__MoodCue85/86 that take only the SoundCueSet and write
 * a fixed three-voice tone request into it.
 *
 * Entity derives from Class65650 (code_55dd4.h), whose TOD fields and slots
 * it inherits at the same offsets: this round renamed the two only this unit
 * touches (`todFramePtr` +0x88, `applyTodFrame` +0x134) and proposed the
 * shared ones (setTod/playTod/stopTod, todIndex/todFrame, companion2, and
 * moodDuration -> todFrameCount) in Entity__MoodCue93.md; the head applied
 * all of them by type scope at merge.
 */
#include "common.h"
#include "Entity.h"

/* Data tables reached with a raw pointer by this unit's mood-dispatch
 * handlers -- same convention as Entity_d.c/Entity_c.c's own SCALE_Y2/
 * SCALE_SIX/etc externs (separate local view per translation unit, not
 * shared via the header). */
extern u8 ROTATION_YAW_PLUS9[];
extern u8 ROTATION_YAW_PLUS180[];
extern u8 ROTATION_ZPLUS1[];
extern u8 ROTATION_ZMINUS9[];
extern u8 TRANSLATE_Y_MINUS256[];
extern u8 D_80089DE4[];
extern u8 SCALE_SIX[];

/* Forward declarations: both are defined later in this file (in ROM
 * order), but Entity__MoodCue85 and Entity__MoodCue86 call them before their own
 * definitions appear -- same convention as Entity_d.c's own forward calls. */
void SetCueTones7_7_7(EntityMoodHandlerArg *out);
void SetCueTones18_3_3(EntityMoodHandlerArg *out);

void Entity__MoodCue82(Entity *this, EntityMoodHandlerArg *out) {
    if (this->moodState == 0 && this->moodTimer == 0) {
        if (rand() % 3 != 0) {
            this->moodState = (rand() & 1) ? 0xB : 0xC;
        } else {
            this->methods->stopSoundCue(this);
            this->methods->slotC4(this, -0x5000, 0);
            rand();
        }
    }
    if (this->moodState == 0xC) {
        Class6B5CC__FaceTarget((Class6B5CC *)this, (Class6B5CC *)this->target, 1, 0, 0);
        if (this->moodTimer == 0x14) {
            out->unk1C = 0x12;
            out->unk10 = 0;
            out->unk30 = 3;
            this->target->methods->slot130(this->target, 1);
        }
        if (this->moodTimer >= 0x15) {
            this->methods->slotC4(this, -0x28, 0);
        }
        if (this->moodTimer == 0x28) {
            this->methods->notifyParents(this, 0xA);
        }
    } else if (this->moodState == 0xB) {
        this->methods->stopTod(this);
        if (this->methods->distanceToRegion(this, this->target) < 0x200) {
            if (rand() % 3 != 0) {
                this->methods->deactivate(this);
            } else {
                this->methods->stopSoundCue(this);
            }
        }
    }
}

void Entity__MoodCue83(Entity *this, EntityMoodHandlerArg *out) {
    if (this->todFrame % 15 == 0) {
        out->unk10 = 0;
        out->unk1C = 0xC;
        out->unk20 = 2;
    }
    if (this->moodTimer == this->todFrameCount) {
        out->unk1C = -2;
        this->methods->stopSoundCue(this);
        this->moodState = 1;
    }
}

void Entity__MoodCue84(Entity *this, EntityMoodHandlerArg *out) {
    out->unk10 = this->methods->getProximityRatio(this);
    if (this->todFrame < 0x28) {
        out->unk1C = 0xC;
        out->unk20 = -2;
        out->unk44 = 5;
        out->unk48 = -1;
        return;
    }
    if (this->todFrame == 0x28) {
        out->unk1C = -2;
        out->unk44 = -2;
        return;
    }
    if (this->todFrame == 0x2D) {
        out->unk30 = 0x12;
        out->unk34 = 1;
        return;
    }
    if (this->todFrame == 0x40) {
        out->unk1C = 7;
        return;
    }
    if (this->todFrame == 0x59) {
        this->methods->stopSoundCue(this);
        this->moodState = 1;
    }
}

void Entity__MoodCue85(Entity *this, EntityMoodHandlerArg *out) {
    if (this->moodState == 0) {
        if (this->todFrame == 5) {
            SetCueTones7_7_7(out);
        }
        if (this->moodTimer == this->todFrameCount) {
            this->methods->stopTod(this);
            this->moodState = 0xA;
            this->moodTimer = -1;
        }
    } else if (this->moodState == 0xA) {
        if (this->moodTimer < 0xA) {
            this->methods->updateRotation(this, 0, ROTATION_YAW_PLUS9);
            if (this->target->methods->slot100(this->target) != 0) {
                SetCueTones7_7_7(out);
                this->moodState = 0xC;
                this->moodTimer = -1;
            }
        } else {
            this->target->methods->slot130(this->target, 1);
            this->moodState = 0xB;
            this->moodTimer = -1;
        }
    } else if (this->moodState == 0xB) {
        Class6B5CC__FaceTarget((Class6B5CC *)this, (Class6B5CC *)this->target, 1, 0, 0);
        if (this->moodTimer < 0x1E) {
            this->methods->slotC4(this, -0xA, 0);
        } else {
            SetCueTones7_7_7(out);
            if (Entity__GetOrCreateUnk100(this, NULL, NULL, (void *)0x1E, 0) != NULL) {
                this->unk100->methods->slotD4(this->unk100, this->companion2, 7, 0);
            }
            this->moodState = 0xD;
            this->moodTimer = -1;
        }
    } else if (this->moodState == 0xD) {
        if (this->moodTimer < 0x5A) {
            if (this->moodTimer == 0x1E) {
                if (Entity__GetOrCreateUnk100(this, NULL, NULL, (void *)0xA, 0) != NULL) {
                    this->unk100->methods->slotD8(this->unk100, this->companion2, 0, 0);
                }
            }
            this->target->methods->slot44(this->target, 0, ROTATION_ZPLUS1);
        } else {
            SetCueTones18_3_3(out);
            this->target->methods->slot44(this->target, 1, ROTATION_YAW_PLUS180);
            this->methods->notifyParents(this, (rand() % 5 != 0) ? 0xA : 0xC);
            this->moodState = 0xE;
        }
    } else if (this->moodState == 0xC) {
        if (this->moodTimer < 0xA) {
            this->methods->updateRotation(this, 0, ROTATION_ZMINUS9);
        } else {
            SetCueTones18_3_3(out);
            this->methods->stopSoundCue(this);
            this->moodState = 1;
        }
    }
}

void Entity__MoodCue86(Entity *this, EntityMoodHandlerArg *out) {
    if (this->moodTimer < 0xA) {
        this->methods->stopTod(this);
    } else if (this->moodTimer == 0xA) {
        this->methods->playTod(this);
    }
    if (this->todFrame == 0xA) {
        SetCueTones18_3_3(out);
    }
    if (this->moodTimer == this->todFrameCount + 0xA) {
        this->methods->stopSoundCue(this);
        this->moodState = 1;
    }
}

void SetCueTones7_7_7(EntityMoodHandlerArg *out) {
    out->unk10 = 0;
    out->unk1C = 7;
    out->unk20 = -2;
    out->unk30 = 7;
    out->unk34 = -2;
    out->unk44 = 7;
    out->unk48 = -2;
}

void SetCueTones18_3_3(EntityMoodHandlerArg *out) {
    out->unk1C = 0x12;
    out->unk10 = 0;
    out->unk30 = 3;
    out->unk44 = 3;
}

void Entity__MoodCue87(Entity *this, EntityMoodHandlerArg *out) {
    out->unk10 = this->methods->getProximityRatio(this);
    if (out->unk4 == 0) {
        out->unk1C = 0x12;
    }
    if (out->unk4 >= this->todFrameCount - 1) {
        out->unk4 = -1;
    }
}

void Entity__MoodCue88(Entity *this, EntityMoodHandlerArg *out) {
    out->unk10 = this->methods->getProximityRatio(this);
    if (out->unk4 == this->todFrameCount / 2) {
        out->unk1C = 0x12;
    }
    if (out->unk4 >= this->todFrameCount - 1) {
        out->unk4 = -1;
    }
}

void Entity__MoodCue89(Entity *this, EntityMoodHandlerArg *out) {
    if (this->moodTimer == 0x14) {
        out->unk1C = 0x12;
        out->unk10 = 0;
        out->unk30 = 3;
        return;
    }
    if (this->moodTimer == this->todFrameCount) {
        this->methods->stopSoundCue(this);
        this->moodState = 1;
        if (rand() & 1) {
            this->methods->notifyParents(this, 0xB);
        }
    }
}

void Entity__MoodCue90(Entity *this, EntityMoodHandlerArg *out) {
    out->unk10 = this->methods->getProximityRatio(this);
    if (out->unk4 == 0) {
        out->unk1C = (rand() & 1) ? 2 : 1;
        out->unk20 = 2;
    }
}

void Entity__MoodCue91(Entity *this, EntityMoodHandlerArg *out) {
    if (this->moodTimer == 0) {
        if (Entity__GetOrCreateUnk100(this, NULL, NULL, (void *)5, 0) != NULL) {
            if (rand() & 1) {
                this->methods->addVec14(this, TRANSLATE_Y_MINUS256);
            }
            this->unk100->methods->slotD4(this->unk100, this->companion2, 0, 0);
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
        this->methods->slotC4(this, -0x14, 0);
        this->target->methods->slot130(this->target, 1);
    }
    if (this->moodTimer == 0x32) {
        this->methods->notifyParents(this, 0xA);
    } else if (this->moodTimer == 0xC) {
        out->unk10 = 0;
        out->unk1C = 0x15;
    }
    this->methods->updateScale(this, 1, D_80089DE4);
}

void Entity__MoodCue92(Entity *this, EntityMoodHandlerArg *out) {
    if (this->todIndex == 0) {
        if (this->targetReached != 0) {
            Class6B5CC__FaceTarget((Class6B5CC *)this, (Class6B5CC *)this->target, 1, 0, 0);
            this->methods->setTod(this, 1);
            this->target->methods->slot130(this->target, 1);
        } else if (this->todFrame == 0) {
            do {
                this->todFramePtr = this->methods->applyTodFrame(this, this->todFramePtr, 0);
                this->todFrame += 1;
            } while (this->todFrame < 0x18);
        }
    } else {
        if (this->todFrame == 0) {
            out->unk10 = 0;
            out->unk1C = 0x16;
        } else if (this->todFrame == this->todFrameCount - 1) {
            out->unk10 = 0;
            out->unk30 = 0x12;
            this->methods->notifyParents(this, 0xA);
        }
    }
    this->methods->updateScale(this, 1, D_80089DE4);
}

void Entity__MoodCue93(Entity *this, EntityMoodHandlerArg *out) {
    out->unk10 = this->methods->getProximityRatio(this);
    if (out->unk4 % 10 == 0) {
        out->unk1C = 3;
    }
    if (this->moodTimer == this->todFrameCount) {
        this->methods->setTod(this, 1);
    }
    if (this->todIndex == 1) {
        this->methods->slotC4(this, -0x80, 1);
    }
}

void Entity__MoodCue94(Entity *this, EntityMoodHandlerArg *out) {
    if (this->moodTimer == 0) {
        if (this->target->methods->slot200(this->target) != 7) {
            this->moodState = 0xB;
        }
    }
    out->unk10 = this->methods->getProximityRatio(this);
    if (out->unk4 % 10 == 0) {
        out->unk1C = 0xE;
    }
    if (this->moodTimer == this->todFrameCount) {
        this->methods->setTod(this, 1);
        if (this->moodState != 0) {
            if ((rand() & 1) == 0) {
                this->methods->updateScale(this, 1, SCALE_SIX);
                this->methods->slotCC(this, 0x800, 0);
            }
        }
        if (rand() % 3 == 0) {
            this->methods->updateRotation(this, 0, ROTATION_YAW_PLUS180);
        }
    }
    if (this->todIndex != 0) {
        this->methods->slotC4(this, -0x80, 1);
    }
}

void Entity__MoodCue95(Entity *this, EntityMoodHandlerArg *out) {
    if (this->moodTimer == 0) {
        this->methods->setTod(this, 3);
    } else if (this->moodTimer == this->todFrameCount) {
        this->methods->setTod(this, 1);
    }
    if (this->todIndex == 1) {
        this->methods->slotC4(this, -0x80, 0);
    }
}

void Entity__MoodCue96(Entity *this, EntityMoodHandlerArg *out) {
    if (this->moodTimer == 0) {
        this->methods->setTod(this, rand() % 4);
        return;
    }
    if (this->moodTimer % this->todFrameCount == 0) {
        this->methods->setTod(this, rand() % 4);
        out->unk10 = this->methods->getProximityRatio(this);
        out->unk1C = 0x16;
        out->unk20 = 2;
        out->unk24 = 0x40;
        out->unk28 = 0x20;
    }
}
