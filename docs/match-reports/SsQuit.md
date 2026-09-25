# SsQuit

> Renamed from `QuitSpu` on 2026-09-25 (tools/rename.py). Address 0x80032a7c.

> Renamed from `ClearSpuMute` on 2026-09-23 (tools/rename.py). Address 0x80032a7c.

> Renamed from `func_80032A7C` on 2026-09-23 (tools/rename.py). Address 0x80032a7c.

**Unit:** code_179d8_c · **Size:** 8 instructions · **Status:** MATCHED (8/8 words)

## What it does

A zero-argument tail-call wrapper around `func_80038FB0` (lives in a
different, already-carved segment: `the 0x272C8..0x2C054 Psy-Q block (now linked from lib/ where a disc owns it, formerly asm/psyq_SpuSetMute.s)`). That
function guards on an SPU-mute-ish flag (`D_8006E184`), and when set,
clears it, resets several sound globals, and calls two SPU voice-key
helpers (`func_8003902C`/`func_8003903C`, both trivial `jr`-based
trampolines). It takes no arguments (called with a `nop` delay slot
everywhere it's invoked) and never sets `$v0`, so it is `void(void)`.

## The C

```c
extern void func_80038FB0(void);

void SsQuit(void)
{
    func_80038FB0();
}
```

## Provenance

round 16 (2026-09-04), runner delta, unit code_179d8_c (fresh carve).
Matched first attempt.

## Naming

Round 69. Tier A: a pure forwarder whose mechanics are its purpose. Its one
callee is `SpuQuit` (0x80038FB0, Sony's `libspu/s_q`, linked from the SDK
object; the `func_80038FB0` in "The C" above is its pre-round-34 name). The
"SPU-mute flag" in "What it does" is the round-16 reading of `SpuQuit`'s
library-initialised flag.

Head correction at merge: delta named this `ClearSpuMute` (tier B) from that
round-16 description without resolving the callee's current name, which the
naming prompt asks for (grep the placeholder callee). `SpuQuit` shuts the SPU
library down; nothing here clears a mute. Renamed `ClearSpuMute -> SsQuit`
with `tools/rename.py`, image byte-identical.

## Track 2 identification, round 79

This function itself IS Sony's `SsQuit` (`libsnd/ssinit`, disc 3.3), not a
game-authored wrapper -- the previous name (`QuitSpu`, tier A) was a
game-worded name over a Sony function, which `plan.py` flags under track 2.
`config/sdk-in-game.txt` carries it as a LEAD: `libsnd/scnoff:SsSetNoiseOff
libsnd/ssinit:SsQuit libsnd/ssnoff:SsSetNoiseOff libsnd/ssquit:SsQuit`,
AMBIGUOUS (18-way exact-fingerprint tie on this trivial 8-word shape, per
`tools/sdkname.py`). Settled by position: this address sits with zero gap
between `SsEnd` (0x800329D8) and `_SsTrapIntrVSync` (0x80032A9C), both
already identified as `libsnd/ssinit` at disc 3.3 (round 71). Of the
AMBIGUOUS candidates, only `SsQuit` (`libsnd/ssinit`, disc 3.3) and
`SsSetNoiseOff` (`libsnd/scnoff`/`libsnd/ssnoff`) share a libsnd library at
all, and only `SsQuit`'s module and disc match both neighbours' exactly.
The body (a single tail call into `SpuQuit`) is also what LIBSND.H's `SsQuit`
documents doing, not `SsSetNoiseOff`. Two independent evidence kinds
(fingerprint + position), both landing on `SsQuit`. Renamed `QuitSpu ->
SsQuit` with `tools/rename.py`, image byte-identical.
