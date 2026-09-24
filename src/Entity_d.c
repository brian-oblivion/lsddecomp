#include "common.h"
#include "Entity.h"

extern s16 sScaleTemplateZDenom;

/* Still uncarved (asm/Entity_e.s). Called directly by name (jal), not
 * through a vtable -- typed from this function's own call site: a0=this,
 * a1=the mood-dispatch "out" struct, a2=0xBB8, a3=0x1F4, and the LITERAL
 * -0x100 goes on the STACK as the 5th argument (not into a2, despite being
 * computed first in the instruction stream -- the compiler filled a2/a3
 * from the later two literals and left the stack slot for the first).
 * Return value is unused at this, its only known call site, so void is a
 * safe read regardless of the real return type (same caveat CLAUDE.md notes
 * for every other such wrapper in this unit). */
extern void func_80064FBC(Entity *this, EntityMoodHandlerArg *out, s32 arg2, s32 arg3, s32 arg4);

/* Data tables reached with a raw pointer by this unit's mood-dispatch
 * handlers -- same convention as Entity_c.c's own SCALE_Y2/SCALE_SIX/etc
 * externs (separate local view per translation unit, not shared via the
 * header). */
extern u8 ROTATION_YAW_PLUS9[];
extern u8 ROTATION_YAW_MINUS9[];
extern u8 SCALE_X3[];
extern u8 TRANSLATE_Z_MINUS256[];
extern u8 SCALE_Y2[];
extern u8 SCALE_SIX[];
extern u8 ROTATION_YAW_PLUS180[];
extern u8 ROTATION_YAW_MINUS90[];
extern u8 ROTATION_YAW_PLUS90[];
extern u8 SCALE_Y4[];
extern u8 ROTATION_ZMINUS90[];

/* Forward declaration: Entity__func_80060710 is defined later in this file (higher
 * ROM address) but Entity__MoodCue43, at a lower address, calls it directly. */
void Entity__func_80060710(Entity *this);

void Entity__MoodCue39(Entity *this, EntityMoodHandlerArg *out) {
    s32 r;

    if (this->moodTimer == 0) {
        r = this->target->methods->slot1A0(this->target, 0) % 3;
        if (r == 0) {
            if (rand() % 3 != 0) {
                goto skip48;
            }
        } else if (r != 2) {
            goto skip48;
        }
        this->methods->updateScale(this, 1, SCALE_Y4);
    }
skip48:
    if (out->unk4 % 22 == 0) {
        out->unk10 = this->methods->getProximityRatio(this);
        out->unk1C = 2;
    }
    if (rand() % 12 == 0) {
        this->methods->slot130(this);
    } else if (rand() % 6 == 0) {
        this->methods->slot12C(this);
    }
}

void Entity__MoodCue40(Entity *this, EntityMoodHandlerArg *out) {
    void *table = NULL;

    if (out->unk4 % 7 == 0) {
        out->unk10 = this->methods->getProximityRatio(this);
        out->unk1C = 3;
        out->unk24 = 0x40;
        out->unk28 = 0x40;
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
    this->methods->slotD0(this, -0x1E, 0);
    if (this->unk28 != 0) {
        this->methods->slotCC(this, -0xC8, 0);
    }
}

void Entity__MoodCue41(Entity *this, EntityMoodHandlerArg *out) {
    s32 rv;
    s16 *tablePtr;

    if (this->moodTimer == 0) {
        this->unk44 = rand() % 5 + 0xA;
    }
    if (this->unk44 < 0xE || this->moodTimer < 0x140) {
        func_80064FBC(this, out, 0xBB8, 0x1F4, -0x100);
        return;
    }
    if (this->unk44 == 0xE) {
        if ((this->moodTimer & 3) == 0) {
            rv = rand();
            tablePtr = &sScaleTemplateZDenom;
            *tablePtr = rv % 32 + 1;
            this->methods->updateScale(this, 1, (u8 *)tablePtr - 0xA);
        }
    }
}

void Entity__MoodCue42(Entity *this, EntityMoodHandlerArg *out) {
    s32 divisor;

    if (this->moodTimer < 0x14) {
        this->methods->slot130(this);
        this->methods->slotC4(this, -0x1E, 0);
    } else if (this->moodTimer == 0x14) {
        this->methods->slot12C(this);
        out->unk10 = 0;
        out->unk1C = 5;
    } else {
        divisor = this->unk80 * 3 + 0x14;
        if (this->moodTimer % divisor == 0) {
            this->methods->slot130(this);
            out->unk1C = -2;
        }
    }
}

void Entity__MoodCue43(Entity *this, EntityMoodHandlerArg *out) {
    s32 a1val;
    s32 r2;
    u8 *table;

    Entity__func_80060710(this);
    out->unk10 = this->methods->getProximityRatio(this);
    if (this->unk84 == 0 || this->unk84 == 0xF) {
        out->unk1C = 0x12;
        out->unk30 = 0x12;
    }
    if (this->moodTimer >= 0x141) {
        a1val = (rand() & 1) ? -0x3C : 0x3C;
        this->methods->slotC8(this, a1val, 0);
        r2 = rand();
        table = ROTATION_YAW_PLUS9;
        if ((r2 & 3) != 0) {
            table = ROTATION_YAW_MINUS9;
        }
        this->methods->updateRotation(this, 0, table);
    }
}

void Entity__MoodCue44(Entity *this, EntityMoodHandlerArg *out) {
    s32 a1val;
    s32 r1;
    s32 r2;
    u8 *table;

    Entity__func_80060710(this);
    out->unk10 = this->methods->getProximityRatio(this);
    if (this->unk84 == 7 || this->unk84 == 0x16) {
        out->unk1C = 3;
    }
    if ((u32)(this->moodTimer - 0x12C) < 0x14) {
        this->methods->slotC4(this, -0x3C, 0);
    } else if ((u32)(this->moodTimer - 0x141) < 0x13) {
        this->methods->updateRotation(this, 0, ROTATION_YAW_MINUS9);
    } else if (this->moodTimer >= 0x141) {
        r1 = rand();
        a1val = -0x80;
        if ((r1 & 1) != 0) {
            a1val = 0x80;
        }
        this->methods->slotC8(this, a1val, 1);
        r2 = rand();
        table = ROTATION_YAW_PLUS9;
        if ((r2 & 3) != 0) {
            table = ROTATION_YAW_MINUS9;
        }
        this->methods->updateRotation(this, 0, table);
    }
}

void Entity__func_80060710(Entity *this) {
    s32 r;

    if (this->moodTimer == 0) {
        r = rand() % 10;
        if (r >= 8) {
            this->methods->updateScale(this, 1, SCALE_X3);
        } else if (r >= 5) {
            this->unk44 = 0xA;
        }
    }
    if (this->unk44 == 0xA && this->moodTimer >= 0xC9) {
        this->methods->addVec14(this, TRANSLATE_Z_MINUS256);
    }
}

void Entity__MoodCue45(void) {
}

void Entity__MoodCue46(Entity *this, EntityMoodHandlerArg *out) {
    s32 r;

    if (this->moodTimer == 0) {
        r = this->target->methods->slot1A0(this->target, 0) % 3;
        if (r == 0) {
            if (rand() % 3 != 0) {
                goto skip48;
            }
        } else if (r != 1) {
            goto skip48;
        }
        this->methods->updateScale(this, 1, SCALE_SIX);
    }
skip48:
    if (out->unk4 == 0) {
        out->unk10 = 0;
        out->unk1C = 0x12;
    }
    Class6B5CC__FaceTarget(this, this->target, 1, 0, 0);
}

void Entity__MoodCue47(Entity *this) {
    if (this->unkF4 == 0) {
        return;
    }
    if (this->unk44 == 0) {
        this->unk44 = 0xC;
        this->moodTimer = 0;
        return;
    }
    if (this->unk44 == 0xC) {
        if (this->moodTimer < 0x1E) {
            if (this->target->methods->slot100(this->target) != 0) {
                this->target->methods->slot130(this->target, 0);
                this->moodTimer = 0;
                this->unk44 = 0xB;
            }
        } else {
            this->methods->notifyParents(this, 0xB);
            this->unk44 = 0xA;
        }
    } else if (this->unk44 == 0xB) {
        if (this->moodTimer == 0x64) {
            this->methods->notifyParents(this, 0xC);
        } else {
            this->target->methods->slotCC(this->target, -0x64, 0);
        }
    }
}

void Entity__MoodCue48(Entity *this, EntityMoodHandlerArg *out) {
    s32 a1val;
    EntityMethods *methods;

    if (this->unk84 == 0x26) {
        Class6B5CC__FaceTarget(this, this->target, 1, 0, 0);
        out->unk10 = this->methods->getProximityRatio(this);
        out->unk1C = 6;
    }
    methods = this->methods;
    a1val = (this->moodTimer % 10 < 5) ? -0x1E : 0x1E;
    methods->slotCC(this, a1val, 0);
    this->methods->slotC4(this, -0x1E, 1);
}

void Entity__MoodCue49(Entity *this, EntityMoodHandlerArg *out) {
    Unk94Methods *methods94;
    void *a1;

    if (out->unk4 == 6) {
        out->unk10 = 0;
        out->unk1C = 4;
        out->unk30 = 4;
        out->unk44 = 4;
    }
    if (this->unkF4 != 0) {
        if (this->unk44 == 0) {
            this->unk44 = 0xA;
            this->moodTimer = 0;
        } else if (this->unk44 == 0xA) {
            if (this->moodTimer == 0xA) {
                this->methods->notifyParents(this, 0xA);
            } else if (this->target->methods->slot100(this->target) != 0) {
                methods94 = this->target->methods;
                a1 = this->unk0C ? (u8 *)this->unk14 + 0x38 : NULL;
                methods94->slotB8(this->target, a1);
                this->target->methods->slot44(this->target, 1, ROTATION_YAW_MINUS90);
                this->target->methods->slot130(this->target, 0);
                this->moodTimer = 0;
                this->unk44 = 0xB;
            }
        } else if (this->unk44 == 0xB) {
            methods94 = this->target->methods;
            a1 = this->unk0C ? (u8 *)this->unk14 + 0x38 : NULL;
            methods94->slotB8(this->target, a1);
            if (this->moodTimer == 0x64) {
                this->methods->notifyParents(this, 0xA);
            }
        }
    }
    this->methods->slotC4(this, -0x100, 0);
}

void Entity__MoodCue50(Entity *this, EntityMoodHandlerArg *out) {
    if (this->moodTimer < this->unk80 * 5) {
        if (this->unk84 == 0xF || this->unk84 == 0x46) {
            out->unk10 = 0;
            out->unk1C = 7;
            out->unk30 = 7;
            out->unk44 = 7;
        }
    } else {
        this->methods->deactivate(this);
        this->unk44 = 1;
    }
}

void Entity__MoodCue51(Entity *this, EntityMoodHandlerArg *out) {
    void *table;

    if (this->moodTimer == 0 && rand() % 5 == 0 && this->unk44 == 0) {
        this->methods->updateScale(this, 1, SCALE_SIX);
        this->methods->slotCC(this, 0x320, 0);
        this->unk44 = 0xB;
    }
    table = NULL;
    if (out->unk4 % 5 == 0) {
        out->unk10 = this->methods->getProximityRatio(this);
        out->unk1C = 8;
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
    this->methods->slotC4(this, -0x50, 1);
}

void Entity__MoodCue52(Entity *this, EntityMoodHandlerArg *out) {
    out->unk10 = this->methods->getProximityRatio(this);
    if (this->moodTimer < 0xBC) {
        if (this->moodTimer == 0x54) {
            this->methods->updateRotation(this, 0, ROTATION_YAW_PLUS180);
        }
        if (out->unk4 % 20 == 0) {
            out->unk1C = 9;
        }
    } else if (this->moodTimer < 0xC8) {
        this->methods->updateRotation(this, 0, ROTATION_YAW_PLUS9);
    } else {
        this->methods->deactivate(this);
        out->unk30 = 0x1E;
        this->unk44 = 1;
    }
    this->methods->slotD0(this, -0x200, 0);
}

void Entity__MoodCue55(Entity *this, EntityMoodHandlerArg *out) {
    s32 mood = this->unk84;

    if (this->moodTimer == 0) {
        if (rand() % 3 == 0) {
            this->methods->updateScale(this, 1, SCALE_Y2);
        }
    }
    out->unk10 = this->methods->getProximityRatio(this);
    if (mood >= 0x20) {
        mood -= 0x20;
    }
    if (mood == 9 || mood == 0x11 || mood == 0x17) {
        out->unk1C = 0x13;
    }
    if (mood == 0x17) {
        out->unk30 = 0x13;
    }
}

void Entity__MoodCue56(Entity *this) {
    if (this->moodTimer == 0) {
        this->methods->slotCC(this, -0xC8, 0);
    }
}

void Entity__MoodCue57(Entity *this, EntityMoodHandlerArg *out) {
    s32 mood;

    if (this->moodTimer == 0) {
        this->unk44 = rand() % 3;
        if (this->unk44 == 0) {
            this->methods->slot130(this);
            this->methods->slotCC(this, 0x1800, 0);
        }
    }
    out->unk10 = this->methods->getProximityRatio(this);
    if (this->unk44 != 0) {
        mood = this->unk84;
        if (mood < 0x1E) {
            out->unk1C = 0xC;
            out->unk20 = -1;
        } else if (mood == 0x1E) {
            out->unk1C = -2;
        } else if (mood == 0x23) {
            out->unk44 = 0x16;
            out->unk48 = -2;
        } else if (mood == 0x30) {
            if (Entity__IsNearTarget(this, &this->unk14->x, 0xF, 0xA)) {
                if (Entity__GetOrCreateUnk100(this, NULL, NULL, (void *)0xA, 0) != NULL) {
                    this->unk100->methods->slotD4(this->unk100, this->unk50, 4, 0);
                }
                if (rand() & 1) {
                    this->methods->notifyParents(this, 0xB);
                }
            }
        } else if (mood == 0x3B) {
            this->methods->deactivate(this);
            this->unk44 = 1;
        }
        return;
    }
    out->unk1C = 0xC;
    out->unk20 = -1;
    this->methods->slotC4(this, -0x200, 0);
    if ((u32)(this->moodTimer - 0x80) < 0xC2) {
        this->methods->slotCC(this, -0x80, 0);
    } else if (this->moodTimer == 0x142) {
        this->methods->slot12C(this);
        this->unk44 = 1;
    }
}

void Entity__MoodCue58(Entity *this, EntityMoodHandlerArg *out) {
    if (this->moodTimer == 0) {
        out->unk10 = 0;
        out->unk1C = 0xC;
        if (this->target->methods->slot200(this->target) == 6) {
            this->unk44 = 0xB;
        } else if (rand() % 3 == 0) {
            this->unk44 = 0xC;
        }
    }
    if (out->unk4 % 100 == 0) {
        out->unk10 = this->methods->getProximityRatio(this);
        out->unk1C = 0xC;
        out->unk20 = -1;
    }
    if (this->unk44 == 0xB) {
        if (this->methods->slot144(this, this->target) < 0x400) {
            this->target->methods->slot130(this->target, 0);
            this->unk44 = 0xD;
            this->moodTimer = 0;
        }
    } else if (this->unk44 == 0xC) {
        if (this->methods->slot144(this, this->target) < 0x400) {
            this->methods->slot130(this);
            this->unk44 = 0xE;
            this->moodTimer = 0;
        }
    }
    if (this->unk44 == 0xD) {
        if (this->moodTimer < 0x32) {
            this->target->methods->slotCC(this->target, -0x14, 0);
        } else if (this->moodTimer < 0x1F4) {
            this->target->methods->slotC8(this->target, (this->moodTimer % 40 < 0x14) ? -5 : 5, 0);
        } else if (this->moodTimer == 0x1F4) {
            this->methods->notifyParents(this, 0xC);
        }
    }
    if (this->unk44 == 0xE) {
        if (this->moodTimer < 0xA) {
            this->methods->slotCC(this, 0xC8, 0);
            return;
        }
        if (this->moodTimer == 0xA) {
            out->unk1C = 0x12;
            out->unk10 = 0;
            out->unk30 = 3;
            this->methods->updateRotation(this, 1, ROTATION_ZMINUS90);
            this->methods->slotC8(this, 0x960, 0);
            this->methods->slotCC(this, 0x5DC, 0);
            this->unk70->unk4->methods->slot60(this->unk70->unk4, 0);
            this->unk44 = 1;
        }
    }
}

void Entity__MoodCue115(Entity *this, EntityMoodHandlerArg *out) {
    if (this->unk44 == 0) {
        if (Entity__IsTargetInRange(this, 0x800) != 0) {
            this->unk44 = 0xB;
            Class6B5CC__FaceTarget(this, this->target, 1, 0, 0);
            Class6B5CC__FaceTarget(this->target, this, 1, 1, 0);
            this->methods->activate(this);
            this->methods->startSoundCue(this);
            this->target->methods->slot130(this->target, 1);
            out->unk10 = 0;
            out->unk1C = 0xC;
            this->moodTimer = 0;
        }
    }
    if (this->unk44 == 0) {
        this->methods->deactivate(this);
        this->methods->stopSoundCue(this);
        goto tail;
    }
    if (out->unk4 % 100 == 0) {
        out->unk10 = this->methods->getProximityRatio(this);
        out->unk1C = 0xC;
        out->unk20 = -1;
    }
    if (this->moodTimer < 3) {
        this->methods->slotCC(this, 0x96, 0);
    } else if (this->moodTimer < 7) {
        this->methods->slotCC(this, (this->moodTimer & 1) ? -0x32 : 0x32, 0);
    } else if (this->moodTimer == 0x64) {
        if (rand() & 1) {
            this->unk44 = 0xC;
            this->methods->slot130(this);
        }
    } else if (this->moodTimer == 0xF0) {
        this->target->methods->slot134(this->target, 1, 1);
    }
    if (this->unk44 == 0xC) {
        if (this->moodTimer < 0x82) {
            this->methods->slotCC(this, 0xA, 0);
        } else if (this->moodTimer < 0xA0) {
            this->methods->slotC4(this, -0x1E, 0);
        } else if (this->moodTimer < 0x12D) {
            /* nothing */
        } else {
            Class6B5CC__FaceTarget(this, this->target, 1, 0, 0);
            this->methods->slotC4(this, -0x1E, 0);
        }
    }
tail:
    if (this->methods->slot144(this, this->target) < 0x200) {
        this->methods->deactivate(this);
        this->methods->notifyParents(this, 0xA);
    }
}

