/*
 * class_3bb8c_t -- functions 96..112 of the 113-function `class_3bb8c_n`
 * remainder, 0x48738..0x48F74 (vram 0x80057F38..0x80058774).  Carved
 * MID-round 17 (2026-09-04) to re-staff a runner whose own unit was
 * exhausted.  This is the LAST slice of the class_3bb8c block.
 *
 * Blocker census, three-grep screen run per function at carve time:
 * 16 of 17 clean.
 *
 * BLOCKED, stub report already filed, do NOT spend attempts on it:
 *   addiu_at: func_800585B4
 *
 * Four of the 17 are 2-instruction leaves that splat matched itself.
 *
 * This slice holds D_8001176C and D_80011778, the two strings left
 * STANDALONE when the 0x1EF4 rodata slot was split -- referenced from
 * func_80057FEC and func_80058084, both of which are in this unit.  A
 * string is referenced by SYMBOL and a standalone rodata object resolves
 * that fine (the `code_8220` / 0xA8C precedent in Gate 2), so no attach was
 * needed and the link came up green, which is the check that settles it.
 * The slice owns no `jtbl_` reference either.
 *
 * EXPECT THIS SLICE TO SPAN MORE THAN ONE CLASS.  It is cut at ROM
 * addresses, not class boundaries.  Identify each with tools/classtable.py.
 */
#include "common.h"

void func_80057F38(void) {
}

void func_80057F40(void) {
}

void func_80057F48(void) {
}

void func_80057F50(void) {
}

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_t", func_80057F58);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_t", func_80057F68);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_t", func_80057FC8);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_t", func_80058078);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_t", func_800580E0);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_t", func_800581C4);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_t", func_80058228);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_t", func_80058308);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_t", func_80058390);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_t", func_80058404);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_t", func_800585B4);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_t", func_80058694);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_t", func_80058764);
