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

INCLUDE_ASM("asm/nonmatchings/code_179d8_m", func_8002EA44);

INCLUDE_ASM("asm/nonmatchings/code_179d8_m", func_8002EDD4);

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

INCLUDE_ASM("asm/nonmatchings/code_179d8_m", func_8002F3E8);

/* "Currently selected channel" scratch global -- same idiom as
 * D_8008EA26 above, write-only here (see code_179d8_j.c's own reading
 * of this symbol). */
extern u16 D_8008EA22;

extern s32 func_80032148(s16 a0, s16 a1);
extern s16 func_8002F3E8(s16 a0, s32 a1, s16 a2, s16 a3, u16 a4);

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

INCLUDE_ASM("asm/nonmatchings/code_179d8_m", func_8002F700);

INCLUDE_ASM("asm/nonmatchings/code_179d8_m", func_8002FAC4);

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
