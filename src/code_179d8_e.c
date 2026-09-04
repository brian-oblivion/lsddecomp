/*
 * code_179d8_e -- functions 120..148 of the original 274-function code_179d8
 * monolith, 0x1CC08..0x1D508 (vram 0x8002C408..0x8002CD08).  Carved round 17
 * (2026-09-04) out of what the yaml called `code_179d8_mid_b`; the remainder
 * behind it is now `code_179d8_mid_c`.
 *
 * Blocker census, three-grep screen run per function at carve time:
 * 21 of the 29 clean.  READ THAT WITH CARE -- ten of the 29 are two-word
 * `jr $ra; nop` leaves and splat matched eight of them itself, so the real
 * queue is the 21 INCLUDE_ASMs below, of which eight are blocked.
 *
 * BLOCKED, stub reports already filed, do NOT spend attempts on these:
 *   gp_rel: func_8002C448, func_8002C468, func_8002C4E0, func_8002C638,
 *           func_8002C6FC, func_8002C890, func_8002CC1C, func_8002CC28
 * That is a cluster, not a scatter: these functions are the accessors for
 * one band of small-data globals (D_8008A8B0..D_8008A8CC), which is exactly
 * the shape the gp-relative blocker takes.  The bodies AROUND them that do
 * not touch that band are clean.
 *
 * Owns NO switch jump table -- zero `jtbl_` references anywhere in the slice
 * -- so no rodata sub-slot is attached to this unit.
 *
 * This region is NOT class-framework code; the sibling slice code_179d8_b
 * established that for this neighbourhood in round 16 (no tools/classtable.py
 * hit anywhere near these globals).  Do not go looking for a vtable or a
 * `this` pointer here.
 */
#include "common.h"

INCLUDE_ASM("asm/nonmatchings/code_179d8_e", func_8002C408);

void func_8002C410(void) {
}

void func_8002C418(void) {
}

void func_8002C420(void) {
}

void func_8002C428(void) {
}

void func_8002C430(void) {
}

INCLUDE_ASM("asm/nonmatchings/code_179d8_e", func_8002C438);

INCLUDE_ASM("asm/nonmatchings/code_179d8_e", func_8002C448);

INCLUDE_ASM("asm/nonmatchings/code_179d8_e", func_8002C468);

INCLUDE_ASM("asm/nonmatchings/code_179d8_e", func_8002C478);

INCLUDE_ASM("asm/nonmatchings/code_179d8_e", func_8002C480);

INCLUDE_ASM("asm/nonmatchings/code_179d8_e", func_8002C4E0);

INCLUDE_ASM("asm/nonmatchings/code_179d8_e", func_8002C638);

INCLUDE_ASM("asm/nonmatchings/code_179d8_e", func_8002C6FC);

INCLUDE_ASM("asm/nonmatchings/code_179d8_e", func_8002C824);

INCLUDE_ASM("asm/nonmatchings/code_179d8_e", func_8002C890);

INCLUDE_ASM("asm/nonmatchings/code_179d8_e", func_8002CA3C);

INCLUDE_ASM("asm/nonmatchings/code_179d8_e", func_8002CB18);

INCLUDE_ASM("asm/nonmatchings/code_179d8_e", func_8002CB58);

INCLUDE_ASM("asm/nonmatchings/code_179d8_e", func_8002CB9C);

void func_8002CBDC(void) {
}

void func_8002CBE4(void) {
}

void func_8002CBEC(void) {
}

INCLUDE_ASM("asm/nonmatchings/code_179d8_e", func_8002CBF4);

INCLUDE_ASM("asm/nonmatchings/code_179d8_e", func_8002CC0C);

INCLUDE_ASM("asm/nonmatchings/code_179d8_e", func_8002CC1C);

INCLUDE_ASM("asm/nonmatchings/code_179d8_e", func_8002CC28);

INCLUDE_ASM("asm/nonmatchings/code_179d8_e", func_8002CC34);

INCLUDE_ASM("asm/nonmatchings/code_179d8_e", func_8002CC84);
