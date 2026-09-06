/*
 * code_179d8_i -- functions 220..237 of the original code_179d8 monolith,
 * 0x23500..0x24938 (18 functions).  Carved round 21 (2026-09-06) as the
 * FRONT slice of the old code_179d8_tail; the remainder keeps both of that
 * segment's switch jump tables, so this unit owns no rodata.
 *
 * Blocker census at carve time (four screens, canonical shell forms):
 * 9 of 18 clean, 9 addiu-$at.  Every blocked function has a stub report in
 * docs/match-reports/ -- do not re-screen them, and do not spend attempts
 * on them; they are the operator's call, not a matching problem.
 *
 * WORKABLE (all four screens clean):
 *   func_80032D00 13w   func_800334A0 20w   func_800336CC 16w
 *   func_8003370C 11w   func_80033738 157w  func_800339AC 40w
 *   func_80033A4C 25w   func_80033FB8 26w   func_8003410C 11w
 *
 * Expect this slice to span more than one class -- a ~20-function window cut
 * at ROM-address boundaries has no reason to align with class boundaries.
 * Identify each with tools/classtable.py rather than assuming the unit has
 * one.  Keep every function in strict ROM-address order.
 */

#include "common.h"

extern s16 func_80032D34(void *a0, s16 a1, s32 a2, s32 a3);

s16 func_80032D00(void *a0, s16 a1, s32 a2)
{
    return func_80032D34(a0, a1, 1, a2);
}

INCLUDE_ASM("asm/nonmatchings/code_179d8_i", func_80032D34);

INCLUDE_ASM("asm/nonmatchings/code_179d8_i", SsUtGetVabHdr);

INCLUDE_ASM("asm/nonmatchings/code_179d8_i", func_80033260);

INCLUDE_ASM("asm/nonmatchings/code_179d8_i", func_800334A0);

INCLUDE_ASM("asm/nonmatchings/code_179d8_i", func_800334F0);

INCLUDE_ASM("asm/nonmatchings/code_179d8_i", func_800335FC);

extern void func_80036AC8(s32 a0);

void func_800336CC(u8 a0)
{
    s32 mode;

    if (a0 != 0) {
        if (a0 != 1) {
            return;
        }
        mode = 1;
    } else {
        mode = 0;
    }
    func_80036AC8(mode);
}

extern s32 func_8003904C(s16 a0);

s16 func_8003370C(s16 a0)
{
    return func_8003904C(a0);
}

/* A 172 (0xAC)-byte record; D_800902E8 is an array of pointers to arrays of
 * these, indexed [screen][slot]-style by two signed 16-bit indices. This is
 * a reduced LOCAL view -- only the fields this unit's functions touch are
 * named. See code_179d8_f.c's own Entry90902E8 for a fuller layout of the
 * same array; each unit keeps its own independent reading, per project
 * convention (multiple local views of one struct are expected here). */
typedef struct {
    u8 pad0[0x2B];
    u8 unk2B;
    u8 pad2C[0x90 - 0x2C];
    s32 unk90;
    u8 pad94[0xAC - 0x94];
} Entry90902E8;

extern Entry90902E8 *D_800902E8[];

INCLUDE_ASM("asm/nonmatchings/code_179d8_i", func_80033738);

extern s32 func_8003069C(s32 a0);

void func_800339AC(s32 a0, s32 a1)
{
    s16 sa0 = (s16)a0;
    s16 sa1 = (s16)a1;
    Entry90902E8 *p = &D_800902E8[sa0][sa1];

    func_8003069C((sa1 << 8) | sa0);
    p->unk2B = 0;
    D_800902E8[sa0][sa1].unk90 &= ~2;
}

void func_80033A4C(s32 a0, s32 a1)
{
    s16 sa0 = (s16)a0;
    s16 sa1 = (s16)a1;

    D_800902E8[sa0][sa1].unk2B = 0;
    D_800902E8[sa0][sa1].unk90 &= ~0x100;
}

INCLUDE_ASM("asm/nonmatchings/code_179d8_i", func_80033AB0);

INCLUDE_ASM("asm/nonmatchings/code_179d8_i", func_80033C90);

INCLUDE_ASM("asm/nonmatchings/code_179d8_i", func_80033FB8);

INCLUDE_ASM("asm/nonmatchings/code_179d8_i", func_80034020);

INCLUDE_ASM("asm/nonmatchings/code_179d8_i", func_800340B0);

extern s32 func_80034138(s16 a0, s16 a1);

s32 func_8003410C(s16 a0, s16 a1)
{
    return func_80034138(a0, a1);
}
