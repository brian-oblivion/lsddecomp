#include "common.h"
#include "Entity.h"

void func_80061A90(Entity *this, EntityMoodHandlerArg *out) {
    if (this->unkFC == 0 && rand() % 10 == 0) {
        this->unk44 = 0xC;
    }
    if (out->unk4 % 10 == 0) {
        out->unk10 = this->methods->slot148(this);
        out->unk1C = 0xC;
        out->unk20 = -1;
    }
    if (this->unkFC == 0) {
        if (rand() & 1) {
            this->methods->slotCC(this, 0x800, 0);
        }
    }
    this->methods->slotC4(this, -0x80, 0);
    if (this->unk44 == 0xC && this->unkFC == 0x12C) {
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

INCLUDE_ASM("asm/nonmatchings/Entity_e", func_80061C2C);

void func_80061E60(Entity *this, EntityMoodHandlerArg *out) {
    s32 r = out->unk4 % 300;

    out->unk10 = this->methods->slot148(this);
    if (r < 0x14) {
        out->unk1C = 5;
        out->unk20 = -2;
    } else if (r == 0x16) {
        out->unk1C = -2;
    }
    this->methods->slotC4(this, -0xA, 0);
}

extern u8 D_80089DD8[];
extern u8 D_80089C88[];
extern u8 D_80089C94[];

void func_80061F30(Entity *this, EntityMoodHandlerArg *out) {
    if (this->unkFC == 0) {
        if (rand() % 3 == 0) {
            this->methods->slot48(this, 1, D_80089DD8);
            this->methods->slotCC(this, -0x12C, 0);
            this->methods->slot44(this, 1, D_80089C88);
            this->unk44 = 0xB;
        }
    }
    if (this->unk44 == 0xB) {
        if (this->unkFC == 0x7D0) {
            this->methods->slot44(this, 0, D_80089C94);
        }
        this->methods->slotC4(this, -0x14, 0);
    }
}

void func_8006204C(Entity *this, EntityMoodHandlerArg *out) {
    out->unk10 = this->methods->slot148(this);
    if (out->unk4 % 30 == 0) {
        out->unk1C = 0xD;
    }
}

void func_800620C4(Entity *this, EntityMoodHandlerArg *out) {
    if (this->unkFC == 0) {
        if (rand() % 3 == 0) {
            this->unk44 = 0xB;
        }
    }
    if (this->unk44 == 0xB) {
        if (this->unkFC == 0x1F6) {
            this->methods->slotCC(this, 0x800, 0);
            func_8001EACC(this, this->unk94, 1, 0, 0);
        }
        if (this->unkFC >= 0x1F5) {
            this->methods->slotC4(this, -0x200, 0);
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/Entity_e", func_800621A8);

void func_800623E8(Entity *this, EntityMoodHandlerArg *out) {
    out->unk10 = this->methods->slot148(this);
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
    if (this->unkFC == 0) {
        if (rand() & 1) {
            this->unk44 = 0xB;
        }
    }
    if (this->unkFC == 0x12C) {
        this->methods->slot44(this, 0, D_80089C7C);
    }
    if (this->unkFC < 0x258) {
        this->methods->slotC4(this, this->unk44 == 0 ? -0x100 : 0x100, 0);
    }
}

void func_80062570(Entity *this, EntityMoodHandlerArg *out) {
    s32 r;

    out->unk10 = this->methods->slot148(this);
    if (out->unk4 == 0) {
        out->unk1C = 0;
        r = rand() % 3;
        this->methods->slotC8(this, r * 51200, 0);
    }
    if (this->unkFC >= 0x961) {
        func_8001EACC(this, this->unk94, 1, 0, 0);
    }
    this->methods->slotC4(this, -0x1E, 0);
}

extern u8 D_80089C88[];

void func_80062660(Entity *this, EntityMoodHandlerArg *out) {
    out->unk10 = 0;
    if (out->unk4 == 0) {
        this->unk94->methods->slot44(this->unk94, 1, D_80089C88);
        this->unk94->methods->slot130(this->unk94, 1);
        out->unk1C = 0x19;
        out->unk30 = 0x19;
        out->unk44 = 0x19;
    } else if (out->unk4 == 0x14) {
        out->unk30 = 0xD;
    }
    if (this->unkFC == this->unk80 - 1) {
        this->methods->slot160(this);
    }
}

extern u8 D_8008AC1C[];

void func_80062730(Entity *this, EntityMoodHandlerArg *out) {
    if (this->unkFC == 0) {
        this->unk94->unk5C->methods->slot64(this->unk94->unk5C, D_8008AC1C);
        this->unk44 = rand() % 3;
        if (this->unk94->unk14->z < 0x262) {
            this->unk44 = 0;
        }
    }
    if (this->unk44 != 0) {
        if (this->unk80 / 2 < this->unkFC) {
            this->unk94->methods->slotC4(this->unk94, 0x80, 0);
        }
        if (this->unkFC == this->unk80 - 0x1E) {
            this->methods->slot30(this, 0xA);
        }
    } else {
        if ((u32)(this->unkFC - 0x14) < 0x64) {
            this->unk94->methods->slotC4(this->unk94, -((this->unkFC - 0x13) * 0x20), 1);
            if (this->unkFC == 0x55) {
                this->unk94->methods->slot134(this->unk94, 1, 1);
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
        func_8001EACC(this, this->unk94, 1, 0, 0);
    }
    if (this->unkFC == this->unk80) {
        this->methods->slot130(this);
        this->methods->slot30(this, 0xA);
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
    out->unk10 = this->methods->slot148(this);
    if (out->unk4 % 5 == 0) {
        out->unk1C = 0x11;
        out->unk20 = -2;
    }
    if (this->unk7C == 0) {
        if (this->unkFC == this->unk80) {
            this->methods->slot128(this, 1);
            if (rand() & 1) {
                this->unk44 = 0xB;
            }
        }
        return;
    }
    if (this->unk44 == 0) {
        if (this->unkFC == 0x3C || this->unkFC == 0xD4 || this->unkFC == 0x122 || this->unkFC == 0x140) {
            this->methods->slot44(this, 0, D_80089C88);
        }
        if (this->unkFC == 0x18E) {
            this->methods->slot44(this, 0, D_80089C94);
        }
        this->methods->slotD0(this, -0x32, 0);
        return;
    }
    if (this->unkFC == 0x3C || this->unkFC == 0x8C) {
        this->methods->slot44(this, 0, D_80089C88);
    }
    if (this->unkFC < 0xAE) {
        this->methods->slotD0(this, -0x32, 0);
    }
    if (this->unkFC == 0xAE) {
        this->methods->slot16C(this);
        this->unk44 = 1;
    }
}

INCLUDE_ASM("asm/nonmatchings/Entity_e", func_80062C58);

extern u8 D_80089CA0[];

void func_80062FAC(Entity *this, EntityMoodHandlerArg *out) {
    out->unk10 = this->methods->slot148(this);
    if (out->unk4 % 10 == 0) {
        out->unk1C = 0x19;
        out->unk20 = 2;
    }
    this->methods->slot44(this, 0, D_80089CA0);
    if (this->unk44 == 0 && this->unkF4 != 0) {
        this->methods->slot30(this, 0xB);
        this->unk44 = 0xB;
    }
}

extern u8 D_80089D54[];

void func_80063094(Entity *this, EntityMoodHandlerArg *out) {
    if (this->unkFC < this->unk80) {
        if (this->unk84 != 0) {
            if (this->unk84 == 0x14) {
                out->unk10 = 0;
                out->unk1C = 0x10;
            }
        }
    } else {
        this->methods->slot130(this);
        this->methods->slotBC(this, D_80089D54);
    }
    func_8001EACC(this, this->unk94, 1, 0, 0);
}

INCLUDE_ASM("asm/nonmatchings/Entity_e", func_80063144);
