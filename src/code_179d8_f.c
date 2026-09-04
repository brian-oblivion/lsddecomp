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

INCLUDE_ASM("asm/nonmatchings/code_179d8_f", func_80035F3C);

INCLUDE_ASM("asm/nonmatchings/code_179d8_f", func_80035F98);

INCLUDE_ASM("asm/nonmatchings/code_179d8_f", func_80036024);

INCLUDE_ASM("asm/nonmatchings/code_179d8_f", func_80036044);

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
