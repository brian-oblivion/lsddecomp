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

INCLUDE_ASM("asm/nonmatchings/Entity_d", func_800603C4);

INCLUDE_ASM("asm/nonmatchings/Entity_d", func_800604DC);

INCLUDE_ASM("asm/nonmatchings/Entity_d", func_800605D0);

INCLUDE_ASM("asm/nonmatchings/Entity_d", func_80060710);

void func_800607F8(void) {
}

INCLUDE_ASM("asm/nonmatchings/Entity_d", func_80060800);

INCLUDE_ASM("asm/nonmatchings/Entity_d", func_8006090C);

INCLUDE_ASM("asm/nonmatchings/Entity_d", func_80060A4C);

INCLUDE_ASM("asm/nonmatchings/Entity_d", func_80060B34);

INCLUDE_ASM("asm/nonmatchings/Entity_d", func_80060CF0);

INCLUDE_ASM("asm/nonmatchings/Entity_d", func_80060D80);

INCLUDE_ASM("asm/nonmatchings/Entity_d", func_80060F38);

INCLUDE_ASM("asm/nonmatchings/Entity_d", func_80061070);

INCLUDE_ASM("asm/nonmatchings/Entity_d", func_80061158);

INCLUDE_ASM("asm/nonmatchings/Entity_d", func_80061198);

INCLUDE_ASM("asm/nonmatchings/Entity_d", func_80061400);

INCLUDE_ASM("asm/nonmatchings/Entity_d", func_80061778);
