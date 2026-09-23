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
 * IsStyleVariantEven: MATCHED round 44 (was filed BLOCKED/gp_rel; reopened and
 * closed 5/5 on the first build after the fix -- see
 * docs/match-reports/IsStyleVariantEven.md).
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
 *  - StyleCue00 .. StyleCue13 (14 functions) are the complete slot
 *    list of `gStyleCueCallbacks` (14 slots, header word 0). Despite matching
 *    BasicClassMethods' slot COUNT, these bodies are NOT
 *    add/removeChild-shaped -- every one calls the shared helper
 *    `ComputeStyleCueFalloff(ctx)` then dispatches on `target->kind` to
 *    fill in a handful of numeric fields. Kept as its own local view,
 *    `StyleCueParam`/`StyleCueParamMethods` -- nothing here justifies asserting a
 *    BasicClass relationship just because the slot count coincides.
 *  - Class876FC__Class876FC (ctor) / Class876FC__Finalize (dtor) / Class876FC__SetParams (setParams)
 *    / Class876FC__Update are occupants of `gClass876FCMethods` -- the SAME sibling
 *    table `class_3bb8c_o.c` (round 17, previous pass, already merged)
 *    partly resolved from the OTHER side (its own `GetClass876FCMethods` returns
 *    `&gClass876FCMethods`, and its shared-base slots +0x010/+0x014/+0x018/+0x088/
 *    etc. are INHERITED, not overridden, by this class). This unit
 *    supplies the class's OWN slots (ctor/dtor/setParams), confirmed by both
 *    chaining to `DreamSys__GetBaseMethods()` -- the SAME shared-base getter
 *    `class_3bb8c_o.c` already used for its own ctor/New_X pair. Kept as
 *    this unit's own local view, `Class876FC`/`Class876FCMethods` --
 *    `class_3bb8c_o.c` is not this unit's to edit, and per the
 *    multiple-independent-local-views convention there is no reason a
 *    fresh view here should match its field names field-for-field.
 *  - New_Class876FC is a plain `New_X` allocator (0x98 bytes) for the
 *    `gClass876FCMethods` class, dispatching through `GetClass876FCMethods()->ctor`
 *    (cross-unit call into the ALREADY-MATCHED `class_3bb8c_o.c` symbol)
 *    rather than calling `Class876FC__Class876FC` by name.
 *  - ComputeStyleCueFalloff is the shared helper every `gStyleCueCallbacks` occupant calls
 *    first (and `StyleCue11` reaches transitively, via a plain call to
 *    `StyleCue10`): reads a small tag byte off `ctx->methods`, looks it
 *    up (with a NEGATIVE index) into a global table, and returns a
 *    chained division result.
 */
#include "common.h"

/* ------------------------------------------------------------------ *
 * gStyleCueCallbacks's 14 slots (StyleCue00..StyleCue13) plus the shared
 * helper ComputeStyleCueFalloff they all call first.
 * ------------------------------------------------------------------ */

/* StyleCueParam is used for BOTH parameters of every StyleCueNN occupant
 * and of ComputeStyleCueFalloff -- `ctx` (a per-tag config carrying `methods`
 * and `falloff`'s own inputs) and `self` (the live instance whose `kind` and
 * numeric fields below get set). InitSoundCueSet/TryStartStyleCue
 * (class_3bb8c_n.c) install gStyleCueCallbacks' occupants as
 * SoundCueSet::callback (code_179d8_e.c's InitSoundCueSet), the same slot
 * gEntityMoodHandlerTable's MoodCueNN handlers occupy for Entity -- so this
 * is very likely a SoundCueSet-shaped object under a different unit's local
 * view; `falloff`/pad14's offsets line up with SoundCueSet's own
 * unk4/unk14 (code_179d8_e.c), but nothing here confirms `self` and `ctx`
 * are the SAME concrete object, so both stay under one local type per this
 * project's multiple-independent-local-views convention rather than being
 * asserted identical to SoundCueSet. */
typedef struct StyleCueParam StyleCueParam;
typedef struct StyleCueParamMethods {
    u8 pad0[0x6];
    s8 tag; /* +0x006, a small type id -- read signed, used as a NEGATIVE
             * index into gStyleCueDistanceTable (see ComputeStyleCueFalloff). */
} StyleCueParamMethods;
struct StyleCueParam {
    StyleCueParamMethods *methods; /* +0x000 */
    s32 kind;                /* +0x004, a "kind" selector the 14 slot
                               * occupants below all dispatch on */
    u8 pad8[0x10 - 0x8];        /* +0x008 .. +0x00F, unknown */
    s32 falloff;                    /* +0x010, ComputeStyleCueFalloff's own result */
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

/* ComputeStyleCueFalloff's own ROM address (0x8005627C) is AFTER all 14 slot
 * occupants below (it sits right before the blocked IsStyleVariantEven), so
 * its definition lives in that position further down this file to keep
 * strict ROM-address order -- forward-declared here since every occupant
 * calls it. */
s32 ComputeStyleCueFalloff(StyleCueParam *ctx);

void StyleCue00(StyleCueParam *ctx, StyleCueParam *self) {
    s32 kind;

    self->falloff = ComputeStyleCueFalloff(ctx);
    kind = self->kind;
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
        self->kind = -1;
    }
}

void StyleCue01(StyleCueParam *ctx, StyleCueParam *self) {
    s32 kind;

    self->falloff = ComputeStyleCueFalloff(ctx);
    kind = self->kind;
    if (kind == 0) {
        self->unk1C = 0x18;
        self->unk20 = -2;
    } else if (kind >= 0x401) {
        self->kind = -1;
    }
}

void StyleCue02(StyleCueParam *ctx, StyleCueParam *self) {
    s32 kind;

    self->falloff = ComputeStyleCueFalloff(ctx);
    kind = self->kind;
    if (kind == 0) {
        self->unk1C = 0xC;
        self->unk20 = 2;
    } else if (kind >= 5) {
        self->kind = -1;
    }
}

void StyleCue03(StyleCueParam *ctx, StyleCueParam *self) {
    self->falloff = ComputeStyleCueFalloff(ctx);
    if (self->kind % 20 == 0) {
        self->unk1C = 0x1E;
        self->unk24 = 0x20;
        self->unk20 = 0;
        self->unk28 = 0xA;
    }
    if (self->kind % 400 == 0) {
        self->unk30 = 0x1E;
        self->unk34 = 0;
    }
    self->unk44 = 6;
    self->unk4C = 0x20;
    self->unk48 = 0;
    self->unk50 = 0xA;
}

void StyleCue04(StyleCueParam *ctx, StyleCueParam *self) {
    self->falloff = ComputeStyleCueFalloff(ctx);
    if (self->kind % 3 == 0) {
        self->unk1C = 0x1E;
        self->unk20 = 0;
    }
    if (self->kind % 5 == 0) {
        self->unk30 = 0x1E;
        self->unk34 = 0;
        self->unk38 = 0x18;
        self->unk3C = 0x18;
    }
    if (self->kind % 7 == 0) {
        self->unk1C = 0x1E;
        self->unk20 = 0;
    }
    self->unk44 = 6;
    self->unk48 = 1;
    self->unk4C = 0x2A;
    self->unk50 = 0xA;
}

void StyleCue05(StyleCueParam *ctx, StyleCueParam *self) {
    self->falloff = ComputeStyleCueFalloff(ctx);
    if (self->kind == 0) {
        self->unk1C = 0x1E;
        self->unk20 = -1;
    } else if (self->kind < 0x32 && self->kind % 5 == 4) {
        self->unk30 = 0x1E;
        self->unk34 = 0;
        self->unk38 = self->unk38 - self->kind * 2;
        self->unk3C = self->unk38;
    } else if ((u32)(self->kind - 0x65) < 9) {
        self->unk44 = 0xD;
        self->unk48 = 1;
    } else if (self->kind >= 0xC9) {
        self->kind = -1;
    }
}

void StyleCue06(StyleCueParam *ctx, StyleCueParam *self) {
    s32 kind;

    self->falloff = ComputeStyleCueFalloff(ctx);
    kind = self->kind;
    if (kind == 0) {
        self->unk1C = 7;
        self->unk20 = 2;
    } else if (kind >= 0x1B) {
        self->kind = -1;
    }
}

void StyleCue07(StyleCueParam *ctx, StyleCueParam *self) {
    s32 kind;

    self->falloff = ComputeStyleCueFalloff(ctx);
    kind = self->kind;
    if (kind == 0) {
        self->unk1C = 0x14;
        self->unk20 = 1;
    } else if (kind == 3) {
        self->unk20 = 2;
        self->unk24 = 0x18;
        self->unk1C = kind;
        self->unk28 = 0x14;
    } else if (kind >= 0x33) {
        self->kind = -1;
    }
}

void StyleCue08(StyleCueParam *ctx, StyleCueParam *self) {
    self->falloff = ComputeStyleCueFalloff(ctx);
    if (self->kind % 20 == 0) {
        self->unk1C = 9;
        self->unk20 = 0;
        self->unk24 = 0x40;
        self->unk28 = 0x40;
    }
}

void StyleCue09(StyleCueParam *ctx, StyleCueParam *self) {
    self->falloff = ComputeStyleCueFalloff(ctx);
    if (self->kind % 20 == 0) {
        self->unk1C = 9;
        self->unk20 = -2;
    }
}

void StyleCue10(StyleCueParam *ctx, StyleCueParam *self) {
    s32 rem;

    self->falloff = ComputeStyleCueFalloff(ctx);
    rem = self->kind % 20;
    if (rem == 1) {
        self->unk1C = 9;
        self->unk20 = -2;
    } else if (rem == 16) {
        self->unk30 = 9;
        self->unk34 = -2;
    }
}

void StyleCue11(StyleCueParam *ctx, StyleCueParam *self) {
    s32 rem;

    StyleCue10(ctx, self);
    rem = self->kind % 70;
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

void StyleCue12(StyleCueParam *ctx, StyleCueParam *self) {
    s32 kind;

    self->falloff = ComputeStyleCueFalloff(ctx);
    kind = self->kind;
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
        self->kind = -1;
    }
}

void StyleCue13(StyleCueParam *ctx, StyleCueParam *self) {
    self->falloff = ComputeStyleCueFalloff(ctx);
    if (self->kind == 0) {
        self->unk1C = 0x18;
        self->unk20 = 0;
    }
}

/* A 15-entry table indexed with the NEGATIVE of `ctx->methods->tag`
 * (`gStyleCueDistanceTable - tag*4`, i.e. `gStyleCueDistanceTable[-tag]` for `tag` in [-14, 0]).
 * `asm/data/76DC8.data.s` confirms exactly 15 words at this address. */
extern s32 gStyleCueDistanceTable[];

s32 ComputeStyleCueFalloff(StyleCueParam *ctx) {
    s32 t = gStyleCueDistanceTable[-ctx->methods->tag];
    s32 q = t / ctx->unk28;

    return ctx->falloff / q;
}

extern s32 gStyleVariant;

s32 IsStyleVariantEven(void) {
    return (gStyleVariant & 1) ^ 1;
}

/* ------------------------------------------------------------------ *
 * gClass876FCMethods's own slots (ctor/dtor/setParams), plus its `New_X` allocator.
 * See the file banner: this is the SAME sibling table `class_3bb8c_o.c`
 * (round 17, previous pass) already partly resolved; this unit's own
 * local view is kept independent, per the multiple-independent-views
 * convention -- `class_3bb8c_o.c` is not this unit's to edit.
 * ------------------------------------------------------------------ */

typedef struct Class876FC Class876FC;
typedef struct Class876FCMethods {
    u8 pad0[0x8];
    void *(*ctor)(Class876FC *self, void *arg1, void *arg2, void *arg3, void *arg4); /* +0x008 Class876FC__Class876FC (this unit) */
    void *(*dtor)(Class876FC *self);                                                   /* +0x00C Class876FC__Finalize (this unit) */
    u8 pad10[0x40 - 0x10];                                                              /* +0x010 .. +0x03F, shared-base slots, not this unit's to name */
    void (*setParams)(Class876FC *self, void *arg1);                                           /* +0x040 Class876FC__SetParams (this unit) */
} Class876FCMethods;

/* Declared as a WORD array, not a byte array, so a whole-struct assignment
 * reproduces retail's aligned 4-word-per-iteration block-move codegen
 * (the already-confirmed idiom, e.g. DreamSys.h's DreamSysUnk14Tail) --
 * a byte array has alignment 1 and compiles the copy as a generic
 * runtime-alignment-checked memcpy loop instead. */
typedef struct Block24 {
    s32 raw[0x24 / 4];
} Block24;

struct Class876FC {
    Class876FCMethods *methods; /* +0x000 */
    u8 pad4[0x24 - 0x4];        /* +0x004 .. +0x023, unknown */
    s32 tick;                     /* +0x024, cleared by Class876FC__SetParams */
    u8 pad28[0x44 - 0x28];          /* +0x028 .. +0x043, unknown */
    s32 unk44;                        /* +0x044 */
    u8 pad48[0x54 - 0x48];               /* +0x048 .. +0x053, unknown */
    void *kind;                            /* +0x054, the ctor's own arg1, stashed verbatim */
    Block24 params;                          /* +0x058, Class876FC__SetParams's own 0x24-byte block-copy target */
};

/* The shared base-class table getter, SAME symbol `class_3bb8c_o.c`
 * already established as `DreamSys__GetBaseMethods` there (also MEASURED to take no
 * real arguments). Fresh local reading here: this unit needs both `ctor`
 * (+0x008, checked against NULL) and `dtor` (+0x00C, its return value
 * forwarded by Class876FC__Finalize). */
typedef struct FixedBaseTableR {
    u8 pad0[0x8];
    void *(*ctor)(void *self); /* +0x008 */
    void *(*dtor)(void *self);   /* +0x00C */
} FixedBaseTableR;
extern FixedBaseTableR *DreamSys__GetBaseMethods(void);

extern void *func_80017B34(s32 size);
extern void *func_80017CFC(void *ptr);
extern Class876FCMethods *GetClass876FCMethods(void); /* class_3bb8c_o.c, round 17, ALREADY MATCHED -- returns &gClass876FCMethods */
extern void Class876FC__ReleaseByKind(Class876FC *self);
extern void *Class876FC__InitByKind(Class876FC *self, void *arg1, void *arg2);

void *New_Class876FC(void *arg0, void *arg1, void *arg2, void *arg3) {
    Class876FC *self = func_80017B34(0x98);

    if (self != NULL) {
        if (GetClass876FCMethods()->ctor(self, arg0, arg1, arg2, arg3) != NULL) {
            return self;
        }
        func_80017CFC(self);
        return NULL;
    }
    return NULL;
}

void *Class876FC__Class876FC(Class876FC *self, void *arg1, void *arg2, void *arg3, void *arg4) {
    if (DreamSys__GetBaseMethods()->ctor(self) == NULL) {
        goto fail;
    }
    self->methods = GetClass876FCMethods();
    self->unk44 = 0;
    self->kind = arg1;
    self->methods->setParams(self, arg2);
    Class876FC__InitByKind(self, arg3, arg4);
    return self;
fail:
    return NULL;
}

void *Class876FC__Finalize(Class876FC *self) {
    Class876FC__ReleaseByKind(self);
    return DreamSys__GetBaseMethods()->dtor(self);
}

void Class876FC__SetParams(Class876FC *self, Block24 *src) {
    self->params = *src;
    self->tick = 0;
}

extern void Class876FC__UpdateByKind(Class876FC *self); /* arity-ok: the definition is 2-parameter and the callee DOES read $a1 (`move s1,a1` at 0x80056650), but Class876FC__Update passes nothing for it -- retail's jal at 0x80056508 has `sw v0,36(a0)` in the delay slot and leaves its own incoming $a1 in place */

void Class876FC__Update(Class876FC *self) {
    self->tick = self->tick + 1;
    Class876FC__UpdateByKind(self);
}
