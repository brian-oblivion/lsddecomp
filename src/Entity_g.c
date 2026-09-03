#include "common.h"
#include "Entity.h"

/* Data rows this unit's mood-dispatch handlers pass through to a vtable
 * call as an opaque argument -- never dereferenced here, so an opaque byte
 * array is enough to form &D_8008xxxx correctly. Real element type/count
 * unknown. Same per-unit local-declaration convention as Entity_b.c/
 * Entity_c.c/Entity_e.c (each unit keeps its own extern, not shared). */
extern u8 D_80089CAC[];
extern u8 D_80089E14[];
extern u8 D_80089DE4[];
extern u8 D_80089DD8[];
extern u8 D_80089E20[];
extern u8 D_80089DCC[];
extern u8 D_80089CB8[];
extern u8 D_80089E2C[];
extern u8 D_80089E38[];
extern u8 D_80089E44[];
extern u8 D_80089D00[];
extern u8 D_80089D18[];
extern u8 D_80089C70[];
extern u8 D_80089C64[];

/* func_80064FBC, this unit's own function, is called by func_80064E34
 * (earlier in ROM order) before its own definition below -- forward
 * declaration, same convention CLAUDE.md documents for calling into a
 * still-INCLUDE_ASM function. Already known cross-unit from Entity_d.c's
 * own extern (func_80060148's caller there), reproduced here matching. */
extern void func_80064FBC(Entity *this, EntityMoodHandlerArg *out, s32 arg2, s32 arg3, s32 arg4);

void func_80064618(Entity *this, EntityMoodHandlerArg *out) {
    if (this->unkF4 != 0) {
        if (func_8005D108(this, NULL, 0, 0xA, 0) != 0) {
            this->unk100->methods->slotD4(this->unk100, this->unk50, 7, 0);
            this->methods->slot160(this);
            this->unk94->methods->slot21C(this->unk94);
        }
    }
    this->methods->slotC4(this, -0x1E, 1);
}

void func_800646D8(Entity *this, EntityMoodHandlerArg *out) {
    void *a2;

    if (this->unkFC == 0) {
        this->methods->slotCC(this, -0x200, 0);
    }
    out->unk10 = this->methods->slot148(this);
    if (this->unk84 == this->unk80 / 2) {
        out->unk1C = 7;
        out->unk20 = -2;
        out->unk30 = 3;
        out->unk34 = -2;
    }
    if (this->unkFC >= 0x33) {
        this->methods->slot44(this, 0, D_80089CAC);
    }
    if (this->unkFC >= 0x30D) {
        func_8001EACC(this, this->unk94, 1, 0, 0);
        if (this->unkFC >= 0x790) {
            a2 = D_80089E14;
        } else if (this->unkFC >= 0x78B) {
            a2 = D_80089DE4;
        } else if (this->unkFC >= 0x786) {
            a2 = D_80089DD8;
        } else if (this->unkFC >= 0x781) {
            a2 = D_80089DCC;
        } else {
            a2 = D_80089E20;
        }
        this->methods->slot48(this, 1, a2);
        if (this->unkFC < 0x7D0) {
            this->methods->slotC4(this, -0x40, 0);
        } else {
            this->unk44 = 1;
        }
    } else {
        this->methods->slotC4(this, -0x100, 0);
    }
    if (this->unkF4 != 0 && this->unk44 == 0) {
        this->unk44 = 0xC;
        this->unk94->methods->slot130(this->unk94, 1);
        this->methods->slot30(this, 0xA);
    }
    if (this->unk44 == 0xC) {
        this->unk94->methods->slotC4(this->unk94, 0x100, 0);
    }
}

void func_80064928(Entity *this, EntityMoodHandlerArg *out) {
    if (this->unkFC == 0x2BC) {
        if (rand() % 3 == 0) {
            this->unk44 = 0xB;
        }
    }
    if (this->unk44 == 0xB) {
        if (this->unkFC < 0x3FC) {
            this->methods->slot44(this, 0, D_80089CB8);
            this->methods->slotCC(this, 0x1E, 0);
        }
        if (this->unkFC == 0x3A2) {
            this->methods->slot30(this, 0xA);
        }
    } else if (this->unkFC == 0x64 || this->unkFC == 0x320) {
        if (rand() % 5 == 0) {
            this->unk4C->methods->slot138(this->unk4C, 4, 0);
        }
    }
    this->methods->slotC4(this, -0x1E, 0);
}

void func_80064AA4(Entity *this, EntityMoodHandlerArg *out) {
    this->methods->slot48(this, 1, D_80089DCC);
    if ((u32)(this->unkFC - 0xC9) < 0x63) {
        this->methods->slotCC(this, -0x20, 0);
    }
}

void func_80064B14(Entity *this, EntityMoodHandlerArg *out) {
    this->methods->slot60(this, rand() % 20 == 0);
}

void func_80064B80(Entity *this, EntityMoodHandlerArg *out) {
    if (this->unkFC == 0) {
        if ((rand() & 1) == 0) {
            this->unk44 = 0xB;
        }
    }
    if (this->unk44 == 0xB) {
        void *fn;

        if (this->unkFC == 0) {
            this->methods->slot130(this);
        }
        fn = this->methods->slot48;
        ((void (*)(Entity *, s32, void *))fn)(this, 1, D_80089E2C);
        return;
    }
    if (this->unkFC == 0) {
        this->methods->slot128(this, 1);
    }
    this->methods->slotC8(this, (this->unkFC % 20 < 10) ? 0x20 : -0x20, 0);
}

void func_80064CA4(Entity *this, EntityMoodHandlerArg *out) {
    func_80062570(this, out);
    this->methods->slot48(this, 1, D_80089E38);
}

void func_80064CEC(Entity *this, EntityMoodHandlerArg *out) {
    this->methods->slot48(this, 1, D_80089DD8);
    this->methods->slotC4(this, -0xA, 0);
}

void func_80064D48(Entity *this, EntityMoodHandlerArg *out) {
    this->methods->slot48(this, 1, D_80089E44);
    this->methods->slot130(this);
    if (this->unk44 == 0) {
        if (this->methods->slot144(this, this->unk94) < 0x800) {
            this->unk44 = 0xA;
            this->unkFC = 0;
        }
    }
    if (this->unk44 == 0xA) {
        if (this->unkFC < 0x2D) {
            this->methods->slot44(this, 0, D_80089D00);
        }
        if (this->unkFC >= 0x1F5) {
            this->unk44 = 0;
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/Entity_g", func_80064E34);

INCLUDE_ASM("asm/nonmatchings/Entity_g", func_80064FBC);

void func_800650D4(Entity *this, EntityMoodHandlerArg *out) {
    func_80060D80(this, out);
}

void func_800650F4(Entity *this, EntityMoodHandlerArg *out) {
    void *a2;

    if (rand() % 3 == 0) {
        return;
    }
    if (rand() % 3 != 0) {
        a2 = D_80089C64;
    } else {
        a2 = D_80089C70;
    }
    this->methods->slot44(this, 0, a2);
}

void func_8006519C(Entity *this, EntityMoodHandlerArg *out) {
    this->methods->slot48(this, 1, D_80089E38);
}

void func_800651D0(Entity *this, EntityMoodHandlerArg *out) {
    this->methods->slot48(this, 1, D_80089E38);
}

void func_80065204(Entity *this, EntityMoodHandlerArg *out) {
    this->methods->slot48(this, 1, D_80089DCC);
}

INCLUDE_ASM("asm/nonmatchings/Entity_g", func_80065238);

INCLUDE_ASM("asm/nonmatchings/Entity_g", func_8006536C);

INCLUDE_ASM("asm/nonmatchings/Entity_g", func_800654A0);

INCLUDE_ASM("asm/nonmatchings/Entity_g", func_80065514);
