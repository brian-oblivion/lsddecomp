# Entity__MoodCue21 -- MATCHED (52/52 words)

> Renamed from `func_8005F0D8` on 2026-09-24 (tools/rename.py). Address 0x8005f0d8.

Unit: `Entity_c`. Runner: bravo.

## Shape

```c
void Entity__MoodCue21(Entity *this, EntityMoodHandlerArg *out) {
    s32 half;
    s32 rem;

    out->unk10 = this->methods->slot148(this);
    half = this->unk80 / 2;
    rem = out->unk4 % half;
    if (rem == 0) {
        out->unk1C = 0xA;
    } else if (rem == 3) {
        out->unk30 = 0xD;
    }
    this->methods->slotC4(this, -0x1E, 1);
}
```

## Notes

- `this->unk80 / 2` is the confirmed signed-halving idiom already documented
  on the field in `include/Entity.h` (`(x + (unsigned)x>>31) >> 1`), shared
  with `Entity__MoodCue09`/`Entity__MoodCue11` in `Entity_b.c`.
- The remainder of `out->unk4 % half` is computed once and tested against two
  literals (`0`, `3`) with different effects -- an ordinary `if`/`else if`
  reproduced the branch structure exactly; both branches converge before the
  unconditional `slotC4` call, matching retail's single merge label.
- Clean of both open toolchain blockers.

Matched first attempt (1/30).

## Naming

Why `MoodCueNN`: the function's address sits in `gEntityMoodHandlerTable` row 21 (`asm/data/79528.data.s`, base 0x80089EB0, stride 0x10; row = (slot address - 0x80089EB0) / 0x10), read directly off the table (this unit's own row assignment, round 78). Tier B: the row-to-function mapping is a compiler fact, not a guess, but which dream object or mood state each row represents is not established -- the row number is kept decimal and zero-padded so the names sort in table order, same convention as Entity_b/d/e/g.

## Proposed field names

`Entity::unk80` (+0x80) -- **tier B, proposed as `moodDuration`.** CROSS-UNIT
(read by every one of Entity_b/c/d/e/f/g, `grep -rn -- '->unk80\b'
src/Entity_*.c`), so proposed rather than applied. This function's own body
adds a third independent use to the ones already on file (`Entity__MoodCue09`/
`Entity__MoodCue11`'s halving, Entity_d/_f/_g's `moodTimer == unk80` /
`moodTimer < unk80 * K` threshold comparisons): `rem = out->unk4 % (unk80 /
2)`, a periodic-action modulus derived from the same field. Every known
reader across all six sibling units treats it as either (a) a threshold
`moodTimer` counts up to/through, or (b) a divisor/modulus scaling how often
a periodic action fires within that span -- both consistent with a single
per-instance "how long this mood state runs" constant, distinct from
`moodTimer` (the live counter) and `moodIndex` (the row selector). Not
proposing `unk84` alongside it: `Entity__MoodCue47`'s report (round 76)
already found `unk84` carries a SECOND, unrelated loop-counter meaning in
`Entity_f.c` (`func_80063ED4`/`func_80064078`), so a single name would
misdescribe one of its two uses; that objection does not apply to `unk80`
itself, which this unit's three call sites and every sibling unit's reads
treat uniformly.

`Entity::unkF4` (+0xF4) -- consistent with the existing tier-B proposal
`targetReached` (`Entity__UpdateTargetProximity.md`, Entity_b): this unit's
own two readers (`Entity__MoodCue23`, `Entity__MoodCue33`) both branch on it
as "target is within proximity range" (a movement-pose switch and a scale
change respectively), adding no new mechanics but no counter-evidence
either. Not re-proposed here, just corroborated.
