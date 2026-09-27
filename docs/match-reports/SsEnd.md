# SsEnd

> Renamed from `CancelSeqTimer` on 2026-09-23 (tools/rename.py). Address 0x800329d8.

> Renamed from `func_800329D8` on 2026-09-23 (tools/rename.py). Address 0x800329d8.

**Unit:** libsnd_ssinit · **Size:** 41 instructions · **Status:** MATCHED (41/41 words)

## What it does

A callback-registration teardown/re-arm routine, guarded by
`_snd_seq_no_tick` (returns immediately, doing nothing, if it's nonzero).
Clears `_snd_1per2`, calls `func_80024CE0` (unconditional per-call
setup), then:

- if `_snd_use_vsync_cb` is set, calls `func_80024DA0(0)` and clears it;
- otherwise, if `_snd_use_interrupt_id` isn't the sentinel `-1`: when it holds a
  real id (`!= 0`) deregisters via `func_80024D40(id, NULL)`, else
  registers the callback `_snd_vsync_cb` via `func_80024D40(0,
  _snd_vsync_cb)`; either way resets `_snd_use_interrupt_id` to `-1` afterward.

Both paths fall through to a final `func_80024CF0()` call before
returning.

## The C

```c
extern void func_80024CE0(void);
extern void func_80024DA0(s32 arg0);
extern void func_80024D40(s32 arg0, void (*callback)(void));
extern void func_80024CF0(void);
extern s32 _snd_seq_no_tick;
extern s32 _snd_1per2;
extern s32 _snd_use_vsync_cb;
extern s32 _snd_use_interrupt_id;
extern void (*_snd_vsync_cb)(void);

void SsEnd(void)
{
    s32 v;

    if (_snd_seq_no_tick != 0) {
        return;
    }

    _snd_1per2 = 0;
    func_80024CE0();

    if (_snd_use_vsync_cb != 0) {
        func_80024DA0(0);
        _snd_use_vsync_cb = 0;
    } else {
        v = _snd_use_interrupt_id;
        if (v != -1) {
            if (v != 0) {
                func_80024D40(v, NULL);
            } else {
                func_80024D40(0, _snd_vsync_cb);
            }
            _snd_use_interrupt_id = -1;
        }
    }

    func_80024CF0();
}
```

## Residue note: another instance of the `_SsSeqCalledTbyT_1per2` block-order lever

First attempt wrote the if/else in "natural" order (`if (_snd_use_vsync_cb ==
0) { the _snd_use_interrupt_id logic } else { the func_80024DA0 teardown }`), which
compiled with the WRONG block placed inline: GCC 2.6.3 put the `==0`
branch inline/fallthrough and the `!=0` branch out-of-line, where retail
does the opposite (the `!=0`/teardown branch is the fallthrough, the
`==0`/re-arm branch is reached by a taken `beqz`). Swapping the C to test
`_snd_use_vsync_cb != 0` first (matching which block retail places first)
reproduced the exact instruction sequence with no other change --
consistent with the `_SsSeqCalledTbyT_1per2` finding, though note this is a
*different* shape than that one: this is a full `if/else` where BOTH
arms do real work and rejoin at a shared tail (not a guard-clause-with-
early-return), so the "boundary" written up in `GetRCnt.md` /
`ResetRCnt.md` (block-order flip doesn't help guard-clauses) does
not contradict this -- this residue confirms the lever DOES apply to
plain two-sided `if/else` compiled from a raw loaded value, just not to
"if (range check fails) return fail;" guards.

### Proposed learning

`_SsSeqCalledTbyT_1per2`'s block-order lever generalises to any two-sided `if
(rawValue) {A} else {B}` where GCC needs to decide which arm is
fallthrough vs. out-of-line -- not just single-sided toggles. The
distinguishing factor found so far across four instances this round: it
applies to raw-value/full-if-else shapes (`_SsSeqCalledTbyT_1per2`,
`SsEnd`), NOT to guard-clause range-check-then-bail shapes
(`GetRCnt`, `ResetRCnt`, `SetRCnt`), where negative-first is
already what retail compiles to and flipping regresses.

## Provenance

round 16 (2026-09-04), runner delta, unit libsnd_ssinit (fresh carve,
second pass, head-directed follow-up). Matched second attempt (one
if/else block-order flip after the head's `_SsSeqCalledTbyT_1per2` lever
generalisation request surfaced the same axis here).

## Naming

**Superseded, round 71 (track 2):** the track-3 game name `CancelSeqTimer`
is replaced by Sony's own name -- fingerprint EXACT masked 1.00 vs
libsnd/ssinit `SsEnd` (disc 3.3). This is Sony's SDK code, not decompiled
game logic; track 2 names those functions and moves them out of tracks
1/1b/3.

Round 69 (delta). `SsEnd` (was `func_800329D8`): guarded by
`_snd_seq_no_tick`, clears `_snd_1per2`, and either cancels a
pending stop (`VSyncCallback(0)`, clearing `_snd_use_vsync_cb`) or
deregisters/registers the RCnt interrupt callback via `InterruptCallback`
and resets `_snd_use_interrupt_id` to its `-1` sentinel -- the inverse of what
`_SsStart` arms. Tier B: the mechanism (tear down whatever
`_SsStart` set up) is clear from the shared globals; the caller
that decides WHEN to cancel is outside this unit.
