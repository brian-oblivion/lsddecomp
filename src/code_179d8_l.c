/*
 * code_179d8_l -- FRONT half of what was the `code_179d8_mid_c` asm
 * remainder: functions 149..160 of the original 274-function code_179d8
 * monolith, 0x1D508..0x1ECD8 (vram 0x8002CD08..0x8002E4D8), 12 functions.
 * Carved round 24 (2026-09-08).  `code_179d8_m` is the back half; between
 * them they consume the remainder whole.
 *
 * WHY IT WAS UNCARVED, AND WHY THAT VERDICT IS DEAD.  The splat comment on
 * the old remainder called 0x1D508 "the addiu-$at dense heart of this
 * monolith (of its 51 functions only ~12 are clean)".  `addiu_at` was
 * RESOLVED in round 21 (maspsx `--addiu-at`;
 * docs/research/addiu-at-blocker.md), so that census measured an
 * obstruction that no longer exists.  Re-censused 2026-09-08 over all 24
 * functions of the remainder with the four screens, canonical shell forms
 * (`grep -A2` FORWARD for nop_mflo_mfhi): 21 of 24 CLEAN, zero gp_rel,
 * zero `jr $t2` trampolines.
 *
 * The remainder is UNIFORM in density now, so the cut between _l and _m is
 * a STAFFING cut (one unit per runner) and not a density cut.  All three
 * surviving blocked functions landed in this half:
 *
 * BLOCKED on nop_mflo_mfhi, which is STILL OPEN -- do NOT spend attempts:
 *   func_8002CD08 (132w), func_8002D1B4 (316w), func_8002D8E0 (311w)
 * That blocker is an `mflo`/`mfhi` FOLLOWED WITHIN TWO INSTRUCTIONS BY a
 * `mult`/`div`; it is the one construct in docs/research/addiu-at-blocker.md
 * that round 21 did not fix.  Each has a stub report.
 *
 * 9 CLEAN of 12, cheapest first:
 *   func_8002E2F8    2w  <- already matched: splat emitted the empty C body
 *   func_8002E300    2w  <- itself.  Not work, and not yours to redo.
 *   func_8002DF7C   47w   func_8002E038   64w   func_8002DDBC  112w
 *   func_8002E138  112w   func_8002E308  116w   func_8002D6A4  143w
 *   func_8002CF18  167w
 *
 * func_8002E308's opening `addu $t3, $a0, $zero` is REGISTER PRESSURE with
 * s16 argument narrowing, NOT a BIOS trampoline -- checked by hand at carve
 * time, because the `jr $t2` trampoline screen is blind to variants and a
 * trampoline-dense segment reads as the cleanest ground in the file while
 * being the least matchable (round 17, class_3bb8c_h).  It is also a
 * near-identical sibling of func_8002E874 in code_179d8_m: same prologue,
 * same narrowing shape, same early-out branch.  If you match one, say so in
 * the report -- the other unit's runner is deriving the same shape.
 *
 * Owns NO jump table: zero `jtbl_` references, and no rodata word anywhere
 * in the image points into 0x8002CD08..0x800300D0 (checked at carve time
 * both numerically and for symbolic `.word .L`), so no rodata attach.
 * Boundary checks both sides: no function has more than one
 * `addiu $sp, $sp, -N`, every one ends in its own `jr $ra`, zero `alabel`,
 * and the frameless ones open on their own arguments or on a global, never
 * on $sp.
 *
 * Expect this slice to span more than one class; identify each with
 * tools/classtable.py rather than assuming the unit has one.
 */
#include "common.h"

INCLUDE_ASM("asm/nonmatchings/code_179d8_l", func_8002CD08);

INCLUDE_ASM("asm/nonmatchings/code_179d8_l", func_8002CF18);

INCLUDE_ASM("asm/nonmatchings/code_179d8_l", func_8002D1B4);

INCLUDE_ASM("asm/nonmatchings/code_179d8_l", func_8002D6A4);

INCLUDE_ASM("asm/nonmatchings/code_179d8_l", func_8002D8E0);

INCLUDE_ASM("asm/nonmatchings/code_179d8_l", func_8002DDBC);

extern u8 D_8008EA0E;
extern u8 D_8008EA1C;
extern u8 D_8008EA1D;
extern u16 D_8006DAD8[];

s32 func_8002DF7C(void) {
    s32 a0;
    s32 q12;
    s16 rem12;
    u8 a2;
    u16 v1;

    a0 = (s16)(D_8008EA0E + 0x3C - D_8008EA1C);
    q12 = a0 / 12;
    a2 = D_8008EA1D >> 3;
    rem12 = a0 - q12 * 12;
    if (a2 >= 16) {
        a2 = 15;
    }
    v1 = D_8006DAD8[a2 + rem12 * 16];
    if ((s16)(q12 - 5) > 0) {
        v1 <<= (s16)(q12 - 5);
    } else if ((s16)(q12 - 5) < 0) {
        v1 = (u16)v1 >> -(s16)(q12 - 5);
    }
    return v1;
}

extern u8 D_8008EA13;
extern u8 D_8008EA18;

typedef struct {
    u8 unk0[4];
    u8 unk4;
    u8 unk5;
    u8 unk6[26];
} D8008E978Entry;

extern D8008E978Entry *D_8008E978;

s32 func_8002E038(s32 a0, s32 a1) {
    s32 origA0;
    s32 idx;
    s32 tblIdx;
    D8008E978Entry *e;
    s32 v0;
    s32 div8;
    u8 a2;
    s16 a3;
    s32 diff;
    s32 q12;
    s16 rem12;
    u16 v1;

    origA0 = a0;
    idx = D_8008EA18 + (D_8008EA13 << 4);
    e = &D_8008E978[idx];
    v0 = (u16)a1 + e->unk5;
    div8 = v0 / 8;
    a3 = div8;
    a2 = 0;
    if (div8 >= 16) {
        a2 = 1;
        a3 = div8 - 16;
    }
    diff = (s16)(a2 + (origA0 + 0x3C - e->unk4));
    q12 = diff / 12;
    rem12 = diff - q12 * 12;
    tblIdx = rem12 * 16;
    tblIdx = tblIdx + a3;
    v1 = D_8006DAD8[tblIdx];
    if ((s16)(q12 - 5) > 0) {
        v1 <<= (s16)(q12 - 5);
    } else if ((s16)(q12 - 5) < 0) {
        v1 = (u16)v1 >> -(s16)(q12 - 5);
    }
    return v1;
}

INCLUDE_ASM("asm/nonmatchings/code_179d8_l", func_8002E138);

void func_8002E2F8(void) {
}

void func_8002E300(void) {
}

INCLUDE_ASM("asm/nonmatchings/code_179d8_l", func_8002E308);
