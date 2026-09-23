# SsStart2

> Renamed from `StopSeqTimer` on 2026-09-23 (tools/rename.py). Address 0x800329b8.

> Renamed from `func_800329B8` on 2026-09-23 (tools/rename.py). Address 0x800329b8.

**Unit:** code_179d8_c · **Size:** 8 instructions · **Status:** MATCHED (8/8 words)

## What it does

Sibling of `SsStart`: the same one-argument tail-call wrapper
around `_SsStart`, called with `0` instead of `1`. See
`SsStart.md` for what `_SsStart` itself does and why it is
`void`.

## The C

```c
void SsStart2(void)
{
    _SsStart(0);
}
```

(shares the `extern void _SsStart(s32 arg0);` prototype declared
above `SsStart` in `src/code_179d8_c.c`.)

## Provenance

round 16 (2026-09-04), runner delta, unit code_179d8_c (fresh carve).
Matched first attempt, alongside `SsStart`.

## Naming

Round 69 (delta). `SsStart2` (was `func_800329B8`): tail-call wrapper
calling `_SsStart(0)`. Tier B, mirroring `SsStart` -- see
`_SsStart.md`'s case-5 description: `arg0==0` increments
`gSeqTimerStopPending` and falls straight into the teardown path
(`VSyncCallback(SsSeqCalledTbyT)`) instead of arming a new rate.
