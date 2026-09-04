/*
 * code_179d8_b -- window [60..79] of the original 274-function code_179d8
 * monolith, 0x194E0..0x1A1BC (vram 0x80028CE0..0x800299BC).
 *
 * Carved round 16 by blocker DENSITY, not by "next": code_179d8 is 44%
 * blocked in aggregate but the blockers CLUSTER, so the aggregate says
 * nothing about any particular window. This one screened 16/20 clean.
 * The four blocked functions are already stubbed as match reports:
 *   func_80028CF8, func_80028D30, func_80029478  -- addiu_at
 *   func_800292F4                                -- nop_mflo_mfhi
 *
 * func_80029478 owns jtbl_800109F8, whose sub-slot of the 0xFD8 rodata
 * region is ATTACHED to this unit in the splat yaml. Leave that alone.
 *
 * This slice was cut at ROM-address boundaries, so it has no reason to
 * align with class boundaries -- expect it to span more than one class,
 * and identify each with tools/classtable.py rather than assuming one.
 *
 * Declarations: keep anything that encodes THIS unit's reading of a class
 * next to the code, in this file. Do not create a shared code_179d8*.h --
 * the sibling slices are staffed independently and a shared header is what
 * makes their merges collide.
 */
#include "common.h"

INCLUDE_ASM("asm/nonmatchings/code_179d8_b", func_80028CE0);

INCLUDE_ASM("asm/nonmatchings/code_179d8_b", func_80028CF8);

INCLUDE_ASM("asm/nonmatchings/code_179d8_b", func_80028D30);

INCLUDE_ASM("asm/nonmatchings/code_179d8_b", func_80028D68);

INCLUDE_ASM("asm/nonmatchings/code_179d8_b", func_80028D88);

INCLUDE_ASM("asm/nonmatchings/code_179d8_b", func_80028DA8);

INCLUDE_ASM("asm/nonmatchings/code_179d8_b", func_80028DC0);

INCLUDE_ASM("asm/nonmatchings/code_179d8_b", func_80028DD8);

INCLUDE_ASM("asm/nonmatchings/code_179d8_b", func_80028DF0);

INCLUDE_ASM("asm/nonmatchings/code_179d8_b", func_80028F38);

INCLUDE_ASM("asm/nonmatchings/code_179d8_b", func_80029074);

INCLUDE_ASM("asm/nonmatchings/code_179d8_b", func_800291C8);

INCLUDE_ASM("asm/nonmatchings/code_179d8_b", func_800291EC);

INCLUDE_ASM("asm/nonmatchings/code_179d8_b", func_80029210);

INCLUDE_ASM("asm/nonmatchings/code_179d8_b", func_80029234);

INCLUDE_ASM("asm/nonmatchings/code_179d8_b", func_80029254);

INCLUDE_ASM("asm/nonmatchings/code_179d8_b", func_80029274);

INCLUDE_ASM("asm/nonmatchings/code_179d8_b", func_800292F4);

INCLUDE_ASM("asm/nonmatchings/code_179d8_b", func_800293F8);

INCLUDE_ASM("asm/nonmatchings/code_179d8_b", func_80029478);
