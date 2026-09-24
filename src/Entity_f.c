#include "common.h"
#include "Entity.h"

/* Data tables reached with a raw pointer by this unit's mood-dispatch
 * handlers -- same convention as Entity_d.c/Entity_c.c's own SCALE_Y2/
 * SCALE_SIX/etc externs (separate local view per translation unit, not
 * shared via the header). */
extern u8 ROTATION_YAW_PLUS9[];
extern u8 ROTATION_YAW_PLUS180[];
extern u8 D_80089CD0[];
extern u8 D_80089CDC[];
extern u8 D_80089D90[];
extern u8 D_80089DE4[];
extern u8 SCALE_SIX[];

/* Forward declarations: both are defined later in this file (in ROM
 * order), but func_80063874 and func_80063BC0 call them before their own
 * definitions appear -- same convention as Entity_d.c's own forward calls. */
void func_80063C84(EntityMoodHandlerArg *out);
void func_80063CAC(EntityMoodHandlerArg *out);

void func_800634A8(Entity *this, EntityMoodHandlerArg *out) {
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
        Class6B5CC__FaceTarget(this, this->target, 1, 0, 0);
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
        this->methods->slot130(this);
        if (this->methods->distanceToRegion(this, this->target) < 0x200) {
            if (rand() % 3 != 0) {
                this->methods->deactivate(this);
            } else {
                this->methods->stopSoundCue(this);
            }
        }
    }
}

void func_800636E4(Entity *this, EntityMoodHandlerArg *out) {
    if (this->unk84 % 15 == 0) {
        out->unk10 = 0;
        out->unk1C = 0xC;
        out->unk20 = 2;
    }
    if (this->moodTimer == this->moodDuration) {
        out->unk1C = -2;
        this->methods->stopSoundCue(this);
        this->moodState = 1;
    }
}

void func_80063784(Entity *this, EntityMoodHandlerArg *out) {
    out->unk10 = this->methods->getProximityRatio(this);
    if (this->unk84 < 0x28) {
        out->unk1C = 0xC;
        out->unk20 = -2;
        out->unk44 = 5;
        out->unk48 = -1;
        return;
    }
    if (this->unk84 == 0x28) {
        out->unk1C = -2;
        out->unk44 = -2;
        return;
    }
    if (this->unk84 == 0x2D) {
        out->unk30 = 0x12;
        out->unk34 = 1;
        return;
    }
    if (this->unk84 == 0x40) {
        out->unk1C = 7;
        return;
    }
    if (this->unk84 == 0x59) {
        this->methods->stopSoundCue(this);
        this->moodState = 1;
    }
}

void func_80063874(Entity *this, EntityMoodHandlerArg *out) {
    if (this->moodState == 0) {
        if (this->unk84 == 5) {
            func_80063C84(out);
        }
        if (this->moodTimer == this->moodDuration) {
            this->methods->slot130(this);
            this->moodState = 0xA;
            this->moodTimer = -1;
        }
    } else if (this->moodState == 0xA) {
        if (this->moodTimer < 0xA) {
            this->methods->updateRotation(this, 0, ROTATION_YAW_PLUS9);
            if (this->target->methods->slot100(this->target) != 0) {
                func_80063C84(out);
                this->moodState = 0xC;
                this->moodTimer = -1;
            }
        } else {
            this->target->methods->slot130(this->target, 1);
            this->moodState = 0xB;
            this->moodTimer = -1;
        }
    } else if (this->moodState == 0xB) {
        Class6B5CC__FaceTarget(this, this->target, 1, 0, 0);
        if (this->moodTimer < 0x1E) {
            this->methods->slotC4(this, -0xA, 0);
        } else {
            func_80063C84(out);
            if (Entity__GetOrCreateUnk100(this, NULL, NULL, (void *)0x1E, 0) != NULL) {
                this->unk100->methods->slotD4(this->unk100, this->unk50, 7, 0);
            }
            this->moodState = 0xD;
            this->moodTimer = -1;
        }
    } else if (this->moodState == 0xD) {
        if (this->moodTimer < 0x5A) {
            if (this->moodTimer == 0x1E) {
                if (Entity__GetOrCreateUnk100(this, NULL, NULL, (void *)0xA, 0) != NULL) {
                    this->unk100->methods->slotD8(this->unk100, this->unk50, 0, 0);
                }
            }
            this->target->methods->slot44(this->target, 0, D_80089CD0);
        } else {
            func_80063CAC(out);
            this->target->methods->slot44(this->target, 1, ROTATION_YAW_PLUS180);
            this->methods->notifyParents(this, (rand() % 5 != 0) ? 0xA : 0xC);
            this->moodState = 0xE;
        }
    } else if (this->moodState == 0xC) {
        if (this->moodTimer < 0xA) {
            this->methods->updateRotation(this, 0, D_80089CDC);
        } else {
            func_80063CAC(out);
            this->methods->stopSoundCue(this);
            this->moodState = 1;
        }
    }
}

void func_80063BC0(Entity *this, EntityMoodHandlerArg *out) {
    if (this->moodTimer < 0xA) {
        this->methods->slot130(this);
    } else if (this->moodTimer == 0xA) {
        this->methods->slot12C(this);
    }
    if (this->unk84 == 0xA) {
        func_80063CAC(out);
    }
    if (this->moodTimer == this->moodDuration + 0xA) {
        this->methods->stopSoundCue(this);
        this->moodState = 1;
    }
}

void func_80063C84(EntityMoodHandlerArg *out) {
    out->unk10 = 0;
    out->unk1C = 7;
    out->unk20 = -2;
    out->unk30 = 7;
    out->unk34 = -2;
    out->unk44 = 7;
    out->unk48 = -2;
}

void func_80063CAC(EntityMoodHandlerArg *out) {
    out->unk1C = 0x12;
    out->unk10 = 0;
    out->unk30 = 3;
    out->unk44 = 3;
}

void func_80063CC8(Entity *this, EntityMoodHandlerArg *out) {
    out->unk10 = this->methods->getProximityRatio(this);
    if (out->unk4 == 0) {
        out->unk1C = 0x12;
    }
    if (out->unk4 >= this->moodDuration - 1) {
        out->unk4 = -1;
    }
}

void func_80063D40(Entity *this, EntityMoodHandlerArg *out) {
    out->unk10 = this->methods->getProximityRatio(this);
    if (out->unk4 == this->moodDuration / 2) {
        out->unk1C = 0x12;
    }
    if (out->unk4 >= this->moodDuration - 1) {
        out->unk4 = -1;
    }
}

void func_80063DC8(Entity *this, EntityMoodHandlerArg *out) {
    if (this->moodTimer == 0x14) {
        out->unk1C = 0x12;
        out->unk10 = 0;
        out->unk30 = 3;
        return;
    }
    if (this->moodTimer == this->moodDuration) {
        this->methods->stopSoundCue(this);
        this->moodState = 1;
        if (rand() & 1) {
            this->methods->notifyParents(this, 0xB);
        }
    }
}

void func_80063E68(Entity *this, EntityMoodHandlerArg *out) {
    out->unk10 = this->methods->getProximityRatio(this);
    if (out->unk4 == 0) {
        out->unk1C = (rand() & 1) ? 2 : 1;
        out->unk20 = 2;
    }
}

void func_80063ED4(Entity *this, EntityMoodHandlerArg *out) {
    if (this->moodTimer == 0) {
        if (Entity__GetOrCreateUnk100(this, NULL, NULL, (void *)5, 0) != NULL) {
            if (rand() & 1) {
                this->methods->addVec14(this, D_80089D90);
            }
            this->unk100->methods->slotD4(this->unk100, this->unk50, 0, 0);
        }
    } else {
        if (this->unk84 == 0) {
            do {
                this->unk88 = this->methods->slot134(this, this->unk88, 0);
                this->unk84 += 1;
            } while (this->unk84 < 0x18);
        }
    }
    if (this->unk84 >= 0x19) {
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

void func_80064078(Entity *this, EntityMoodHandlerArg *out) {
    if (this->unk7C == 0) {
        if (this->targetReached != 0) {
            Class6B5CC__FaceTarget(this, this->target, 1, 0, 0);
            this->methods->slot128(this, 1);
            this->target->methods->slot130(this->target, 1);
        } else if (this->unk84 == 0) {
            do {
                this->unk88 = this->methods->slot134(this, this->unk88, 0);
                this->unk84 += 1;
            } while (this->unk84 < 0x18);
        }
    } else {
        if (this->unk84 == 0) {
            out->unk10 = 0;
            out->unk1C = 0x16;
        } else if (this->unk84 == this->moodDuration - 1) {
            out->unk10 = 0;
            out->unk30 = 0x12;
            this->methods->notifyParents(this, 0xA);
        }
    }
    this->methods->updateScale(this, 1, D_80089DE4);
}

void func_800641C0(Entity *this, EntityMoodHandlerArg *out) {
    out->unk10 = this->methods->getProximityRatio(this);
    if (out->unk4 % 10 == 0) {
        out->unk1C = 3;
    }
    if (this->moodTimer == this->moodDuration) {
        this->methods->slot128(this, 1);
    }
    if (this->unk7C == 1) {
        this->methods->slotC4(this, -0x80, 1);
    }
}

void func_80064294(Entity *this, EntityMoodHandlerArg *out) {
    if (this->moodTimer == 0) {
        if (this->target->methods->slot200(this->target) != 7) {
            this->moodState = 0xB;
        }
    }
    out->unk10 = this->methods->getProximityRatio(this);
    if (out->unk4 % 10 == 0) {
        out->unk1C = 0xE;
    }
    if (this->moodTimer == this->moodDuration) {
        this->methods->slot128(this, 1);
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
    if (this->unk7C != 0) {
        this->methods->slotC4(this, -0x80, 1);
    }
}

void func_80064450(Entity *this, EntityMoodHandlerArg *out) {
    if (this->moodTimer == 0) {
        this->methods->slot128(this, 3);
    } else if (this->moodTimer == this->moodDuration) {
        this->methods->slot128(this, 1);
    }
    if (this->unk7C == 1) {
        this->methods->slotC4(this, -0x80, 0);
    }
}

void func_800644E8(Entity *this, EntityMoodHandlerArg *out) {
    if (this->moodTimer == 0) {
        this->methods->slot128(this, rand() % 4);
        return;
    }
    if (this->moodTimer % this->moodDuration == 0) {
        this->methods->slot128(this, rand() % 4);
        out->unk10 = this->methods->getProximityRatio(this);
        out->unk1C = 0x16;
        out->unk20 = 2;
        out->unk24 = 0x40;
        out->unk28 = 0x20;
    }
}
