# Entity__MoodCue39 -- MATCHED (115/115 words)

> Renamed from `func_8005FF7C` on 2026-09-24 (tools/rename.py). Address 0x8005ff7c.

Unit: `Entity_d` (second pass, round 2026-09-03). Mood-dispatch handler,
lowest ROM address in this unit's queue. `void
Entity__MoodCue39(Entity *this, EntityMoodHandlerArg *out)`.

## Final source

```c
void Entity__MoodCue39(Entity *this, EntityMoodHandlerArg *out) {
    s32 r;

    if (this->unkFC == 0) {
        r = this->unk94->methods->slot1A0(this->unk94, 0) % 3;
        if (r == 0) {
            if (rand() % 3 != 0) {
                goto skip48;
            }
        } else if (r != 2) {
            goto skip48;
        }
        this->methods->slot48(this, 1, SCALE_Y4);
    }
skip48:
    if (out->unk4 % 22 == 0) {
        out->unk10 = this->methods->slot148(this);
        out->unk1C = 2;
    }
    if (rand() % 12 == 0) {
        this->methods->slot130(this);
    } else if (rand() % 6 == 0) {
        this->methods->slot12C(this);
    }
}
```

## Derivation notes

Matched first attempt, no iteration needed -- this is the third function in
this unit to reuse the exact `Unk94Methods::slot1A0` result-mod-3 dispatch
shape first derived in `Entity__MoodCue46`'s report (see that report for the
full residue history: the reachability condition `(r==0 && rand()%3==0) ||
r==2` needed the same `goto skip48;` pattern proven there, applied directly
with no new derivation needed this time).

- `out->unk4 % 22 == 0` is the magic-multiply-by-22 idiom -- same family as
  `Entity__MoodCue52`'s divide-by-20 and this unit's divide-by-5/divide-by-10
  instances, extending the confirmed generalization further (22 = not a
  power of 5 times 2^n this time, a genuinely different magic constant
  `0x2E8BA2E9`, and it still needed no manual reconstruction -- plain `%`
  reproduced it exactly).
- **The same base magic constant (`0x2AAAAAAB`) computes TWO DIFFERENT
  divisors depending on whether an extra `sra ...,1` follows the `mfhi`.**
  With the extra shift: divide-by-12. Without it: divide-by-6. Both are
  used in this one function, back to back (`rand() % 12 == 0` then, only if
  that fails, `rand() % 6 == 0` on a FRESH `rand()` call). Both reproduced
  directly with plain `%`; no manual arithmetic needed, but worth recording
  because it means the SAME hex magic constant appearing twice in a
  disassembly is not evidence of the same divisor -- check the post-`mfhi`
  shift amount each time.
- Reuses already-typed `slot1A0`, `slot48`, `slot148`, `slot130`, and
  `slot12C` (the last two both `EntityMethods` slots, not `Unk94Methods` --
  worth noting since the function mixes calls through `this->methods` and
  `this->unk94->methods` in the same body, and it is easy to mis-route a
  slot number to the wrong table when both happen to be in play). No header
  changes needed.

### Proposed learning

- **A magic-multiply divisor family confirmed via `Entity__MoodCue46`
  generalizes across functions with zero rederivation once the C idiom
  (`goto` to a shared label for a two-clause OR reachability condition) is
  established** -- this function needed no new investigation, just applying
  the known pattern. Worth checking any REMAINING unattempted function
  in this unit for the identical `slot1A0 % 3` dispatch shape before
  re-deriving it from scratch.
- **The same magic-multiply constant can encode different divisors
  depending on an extra post-`mfhi` shift.** Do not assume two appearances
  of the same hex constant in one function imply the same divisor -- check
  the shift amount at each site independently.

## Naming

`Entity__MoodCue39` -- tier B (round 76, runner delta, FINISHING-PLAN track 3). Same
convention as `Entity__MoodCue00` (round 71): the function's address is the
handler word of `gEntityMoodHandlerTable` (`asm/data/79528.data.s`, base
0x80089EB0, 0x10-byte stride) at row 39, read directly from
`disk/SLPS_015.56` (not inferred from address proximity -- see
`src/Entity_d.c`'s unit header comment, which flags that row order does NOT
track code address once row 115 is reached). Mechanics established
(mood-tick sound-cue-set callback, per `Entity__StartSoundCue`/
`Entity_b.c`'s own header comment); which dream object owns the row is not.

## Track 4 (2026-09-26, round 88, echo)

Measured: through Class65650's `s32 playTod` slot this function grows 3 words (the playTod call no longer cross-jumps with the void stopTod call). Every Entity playTod call casts the slot to `EntityPlayTodFn` (void), which emits no code.

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/Entity.h`: a Class65650 subclass whose table and object expand `CLASS65650_SLOTS`/`CLASS65650_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
