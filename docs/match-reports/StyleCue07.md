# StyleCue07 -- MATCHED (33/33 words)

> Renamed from `func_80055EF0` on 2026-09-23 (tools/rename.py). Address 0x80055ef0.

Unit: `class_3bb8c_r` (round 17 continuation). Slot occupant #8 of
`gStyleCueCallbacks`. Three-way discrete-`kind` dispatch, and the function whose
derivation errors were caught and fixed during this round -- documented
here in detail since the same two mistakes are easy to repeat on any
sibling.

## Final source

```c
void StyleCue07(StyleCueParam *ctx, StyleCueParam *self) {
    s32 kind;

    self->falloff = ComputeStyleCueFalloff(ctx);
    kind = self->kind;
    if (kind == 0) {
        self->unk1C = 0x14;
        self->unk20 = 1;
    } else if (kind == 3) {
        self->unk20 = 2;
        self->unk24 = 0x18;
        self->unk1C = kind;
        self->unk28 = 0x14;
    } else if (kind >= 0x33) {
        self->kind = -1;
    }
}
```

## Derivation, with the two mistakes that were caught

1. **Misread field offsets in the `kind==3` arm on the first transcription
   pass.** The raw disassembly for that arm is:
   ```
   ori v0,zero,0x2
   sw   v0,0x20(s0)      # unk20 = 2
   ori  v0,zero,0x18
   sw   v0,0x24(s0)      # unk24 = 0x18   <- first pass read this as unk28
   ori  v0,zero,0x14
   sw   v1,0x1c(s0)      # unk1c = v1 (kind)
   j    END
    sw  v0,0x28(s0)      # unk28 = 0x14 (v0 reloaded to 0x14 first!)
   ```
   The first pass wrote `self->unk28 = 0x18;` and dropped `unk24`
   entirely -- plausible-LOOKING C (compiled clean, ran, "looked" like a
   parameter table) but wrong on two separate fields AND missing a third
   write. The build/funcdiff loop caught it immediately as a genuine
   word-count/content mismatch (17/33, not a near-miss), not something
   that would have passed a casual read-through. **The fix was to
   re-read the raw `.s` file's own `sw` offsets one instruction at a time
   rather than trust an earlier paraphrase of them** -- this is the
   single most effective check across this whole 14-function family.
2. **The `kind==3` arm needs `self->unk1C = kind;` (a cached local),
   not `self->unk1C = self->kind;` (a fresh field read).** The
   disassembly reuses the SAME register (`$v1`, still holding the value
   loaded for the `kind==3` comparison) for the `unk1C` store -- no
   reload. Writing `self->unk1C = self->kind;` directly (without a
   `kind` local) triggered an unwanted extra `lw` reload, costing one
   word, for the same "intervening scalar store defeats the next
   unrelated-field read" reason documented in `StyleCue05`'s report.
   Introducing the `kind` local (already used by the SIMPLER two-way
   siblings) fixed it, closing 17/33 -> 33/33 in one step.

### Proposed learning

- **Re-verify `sw`/`ori` OFFSETS and VALUES from the raw `.s` file
  directly, one instruction at a time, for every arm of a
  multi-field-write function -- do not trust an earlier paraphrase, even
  your own, especially across a family of near-identical functions where
  it's easy to let one arm's derivation "rhyme" with a sibling's instead
  of being independently re-read.** This function's own mis-transcription
  (`unk28=0x18` instead of the correct `unk24=0x18`/`unk28=0x14` pair)
  compiled clean and only surfaced as a genuine word-count mismatch in
  `funcdiff`, not as a subtle residue -- which is the GOOD case (the
  oracle catches it), but it burned an avoidable iteration that a
  slower, line-by-line re-read of the `.s` file would have caught before
  ever writing the C.

## Naming

**Tier B.** `StyleCue07` is row +0x020 of `gStyleCueCallbacks` (`tools/classtable.py 0x800874B0`, round 73). `TryStartStyleCue` (class_3bb8c_k.c) installs `gStyleCueCallbacks[sub->countSign]` as `SoundCueSet::callback` via `InitSoundCueSet` (PlacementGridVabSound.c) -- the same per-tag sound-cue-callback slot `gEntityMoodHandlerTable`'s `MoodCueNN` occupants hold for `Entity` (`Entity__MoodCueNN` match reports). The `StyleCueNN` numbering follows table row order, same convention as `MoodCueNN`. Mechanics are established (a per-tag callback that reads `self->kind` and writes a handful of numeric fields, calling `ComputeStyleCueFalloff` first); which dream/style object or which field means what in the running game is not, so the specific `kind` branches and the numeric literals they write stay unnamed.

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

## Track 7 (2026-09-27, round 96, charlie)

- Every literal is decimal (VAB program numbers, volumes and tick counts are
  counts, not masks). None is named: a program number's sound is not
  established, and a name like `PROGRAM_30` would only restate it. Zero bytes.
- The `tick == 3` arm writes `program = 3` rather than `program = tick`:
  byte-exact (33/33 and the whole image, measured this round). GCC stores
  the register already holding the compared tick either way; what the
  Derivation's point 2 measured was a fresh `set->tick` field read, which
  still costs the reload, not a literal.
