/*
 * class_3bb8c_p -- functions 74..93 of the 113-function `class_3bb8c_n`
 * remainder, 0x47CC4..0x485BC (vram 0x800574C4..0x80057DBC).  Carved round 17
 * (2026-09-04), immediately behind `class_3bb8c_o`.
 *
 * Blocker census, three-grep screen run per function at carve time:
 * 20 of 20 clean -- no gp_rel, no addiu-$at, no nop_mflo_mfhi anywhere in the
 * slice.  This is the cleanest window found in the whole executable this
 * round.  Four of the 20 are two-word leaves; splat matched two of them
 * itself, so the queue below is 18.
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

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_p", func_800574C4);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_p", func_800574FC);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_p", func_80057534);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_p", func_800575B0);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_p", func_800575E0);

void func_80057610(void) {
}

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_p", func_80057618);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_p", func_80057668);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_p", func_80057784);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_p", func_80057954);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_p", func_80057A18);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_p", func_80057B54);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_p", func_80057B90);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_p", func_80057C14);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_p", func_80057C6C);

void func_80057C74(void) {
}

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_p", func_80057C7C);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_p", func_80057C84);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_p", func_80057C94);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_p", func_80057D10);
