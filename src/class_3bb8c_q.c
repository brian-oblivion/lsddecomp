/*
 * class_3bb8c_q -- functions 94..95 of the class_3bb8c remainder,
 * 0x485BC..0x48738 (95 words).  Carved round 47 (2026-09-16).
 *
 * THE CARVE NOTE THAT STOOD HERE FOR 26 ROUNDS WAS STALE AND SAID "nothing to
 * staff here": D800879C4__SetVariantClut was filed as addiu-$at blocked (resolved round
 * 21) and D800879C4__UpdateScale as nop_mflo_mfhi blocked (resolved round 42).
 * `tools/uncarved.py` measures both blocker-clean.  Both are frameless leaves
 * (zero `addiu $sp, $sp, -N`), so neither is expected to have a stack frame.
 *
 * Owns no jump table, so no rodata attach.  Expect this slice to span more
 * than one class; identify each with `tools/classtable.py`.
 */

#include "common.h"

/*
 * Table D_800879C4 (49 slots, resolved with `tools/classtable.py
 * 0x800879C4`): D800879C4__SetVariantClut is slot40, D800879C4__UpdateScale is slot48.  The
 * neighbouring class_3bb8c_p unit already carries its OWN local view of
 * this same table/object (`D_800879C4Methods`/`D_800879C4Obj` in that
 * file, only exposing the ctor slot and `+0xA4`) -- per the project's
 * multiple-independent-local-views convention this unit does not touch
 * that file, it defines its own view sized for what THESE two functions
 * read/write. The allocator (class_3bb8c_p's New_D800879C4) sizes the
 * object at 0xA8 bytes, which every offset below stays inside.
 */
typedef struct D800879C4Obj D800879C4Obj;
struct D800879C4Obj {
    u8 pad00[0x58];
    /* +0x058..+0x063: written by the base class's slot +0x040 init
     * (func_8004202C, asm/psyq_322b4.s: unk58 = 0) and read only by
     * D800879C4__UpdateScale below; func_80012064's sprite draw path never
     * reads them. What sets unk58 non-zero is not established. */
    s32 unk58;                  /* +0x058, UpdateScale: non-zero selects the scale-in-place path */
    s32 unk5C;                  /* +0x05C, UpdateScale: multiplied by the x ratio when unk58 != 0 */
    s32 unk60;                  /* +0x060, UpdateScale: multiplied by the y ratio when unk58 != 0 */
    /* +0x064: an embedded GsSPRITE (layout confirmed by the base init
     * func_8004208C, which fills attribute/w/h/tpage/u/v/cx/cy/rgb/mx/my/
     * scalex/scaley/rotate at their GsSPRITE offsets, and by
     * func_80012064, which hands self+0x64 to GsSortSprite). Only the
     * members this unit touches are spelled out. */
    u8 pad64[0x74 - 0x64];
    s16 spriteClutX;            /* +0x074, GsSPRITE.cx */
    s16 spriteClutY;            /* +0x076, GsSPRITE.cy */
    u8 pad78[0x80 - 0x78];
    s16 spriteScaleX;           /* +0x080, GsSPRITE.scalex (20.12) */
    s16 spriteScaleY;           /* +0x082, GsSPRITE.scaley (20.12) */
    u8 pad84[0xA0 - 0x84];
    /* +0x0A0: the ctor's arg1, which also picks this object's texture cell
     * (&D_80087A8C[arg1], passed to the base ctor by D800879C4__D800879C4). */
    s32 variant;
};

/*
 * Two parallel lookup tables, 2 entries each, stride 4 bytes (indexed as
 * `variant * 2` of `s16`, i.e. `variant * 4` bytes) even though each holds only
 * a 2-byte element at that stride -- retail takes two SEPARATE %hi/%lo
 * bases (gD800879C4ClutX, gD800879C4ClutY) rather than one struct array, so this is
 * the only spelling that reproduces the two relocations. Values:
 * gD800879C4ClutX = {0x03D0, 0x03E0}, gD800879C4ClutY = {0x01FF, 0x01FF} once read
 * at the real stride (confirmed against asm/data/76DC8.data.s).
 */
extern const s16 gD800879C4ClutX[];
extern const s16 gD800879C4ClutY[];

void D800879C4__SetVariantClut(D800879C4Obj *self, s32 variant) {
    self->variant = variant;
    self->spriteClutX = gD800879C4ClutX[variant * 2];
    self->spriteClutY = gD800879C4ClutY[variant * 2];
}

/*
 * Slot +0x048, overriding Class6B5CC__UpdateScale (tools/classtable.py
 * D_800879C4 --vs gClass6B5CCMethods). `ratios` is two s16 num/den pairs
 * (x at +0x0/+0x2, y at +0x4/+0x6) -- every caller passes one of
 * gSpriteScaleLarge/gSpriteScaleSmall ({6,5}, {4,6}) or a table forwarded
 * from Class876FC__SpawnSprites -- each turned into a 20.12 fixed-point
 * ratio by the split-division idiom RatioToFixed12 uses (code_d294_c.c).
 * Unlike the base method, `set` is never read: every path assigns. Read as
 * a raw `s16 *` per the RatioToFixed12 precedent for this shape.
 */
void D800879C4__UpdateScale(D800879C4Obj *self, s32 set, s16 *ratios) {
    s32 q1, r1, q2, ratio1;
    s32 q3, r3, q4, ratio2;
    s16 short1, short2;

    q1 = ratios[0] / ratios[1];
    r1 = ratios[0] % ratios[1];
    q2 = (r1 << 12) / ratios[1];
    ratio1 = (q1 << 12) + q2;
    short1 = (s16)ratio1;

    q3 = ratios[2] / ratios[3];
    r3 = ratios[2] % ratios[3];
    q4 = (r3 << 12) / ratios[3];
    ratio2 = (q3 << 12) + q4;
    short2 = (s16)ratio2;

    if (self->unk58 != 0) {
        self->unk5C = ((s16)ratio1 * self->unk5C) >> 12;
        self->unk60 = ((s16)ratio2 * self->unk60) >> 12;
    } else {
        self->spriteScaleX = short1;
        self->spriteScaleY = short2;
    }
}
