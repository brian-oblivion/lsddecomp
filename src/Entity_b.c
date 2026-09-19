#include "common.h"
#include "Entity.h"

/* Data rows this unit's mood-dispatch handlers pass through to a vtable
 * call as an opaque argument -- never dereferenced here, so an opaque byte
 * array is enough to form &D_8008xxxx correctly. Real element type/count
 * unknown. */
extern u8 D_80089DD8[];
extern u8 D_80089DF0[];
extern u8 D_80089D78[];
extern u8 D_80089CA0[];
extern u8 D_80089C94[];
extern u8 D_80089C88[];
extern u8 D_80089D3C[];
extern u8 D_80089DA8[];
extern u8 D_80089CE8[];
extern u8 D_80089CF4[];

s32 func_8005DE18(Entity *this) {
    EntityMoodRow *row;
    s32 *xptr;
    s32 dist;

    row = &gEntityMoodTable[this->moodIndex];
    if (this->active != 0) {
        if (this->unkF4 == 0) {
            xptr = &this->unk14->x;
            dist = row->unk6;
            if (dist < 0) {
                dist = ~dist + 1;
            }
            if (Entity__IsNearTarget(this, xptr, dist, row->unk9) != 0) {
                this->methods->slot164(this, 1);
            }
        }
        if (row->unk6 < 0) {
            Class6B5CC__FaceTarget(this, this->target, 1, 0, 0);
        }
    }
    return this->unkF4;
}

s32 func_8005DEE0(Entity *this) {
    EntityMoodRow *row;
    s32 *xptr;
    s32 dist;

    if (this->active != 0 && this->soundCueActive == 0 && this->unk44 != 1) {
        row = &gEntityMoodTable[this->moodIndex];
        if (row->unkB != 0) {
            xptr = &this->unk14->x;
            dist = row->unkB;
            if (dist < 0) {
                dist = ~dist + 1;
            }
            if (Entity__IsNearTarget(this, xptr, dist, row->unk9) != 0) {
                this->methods->startSoundCue(this);
            }
        }
    }
    return this->soundCueActive;
}

/* Defined immediately after this function in ROM order, in this same unit;
 * declared here rather than in include/Entity.h, which seven units share. Its
 * return value is tested (`beqz` straight off the `jal`), so it is not void. */
extern s32 func_8005E02C(Entity *this, s32 arg1);

/* arg1 is unused here; the canonical declaration in include/Entity.h has it
 * and func_8005DABC passes 0. Do not drop it -- `conflicting types`. */
void func_8005DF9C(Entity *this, s32 arg1) {
    if (gEntityLinkStageTable[this->moodIndex * 0x10] < 0 &&
        gEntityEventVideoTable[this->moodIndex * 0x10] != 0 &&
        func_8005E02C(this, gEntityEventVideoTable[this->moodIndex * 0x10] << 9)) {
        this->methods->notifyParents(this, 0xA);
    }
}

s32 func_8005E02C(Entity *this, s32 arg1) {
    Unk94Obj *other;
    s32 oy, ty;

    __asm__("");
    other = this->target;
    oy = other->unk14->y;
    ty = this->unk14->y;
    if (oy + 0x200 < ty) {
        goto fail;
    }
    if (ty < oy - 0x200) {
        goto fail;
    }
    if (this->methods->slot144(this, other) < arg1) {
        return 1;
    }
fail:
    return 0;
}

s32 func_8005E0B0(Entity *this) {
    EntityMoodRow *row;
    s32 *xptr;
    s32 dist;

    if (this->active != 0 && this->soundCueActive != 0) {
        row = &gEntityMoodTable[this->moodIndex];
        dist = row->unkB;
        if (dist < 0) {
            dist = ~dist + 1;
            xptr = &this->unk14->x;
            if (Entity__IsNearTarget(this, xptr, dist, row->unk9) == 0) {
                this->methods->stopSoundCue(this);
            }
        }
    }
    return this->soundCueActive;
}

EntityMethods *Get_vtable_Entity(void) {
    return &ENTITY_METHODS;
}

void func_8005E160(Entity *this, EntityMoodHandlerArg *out) {
    if (out->unk4 == 0) {
        if (this->target->methods->slot200(this->target) == 5) {
            this->unk44 = 0x64;
        }
    }
    out->unk10 = this->methods->getProximityRatio(this);
    if (this->unk44 == 0) {
        if (out->unk4 % 10 == 0) {
            out->unk1C = 5;
            out->unk20 = -2;
        }
        if (this->moodTimer == 0x960) {
            this->moodTimer = -1;
        } else if (this->moodTimer < 0x4B0) {
            this->methods->slotC4(this, 0x32, 0);
        } else {
            this->methods->slotC4(this, -0x32, 0);
        }
    } else if (this->moodTimer < 0xFA) {
        if (out->unk4 % 10 == 0) {
            out->unk1C = 5;
            out->unk20 = -2;
        }
        if (this->moodTimer < 0x64) {
            this->methods->slotC4(this, 0x32, 0);
        } else if (this->moodTimer < 0xFA) {
            this->methods->addVec14(this, D_80089DA8);
        }
    } else if (this->moodTimer == 0xFA) {
        this->methods->slot130(this);
        out->unk1C = -2;
    } else if (this->moodTimer >= 0x105 && this->moodTimer < 0x238) {
        this->methods->slotC4(this, -0x32, 0);
        this->methods->slot44(this, 1, D_80089CE8);
    } else if (this->moodTimer >= 0x239) {
        this->methods->slot44(this, 1, D_80089CF4);
    }
}

void func_8005E3C4(Entity *this, EntityMoodHandlerArg *out) {
    out->unk10 = 0;
    if (out->unk4 == 0) {
        out->unk1C = 0x14;
        out->unk30 = 0x14;
        out->unk44 = 0x14;
        this->target->methods->slot130(this->target, 1);
    }
    Class6B5CC__FaceTarget(this, this->target, 1, 0, 0);
    this->methods->slotC4(this, -0x5A, 0);
    if (this->moodTimer == 0x1E) {
        this->methods->notifyParents(this, 0xA);
    }
}

void func_8005E480(Entity *this, EntityMoodHandlerArg *out) {
    out->unk10 = this->methods->getProximityRatio(this);
    if (out->unk4 == 0) {
        out->unk1C = 0x17;
    }
}

void func_8005E4D0(Entity *this, EntityMoodHandlerArg *out) {
    out->unk10 = this->methods->getProximityRatio(this);
    if (this->unk84 == this->unk80 / 2) {
        out->unk1C = 0x7;
        out->unk20 = -0x2;
        out->unk30 = 0x3;
        out->unk34 = -0x2;
    }
    if (out->unk4 % 90 < 3) {
        out->unk44 = 0x6;
        out->unk48 = -0x1;
    }
    if (this->moodTimer >= 0x79) {
        this->methods->slot44(this, 0, D_80089CA0);
        this->methods->slotC4(this, -0x140, 0);
    } else if (this->moodTimer >= 0x38 ||
               Entity__IsNearTarget(this, &this->unk14->x, 1, 1) != 0) {
        this->methods->addVec14(this, D_80089D78);
    } else if (this->moodTimer >= 0xA) {
        Class6B5CC__FaceTarget(this, this->target, 1, 0, 0);
        this->methods->slotC4(this, -0x100, 0);
    } else {
        Class6B5CC__FaceTarget(this, this->target, 1, 0, 0);
    }
}

void func_8005E694(Entity *this) {
    this->methods->slot48(this, 1, D_80089DF0);
    this->methods->addVec14(this, D_80089D78);
}

void func_8005E6F0(Entity *this, EntityMoodHandlerArg *out) {
    out->unk10 = this->methods->getProximityRatio(this);
    if (out->unk4 % (this->unk80 / 2) == 0) {
        out->unk1C = 0xA;
    }
    this->methods->slotC4(this, -0x1E, 0);
}

void func_8005E7A8(Entity *this, EntityMoodHandlerArg *out) {
    out->unk10 = 0;
    if (out->unk4 == 0) {
        out->unk1C = 0xB;
        out->unk30 = 0xB;
        out->unk44 = 0xB;
    }
    this->methods->slotC4(this, -0x1E, 0);
}

void func_8005E7F8(Entity *this, EntityMoodHandlerArg *out) {
    u8 *row;

    this->unk48 = -0x14;
    out->unk10 = this->methods->getProximityRatio(this);
    row = 0;
    if (out->unk4 % (this->unk80 / 2) == 0) {
        out->unk1C = 0xA;
        out->unk20 = 1;
    }
    if (this->unk44 == 0xB) {
        if (this->moodTimer == 0xA8C) {
            row = D_80089C94;
        }
        if (this->moodTimer == 0xC6C) {
            row = D_80089C88;
        }
        if (this->moodTimer == 0xE10) {
            row = D_80089C94;
        }
        if ((u32)(this->moodTimer - 0xD5D) < 0x78) {
            if (this->target->methods->slot100(this->target) != 0) {
                this->moodTimer = 0;
                this->unk44 = 0xD;
            }
        }
    } else if (this->unk44 == 0xC) {
        if (this->moodTimer == 0x7BC) {
            row = D_80089C94;
        }
    } else if (this->unk44 == 0xD) {
        this->unk48 = -0x78;
        Class6B5CC__FaceTarget(this, this->target, 1, 0, 0);
        this->methods->slot48(this, 1, D_80089DD8);
        if (this->methods->slot144(this, this->target) < 0x400) {
            this->methods->notifyParents(this, 0xB);
        }
    }
    if (this->moodTimer == 0x618) {
        if ((rand() & 1) != 0) {
            row = D_80089C88;
            this->unk44 = 0xB;
        } else {
            row = D_80089C94;
            this->unk44 = 0xC;
        }
    }
    if (row != 0) {
        this->methods->slot44(this, 0, row);
    }
    this->methods->slotD0(this, this->unk48, 0);
    if (this->unk44 != 0xC) {
        if (this->unk28 != 0) {
            this->methods->slotCC(this, -0xC8, 0);
        }
    }
}

void func_8005EA94(Entity *this) {
    s32 y;
    s32 result;
    s32 oldFC;

    if (this->moodTimer == 0) {
        if ((rand() & 1) == 0) {
            this->unk44 = 0xB;
        }
    }
    y = this->unk14->y;
    if (y < 0x7D0) {
        Class6B5CC__FaceTarget(this, this->target, 1, 0, 0);
    }
    if (this->unk44 == 0xB) {
        result = this->methods->slot144(this, this->target);
        if (result < 0xA00) {
            this->unk4C->methods->slot138(this->unk4C, 1, 1);
            this->moodTimer = 1;
            this->unk44 = 0xC;
        }
    } else if (this->unk44 == 0xC) {
        oldFC = this->moodTimer;
        this->moodTimer = oldFC + 1;
        if (oldFC == 0x12C) {
            this->methods->notifyParents(this, 0xC);
        }
    }
}

void func_8005EBB4(Entity *this, EntityMoodHandlerArg *out) {
    out->unk10 = this->methods->getProximityRatio(this);
    if (out->unk4 == 0) {
        out->unk1C = 0xC;
        this->unk44++;
    } else if (out->unk4 >= this->unk80 - 1) {
        out->unk4 = -1;
    }
    if (this->unk44 == 0x24) {
        if (rand() % 3 == 0) {
            this->methods->notifyParents(this, 0xB);
        }
    }
}

void func_8005EC98(Entity *this, EntityMoodHandlerArg *out) {
    out->unk10 = this->methods->getProximityRatio(this);
    if (this->unk84 == 0xA) {
        out->unk1C = 0xD;
    }
    this->methods->slotC4(this, -0xA, 0);
}

void func_8005ED10(Entity *this, EntityMoodHandlerArg *out) {
    if (this->moodTimer == 0) {
        out->unk10 = 0;
        out->unk1C = 0xF;
    }
}

void func_8005ED30(Entity *this) {
    u8 *arg2;
    s32 roll;

    if (this->moodTimer == 0) {
        if ((rand() & 1) != 0) {
            this->unk44 = 0xB;
        }
    }
    if (this->unk44 == 0) {
        if (this->moodTimer < 0x40) {
            this->methods->slotC4(this, -0x5A, 0);
        } else if (this->moodTimer == 0x40) {
            roll = rand() & 1;
            arg2 = D_80089C94;
            if (roll != 0) {
                arg2 = D_80089C88;
            }
            this->methods->slot44(this, 0, arg2);
            this->methods->addVec14(this, D_80089D3C);
        } else {
            this->methods->slotD0(this, -0x176, rand() % 2);
        }
    } else if (this->unk44 == 0xB) {
        if (this->moodTimer % 5 == 0) {
            this->methods->slot44(this, 0, D_80089C88);
        }
        this->methods->slotC4(this, -0x800, 0);
        this->methods->slot60(this, (rand() % 7) == 0);
    }
}

s32 func_8005EF20(Entity *this) {
    return this->methods->slot48(this, 1, D_80089DD8);
}
