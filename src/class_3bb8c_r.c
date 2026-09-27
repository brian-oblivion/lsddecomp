/*
 * class_3bb8c_r -- the style layer's sound cues and StyleEffect's lifecycle.
 * Two unrelated groups share this file, with one predicate between them:
 *
 *  - StyleCue00..StyleCue13, the 14 rows of gStyleCueCallbacks, and their
 *    helper ComputeStyleCueFalloff. Each is a SoundCueSet callback
 *    (include/SoundCueSet.h): TryStartStyleCue (class_3bb8c_n.c) starts a
 *    style-cue slot's embedded set with the claimed cue record's index as the
 *    tag and that row of the table as the callback, as Entity does with its
 *    Entity__MoodCueNN handlers. Every tick a callback sets the set's
 *    attenuation from the slot's distance to the target
 *    (ComputeStyleCueFalloff) and, on the ticks its pattern selects,
 *    requests VAB programs on the three voices; most restart the pattern by
 *    setting `tick` to -1 once it passes a limit.
 *  - IsStyleVariantEven: whether the variant PickStyleFallbackConfig chose
 *    (gStyleVariant, class_3bb8c_n.c) is even.
 *  - StyleEffect (include/StyleEffect.h), the Actor subclass the style layer
 *    keeps at an offset from its target: its ctor, finalize, reset
 *    (StyleEffect__SetParams) and update slot occupants, and the
 *    New_StyleEffect allocator. The per-kind work is in class_3bb8c_s.c and
 *    class_3bb8c_o.c.
 */
#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "Actor.h"
#include "StyleEffect.h"
#include "SoundCueSet.h"

/* ------------------------------------------------------------------ *
 * gStyleCueCallbacks's 14 slots (StyleCue00..StyleCue13) plus the shared
 * helper ComputeStyleCueFalloff they all call first.
 * ------------------------------------------------------------------ */

/* The owner every StyleCueNN callback receives: one of class_3bb8c_n.c's
 * style-cue slots (its `StyleCueSlot`, of which this is a local view).
 * TryStartStyleCue passes the slot as InitSoundCueSet's owner and its
 * embedded `cueSet` as the set, so a callback's `set` is `&ctx->cueSet`. */
typedef struct StyleCueParam StyleCueParam;

/* The cue-table record the slot claimed (class_3bb8c_n.c's
 * `StyleCueEntryView`). */
typedef struct StyleCueParamMethods {
    u8 pad0[0x6];
    s8 countSign; /* +0x006, the record's cue index (its gStyleCueCallbacks
                   * row and InitSoundCueSet tag), negated while a slot has
                   * it claimed; ComputeStyleCueFalloff indexes
                   * gStyleCueDistanceTable with its negative. */
} StyleCueParamMethods;

struct StyleCueParam {
    StyleCueParamMethods *entry; /* +0x000 */
    u8 pad4[0x10 - 0x4];
    s32 lastDist;       /* +0x010, IsStyleCueNear's distance to the target */
    SoundCueSet cueSet; /* +0x014 */
};

/* Every callback calls this first; it is defined after them, in ROM order. */
s32 ComputeStyleCueFalloff(StyleCueParam *ctx);

void StyleCue00(StyleCueParam *ctx, SoundCueSet *set) {
    s32 tick;

    set->attenuation = ComputeStyleCueFalloff(ctx);
    tick = set->tick;
    if (tick == 0) {
        set->slots[0].program = 7;
        set->slots[0].octave = 0;
    } else if (tick == 2) {
        set->slots[1].program = 7;
        set->slots[1].octave = 0;
    } else if (tick == 5) {
        set->slots[2].program = 7;
        set->slots[2].octave = 0;
    } else if (tick >= 8) {
        set->tick = -1;
    }
}

void StyleCue01(StyleCueParam *ctx, SoundCueSet *set) {
    s32 tick;

    set->attenuation = ComputeStyleCueFalloff(ctx);
    tick = set->tick;
    if (tick == 0) {
        set->slots[0].program = 24;
        set->slots[0].octave = -2;
    } else if (tick >= 1025) {
        set->tick = -1;
    }
}

void StyleCue02(StyleCueParam *ctx, SoundCueSet *set) {
    s32 tick;

    set->attenuation = ComputeStyleCueFalloff(ctx);
    tick = set->tick;
    if (tick == 0) {
        set->slots[0].program = 12;
        set->slots[0].octave = 2;
    } else if (tick >= 5) {
        set->tick = -1;
    }
}

void StyleCue03(StyleCueParam *ctx, SoundCueSet *set) {
    set->attenuation = ComputeStyleCueFalloff(ctx);
    if (set->tick % 20 == 0) {
        set->slots[0].program = 30;
        set->slots[0].vol = 32;
        set->slots[0].octave = 0;
        set->slots[0].endVol = 10;
    }
    if (set->tick % 400 == 0) {
        set->slots[1].program = 30;
        set->slots[1].octave = 0;
    }
    set->slots[2].program = 6;
    set->slots[2].vol = 32;
    set->slots[2].octave = 0;
    set->slots[2].endVol = 10;
}

void StyleCue04(StyleCueParam *ctx, SoundCueSet *set) {
    set->attenuation = ComputeStyleCueFalloff(ctx);
    if (set->tick % 3 == 0) {
        set->slots[0].program = 30;
        set->slots[0].octave = 0;
    }
    if (set->tick % 5 == 0) {
        set->slots[1].program = 30;
        set->slots[1].octave = 0;
        set->slots[1].vol = 24;
        set->slots[1].endVol = 24;
    }
    if (set->tick % 7 == 0) {
        set->slots[0].program = 30;
        set->slots[0].octave = 0;
    }
    set->slots[2].program = 6;
    set->slots[2].octave = 1;
    set->slots[2].vol = 42;
    set->slots[2].endVol = 10;
}

void StyleCue05(StyleCueParam *ctx, SoundCueSet *set) {
    set->attenuation = ComputeStyleCueFalloff(ctx);
    if (set->tick == 0) {
        set->slots[0].program = 30;
        set->slots[0].octave = -1;
    } else if (set->tick < 50 && set->tick % 5 == 4) {
        set->slots[1].program = 30;
        set->slots[1].octave = 0;
        set->slots[1].vol = set->slots[1].vol - set->tick * 2;
        set->slots[1].endVol = set->slots[1].vol;
    } else if (set->tick >= 101 && set->tick < 110) {
        set->slots[2].program = 13;
        set->slots[2].octave = 1;
    } else if (set->tick >= 201) {
        set->tick = -1;
    }
}

void StyleCue06(StyleCueParam *ctx, SoundCueSet *set) {
    s32 tick;

    set->attenuation = ComputeStyleCueFalloff(ctx);
    tick = set->tick;
    if (tick == 0) {
        set->slots[0].program = 7;
        set->slots[0].octave = 2;
    } else if (tick >= 27) {
        set->tick = -1;
    }
}

void StyleCue07(StyleCueParam *ctx, SoundCueSet *set) {
    s32 tick;

    set->attenuation = ComputeStyleCueFalloff(ctx);
    tick = set->tick;
    if (tick == 0) {
        set->slots[0].program = 20;
        set->slots[0].octave = 1;
    } else if (tick == 3) {
        set->slots[0].octave = 2;
        set->slots[0].vol = 24;
        set->slots[0].program = 3;
        set->slots[0].endVol = 20;
    } else if (tick >= 51) {
        set->tick = -1;
    }
}

void StyleCue08(StyleCueParam *ctx, SoundCueSet *set) {
    set->attenuation = ComputeStyleCueFalloff(ctx);
    if (set->tick % 20 == 0) {
        set->slots[0].program = 9;
        set->slots[0].octave = 0;
        set->slots[0].vol = 64;
        set->slots[0].endVol = 64;
    }
}

void StyleCue09(StyleCueParam *ctx, SoundCueSet *set) {
    set->attenuation = ComputeStyleCueFalloff(ctx);
    if (set->tick % 20 == 0) {
        set->slots[0].program = 9;
        set->slots[0].octave = -2;
    }
}

void StyleCue10(StyleCueParam *ctx, SoundCueSet *set) {
    s32 rem;

    set->attenuation = ComputeStyleCueFalloff(ctx);
    rem = set->tick % 20;
    if (rem == 1) {
        set->slots[0].program = 9;
        set->slots[0].octave = -2;
    } else if (rem == 16) {
        set->slots[1].program = 9;
        set->slots[1].octave = -2;
    }
}

void StyleCue11(StyleCueParam *ctx, SoundCueSet *set) {
    s32 rem;

    StyleCue10(ctx, set);
    rem = set->tick % 70;
    if (rem == 50) {
        set->slots[2].program = 20;
        set->slots[2].octave = 1;
    } else if (rem >= 54 && rem < 59) {
        set->slots[2].program = 13;
        set->slots[2].octave = 1;
    } else if (rem == 61) {
        set->slots[2].program = 9;
        set->slots[2].octave = -1;
    }
}

void StyleCue12(StyleCueParam *ctx, SoundCueSet *set) {
    s32 tick;

    set->attenuation = ComputeStyleCueFalloff(ctx);
    tick = set->tick;
    if (tick == 0) {
        set->slots[0].program = 20;
        set->slots[0].octave = -2;
        set->slots[1].program = 20;
        set->slots[1].octave = -2;
    } else if (tick == 4) {
        set->slots[1].program = 20;
        set->slots[1].octave = -2;
    } else if (tick == 20) {
        set->slots[0].program = 16;
        set->slots[0].octave = -2;
        set->slots[1].program = 18;
        set->slots[1].octave = -2;
    } else if (tick >= 201) {
        set->tick = -1;
    }
}

void StyleCue13(StyleCueParam *ctx, SoundCueSet *set) {
    set->attenuation = ComputeStyleCueFalloff(ctx);
    if (set->tick == 0) {
        set->slots[0].program = 24;
        set->slots[0].octave = 0;
    }
}

/* One range per cue record (15), indexed by the record's cue index: the
 * negative of `countSign` while a slot has the record claimed. IsStyleCueNear
 * tests the slot's distance against the same row. */
extern s32 gStyleCueDistanceTable[];

s32 ComputeStyleCueFalloff(StyleCueParam *ctx) {
    s32 range = gStyleCueDistanceTable[-ctx->entry->countSign];
    s32 stepDist = range / ctx->cueSet.attenuationSteps;

    return ctx->lastDist / stepDist;
}

extern s32 gStyleVariant;

s32 IsStyleVariantEven(void) {
    return (gStyleVariant & 1) ^ 1;
}

/* ------------------------------------------------------------------ *
 * StyleEffect's slot occupants (ctor, finalize, reset = SetParams, +0x0EC =
 * Update), plus its `New_` allocator. The class: include/StyleEffect.h.
 * ------------------------------------------------------------------ */

extern void *BMemPMgrAlloc(s32 size);
extern void *BMemPMgrFree(void *ptr);

StyleEffect *New_StyleEffect(s32 kind, StyleEffectParams *params, SceneNode *parent, LongVec3 *pos) {
    StyleEffect *self = BMemPMgrAlloc(sizeof(StyleEffect));

    if (self != NULL) {
        if (GetStyleEffectMethods()->ctor(self, kind, params, parent, pos) != NULL) {
            return self;
        }
        BMemPMgrFree(self);
        return NULL;
    }
    return NULL;
}

/* `kind` goes into Actor's pendingExtra (+0x054): see include/StyleEffect.h. */
StyleEffect *StyleEffect__StyleEffect(StyleEffect *self, s32 kind, StyleEffectParams *params,
                                      SceneNode *parent, LongVec3 *pos) {
    if (GetActorMethods()->ctor((Actor *)self) == NULL) {
        goto fail;
    }
    self->methods = GetStyleEffectMethods();
    self->state = 0;
    self->pendingExtra = kind;
    ((StyleEffectSetParamsFn)self->methods->reset)(self, params);
    StyleEffect__InitByKind(self, parent, pos);
    return self;
fail:
    return NULL;
}

/* Actor's finalize (SceneNode__Finalize) returns nothing, so neither does this. */
void StyleEffect__Finalize(StyleEffect *self) {
    StyleEffect__ReleaseByKind(self);
    GetActorMethods()->finalize((Actor *)self);
}

void StyleEffect__SetParams(StyleEffect *self, StyleEffectParams *params) {
    self->params = *params;
    self->tick = 0;
}

/* `pos` arrives from StyleUpdateEffectSlots and is forwarded untouched. */
void StyleEffect__Update(StyleEffect *self, LongVec3 *pos) {
    self->tick = self->tick + 1;
    StyleEffect__UpdateByKind(self, pos);
}
