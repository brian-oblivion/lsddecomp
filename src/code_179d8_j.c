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

/* ------------------------------------------------------------------------
 * Cross-unit calls, typed per-call-site from the registers loaded before
 * each `jal` -- none of these callees have an established prototype yet
 * (several are themselves addiu-$at blocked in their own units), so these
 * are local guesses, not authoritative.  See CLAUDE.md's note on this.
 * ------------------------------------------------------------------------ */
extern s32 func_8002FAC4(s32 a0, s16 a1, s16 a2, u16 a3, u16 a4, u16 a5);
extern s32 func_800300D0(s32 a0, s16 a1, s16 a2, u16 a3);
extern s32 func_80032148(s16 a0, s16 a1);
extern s16 func_8002F3E8(s16 a0, s32 a1, s16 a2, s16 a3, u16 a4);
extern void func_8002E308(s16 a0, s16 a1, s16 a2, s16 a3);
extern void func_8002E874(s16 a0, s16 a1, s16 a2, s16 a3);

extern u16 D_8008EA22;

/* Base pointer for a table of 0x10-byte slots, indexed by the small
 * (<0x10) channel id `func_80032148` validates/selects.  Only the two
 * byte fields this unit's own accessors touch are named -- everything
 * else is opaque per this project's local-reading convention. */
typedef struct SlotE968 {
    u8 pad0[0x1];
    u8 unk1; /* +0x1 */
    u8 pad2[0x4 - 0x2];
    u8 unk4; /* +0x4 */
    u8 pad5[0x10 - 0x5];
} SlotE968;
extern SlotE968 *D_8008E968;

/* Base pointer for a table of 0x10-byte entries, indexed by a 0..0x17
 * id.  Only the two leading s16 fields this unit's own accessors touch
 * are named. */
typedef struct EntryDAD4 {
    s16 unk0; /* +0x0 */
    s16 unk2; /* +0x2 */
    u8 pad4[0x10 - 0x4];
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

INCLUDE_ASM("asm/nonmatchings/code_179d8_j", func_80030648);

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

INCLUDE_ASM("asm/nonmatchings/code_179d8_j", func_80031280);

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

INCLUDE_ASM("asm/nonmatchings/code_179d8_j", func_80031A44);

INCLUDE_ASM("asm/nonmatchings/code_179d8_j", func_80031BA4);

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
