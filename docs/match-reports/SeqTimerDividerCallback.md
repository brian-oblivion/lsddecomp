# SeqTimerDividerCallback

> Renamed from `func_80032AD0` on 2026-09-23 (tools/rename.py). Address 0x80032ad0.

**Unit:** code_179d8_c · **Size:** 18 instructions · **Status:** MATCHED (18/18 words)

## What it does

A toggle: flips the global flag `D_8006DCA0` between `0` and `1`, and
whenever it transitions back to `0` also calls `func_80033738` (the same
external cleanup/teardown routine `SeqTimerCallback` calls unconditionally
-- see that report). Reads naturally as a pause/mute-style toggle: "turn
on" just sets the flag, "turn off" clears it and runs the teardown.

## The C

```c
extern s32 D_8006DCA0;

void SeqTimerDividerCallback(void)
{
    if (D_8006DCA0 == 0) {
        D_8006DCA0 = 1;
    } else {
        D_8006DCA0 = 0;
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
