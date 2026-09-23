#include "common.h"
#include "Entity.h"

void func_80061A90(Entity *this, EntityMoodHandlerArg *out) {
    if (this->moodTimer == 0 && rand() % 10 == 0) {
        this->unk44 = 0xC;
    }
    if (out->unk4 % 10 == 0) {
        out->unk10 = this->methods->getProximityRatio(this);
        out->unk1C = 0xC;
        out->unk20 = -1;
    }
    if (this->moodTimer == 0) {
        if (rand() & 1) {
            this->methods->slotCC(this, 0x800, 0);
        }
    }
    this->methods->slotC4(this, -0x80, 0);
    if (this->unk44 == 0xC && this->moodTimer == 0x12C) {
        this->unk4C->methods->slot138(this->unk4C, 1, 1);
    }
}

void func_80061C04(Entity *this, EntityMoodHandlerArg *out) {
    if (this->unk84 == 0x1E) {
        out->unk1C = 0x12;
        out->unk10 = 0;
        out->unk20 = -1;
    }
}

void func_80061C2C(Entity *this, EntityMoodHandlerArg *out) {
    if (this->unk84 % 30 == 0) {
        out->unk1C = 3;
        out->unk10 = 0;
        out->unk20 = -2;
    }
    if (this->moodTimer >= 0x65) {
        Class6B5CC__FaceTarget(this, this->target, 1, 0, 0);
    }
    this->methods->slotC4(this, -5, 0);
    if (this->moodTimer == 0x12C && this->methods->slot144(this, this->target) < 0x1000) {
        this->target->methods->slot130(this->target, 0);
    } else if (this->moodTimer == 0x1F4) {
        this->target->methods->slot134(this->target, 1, 1);
    }
    if (this->unk44 == 0) {
        if (this->methods->slot144(this, this->target) < 0x400) {
            if (rand() & 1) {
                out->unk30 = 6;
                out->unk10 = 0;
                out->unk34 = -1;
                if (rand() & 1) {
                    this->unk4C->methods->slot138(this->unk4C, -1, 0);
                }
                this->moodTimer = 0;
                this->unk44 = 0xA;
            } else {
                this->unk44 = 0xB;
            }
        }
    }
    if (this->unk44 == 0xA && this->moodTimer == 0x46) {
        this->methods->notifyParents(this, (rand() & 1) ? 0xC : 0xB);
    }
}

void func_80061E60(Entity *this, EntityMoodHandlerArg *out) {
    s32 r = out->unk4 % 300;

    out->unk10 = this->methods->getProximityRatio(this);
    if (r < 0x14) {
        out->unk1C = 5;
        out->unk20 = -2;
    } else if (r == 0x16) {
        out->unk1C = -2;
    }
    this->methods->slotC4(this, -0xA, 0);
}

extern u8 SCALE_HALF[];
extern u8 ROTATION_YAW_PLUS90[];
extern u8 ROTATION_YAW_MINUS90[];

void func_80061F30(Entity *this, EntityMoodHandlerArg *out) {
    if (this->moodTimer == 0) {
        if (rand() % 3 == 0) {
            this->methods->slot48(this, 1, SCALE_HALF);
            this->methods->slotCC(this, -0x12C, 0);
            this->methods->slot44(this, 1, ROTATION_YAW_PLUS90);
            this->unk44 = 0xB;
        }
    }
    if (this->unk44 == 0xB) {
        if (this->moodTimer == 0x7D0) {
            this->methods->slot44(this, 0, ROTATION_YAW_MINUS90);
        }
        this->methods->slotC4(this, -0x14, 0);
    }
}

void func_8006204C(Entity *this, EntityMoodHandlerArg *out) {
    out->unk10 = this->methods->getProximityRatio(this);
    if (out->unk4 % 30 == 0) {
        out->unk1C = 0xD;
    }
}

void func_800620C4(Entity *this, EntityMoodHandlerArg *out) {
    if (this->moodTimer == 0) {
        if (rand() % 3 == 0) {
            this->unk44 = 0xB;
        }
    }
    if (this->unk44 == 0xB) {
        if (this->moodTimer == 0x1F6) {
            this->methods->slotCC(this, 0x800, 0);
            Class6B5CC__FaceTarget(this, this->target, 1, 0, 0);
        }
        if (this->moodTimer >= 0x1F5) {
            this->methods->slotC4(this, -0x200, 0);
        }
    }
}

extern u8 D_80089CC4[];
extern u8 D_80089D6C[];

void func_800621A8(Entity *this, EntityMoodHandlerArg *out) {
    out->unk10 = this->methods->getProximityRatio(this);
    if (out->unk4 == 0) {
        this->unk48 = (rand() & 1) ? -0x176 : -0xC0;
    }
    if (this->unk44 == 0) {
        if (out->unk4 % 10 == 0) {
            out->unk1C = 0x1C;
        }
        if (out->unk4 % 20 == 0) {
            out->unk30 = 0x17;
            out->unk34 = -1;
            out->unk44 = 0x17;
            out->unk48 = -1;
        } else if (out->unk4 % 20 == 0xE) {
            out->unk30 = -2;
            out->unk44 = -2;
        }
        if ((this->moodTimer & 1) == 0) {
            if (this->target->methods->slot100(this->target) != 0) {
                this->moodTimer = -1;
                this->unk44 = 0xA;
                out->unk1C = 0x12;
            }
        }
        Class6B5CC__FaceTarget(this, this->target, 1, 0, 0);
        this->methods->slotC4(this, this->unk48, 1);
    } else if (this->unk44 == 0xA) {
        if (this->moodTimer < 8) {
            this->methods->slot44(this, 0, D_80089CC4);
            this->methods->addVec14(this, D_80089D6C);
        } else {
            u32 r;

            out->unk1C = 0x12;
            out->unk30 = 3;
            this->methods->stopSoundCue(this);
            r = rand() & 1;
            this->unk44 = r < 1;
        }
    }
}

void func_800623E8(Entity *this, EntityMoodHandlerArg *out) {
    out->unk10 = this->methods->getProximityRatio(this);
    if (this->unk84 == this->unk80 - 1) {
        out->unk1C = 0x19;
        out->unk20 = -2;
    }
    if (out->unk4 % 4 == 0) {
        out->unk30 = 0x15;
        out->unk34 = -1;
    }
    if (out->unk4 % 200 == 0) {
        out->unk44 = 0xD;
        out->unk48 = 1;
    }
}

/* Data tables reached with a raw pointer by this unit's mood-dispatch
 * handlers -- same convention as Entity_c.c/Entity_d.c's own separate
 * per-unit externs, not shared via the header. */
extern u8 D_80089C7C[];

void func_800624BC(Entity *this, EntityMoodHandlerArg *out) {
    if (this->moodTimer == 0) {
        if (rand() & 1) {
            this->unk44 = 0xB;
        }
    }
    if (this->moodTimer == 0x12C) {
        this->methods->slot44(this, 0, D_80089C7C);
    }
    if (this->moodTimer < 0x258) {
        this->methods->slotC4(this, this->unk44 == 0 ? -0x100 : 0x100, 0);
    }
}

void func_80062570(Entity *this, EntityMoodHandlerArg *out) {
    s32 r;

    out->unk10 = this->methods->getProximityRatio(this);
    if (out->unk4 == 0) {
        out->unk1C = 0;
        r = rand() % 3;
        this->methods->slotC8(this, r * 51200, 0);
    }
    if (this->moodTimer >= 0x961) {
        Class6B5CC__FaceTarget(this, this->target, 1, 0, 0);
    }
    this->methods->slotC4(this, -0x1E, 0);
}

extern u8 ROTATION_YAW_PLUS90[];

void func_80062660(Entity *this, EntityMoodHandlerArg *out) {
    out->unk10 = 0;
    if (out->unk4 == 0) {
        this->target->methods->slot44(this->target, 1, ROTATION_YAW_PLUS90);
        this->target->methods->slot130(this->target, 1);
        out->unk1C = 0x19;
        out->unk30 = 0x19;
        out->unk44 = 0x19;
    } else if (out->unk4 == 0x14) {
        out->unk30 = 0xD;
    }
    if (this->moodTimer == this->unk80 - 1) {
        this->methods->deactivate(this);
    }
}

extern u8 D_8008AC1C[];

void func_80062730(Entity *this, EntityMoodHandlerArg *out) {
    if (this->moodTimer == 0) {
        this->target->unk5C->methods->slot64(this->target->unk5C, D_8008AC1C);
        this->unk44 = rand() % 3;
        if (this->target->unk14->z < 0x262) {
            this->unk44 = 0;
        }
    }
    if (this->unk44 != 0) {
        if (this->unk80 / 2 < this->moodTimer) {
            this->target->methods->slotC4(this->target, 0x80, 0);
        }
        if (this->moodTimer == this->unk80 - 0x1E) {
            this->methods->notifyParents(this, 0xA);
        }
    } else {
        if ((u32)(this->moodTimer - 0x14) < 0x64) {
            this->target->methods->slotC4(this->target, -((this->moodTimer - 0x13) * 0x20), 1);
            if (this->moodTimer == 0x55) {
                this->target->methods->slot134(this->target, 1, 1);
            }
        }
    }
}

void func_800628D4(Entity *this, EntityMoodHandlerArg *out) {
    if (out->unk4 == 0) {
        out->unk10 = 0;
        out->unk1C = 0x19;
        out->unk30 = 0x19;
        out->unk44 = 0x19;
        Class6B5CC__FaceTarget(this, this->target, 1, 0, 0);
    }
    if (this->moodTimer == this->unk80) {
        this->methods->slot130(this);
        this->methods->notifyParents(this, 0xA);
    }
}

extern u8 D_80089DFC[];
extern u8 D_80089C64[];

void func_80062970(Entity *this, EntityMoodHandlerArg *out) {
    void (*fn)(Entity *self, s32 arg1, void *arg2);
    void *table;

    if (this->unkF4 != 0) {
        this->methods->slot12C(this);
        if (this->unk84 == this->unk80 - 1) {
            this->methods->slot130(this);
            fn = (void (*)(Entity *, s32, void *))this->methods->slot48;
            table = D_80089DFC;
            fn(this, 0, table);
        }
    } else {
        this->methods->slot130(this);
        fn = this->methods->slot44;
        table = D_80089C64;
        fn(this, 0, table);
    }
}

void func_80062A40(Entity *this, EntityMoodHandlerArg *out) {
    out->unk10 = this->methods->getProximityRatio(this);
    if (out->unk4 % 5 == 0) {
        out->unk1C = 0x11;
        out->unk20 = -2;
    }
    if (this->unk7C == 0) {
        if (this->moodTimer == this->unk80) {
            this->methods->slot128(this, 1);
            if (rand() & 1) {
                this->unk44 = 0xB;
            }
        }
        return;
    }
    if (this->unk44 == 0) {
        if (this->moodTimer == 0x3C || this->moodTimer == 0xD4 || this->moodTimer == 0x122 || this->moodTimer == 0x140) {
            this->methods->slot44(this, 0, ROTATION_YAW_PLUS90);
        }
        if (this->moodTimer == 0x18E) {
            this->methods->slot44(this, 0, ROTATION_YAW_MINUS90);
        }
        this->methods->slotD0(this, -0x32, 0);
        return;
    }
    if (this->moodTimer == 0x3C || this->moodTimer == 0x8C) {
        this->methods->slot44(this, 0, ROTATION_YAW_PLUS90);
    }
    if (this->moodTimer < 0xAE) {
        this->methods->slotD0(this, -0x32, 0);
    }
    if (this->moodTimer == 0xAE) {
        this->methods->stopSoundCue(this);
        this->unk44 = 1;
    }
}

extern u8 D_80089D0C[];
extern u8 D_80089E50[];
extern u8 D_80089E14[];
extern s32 D_8008ACCC;

void func_80062C58(Entity *this, EntityMoodHandlerArg *out) {
    s32 tmp;
    void *table;

    if (out->unk4 == 0) {
        D_8008ACCC = 0;
        tmp = rand() % 3;
        if (tmp == 1) {
            this->unk44 = 0xB;
        }
        if (tmp == 2) {
            this->unk44 = 0xC;
        }
    }

    out->unk10 = this->methods->getProximityRatio(this);

    if (this->unk90 != 0 && (out->unk4 & 3) == 0) {
        out->unk1C = 0x1C;
    }

    if (this->moodTimer == this->unk80 - 1) {
        this->moodTimer = -1;
    } else {
        if (this->moodTimer >= this->unk80 / 2 + this->unk80 / 4) {
            this->methods->slot12C(this);
        } else if (this->moodTimer >= this->unk80 / 2) {
            if (this->moodTimer == this->unk80 / 2) {
                out->unk1C = 0x10;
            }
            this->methods->slotC4(this, 0x6E, 0);
        } else if (this->moodTimer < this->unk80 / 4) {
            /* nothing */
        } else {
            this->methods->slot130(this);
            this->methods->slotC4(this, -0x6E, 0);
        }
    }

    if (this->unk44 == 0xB && out->unk4 == 0x1FE) {
        this->methods->slotCC(this, -0x17C, 0);
        this->methods->slot44(this, 0, D_80089D0C);
        this->methods->stopSoundCue(this);
        this->unk44 = 1;
        D_8008ACCC = 1;
    } else if (this->unk44 >= 0xC && out->unk4 >= 0x14A && (out->unk4 % 60) == 30) {
        tmp = 0;
        if (rand() & 1) {
            table = D_80089E50;
            tmp = (this->unk44 == 0xC) ? 0x190 : 0;
            this->unk44 = 0xD;
        } else {
            table = D_80089E14;
            if (this->unk44 == 0xD) {
                tmp = -0x190;
            }
            this->unk44 = 0xC;
        }
        this->methods->slot48(this, 1, table);
        this->methods->slotCC(this, tmp, 0);
    }

    if (out->unk4 == 0x208 && D_8008ACCC != 0) {
        this->methods->stopSoundCue(this);
        this->unk44 = 1;
    }
}


extern u8 ROTATION_YAW_PLUS2[];

void func_80062FAC(Entity *this, EntityMoodHandlerArg *out) {
    out->unk10 = this->methods->getProximityRatio(this);
    if (out->unk4 % 10 == 0) {
        out->unk1C = 0x19;
        out->unk20 = 2;
    }
    this->methods->slot44(this, 0, ROTATION_YAW_PLUS2);
    if (this->unk44 == 0 && this->unkF4 != 0) {
        this->methods->notifyParents(this, 0xB);
        this->unk44 = 0xB;
    }
}

extern u8 D_80089D54[];

void func_80063094(Entity *this, EntityMoodHandlerArg *out) {
    if (this->moodTimer < this->unk80) {
        if (this->unk84 != 0) {
            if (this->unk84 == 0x14) {
                out->unk10 = 0;
                out->unk1C = 0x10;
            }
        }
    } else {
        this->methods->slot130(this);
        this->methods->addVec14(this, D_80089D54);
    }
    Class6B5CC__FaceTarget(this, this->target, 1, 0, 0);
}

extern u8 D_80089E08[];

void func_80063144(Entity *this, EntityMoodHandlerArg *out) {
    s32 mod;
    s32 mood;

    if (this->moodTimer == 0 && (rand() & 1)) {
        this->unk44 = rand() % 3;
    }
    if (this->unk44 != 0 && this->unkF4 != 0) {
        if (out->unk4 >= 0x3D) {
            out->unk4 = 0;
        }
        if (out->unk4 % 20 == 0) {
            out->unk1C = 0x17;
            out->unk10 = 0;
            out->unk20 = -1;
            out->unk44 = out->unk30 = 0x17;
            out->unk34 = -1;
            out->unk48 = -2;
        } else if (out->unk4 % 20 == 0xE) {
            out->unk1C = -2;
            out->unk30 = -2;
            out->unk44 = -2;
        }
        Class6B5CC__FaceTarget(this, this->target, 1, 0, 0);
        mood = this->unk44;
        if (mood == 1) {
            this->methods->slot48(this, 0, D_80089E08);
            mod = -0x176;
            if (this->methods->slot144(this, this->target) < 0x200) {
                this->methods->deactivate(this);
                this->unk44 = mood;
            }
        } else {
            this->target->methods->slot130(this->target, 1);
            Class6B5CC__FaceTarget((Entity *)this->target, this, 1, 1, 0);
            if (this->unk44 == 2) {
                if (this->methods->slot144(this, this->target) < 0x960) {
                    this->unk44 = 0xB;
                    this->methods->notifyParents(this, 0xC);
                }
                mod = -0x60;
            } else {
                mod = 0;
            }
        }
    } else {
        out->unk10 = this->methods->getProximityRatio(this);
        if (out->unk4 % 22 == 0) {
            out->unk1C = 0x1C;
        }
        if (this->moodTimer >= 0x1F5) {
            Class6B5CC__FaceTarget(this, this->target, 1, 0, 0);
        }
        if (this->methods->slot144(this, this->target) < 0x800) {
            this->target->methods->slotC4(this->target, -0x800, 0);
        }
        mod = -0x14;
    }
    this->methods->slotD0(this, mod, 1);
    if (this->unk28 != 0) {
        this->methods->slotCC(this, -0xC8, 0);
    }
}

