/*
 * code_179d8_j -- functions 173..198 of the original code_179d8 monolith,
 * 0x20ADC..0x2273C (26 functions).  Carved round 21 (2026-09-06) out of the
 * MIDDLE of the old code_179d8_mid_c, chosen by blocker density rather than
 * by "next": the head of that segment is 5 clean of 24, this window is 12
 * clean of 26.  code_179d8_mid_c contains zero jtbl/.word .L, so no unit
 * here owns rodata.
 *
 * Blocker census at carve time (four screens, canonical shell forms):
 * 12 of 26 clean, 13 addiu-$at, 1 addiu-$at + nop_mflo_mfhi.  Every blocked
 * function has a stub report in docs/match-reports/ -- do not re-screen
 * them and do not spend attempts on them.
 *
 * func_800303FC is a two-instruction `jr $ra; nop` leaf that splat emitted
 * as C itself.  It was never work; do not count it as one.
 *
 * WORKABLE (all four screens clean):
 *   func_800302DC 59w   func_800303C8 13w   func_800307F0 29w
 *   func_80030864 21w   func_800308B8 29w   func_8003092C 21w
 *   func_800319B4 36w   func_80031C98 22w   func_80031D6C 35w
 *   func_80031E94 21w   func_80031EE8 21w
 *
 * Expect this slice to span more than one class; identify each with
 * tools/classtable.py rather than assuming the unit has one.  Keep every
 * function in strict ROM-address order.
 */

#include "common.h"

INCLUDE_ASM("asm/nonmatchings/code_179d8_j", func_800302DC);

INCLUDE_ASM("asm/nonmatchings/code_179d8_j", func_800303C8);

void func_800303FC(void) {
}

INCLUDE_ASM("asm/nonmatchings/code_179d8_j", func_80030404);

INCLUDE_ASM("asm/nonmatchings/code_179d8_j", func_80030584);

INCLUDE_ASM("asm/nonmatchings/code_179d8_j", func_800305F4);

INCLUDE_ASM("asm/nonmatchings/code_179d8_j", func_80030648);

INCLUDE_ASM("asm/nonmatchings/code_179d8_j", func_8003069C);

INCLUDE_ASM("asm/nonmatchings/code_179d8_j", func_800307F0);

INCLUDE_ASM("asm/nonmatchings/code_179d8_j", func_80030864);

INCLUDE_ASM("asm/nonmatchings/code_179d8_j", func_800308B8);

INCLUDE_ASM("asm/nonmatchings/code_179d8_j", func_8003092C);

INCLUDE_ASM("asm/nonmatchings/code_179d8_j", func_80030980);

INCLUDE_ASM("asm/nonmatchings/code_179d8_j", func_80030E90);

INCLUDE_ASM("asm/nonmatchings/code_179d8_j", func_80031280);

INCLUDE_ASM("asm/nonmatchings/code_179d8_j", func_8003149C);

INCLUDE_ASM("asm/nonmatchings/code_179d8_j", func_80031890);

INCLUDE_ASM("asm/nonmatchings/code_179d8_j", func_800319B4);

INCLUDE_ASM("asm/nonmatchings/code_179d8_j", func_80031A44);

INCLUDE_ASM("asm/nonmatchings/code_179d8_j", func_80031BA4);

INCLUDE_ASM("asm/nonmatchings/code_179d8_j", func_80031C98);

INCLUDE_ASM("asm/nonmatchings/code_179d8_j", func_80031CF0);

INCLUDE_ASM("asm/nonmatchings/code_179d8_j", func_80031D6C);

INCLUDE_ASM("asm/nonmatchings/code_179d8_j", func_80031DF8);

INCLUDE_ASM("asm/nonmatchings/code_179d8_j", func_80031E94);

INCLUDE_ASM("asm/nonmatchings/code_179d8_j", func_80031EE8);
