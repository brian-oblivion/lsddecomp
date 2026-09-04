/*
 * class_3bb8c_r -- functions 23..43 of the 113-function `class_3bb8c_n`
 * remainder, 0x46288..0x46D20 (vram 0x80055A88..0x80056520).  Carved MID-round
 * 17 (2026-09-04) to re-staff a runner whose own unit was exhausted.
 *
 * Blocker census, three-grep screen run per function at carve time:
 * 20 of 21 clean, and ZERO trivial leaves -- every one is a real body, several
 * in the 40-70 instruction range.
 *
 * BLOCKED, stub report already filed, do NOT spend attempts on it:
 *   gp_rel: func_8005630C (only 5 instructions, so nothing is lost)
 *
 * Owns NO switch jump table -- zero `jtbl_` references in the slice -- so no
 * rodata sub-slot is attached to this unit.
 *
 * The 23 functions in FRONT of this slice (still `class_3bb8c_n`) are the
 * gp_rel-densest ground in the executable -- exactly two of them are clean --
 * which is why the cut is here rather than at the segment start.
 *
 * EXPECT THIS SLICE TO SPAN MORE THAN ONE CLASS.  It is cut at ROM addresses,
 * not at class boundaries.  Identify each class with tools/classtable.py
 * rather than assuming the unit has one.
 *
 * Confirmed round 17 continuation (runner bravo): TWO classes, resolved with
 * `tools/classtable.py --scan` plus a direct read of `asm/data/76DC8.data.s`
 * since one table's own slot count coincides with (but is NOT) BasicClass's:
 *
 *  - func_80055A88 .. func_80056238 (14 functions) are the complete slot
 *    list of `D_800874B0` (14 slots, header word 0). Despite matching
 *    BasicClassMethods' slot COUNT, these bodies are NOT
 *    add/removeChild-shaped -- every one calls the shared helper
 *    `func_8005627C(ctx)` then dispatches on `target->unk4` ("kind") to
 *    fill in a handful of numeric fields. Kept as its own local view,
 *    `ParamObj`/`ParamMethods` -- nothing here justifies asserting a
 *    BasicClass relationship just because the slot count coincides.
 *  - func_800563C0 (ctor) / func_80056464 (dtor) / func_800564A4 (slot40)
 *    / func_800564F4 are occupants of `D_800876FC` -- the SAME sibling
 *    table `class_3bb8c_o.c` (round 17, previous pass, already merged)
 *    partly resolved from the OTHER side (its own `func_80056F4C` returns
 *    `&D_800876FC`, and its shared-base slots +0x010/+0x014/+0x018/+0x088/
 *    etc. are INHERITED, not overridden, by this class). This unit
 *    supplies the class's OWN slots (ctor/dtor/slot40), confirmed by both
 *    chaining to `func_80057C84()` -- the SAME shared-base getter
 *    `class_3bb8c_o.c` already used for its own ctor/New_X pair. Kept as
 *    this unit's own local view, `Obj876FC`/`Obj876FCMethods` --
 *    `class_3bb8c_o.c` is not this unit's to edit, and per the
 *    multiple-independent-local-views convention there is no reason a
 *    fresh view here should match its field names field-for-field.
 *  - func_80056320 is a plain `New_X` allocator (0x98 bytes) for the
 *    `D_800876FC` class, dispatching through `func_80056F4C()->ctor`
 *    (cross-unit call into the ALREADY-MATCHED `class_3bb8c_o.c` symbol)
 *    rather than calling `func_800563C0` by name.
 *  - func_8005627C is the shared helper every `D_800874B0` occupant calls
 *    first (and `func_800560E4` reaches transitively, via a plain call to
 *    `func_80056054`): reads a small tag byte off `ctx->methods`, looks it
 *    up (with a NEGATIVE index) into a global table, and returns a
 *    chained division result.
 */
#include "common.h"

/* ------------------------------------------------------------------ *
 * D_800874B0's 14 slots (func_80055A88..func_80056238) plus the shared
 * helper func_8005627C they all call first.
 * ------------------------------------------------------------------ */

typedef struct ParamObj ParamObj;
typedef struct ParamMethods {
    u8 pad0[0x6];
    s8 tag; /* +0x006, a small type id -- read signed, used as a NEGATIVE
             * index into D_80087474 (see func_8005627C). */
} ParamMethods;
struct ParamObj {
    ParamMethods *methods; /* +0x000 */
    s32 unk4;                /* +0x004, a "kind" selector the 14 slot
                               * occupants below all dispatch on */
    u8 pad8[0x10 - 0x8];        /* +0x008 .. +0x00F, unknown */
    s32 unk10;                    /* +0x010, func_8005627C's own result */
    u8 pad14[0x1C - 0x14];          /* +0x014 .. +0x01B, unknown */
    s32 unk1C;
    s32 unk20;
    s32 unk24;
    s32 unk28;
    s32 unk2C;
    s32 unk30;
    s32 unk34;
    s32 unk38;
    s32 unk3C;
    u8 pad40[0x44 - 0x40]; /* +0x040 .. +0x043, unknown */
    s32 unk44;
    s32 unk48;
    s32 unk4C;
    s32 unk50;
};

/* func_8005627C's own ROM address (0x8005627C) is AFTER all 14 slot
 * occupants below (it sits right before the blocked func_8005630C), so
 * its definition lives in that position further down this file to keep
 * strict ROM-address order -- forward-declared here since every occupant
 * calls it. */
s32 func_8005627C(ParamObj *ctx);

void func_80055A88(ParamObj *ctx, ParamObj *self) {
    s32 kind;

    self->unk10 = func_8005627C(ctx);
    kind = self->unk4;
    if (kind == 0) {
        self->unk1C = 7;
        self->unk20 = 0;
    } else if (kind == 2) {
        self->unk30 = 7;
        self->unk34 = 0;
    } else if (kind == 5) {
        self->unk44 = 7;
        self->unk48 = 0;
    } else if (kind >= 8) {
        self->unk4 = -1;
    }
}

void func_80055B10(ParamObj *ctx, ParamObj *self) {
    s32 kind;

    self->unk10 = func_8005627C(ctx);
    kind = self->unk4;
    if (kind == 0) {
        self->unk1C = 0x18;
        self->unk20 = -2;
    } else if (kind >= 0x401) {
        self->unk4 = -1;
    }
}

void func_80055B6C(ParamObj *ctx, ParamObj *self) {
    s32 kind;

    self->unk10 = func_8005627C(ctx);
    kind = self->unk4;
    if (kind == 0) {
        self->unk1C = 0xC;
        self->unk20 = 2;
    } else if (kind >= 5) {
        self->unk4 = -1;
    }
}

void func_80055BC8(ParamObj *ctx, ParamObj *self) {
    self->unk10 = func_8005627C(ctx);
    if (self->unk4 % 20 == 0) {
        self->unk1C = 0x1E;
        self->unk24 = 0x20;
        self->unk20 = 0;
        self->unk28 = 0xA;
    }
    if (self->unk4 % 400 == 0) {
        self->unk30 = 0x1E;
        self->unk34 = 0;
    }
    self->unk44 = 6;
    self->unk4C = 0x20;
    self->unk48 = 0;
    self->unk50 = 0xA;
}

void func_80055CA8(ParamObj *ctx, ParamObj *self) {
    self->unk10 = func_8005627C(ctx);
    if (self->unk4 % 3 == 0) {
        self->unk1C = 0x1E;
        self->unk20 = 0;
    }
    if (self->unk4 % 5 == 0) {
        self->unk30 = 0x1E;
        self->unk34 = 0;
        self->unk38 = 0x18;
        self->unk3C = 0x18;
    }
    if (self->unk4 % 7 == 0) {
        self->unk1C = 0x1E;
        self->unk20 = 0;
    }
    self->unk44 = 6;
    self->unk48 = 1;
    self->unk4C = 0x2A;
    self->unk50 = 0xA;
}

void func_80055DB4(ParamObj *ctx, ParamObj *self) {
    self->unk10 = func_8005627C(ctx);
    if (self->unk4 == 0) {
        self->unk1C = 0x1E;
        self->unk20 = -1;
    } else if (self->unk4 < 0x32 && self->unk4 % 5 == 4) {
        self->unk30 = 0x1E;
        self->unk34 = 0;
        self->unk38 = self->unk38 - self->unk4 * 2;
        self->unk3C = self->unk38;
    } else if ((u32)(self->unk4 - 0x65) < 9) {
        self->unk44 = 0xD;
        self->unk48 = 1;
    } else if (self->unk4 >= 0xC9) {
        self->unk4 = -1;
    }
}

void func_80055E94(ParamObj *ctx, ParamObj *self) {
    s32 kind;

    self->unk10 = func_8005627C(ctx);
    kind = self->unk4;
    if (kind == 0) {
        self->unk1C = 7;
        self->unk20 = 2;
    } else if (kind >= 0x1B) {
        self->unk4 = -1;
    }
}

void func_80055EF0(ParamObj *ctx, ParamObj *self) {
    s32 kind;

    self->unk10 = func_8005627C(ctx);
    kind = self->unk4;
    if (kind == 0) {
        self->unk1C = 0x14;
        self->unk20 = 1;
    } else if (kind == 3) {
        self->unk20 = 2;
        self->unk24 = 0x18;
        self->unk1C = kind;
        self->unk28 = 0x14;
    } else if (kind >= 0x33) {
        self->unk4 = -1;
    }
}

void func_80055F74(ParamObj *ctx, ParamObj *self) {
    self->unk10 = func_8005627C(ctx);
    if (self->unk4 % 20 == 0) {
        self->unk1C = 9;
        self->unk20 = 0;
        self->unk24 = 0x40;
        self->unk28 = 0x40;
    }
}

void func_80055FE8(ParamObj *ctx, ParamObj *self) {
    self->unk10 = func_8005627C(ctx);
    if (self->unk4 % 20 == 0) {
        self->unk1C = 9;
        self->unk20 = -2;
    }
}

void func_80056054(ParamObj *ctx, ParamObj *self) {
    s32 rem;

    self->unk10 = func_8005627C(ctx);
    rem = self->unk4 % 20;
    if (rem == 1) {
        self->unk1C = 9;
        self->unk20 = -2;
    } else if (rem == 16) {
        self->unk30 = 9;
        self->unk34 = -2;
    }
}

void func_800560E4(ParamObj *ctx, ParamObj *self) {
    s32 rem;

    func_80056054(ctx, self);
    rem = self->unk4 % 70;
    if (rem == 50) {
        self->unk44 = 0x14;
        self->unk48 = 1;
    } else if ((u32)(rem - 54) < 5) {
        self->unk44 = 0xD;
        self->unk48 = 1;
    } else if (rem == 61) {
        self->unk44 = 9;
        self->unk48 = -1;
    }
}

void func_80056194(ParamObj *ctx, ParamObj *self) {
    s32 kind;

    self->unk10 = func_8005627C(ctx);
    kind = self->unk4;
    if (kind == 0) {
        self->unk1C = 0x14;
        self->unk20 = -2;
        self->unk30 = 0x14;
        self->unk34 = -2;
    } else if (kind == 4) {
        self->unk30 = 0x14;
        self->unk34 = -2;
    } else if (kind == 0x14) {
        self->unk1C = 0x10;
        self->unk20 = -2;
        self->unk30 = 0x12;
        self->unk34 = -2;
    } else if (kind >= 0xC9) {
        self->unk4 = -1;
    }
}

void func_80056238(ParamObj *ctx, ParamObj *self) {
    self->unk10 = func_8005627C(ctx);
    if (self->unk4 == 0) {
        self->unk1C = 0x18;
        self->unk20 = 0;
    }
}

/* A 15-entry table indexed with the NEGATIVE of `ctx->methods->tag`
 * (`D_80087474 - tag*4`, i.e. `D_80087474[-tag]` for `tag` in [-14, 0]).
 * `asm/data/76DC8.data.s` confirms exactly 15 words at this address. */
extern s32 D_80087474[];

s32 func_8005627C(ParamObj *ctx) {
    s32 t = D_80087474[-ctx->methods->tag];
    s32 q = t / ctx->unk28;

    return ctx->unk10 / q;
}

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_r", func_8005630C);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_r", func_80056320);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_r", func_800563C0);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_r", func_80056464);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_r", func_800564A4);

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_r", func_800564F4);
