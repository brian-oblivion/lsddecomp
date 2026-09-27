/*
 * class_3bb8c_r -- 0x46288..0x46D20 (vram 0x80055A88..0x80056520), the tail
 * of the `class_3bb8c_n` remainder (carved round 17). All 21 functions are
 * MATCHED. Two unrelated classes share the slice, cut at ROM addresses
 * rather than at a class boundary (tools/classtable.py, round 17):
 *
 *  - StyleCue00..StyleCue13, the complete 14-slot table `gStyleCueCallbacks`,
 *    and their helper ComputeStyleCueFalloff. They are SoundCueSet
 *    callbacks (include/SoundCueSet.h): TryStartStyleCue (class_3bb8c_n.c)
 *    starts a style-cue slot's embedded set with the claimed record's cue
 *    index as the tag and that row of the table as the callback, as
 *    Entity does with gEntityMoodHandlerTable's MoodCueNN handlers. Each
 *    tick a callback sets the set's attenuation from the slot's distance
 *    (ComputeStyleCueFalloff) and, on the ticks its pattern selects,
 *    requests programs on the three voices; most restart the pattern by setting
 *    `tick` to -1 once it passes a limit.
 *  - StyleEffect (include/StyleEffect.h), the Actor subclass the style
 *    layer keeps at an offset from its target: this unit supplies its slot
 *    occupants (StyleEffect__StyleEffect/__Finalize/__SetParams/__Update)
 *    and the `New_StyleEffect` allocator; its per-kind work is in
 *    class_3bb8c_s.c and class_3bb8c_o.c.
 *
 * Named round 73 (charlie); tiers and evidence in each function's match
 * report.
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

/* ComputeStyleCueFalloff's own ROM address (0x8005627C) is AFTER all 14 slot
 * occupants below (it sits right before the blocked IsStyleVariantEven), so
 * its definition lives in that position further down this file to keep
 * strict ROM-address order -- forward-declared here since every occupant
 * calls it. */
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
        set->slots[0].program = 0x18;
        set->slots[0].octave = -2;
    } else if (tick >= 0x401) {
        set->tick = -1;
    }
}

void StyleCue02(StyleCueParam *ctx, SoundCueSet *set) {
    s32 tick;

    set->attenuation = ComputeStyleCueFalloff(ctx);
    tick = set->tick;
    if (tick == 0) {
        set->slots[0].program = 0xC;
        set->slots[0].octave = 2;
    } else if (tick >= 5) {
        set->tick = -1;
    }
}

void StyleCue03(StyleCueParam *ctx, SoundCueSet *set) {
    set->attenuation = ComputeStyleCueFalloff(ctx);
    if (set->tick % 20 == 0) {
        set->slots[0].program = 0x1E;
        set->slots[0].vol = 0x20;
        set->slots[0].octave = 0;
        set->slots[0].endVol = 0xA;
    }
    if (set->tick % 400 == 0) {
        set->slots[1].program = 0x1E;
        set->slots[1].octave = 0;
    }
    set->slots[2].program = 6;
    set->slots[2].vol = 0x20;
    set->slots[2].octave = 0;
    set->slots[2].endVol = 0xA;
}

void StyleCue04(StyleCueParam *ctx, SoundCueSet *set) {
    set->attenuation = ComputeStyleCueFalloff(ctx);
    if (set->tick % 3 == 0) {
        set->slots[0].program = 0x1E;
        set->slots[0].octave = 0;
    }
    if (set->tick % 5 == 0) {
        set->slots[1].program = 0x1E;
        set->slots[1].octave = 0;
        set->slots[1].vol = 0x18;
        set->slots[1].endVol = 0x18;
    }
    if (set->tick % 7 == 0) {
        set->slots[0].program = 0x1E;
        set->slots[0].octave = 0;
    }
    set->slots[2].program = 6;
    set->slots[2].octave = 1;
    set->slots[2].vol = 0x2A;
    set->slots[2].endVol = 0xA;
}

void StyleCue05(StyleCueParam *ctx, SoundCueSet *set) {
    set->attenuation = ComputeStyleCueFalloff(ctx);
    if (set->tick == 0) {
        set->slots[0].program = 0x1E;
        set->slots[0].octave = -1;
    } else if (set->tick < 0x32 && set->tick % 5 == 4) {
        set->slots[1].program = 0x1E;
        set->slots[1].octave = 0;
        set->slots[1].vol = set->slots[1].vol - set->tick * 2;
        set->slots[1].endVol = set->slots[1].vol;
    } else if ((u32)(set->tick - 0x65) < 9) {
        set->slots[2].program = 0xD;
        set->slots[2].octave = 1;
    } else if (set->tick >= 0xC9) {
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
    } else if (tick >= 0x1B) {
        set->tick = -1;
    }
}

void StyleCue07(StyleCueParam *ctx, SoundCueSet *set) {
    s32 tick;

    set->attenuation = ComputeStyleCueFalloff(ctx);
    tick = set->tick;
    if (tick == 0) {
        set->slots[0].program = 0x14;
        set->slots[0].octave = 1;
    } else if (tick == 3) {
        set->slots[0].octave = 2;
        set->slots[0].vol = 0x18;
        set->slots[0].program = tick;
        set->slots[0].endVol = 0x14;
    } else if (tick >= 0x33) {
        set->tick = -1;
    }
}

void StyleCue08(StyleCueParam *ctx, SoundCueSet *set) {
    set->attenuation = ComputeStyleCueFalloff(ctx);
    if (set->tick % 20 == 0) {
        set->slots[0].program = 9;
        set->slots[0].octave = 0;
        set->slots[0].vol = 0x40;
        set->slots[0].endVol = 0x40;
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
        set->slots[2].program = 0x14;
        set->slots[2].octave = 1;
    } else if ((u32)(rem - 54) < 5) {
        set->slots[2].program = 0xD;
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
        set->slots[0].program = 0x14;
        set->slots[0].octave = -2;
        set->slots[1].program = 0x14;
        set->slots[1].octave = -2;
    } else if (tick == 4) {
        set->slots[1].program = 0x14;
        set->slots[1].octave = -2;
    } else if (tick == 0x14) {
        set->slots[0].program = 0x10;
        set->slots[0].octave = -2;
        set->slots[1].program = 0x12;
        set->slots[1].octave = -2;
    } else if (tick >= 0xC9) {
        set->tick = -1;
    }
}

void StyleCue13(StyleCueParam *ctx, SoundCueSet *set) {
    set->attenuation = ComputeStyleCueFalloff(ctx);
    if (set->tick == 0) {
        set->slots[0].program = 0x18;
        set->slots[0].octave = 0;
    }
}

/* A 15-entry table indexed with the NEGATIVE of `ctx->entry->countSign`
 * (`gStyleCueDistanceTable - tag*4`, i.e. `gStyleCueDistanceTable[-tag]` for `tag` in [-14, 0]).
 * `asm/data/76DC8.data.s` confirms exactly 15 words at this address. */
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
    StyleEffect *self = BMemPMgrAlloc(0x98);

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

/* The base finalize is SceneNode__Finalize, which returns nothing: the old
 * view's `return base->dtor(self)` forwarded a $v0 no one sets. */
void StyleEffect__Finalize(StyleEffect *self) {
    StyleEffect__ReleaseByKind(self);
    GetActorMethods()->finalize((Actor *)self);
}

void StyleEffect__SetParams(StyleEffect *self, StyleEffectParams *params) {
    self->params = *params;
    self->tick = 0;
}

/* `pos` arrives from StyleUpdateEffectSlots and is forwarded untouched in
 * $a1 (retail's jal at 0x80056508 sets no $a1). */
void StyleEffect__Update(StyleEffect *self, LongVec3 *pos) {
    self->tick = self->tick + 1;
    StyleEffect__UpdateByKind(self, pos);
}
