#include "common.h"
#include "Entity.h"

INCLUDE_ASM("asm/nonmatchings/Entity_e", func_80061A90);

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

INCLUDE_ASM("asm/nonmatchings/Entity_e", func_80061F30);

void func_8006204C(Entity *this, EntityMoodHandlerArg *out) {
    out->unk10 = this->methods->slot148(this);
    if (out->unk4 % 30 == 0) {
        out->unk1C = 0xD;
    }
}

INCLUDE_ASM("asm/nonmatchings/Entity_e", func_800620C4);

INCLUDE_ASM("asm/nonmatchings/Entity_e", func_800621A8);

INCLUDE_ASM("asm/nonmatchings/Entity_e", func_800623E8);

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

INCLUDE_ASM("asm/nonmatchings/Entity_e", func_80062570);

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

INCLUDE_ASM("asm/nonmatchings/Entity_e", func_80062730);

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

INCLUDE_ASM("asm/nonmatchings/Entity_e", func_80062A40);

INCLUDE_ASM("asm/nonmatchings/Entity_e", func_80062C58);

INCLUDE_ASM("asm/nonmatchings/Entity_e", func_80062FAC);

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
