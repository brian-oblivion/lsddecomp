# StyleCue00 -- MATCHED (34/34 words)

> Renamed from `func_80055A88` on 2026-09-23 (tools/rename.py). Address 0x80055a88.

Unit: `class_3bb8c_r` (round 17 continuation). Slot occupant #1 of
`gStyleCueCallbacks` (14 slots, header word 0 -- see the unit's own file banner
for why this is NOT a BasicClass override despite the matching slot
count). Calls the shared helper `ComputeStyleCueFalloff`, stores its result, then
dispatches on `self->kind` ("kind") to fill in a handful of fields.

## Final source

```c
void StyleCue00(StyleCueParam *ctx, StyleCueParam *self) {
    s32 kind;

    self->falloff = ComputeStyleCueFalloff(ctx);
    kind = self->kind;
    if (kind == 0) {
        self->unk1C = 7;
        self->unk20 = 0;
    } else if (kind == 2) {
        self->unk30 = 7;
        self->unk34 = 0;
    } else if (kind == 5) {
        self->unk44 = 7;
        self->unk48 = 0;
    } else if (kind >= 8) {
        self->kind = -1;
    }
}
```

## Derivation

Straight transcription of the disassembly's if/else-if chain: `kind` is
loaded once into a local (matching retail's own single load, reused
across all four comparisons with no reload -- see `StyleCue07`'s report
for a case where the same field genuinely does need reloading after an
intervening write). Two-argument shape (`ctx`, `self`) established from
`ComputeStyleCueFalloff`'s own call (`ctx` forwarded unchanged; `self` is the
function's own second parameter, saved into `$s0` and used for every
field write).

### Proposed learning

None -- the discriminating levers for this whole 14-function family are
written up once, in `ComputeStyleCueFalloff`'s and `StyleCue07`'s reports.

## Naming

**Tier B.** `StyleCue00` is row +0x004 of `gStyleCueCallbacks` (`tools/classtable.py 0x800874B0`, round 73). `TryStartStyleCue` (class_3bb8c_n.c) installs `gStyleCueCallbacks[sub->countSign]` as `SoundCueSet::callback` via `InitSoundCueSet` (code_179d8_d.c) -- the same per-tag sound-cue-callback slot `gEntityMoodHandlerTable`'s `MoodCueNN` occupants hold for `Entity` (`Entity__MoodCueNN` match reports). The `StyleCueNN` numbering follows table row order, same convention as `MoodCueNN`. Mechanics are established (a per-tag callback that reads `self->kind` and writes a handful of numeric fields, calling `ComputeStyleCueFalloff` first); which dream/style object or which field means what in the running game is not, so the specific `kind` branches and the numeric literals they write stay unnamed.

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

The unit banner of `src/class_3bb8c_r.c` was rewritten to say what the file
holds (and now names IsStyleVariantEven). The old one, verbatim:

```c
/*
 * class_3bb8c_r -- 0x46288..0x46D20 (vram 0x80055A88..0x80056520), the tail
 * of the `class_3bb8c_n` remainder (carved round 17). All 21 functions are
 * MATCHED. Two unrelated classes share the slice, cut at ROM addresses
 * rather than at a class boundary (tools/classtable.py, round 17):
 *
 *  - StyleCue00..StyleCue13, the complete 14-slot table `gStyleCueCallbacks`,
 *    and their helper ComputeStyleCueFalloff. They are SoundCueSet
 *    callbacks (include/SoundCueSet.h): TryStartStyleCue (class_3bb8c_n.c)
 *    starts a style-cue slot's embedded set with the claimed record's cue
 *    index as the tag and that row of the table as the callback, as
 *    Entity does with gEntityMoodHandlerTable's MoodCueNN handlers. Each
 *    tick a callback sets the set's attenuation from the slot's distance
 *    (ComputeStyleCueFalloff) and, on the ticks its pattern selects,
 *    requests programs on the three voices; most restart the pattern by setting
 *    `tick` to -1 once it passes a limit.
 *  - StyleEffect (include/StyleEffect.h), the Actor subclass the style
 *    layer keeps at an offset from its target: this unit supplies its slot
 *    occupants (StyleEffect__StyleEffect/__Finalize/__SetParams/__Update)
 *    and the `New_StyleEffect` allocator; its per-kind work is in
 *    class_3bb8c_s.c and class_3bb8c_o.c.
 *
 * Named round 73 (charlie); tiers and evidence in each function's match
 * report.
 */
```
