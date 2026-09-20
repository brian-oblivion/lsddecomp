/*
 * ROUND 42 CORRECTION (2026-09-15) -- READ BEFORE ANY "BLOCKED" LINE BELOW:
 * every claim in this comment that a function is BLOCKED by `gp_rel`,
 * `nop_mflo_mfhi` or `addiu_at` is STALE.  All three constructs are RESOLVED
 * by pinned maspsx flags (CLAUDE.md, "Open toolchain blockers");
 * `tools/nearmiss.py` reports them tagged (RESOLVED-not-a-blocker) and counts
 * none of them.  Any "do NOT spend attempts on these" directive below is
 * therefore RETRACTED: those functions are ordinary matching work, and most
 * carry a mechanism-correct partial derivation already.  The rest of this
 * comment still stands -- only the blocker verdicts are withdrawn.
 * Screen: `python3 tools/nearmiss.py`, round 43 (2026-09-15).
 *
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
 * ROUND 44: THE THREE BELOW ARE ASSIGNABLE AND ARE THIS UNIT'S FRESH GROUND.
 * ~~BLOCKED on nop_mflo_mfhi, which is STILL OPEN -- do NOT spend attempts:~~
 *   func_8002CD08 (132w), func_8002D1B4 (316w), func_8002D8E0 (311w)
 * nop_mflo_mfhi was RESOLVED in round 42 (`--no-nop-mflo-mfhi`), so the
 * "do NOT spend attempts" directive above is WITHDRAWN.  All three are
 * never-attempted cold ground whose reports were marked REOPENED in round 44.
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
 * near-identical sibling of BeginVoiceFade in code_179d8_m: same prologue,
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

/* STALL -- see docs/match-reports/func_8002CD08.md. Best body reached
 * (110/132 built words, length EXACT at 132/132) preserved there in #if 0. */
INCLUDE_ASM("asm/nonmatchings/code_179d8_l", func_8002CD08);

INCLUDE_ASM("asm/nonmatchings/code_179d8_l", func_8002CF18);

/* Shared with func_8002D8E0 below (same two-level entry table, same
 * blend-cascade shape); declared once here since func_8002D1B4 is
 * ROM-earlier, reused there rather than redeclared. */
typedef struct {
    u8 pad0[0x74];
    u16 unk74;
    u16 unk76;
    u8 pad78[0xAC - 0x78];
} D800902E8Entry;
extern D800902E8Entry *D_800902E8[];

extern u8 D_8008EA16;
extern u8 D_8008EA19;
extern u8 D_8008EA17;
extern u8 D_8008EA11;
extern u8 D_8008EA1A;
extern s16 D_8008E8C0;
extern u16 D_8008EA22;
extern u8 D_8008EA20;
extern u8 D_8008D970[];
extern u8 D_8008D98C[];
extern u8 D_8008D9A3[];
extern u16 D_8008E228;
extern u16 D_8008E22C;
extern u16 D_80090C60;
extern u16 D_80090C64;
extern u16 D_8008E230;
extern u16 D_8008E234;

/* STALL -- see docs/match-reports/func_8002D1B4.md. Best body reached
 * (332/316 built words, 16 words LONG; 33/316 raw word-match, drift-
 * affected) preserved there in #if 0. */
INCLUDE_ASM("asm/nonmatchings/code_179d8_l", func_8002D1B4);

extern u8 D_8008EA13;
extern u8 D_8008EA18;

/* Shared with func_8002E038 below (same base pointer, same table); this
 * function needs the +0x10/+0x12 halfwords too, so the struct is declared
 * once here (ROM-address order: func_8002D6A4 precedes func_8002E038) and
 * reused there rather than redeclared -- see docs/match-reports/func_8002D6A4.md. */
typedef struct {
    u8 unk0[4];
    u8 unk4;
    u8 unk5;
    u8 unk6[6];
    u8 unk12;
    u8 unk13;
    u8 pad14[2];
    u16 unk16; /* +0x10 */
    u16 unk18; /* +0x12 */
    u8 pad20[0x20 - 20];
} D8008E978Entry;
extern D8008E978Entry *D_8008E978;

INCLUDE_ASM("asm/nonmatchings/code_179d8_l", func_8002D6A4);

/* D800902E8Entry, D_800902E8 and the blend-cascade globals
 * (D_8008EA16/17/19/1A/11/20/22, D_8008E8C0, D_8008E228/22C, D_80090C60/64,
 * D_8008E230/234, D_8008D970/98C/9A3) are already declared above, before
 * func_8002D1B4 (ROM-earlier, same shapes) -- reused here, not redeclared. */
extern u8 D_8008EA0E;
extern u8 D_8008EA1C;
extern u16 *D_8006DAD4;
extern u8 D_8008D7F0[];
extern u8 D_8008D7F2[];
extern u8 D_8008E9D0;

/* STALL -- see docs/match-reports/func_8002D8E0.md. Best body reached
 * (309/311 built words, 2 words SHORT) preserved there in #if 0. */
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
