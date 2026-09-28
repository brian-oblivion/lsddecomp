# IsDayInPeriodPhase -- MATCHED

> Renamed from `MatchesDreamAuxProgression` on 2026-09-27 (tools/rename.py). Address 0x8005cda8.

> Renamed from `MatchesDreamAuxRange` on 2026-09-21 (tools/rename.py). Address 0x8005cda8.

> Renamed from `func_8005CDA8` on 2026-09-21 (tools/rename.py). Address 0x8005cda8.

Unit `DreamAux` (was `code_4cd08`). 20/20 words, `0x4D5A8`-`0x4D5F8`. Whole-image
`build-and-verify.sh` green. First attempt matched.

```c
bool IsDayInPeriodPhase(s32 a0, s32 a1)
{
    s32 target = (a0 - 1) / 30 + 1;
    s32 i;

    for (i = 0; i < 4; i++) {
        if (target == a1) {
            return true;
        }
        a1 += 3;
    }
    return false;
}
```

## What it does

`target = (a0 - 1) / 30 + 1` (C truncating division), then tests whether
`a1` equals `target`, `target+3`, `target+6`, or `target+9` in turn (a
4-step arithmetic-progression search with stride 3), returning `true` on
the first match and `false` if none of the 4 hit. Called from
`CheckDreamAuxTriggerCondition` (still `INCLUDE_ASM`, see its stall report) as
`IsDayInPeriodPhase(value, idx - 1)` for switch indices 0-2 (i.e. `idx-1` in
`{1,2,3}`), suggesting `a1` is something like "which of a small fixed set of
3-wide day/slot buckets does `value` fall into" and this checks against
3 different starting offsets depending on which of those 3 switch cases
fired.

## Deriving the division constant

`0x88888889` (`mult`) with a post-multiply `>>4` (`sra ..., 4`) is GCC
2.6.3's standard signed-division-by-constant expansion (Hacker's-Delight-
style magic number + shift, needing the `+n` correction because the magic
overflows the sign bit). Rather than guess, solved it: the standard
magic-number-for-signed-division algorithm, run for candidate divisors,
produces `M=0x88888889, shift=4` for **d=30** (also unofficially for
15/60/120, but shift 4 pins it to 30). Confirmed by brute-force simulating
the exact instruction sequence
(`(a0-1)`, `mulhi`, `+(a0-1)`, `>>4`, `- sign(a0-1)`, `+1`) against
`(a0-1)/30 + 1` computed with C-style truncating division across a wide
range of inputs including negatives -- exact match at every point, including
the asymmetric boundary at `a0 = -28` where naive floor-division intuition
gives the wrong bucket but truncating (round-toward-zero) division doesn't.

## Proposed learning

- **When a `mult`+shift div-by-constant sequence doesn't obviously match a
  "nice" divisor, brute-force the standard magic-number algorithm across a
  candidate range rather than guess-and-check by hand.** `0x88888889` alone
  suggested "divide by 9" from familiarity with smaller magic constants, but
  the `>>4` shift is part of the SAME division, not a separate `/16` — the
  actual divisor was 30, only reachable by solving `magic_signed(d)` for
  many `d` and matching both the constant AND the shift together. Hand-
  matching just the multiplier is not enough since several divisors
  (15/30/60/120) share close constants at different shifts.

## Naming

**IsDayInPeriodPhase** — tier A. A pure predicate: `target = (a0-1)/30+1`,
then tests whether `a1` equals `target`, `target+3`, `target+6` or
`target+9` (stride-3, 4-step arithmetic progression), true on the first hit.
The mechanics (a range/bucket membership test) ARE the name; tier A by the
pure-leaf rule. Deliberately did NOT name this around "day" despite the
division by 30 -- `a0` traces back to an external caller's opaque field
(`child->unk4->unk34` via `TryDreamAuxTrigger`), and nothing in this unit
ties it to an actual day counter.

## Head review, round 63: `MatchesDreamAuxRange` -> `IsDayInPeriodPhase`

Renamed at merge review (`tools/rename.py`, image byte-identical). The runner's
evidence for tier A is sound -- this is a pure leaf whose mechanics are its
purpose -- but the word "Range" asserted a property the body does not have.

The body derives `target = (a0 - 1) / 30 + 1` and then tests `target` against
`a1`, `a1 + 3`, `a1 + 6`, `a1 + 9`: membership in a four-element arithmetic
progression of STRIDE 3, not in a contiguous range. A reader who trusted
"Range" would expect `a1 <= target <= a1 + 3` and would misread the sole call
site (`!IsDayInPeriodPhase(value, idx - 1)`) in a way the disassembly
does not support. Tier stays A; only the noun changed.

The `/ 30` is suggestive of a day-to-period conversion given this project's
`DreamSys__AdvanceDay`, but nothing in this unit establishes it, so the name
does not encode it.

## Round 100 (alpha): track 7, moved from src/DreamAux.c and include/DreamAux.h

## Naming (round 100)

**IsDayInPeriodPhase** (was MatchesDreamAuxProgression) -- tier B.
`(day - 1) / 30 + 1` is the day's 30-day period counted from 1 (day is
DreamSys's 1-based current day), and the loop accepts phase, phase + 3,
phase + 6 and phase + 9: every third period of twelve. Its caller passes
phase 1..3 (conditions 2..4). 30 is DREAM_PERIOD_DAYS. Tier B: the
mechanics are exact, the periods' meaning in the game is not established.
a0/a1/target -> day/phase/period. Byte-identical.
