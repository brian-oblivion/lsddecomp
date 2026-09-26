# Entity__MoodCue52 -- MATCHED (78/78 words)

> Renamed from `func_80060F38` on 2026-09-24 (tools/rename.py). Address 0x80060f38.

Unit: `Entity_d` (second pass, round 2026-09-03). Mood-dispatch handler:
`void Entity__MoodCue52(Entity *this, EntityMoodHandlerArg *out)`.

## Final source

```c
void Entity__MoodCue52(Entity *this, EntityMoodHandlerArg *out) {
    out->unk10 = this->methods->slot148(this);
    if (this->unkFC < 0xBC) {
        if (this->unkFC == 0x54) {
            this->methods->slot44(this, 0, ROTATION_YAW_PLUS180);
        }
        if (out->unk4 % 20 == 0) {
            out->unk1C = 9;
        }
    } else if (this->unkFC < 0xC8) {
        this->methods->slot44(this, 0, ROTATION_YAW_PLUS9);
    } else {
        this->methods->slot160(this);
        out->unk30 = 0x1E;
        this->unk44 = 1;
    }
    this->methods->slotD0(this, -0x200, 0);
}
```

## Derivation notes

Matched first attempt, no iteration needed. Straightforward three-way
`if`/`else if`/`else` dispatch on `this->unkFC`, with a nested nested check
inside the first arm. Two things worth noting for the write-up, neither of
which needed a fix:

- The `out->unk4 % 20` check runs UNCONDITIONALLY inside the `unkFC < 0xBC`
  arm (both when `unkFC == 0x54` triggers the `slot44` call and when it
  doesn't) -- both sub-paths converge on the same magic-multiply-by-20
  check before falling to the shared `slotD0` tail call. The magic
  constant `0x66666667` with a `sra ...,3` post-shift is the same family as
  the divide-by-5 (`sra ...,1`) and divide-by-10 (`sra ...,2`) idioms seen
  elsewhere in this unit, generalizing to divide-by-`5*2^(shift-1)`; here
  shift 3 gives divide-by-20, and the final compare-against-`q*20`
  reconstructs a `% 20 == 0` test. No manual instruction reconstruction
  needed -- plain `%` reproduces it.
- All three vtable slots involved (`slot148`, `slot44`, `slot160`,
  `slotD0`) were already correctly typed in `include/Entity.h` from earlier
  units/functions; no header change needed here beyond adding the
  `ROTATION_YAW_PLUS180` data-table extern (same convention as this unit's other
  `D_80089Cxx`/`D_80089Exx` externs).

### Proposed learning

None beyond the already-documented magic-multiply-divisor family --this
confirms the `0x66666667` constant's shift amount generalizes cleanly to
`sra ...,3` for divide-by-20, extending the previously-confirmed
divide-by-5/divide-by-10 instances in this same unit.

## Naming

`Entity__MoodCue52` -- tier B (round 76, runner delta, FINISHING-PLAN track 3). Same
convention as `Entity__MoodCue00` (round 71): the function's address is the
handler word of `gEntityMoodHandlerTable` (`asm/data/79528.data.s`, base
0x80089EB0, 0x10-byte stride) at row 52, read directly from
`disk/SLPS_015.56` (not inferred from address proximity -- see
`src/Entity_d.c`'s unit header comment, which flags that row order does NOT
track code address once row 115 is reached). Mechanics established
(mood-tick sound-cue-set callback, per `Entity__StartSoundCue`/
`Entity_b.c`'s own header comment); which dream object owns the row is not.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `ENTITY_METHODS`) is unified as `Entity` in `include/Entity.h`: a Class65650 subclass whose table and object expand `CLASS65650_SLOTS`/`CLASS65650_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
