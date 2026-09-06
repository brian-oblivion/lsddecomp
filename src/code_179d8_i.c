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

INCLUDE_ASM("asm/nonmatchings/code_179d8_i", func_80032D00);

INCLUDE_ASM("asm/nonmatchings/code_179d8_i", func_80032D34);

INCLUDE_ASM("asm/nonmatchings/code_179d8_i", SsUtGetVabHdr);

INCLUDE_ASM("asm/nonmatchings/code_179d8_i", func_80033260);

INCLUDE_ASM("asm/nonmatchings/code_179d8_i", func_800334A0);

INCLUDE_ASM("asm/nonmatchings/code_179d8_i", func_800334F0);

INCLUDE_ASM("asm/nonmatchings/code_179d8_i", func_800335FC);

INCLUDE_ASM("asm/nonmatchings/code_179d8_i", func_800336CC);

INCLUDE_ASM("asm/nonmatchings/code_179d8_i", func_8003370C);

INCLUDE_ASM("asm/nonmatchings/code_179d8_i", func_80033738);

INCLUDE_ASM("asm/nonmatchings/code_179d8_i", func_800339AC);

INCLUDE_ASM("asm/nonmatchings/code_179d8_i", func_80033A4C);

INCLUDE_ASM("asm/nonmatchings/code_179d8_i", func_80033AB0);

INCLUDE_ASM("asm/nonmatchings/code_179d8_i", func_80033C90);

INCLUDE_ASM("asm/nonmatchings/code_179d8_i", func_80033FB8);

INCLUDE_ASM("asm/nonmatchings/code_179d8_i", func_80034020);

INCLUDE_ASM("asm/nonmatchings/code_179d8_i", func_800340B0);

INCLUDE_ASM("asm/nonmatchings/code_179d8_i", func_8003410C);
