# StyleCue10 -- MATCHED (36/36 words)

> Renamed from `func_80056054` on 2026-09-23 (tools/rename.py). Address 0x80056054.

Unit: `class_3bb8c_r` (round 17 continuation). Slot occupant #11 of
`gStyleCueCallbacks`. Dispatches on the ACTUAL REMAINDER of `self->kind % 20`
(not just divisibility) -- the first sibling in this family to keep the
remainder value itself rather than only testing it against zero.

## Final source

```c
void StyleCue10(StyleCueParam *ctx, StyleCueParam *self) {
    s32 rem;

    self->falloff = ComputeStyleCueFalloff(ctx);
    rem = self->kind % 20;
    if (rem == 1) {
        self->unk1C = 9;
        self->unk20 = -2;
    } else if (rem == 16) {
        self->unk30 = 9;
        self->unk34 = -2;
    }
}
```

## Derivation

Retail computes `q = kind/20` (same `%20` magic as `StyleCue03`) and
then `rem = kind - q*20` explicitly (the SUBTRACTION result itself is
kept and compared against 1 and 16, rather than the earlier siblings'
pattern of comparing the dividend against `q*N` directly). Writing
`self->kind % 20` and caching it in a `rem` local reproduces this --
GCC's ordinary `%` codegen computes exactly this quotient-then-subtract
sequence, and caching it avoids two redundant re-derivations for the two
comparisons.

### Proposed learning

None -- a natural extension of the `%20` idiom already confirmed in
`StyleCue03`'s report, this time keeping the remainder value itself.

## Naming

**Tier B.** `StyleCue10` is row +0x02C of `gStyleCueCallbacks` (`tools/classtable.py 0x800874B0`, round 73). `TryStartStyleCue` (class_3bb8c_k.c) installs `gStyleCueCallbacks[sub->countSign]` as `SoundCueSet::callback` via `InitSoundCueSet` (PlacementGridVabSound.c) -- the same per-tag sound-cue-callback slot `gEntityMoodHandlerTable`'s `MoodCueNN` occupants hold for `Entity` (`Entity__MoodCueNN` match reports). The `StyleCueNN` numbering follows table row order, same convention as `MoodCueNN`. Mechanics are established (a per-tag callback that reads `self->kind` and writes a handful of numeric fields, calling `ComputeStyleCueFalloff` first); which dream/style object or which field means what in the running game is not, so the specific `kind` branches and the numeric literals they write stay unnamed.

## Track 6 (2026-09-26, round 92, alpha): `set` is a SoundCueSet

class_3bb8c_r.c's `StyleCueParam` used to type both parameters of every
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
