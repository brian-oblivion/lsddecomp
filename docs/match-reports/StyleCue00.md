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

**Tier B.** `StyleCue00` is row +0x004 of `gStyleCueCallbacks` (`tools/classtable.py 0x800874B0`, round 73). `TryStartStyleCue` (class_3bb8c_n.c) installs `gStyleCueCallbacks[sub->countSign]` as `SoundCueSet::callback` via `InitSoundCueSet` (code_179d8_e.c) -- the same per-tag sound-cue-callback slot `gEntityMoodHandlerTable`'s `MoodCueNN` occupants hold for `Entity` (`Entity__MoodCueNN` match reports). The `StyleCueNN` numbering follows table row order, same convention as `MoodCueNN`. Mechanics are established (a per-tag callback that reads `self->kind` and writes a handful of numeric fields, calling `ComputeStyleCueFalloff` first); which dream/style object or which field means what in the running game is not, so the specific `kind` branches and the numeric literals they write stay unnamed.
