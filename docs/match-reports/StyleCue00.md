# StyleCue00 -- MATCHED (34/34 words)

> Renamed from `func_80055A88` on 2026-09-23 (tools/rename.py). Address 0x80055a88.

Unit: `class_3bb8c_r` (round 17 continuation). Slot occupant #1 of
`gStyleCueCallbacks` (14 slots, header word 0 -- see the unit's own file banner
for why this is NOT a BasicClass override despite the matching slot
count). Calls the shared helper `ComputeStyleCueFalloff`, stores its result, then
dispatches on `self->unk4` ("kind") to fill in a handful of fields.

## Final source

```c
void StyleCue00(ParamObj *ctx, ParamObj *self) {
    s32 kind;

    self->unk10 = ComputeStyleCueFalloff(ctx);
    kind = self->unk4;
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
        self->unk4 = -1;
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
