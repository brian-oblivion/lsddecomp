/*
 * code_10ee0 -- GAME code carved from the head of psyq_10ee0 on 2026-09-25
 * (FINISHING-PLAN revision 18). 0x10EE0..0x11474 (vram
 * 0x800206E0..0x80020C74). It was counted as Psy-Q SDK by segment name;
 * tools/gameinsdk.py measured it as game (a call into game code, a method-
 * table entry beside game methods, or contiguity with those, and no Sony
 * fingerprint). What it holds: the 19 methods of D_8006C070 (new_class_6c078
 * allocates it) and a getter/setter pair for the game gp variable D_8008A83C.
 * libgpu/sys starts right after, at ResetGraph (now psyq_11474).
 *
 * Nothing here is matched yet: every function is fresh track-1 ground.
 */
#include "common.h"

INCLUDE_ASM("asm/nonmatchings/code_10ee0", new_class_6c078);
INCLUDE_ASM("asm/nonmatchings/code_10ee0", func_80020730);
INCLUDE_ASM("asm/nonmatchings/code_10ee0", func_80020784);
INCLUDE_ASM("asm/nonmatchings/code_10ee0", func_800207DC);
INCLUDE_ASM("asm/nonmatchings/code_10ee0", func_8002085C);
INCLUDE_ASM("asm/nonmatchings/code_10ee0", func_8002089C);
INCLUDE_ASM("asm/nonmatchings/code_10ee0", func_800208B8);
INCLUDE_ASM("asm/nonmatchings/code_10ee0", func_800208D8);
INCLUDE_ASM("asm/nonmatchings/code_10ee0", func_800208F8);
INCLUDE_ASM("asm/nonmatchings/code_10ee0", func_80020970);
INCLUDE_ASM("asm/nonmatchings/code_10ee0", func_800209A0);
INCLUDE_ASM("asm/nonmatchings/code_10ee0", func_80020A1C);
INCLUDE_ASM("asm/nonmatchings/code_10ee0", func_80020A24);
INCLUDE_ASM("asm/nonmatchings/code_10ee0", func_80020A74);
INCLUDE_ASM("asm/nonmatchings/code_10ee0", func_80020AF4);
INCLUDE_ASM("asm/nonmatchings/code_10ee0", func_80020B4C);
INCLUDE_ASM("asm/nonmatchings/code_10ee0", func_80020B68);
INCLUDE_ASM("asm/nonmatchings/code_10ee0", func_80020B74);
INCLUDE_ASM("asm/nonmatchings/code_10ee0", func_80020C08);
INCLUDE_ASM("asm/nonmatchings/code_10ee0", func_80020C3C);
INCLUDE_ASM("asm/nonmatchings/code_10ee0", func_80020C44);
INCLUDE_ASM("asm/nonmatchings/code_10ee0", func_80020C4C);
INCLUDE_ASM("asm/nonmatchings/code_10ee0", func_80020C5C);
INCLUDE_ASM("asm/nonmatchings/code_10ee0", func_80020C68);
