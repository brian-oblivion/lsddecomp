/*
 * class_3bb8c_r -- functions 23..43 of the 113-function `class_3bb8c_n`
 * remainder, 0x46288..0x46D20 (vram 0x80055A88..0x80056520).  Carved MID-round
 * 17 (2026-09-04) to re-staff a runner whose own unit was exhausted.
 *
 * Blocker census, three-grep screen run per function at carve time:
 * 20 of 21 clean, and ZERO trivial leaves -- every one is a real body, several
 * in the 40-70 instruction range.
 *
 * BLOCKED, stub report already filed, do NOT spend attempts on it:
 *   gp_rel: func_8005630C (only 5 instructions, so nothing is lost)
 *
 * Owns NO switch jump table -- zero `jtbl_` references in the slice -- so no
 * rodata sub-slot is attached to this unit.
 *
 * The 23 functions in FRONT of this slice (still `class_3bb8c_n`) are the
 * gp_rel-densest ground in the executable -- exactly two of them are clean --
 * which is why the cut is here rather than at the segment start.
 *
 * EXPECT THIS SLICE TO SPAN MORE THAN ONE CLASS.  It is cut at ROM addresses,
 * not at class boundaries.  Identify each class with tools/classtable.py
 * rather than assuming the unit has one.
 */
#include "common.h"

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_r", func_80055A88);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_r", func_80055B10);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_r", func_80055B6C);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_r", func_80055BC8);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_r", func_80055CA8);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_r", func_80055DB4);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_r", func_80055E94);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_r", func_80055EF0);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_r", func_80055F74);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_r", func_80055FE8);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_r", func_80056054);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_r", func_800560E4);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_r", func_80056194);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_r", func_80056238);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_r", func_8005627C);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_r", func_8005630C);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_r", func_80056320);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_r", func_800563C0);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_r", func_80056464);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_r", func_800564A4);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_r", func_800564F4);
