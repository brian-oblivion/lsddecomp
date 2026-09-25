# _SsTrapIntrVSync

> Renamed from `SeqTimerCallback` on 2026-09-23 (tools/rename.py). Address 0x80032a9c.

> Renamed from `func_80032A9C` on 2026-09-23 (tools/rename.py). Address 0x80032a9c.

**Unit:** code_179d8_c · **Size:** 13 instructions · **Status:** MATCHED (13/13 words)

## What it does

Calls through a global callback pointer if one is registered, then always
calls `func_80033738` (external, lives in the still-uncarved
`asm/code_179d8_tail.s` monolith -- declared `extern` locally per this
unit's own-declarations convention, not shared). `_snd_vsync_cb` is set
elsewhere (by `_SsStart`, not attempted this round) to either `0` or
the address of a callback such as this very function's sibling
`_SsSeqCalledTbyT_1per2`.

## The C

```c
extern void func_80033738(void);
extern void (*_snd_vsync_cb)(void);

void _SsTrapIntrVSync(void)
{
    if (_snd_vsync_cb != NULL) {
        _snd_vsync_cb();
    }
    func_80033738();
}
```

## Provenance

round 16 (2026-09-04), runner delta, unit code_179d8_c (fresh carve).
Matched first attempt.

## Naming

**Superseded, round 71 (track 2):** the track-3 game name `SeqTimerCallback`
is replaced by Sony's own name -- fingerprint EXACT masked 1.00 vs
libsnd/ssinit's internal `_SsTrapIntrVSync` (disc 3.3). This is Sony's SDK
code, not decompiled game logic; track 2 names those functions and moves
them out of tracks 1/1b/3.

Round 69 (delta). `_SsTrapIntrVSync` (was `func_80032A9C`): calls
`_snd_vsync_cb` if one was saved, then unconditionally calls
`SsSeqCalledTbyT`. This is the callback `_SsStart` installs via
`InterruptCallback` on the "no id yet" path (see `_SsStart.md`) --
i.e. it IS the root-counter/vsync interrupt handler that drives the
sequencer, chaining to whatever handler was previously installed. Tier B:
the ISR-chaining mechanism is directly evident from the body and the call
site; "drives the sequencer" rests on `SsSeqCalledTbyT`'s own name (Sony's,
per round 34's SDK linkage), not re-derived here.
