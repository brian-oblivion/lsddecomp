# SeqTimerCallback

> Renamed from `func_80032A9C` on 2026-09-23 (tools/rename.py). Address 0x80032a9c.

**Unit:** code_179d8_c · **Size:** 13 instructions · **Status:** MATCHED (13/13 words)

## What it does

Calls through a global callback pointer if one is registered, then always
calls `func_80033738` (external, lives in the still-uncarved
`asm/code_179d8_tail.s` monolith -- declared `extern` locally per this
unit's own-declarations convention, not shared). `gSeqTimerChainedCallback` is set
elsewhere (by `SeqTimerControl`, not attempted this round) to either `0` or
the address of a callback such as this very function's sibling
`SeqTimerDividerCallback`.

## The C

```c
extern void func_80033738(void);
extern void (*gSeqTimerChainedCallback)(void);

void SeqTimerCallback(void)
{
    if (gSeqTimerChainedCallback != NULL) {
        gSeqTimerChainedCallback();
    }
    func_80033738();
}
```

## Provenance

round 16 (2026-09-04), runner delta, unit code_179d8_c (fresh carve).
Matched first attempt.

## Naming

Round 69 (delta). `SeqTimerCallback` (was `func_80032A9C`): calls
`gSeqTimerChainedCallback` if one was saved, then unconditionally calls
`SsSeqCalledTbyT`. This is the callback `SeqTimerControl` installs via
`InterruptCallback` on the "no id yet" path (see `SeqTimerControl.md`) --
i.e. it IS the root-counter/vsync interrupt handler that drives the
sequencer, chaining to whatever handler was previously installed. Tier B:
the ISR-chaining mechanism is directly evident from the body and the call
site; "drives the sequencer" rests on `SsSeqCalledTbyT`'s own name (Sony's,
per round 34's SDK linkage), not re-derived here.
