# Entity__MoodCue46 -- MATCHED (67/67 words)

> Renamed from `func_80060800` on 2026-09-24 (tools/rename.py). Address 0x80060800.

Unit: `Entity_d` (second pass, round 2026-09-03). Mood-dispatch handler:
`void Entity__MoodCue46(Entity *this, EntityMoodHandlerArg *out)`.

## Final source

```c
void Entity__MoodCue46(Entity *this, EntityMoodHandlerArg *out) {
    s32 r;

    if (this->unkFC == 0) {
        r = this->unk94->methods->slot1A0(this->unk94, 0) % 3;
        if (r == 0) {
            if (rand() % 3 != 0) {
                goto skip48;
            }
        } else if (r != 1) {
            goto skip48;
        }
        this->methods->slot48(this, 1, SCALE_SIX);
    }
skip48:
    if (out->unk4 == 0) {
        out->unk10 = 0;
        out->unk1C = 0x12;
    }
    SceneNode__FaceTarget(this, this->unk94, 1, 0, 0);
}
```

## Derivation notes

- New vtable slot discovered: `Unk94Methods::slot1A0` (`s32 (*)(Unk94Obj
  *self, s32 arg1)`), called here as
  `this->unk94->methods->slot1A0(this->unk94, 0)`. Its return value is
  taken `% 3` immediately, so it is value-returning, not `void`. Added to
  `include/Entity.h`, shrinking the `pad134` gap between `slot130` and
  `slot200` in `Unk94Methods` (a new slot, not a retype of anything
  existing).
- **Textbook instance of "retail dispatches into a shared continuation with
  one arm's failure path relocated PAST the shared block; no arrangement of
  nested conditionals reproduces the layout" from
  `docs/DECOMPILATION_LEARNINGS.md`.** The reachability condition for the
  single `slot48` call is `(r == 0 && rand() % 3 == 0) || r == 1`, and every
  attempt to write it as a single boolean expression -- even a
  logically-correct `||` -- forced the compiler to keep the modulo result
  `r` alive in an EXTRA callee-saved register (bloating the frame by 8
  bytes, `s0`-`s3` instead of retail's `s0`-`s2`) purely so the second
  operand of `||` could re-read it after the intervening `rand()` call,
  even though that re-read is only reachable on a path where `r` was never
  clobbered. Retail's own bytes have no such register.
- The fix needed BOTH `goto`/label (to reach one physical `slot48` call site
  from two different predecessors without register bloat) AND the right
  choice of which arm is `if` vs `else if`. A first `goto` rewrite with `r
  != 0` as the primary arm fixed the register bloat (register allocation
  matched immediately) but left a structural residue: my `r == 1` case
  needed an extra `move`+`j` pair to reach the call site, because in my
  layout that check sat physically BEFORE the `rand()` block, while retail
  positions it physically AFTER the `rand()` block and immediately before
  the shared `slot48` call, letting the `r == 1` case fall through with no
  jump at all. Swapping to `r == 0` as the `if` (primary, laid out first)
  and `r != 1` as the `else if` (laid out second, immediately adjacent to
  the shared call) reproduced retail's exact block order.

### Proposed learning

- **A boolean `||`/`&&` across an intervening function call, where a later
  operand re-reads a value used in an earlier operand, can force that value
  into an otherwise-unnecessary callee-saved register** -- even when the
  re-read is only reachable on a control path where the value provably was
  never live past the call. The fix is `goto`/labels to reach the shared
  continuation explicitly, matching the general "shared continuation with a
  relocated failure path" idiom already documented, but the SPECIFIC tell
  here is a frame that gained one extra callee-saved slot versus retail
  purely from writing the reachability condition as one expression.
- **When a `goto`-based rewrite fixes register allocation but leaves an
  extra `move`+`j` pair, check whether swapping which arm is `if` vs `else
  if` removes it before trying anything else.** Retail lays out
  `if`-branch bodies in the ORDER WRITTEN, and whichever arm sits physically
  adjacent to a following shared block can fall through into it for free;
  the other arm always needs an explicit jump. Guess wrong and you pay for
  a jump retail's bytes don't have.

## Naming

`Entity__MoodCue46` -- tier B (round 76, runner delta, FINISHING-PLAN track 3). Same
convention as `Entity__MoodCue00` (round 71): the function's address is the
handler word of `gEntityMoodHandlerTable` (`asm/data/79528.data.s`, base
0x80089EB0, 0x10-byte stride) at row 46, read directly from
`disk/SLPS_015.56` (not inferred from address proximity -- see
`src/Entity_d.c`'s unit header comment, which flags that row order does NOT
track code address once row 115 is reached). Mechanics established
(mood-tick sound-cue-set callback, per `Entity__StartSoundCue`/
`Entity_b.c`'s own header comment); which dream object owns the row is not.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/Entity.h`: a TodActor subclass whose table and object expand `TODACTOR_SLOTS`/`TODACTOR_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 7 (round 93, bravo)

Every literal in the live body is in its base: decimal for moodTimer ticks, distances, TOD frames, VAB programs, volumes and `state` phases (hex remains only for masks). Byte-identical (whole image green).
