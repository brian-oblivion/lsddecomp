# _SsSeqCalledTbyT_1per2

> Renamed from `SeqTimerDividerCallback` on 2026-09-23 (tools/rename.py). Address 0x80032ad0.

> Renamed from `func_80032AD0` on 2026-09-23 (tools/rename.py). Address 0x80032ad0.

**Unit:** code_179d8_c · **Size:** 18 instructions · **Status:** MATCHED (18/18 words)

## What it does

A toggle: flips the global flag `gSeqTimerDividerFlag` between `0` and `1`, and
whenever it transitions back to `0` also calls `func_80033738` (the same
external cleanup/teardown routine `_SsTrapIntrVSync` calls unconditionally
-- see that report). Reads naturally as a pause/mute-style toggle: "turn
on" just sets the flag, "turn off" clears it and runs the teardown.

## The C

```c
extern s32 gSeqTimerDividerFlag;

void _SsSeqCalledTbyT_1per2(void)
{
    if (gSeqTimerDividerFlag == 0) {
        gSeqTimerDividerFlag = 1;
    } else {
        gSeqTimerDividerFlag = 0;
        func_80033738();
    }
}
```

## Residue note: branch direction is not free to pick

The first attempt wrote the semantically-identical `if (flag != 0) {
clear+call } else { set }`. It compiled to the RIGHT instructions but the
WRONG block ordering: retail's raw `bnez $v0` (a direct truthy test on
the loaded flag, no separate compare) falls through into the `flag==0`
case and jumps *forward* into the `flag!=0`/call case. GCC 2.6.3 places
the textual `if`-body inline (fallthrough) and the `else`-body
out-of-line (jumped to) for this pattern, so getting the *inline* block
right requires writing the equality test in the direction that matches
which block is meant to be reached first. Flipping to `if (flag == 0)
{set} else {clear+call}` reproduced retail's block order exactly with no
other change. Filed here because it is a cheap, general check for this
project's future truthy-flag toggles: **when a raw (uncompared) register
feeds a `beqz`/`bnez` directly, the `==0`/`!=0` spelling controls block
order, not just polarity.**

## Provenance

round 16 (2026-09-04), runner delta, unit code_179d8_c (fresh carve).
Matched second attempt (one branch-direction flip).

## Naming

**Superseded, round 71 (track 2):** the track-3 game name
`SeqTimerDividerCallback` is replaced by Sony's own name -- fingerprint
EXACT masked 1.00 vs libsnd/ssinit's internal `_SsSeqCalledTbyT_1per2`
(disc 3.3). This is Sony's SDK code, not decompiled game logic; track 2
names those functions and moves them out of tracks 1/1b/3.

Round 69 (delta). `_SsSeqCalledTbyT_1per2` (was `func_80032AD0`): flips
`gSeqTimerDividerFlag` between 0 and 1, calling `SsSeqCalledTbyT` only on
the transition back to 0. `_SsStart` installs this as the RCnt/vsync
ISR callback (in place of `_SsTrapIntrVSync`) exactly when
`gSeqTimerRateFlag` is set -- i.e. when the timer is running a custom
(halved) rate, this callback only fires the sequencer tick every OTHER
interrupt. Tier B: the divide-by-two mechanism and its selection condition
are both directly evident from `_SsStart`'s body; the previous
"pause/mute-style toggle" guess in this report's older section is
superseded by this reading, which is grounded in the actual call site
rather than the toggle shape alone. `gSeqTimerDividerFlag` (was
`D_8006DCA0`) is named for the same reason.
