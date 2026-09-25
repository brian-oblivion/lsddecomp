/*
 * code_fa50 -- GAME code carved from psyq_fa50 on 2026-09-25 (FINISHING-PLAN
 * revision 18). 0xFA50..0x10D48 (vram 0x8001F250..0x80020548). It was counted
 * as Psy-Q SDK by segment name; tools/gameinsdk.py measured it as game (a call
 * into game code, a method-table entry beside game methods, or contiguity with
 * those, and no Sony fingerprint). What it holds: the class of method table
 * D_8006BEA0 (new_class_6bea0 allocates it with BMemPMgrAlloc and chains to
 * Get_vtable_BasicClass) and helpers called only from Class6B5CC/BaseObjO
 * code. Owns jtbl_80010354 (attached rodata sub-slot 0xB54).
 *
 * Nothing here is matched yet: every function is fresh track-1 ground.
 */
#include "common.h"

INCLUDE_ASM("asm/nonmatchings/code_fa50", new_class_6bea0);
INCLUDE_ASM("asm/nonmatchings/code_fa50", func_8001F2B0);
INCLUDE_ASM("asm/nonmatchings/code_fa50", func_8001F314);
INCLUDE_ASM("asm/nonmatchings/code_fa50", func_8001F33C);
INCLUDE_ASM("asm/nonmatchings/code_fa50", func_8001F360);
INCLUDE_ASM("asm/nonmatchings/code_fa50", func_8001F37C);
INCLUDE_ASM("asm/nonmatchings/code_fa50", func_8001F384);
INCLUDE_ASM("asm/nonmatchings/code_fa50", func_8001F394);
INCLUDE_ASM("asm/nonmatchings/code_fa50", func_8001F3A4);
INCLUDE_ASM("asm/nonmatchings/code_fa50", func_8001F3B0);
INCLUDE_ASM("asm/nonmatchings/code_fa50", func_8001F4E4);
INCLUDE_ASM("asm/nonmatchings/code_fa50", func_8001F50C);
INCLUDE_ASM("asm/nonmatchings/code_fa50", func_8001F51C);
INCLUDE_ASM("asm/nonmatchings/code_fa50", func_8001F66C);
INCLUDE_ASM("asm/nonmatchings/code_fa50", func_8001F8B8);
INCLUDE_ASM("asm/nonmatchings/code_fa50", func_80020050);
INCLUDE_ASM("asm/nonmatchings/code_fa50", func_800204D0);
INCLUDE_ASM("asm/nonmatchings/code_fa50", func_80020510);
