# StartSeqTimer

> Renamed from `func_80032998` on 2026-09-23 (tools/rename.py). Address 0x80032998.

**Unit:** code_179d8_c · **Size:** 8 instructions · **Status:** MATCHED (8/8 words)

## What it does

A one-argument tail-call wrapper around `SeqTimerControl`, called with `1`.
`SeqTimerControl` (still `INCLUDE_ASM` in this unit -- not attempted this
round, 185-line body) is a CD-audio/root-counter driven playback-rate
routine: it busy-waits on two software delay loops, reads a CD status
global (`gSeqTimerRateMode`), branches into several `div`-based rate
calculations, and conditionally calls `SetRCnt`/`SetIrqMask`/
`ResetRCnt` (all in this same unit) among others. It never sets `$v0`
on any exit path, so it is `void`.

## The C

```c
extern void SeqTimerControl(s32 arg0);

void StartSeqTimer(void)
{
    SeqTimerControl(1);
}
```

## Signature note

Same tail-call caveat as `func_80032368`: the byte pattern alone doesn't
tell you the callee's return type. Resolved from `SeqTimerControl`'s own
body -- no exit path sets `$v0`.

## Provenance

round 16 (2026-09-04), runner delta, unit code_179d8_c (fresh carve).
Matched first attempt.
