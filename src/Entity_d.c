#include "common.h"
#include "Entity.h"

extern s16 D_80089EA2;

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
 * handlers -- same convention as Entity_c.c's own D_80089E50/D_80089E38/etc
 * externs (separate local view per translation unit, not shared via the
 * header). */
extern u8 D_80089C64[];
extern u8 D_80089C70[];
extern u8 D_80089E8C[];
extern u8 D_80089DC0[];
extern u8 D_80089E50[];

/* Forward declaration: func_80060710 is defined later in this file (higher
 * ROM address) but func_800604DC, at a lower address, calls it directly. */
void func_80060710(Entity *this);

INCLUDE_ASM("asm/nonmatchings/Entity_d", func_8005FF7C);

INCLUDE_ASM("asm/nonmatchings/Entity_d", func_80060148);

void func_800602AC(Entity *this, EntityMoodHandlerArg *out) {
    s32 rv;
    s16 *tablePtr;

    if (this->unkFC == 0) {
        this->unk44 = rand() % 5 + 0xA;
    }
    if (this->unk44 < 0xE || this->unkFC < 0x140) {
        func_80064FBC(this, out, 0xBB8, 0x1F4, -0x100);
        return;
    }
    if (this->unk44 == 0xE) {
        if ((this->unkFC & 3) == 0) {
            rv = rand();
            tablePtr = &D_80089EA2;
            *tablePtr = rv % 32 + 1;
            this->methods->slot48(this, 1, (u8 *)tablePtr - 0xA);
        }
    }
}

void func_800603C4(Entity *this, EntityMoodHandlerArg *out) {
    s32 divisor;

    if (this->unkFC < 0x14) {
        this->methods->slot130(this);
        this->methods->slotC4(this, -0x1E, 0);
    } else if (this->unkFC == 0x14) {
        this->methods->slot12C(this);
        out->unk10 = 0;
        out->unk1C = 5;
    } else {
        divisor = this->unk80 * 3 + 0x14;
        if (this->unkFC % divisor == 0) {
            this->methods->slot130(this);
            out->unk1C = -2;
        }
    }
}

void func_800604DC(Entity *this, EntityMoodHandlerArg *out) {
    s32 a1val;
    s32 r2;
    u8 *table;

    func_80060710(this);
    out->unk10 = this->methods->slot148(this);
    if (this->unk84 == 0 || this->unk84 == 0xF) {
        out->unk1C = 0x12;
        out->unk30 = 0x12;
    }
    if (this->unkFC >= 0x141) {
        a1val = (rand() & 1) ? -0x3C : 0x3C;
        this->methods->slotC8(this, a1val, 0);
        r2 = rand();
        table = D_80089C64;
        if ((r2 & 3) != 0) {
            table = D_80089C70;
        }
        this->methods->slot44(this, 0, table);
    }
}

INCLUDE_ASM("asm/nonmatchings/Entity_d", func_800605D0);

void func_80060710(Entity *this) {
    s32 r;

    if (this->unkFC == 0) {
        r = rand() % 10;
        if (r >= 8) {
            this->methods->slot48(this, 1, D_80089E8C);
        } else if (r >= 5) {
            this->unk44 = 0xA;
        }
    }
    if (this->unk44 == 0xA && this->unkFC >= 0xC9) {
        this->methods->slotBC(this, D_80089DC0);
    }
}

void func_800607F8(void) {
}

INCLUDE_ASM("asm/nonmatchings/Entity_d", func_80060800);

INCLUDE_ASM("asm/nonmatchings/Entity_d", func_8006090C);

void func_80060A4C(Entity *this, EntityMoodHandlerArg *out) {
    s32 a1val;
    EntityMethods *methods;

    if (this->unk84 == 0x26) {
        func_8001EACC(this, this->unk94, 1, 0, 0);
        out->unk10 = this->methods->slot148(this);
        out->unk1C = 6;
    }
    methods = this->methods;
    a1val = (this->unkFC % 10 < 5) ? -0x1E : 0x1E;
    methods->slotCC(this, a1val, 0);
    this->methods->slotC4(this, -0x1E, 1);
}

INCLUDE_ASM("asm/nonmatchings/Entity_d", func_80060B34);

void func_80060CF0(Entity *this, EntityMoodHandlerArg *out) {
    if (this->unkFC < this->unk80 * 5) {
        if (this->unk84 == 0xF || this->unk84 == 0x46) {
            out->unk10 = 0;
            out->unk1C = 7;
            out->unk30 = 7;
            out->unk44 = 7;
        }
    } else {
        this->methods->slot160(this);
        this->unk44 = 1;
    }
}

INCLUDE_ASM("asm/nonmatchings/Entity_d", func_80060D80);

INCLUDE_ASM("asm/nonmatchings/Entity_d", func_80060F38);

void func_80061070(Entity *this, EntityMoodHandlerArg *out) {
    s32 mood = this->unk84;

    if (this->unkFC == 0) {
        if (rand() % 3 == 0) {
            this->methods->slot48(this, 1, D_80089E50);
        }
    }
    out->unk10 = this->methods->slot148(this);
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

INCLUDE_ASM("asm/nonmatchings/Entity_d", func_80061158);

INCLUDE_ASM("asm/nonmatchings/Entity_d", func_80061198);

INCLUDE_ASM("asm/nonmatchings/Entity_d", func_80061400);

INCLUDE_ASM("asm/nonmatchings/Entity_d", func_80061778);
