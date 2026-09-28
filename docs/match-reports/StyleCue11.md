# StyleCue11 -- MATCHED (44/44 words)

> Renamed from `func_800560E4` on 2026-09-23 (tools/rename.py). Address 0x800560e4.

Unit: `class_3bb8c_k` (round 17 continuation). Slot occupant #12 of
`gStyleCueCallbacks`. Chains to `StyleCue10` (a PLAIN CALL by symbol, not
through `ComputeStyleCueFalloff` again) then dispatches on `self->kind % 70`.

## Final source

```c
void StyleCue11(StyleCueParam *ctx, StyleCueParam *self) {
    s32 rem;

    StyleCue10(ctx, self);
    rem = self->kind % 70;
    if (rem == 50) {
        self->unk44 = 0x14;
        self->unk48 = 1;
    } else if ((u32)(rem - 54) < 5) {
        self->unk44 = 0xD;
        self->unk48 = 1;
    } else if (rem == 61) {
        self->unk44 = 9;
        self->unk48 = -1;
    }
}
```

## Derivation

The FIRST instruction is `jal StyleCue10`, not `jal ComputeStyleCueFalloff` --
this function does not call the shared helper itself, it delegates
entirely to its sibling slot occupant (which internally calls
`ComputeStyleCueFalloff` and sets `self->falloff` on its own). `self->kind % 70` uses
magic `0xEA0EA0EB` with the "ADD variant" (`mfhi` result added to the
dividend before the final shift, rather than subtracted, matching the
same `addu`-before-`sra` pattern already seen for divisor 7 in
`StyleCue04`) -- recombination confirms N=70 (`sll3,addu,sll2,subu,sll1`
= `q*9, +q(=10), *4(=40)-q... ` reduces to `q*70`; verified directly
against the disassembly rather than by formula alone, per
`StyleCue03`'s caution). The `(u32)(rem-54) < 5` arm is the same
unsigned-range idiom as `StyleCue05`'s report, here testing
`rem` in `[54, 58]`.

### Proposed learning

None -- confirms the unsigned-range idiom and the "ADD variant" magic
division shape already documented elsewhere in this unit.

## Naming

**Tier B.** `StyleCue11` is row +0x030 of `gStyleCueCallbacks` (`tools/classtable.py 0x800874B0`, round 73). `TryStartStyleCue` (class_3bb8c_k.c) installs `gStyleCueCallbacks[sub->countSign]` as `SoundCueSet::callback` via `InitSoundCueSet` (PlacementGridVabSound.c) -- the same per-tag sound-cue-callback slot `gEntityMoodHandlerTable`'s `MoodCueNN` occupants hold for `Entity` (`Entity__MoodCueNN` match reports). The `StyleCueNN` numbering follows table row order, same convention as `MoodCueNN`. Mechanics are established (a per-tag callback that reads `self->kind` and writes a handful of numeric fields, calling `ComputeStyleCueFalloff` first); which dream/style object or which field means what in the running game is not, so the specific `kind` branches and the numeric literals they write stay unnamed.

## Track 6 (2026-09-26, round 92, alpha): `set` is a SoundCueSet

class_3bb8c_k.c's `StyleCueParam` used to type both parameters of every
StyleCueNN callback and of ComputeStyleCueFalloff; its old comment called it
"very likely a SoundCueSet-shaped object" but kept one local type because
nothing confirmed `self` and `ctx` were the same object. They are not, and
the two now have different types:

- The second parameter (was `self`, now `set`) is the SoundCueSet
  (include/SoundCueSet.h). ServiceSoundCueSet calls `callback(owner, set)`,
  and every offset the callbacks write agrees: +0x04 (was `kind`, the value
  every callback dispatches on) is `tick`, +0x10 (was `falloff`) is
  `attenuation`, and +0x1C..+0x50 (was `unk1C`..`unk50`) are
  `slots[0..2].program/octave/vol/endVol`. The callbacks' `-1` store to
  +0x04 is the same restart the Entity__MoodCueNN handlers do.
- The first parameter (`ctx`) is the owner, class_3bb8c_k.c's
  `StyleCueSlot`: TryStartStyleCue passes the slot as InitSoundCueSet's
  owner and `&slot->cueSet` (+0x14) as the set. So `StyleCueParam` is now a
  local view of StyleCueSlot: `methods` (+0x00) is the claimed record,
  renamed `entry`, whose +0x06 `tag` is class_3bb8c_k's `countSign`;
  `falloff` (+0x10) is `lastDist`; and `unk28` (+0x28) is
  `cueSet.attenuationSteps` (+0x14 + 0x14). ComputeStyleCueFalloff therefore
  scales the slot's last distance into 0..attenuationSteps against the cue's
  distance limit, as Entity__GetProximityRatio does for Entity.

Locals `kind` became `tick`. Zero bytes.

## Track 7 (2026-09-27, round 96, charlie)

- Every literal is decimal (VAB program numbers, volumes and tick counts are
  counts, not masks; the `% 70` phase windows). None is named: a program number's sound is not
  established, and a name like `PROGRAM_30` would only restate it. Zero bytes.
- `(u32)(rem - 54) < 5` is now `rem >= 54 && rem < 59`, byte-exact (44/44
  and the whole image, measured this round): GCC folds it to the same
  unsigned range test.
