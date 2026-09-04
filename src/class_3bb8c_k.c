/*
 * class_3bb8c_k -- fifth carved slice of the class_3bb8c block
 * (0x429D4..0x435E0, vram 0x800521D4..0x80052DE0), 20 functions.
 * Carved round 15.
 *
 * Blocker profile (head's Gate 1 three-grep screen at carve time):
 *   func_800522DC  addiu-$at, and it OWNS jtbl_800116F4 -- the rodata slot
 *                  at 0x1EF4 is attached to this unit for that reason
 *   func_80052644  gp_rel
 * Both have stub reports; do not attempt either. The other 18 are clean.
 *
 * include/class_3bb8c.h is SHARED with every other class_3bb8c_* slice.
 * Header edits must be strictly ADDITIVE.
 */
#include "common.h"

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_k", func_800521D4);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_k", func_8005227C);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_k", func_800522DC);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_k", func_800523F0);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_k", func_80052430);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_k", func_80052498);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_k", func_800524F8);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_k", func_80052598);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_k", func_80052644);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_k", func_8005278C);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_k", func_8005281C);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_k", func_8005292C);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_k", func_800529FC);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_k", func_80052A58);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_k", func_80052B54);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_k", func_80052B60);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_k", func_80052B70);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_k", func_80052C10);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_k", func_80052CD8);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_k", func_80052D10);
