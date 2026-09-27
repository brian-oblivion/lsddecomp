# StyleCue05 -- MATCHED (56/56 words)

> Renamed from `func_80055DB4` on 2026-09-23 (tools/rename.py). Address 0x80055db4.

Unit: `class_3bb8c_r` (round 17 continuation). Slot occupant #6 of
`gStyleCueCallbacks`, and the most structurally involved of the fourteen: a
four-way `if`/`else-if` mixing a discrete check, a range+modulo
condition, an unsigned-range trick, and a plain threshold, with a
field that both READS and WRITES `self->kind` inside one arm.

## Final source

```c
void StyleCue05(StyleCueParam *ctx, StyleCueParam *self) {
    self->falloff = ComputeStyleCueFalloff(ctx);
    if (self->kind == 0) {
        self->unk1C = 0x1E;
        self->unk20 = -1;
    } else if (self->kind < 0x32 && self->kind % 5 == 4) {
        self->unk30 = 0x1E;
        self->unk34 = 0;
        self->unk38 = self->unk38 - self->kind * 2;
        self->unk3C = self->unk38;
    } else if ((u32)(self->kind - 0x65) < 9) {
        self->unk44 = 0xD;
        self->unk48 = 1;
    } else if (self->kind >= 0xC9) {
        self->kind = -1;
    }
}
```

## Derivation

- **No cached `kind` local -- every branch re-reads `self->kind`
  directly.** Unlike the discrete-`kind` siblings (`StyleCue00` etc.),
  this function's disassembly reloads `self->kind` from memory at the
  START of the range+modulo arm, again inside that arm (for the
  `unk38 -= kind*2` arithmetic), and again in the unsigned-range arm --
  even though nothing between those reads writes `self->kind` itself.
  Caching into a local `kind` variable (as the simpler siblings do)
  compiled to ONE load reused everywhere, one word too few. Writing
  `self->kind` directly at each site reproduces the reloads. The likely
  cause: an intervening STORE to a DIFFERENT field of the same struct
  (`self->unk34`, `self->unk38`) makes this compiler's weak alias
  analysis reload other fields conservatively on the next read, even
  though the actual field read (`kind`) was never written -- the same
  family of caution documented for `DreamSys__SaveLinkSnapshot` in
  `DECOMPILATION_LEARNINGS.md`, seen here from the "do NOT cache" side
  rather than the "DO cache" side.
- **`(u32)(x - 0x65) < 9`** is the unsigned-range-check idiom for
  `x >= 0x65 && x < 0x6E`, transcribed directly from the disassembly's
  own `addiu`+`sltiu` pair rather than reconstructed as two separate
  comparisons (which would cost an extra instruction).
- **`self->kind % 5 == 4`**, not a hand-expanded remainder-4 test,
  matches the `mult`/`mfhi`/recombination chain for divisor 5 (same
  magic+shift already confirmed in `StyleCue04`'s report).

### Proposed learning

- **A field that gets reloaded even though nothing wrote it directly can
  still need "do not cache" treatment, if a DIFFERENT field of the same
  struct was written in between.** This is the mirror image of
  `DreamSys__SaveLinkSnapshot`'s already-documented "an intervening whole-struct
  assignment defeats CSE of a pointer field" entry -- here the
  intervening write is an ordinary SCALAR field store (`self->unk34 = 0`,
  `self->unk38 = ...`), not an aggregate assignment, and it still
  triggers the same conservative reload of a DIFFERENT field
  (`self->kind`) on the next read. Worth checking for on any function
  with more than one struct-field write before a later read of an
  UNRELATED field of that same struct.

## Naming

**Tier B.** `StyleCue05` is row +0x018 of `gStyleCueCallbacks` (`tools/classtable.py 0x800874B0`, round 73). `TryStartStyleCue` (class_3bb8c_n.c) installs `gStyleCueCallbacks[sub->countSign]` as `SoundCueSet::callback` via `InitSoundCueSet` (PlacementGridVabSound.c) -- the same per-tag sound-cue-callback slot `gEntityMoodHandlerTable`'s `MoodCueNN` occupants hold for `Entity` (`Entity__MoodCueNN` match reports). The `StyleCueNN` numbering follows table row order, same convention as `MoodCueNN`. Mechanics are established (a per-tag callback that reads `self->kind` and writes a handful of numeric fields, calling `ComputeStyleCueFalloff` first); which dream/style object or which field means what in the running game is not, so the specific `kind` branches and the numeric literals they write stay unnamed.

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
- `(u32)(set->tick - 0x65) < 9` is now `set->tick >= 101 && set->tick < 110`,
  byte-exact (56/56 and the whole image, measured this round): GCC 2.6.3
  folds the two comparisons into the same `addiu`/`sltiu` pair itself. The
  Derivation above says the two-comparison form costs an instruction; that
  was never measured and is wrong.
