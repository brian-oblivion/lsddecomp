# StopSeqTimer

> Renamed from `func_800329B8` on 2026-09-23 (tools/rename.py). Address 0x800329b8.

**Unit:** code_179d8_c · **Size:** 8 instructions · **Status:** MATCHED (8/8 words)

## What it does

Sibling of `StartSeqTimer`: the same one-argument tail-call wrapper
around `SeqTimerControl`, called with `0` instead of `1`. See
`StartSeqTimer.md` for what `SeqTimerControl` itself does and why it is
`void`.

## The C

```c
void StopSeqTimer(void)
{
    SeqTimerControl(0);
}
```

(shares the `extern void SeqTimerControl(s32 arg0);` prototype declared
above `StartSeqTimer` in `src/code_179d8_c.c`.)

## Provenance

round 16 (2026-09-04), runner delta, unit code_179d8_c (fresh carve).
Matched first attempt, alongside `StartSeqTimer`.
