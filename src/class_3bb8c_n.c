/*
 * class_3bb8c_n -- functions 0..22 of the old 113-function class_3bb8c
 * remainder, 0x44F14..0x46288.  23 functions (19 matched, 4 STALL), 1245
 * words.  Carved round 45 (2026-09-15); staffed round 46.
 *
 * NAMING PASS, round 72 (runner alpha).  Every function, and the thirteen
 * globals its functions set up or gate on, renamed via `tools/rename.py`,
 * tree-wide.  The evidence for the `Style` prefix: this unit's global-state
 * cluster (`gStyleKind`/`gStyleCounter`/`gStyleTargetObj`/`gStyleVariant`/
 * `gStyleDecorObj`/`gStyleCueSelf`/`gStyleTickCount`, formerly
 * `D_8008AC6C`/`74`/`7C`/`80`/`94`, `D_8008AB4C`/`70`) is the SAME cluster
 * `class_3bb8c_m.c`'s already-confirmed "Style" subsystem sets
 * (`RegisterStyleConfig`/`ApplyStyleConfig`/`FillStyleFromConfig`/
 * `ApplyStyleDecorationIfSet`, round 69) -- a cross-unit fact, not a guess
 * made here.
 *
 * None of this unit's functions are themselves class methods (no vtable
 * self-dispatch on their OWN symbol); they are free functions dispatching
 * into THREE separate object families through local method-table views: a
 * decoration object (`gStyleDecorObj`, `New_ClassEAC0`-allocated), an
 * 18-slot "decor set" array (`gStyleDecorSlots`, same allocator) and an
 * `Obj876FC`-class "effect slots" array (`gStyleEffectSlots`,
 * `class_3bb8c_r.c`'s `New_Class876FC` allocator, kind-tagged 0..3 by
 * `StyleFillEffectKind0`..`3`'s literal first argument), plus a two-slot
 * positional sound-cue subsystem (`gStyleCueSlots`, `TryStartStyleCue`/
 * `FindNearestStyleCueEntry`/`FlushStyleCue`/`StopStyleCueIfNear`/
 * `IsStyleCueNear`). `TickStyle` is the per-frame entry point (called from
 * `src/class_3bb8c_l.c`); `StyleTeardown` is the scene-exit release of
 * everything `TickStyle` builds.
 *
 * What the "Style" subsystem is FOR in gameplay terms -- which dream/link
 * property `gStyleKind` actually selects -- remains UNESTABLISHED; every
 * name above describes MECHANICS, not a guessed purpose, per track 3's
 * naming rule. Full evidence and tier per function: `docs/match-reports/
 * <name>.md`, `## Naming`.
 *
 * Owns NO switch jump table (zero `jtbl_` references). All four blocker
 * constructs (`gp_rel`, `addiu_at`, `nop_mflo_mfhi`, `nop_at_expansion`) are
 * RESOLVED project-wide (CLAUDE.md, "Open toolchain blockers"); this unit's
 * four STALLs (`StyleBuildDecorSet`, `StyleFillEffectKind0`/`2`/`3`) are
 * ordinary register-identity residues, not toolchain blockers -- see their
 * own reports.
 *
 * This unit includes include/class_3bb8c.h, which eleven other units also
 * include. Whoever edits this unit's C should be the ONLY runner in the
 * class_3bb8c block that round, or price the contention with
 * `python3 tools/headercontention.py` first.
 */

#include "common.h"

/* Local view only, not the shared header: `gStyleDecorObj` is already established
 * as a `LocalM4D0Obj *` in `src/class_3bb8c_m.c` (round 15, own local type),
 * with named slots at +0x04C/+0x064/+0x068.  This function dispatches +0x004
 * instead, a slot that unit never names -- kept as its own minimal local
 * view rather than importing that unit's type (multiple-independent-local-
 * views convention; class_3bb8c_m.c is not this unit's to edit). */
typedef struct ObjAB54 ObjAB54;
typedef struct ObjAB54Methods ObjAB54Methods;
struct ObjAB54Methods {
    u8 pad0[0x4];
    void (*slot4)(ObjAB54 *self); /* +0x004 */
};
struct ObjAB54 {
    ObjAB54Methods *methods; /* +0x000 */
};

extern const u8 *gStyleDecorColor;
extern s32 gStyleDecorObj;

void StyleFlushDecoration(void) {
    if (gStyleDecorColor != 0) {
        ((ObjAB54 *) gStyleDecorObj)->methods->slot4((ObjAB54 *) gStyleDecorObj);
        gStyleDecorColor = 0;
    }
}

extern s32 gStyleCounter;
extern s32 gStyleKind;
extern s8 D_800873DC[];
extern s32 gStyleVariant;
extern s8 D_800873D8[];
extern s32 D_8008AC84;
extern s32 D_800873C8[];
extern s32 gStyleFlushColor;
extern u8 D_8008726C[];
extern u8 D_800872C4[];
extern s32 gStyleColorTable;
extern u8 D_80087234[];
extern s32 gStyleDecorVariant;

void *PickStyleFallbackConfig(void) {
    s32 sum;
    s32 kind;
    s32 divisor;
    s32 remainder;
    s8 *result;
    s32 b3;
    s32 b2;
    u8 *tab;

    sum = gStyleCounter + gStyleKind;
    kind = D_800873DC[sum & 0xF];
    gStyleVariant = kind;
    divisor = D_800873D8[kind];
    remainder = sum % divisor;
    D_8008AC84 = remainder;
    result = (s8 *) D_800873C8[kind] + remainder * 4;
    if (kind == 0) {
        b3 = result[3];
        gStyleFlushColor = (s32) (D_800872C4 + b3 * 3);
        b2 = result[2];
        tab = D_8008726C;
        if (b2 != 0x12) {
            tab = D_80087234;
        }
        gStyleColorTable = (s32) tab;
        if (remainder < 4) {
            gStyleDecorVariant = 1;
        } else if (remainder < 6) {
            gStyleDecorVariant = 2;
        }
    }
    return result;
}

extern s32 gStyleDecorPosAX;
extern s32 gStyleDecorPosAY;
extern s32 gStyleDecorPosBX;
extern s32 gStyleDecorPosBY;
extern void *New_ClassEAC0(void *a0, void *a1, s32 a2);
extern void *gStyleDecorSlots[];
extern s32 gStyleTargetObj;

typedef struct ObjSlot4C ObjSlot4C;
typedef struct ObjSlot4CMethods ObjSlot4CMethods;
struct ObjSlot4CMethods {
    u8 pad4C[0x4C];
    void (*slot4C)(ObjSlot4C *self, void *arg1, void *arg2); /* +0x04C */
};
struct ObjSlot4C {
    ObjSlot4CMethods *methods; /* +0x000 */
};

typedef struct ObjSlotAC ObjSlotAC;
typedef struct ObjSlotACMethods ObjSlotACMethods;
struct ObjSlotACMethods {
    u8 padAC[0xAC];
    void *(*slotAC)(ObjSlotAC *self); /* +0x0AC */
};
struct ObjSlotAC {
    ObjSlotACMethods *methods; /* +0x000 */
};

/* Local view: gStyleDecorPosAX/gStyleDecorPosAY and gStyleDecorPosBX/gStyleDecorPosBY are two
 * adjacent 8-byte pairs, and this unit copies each into a local pair as a
 * WHOLE-STRUCT assignment rather than field by field.  That is not a style
 * choice -- it is load-bearing.  A BLKmode set makes gcc 2.6.3's cse.c call
 * invalidate_memory(), dropping every cached memory value, which is what
 * produces retail's otherwise inexplicable reload of gStyleDecorVariant for the
 * `== 2` test and its reload of the pair's second word right after writing
 * it.  Written as two scalar stores, neither reload appears and the body is
 * several words short.  Round 61; see docs/match-reports/StyleBuildDecorSet.md. */
typedef struct PairXY PairXY;
struct PairXY {
    s32 x; /* +0x000 */
    s32 y; /* +0x004 */
};

/* STALL, 17/86 words (best), 1 word SHORT (map-measured 85 words), first
 * real diff at word 15 (0x4508C / vram 0x8005488C) -- see
 * docs/match-reports/StyleBuildDecorSet.md.  Round 61 REVISIT: 12/86 and 6 short
 * -> 17/86 and 1 short.  Residue is one redundant `move` retail emits and
 * two scheduling reorderings.  Preserved near-miss body: */
#if 0
void StyleBuildDecorSet(void) {
    PairXY paramA;
    PairXY paramB;
    s32 i;
    s32 s1;
    void **arr;
    void **wp;
    void *obj;
    ObjSlotAC *self2;
    void *result;

    if (gStyleDecorVariant == 0) {
        return;
    }
    paramA = *(PairXY *) &gStyleDecorPosAX;
    if (gStyleDecorVariant == 2) {
        paramA.y += 0x1E;
    }
    paramB = *(PairXY *) &gStyleDecorPosBX;
    i = 1;
    s1 = 3;
    obj = New_ClassEAC0(&paramB, (void *) gStyleColorTable, 0x1FFF);
    __asm__("");
    arr = gStyleDecorSlots;
    wp = arr + 1;
    *arr = obj;
    do {
        obj = New_ClassEAC0(&paramB, (void *) (s1 + gStyleColorTable), 0x1FFF);
        *wp = obj;
        wp++;
        ((ObjSlot4C *) obj)->methods->slot4C(obj, arr[0], &paramA);
        s1 += 3;
        paramA.y += 3;
        paramB.y -= 7;
        i++;
    } while (i < 0x12);

    self2 = *(ObjSlotAC **) (gStyleTargetObj + 0xC);
    result = self2->methods->slotAC(self2);
    ((ObjSlot4C *) gStyleDecorSlots[0])->methods->slot4C(gStyleDecorSlots[0], result, &paramA);
}
#endif
INCLUDE_ASM("asm/nonmatchings/class_3bb8c_n", StyleBuildDecorSet);

typedef struct ObjAC7CSub ObjAC7CSub;
typedef struct ObjAC7CSubMethods ObjAC7CSubMethods;
struct ObjAC7CSubMethods {
    u8 pad64[0x64];
    void (*slot64)(ObjAC7CSub *self, void *arg1); /* +0x064 */
};
struct ObjAC7CSub {
    ObjAC7CSubMethods *methods; /* +0x000 */
    u8 pad4[0x18 - 0x4];
    s32 field18; /* +0x018 */
    u8 pad1C[0x24 - 0x1C];
    s32 field24; /* +0x024 */
};

typedef struct ObjSlotB8B8 ObjSlotB8B8;
typedef struct ObjSlotB8B8Methods ObjSlotB8B8Methods;
struct ObjSlotB8B8Methods {
    u8 padB8[0xB8];
    void (*slotB8)(ObjSlotB8B8 *self, s32 arg1, void *arg2); /* +0x0B8 */
    void (*slotBC)(ObjSlotB8B8 *self, void *arg1); /* +0x0BC */
};
struct ObjSlotB8B8 {
    ObjSlotB8B8Methods *methods; /* +0x000 */
};

/* MATCHED round 61 (bravo), first attempt, after the BLKmode-struct-copy
 * lever found on StyleBuildDecorSet -- see docs/match-reports/StyleUpdateDecorSet.md.
 * `rgb` is written only at [0..2] (by AdjustRgbByDelta); its declared size of 8
 * is inferred from the STACK LAYOUT (it occupies sp+0x10..0x17, with `pos`
 * at sp+0x18), not from any access. */
void StyleUpdateDecorSet(void) {
    ObjAC7CSub *self;
    s32 delta;
    s32 shift;
    u8 rgb[8];
    PairXY pos;
    s32 srcOfs;
    s32 i;
    void **wp;
    ObjSlotB8B8 *obj;

    if (gStyleDecorVariant == 0) {
        return;
    }
    self = *(ObjAC7CSub **) (gStyleTargetObj + 0xC);
    delta = self->field18 - self->field24;
    shift = (delta / 600) * 3;
    if (shift <= 0) {
        return;
    }
    pos = *(PairXY *) &gStyleDecorPosAX;
    i = 0;
    if (gStyleDecorVariant == 2) {
        pos.y += 0x1E;
    }
    wp = gStyleDecorSlots;
    srcOfs = 0;
    pos.y += shift * 3;
    do {
        AdjustRgbByDelta(rgb, (u8 *) (srcOfs + gStyleColorTable), shift);
        obj = (ObjSlotB8B8 *) *wp;
        obj->methods->slotB8(obj, 1, rgb);
        obj = (ObjSlotB8B8 *) *wp;
        i++;
        srcOfs += 3;
        obj->methods->slotBC(obj, &pos);
        pos.y += 3;
        wp++;
    } while (i < 0x12);
    AdjustRgbByDelta(rgb, (u8 *) gStyleFlushColor, shift);
    self->methods->slot64(self, rgb);
}

void AdjustRgbByDelta(u8 *dst, u8 *src, s32 delta) {
    dst[0] = src[0] - delta;
    dst[1] = src[1] - delta;
    dst[2] = src[2] + delta;
}

extern void ReleaseBasicClassArray(void **array, s32 count);
extern s32 gStyleDecorVariant;
extern void *gStyleDecorSlots[];

void StyleReleaseDecorSet(void) {
    if (gStyleDecorVariant != 0) {
        ReleaseBasicClassArray(gStyleDecorSlots, 0x12);
        gStyleDecorVariant = 0;
    }
}

extern s32 gStyleVariant;
extern s32 gStyleTargetObj;
extern void BaseObjO__func_56f5c(s32 arg0, void *arg1, s32 arg2, s32 arg3);
extern s32 rand(void);
extern s8 D_80087324[];
extern s32 gStyleEffectSlotCount;
extern void *gStyleEffectSlots[];
extern void *StyleFillEffectKind0(void *arg0, s32 arg1, void *arg2);
extern void **StyleFillEffectKind1(void **arg0, s32 arg1, void *arg2);
extern void **StyleFillEffectKind3(void **arg0, void *arg1);
extern void **StyleFillEffectKind2(void **arg0, void *arg1);

void StyleBuildEffectSlots(void *arg0) {
    s32 base;
    s32 val;
    s32 count;
    void **filled;

    if (gStyleVariant < 0) {
        return;
    }
    base = gStyleTargetObj;
    BaseObjO__func_56f5c(gStyleVariant, (void *) *(s32 *) (base + 4), *(s32 *) (base + 8), *(s32 *) (base + 0xC));
    val = D_80087324[rand() & 3];
    count = (gStyleVariant == 2) ? 0x10 - val : 0;
    gStyleEffectSlotCount = val + count;
    filled = (void **) StyleFillEffectKind0(gStyleEffectSlots, val, arg0);
    filled = StyleFillEffectKind1(filled, count, arg0);
    if (gStyleVariant == 0) {
        StyleFillEffectKind3(filled, arg0);
    } else if (gStyleVariant == 2) {
        StyleFillEffectKind2(filled, arg0);
    } else {
        return;
    }
    gStyleEffectSlotCount = gStyleEffectSlotCount + 1;
}

/* Local view: array elements at gStyleEffectSlots are objects with a method table
 * pointer at offset 0, dispatched here through slot +0xEC as
 * slotEC(self, arg1) -- mirrors the ObjAB54 pattern above. */
typedef struct ObjE0C8 ObjE0C8;
typedef struct ObjE0C8Methods ObjE0C8Methods;
struct ObjE0C8Methods {
    u8 padEC[0xEC];
    void (*slotEC)(ObjE0C8 *self, void *arg1); /* +0x0EC */
};
struct ObjE0C8 {
    ObjE0C8Methods *methods; /* +0x000 */
};

extern s32 gStyleVariant;
extern s32 gStyleEffectSlotCount;
extern void *gStyleEffectSlots[];

void StyleUpdateEffectSlots(void *arg0) {
    s32 i;
    ObjE0C8 *obj;

    if (gStyleVariant < 0) {
        return;
    }
    for (i = 0; i < gStyleEffectSlotCount; i++) {
        obj = (ObjE0C8 *) gStyleEffectSlots[i];
        obj->methods->slotEC(obj, arg0);
    }
}

extern s32 gStyleVariant;
extern s32 gStyleEffectSlotCount;
extern void *gStyleEffectSlots[];

void StyleReleaseEffectSlots(void) {
    if (gStyleVariant >= 0) {
        ReleaseBasicClassArray(gStyleEffectSlots, gStyleEffectSlotCount);
    }
}

/* Local view only: `FlushStyleCue` (defined later in this unit, in strict
 * ROM order) takes one of these two per-slot objects. `StyleCueEntryView`
 * is the SAME record `FindNearestStyleCueEntry` returns as `EntrySlot *`
 * below -- a second independent local view of one struct, per the
 * multiple-independent-local-views convention, not merged with it: this
 * view only ever touches `countSign` (+0x6, an `EntrySlot::count`-typed
 * byte, address-taken then chased and toggled), where `EntrySlot` names the
 * rest. `StyleCueSlot::entry` is that claimed record, released by
 * `FlushStyleCue`/`StopStyleCueIfNear`. `posX`/`posZ` are the slot's own 2D
 * (X/Z) position, read by `IsStyleCueNear`'s distance check; `lastDist` is
 * that check's own last-computed distance (also the out-parameter
 * `FindNearestStyleCueEntry` writes). `cueSet` is only ever address-taken,
 * as an embedded sub-object handed to `FlushSoundCueSet`/`func_8002CD08`
 * (same discard-return caveat as `include/Entity.h`'s `unk9C` -- a field
 * only ever address-taken carries no evidence about its own declared
 * type). */
typedef struct StyleCueEntryView StyleCueEntryView;
struct StyleCueEntryView {
    u8 pad0[0x6];
    s8 countSign; /* +0x006 */
};

typedef struct StyleCueSlot StyleCueSlot;
struct StyleCueSlot {
    StyleCueEntryView *entry; /* +0x000 */
    s32 posX; /* +0x004 */
    u8 pad8[0x4];
    s32 posZ; /* +0x00C */
    s32 lastDist; /* +0x010 */
    s32 cueSet; /* +0x014 */
};

extern s32 FlushStyleCue(StyleCueSlot *arg0);

extern s32 gStyleCueSelf;
extern StyleCueSlot *gStyleCueSlots[2];

void StyleTeardown(void) {
    s32 i;

    StyleFlushDecoration();
    StyleReleaseDecorSet();
    StyleReleaseEffectSlots();
    for (i = 0; i < 2; i++) {
        gStyleCueSlots[i] = (StyleCueSlot *) FlushStyleCue(gStyleCueSlots[i]);
    }
    if (gStyleCueSelf != 0) {
        gStyleCueSelf = 0;
    }
}

extern u8 D_800871C8[];
extern s32 D_80087328[];
extern u8 *D_8008E0B4;
extern s32 D_8008E0BC;
extern s32 D_8008E0A4;
extern void SetupStyleSpawnParamsA(void *arg0, void *arg1);
extern void SetupStyleSpawnParamsB(void *arg0, void *arg1);
extern void *New_Class876FC(void *arg0, void *arg1, void *arg2, void *arg3);

/* STALL, 93/99 words (length matches, 0x18C, re-measured round 48; earlier
 * round 47 report recorded 87/99), whole-function arg0/arg1/arg2
 * register-colour rotation (s2/s5/s4) -- see
 * docs/match-reports/StyleFillEffectKind0.md. Round 48: check 3 confirms AGREE
 * (permuter scaffold: Insertions 0, Deletions 0, Reorderings 0, pure
 * Stack/Register-field residue -- matches the in-tree rebuild's pure
 * word-level register-field diffs). Preserved near-miss body: */
#if 0
void *StyleFillEffectKind0(void *arg0, s32 arg1, void *arg2) {
    void **arr;
    s32 i;
    s32 t3;
    void (*fp)(void *, void *);

    arr = (void **) arg0;
    D_8008E0BC = rand() % 7;
    D_8008E0B4 = (u8 *) D_800871C8 + ((u32) rand() % 5) * 12;
    t3 = (u32) rand() % 5;
    if (t3 != 0) {
        t3 = D_80087328[t3];
    }
    fp = SetupStyleSpawnParamsB;
    if (gStyleCounter % 7 != 0) {
        fp = SetupStyleSpawnParamsA;
    }
    for (i = 0; i < arg1; i++) {
        fp(arg2, (void *) t3);
        *arr = New_Class876FC((void *) 0, &D_8008E0A4, (void *) gStyleCueSelf, arg2);
        arr++;
    }
    return (void *) arr;
}
#endif
INCLUDE_ASM("asm/nonmatchings/class_3bb8c_n", StyleFillEffectKind0);

extern s32 D_80087330;
extern u8 D_80087204[];

void **StyleFillEffectKind1(void **arg0, s32 arg1, void *arg2) {
    s32 i;
    s32 val;

    val = D_80087330;
    D_8008E0B4 = D_80087204;
    for (i = 0; i < arg1; i++) {
        SetupStyleSpawnParamsA(arg2, (void *) val);
        *arg0 = New_Class876FC((void *) 1, &D_8008E0A4, (void *) gStyleCueSelf, arg2);
        arg0++;
    }
    return arg0;
}

extern s32 D_80087330;
extern void SetupStyleSpawnParamsA(void *arg0, void *arg1);
extern s32 D_8008E0C0[];
extern u8 *D_8008E0B0;
extern u8 D_80087174[];
extern s32 D_8008E0A8;
extern s32 D_8008E0AC;
extern u8 D_8008721C[];

/* STALL, 78/81 words, length EXACT (no drift), first real diff at word 69
 * (0x458EC / vram 0x800550EC) -- see docs/match-reports/StyleFillEffectKind3.md.
 * Round 61 REVISIT: 38/81 -> 78/81. Round 47's "pure arg0/arg1 register-
 * colour swap, ZERO drift" verdict was wrong on both counts; the equal
 * length was two defects cancelling. Residue is now ONE instruction's
 * placement: the `*q = D_80087174` store must sink below `lw a2` and
 * `move a3` into the jal's delay slot, and cannot because a store through
 * a pointer is an opaque MEM that gcc 2.6.3's scheduler will not let the
 * gp-relative load hoist across. Preserved near-miss body: */
/* STALL, 79/81 words, length EXACT, insertions 0 / deletions 0, first real
 * diff at word 60 (0x458C8 / vram 0x800550C8) -- see
 * docs/match-reports/StyleFillEffectKind3.md.  Round 61 REVISIT: 38/81 -> 79/81.
 * Round 47's "pure arg0/arg1 register-colour swap, ZERO drift" verdict was
 * wrong on both counts; the equal length was two defects cancelling.  What
 * is left is 2 words of genuine register identity: the else branch's
 * computed value sits in $a2 here and in $v1 in retail, because `t`'s single
 * pseudo (deliberately shared -- splitting it into two variables costs 4
 * words) coalesces with the third-argument register.  Preserved near-miss
 * body: */
#if 0
void **StyleFillEffectKind3(void **arg0, void *arg1) {
    s32 t;
    s32 *p;
    u8 **q;

    SetupStyleSpawnParamsA(arg1, (void *) D_80087330);
    if (gStyleDecorVariant != 0 && gStyleColorTable == (s32) D_8008726C) {
        D_8008E0A4 = 0xFFFF5000;
        D_8008E0A8 = -0x2000;
        D_8008E0AC = 0;
        D_8008E0C0[0] = (s32) (D_8008721C + 3);
    } else {
        p = &D_8008E0AC;
        if (*p > 0) {
            *p = -*p;
        }
        if (*p < -0x7800) {
            *p = -0x7800;
        }
        t = (s32) (D_8008721C + ((u32) rand() % 3) * 3);
        D_8008E0C0[0] = t;
    }
    t = gStyleCueSelf;
    q = &D_8008E0B0;
    *q = D_80087174;
    *arg0 = New_Class876FC((void *) 3, (u8 *) q - 0xC, (void *) t, arg1);
    arg0++;
    return arg0;
}
#endif
INCLUDE_ASM("asm/nonmatchings/class_3bb8c_n", StyleFillEffectKind3);

extern s32 D_80087430;
extern u8 D_80087228[];
extern s32 D_8008E0C0[];
extern u8 *D_8008E0B0;
extern u8 D_80087174[];
extern s32 D_8008E0BC;

/* STALL, 49/79 words, length EXACT (round 64, charlie: was 16/79 and 1 word
 * short).  Two levers closed the length gap and the whole prologue:
 *   (1) `u8 **q = &D_8008E0B0;` -- retail CACHES that address in a
 *       callee-saved register and derives D_8008E0A4's address from it as
 *       `q - 0xC`, which is the idiom StyleFillEffectKind3 below already uses.
 *       16/79 (1 short) -> 42/79 (exact).
 *   (2) the arg0 shape of the matched sibling StyleFillEffectKind1 -- `void **`
 *       parameter, `*arg0 = ...; arg0++; return arg0;` rather than a cast
 *       and `+ 4`.  42/79 -> 49/79, and the $s1/$s2 parameter colours and
 *       the entire prologue then matched exactly.
 * Residue is now ONLY the scheduling interleave (retail hoists the /20
 * `mult` and the gStyleCounter load into the /3 `multu`'s latency window,
 * ahead of the D_8008E0C0 address) plus the if/else block order.
 * MEASURED NEGATIVE, twice, on two different bodies: inverting the arms to
 * `% 20 != 0` first regresses (16->9/79 and 49->13/79) and reintroduces
 * length drift -- cc1 collapses `v0 = 0` into the branch delay slot and
 * loses retail's `addiu a2,a2,4` there.  Do not re-derive that.
 * Preserved near-miss body: */
#if 0
void **StyleFillEffectKind2(void **arg0, void *arg1) {
    s32 idx;
    s32 randval;
    s32 v0;
    s32 *slot;
    u8 **q;

    idx = (u32) rand() % 3;
    slot = D_8008E0C0;
    *slot = (s32) (D_80087228 + idx * 3);
    slot++;
    if (gStyleCounter % 20 == 0) {
        v0 = 0;
    } else {
        v0 = D_80087430;
    }
    *slot = v0;
    SetupStyleSpawnParamsA(arg1, (void *) D_80087330);
    q = &D_8008E0B0;
    *q = D_80087174;
    randval = rand();
    D_8008E0BC = randval - (randval / 3) * 6;
    *arg0 = New_Class876FC((void *) 2, (u8 *) q - 0xC, (void *) gStyleCueSelf, arg1);
    arg0++;
    return arg0;
}
#endif
INCLUDE_ASM("asm/nonmatchings/class_3bb8c_n", StyleFillEffectKind2);

extern s32 D_8008E0A8;
extern s32 D_8008E0AC;
extern u8 *D_8008E0B0;
extern u8 D_80087174[];
extern s32 D_8008E0B8;

/* MATCHED round 64 (charlie), 110/110, ins 0 / del 0, one build.  The
 * round-46..48 residue (an extra callee-saved register caching
 * `D_8008E0A4`'s address, frame -0x18 -> -0x20) was NOT register identity:
 * `D_8008E0A4` was declared as an INCOMPLETE ARRAY.  Every reference to
 * `extern T D_8008E0A4[]` is an array decay, i.e. an address-take VALUE,
 * which cc1 2.6.3's CSE promotes into a callee-saved register across the
 * intervening `rand()` calls; declared `extern s32 D_8008E0A4` it emits
 * retail's absolute `lui $at, %hi / sw %lo($at)` fresh at each of the three
 * accesses.  Four in-tree variants pin the axis to ARRAY vs SCALAR: the
 * element type (`u8[]` vs `s32[]`) and the cast spelling (`*(s32 *) &D_X`
 * vs `D_X`) are both measurably INERT.  Do not restate this as "the
 * declared type" -- that was the first, wrong, reading.
 * See docs/match-reports/SetupStyleSpawnParamsA.md. */
void SetupStyleSpawnParamsA(void *arg0, void *arg1) {
    if (arg1 == 0) {
        arg1 = (void *) D_80087328[rand() & 3];
    }
    D_8008E0A8 = (s32) arg1;
    D_8008E0A4 = (rand() % 23) << 11;
    if (rand() & 1) {
        D_8008E0A4 = -D_8008E0A4;
    }
    D_8008E0AC = (rand() % 23) << 11;
    if (rand() & 1) {
        D_8008E0AC = -D_8008E0AC;
    }
    D_8008E0B0 = D_80087174 + ((u32) rand() % 7) * 12;
    D_8008E0B8 = rand() % 5;
}

extern s32 D_8008732C;
extern s32 D_8008E0B8;

/* MATCHED round 64 (charlie), 87/87, ins 0 / del 0.  The round-46..48
 * residue -- filed as "pervasive $v0/$v1/$a0/$a1 temp-register renaming" and
 * searched for 900s / 136367 permuter iterations without a zero -- was ONE
 * named local.  The body used a single `s32 r` for all three `rand()`
 * results, whose live range spans the calls, so cc1 could not coalesce
 * `rand`'s `$v0` into it and emitted `move $a1,$v0` after each `jal rand`
 * (two visible, a third word from the knock-on).  Deleting `r` and calling
 * `rand()` inline in each expression -- exactly the idiom the matched
 * sibling SetupStyleSpawnParamsA above already uses -- keeps the value in `$v0` and
 * recolours the whole body to retail's.  `mod3` stays a local: it has two
 * genuine use points.  A permuter mutates a body but never deletes its
 * locals, which is why the 136367-iteration negative bounded the search and
 * not the function (round 63's LOCAL COUNT corollary).
 * The signature keeps round 47's two dead void* params: StyleFillEffectKind0
 * dispatches this through a function pointer shared with SetupStyleSpawnParamsA, so
 * the ABI slot is call-site-determined.  Dead params cost nothing here.
 * See docs/match-reports/SetupStyleSpawnParamsB.md. */
void SetupStyleSpawnParamsB(void *arg0, void *arg1) {
    s32 mod3;

    rand();
    D_8008E0A8 = D_8008732C;
    D_8008E0A4 = (rand() % 20) << 11;
    mod3 = gStyleCounter % 3;
    D_8008E0AC = 0xA000;
    if (mod3 == 1) {
        D_8008E0AC = -0xA000;
    } else if (mod3 == 2) {
        D_8008E0AC = 0x800;
    }
    D_8008E0B0 = D_80087174 + ((u32) rand() % 7) * 12;
    D_8008E0B8 = rand() % 5;
}

extern s32 gStyleTargetObj;
extern void *FindNearestStyleCueEntry(void *arg0, s32 *arg1, void *arg2);
extern s32 gStyleCueCallbacks[];
extern s32 InitSoundCueSet(s32 arg0, void *arg1, s32 arg2, void *arg3, s32 arg4);

StyleCueSlot *TryStartStyleCue(StyleCueSlot *arg0, s32 *arg1, void *arg2, void *arg3) {
    StyleCueEntryView *sub;

    sub = (StyleCueEntryView *) FindNearestStyleCueEntry(&arg0->posX, &arg0->lastDist, arg2);
    if (sub != 0) {
        arg0->entry = sub;
        InitSoundCueSet(*(s32 *) gStyleTargetObj, &arg0->cueSet, sub->countSign, arg0, gStyleCueCallbacks[sub->countSign]);
        if (sub->countSign == *arg1) {
            *arg1 = -sub->countSign;
        }
        sub->countSign = -sub->countSign;
        return arg0;
    }
    return 0;
}

extern s32 gStyleKind;
extern s32 gStyleCueRecordIndex;
extern u8 *D_800876B4[];
extern u8 D_800876EC[];
extern u8 D_800874EC[];
extern s32 gStyleCueDistanceTable[];

/* Local view only: `gStyleCueSelf`'s value is another "pointer stored as a
 * plain s32" (same idiom as `gStyleTargetObj`), here treated as a "self" object
 * with a method table at offset 0, dispatched through slot +0x0E8. Moved
 * ahead of its original spot (just before TickStyle) because
 * FindNearestStyleCueEntry, ROM-earlier, also dispatches through it. */
typedef struct ObjAB4C ObjAB4C;
typedef struct ObjAB4CMethods ObjAB4CMethods;
struct ObjAB4CMethods {
    u8 padE8[0xE8];
    void (*slotE8)(ObjAB4C *self, void *arg1, void *arg2); /* +0x0E8 */
};
struct ObjAB4C {
    ObjAB4CMethods *methods; /* +0x000 */
};

typedef struct Pos4 Pos4;
struct Pos4 {
    s16 hi;
    s16 lo;
};

typedef struct TabEntry TabEntry;
struct TabEntry {
    Pos4 head;
    s16 tail;
};

typedef struct EntrySlot EntrySlot;
struct EntrySlot {
    Pos4 pos;  /* +0x0 */
    u8 idx;    /* +0x4 */
    u8 pad5;   /* +0x5 */
    s8 count;  /* +0x6 */
    u8 pad7;   /* +0x7 */
};

typedef struct LocalBuf LocalBuf;
struct LocalBuf {
    Pos4 pos;
    TabEntry tab;
};

/* MATCHED round 48 (alpha), 111/111 -- see docs/match-reports/FindNearestStyleCueEntry.md
 * for the round 47 (bravo) recovery and the round 48 permuter lead that
 * closed it: the `if (n <= 0) goto fail;` early exit is redundant (the
 * `for (j = 0; j < n; ...)` loop already falls through to the same
 * `fail: return 0;` when n <= 0) and dropping it, plus writing the
 * `entry` pointer's address computation as `offset + (s32) base` instead
 * of `base + offset`, closed the last word (a pure commutative-operand
 * encoding-order residue in the `addu`). */
void *FindNearestStyleCueEntry(void *arg0, s32 *arg1, void *arg2) {
    s32 j, n;
    u8 *base;
    EntrySlot *entry;
    LocalBuf buf;
    s32 d1, d2, dist;
    void *self;

    if (arg2 == 0) {
        goto fail;
    }
    base = D_800876B4[gStyleKind];
    n = D_800876EC[gStyleKind] - gStyleCueRecordIndex;
    entry = (EntrySlot *) (gStyleCueRecordIndex * 8 + (s32) base);
    for (j = 0; j < n; j++, entry++) {
        gStyleCueRecordIndex++;
        if (entry->count > 0) {
            buf.pos = entry->pos;
            buf.tab = *(TabEntry *) (D_800874EC + entry->idx * 6);
            self = (void *) gStyleCueSelf;
            ((ObjAB4C *) self)->methods->slotE8((ObjAB4C *) self, arg0, &buf);
            d1 = *(s32 *) arg0 - *(s32 *) arg2;
            if (d1 < 0) {
                d1 = ~d1 + 1;
            }
            d2 = *(s32 *) ((u8 *) arg0 + 8) - *(s32 *) ((u8 *) arg2 + 8);
            if (d2 >= 0) {
                dist = d1 + d2;
            } else {
                dist = d1 - d2;
            }
            *arg1 = dist;
            if (dist < gStyleCueDistanceTable[entry->count]) {
                return entry;
            }
        }
    }
fail:
    return 0;
}

extern s32 gStyleTargetObj;
extern void FlushSoundCueSet(s32 arg0, void *arg1);

s32 FlushStyleCue(StyleCueSlot *arg0) {
    FlushSoundCueSet(*(s32 *) gStyleTargetObj, &arg0->cueSet);
    arg0->entry->countSign = -arg0->entry->countSign;
    return 0;
}

extern s32 IsStyleCueNear(StyleCueSlot *arg0, void *arg1);
extern void func_8002CD08(s32 arg0, void *arg1);

s32 StopStyleCueIfNear(StyleCueSlot *arg0, void *arg1, void *arg2) {
    if (IsStyleCueNear(arg0, arg1) != 0) {
        func_8002CD08(*(s32 *) gStyleTargetObj, &arg0->cueSet);
        return 1;
    }
    return 0;
}

extern s32 gStyleCueDistanceTable[];

s32 IsStyleCueNear(StyleCueSlot *arg0, void *arg1) {
    s32 dx, dy, dist;
    s8 idx;

    if (arg1 == 0) {
        return 0;
    }
    dx = arg0->posX - *(s32 *) arg1;
    if (dx < 0) {
        dx = ~dx + 1;
    }
    dy = arg0->posZ - *(s32 *) ((u8 *) arg1 + 0x8);
    if (dy >= 0) {
        dist = dx + dy;
    } else {
        dist = dx - dy;
    }
    arg0->lastDist = dist;
    idx = arg0->entry->countSign;
    if (dist < gStyleCueDistanceTable[-idx]) {
        dist = 1;
        return dist;
    }
    return 0;
}

extern void StyleBuildDecorSet(void);
extern void StyleUpdateDecorSet(void);
extern void DrawStyleTables(void);
extern s32 gStyleTickCount;
extern s32 gStyleCueRecordIndex;
extern u8 gStyleCueSlotPool[];
extern StyleCueSlot *TryStartStyleCue(StyleCueSlot *arg0, s32 *arg1, void *arg2, void *arg3);
extern s32 StopStyleCueIfNear(StyleCueSlot *arg0, void *arg1, void *arg2);

s32 TickStyle(void *arg0, void *arg1, s32 arg2) {
    void *ctx;
    u8 buf[0x10];
    s32 i;

    ctx = 0;
    if (arg0 != 0) {
        ctx = buf;
        ((ObjAB4C *) gStyleCueSelf)->methods->slotE8((ObjAB4C *) gStyleCueSelf, ctx, arg0);
    }
    if (gStyleTickCount++ == 0) {
        ApplyStyleDecorationIfSet();
        StyleBuildDecorSet();
        StyleBuildEffectSlots(ctx);
    }
    StyleUpdateDecorSet();
    StyleUpdateEffectSlots(ctx);
    DrawStyleTables();
    gStyleCueRecordIndex = 0;
    for (i = 0; i < 2; i++) {
        if (gStyleCueSlots[i] != 0) {
            if (StopStyleCueIfNear(gStyleCueSlots[i], ctx, arg1) == 0) {
                gStyleCueSlots[i] = (StyleCueSlot *) FlushStyleCue(gStyleCueSlots[i]);
            }
            /* INERT ON PURPOSE -- DO NOT DELETE. This pair is a semantic
             * no-op (`i` is the initialized loop counter, so nothing here is
             * an uninitialized read), and it exists solely because it
             * perturbs GCC 2.6.3's allocator back into retail's register
             * colours for `ctx`/`i`. Found by the permuter at iteration 4
             * and kept because the WHOLE-IMAGE SHA1 verifies with it, not
             * because the permuter's own scorer liked it (round 41: a
             * scorer zero is a lead, an `OK: build matches retail` is an
             * answer). Removing these two lines re-breaks TickStyle. */
            i++;
            i--;
        } else {
            gStyleCueSlots[i] = TryStartStyleCue((StyleCueSlot *) (gStyleCueSlotPool + i * 0x68), &arg2, ctx, arg1);
        }
    }
    return arg2;
}

extern s32 gStyleKind;
extern void func_8003B624(void *arg0, s32 arg1, void *arg2);
extern s32 D_80087444[];
extern s32 D_80087450[];
extern s32 D_8008745C[];
extern s32 D_80087468[];

void DrawStyleTables(void) {
    void *a0, *a2;
    s32 a1;

    if (gStyleKind == 2) {
        a0 = D_80087444;
        a2 = D_80087450;
        a1 = 1;
    } else if ((u32) (gStyleKind - 3) < 3) {
        a1 = 1;
        a0 = D_8008745C;
        a2 = D_80087468;
    } else {
        return;
    }
    func_8003B624(a0, a1, a2);
}
