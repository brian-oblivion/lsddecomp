/*
 * class_3bb8c_r -- 0x46288..0x46D20 (vram 0x80055A88..0x80056520), the tail
 * of the `class_3bb8c_n` remainder (carved round 17). All 21 functions are
 * MATCHED. Two unrelated classes share the slice, cut at ROM addresses
 * rather than at a class boundary (tools/classtable.py, round 17):
 *
 *  - StyleCue00..StyleCue13, the complete 14-slot table `gStyleCueCallbacks`.
 *    TryStartStyleCue (class_3bb8c_n.c) installs each occupant as
 *    SoundCueSet::callback via InitSoundCueSet (code_179d8_e.c) -- the same
 *    per-tag sound-cue-callback mechanism `gEntityMoodHandlerTable`'s
 *    MoodCueNN occupants use for Entity. Each occupant dispatches on
 *    `self->kind` (its own tag) to fill in a handful of numeric fields, and
 *    all 14 call the shared helper `ComputeStyleCueFalloff` first. Local
 *    view `StyleCueParam`/`StyleCueParamMethods`; tier B throughout --
 *    the callback mechanism is established, which specific numeric fields
 *    mean in the running game is not.
 *  - Class876FC (include/Class876FC.h, unified round 88), an Actor
 *    subclass (id 0xEF34, parent 0x34): this unit supplies its slot
 *    occupants (Class876FC__Class876FC/__Finalize/__SetParams/__Update)
 *    and the `New_Class876FC` allocator; its private helpers are in
 *    class_3bb8c_s.c and class_3bb8c_o.c.
 *
 * Named round 73 (charlie); tiers and evidence in each function's match
 * report.
 */
#include "common.h"
#include "Actor.h"
#include "Class876FC.h"

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
    s32 kind;                      /* +0x004, a "kind" selector the 14 slot
                               * occupants below all dispatch on */
    u8 pad8[0x10 - 0x8];           /* +0x008 .. +0x00F, unknown */
    s32 falloff;                   /* +0x010, ComputeStyleCueFalloff's own result */
    u8 pad14[0x1C - 0x14];         /* +0x014 .. +0x01B, unknown */
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
 * Class876FC's slot occupants (ctor, finalize, reset = SetParams, +0x0EC =
 * Update), plus its `New_` allocator. The class: include/Class876FC.h.
 * ------------------------------------------------------------------ */

extern void *BMemPMgrAlloc(s32 size);
extern void *BMemPMgrFree(void *ptr);

Class876FC *New_Class876FC(s32 kind, Class876FCParams *params, SceneNode *parent, LongVec3 *pos) {
    Class876FC *self = BMemPMgrAlloc(0x98);

    if (self != NULL) {
        if (GetClass876FCMethods()->ctor(self, kind, params, parent, pos) != NULL) {
            return self;
        }
        BMemPMgrFree(self);
        return NULL;
    }
    return NULL;
}

/* `kind` goes into Actor's pendingExtra (+0x054): see include/Class876FC.h. */
Class876FC *Class876FC__Class876FC(Class876FC *self, s32 kind, Class876FCParams *params,
                                   SceneNode *parent, LongVec3 *pos) {
    if (GetActorMethods()->ctor((Actor *)self) == NULL) {
        goto fail;
    }
    self->methods = GetClass876FCMethods();
    self->state = 0;
    self->pendingExtra = kind;
    ((Class876FCSetParamsFn)self->methods->reset)(self, params);
    Class876FC__InitByKind(self, parent, pos);
    return self;
fail:
    return NULL;
}

/* The base finalize is SceneNode__Finalize, which returns nothing: the old
 * view's `return base->dtor(self)` forwarded a $v0 no one sets. */
void Class876FC__Finalize(Class876FC *self) {
    Class876FC__ReleaseByKind(self);
    GetActorMethods()->finalize((Actor *)self);
}

void Class876FC__SetParams(Class876FC *self, Class876FCParams *params) {
    self->params = *params;
    self->tick = 0;
}

/* `pos` arrives from StyleUpdateEffectSlots and is forwarded untouched in
 * $a1 (retail's jal at 0x80056508 sets no $a1). */
void Class876FC__Update(Class876FC *self, LongVec3 *pos) {
    self->tick = self->tick + 1;
    Class876FC__UpdateByKind(self, pos);
}
