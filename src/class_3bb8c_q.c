/*
 * class_3bb8c_q -- functions 94..95 of the class_3bb8c remainder,
 * 0x485BC..0x48738 (95 words).  Carved round 47 (2026-09-16).
 *
 * THE CARVE NOTE THAT STOOD HERE FOR 26 ROUNDS WAS STALE AND SAID "nothing to
 * staff here": func_80057DBC was filed as addiu-$at blocked (resolved round
 * 21) and func_80057DF4 as nop_mflo_mfhi blocked (resolved round 42).
 * `tools/uncarved.py` measures both blocker-clean.  Both are frameless leaves
 * (zero `addiu $sp, $sp, -N`), so neither is expected to have a stack frame.
 *
 * Owns no jump table, so no rodata attach.  Expect this slice to span more
 * than one class; identify each with `tools/classtable.py`.
 */

#include "common.h"

/*
 * Table D_800879C4 (49 slots, resolved with `tools/classtable.py
 * 0x800879C4`): func_80057DBC is slot40, func_80057DF4 is slot48.  The
 * neighbouring class_3bb8c_p unit already carries its OWN local view of
 * this same table/object (`D_800879C4Methods`/`D_800879C4Obj` in that
 * file, only exposing the ctor slot and `+0xA4`) -- per the project's
 * multiple-independent-local-views convention this unit does not touch
 * that file, it defines its own view sized for what THESE two functions
 * read/write. The allocator (class_3bb8c_p's New_D800879C4) sizes the
 * object at 0xA8 bytes, which every offset below stays inside.
 */
typedef struct D_800879C4Obj_q D_800879C4Obj_q;
struct D_800879C4Obj_q {
    u8 pad00[0x58];
    s32 unk58;                  /* +0x058, func_80057DF4: non-zero selects the scale-in-place path */
    s32 unk5C;                  /* +0x05C, func_80057DF4: scaled in place by the first ratio when unk58 != 0 */
    s32 unk60;                  /* +0x060, func_80057DF4: scaled in place by the second ratio when unk58 != 0 */
    u8 pad64[0x74 - 0x64];
    s16 unk74;                  /* +0x074, func_80057DBC: D_80087AA4[arg1] */
    s16 unk76;                  /* +0x076, func_80057DBC: D_80087AA6[arg1] */
    u8 pad78[0x80 - 0x78];
    s16 unk80;                  /* +0x080, func_80057DF4: raw first ratio (truncated) when unk58 == 0 */
    s16 unk82;                  /* +0x082, func_80057DF4: raw second ratio (truncated) when unk58 == 0 */
    u8 pad84[0xA0 - 0x84];
    s32 unkA0;                  /* +0x0A0, func_80057DBC: the raw index argument, stored verbatim */
};

/*
 * Two parallel lookup tables, 2 entries each, stride 4 bytes (indexed as
 * `arg1 * 2` of `s16`, i.e. `arg1 * 4` bytes) even though each holds only
 * a 2-byte element at that stride -- retail takes two SEPARATE %hi/%lo
 * bases (D_80087AA4, D_80087AA6) rather than one struct array, so this is
 * the only spelling that reproduces the two relocations. Values:
 * D_80087AA4 = {0x03D0, 0x03E0}, D_80087AA6 = {0x01FF, 0x01FF} once read
 * at the real stride (confirmed against asm/data/76DC8.data.s).
 */
extern const s16 D_80087AA4[];
extern const s16 D_80087AA6[];

void func_80057DBC(D_800879C4Obj_q *self, s32 arg1) {
    self->unkA0 = arg1;
    self->unk74 = D_80087AA4[arg1 * 2];
    self->unk76 = D_80087AA6[arg1 * 2];
}

/*
 * `pair` is a caller-supplied `{s16 whole; s16 frac;}` pair, twice over
 * (axis 0 at +0x0/+0x2, axis 1 at +0x4/+0x6) -- same 20.12 fixed-point
 * split-division idiom as code_2cc8c_d.c's func_8003EC2C and
 * code_d294_c.c's RatioToFixed12 (divide once for quotient+remainder, then
 * divide the shifted remainder again for the fractional part), read as a
 * raw `s16 *` rather than a named struct per those units' own precedent
 * for this exact shape (code_2cc8c.h's note on WholeFrac_d294: "a
 * different unit's own local view of the same shape, not a shared
 * type"). `arg1` is read by nothing in the whole function body (retail
 * never touches it after entry) -- kept as an unused parameter to match
 * the real arity.
 */
void func_80057DF4(D_800879C4Obj_q *self, s32 arg1, s16 *pair) {
    s32 q1, r1, q2, ratio1;
    s32 q3, r3, q4, ratio2;
    s16 short1, short2;

    q1 = pair[0] / pair[1];
    r1 = pair[0] % pair[1];
    q2 = (r1 << 12) / pair[1];
    ratio1 = (q1 << 12) + q2;
    short1 = (s16)ratio1;

    q3 = pair[2] / pair[3];
    r3 = pair[2] % pair[3];
    q4 = (r3 << 12) / pair[3];
    ratio2 = (q3 << 12) + q4;
    short2 = (s16)ratio2;

    if (self->unk58 != 0) {
        self->unk5C = ((s16)ratio1 * self->unk5C) >> 12;
        self->unk60 = ((s16)ratio2 * self->unk60) >> 12;
    } else {
        self->unk80 = short1;
        self->unk82 = short2;
    }
}
