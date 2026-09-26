# StyleFillEffectKind1 -- MATCHED (42/42 words), class_3bb8c_n

> Renamed from `func_80054F30` on 2026-09-23 (tools/rename.py). Address 0x80054f30.

Round 46 (second sitting, alpha). Byte-exact, whole-image SHA1 verified.

## Signature

```c
void **StyleFillEffectKind1(void **arg0, s32 arg1, void *arg2);
```

Fills `arg1` slots of the array at `arg0` (advancing it by one pointer each
time) with `New_StyleEffect(...)` results, and returns the pointer one past
the last slot written -- the classic "array fill, return next free slot"
idiom (matches this unit's already-established preference for that shape;
see `StyleTeardown`'s per-index rewrite of `gStyleCueSlots`).

## New externs

```c
extern s32 gStyleSpawnYChoice2;             /* only element [0] read here */
extern u8 gStyleKind1Scale[];            /* address only taken, never indexed */
extern u8 gStyleSpawnOffsetX[];            /* address only taken, passed to New_X */
extern u8 *gStyleSpawnScale;             /* set to &gStyleKind1Scale unconditionally */
extern void *SetupStyleSpawnParamsA(void *arg0, void *arg1);   /* forward decl, own unit, cold */
extern void *New_StyleEffect(void *arg0, void *arg1, void *arg2, void *arg3); /* class_3bb8c_r.c, ALREADY MATCHED */
```

`gStyleSpawnYChoice2` is a 2-word dlabel in `asm/data/76DC8.data.s`; only the first
word is read here (`lw`, not indexed), so it is declared scalar rather than
an array -- if a sibling function later indexes `[1]`, retype there, not
here (no other unit references any of these four symbols currently).
`New_StyleEffect` is `class_3bb8c_r.c`'s already-matched `New_X`-style
allocator (`void *(void*,void*,void*,void*)`), called cross-unit by
prototype only. `SetupStyleSpawnParamsA` is one of this unit's own still-cold
functions (110w, queued later); its return value is discarded here (`jal`
result overwritten before use), so the forward declaration's return type is
unconstrained by this call site -- reconcile if its own definition needs a
narrower type.

## Body

```c
void **StyleFillEffectKind1(void **arg0, s32 arg1, void *arg2) {
    s32 i;
    s32 val;

    val = gStyleSpawnYChoice2;
    gStyleSpawnScale = gStyleKind1Scale;
    for (i = 0; i < arg1; i++) {
        SetupStyleSpawnParamsA(arg2, (void *) val);
        *arg0 = New_StyleEffect((void *) 1, gStyleSpawnOffsetX, (void *) gStyleGrid, arg2);
        arg0++;
    }
    return arg0;
}
```

## Lever: capture a global into a local BEFORE a loop that calls through it,
even when the call's return value is discarded

First attempt read `gStyleSpawnYChoice2` directly inside the loop body
(`SetupStyleSpawnParamsA(arg2, (void *) gStyleSpawnYChoice2)`), which is semantically identical
-- but GCC 2.6.3 can't prove `SetupStyleSpawnParamsA` doesn't write back to
`gStyleSpawnYChoice2`, so it reloads the global from memory on every iteration
(`lui`/`lw` inside the loop, one fewer callee-saved register overall: 39
words instead of retail's 42). Retail hoists the read to a local
(`s32 val = gStyleSpawnYChoice2;`) *before* the loop, which is picked up into a
saved register ($s4) that survives across the loop's two calls -- 3 extra
words (the load-once-and-keep pattern) and matches exactly.

### Proposed learning

**A global read inside a loop that also contains a call is NOT
automatically loop-invariant to GCC 2.6.3, even if the loop body never
writes that global** -- the compiler still reloads it every iteration
because the called function could alias it. If retail's version is 2-3
words LONGER and uses one MORE saved register than a straightforward
transliteration, check whether a global read inside the loop should instead
be captured into a local *before* the loop starts.

## Attempts

2 (first attempt: global read inside the loop, wrong -- one fewer saved
register, 3-word-short mismatch, out-of-range drift as expected for a size
change; second attempt: hoisted to a local before the loop, byte-exact).

## Naming

**`StyleFillEffectKind1`, tier B.**

Sibling of `StyleFillEffectKind0`: fills `arg1` slots of `gStyleEffectSlots`
via the same `New_StyleEffect` allocator, this time with a literal kind
argument of `1`. Called unconditionally (every `gStyleVariant`) from
`StyleBuildEffectSlots`, right after `StyleFillEffectKind0`. MATCHED,
42/42, second build (one lever: hoist the read of `gStyleSpawnYChoice2` out of the
loop).

## Track 4 (2026-09-26, round 88, charlie)

`gStyleEffectSlots` holds StyleEffect objects (New_StyleEffect), so the walking pointer is `StyleEffect **` and the position `LongVec3 *`; `kind` is passed as a plain `s32` (was `(void *) N`), the params block as `(StyleEffectParams *)` over the separately-declared gStyleSpawnOffsetX.. symbols (one 0x24-byte StyleEffectParams in the bytes; left as they are, a track 4b job), and gStyleGrid as the `SceneNode *` parent. Image byte-identical.

## Round 93 polish (delta, track 7)

### Naming

| old | new | tier | evidence |
| --- | --- | --- | --- |
| `D_80087204` | `gStyleKind1Scale` | A | stored as the params' scale for every kind-1 effect. |
| `D_80087330` | `gStyleSpawnYChoice2` | A | the word at `gStyleSpawnYChoices[2]` (-0x3800), a separate splat symbol; spelling it as the array element changes StyleFillEffectKind2's bytes (measured round 93), so the symbol stays. |

Locals: `slots`, `count`, `pos`, `offsetY`.
