/*
 * code_179d8_f -- functions 256..273 of the original 274-function code_179d8
 * monolith, 0x2673C..0x272C8 (vram 0x80035F3C..0x80036AC8), i.e. its very
 * tail.  Carved round 17 (2026-09-04) off the front of `code_179d8_tail`,
 * which keeps that name for the 36 functions still in front of this slice.
 *
 * Blocker census, three-grep screen run per function at carve time:
 * 16 of the 18 clean, and NONE of the 18 is a trivial leaf -- every one is a
 * real body.  This is the densest-in-real-work unit carved this round.
 *
 * BLOCKED, stub reports already filed, do NOT spend attempts on these:
 *   addiu_at: func_80036230 (115 insn), func_80036528 (240 insn)
 * Both are large, which is the point of screening at carve time rather than
 * discovering it after a derivation.
 *
 * Owns NO switch jump table.  Both of this segment's jump-table owners
 * (func_80034690, func_800357B0) sit in the `code_179d8_tail` remainder in
 * FRONT of this slice -- that is where the cut was chosen, so that this unit
 * needs no rodata attach and the remainder carries that debt instead.
 *
 * Expect low-level driver-shaped code rather than class-framework code, as
 * elsewhere in code_179d8; confirm with tools/classtable.py, do not assume.
 */
#include "common.h"

/* 9 packed halfword fields, unpacked from two 16-bit-ish words (a0, a1).
 * Field semantics unknown -- named by offset per project convention. */
typedef struct {
    s16 unk0;  /* +0x0 */
    s16 unk2;  /* +0x2 */
    s16 unk4;  /* +0x4 */
    s16 unk6;  /* +0x6 */
    s16 unk8;  /* +0x8 */
    s16 unkA;  /* +0xA */
    s16 unkC;  /* +0xC */
    s16 unkE;  /* +0xE */
    s16 unk10; /* +0x10 */
} UnkStruct80035F3C;

void func_80035F3C(s32 a0, s32 a1, UnkStruct80035F3C *a2)
{
    int t;

    a2->unkA = a0 & 0x8000;
    t = a1 & 0x8000;
    a2->unkC = t;
    a2->unk10 = a1 & 0x4000;
    a2->unkE = a1 & 0x20;
    a2->unk0 = ((u16)a0 >> 8) & 0x7F;
    a2->unk2 = ((u16)a0 >> 4) & 0xF;
    a2->unk4 = a0 & 0xF;
    a2->unk6 = ((u32)a1 >> 6) & 0x7F;
    a2->unk8 = a1 & 0x1F;
}

INCLUDE_ASM("asm/nonmatchings/code_179d8_f", func_80035F98);

/* func_80038D74 is defined in the uncarved psyq_SpuSetMute unit; a leading
 * `beqz $a0` / `beq $a0,1` / else dispatch on a mode, and it returns a real
 * value read from D_8006DD2C right before its own jr $ra. */
extern s32 func_80038D74(s32 a0);

s32 func_80036024(void)
{
    return func_80038D74(1);
}

s32 func_80036044(void)
{
    return func_80038D74(0);
}

INCLUDE_ASM("asm/nonmatchings/code_179d8_f", func_80036064);

INCLUDE_ASM("asm/nonmatchings/code_179d8_f", func_80036108);

INCLUDE_ASM("asm/nonmatchings/code_179d8_f", func_80036118);

INCLUDE_ASM("asm/nonmatchings/code_179d8_f", func_800361B0);

INCLUDE_ASM("asm/nonmatchings/code_179d8_f", func_800361F0);

INCLUDE_ASM("asm/nonmatchings/code_179d8_f", func_80036230);

INCLUDE_ASM("asm/nonmatchings/code_179d8_f", func_800363FC);

INCLUDE_ASM("asm/nonmatchings/code_179d8_f", func_80036410);

INCLUDE_ASM("asm/nonmatchings/code_179d8_f", func_80036518);

INCLUDE_ASM("asm/nonmatchings/code_179d8_f", func_80036528);

INCLUDE_ASM("asm/nonmatchings/code_179d8_f", func_800368E8);

INCLUDE_ASM("asm/nonmatchings/code_179d8_f", func_80036A54);

INCLUDE_ASM("asm/nonmatchings/code_179d8_f", func_80036A7C);

INCLUDE_ASM("asm/nonmatchings/code_179d8_f", func_80036AA8);
