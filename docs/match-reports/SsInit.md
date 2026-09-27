# SsInit

> **Head, round 78: SONY CODE, identified under FINISHING-PLAN track 2 as `SsInit`** (libsnd/ssinit, body `_SsInit(0)`): EXACT `sdkname.py` fingerprint, 4-way AMBIGUOUS, settled by position and body; evidence on its symbols-file `identified` line. It counts as library now; the `## Naming` section below that kept `func_` is superseded.

> Renamed from `func_80032368` on 2026-09-24 (tools/rename.py). Address 0x80032368.

**Unit:** libsnd_ssinit · **Size:** 8 instructions · **Status:** MATCHED (8/8 words)

## What it does

A one-argument tail-call wrapper around `_SsInit`, called with `0`.
`_SsInit` (still `INCLUDE_ASM` in this unit -- not attempted this
round) is an SPU/sound-system init routine: it branches on its argument,
calls one of two setup helpers (`SpuInit` / `SpuInitHot`)
depending on whether the argument is zero, then unconditionally copies two
fixed tables into the SPU control register window at `0x1F801C00`/
`0x1F801D80` and clears several sound-system globals. It never sets `$v0`
on any return path, so it is `void`.

## The C

```c
extern void _SsInit(s32 arg0);

void SsInit(void)
{
    _SsInit(0);
}
```

## Signature note

The byte-identical shape here (`addiu sp; sw ra; jal; nop-equivalent
delay; lw ra; addiu sp; jr ra`) tells you nothing about `_SsInit`'s
own return type -- see CLAUDE.md's tail-call warning. Resolved it instead
from `_SsInit`'s own body: no instruction on any of its exit paths
sets `$v0`, so `void` is correct, not `s32`.

## Provenance

round 16 (2026-09-04), runner delta, unit libsnd_ssinit (fresh carve).
Matched first attempt.

## Naming (round 78, runner alpha)

**Tier C, kept `func_`.** `.venv/bin/python3 tools/sdkname.py SsInit`
returns an EXACT(1.00 masked, 1.00 shape) fingerprint, AMBIGUOUS across 4
libsnd/libspu candidates (`SsInit`, `SsStart2`, `SsUtReverbOff`, `SpuInit`),
position-decided to sit between the placed `libsnd/vm_vsu` object (ends
0x8003221C) and the placed `libsnd/sstable` object (starts 0x800323A8) --
the same "EXACT fingerprint + position between two placed objects" shape
this round's `GsSetNearClip`/`GsSetWorkBase` finding used to withhold a
game name. Its own callee, `_SsInit`, is already Sony's identified name,
and this wrapper's arg=0 call matches the `SsInit` half of the
`SsInit`/`SsInitHot` pair among the candidates. Per lesson 1 (this round's
brief), NOT given a game name; reported to head via broadcast and the unit
header comment instead of renamed.
