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

/* STALL -- see docs/match-reports/func_8002E4D8.md. Best body reached
 * (237/231 built words, 6 words LONG) preserved there in #if 0. */
INCLUDE_ASM("asm/nonmatchings/code_179d8_m", func_8002E4D8);

/* Same 0x34-stride channel-configuration record family documented in
 * code_179d8_j.c (Rec34D994/Rec34Byte/Rec34Half); this unit keeps its
 * own local view rather than sharing that file's header-less types.
 * Six independent 2-bytes-apart symbols share this one shape, same
 * idiom as code_179d8_j.c's own D_8008D994/D_8008D996/... family. */
typedef struct {
    s16 unk0; /* +0x0 */
    u8 pad2[0x34 - 0x2];
} Rec34Half;
extern Rec34Half D_8008D9B0[]; /* "interpolating" flag */
extern Rec34Half D_8008D9B2[]; /* "interpolating" flag (companion pair) */
extern Rec34Half D_8008D9B4[]; /* step/quotient */
extern Rec34Half D_8008D9B6[]; /* step/quotient (companion pair) */
extern Rec34Half D_8008D9B8[]; /* saved start value */
extern Rec34Half D_8008D9BA[]; /* saved end value */

void func_8002E874(s16 a0, s16 a1, s16 a2, s16 a3) {
    s16 q;

    if (a1 == a2) {
        return;
    }
    D_8008D9B0[a0].unk0 = 1;
    D_8008D9B8[a0].unk0 = a1;
    D_8008D9BA[a0].unk0 = a2;
    if ((a1 - a2 < 0 ? a2 - a1 : a1 - a2) < a3) {
        q = a3 / (a1 - a2);
        D_8008D9B2[a0].unk0 = 1;
        D_8008D9B4[a0].unk0 = q;
        D_8008D9B6[a0].unk0 = q;
    } else {
        q = (a1 - a2) / a3;
        D_8008D9B4[a0].unk0 = 0;
        D_8008D9B2[a0].unk0 = q;
    }
}

/* Same 0x34-stride record family, UNSIGNED 16-bit view -- D_8008D9B2,
 * D_8008D9B6 and D_8008D9B8 each need this width (`lhu`) at least once
 * in this function, on top of the plain signed Rec34Half view
 * declared above (which some of these same symbols also need, at a
 * DIFFERENT read site in this same function). Reinterpreted through a
 * cast rather than redeclared, per this project's rule that one
 * extern symbol cannot carry two conflicting C types in one file. */
typedef struct {
    u16 unk0; /* +0x0 */
    u8 pad2[0x34 - 0x2];
} Rec34HalfU;

/* Same 0x10-byte-stride record family code_179d8_j.c documents as
 * Rec16D7F0 (that unit's own D_8008D7F0/D_8008D7F4 pair); local view. */
typedef struct {
    s16 unk0; /* +0x0 */
    u8 pad2[0x10 - 0x2];
} Rec16D7F0;
extern Rec16D7F0 D_8008D7F0[];
extern Rec16D7F0 D_8008D7F2[];

extern u8 D_8008D970[];

/* Pointer to an object; only the fields this unit's functions read are
 * named. func_8002FAC4 (below) additionally needs a u16 field at
 * +0x12 (a "difficulty count" threshold, compared unsigned against
 * D_8008EA13), on top of the existing +0x18 byte field. */
typedef struct {
    u8 pad[0x12];
    u16 unk12; /* +0x12 */
    u8 pad14[0x18 - 0x14];
    u8 unk18; /* +0x18 */
} ObjE970;
extern ObjE970 *D_8008E970;

/* Scratch bytes for a chained percentage-of-percentage volume/pan
 * calculation -- see the match report for how the two 16129 (=127*127)
 * divisions were identified (brute-forced against the pinned
 * toolchain's own magic-multiply constants). */
extern u8 D_8008EA10;
extern u8 D_8008EA11;
extern u8 D_8008EA16;
extern u8 D_8008EA17;
extern u8 D_8008EA19;
extern u8 D_8008EA1A;

/* Mode flag: forces both output channels to the same (maximum) level
 * when set to 1. */
extern s16 D_8008E8C0;

/* STALL -- see docs/match-reports/func_8002EA44.md. Best body reached
 * (223/228 built words, 5 words short; 13/228 raw word-match, drift-
 * affected so not itself a distance measure) preserved there in #if 0. */
INCLUDE_ASM("asm/nonmatchings/code_179d8_m", func_8002EA44);

extern void _spu_setInTransfer(s32 a0);
extern void SpuInitMalloc(s32 a0, void *a1);
extern void func_8002F700(void);

extern u8 D_8008DEB0[];
extern s16 D_8008E9FC;
extern s16 D_8008E84C;
extern s16 D_8008E260;
extern s16 D_8008E262;
extern s16 D_8008E230;
extern s16 D_8008E234;
extern s32 D_8008E258;
extern s32 D_8008E25C;
extern u8 D_8008EA40;
extern s16 D_8008E938;

extern Rec34Half D_8008D98A[]; /* value forced to 0x18 at init */

typedef struct {
    s16 unk0; /* +0x0 */
    u8 pad2[0x34 - 0x2];
} Rec34S16Edd4;
extern Rec34S16Edd4 D_8008D988Edd4[] __asm__("D_8008D988");
extern Rec34S16Edd4 D_8008D996Edd4[] __asm__("D_8008D996");
extern Rec34S16Edd4 D_8008D99AEdd4[] __asm__("D_8008D99A");

extern Rec34Half D_8008D990[];
extern Rec34Half D_8008D98C[];
extern Rec34Half D_8008D98E[];
extern Rec34Half D_8008D998[];
extern Rec34Half D_8008D9A6[];
extern Rec34Half D_8008D9A8[];
extern Rec34Half D_8008D9AA[];
extern Rec34Half D_8008D9AC[];

typedef struct {
    u16 unk0; /* +0x0 */
    u8 pad2[0x34 - 0x2];
} Rec34U16Edd4;
extern Rec34U16Edd4 D_8008D99CEdd4[] __asm__("D_8008D99C");

typedef struct {
    u8 unk0; /* +0x0 */
    u8 pad1[0x34 - 0x1];
} Rec34ByteEdd4;
extern Rec34ByteEdd4 D_8008D992[]; /* byte field, forced to 0x40 at init */
extern Rec34ByteEdd4 D_8008D9A3Edd4[] __asm__("D_8008D9A3");
extern Rec34Half D_8008D9A4[];

extern volatile u16 D_8008EA26;
extern u8 D_8008E9D0;
extern s16 D_80090BD0;
extern u8 D_8008EA2C[];
extern u16 D_80090C60;
extern u16 D_80090C64;
extern u16 D_8008E228;
extern u16 D_8008E22C;

typedef struct {
    u8 pad[0x194];
    u16 unk194; /* +0x194 */
    u16 unk196; /* +0x196 */
} ObjDAD4Edd4;
extern ObjDAD4Edd4 *D_8006DAD4Edd4 __asm__("D_8006DAD4");

/* MATCHED round 32 (bravo), 270/270 -- closes the register-identity stall
 * every prior round's hand-reshaping (10 axes) and one earlier permuter
 * run (100k+ iterations) could not move.  Permuter-found: a SINGLE scratch
 * variable (`scratch`, `s32`), reused for TWO textually unrelated
 * purposes -- once to break the `a0`-vs-`s0` min-clamp tie, and again
 * ~150 lines later as the `D_8006DAD4[woff]` store's index -- is what
 * reproduces retail's exact register allocation.  Splitting these into
 * two separately-named locals (the natural, more readable choice) gives a
 * WORSE result than either leaving `a0` alone or this single-variable
 * reuse; the reuse itself is load-bearing, not cosmetic.  See
 * docs/match-reports/func_8002EDD4.md for the full derivation, the
 * permuter trace, and why the naive two-variable translation regresses
 * sharply (47/270) despite being semantically identical. */
void func_8002EDD4(s32 a0) {
    s16 i;
    s32 scratch;

    _spu_setInTransfer(0);
    D_8008E9FC = 0;
    D_8008E84C = 0;
    SpuInitMalloc(0x20, D_8008DEB0);

    for (i = 0; (u16) i < 0xC0; i++) {
        ((u16 *) D_8008D7F0)[(u16) i] = 0;
    }

    for (i = 0; (u16) i < 0x18; i++) {
        D_8008D970[(u16) i] = 0;
    }

    D_80090BD0 = 0;

    for (i = 0; (u16) i < 0x10; i++) {
        D_8008EA2C[(u16) i] = 0;
    }

    a0 = (u8) a0;
    scratch = a0;
    if ((u32) a0 >= 0x18) {
        D_8008E9D0 = 0x18;
    } else {
        D_8008E9D0 = scratch;
    }

    for (i = 0; (u16) i < D_8008E9D0; i++) {
        u16 woff;
        u16 chan;
        u16 lowMask;
        u16 highMask;

        woff = (u16) i * 8;

        D_8008D98A[(u16) i].unk0 = 0x18;
        D_8008D996Edd4[(u16) i].unk0 = -1;
        D_8008D988Edd4[(u16) i].unk0 = 0xFF;
        D_8008D9A3Edd4[(u16) i].unk0 = 0;
        D_8008D98C[(u16) i].unk0 = 0;
        D_8008D98E[(u16) i].unk0 = 0;
        D_8008D998[(u16) i].unk0 = 0;
        D_8008D99AEdd4[(u16) i].unk0 = 0;
        D_8008D99CEdd4[(u16) i].unk0 = 0xFF;
        D_8008D990[(u16) i].unk0 = 0;
        D_8008D992[(u16) i].unk0 = 0x40;
        D_8008D9A4[(u16) i].unk0 = 0;
        D_8008D9A6[(u16) i].unk0 = 0;
        D_8008D9A8[(u16) i].unk0 = 0;
        D_8008D9AA[(u16) i].unk0 = 0;
        D_8008D9B0[(u16) i].unk0 = 0;
        D_8008D9B2[(u16) i].unk0 = 0;
        D_8008D9B4[(u16) i].unk0 = 0;
        D_8008D9B6[(u16) i].unk0 = 0;
        D_8008D9B8[(u16) i].unk0 = 0;
        D_8008D9AC[(u16) i].unk0 = 0;

        ((s16 *) D_8006DAD4Edd4)[woff + 3] = 0x200;   /* +0x6 */
        scratch = woff;
        ((s16 *) D_8006DAD4Edd4)[woff + 2] = 0x1000;  /* +0x4 */
        ((u16 *) D_8006DAD4Edd4)[woff + 4] = 0x80FF;  /* +0x8 */
        ((s16 *) D_8006DAD4Edd4)[scratch] = 0;         /* +0x0 */
        ((s16 *) D_8006DAD4Edd4)[woff + 1] = 0;       /* +0x2 */
        ((s16 *) D_8006DAD4Edd4)[woff + 5] = 0x4000;  /* +0xA */

        __asm__("");
        D_8008EA26 = i;
        chan = D_8008EA26;
        if (chan < 0x10) {
            lowMask = 1 << chan;
            highMask = 0;
        } else {
            lowMask = 0;
            highMask = 1 << (chan - 0x10);
        }

        D_8008D9A3Edd4[chan].unk0 = 0;
        D_8008D98C[chan].unk0 = 0;
        D_8008D988Edd4[chan].unk0 = 0;
        D_80090C60 |= lowMask;
        D_80090C64 |= highMask;
        woff = D_80090C60;
        D_8008E228 &= ~woff;
        D_8008E22C &= ~D_80090C64;
    }

    D_8008E260 = 0x3FFF;
    D_8008E262 = 0x3FFF;
    D_8008E228 = 0;
    D_8008E22C = 0;
    D_80090C60 = 0;
    D_8008E230 = 0;
    D_8008E234 = 0;
    D_8008E258 = 0;
    D_8008E25C = 0;
    D_8008EA40 = 0;
    D_8008E8C0 = 0;
    D_8008E938 = 0x80;
    func_8002F700();
}

/* "Currently selected channel" scratch global: written as a side
 * effect, then re-read from the global (not a cached register) a few
 * instructions later -- same idiom, and same `volatile u16` type,
 * code_179d8_j.c documents for this symbol.  Genuinely needs
 * `volatile`: without it, this compiler proves (from the narrow range
 * of the values stored here) that the re-read is redundant and elides
 * it entirely, which retail's disassembly shows it does NOT do.
 * `volatile` alone reproduces retail's separate store/reload exactly
 * -- reading it back through `*(u8 *)&D_8008EA26` (a plain, NON-
 * volatile-qualified pointer type) still folds to retail's compact
 * `lui`+`lbu` two-instruction form; it is specifically a
 * VOLATILE-QUALIFIED POINTER TYPE (`volatile u8 *`) that defeats the
 * addressing fold, not the underlying object's volatility. */
extern volatile u16 D_8008EA26;
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

/* Same 0x34-stride channel-configuration record family documented in
 * code_179d8_j.c (Rec34D994/Rec34Byte); this unit keeps its own local
 * view rather than sharing that file's header-less types. Rec34Half
 * itself is declared above, before its first user func_8002E874. */
typedef struct {
    u8 unk0; /* +0x0 */
    u8 pad1[0x34 - 0x1];
} Rec34Byte;
extern Rec34Byte D_8008D9A3[];

extern Rec34Half D_8008D98C[];

/* Pointer to a "current object" whose only fields this function
 * touches sit at a fixed byte offset from the base, not scaled by any
 * index -- a different reading of the same D_8006DAD4 symbol from
 * code_179d8_j.c's array-of-0x10-byte-records view, per this project's
 * multiple-independent-local-views convention. */
typedef struct {
    u8 pad[0x194];
    u16 unk194; /* +0x194 */
    u16 unk196; /* +0x196 */
} ObjDAD4;
extern ObjDAD4 *D_8006DAD4;

void func_8002F2A4(void) {
    s16 i;

    for (i = 0; i < D_8008E9D0; i++) {
        if (D_8008D9A3[i].unk0 == 2) {
            D_8008D9A3[(u8) i].unk0 = 0;
            D_8008D98C[(u8) i].unk0 = 0;
            D_8006DAD4->unk194 = 0;
            D_8006DAD4->unk196 = 0;
        }
    }
}

void func_8002F368(s32 a0, s32 a1) {
    s32 v0;

    D_8008EA1B = 0x7F;
    v0 = func_8002CF18(0xFF) & 0xFF;
    D_8008EA26 = v0;
    if (v0 < D_8008E9D0) {
        func_8002DDBC(*(u8 *)&D_8008EA26, a0 & 0xFFFF, a1 & 0xFFFF, 0x80FF, 0x5FC8);
    }
}

/* Same 0x34-stride channel-configuration record family, s16-field
 * view -- matches code_179d8_j.c's own Rec34D994 shape (that unit's
 * D_8008D994/D_8008D996/D_8008D99A/D_8008D99C/D_8008D99E family); this
 * unit keeps its own independent view rather than sharing the header-
 * less type. D_8008D988 shares the shape too (read here with `lh`, a
 * signed load, unlike D_8008D98C's `lhu`-driven Rec34Half above). */
typedef struct {
    s16 unk0; /* +0x0 */
    u8 pad2[0x34 - 0x2];
} Rec34S16;
extern Rec34S16 D_8008D994[];
extern Rec34S16 D_8008D996[];
extern Rec34S16 D_8008D99A[];
extern Rec34S16 D_8008D99E[];
extern Rec34S16 D_8008D988[];

/* Same 0x34-stride record family, UNSIGNED 16-bit view -- D_8008D99C
 * needs this width (`lhu`) here, and BOTH this width and a plain byte
 * width (`lbu`, same offset) elsewhere in this same function; the byte
 * view is reached via a plain pointer cast, same idiom as D_8008EA26's
 * mixed sh/lbu access.  D_8008D994 needs the same unsigned re-reading
 * here even though func_800300D0 (above) reads the SAME symbol signed
 * (`lh`) -- reinterpreted through a cast rather than redeclared, since
 * one extern symbol cannot carry two conflicting C types in one file. */
typedef struct {
    u16 unk0; /* +0x0 */
    u8 pad2[0x34 - 0x2];
} Rec34U16;
extern Rec34U16 D_8008D99C[];

/* Debug/selected-difficulty byte, read fresh each call. */
extern u8 D_8008EA13;

/* Pointer to a 0x20-byte-stride table. Originally only the two
 * trailing byte fields func_8002F3E8 reads (unkC/unkD) were named;
 * func_8002FAC4 (below) additionally needs unk0/unk1/unk2/unk3/unk4/
 * unk5/unk6/unk7/unk16, all in the same struct (no offset conflicts,
 * per this project's convention of extending rather than duplicating
 * a local view when the fields don't overlap). */
typedef struct {
    u8 unk0; /* +0x0 */
    u8 unk1; /* +0x1 */
    u8 unk2; /* +0x2 */
    u8 unk3; /* +0x3 */
    u8 unk4; /* +0x4 */
    u8 unk5; /* +0x5 */
    u8 unk6; /* +0x6 */
    u8 unk7; /* +0x7 */
    u8 pad8[0xC - 0x8];
    u8 unkC; /* +0xC */
    u8 unkD; /* +0xD */
    u8 pad0E[0x16 - 0xE];
    u8 unk16; /* +0x16 */
    u8 pad17[0x20 - 0x17];
} Tbl32E978;
extern Tbl32E978 *D_8008E978;

/* Same 0x10-byte-stride record family code_179d8_j.c documents as
 * Rec16D7F0 (that unit's D_8008D7F0/D_8008D7F4 pair); local view. */
typedef struct {
    s16 unk0; /* +0x0 */
    u8 pad2[0x10 - 0x2];
} Rec16D7F4;
extern Rec16D7F4 D_8008D7F4[];

/* Plain byte-stride flags array (no per-record multiply in its own
 * addressing -- unlike every 0x34/0x10-stride array above). */
extern u8 D_8008D970[];

/* Selected-channel debug byte, write-only here. */
extern u8 D_8008EA18;

extern s16 func_8002E038(u16 a0, u16 a1);

/* STALL -- see docs/match-reports/func_8002F3E8.md. Best body reached
 * (77/138 words, byte-exact length) preserved there in #if 0. */
INCLUDE_ASM("asm/nonmatchings/code_179d8_m", func_8002F3E8);

/* "Currently selected channel" scratch global -- same idiom as
 * D_8008EA26 above, write-only here (see code_179d8_j.c's own reading
 * of this symbol). */
extern u16 D_8008EA22;

extern s32 func_80032148(s16 a0, s16 a1);
extern s16 func_8002F3E8(s16 a0, s16 a1, s16 a2, s16 a3, u16 a4);

s32 func_8002F610(s16 a0, s16 a1, s16 a2, u16 a3) {
    s16 i;
    s32 sum;

    func_80032148(a1, a2);
    D_8008EA22 = a0;
    sum = 0;
    for (i = 0; i < D_8008E9D0; i++) {
        sum += func_8002F3E8(i, a0, a1, a2, a3);
    }
    return sum;
}

/* STALL -- see docs/match-reports/func_8002F700.md. Best body reached
 * (237/241 built words, 4 words short) preserved there in #if 0. */
INCLUDE_ASM("asm/nonmatchings/code_179d8_m", func_8002F700);

/* STALL -- see docs/match-reports/func_8002FAC4.md. Best body reached
 * (402/387 built words, 15 words LONG) preserved there in #if 0. */
INCLUDE_ASM("asm/nonmatchings/code_179d8_m", func_8002FAC4);

/* A pair of 16-bit bitmasks split across a 0..0x1F channel space (low
 * 16 channels in the first word, next 16 in the second), each paired
 * with an "active mask" word cleared wherever the channel mask bit is
 * set -- same symbols and reading code_179d8_j.c already documents. */
extern u16 D_80090C60;
extern u16 D_80090C64;
extern u16 D_8008E228;
extern u16 D_8008E22C;

u8 func_800300D0(s16 a0, s16 a1, s16 a2, u16 a3) {
    u8 i;
    u8 count;

    count = 0;
    for (i = 0; i < D_8008E9D0; i++) {
        if (D_8008D994[i].unk0 != a3) {
            continue;
        }
        if (D_8008D99A[i].unk0 != a2) {
            continue;
        }
        if (D_8008D996[i].unk0 != a0) {
            continue;
        }
        if (D_8008D99E[i].unk0 != a1) {
            continue;
        }
        if (D_8008D988[i].unk0 == 0xFF) {
            D_8008D9A3[i].unk0 = 0;
            D_8008D98C[i].unk0 = 0;
            D_8006DAD4->unk194 = 0;
            D_8006DAD4->unk196 = 0;
        } else {
            u16 chan;
            u16 lowMask;
            u16 highMask;

            D_8008EA26 = i;
            chan = D_8008EA26;
            if (chan < 0x10) {
                lowMask = 1 << chan;
                highMask = 0;
            } else {
                lowMask = 0;
                highMask = 1 << (chan - 0x10);
            }
            D_8008D9A3[chan].unk0 = 0;
            D_8008D98C[chan].unk0 = 0;
            D_8008D988[chan].unk0 = 0;
            D_80090C60 |= lowMask;
            D_80090C64 |= highMask;
            D_8008E228 &= ~D_80090C60;
            D_8008E22C &= ~D_80090C64;
        }
        count++;
    }
    return count;
}
