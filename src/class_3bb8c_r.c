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
 * class_3bb8c_r -- functions 23..43 of the 113-function `class_3bb8c_n`
 * remainder, 0x46288..0x46D20 (vram 0x80055A88..0x80056520).  Carved MID-round
 * 17 (2026-09-04) to re-staff a runner whose own unit was exhausted.
 *
 * Blocker census, three-grep screen run per function at carve time:
 * 20 of 21 clean, and ZERO trivial leaves -- every one is a real body, several
 * in the 40-70 instruction range.
 *
 * func_8005630C: MATCHED round 44 (was filed BLOCKED/gp_rel; reopened and
 * closed 5/5 on the first build after the fix -- see
 * docs/match-reports/func_8005630C.md).
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
 *    chaining to `DreamSys__GetBaseMethods()` -- the SAME shared-base getter
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

extern s32 gStyleVariant;

s32 func_8005630C(void) {
    return (gStyleVariant & 1) ^ 1;
}

/* ------------------------------------------------------------------ *
 * D_800876FC's own slots (ctor/dtor/slot40), plus its `New_X` allocator.
 * See the file banner: this is the SAME sibling table `class_3bb8c_o.c`
 * (round 17, previous pass) already partly resolved; this unit's own
 * local view is kept independent, per the multiple-independent-views
 * convention -- `class_3bb8c_o.c` is not this unit's to edit.
 * ------------------------------------------------------------------ */

typedef struct Obj876FC Obj876FC;
typedef struct Obj876FCMethods {
    u8 pad0[0x8];
    void *(*ctor)(Obj876FC *self, void *arg1, void *arg2, void *arg3, void *arg4); /* +0x008 func_800563C0 (this unit) */
    void *(*dtor)(Obj876FC *self);                                                   /* +0x00C func_80056464 (this unit) */
    u8 pad10[0x40 - 0x10];                                                              /* +0x010 .. +0x03F, shared-base slots, not this unit's to name */
    void (*slot40)(Obj876FC *self, void *arg1);                                           /* +0x040 func_800564A4 (this unit) */
} Obj876FCMethods;

/* Declared as a WORD array, not a byte array, so a whole-struct assignment
 * reproduces retail's aligned 4-word-per-iteration block-move codegen
 * (the already-confirmed idiom, e.g. DreamSys.h's DreamSysUnk14Tail) --
 * a byte array has alignment 1 and compiles the copy as a generic
 * runtime-alignment-checked memcpy loop instead. */
typedef struct Block24 {
    s32 raw[0x24 / 4];
} Block24;

struct Obj876FC {
    Obj876FCMethods *methods; /* +0x000 */
    u8 pad4[0x24 - 0x4];        /* +0x004 .. +0x023, unknown */
    s32 tick;                     /* +0x024, cleared by func_800564A4 */
    u8 pad28[0x44 - 0x28];          /* +0x028 .. +0x043, unknown */
    s32 unk44;                        /* +0x044 */
    u8 pad48[0x54 - 0x48];               /* +0x048 .. +0x053, unknown */
    void *kind;                            /* +0x054, the ctor's own arg1, stashed verbatim */
    Block24 params;                          /* +0x058, func_800564A4's own 0x24-byte block-copy target */
};

/* The shared base-class table getter, SAME symbol `class_3bb8c_o.c`
 * already established as `DreamSys__GetBaseMethods` there (also MEASURED to take no
 * real arguments). Fresh local reading here: this unit needs both `ctor`
 * (+0x008, checked against NULL) and `dtor` (+0x00C, its return value
 * forwarded by func_80056464). */
typedef struct FixedBaseTableR {
    u8 pad0[0x8];
    void *(*ctor)(void *self); /* +0x008 */
    void *(*dtor)(void *self);   /* +0x00C */
} FixedBaseTableR;
extern FixedBaseTableR *DreamSys__GetBaseMethods(void);

extern void *func_80017B34(s32 size);
extern void *func_80017CFC(void *ptr);
extern Obj876FCMethods *func_80056F4C(void); /* class_3bb8c_o.c, round 17, ALREADY MATCHED -- returns &D_800876FC */
extern void Class876FC__ReleaseByKind(Obj876FC *self);
extern void *Class876FC__InitByKind(Obj876FC *self, void *arg1, void *arg2);

void *func_80056320(void *arg0, void *arg1, void *arg2, void *arg3) {
    Obj876FC *self = func_80017B34(0x98);

    if (self != NULL) {
        if (func_80056F4C()->ctor(self, arg0, arg1, arg2, arg3) != NULL) {
            return self;
        }
        func_80017CFC(self);
        return NULL;
    }
    return NULL;
}

void *func_800563C0(Obj876FC *self, void *arg1, void *arg2, void *arg3, void *arg4) {
    if (DreamSys__GetBaseMethods()->ctor(self) == NULL) {
        goto fail;
    }
    self->methods = func_80056F4C();
    self->unk44 = 0;
    self->kind = arg1;
    self->methods->slot40(self, arg2);
    Class876FC__InitByKind(self, arg3, arg4);
    return self;
fail:
    return NULL;
}

void *func_80056464(Obj876FC *self) {
    Class876FC__ReleaseByKind(self);
    return DreamSys__GetBaseMethods()->dtor(self);
}

void func_800564A4(Obj876FC *self, Block24 *src) {
    self->params = *src;
    self->tick = 0;
}

extern void Class876FC__UpdateByKind(Obj876FC *self); /* arity-ok: the definition is 2-parameter and the callee DOES read $a1 (`move s1,a1` at 0x80056650), but func_800564F4 passes nothing for it -- retail's jal at 0x80056508 has `sw v0,36(a0)` in the delay slot and leaves its own incoming $a1 in place */

void func_800564F4(Obj876FC *self) {
    self->tick = self->tick + 1;
    Class876FC__UpdateByKind(self);
}
