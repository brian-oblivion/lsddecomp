/*
 * code_179d8_m -- BACK half of what was the `code_179d8_mid_c` asm
 * remainder: functions 161..172 of the original 274-function code_179d8
 * monolith, 0x1ECD8..0x20ADC (vram 0x8002E4D8..0x800302DC), 12 functions.
 * Carved round 24 (2026-09-08).  `code_179d8_l` is the front half and
 * carries the shared carve-time census; `code_179d8_j` follows behind.
 *
 * WHY IT WAS UNCARVED, AND WHY THAT VERDICT IS DEAD.  The old remainder was
 * left as "the addiu-$at dense heart of this monolith".  `addiu_at` was
 * RESOLVED in round 21 (maspsx `--addiu-at`;
 * docs/research/addiu-at-blocker.md).  Re-censused 2026-09-08 with the four
 * screens, canonical shell forms (`grep -A2` FORWARD for nop_mflo_mfhi):
 *
 *   12 of 12 CLEAN -- the whole nop_mflo_mfhi cluster (3 functions) fell in
 *   the front half, so this unit has NO blocked function at all.  Zero
 *   gp_rel, zero nop_mflo_mfhi, zero `jr $t2` trampolines.
 *
 * Sizes, cheapest first -- four functions at 32..60 words:
 *   func_8002F368   32w   func_8002F20C   38w   func_8002F2A4   49w
 *   func_8002F610   60w   func_8002E874  116w   func_800300D0  131w
 *   func_8002F3E8  138w   func_8002EA44  228w   func_8002E4D8  231w
 *   func_8002F700  241w   func_8002EDD4  270w   func_8002FAC4  387w
 *
 * func_8002E874 is a near-identical sibling of func_8002E308 in
 * code_179d8_l -- same prologue (`addu $t3, $a0, $zero`), same s16
 * argument-narrowing shape, same early-out branch.  That opening is
 * REGISTER PRESSURE, not a BIOS trampoline; checked by hand at carve time
 * because the `jr $t2` screen is blind to variants.  If you match it, say
 * so in the report: the other unit's runner is deriving the same shape.
 *
 * Owns NO jump table and needs no rodata attach (see code_179d8_l's header
 * for the survey).  Boundary checks both sides: no function has more than
 * one `addiu $sp, $sp, -N`, every one ends in its own `jr $ra`, zero
 * `alabel`, and the frameless ones open on their own arguments or on a
 * global, never on $sp.
 *
 * Expect this slice to span more than one class; identify each with
 * tools/classtable.py rather than assuming the unit has one.
 */
#include "common.h"

INCLUDE_ASM("asm/nonmatchings/code_179d8_m", func_8002E4D8);

INCLUDE_ASM("asm/nonmatchings/code_179d8_m", func_8002E874);

INCLUDE_ASM("asm/nonmatchings/code_179d8_m", func_8002EA44);

INCLUDE_ASM("asm/nonmatchings/code_179d8_m", func_8002EDD4);

/* Scratch state byte: written here (as a u16 store -- upper byte is
 * always 0, the value is masked to 0xFF before the store) then
 * re-read as its low byte a few instructions later for a call
 * argument.  code_179d8_j.c documents this symbol (declared there
 * `volatile u16`) as a "currently selected channel" global written as
 * a side effect and re-read from the global rather than a cached
 * register -- same idiom here, hence the mixed sh-then-lbu widths. */
extern u16 D_8008EA26;
/* Loop bound / threshold, read fresh each call -- same symbol
 * code_179d8_j.c documents as "loop bound for a small table of active
 * objects". */
extern u8 D_8008E9D0;
/* Flag byte forced on unconditionally at entry. */
extern u8 D_8008EA1B;

extern s32 func_8002CF18(s32 a0);
extern void func_8002DDBC(s32 a0, s32 a1, s32 a2, s32 a3, s32 a4);

void func_8002F20C(s32 a0, s32 a1, s32 a2, s32 a3) {
    s32 v0;

    D_8008EA1B = 0x7F;
    v0 = func_8002CF18(0xFF) & 0xFF;
    D_8008EA26 = v0;
    if (v0 < D_8008E9D0) {
        func_8002DDBC(*(u8 *)&D_8008EA26, a0 & 0xFFFF, a1 & 0xFFFF, a2 & 0xFFFF, a3 & 0xFFFF);
    }
}

INCLUDE_ASM("asm/nonmatchings/code_179d8_m", func_8002F2A4);

void func_8002F368(s32 a0, s32 a1) {
    s32 v0;

    D_8008EA1B = 0x7F;
    v0 = func_8002CF18(0xFF) & 0xFF;
    D_8008EA26 = v0;
    if (v0 < D_8008E9D0) {
        func_8002DDBC(*(u8 *)&D_8008EA26, a0 & 0xFFFF, a1 & 0xFFFF, 0x80FF, 0x5FC8);
    }
}

INCLUDE_ASM("asm/nonmatchings/code_179d8_m", func_8002F3E8);

INCLUDE_ASM("asm/nonmatchings/code_179d8_m", func_8002F610);

INCLUDE_ASM("asm/nonmatchings/code_179d8_m", func_8002F700);

INCLUDE_ASM("asm/nonmatchings/code_179d8_m", func_8002FAC4);

INCLUDE_ASM("asm/nonmatchings/code_179d8_m", func_800300D0);
