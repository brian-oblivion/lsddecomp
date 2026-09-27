# SsInitHot

> **Head, round 78: SONY CODE, identified under FINISHING-PLAN track 2 as `SsInitHot`** (libsnd/ssinit, body `_SsInit(1)`): EXACT `sdkname.py` fingerprint, 4-way AMBIGUOUS, settled by position and body; evidence on its symbols-file `identified` line. It counts as library now; the `## Naming` section below that kept `func_` is superseded.

> Renamed from `func_80032388` on 2026-09-24 (tools/rename.py). Address 0x80032388.

**Unit:** libsnd_ssinit · **Size:** 8 instructions · **Status:** MATCHED (8/8 words)

## What it does

Sibling of `SsInit`: the same one-argument tail-call wrapper
around `_SsInit`, called with `1` instead of `0`. See
`SsInit.md` for what `_SsInit` itself does and why it is
`void`.

## The C

```c
void SsInitHot(void)
{
    _SsInit(1);
}
```

(shares the `extern void _SsInit(s32 arg0);` prototype declared
above `SsInit` in `src/libsnd_ssinit.c`.)

## Provenance

round 16 (2026-09-04), runner delta, unit libsnd_ssinit (fresh carve).
Matched first attempt, alongside `SsInit`.

## Naming (round 78, runner alpha)

**Tier C, kept `func_`.** Sibling of `SsInit`'s naming finding:
`.venv/bin/python3 tools/sdkname.py SsInitHot` also returns an
EXACT(1.00 masked, 1.00 shape) fingerprint, AMBIGUOUS across 4
libsnd/libspu candidates (`SsInitHot`, `SsStart`, `SsUtReverbOn`,
`SpuInitHot`), position-decided to the same window between the placed
`libsnd/vm_vsu` and `libsnd/sstable` objects. Its arg=1 call matches the
`SsInitHot` half of the `SsInit`/`SsInitHot` pair. Same shape as this
round's `GsSetNearClip`/`GsSetWorkBase` finding (lesson 1); NOT given a
game name.
