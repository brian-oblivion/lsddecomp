# StyleCue08 -- MATCHED (29/29 words)

> Renamed from `func_80055F74` on 2026-09-23 (tools/rename.py). Address 0x80055f74.

Unit: `class_3bb8c_r` (round 17 continuation). Slot occupant #9 of
`gStyleCueCallbacks`. A single `% 20` divisibility gate.

## Final source

```c
void StyleCue08(StyleCueParam *ctx, StyleCueParam *self) {
    self->falloff = ComputeStyleCueFalloff(ctx);
    if (self->kind % 20 == 0) {
        self->unk1C = 9;
        self->unk20 = 0;
        self->unk24 = 0x40;
        self->unk28 = 0x40;
    }
}
```

## Derivation

Same `%20` magic+shift as `StyleCue03`'s own first block (already
confirmed there: `sll,addu,sll` recombination = `q*20`). Direct
transcription, matched first try -- this function is the simplest of the
divisibility-gated siblings (one condition, four flat writes).

### Proposed learning

None -- see `StyleCue03`'s report for the divisor-derivation caution.

## Naming

**Tier B.** `StyleCue08` is row +0x024 of `gStyleCueCallbacks` (`tools/classtable.py 0x800874B0`, round 73). `TryStartStyleCue` (class_3bb8c_n.c) installs `gStyleCueCallbacks[sub->countSign]` as `SoundCueSet::callback` via `InitSoundCueSet` (code_179d8_e.c) -- the same per-tag sound-cue-callback slot `gEntityMoodHandlerTable`'s `MoodCueNN` occupants hold for `Entity` (`Entity__MoodCueNN` match reports). The `StyleCueNN` numbering follows table row order, same convention as `MoodCueNN`. Mechanics are established (a per-tag callback that reads `self->kind` and writes a handful of numeric fields, calling `ComputeStyleCueFalloff` first); which dream/style object or which field means what in the running game is not, so the specific `kind` branches and the numeric literals they write stay unnamed.

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
- The first parameter (`ctx`) is the owner, class_3bb8c_n.c's
  `StyleCueSlot`: TryStartStyleCue passes the slot as InitSoundCueSet's
  owner and `&slot->cueSet` (+0x14) as the set. So `StyleCueParam` is now a
  local view of StyleCueSlot: `methods` (+0x00) is the claimed record,
  renamed `entry`, whose +0x06 `tag` is class_3bb8c_n's `countSign`;
  `falloff` (+0x10) is `lastDist`; and `unk28` (+0x28) is
  `cueSet.attenuationSteps` (+0x14 + 0x14). ComputeStyleCueFalloff therefore
  scales the slot's last distance into 0..attenuationSteps against the cue's
  distance limit, as Entity__GetProximityRatio does for Entity.

Locals `kind` became `tick`. Zero bytes.

## Track 7 (2026-09-27, round 96, charlie)

- Every literal is decimal (VAB program numbers, volumes and tick counts are
  counts, not masks). None is named: a program number's sound is not
  established, and a name like `PROGRAM_30` would only restate it. Zero bytes.
