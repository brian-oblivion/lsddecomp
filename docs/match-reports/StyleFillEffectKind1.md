# StyleFillEffectKind1 -- MATCHED (42/42 words), class_3bb8c_n

> Renamed from `func_80054F30` on 2026-09-23 (tools/rename.py). Address 0x80054f30.

Round 46 (second sitting, alpha). Byte-exact, whole-image SHA1 verified.

## Signature

```c
void **StyleFillEffectKind1(void **arg0, s32 arg1, void *arg2);
```

Fills `arg1` slots of the array at `arg0` (advancing it by one pointer each
time) with `func_80056320(...)` results, and returns the pointer one past
the last slot written -- the classic "array fill, return next free slot"
idiom (matches this unit's already-established preference for that shape;
see `StyleTeardown`'s per-index rewrite of `gStyleCueSlots`).

## New externs

```c
extern s32 D_80087330;             /* only element [0] read here */
extern u8 D_80087204[];            /* address only taken, never indexed */
extern u8 D_8008E0A4[];            /* address only taken, passed to New_X */
extern u8 *D_8008E0B4;             /* set to &D_80087204 unconditionally */
extern void *SetupStyleSpawnParamsA(void *arg0, void *arg1);   /* forward decl, own unit, cold */
extern void *func_80056320(void *arg0, void *arg1, void *arg2, void *arg3); /* class_3bb8c_r.c, ALREADY MATCHED */
```

`D_80087330` is a 2-word dlabel in `asm/data/76DC8.data.s`; only the first
word is read here (`lw`, not indexed), so it is declared scalar rather than
an array -- if a sibling function later indexes `[1]`, retype there, not
here (no other unit references any of these four symbols currently).
`func_80056320` is `class_3bb8c_r.c`'s already-matched `New_X`-style
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

    val = D_80087330;
    D_8008E0B4 = D_80087204;
    for (i = 0; i < arg1; i++) {
        SetupStyleSpawnParamsA(arg2, (void *) val);
        *arg0 = func_80056320((void *) 1, D_8008E0A4, (void *) gStyleCueSelf, arg2);
        arg0++;
    }
    return arg0;
}
```

## Lever: capture a global into a local BEFORE a loop that calls through it,
even when the call's return value is discarded

First attempt read `D_80087330` directly inside the loop body
(`SetupStyleSpawnParamsA(arg2, (void *) D_80087330)`), which is semantically identical
-- but GCC 2.6.3 can't prove `SetupStyleSpawnParamsA` doesn't write back to
`D_80087330`, so it reloads the global from memory on every iteration
(`lui`/`lw` inside the loop, one fewer callee-saved register overall: 39
words instead of retail's 42). Retail hoists the read to a local
(`s32 val = D_80087330;`) *before* the loop, which is picked up into a
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
