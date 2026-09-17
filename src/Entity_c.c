/* Third slice of the Entity block -- 20 functions, 0x4F754..0x5077C. The
 * remainder is `Entity_d` and is still a monolithic asm segment.
 *
 * The whole 97-function remainder this came out of has zero `jlabel`s and
 * zero `jr $t2`, so no slice of it needs a rodata slot attached and none of
 * it is a BIOS trampoline. Entity/Entity_b's `include/Entity.h` is already
 * heavily typed and these functions are the same class family -- extend that
 * header rather than starting a new one.
 */
#include "common.h"
#include "Entity.h"

extern u8 D_80089E38[];
extern u8 D_80089E50[];
extern u8 D_80089DF0[];
extern u8 D_80089DCC[];
extern u8 D_80089CA0[];
extern u8 D_80089C64[];
extern u8 D_80089C70[];
extern u8 D_80089C58[];
extern u8 D_80089E74[];
extern u8 D_80089D18[];
extern u8 D_80089DB4[];
extern u8 D_80089D78[];
extern u8 D_80089D9C[];
extern u8 D_80089D48[];
extern u8 D_80089D60[];

void func_8005EF54(Entity *this, EntityMoodHandlerArg *out) {
    out->unk10 = this->methods->slot148(this);
    if (out->unk4 % 10 == 0) {
        out->unk1C = 0x11;
    }
    this->methods->slotC4(this, -0x100, 0);
}

void func_8005EFF4(Entity *this, EntityMoodHandlerArg *out) {
    if (this->unkFC == 0 && rand() % 7 == 0) {
        this->methods->slot48(this, 1, D_80089E50);
    }
    if ((out->unk4 & 3) == 0) {
        out->unk10 = this->methods->slot148(this);
        out->unk1C = 0x1C;
    }
    this->methods->slotC4(this, -0x64, 0);
}

void func_8005F0D8(Entity *this, EntityMoodHandlerArg *out) {
    s32 half;
    s32 rem;

    out->unk10 = this->methods->slot148(this);
    half = this->unk80 / 2;
    rem = out->unk4 % half;
    if (rem == 0) {
        out->unk1C = 0xA;
    } else if (rem == 3) {
        out->unk30 = 0xD;
    }
    this->methods->slotC4(this, -0x1E, 1);
}

void func_8005F1A8(Entity *this) {
    Class6B5CC__FaceTarget(this, this->unk94, 1, 0, 0);
}

void func_8005F1D4(Entity *this) {
    s32 arg1;

    Class6B5CC__FaceTarget(this, this->unk94, 1, 0, 0);

    if (this->unkF4 != 0) {
        if (this->unkFC >= 0x41) {
            this->unkFC = 0;
        }
        if (this->unkFC >= 7) {
            this->methods->slotBC(this, D_80089D78);
            this->methods->slotC4(this, 0xA, 0);
        } else {
            this->methods->slotBC(this, D_80089D9C);
        }
    } else if (this->unkFC == 0) {
        this->methods->slotBC(this, D_80089D48);
    } else if (this->unkFC < 0x41) {
        this->methods->slotBC(this, D_80089D60);
    } else if (this->unkFC < 0x47) {
        this->methods->slotBC(this, D_80089D78);
        this->methods->slotC4(this, -0x1E, 0);
    } else {
        EntityMethods *methods = this->methods;

        if (this->unkFC < 0x100) {
            arg1 = -this->unkFC - 0x41;
        } else {
            arg1 = 0xFF;
        }
        methods->slotC4(this, arg1, 0);
    }
}

void func_8005F368(Entity *this, EntityMoodHandlerArg *out) {
    if (out->unk4 % 15 == 0) {
        out->unk10 = this->methods->slot148(this);
        out->unk1C = 7;
        out->unk20 = -2;
    }
    this->methods->slot48(this, 1, D_80089DF0);
    this->methods->slot44(this, 0, D_80089CA0);
    this->methods->slotC4(this, -0x200, 0);
}

void func_8005F454(Entity *this, EntityMoodHandlerArg *out) {
    out->unk10 = 0;
    if (this->unkFC < 0x64) {
        if (out->unk4 % 3 == 0) {
            out->unk1C = 0x16;
            out->unk20 = 1;
        }
        this->methods->slotCC(this, -0x40, 0);
    } else if (this->unkFC < 0x12C) {
        out->unk1C = 0xC;
        out->unk20 = -1;
        out->unk30 = 0xC;
        out->unk34 = -1;
        out->unk44 = 0xC;
        out->unk48 = -1;
        this->methods->slotCC(this, -0x100, 0);
    } else {
        this->methods->slot44(this, 0, D_80089C58);
        this->methods->slotCC(this, -0x200, 0);
    }
}

void func_8005F544(Entity *this, EntityMoodHandlerArg *out) {
    s32 v1;
    s32 arg1;
    void (**slotD0)(Entity *self, s32 arg1, s32 arg2);

    /* The do/while(0) wrapper is a no-op scoping device, load-bearing for
     * register allocation only -- see the match report. Without it GCC
     * swaps which callee-saved register holds `this` vs `out` for the
     * whole function. */
    do {
        if (out->unk4 % this->unk80 == 0) {
            out->unk10 = this->methods->slot148(this);
            out->unk1C = 0x1A;
            __asm__("");
            v1 = 0x6E;
            goto compare;
        }
    } while (0);
    v1 = 0x6E;
compare:
    slotD0 = &this->methods->slotD0;
    arg1 = -0x180;
    if (this->unkFC == v1) {
        arg1 = -0x2D00;
    }
    (*slotD0)(this, arg1, 0);
}

void func_8005F608(Entity *this, EntityMoodHandlerArg *out) {
    if (out->unk4 % 70 == 0) {
        out->unk10 = 0;
        out->unk1C = 0x1B;
    }
    this->methods->slotC4(this, -0x80, 0);
    if (this->unkFC < 0x64) {
        this->methods->slotCC(this, 0x20, 0);
    } else if (this->unkFC >= 0x12D) {
        this->methods->slotCC(this, -0x20, 0);
    }
}

s32 func_8005F6D4(Entity *this) {
    return this->methods->slot48(this, 1, D_80089E38);
}

void func_8005F708(Entity *this) {
    if (this->unkFC == 0) {
        if (this->unk94->methods->slot200(this->unk94) == 7) {
            this->methods->slot48(this, 1, D_80089E74);
            this->methods->slotCC(this, -0x7800, 0);
        }
        this->unk44 = rand() % 5;
    }
    if (this->unk44 == 0) {
        this->methods->slot44(this, 0, D_80089D18);
    }
}

void func_8005F800(Entity *this) {
    if (this->unk44 == 0) {
        if (this->unk94->methods->slot200(this->unk94) == 1) {
            this->unk44 = 0xB;
        } else {
            this->unk44 = 0xC;
        }
    }

    if (this->unk44 == 0xC) {
        this->methods->slot48(this, 1, D_80089DF0);
        this->methods->slotCC(this, -0x1E, 0);
    } else {
        Class6B5CC__FaceTarget(this, this->unk94, 1, 0, 0);
        if (this->unk44 == 0xB) {
            this->methods->slotC4(this, -0x64, 0);
            if ((u32)(this->unkFC - 0x55) < 0x1E) {
                this->methods->slotCC(this, 0x50, 0);
            } else if (this->unkFC == 0x78) {
                this->unk44 = 0xD;
            }
        } else if (this->unk44 == 0xD) {
            this->methods->slotB8(this, &this->unk94->unk14->x);
            this->methods->slotBC(this, D_80089DB4);
        }
    }
}

void func_8005F970(Entity *this, EntityMoodHandlerArg *out) {
    Class6B5CC__FaceTarget(this, this->unk94, 1, 0, 0);
    this->unk94->methods->slot130(this->unk94, 1);
    if ((out->unk4 % 10) < 3) {
        out->unk10 = 0;
        out->unk1C = 0xD;
        out->unk30 = 0xD;
        out->unk44 = 0xD;
    }
    if (this->unkFC == this->unk80) {
        this->methods->slot130(this);
        this->methods->slot30(this, 0xA);
    }
}

void func_8005FA64(Entity *this) {
    this->methods->slotC4(this, -0x1E, 0);
}

void func_8005FA94(Entity *this, EntityMoodHandlerArg *out) {
    if (this->unkF4 != 0) {
        this->methods->slot48(this, 1, D_80089DCC);
    } else if (out->unk4 % 30 == 0) {
        out->unk10 = 0;
        out->unk1C = 3;
    }
    this->methods->slotD0(this, -0x1E, 0);
    if (this->unk28 != 0) {
        this->methods->slotCC(this, -0xC8, 0);
    }
}

void func_8005FB6C(Entity *this) {
    EntityMethods *methods;
    s32 arg1;

    if ((u32)(this->unkFC - 0x190) < 0xA) {
        this->methods->slot44(this, 0, D_80089C64);
    } else if ((u32)(this->unkFC - 0x2BC) < 0xA) {
        this->methods->slot44(this, 0, D_80089C70);
    } else if ((u32)(this->unkFC - 0x33E) < 0x4) {
        this->methods->slot44(this, 0, D_80089C70);
    } else if (this->unkFC >= 0x353) {
        this->methods->slot160(this);
    }
    methods = this->methods;
    arg1 = -0x200;
    if (this->unkFC < 0x320) {
        arg1 = -0x3C;
    }
    methods->slotC4(this, arg1, 1);
}

void func_8005FC58(Entity *this) {
    s32 rem500;
    s32 arg1a;
    s32 arg1b;
    s32 arg1c;
    s32 (**slotCC)(Entity *self, s32 arg1, s32 arg2);
    void (**slotC8)(Entity *self, s32 arg1, s32 arg2);
    void (**slotC4)(Entity *self, s32 arg1, s32 arg2);

    rem500 = this->unkFC % 500;

    slotCC = &this->methods->slotCC;
    if (this->unkFC % 6 < 3) {
        arg1a = -0x40;
    } else {
        arg1a = 0x40;
    }
    (*slotCC)(this, arg1a, 0);

    slotC8 = &this->methods->slotC8;
    if (this->unkFC % 12 < 6) {
        arg1b = -0x40;
    } else {
        arg1b = 0x40;
    }
    (*slotC8)(this, arg1b, 0);

    slotC4 = &this->methods->slotC4;
    if (this->unkFC % 64 < 0x20) {
        arg1c = -0x80;
    } else {
        arg1c = 0x80;
    }
    (*slotC4)(this, arg1c, 0);

    if (rem500 < 0x20) {
        this->methods->slotBC(this, D_80089D78);
    } else if (rem500 < 0x40) {
        this->methods->slotBC(this, D_80089D60);
    }
}

void func_8005FDFC(Entity *this) {
    u8 *arg2;
    s32 roll;

    if (this->unk44 == 0) {
        roll = rand();
        arg2 = D_80089E38;
        if ((roll & 1) != 0) {
            arg2 = D_80089DF0;
        }
        this->methods->slot48(this, 1, arg2);
        this->unk44 = 0xB;
    }
    Class6B5CC__FaceTarget(this, this->unk94, 1, 0, 0);
    if (this->methods->slot144(this, this->unk94) < 0x7000) {
        this->methods->slotC4(this, 0x100, 0);
    }
}

s32 func_8005FEC8(Entity *this) {
    return this->methods->slotCC(this, -0x5A, 0);
}

void func_8005FEF8(Entity *this, EntityMoodHandlerArg *out) {
    if (out->unk4 % 120 == 0) {
        out->unk10 = this->methods->slot148(this);
        out->unk1C = 1;
    }
}
