# StyleCue06 -- MATCHED (23/23 words)

> Renamed from `func_80055E94` on 2026-09-23 (tools/rename.py). Address 0x80055e94.

Unit: `dream_scene` (round 17 continuation). Slot occupant #7 of
`sStyleCueCallbacks`. Same two-way discrete-`kind` shape as `StyleCue01`/
`StyleCue02`.

## Final source

```c
void StyleCue06(StyleCueParam *ctx, StyleCueParam *self) {
    s32 kind;

    self->falloff = ComputeStyleCueFalloff(ctx);
    kind = self->kind;
    if (kind == 0) {
        self->unk1C = 7;
        self->unk20 = 2;
    } else if (kind >= 0x1B) {
        self->kind = -1;
    }
}
```

## Derivation

Direct transcription, matched first try.

### Proposed learning

None -- see `ComputeStyleCueFalloff`'s and `StyleCue07`'s reports.

## Naming

**Tier B.** `StyleCue06` is row +0x01C of `sStyleCueCallbacks` (`tools/classtable.py 0x800874B0`, round 73). `TryStartStyleCue` (dream_scene.c) installs `sStyleCueCallbacks[sub->countSign]` as `SoundCueSet::callback` via `InitSoundCueSet` (vab_sound.c) -- the same per-tag sound-cue-callback slot `gEntityMoodHandlerTable`'s `MoodCueNN` occupants hold for `Entity` (`Entity__MoodCueNN` match reports). The `StyleCueNN` numbering follows table row order, same convention as `MoodCueNN`. Mechanics are established (a per-tag callback that reads `self->kind` and writes a handful of numeric fields, calling `ComputeStyleCueFalloff` first); which dream/style object or which field means what in the running game is not, so the specific `kind` branches and the numeric literals they write stay unnamed.

## Track 6 (2026-09-26, round 92, alpha): `set` is a SoundCueSet

dream_scene.c's `StyleCueParam` used to type both parameters of every
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
- The first parameter (`ctx`) is the owner, dream_scene.c's
  `StyleCueSlot`: TryStartStyleCue passes the slot as InitSoundCueSet's
  owner and `&slot->cueSet` (+0x14) as the set. So `StyleCueParam` is now a
  local view of StyleCueSlot: `methods` (+0x00) is the claimed record,
  renamed `entry`, whose +0x06 `tag` is dream_scene's `countSign`;
  `falloff` (+0x10) is `lastDist`; and `unk28` (+0x28) is
  `cueSet.attenuationSteps` (+0x14 + 0x14). ComputeStyleCueFalloff therefore
  scales the slot's last distance into 0..attenuationSteps against the cue's
  distance limit, as Entity__GetProximityRatio does for Entity.

Locals `kind` became `tick`. Zero bytes.

## Track 7 (2026-09-27, round 96, charlie)

- Every literal is decimal (VAB program numbers, volumes and tick counts are
  counts, not masks). None is named: a program number's sound is not
  established, and a name like `PROGRAM_30` would only restate it. Zero bytes.
