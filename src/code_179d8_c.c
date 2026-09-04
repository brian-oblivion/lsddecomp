/*
 * code_179d8_c -- window [200..219] of the original 274-function code_179d8
 * monolith, 0x22948..0x23500 (vram 0x80032148..0x80032D00).
 *
 * Carved round 16 by blocker DENSITY (see code_179d8_b's header for the
 * full window census). This window screened 16/20 clean. The four blocked
 * functions are already stubbed as match reports, all addiu_at:
 *   func_80032148, func_80032588, func_80032BF0, func_80032C28
 *
 * func_80032588 owns jtbl_80010CD8, whose sub-slot of the 0xFD8 rodata
 * region is ATTACHED to this unit in the splat yaml. Leave that alone.
 *
 * Note func_80032AD0/SetRCnt: this window holds what look like Psy-Q root
 * counter routines linked into game text rather than into a psyq_* segment.
 * They are ordinary work, but do not generalise a finding from them to the
 * game's own code.
 *
 * This slice was cut at ROM-address boundaries, so it has no reason to
 * align with class boundaries -- expect it to span more than one class,
 * and identify each with tools/classtable.py rather than assuming one.
 *
 * Declarations: keep anything that encodes THIS unit's reading of a class
 * next to the code, in this file. Do not create a shared code_179d8*.h --
 * the sibling slices are staffed independently and a shared header is what
 * makes their merges collide.
 */
#include "common.h"

INCLUDE_ASM("asm/nonmatchings/code_179d8_c", func_80032148);

INCLUDE_ASM("asm/nonmatchings/code_179d8_c", func_8003221C);

extern void func_8003221C(s32 arg0);

void func_80032368(void)
{
    func_8003221C(0);
}

void func_80032388(void)
{
    func_8003221C(1);
}

INCLUDE_ASM("asm/nonmatchings/code_179d8_c", func_800323A8);

INCLUDE_ASM("asm/nonmatchings/code_179d8_c", func_80032588);

INCLUDE_ASM("asm/nonmatchings/code_179d8_c", func_80032708);

extern void func_80032708(s32 arg0);

void func_80032998(void)
{
    func_80032708(1);
}

void func_800329B8(void)
{
    func_80032708(0);
}

INCLUDE_ASM("asm/nonmatchings/code_179d8_c", func_800329D8);

extern void func_80038FB0(void);

void func_80032A7C(void)
{
    func_80038FB0();
}

extern void func_80033738(void);
extern void (*D_8006DC9C)(void);

void func_80032A9C(void)
{
    if (D_8006DC9C != NULL) {
        D_8006DC9C();
    }
    func_80033738();
}

extern s32 D_8006DCA0;

void func_80032AD0(void)
{
    if (D_8006DCA0 == 0) {
        D_8006DCA0 = 1;
    } else {
        D_8006DCA0 = 0;
        func_80033738();
    }
}

INCLUDE_ASM("asm/nonmatchings/code_179d8_c", SetRCnt);

INCLUDE_ASM("asm/nonmatchings/code_179d8_c", func_80032BB8);

INCLUDE_ASM("asm/nonmatchings/code_179d8_c", func_80032BF0);

INCLUDE_ASM("asm/nonmatchings/code_179d8_c", func_80032C28);

INCLUDE_ASM("asm/nonmatchings/code_179d8_c", func_80032C60);

INCLUDE_ASM("asm/nonmatchings/code_179d8_c", func_80032C98);

INCLUDE_ASM("asm/nonmatchings/code_179d8_c", func_80032CCC);
