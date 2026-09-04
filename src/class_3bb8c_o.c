/*
 * class_3bb8c_o -- functions 54..73 of the 113-function `class_3bb8c_n`
 * remainder, 0x475F0..0x47CC4 (vram 0x80056DF0..0x800574C4).  Carved round 17
 * (2026-09-04); `class_3bb8c_n` keeps its name for the 54 functions in front
 * of this slice and `class_3bb8c_q` is the 19-function tail behind
 * `class_3bb8c_p`.
 *
 * Blocker census, three-grep screen run per function at carve time:
 * 19 of the 20 clean.
 *
 * BLOCKED, stub report already filed, do NOT spend attempts on it:
 *   gp_rel: func_80056F5C
 *
 * Owns NO switch jump table -- zero `jtbl_` references in the slice -- so no
 * rodata sub-slot is attached to this unit.
 *
 * EXPECT THIS SLICE TO SPAN MORE THAN ONE CLASS.  It is cut at ROM addresses,
 * not at class boundaries, and round 15 measured three of five such slices
 * spanning two or more vtables.  Identify each class with tools/classtable.py
 * rather than assuming the unit has one.  A class that spans a carve boundary
 * is also the normal reason two units name the same table -- see the
 * multiple-independent-local-views convention in CLAUDE.md before deciding
 * whether your view of one belongs in include/class_3bb8c.h or here.
 */
#include "common.h"

void func_80056DF0(void) {
}

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_o", func_80056DF8);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_o", func_80056E1C);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_o", func_80056E44);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_o", func_80056F28);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_o", func_80056F4C);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_o", func_80056F5C);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_o", func_80056FE4);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_o", func_80057044);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_o", func_800570B4);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_o", func_80057130);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_o", func_800571A8);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_o", func_800571E8);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_o", func_800571F8);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_o", func_80057320);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_o", func_80057384);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_o", func_800573A8);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_o", func_800573CC);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_o", func_80057444);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_o", func_8005748C);
