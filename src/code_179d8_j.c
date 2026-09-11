/*
 * code_179d8_j -- functions 173..198 of the original code_179d8 monolith,
 * 0x20ADC..0x2273C (26 functions).  Carved round 21 (2026-09-06) out of the
 * MIDDLE of the old code_179d8_mid_c, chosen by blocker density rather than
 * by "next": the head of that segment is 5 clean of 24, this window is 12
 * clean of 26.  code_179d8_mid_c contains zero jtbl/.word .L, so no unit
 * here owns rodata.
 *
 * Blocker census at carve time (four screens, canonical shell forms):
 * 12 of 26 clean, 13 addiu-$at, 1 addiu-$at + nop_mflo_mfhi.  It ended
 * "do not re-screen them and do not spend attempts on them."
 *
 * THAT CENSUS IS STALE -- re-screened round 23 (2026-09-07).  `addiu_at` was
 * resolved in round 21 (maspsx `--addiu-at`; docs/research/addiu-at-blocker.md)
 * and screening for it now INVENTS blockers.  The 13 it lists are NOT blocked;
 * several have already been matched in the rounds since, and round 23's runner
 * alpha worked four of the remainder to characterised non-toolchain residues
 * (func_80031890 73/73 length with scattered register identity, func_8003069C
 * 85/85 length, func_80031A44 one word short at 87/88 -- a strong permuter
 * target -- and func_80030404 five short at 91/96).  The live screen is TWO
 * greps, `gp_rel` and `nop_mflo_mfhi`; run `python3 tools/nearmiss.py` rather
 * than trusting any transcribed census, this one included.
 *
 * The WORKABLE list below is likewise a carve-time snapshot and several of its
 * entries are now matched.  Derive the live queue from the file's own
 * INCLUDE_ASM entries, not from this comment.
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

/* ------------------------------------------------------------------------
 * Cross-unit calls, typed per-call-site from the registers loaded before
 * each `jal` -- none of these callees have an established prototype yet, so
 * these are local guesses, not authoritative.  (This note used to add "several
 * are themselves addiu-$at blocked in their own units"; that is stale as of
 * round 21 and was removed rather than left to be believed.)  See CLAUDE.md's note on this.
 * ------------------------------------------------------------------------ */
extern s32 func_8002FAC4(s32 a0, s16 a1, s16 a2, u16 a3, u16 a4, u16 a5);
extern s32 func_800300D0(s32 a0, s16 a1, s16 a2, u16 a3);
extern s32 func_80032148(s16 a0, s16 a1);
extern s16 func_8002F3E8(s16 a0, s32 a1, s16 a2, s16 a3, u16 a4);
extern void func_8002E308(s16 a0, s16 a1, s16 a2, s16 a3);
extern void func_8002E874(s16 a0, s16 a1, s16 a2, s16 a3);

extern u16 D_8008EA22;

/* Base pointer for a table of 0x10-byte slots, indexed by the small
 * (<0x10) channel id `func_80032148` validates/selects.  Only the three
 * byte fields this unit's own accessors touch are named -- everything
 * else is opaque per this project's local-reading convention. */
typedef struct SlotE968 {
    u8 unk0; /* +0x0 */
    u8 unk1; /* +0x1 */
    u8 pad2[0x4 - 0x2];
    u8 unk4; /* +0x4 */
    u8 pad5[0x10 - 0x5];
} SlotE968;
extern SlotE968 *D_8008E968;

/* 0x20-byte-stride record indexed by `D_8008EA18 + D_8008EA13*16`
 * (func_80030E90's own computed index, not a channel id). Every field
 * this unit's own accessor touches is named; offsets are exact (read
 * from func_80030E90's own lbu/lhu immediates), field names are not. */
typedef struct {
    u8 unk0;  /* +0x0 */
    u8 unk1;  /* +0x1 */
    u8 unk2;  /* +0x2 */
    u8 unk3;  /* +0x3 */
    u8 unk4;  /* +0x4 */
    u8 unk5;  /* +0x5 */
    u8 unk6;  /* +0x6 */
    u8 unk7;  /* +0x7 */
    u8 pad8[0x16 - 0x8];
    u16 unk16; /* +0x16 */
    u8 pad18[0x20 - 0x18];
} RecordE978;
extern RecordE978 *D_8008E978;

/* Base pointer for a table of 0x10-byte entries, indexed by a 0..0x17
 * id.  Only the two leading s16 fields this unit's own accessors touch
 * are named. */
typedef struct EntryDAD4 {
    s16 unk0; /* +0x0 */
    s16 unk2; /* +0x2 */
    s16 unk4; /* +0x4 -- read by func_80031280, entry index 25 only */
    s16 unk6; /* +0x6 -- read by func_80031280, entry index 25 only */
    u8 pad8[0x10 - 0x8];
} EntryDAD4;
extern EntryDAD4 *D_8006DAD4;

/* A 172 (0xAC)-byte record; D_800902E8 is an array of pointers to arrays of
 * these, indexed [screen][slot]-style by a packed argument (slot in the
 * high byte, screen in the low byte) -- see code_179d8_i.c's own
 * func_800339AC, which builds exactly this packing before calling into
 * this unit's func_8003069C. Reduced local view: only the two leading s16
 * fields this unit's own accessors touch are named. See code_179d8_f.c /
 * code_179d8_i.c's own Entry90902E8 for a fuller layout of the same array;
 * each unit keeps its own independent reading, per project convention. */
typedef struct {
    u8 pad0[0x74];
    s16 unk74; /* +0x74 */
    s16 unk76; /* +0x76 */
    u8 pad78[0xAC - 0x78];
} Entry90902E8;
extern Entry90902E8 *D_800902E8[];

/* Reentrancy lock, same identifier/type as the sibling reading in
 * code_179d8_i.c's func_80033738 -- "if already busy, return/skip;
 * set; ...; clear before returning" guarding a per-channel operation. */
extern s32 D_8008E934;

/* "Currently selected channel" scratch globals: written as a side
 * effect and then re-read from the global (not from the parameter) by
 * the same and sibling functions -- same idiom as this file's own
 * D_8008EA22 above. */
extern volatile u16 D_8008EA26;
extern volatile u8 D_8008EA18;

/* func_80030E90's own scratch globals -- a "start channel" setup
 * routine that stages its parameters and a couple of table lookups
 * into a block of one/two-byte globals before registering a new
 * active-channel record.  Offsets are exact (this unit's own field
 * accesses); names are opaque placeholders per the reduced-local-view
 * convention. */
extern u8 D_8008EA0C;
extern u8 D_8008EA0E;
extern u8 D_8008EA0F;
extern u8 D_8008EA10;
extern u8 D_8008EA11;
extern u8 D_8008EA13;
extern u8 D_8008EA16;
extern u8 D_8008EA17;
extern u8 D_8008EA19;
extern u8 D_8008EA1A;
extern u8 D_8008EA1B;
extern u8 D_8008EA1C;
extern u8 D_8008EA1D;
extern u8 D_8008EA1E;
extern u8 D_8008EA1F;
extern u8 D_8008EA20;
extern u16 D_8008EA24;

extern s32 func_8002CF18(void);
extern void func_8002D6A4(void);
extern void func_8002D8E0(s32 a0);
extern s32 func_8002E038(u16 a0, u16 a1);
extern void func_8002D1B4(s32 a0, u16 a1);

/* Loop bound for a small table of active "objects" (screen/slot
 * pairs); see code_179d8_i.c's D_80090B68/6C for the sibling reading of
 * an analogous count. */
extern u8 D_8008E9D0;

/* A pair of 16-bit bitmasks split across a 0..0x1F channel space
 * (low 16 channels in the first word, next 16 in the second), each
 * paired with an "active mask" word that is cleared wherever the
 * channel mask bit is set. */
extern u16 D_80090C60;
extern u16 D_80090C64;
extern u16 D_8008E228;
extern u16 D_8008E22C;

/* 52 (0x34)-byte-stride channel-configuration record, referenced by
 * several unrelated top-level symbols 2 bytes apart (D_8008D994,
 * D_8008D996, D_8008D99A, D_8008D99C, D_8008D99E, ...) -- each function
 * in this unit only touches the one or two fields it actually reads,
 * per this project's reduced-local-view convention. Only the leading
 * s16 is named; every array below shares this one shape. */
typedef struct {
    s16 unk0; /* +0x0 */
    u8 pad2[0x34 - 0x2];
} Rec34D994;
extern Rec34D994 D_8008D994[];
extern Rec34D994 D_8008D996[];
extern Rec34D994 D_8008D998[];
extern Rec34D994 D_8008D99A[];
extern Rec34D994 D_8008D99C[];
extern Rec34D994 D_8008D99E[];

/* Same 0x34 stride, byte-sized "in use" flag at offset 0 (retail always
 * clears it with `sb`, never `sh`). */
typedef struct {
    u8 unk0; /* +0x0 */
    u8 pad1[0x34 - 0x1];
} Rec34Byte;
extern Rec34Byte D_8008D9A3[];

/* Same 0x34 stride, halfword field at offset 0 (retail always clears it
 * with `sh`). Three independent arrays share this shape (D_8008D988,
 * D_8008D98A and D_8008D98C, each 2 bytes apart in the data section --
 * same "several unrelated top-level symbols" convention as the
 * Rec34D994 group above). D_8008D988's field is read signed
 * (func_80031280 compares it against 0xFF with `lh`, not `lhu`). */
typedef struct {
    s16 unk0; /* +0x0 */
    u8 pad2[0x34 - 0x2];
} Rec34Half;
extern Rec34Half D_8008D988[];
extern Rec34Half D_8008D98A[];
extern Rec34Half D_8008D98C[];

/* 16 (0x10)-byte-stride record with two s16 fields 2 bytes apart --
 * modeled as ONE struct here (not two independent arrays) because
 * func_80030404 computes the second field's address as the first
 * field's cached base register plus a compile-time +0x2, which only
 * happens when the compiler knows both offsets belong to the same
 * object. */
typedef struct {
    s16 unk0; /* +0x0 */
    s16 unk2; /* +0x2 */
    u8 pad4[0x10 - 0x4];
} Rec16D7F0;
extern Rec16D7F0 D_8008D7F0[];
extern Rec16D7F0 D_8008D7F4[]; /* independent array, same shape */

/* Per-slot flag byte, same 0..0x17 id as several of the tables above. */
extern u8 D_8008D970[];

s32 func_800302DC(s32 p0, s32 p1, s32 p2, s32 p3, u16 p4, u16 p5)
{
    u16 outA;
    u16 outB;

    if (p4 == p5) {
        outB = 0x40;
        outA = p4;
    } else if (p5 < p4) {
        outA = p4;
        outB = (p5 << 6) / p4;
    } else {
        outA = p5;
        outB = 0x7F - ((p4 << 6) / p5);
    }
    return func_8002FAC4(0x21, (s16) p0, (s16) p1, (u16) p2, outA, outB);
}

s32 func_800303C8(s16 p0, s16 p1, u16 p2)
{
    return func_800300D0(0x21, p0, p1, p2);
}

void func_800303FC(void) {
}

INCLUDE_ASM("asm/nonmatchings/code_179d8_j", func_80030404);

s32 func_80030584(s32 p0, s16 *out1, s16 *out2)
{
    Entry90902E8 *tbl = D_800902E8[(u8) p0];
    s16 *cur = (s16 *) &D_8008EA22;

    *cur = (s16) p0;
    *out1 = tbl[(p0 & 0xFF00) >> 8].unk74;
    *out2 = tbl[(p0 & 0xFF00) >> 8].unk76;
    return *cur;
}

s32 func_800305F4(s32 p0)
{
    s32 channel = p0 & 0xFF;
    Entry90902E8 *tbl = D_800902E8[channel];
    s32 recIdx = (p0 & 0xFF00) >> 8;

    __asm__("");
    D_8008EA22 = channel;
    return tbl[recIdx].unk74;
}

s32 func_80030648(s32 p0)
{
    Entry90902E8 *tbl = D_800902E8[(u8) p0];

    __asm__("");
    D_8008EA22 = p0;
    return tbl[(p0 & 0xFF00) >> 8].unk76;
}

INCLUDE_ASM("asm/nonmatchings/code_179d8_j", func_8003069C);

s32 func_800307F0(s16 p0, s16 p1, s32 p2)
{
    if (func_80032148(p0, p1) != 0)
        return -1;
    D_8008E968[p1].unk1 = (u8) p2;
    return D_8008E968[p1].unk1;
}

s32 func_80030864(s16 p0, s16 p1)
{
    if (func_80032148(p0, p1) != 0)
        return -1;
    return D_8008E968[p1].unk1;
}

s32 func_800308B8(s16 p0, s16 p1, s32 p2)
{
    if (func_80032148(p0, p1) != 0)
        return -1;
    D_8008E968[p1].unk4 = (u8) p2;
    return D_8008E968[p1].unk4;
}

s32 func_8003092C(s16 p0, s16 p1)
{
    if (func_80032148(p0, p1) != 0)
        return -1;
    return D_8008E968[p1].unk4;
}

INCLUDE_ASM("asm/nonmatchings/code_179d8_j", func_80030980);

INCLUDE_ASM("asm/nonmatchings/code_179d8_j", func_80030E90);

s32 func_80031280(s16 idx, s16 p1, s16 p2, s16 p3, s16 p4)
{
    u16 chan;
    u32 mask0;
    u16 mask1;

    if (D_8008E934 == 1) {
        goto fail_nolock;
    }
    D_8008E934 = 1;
    if ((u16) idx >= 0x18) {
        goto fail;
    }
    if (D_8008D99E[idx].unk0 != p1
     || D_8008D99A[idx].unk0 != p2
     || D_8008D99C[idx].unk0 != p3
     || D_8008D994[idx].unk0 != p4) {
        goto fail;
    }
    if (D_8008D988[idx].unk0 == 0xFF) {
        D_8008D9A3[(u8) idx].unk0 = 0;
        D_8008D98C[(u8) idx].unk0 = 0;
        D_8006DAD4[25].unk4 = 0;
        D_8006DAD4[25].unk6 = 0;
    } else {
        D_8008EA26 = idx;
        chan = D_8008EA26;
        if (chan < 0x10) {
            mask0 = 1 << chan;
            mask1 = 0;
        } else {
            mask0 = 0;
            mask1 = 1 << (chan - 0x10);
        }
        D_8008D9A3[chan].unk0 = 0;
        D_8008D98C[chan].unk0 = 0;
        D_8008D988[chan].unk0 = 0;
        D_80090C60 = mask0 | D_80090C60;
        D_80090C64 |= mask1;
        D_8008E228 &= ~D_80090C60;
        D_8008E22C &= ~D_80090C64;
    }
    D_8008E934 = 0;
    return 0;

fail:
    D_8008E934 = 0;
fail_nolock:
    return -1;
}

INCLUDE_ASM("asm/nonmatchings/code_179d8_j", func_8003149C);

INCLUDE_ASM("asm/nonmatchings/code_179d8_j", func_80031890);

s32 func_800319B4(s32 p0, s16 p1, s16 p2, s32 p3, u16 p4)
{
    func_80032148(p1, p2);
    D_8008EA22 = 0x21;
    if (func_8002F3E8((s16) p0, 0x21, p1, p2, p4) == 0)
        return -1;
    return 0;
}

/* STALL -- see docs/match-reports/func_80031A44.md.  HEAD SALVAGE, round 31:
 * a mid-attempt snapshot scoring 84/88 words (no drift) was recovered from
 * the worktree and is preserved in that report as literal source.  No author
 * applied a stop rule to it. */
INCLUDE_ASM("asm/nonmatchings/code_179d8_j", func_80031A44);

/* Rec34D994 (D_8008D994/99A/99E) and Rec16D7F0 (D_8008D7F8/D_8008D7FA
 * below) are declared once, near the top of this file, and shared by
 * every function in this unit that needs them -- see the comment there.
 * See func_80031CF0's report for why D_8008D7F8/D_8008D7FA are modeled
 * as independent arrays rather than fields of one struct (each access
 * computes its own address), unlike D_8008D7F0/D_8008D7F4 above. */
extern Rec16D7F0 D_8008D7F8[];
extern Rec16D7F0 D_8008D7FA[];

/* `dead` is never read and the write is unreachable; it exists to make GCC
 * allocate retail's empty 8-byte frame, which is what puts the two
 * stack-passed arguments at 0x18/0x1C($sp) instead of 0x10/0x14.  See this
 * function's match report -- the frame is the ONLY thing the idiom is for,
 * and adding anything else on top of it breaks the scheduling. */
s32 func_80031BA4(s16 idx, s16 p1, s16 p2, s16 p3, u16 p4, u16 p5) {
    s32 dead[2];

    if ((u16)idx < 0x18) {
        if (D_8008D99E[idx].unk0 != p1) {
            return -1;
        }
        if (D_8008D99A[idx].unk0 != p2) {
            return -1;
        }
        if (D_8008D994[idx].unk0 != p3) {
            return -1;
        }
        D_8008D7F8[idx].unk0 = p4;
        __asm__("");
        D_8008D7FA[idx].unk0 = p5;
        __asm__("");
        D_8008D970[idx] |= 0x30;
        return 0;
    }
    if (0) {
        dead[0] = 1;
    }
    return -1;
}

s32 func_80031C98(s16 idx, s16 *out1, s16 *out2)
{
    if ((u16) idx < 0x18) {
        *out1 = D_8006DAD4[idx].unk0;
        *out2 = D_8006DAD4[idx].unk2;
        return 0;
    }
    return -1;
}

INCLUDE_ASM("asm/nonmatchings/code_179d8_j", func_80031CF0);

INCLUDE_ASM("asm/nonmatchings/code_179d8_j", func_80031D6C);

INCLUDE_ASM("asm/nonmatchings/code_179d8_j", func_80031DF8);

s32 func_80031E94(s16 p0, s16 p1, s16 p2, s16 p3)
{
    if ((u16) p0 < 0x18) {
        func_8002E308(p0, p1, p2, p3);
        return 0;
    }
    return -1;
}

s32 func_80031EE8(s16 p0, s16 p1, s16 p2, s16 p3)
{
    if ((u16) p0 < 0x18) {
        func_8002E874(p0, p1, p2, p3);
        return 0;
    }
    return -1;
}
