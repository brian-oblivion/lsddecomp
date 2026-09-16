/*
 * code_179d8_s -- the last 7 functions of the original `code_179d8` monolith's
 * head, 0x17AD0..0x18480 (620 words).  Carved round 47 (2026-09-16); with
 * class_3bb8c_q this was the last uncarved game code in the executable.
 *
 * Census 2026-09-16, `tools/uncarved.py --functions`: 7 of 7 blocker-clean.
 * The `gp_rel` tags that tool prints for six of them are
 * RESOLVED-not-a-blocker (maspsx --gp-symbols, round 42) -- an ordinary
 * `lw $v0, %gp_rel(sym)($gp)` is a plain global access here, not a wall.
 * Gate 2 boundary checks all zero: no `jr $t2` trampoline, no `alabel`, no
 * non-`.L` alt-entry label, no function with two prologues.
 *
 * func_80027A24 owns this unit's only jump tables (jtbl_80010810 and
 * jtbl_80010828).  The 0xFD8 rodata slot was SPLIT at 0x1010 to attach them:
 * the two strings in the same slot belong to code_179d8_q and code_179d8_h
 * and stay standalone.  You do not need to do anything about this -- it is
 * recorded so that a link error mentioning either symbol is attributable.
 *
 * Expect this slice to span more than one class; identify each with
 * `tools/classtable.py` rather than assuming the unit has one.
 */

#include "common.h"

INCLUDE_ASM("asm/nonmatchings/code_179d8_s", func_800272D0);

INCLUDE_ASM("asm/nonmatchings/code_179d8_s", func_80027480);

INCLUDE_ASM("asm/nonmatchings/code_179d8_s", func_80027528);

void func_800276C8(void) {
}

INCLUDE_ASM("asm/nonmatchings/code_179d8_s", func_800276D0);

INCLUDE_ASM("asm/nonmatchings/code_179d8_s", func_80027800);

INCLUDE_ASM("asm/nonmatchings/code_179d8_s", func_80027A24);
