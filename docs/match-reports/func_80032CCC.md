# func_80032CCC -- CONVERTED to a linked SDK object (round 33). NOT game code, NOT a stall.

> **ROUND 33 (2026-09-12), head. THIS FUNCTION IS NOW LINKED FROM SONY'S OWN
> OBJECT `libsnd/vs_vh.o` (Psy-Q 3.3) AND IS NAMED `SsVabOpenHeadSticky`.**
> The object straddles the old libsnd_ssinit_libapi_counter / code_179d8_i boundary at
> 0x23500 and owns four functions; both units trimmed and neither needed a
> new name. Every caller in `src/` carries the Sony name. Whole-image SHA1
> green. Nothing here is assignable and there is no stall left to work.
>
> **Everything below is kept as the derivation it was, not as live guidance.**


**Unit:** libsnd_ssinit · **Size:** 13 instructions · **Status:** MATCHED (13/13 words)

## What it does

Sibling of `func_80032C98`: a three-argument tail-call wrapper around
`func_80032D34`, forwarding `a0` and `a1` (`s16`, re-narrowed the same
way) and passing the caller's own third argument through as the callee's
fourth, with the callee's third argument fixed to `1` (vs. `0` in
`func_80032C98`). See `func_80032C98.md` for what `func_80032D34` is.

## The C

```c
s16 func_80032CCC(void *a0, s16 a1, s32 a2)
{
    return func_80032D34(a0, a1, 1, a2);
}
```

(shares the `extern s16 func_80032D34(void *a0, s16 a1, s32 a2, s32
a3);` prototype declared above `func_80032C98` in
`src/libsnd_ssinit.c`.)

## Provenance

round 16 (2026-09-04), runner delta, unit libsnd_ssinit (fresh carve,
second pass). Matched first attempt, alongside `func_80032C98`.
