# StyleBuildEffectSlots -- MATCHED (60/60 words), class_3bb8c_n

> Renamed from `func_80054B84` on 2026-09-23 (tools/rename.py). Address 0x80054b84.

Round 46 (second sitting, alpha). Byte-exact, whole-image SHA1 verified.
Matched on the first build.

## Signature

```c
void StyleBuildEffectSlots(void *arg0);
```

## New externs

```c
extern void Actor__func_56f5c(s32 arg0, void *arg1, s32 arg2, s32 arg3); /* class_3bb8c_o.c, ALREADY MATCHED */
extern s32 rand(void);                                               /* libc, shared local view used project-wide */
extern s8 gStyleKind0Counts[];                                              /* 4-entry table, forward-indexed by rand()&3 */
extern void *StyleFillEffectKind0(void *arg0, s32 arg1, void *arg2);        /* forward decl, own unit, cold */
extern void **StyleFillEffectKind1(void **arg0, s32 arg1, void *arg2);      /* forward decl, own unit, ALREADY MATCHED this round */
extern void StyleFillEffectKind3(void *arg0, void *arg1);                   /* forward decl, own unit, cold */
extern void StyleFillEffectKind2(void *arg0, void *arg1);                   /* forward decl, own unit, cold */
```

`Actor__func_56f5c` is `class_3bb8c_o.c`'s already-matched
`void Actor__func_56f5c(s32 arg0, BaseObjO *self, s32 arg2, s32 arg3)`; this
call site only needs the ABI shape (`s32,void*,s32,s32`), matching the
project's convention of a looser cross-unit local signature. `gStyleVariant`,
`gStyleSceneRefs`, `gStyleEffectSlotCount` and `gStyleEffectSlots` are fresh copies of externs
already declared later in this file, needed here because this function's
ROM address is earlier (same reasoning as `StyleUpdateEffectSlots`/`TryStartStyleCue`).

## Body

```c
void StyleBuildEffectSlots(void *arg0) {
    s32 base;
    s32 val;
    s32 count;
    void **filled;

    if (gStyleVariant < 0) {
        return;
    }
    base = gStyleSceneRefs;
    Actor__func_56f5c(gStyleVariant, (void *) *(s32 *) (base + 4), *(s32 *) (base + 8), *(s32 *) (base + 0xC));
    val = gStyleKind0Counts[rand() & 3];
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
```

`base = gStyleSceneRefs;` reads the "pointer stored as a plain `s32`" global once
(matching this unit's established `gStyleSceneRefs` idiom, e.g.
`FlushSoundCueSet(*(s32 *) gStyleSceneRefs, ...)`), then indexes off it with plain
integer arithmetic (`*(s32 *) (base + 4)`, etc.) -- GCC keeps the single
load in a register and reuses it for all three offset reads, matching
retail's `lw v0, gStyleSceneRefs; lw a1,4(v0); lw a2,8(v0); lw a3,0xC(v0)`
without needing to fight CSE.

The trailing `if (gStyleVariant == 0) {...} else if (gStyleVariant == 2) {...}
else { return; }` (rather than three independent `if`s) is what reproduces
retail's shared "both branches converge, third one skips straight past" tail
exactly: the `gStyleEffectSlotCount = gStyleEffectSlotCount + 1;` increment is genuinely SKIPPED
when `gStyleVariant` is neither 0 nor 2 (retail's third path jumps directly to
the epilogue, bypassing the increment block entirely) -- an early `return`
in the `else` reproduces that skip.

## Attempts

1 (matched on the first build).

## Naming

**`StyleBuildEffectSlots`, tier B.**

Dispatches on `gStyleVariant` (`PickStyleFallbackConfig`'s "kind") to
`Actor__func_56f5c`, then fills `gStyleEffectSlots` via
`StyleFillEffectKind0`/`StyleFillEffectKind1`, then finishes via
`StyleFillEffectKind3` (variant 0) or `StyleFillEffectKind2` (variant 2).
The build/update/release triad naming mirrors `StyleBuildDecorSet` above,
for the SEPARATE `gStyleEffectSlots` array (a different object class --
`Obj876FC`, allocated through `class_3bb8c_r.c`'s `New_Class876FC`, not
`New_BoxFill`). MATCHED, 60/60, first build.

## Track 4 (2026-09-26, round 88, charlie)

`gStyleEffectSlots` retyped `void *[]` -> `Class876FC *[]` (every element is a New_Class876FC object); ReleaseBasicClassArray takes it as `(void **)`. Image byte-identical.

## Round 93 polish (delta, track 7)

### Naming

| old | new | tier | evidence |
| --- | --- | --- | --- |
| `D_80087324` | `gStyleKind0Counts` | A | 4 bytes {0, 3, 8, 16} picked by `rand() & 3`, passed as StyleFillEffectKind0's count. |
| `0x10` | `STYLE_VARIANT2_EFFECTS` (16) | B | variant 2 fills kind 1 up to this many kind-0 plus kind-1 effects. |

`gStyleSceneRefs + 4/8/0xC` are fields of the `StyleSceneRefs` view (ObjM's dreamerTmd, etcTim, cachedViewport). Locals: `pos`, `refs`, `kind0Count`, `kind1Count`, `next`.
