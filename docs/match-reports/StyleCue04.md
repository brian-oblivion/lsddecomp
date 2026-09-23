# StyleCue04 -- MATCHED (67/67 words)

> Renamed from `func_80055CA8` on 2026-09-23 (tools/rename.py). Address 0x80055ca8.

Unit: `class_3bb8c_r` (round 17 continuation). Slot occupant #5 of
`gStyleCueCallbacks`. Three independent divisibility checks (`% 3`, `% 5`, `% 7`)
on `self->unk4`.

## Final source

```c
void StyleCue04(ParamObj *ctx, ParamObj *self) {
    self->unk10 = ComputeStyleCueFalloff(ctx);
    if (self->unk4 % 3 == 0) {
        self->unk1C = 0x1E;
        self->unk20 = 0;
    }
    if (self->unk4 % 5 == 0) {
        self->unk30 = 0x1E;
        self->unk34 = 0;
        self->unk38 = 0x18;
        self->unk3C = 0x18;
    }
    if (self->unk4 % 7 == 0) {
        self->unk1C = 0x1E;
        self->unk20 = 0;
    }
    self->unk44 = 6;
    self->unk48 = 1;
    self->unk4C = 0x2A;
    self->unk50 = 0xA;
}
```

## Derivation

Three separate `magic`/`sra`/`mfhi` chains, each recombined to confirm
the true divisor directly (see `StyleCue03`'s report for why this must
be done arithmetically rather than by shift-alone heuristic): `sll1,addu`
= `q*3`; `sll2,addu` = `q*5`; `sll3,subu` (`q*8-q`) = `q*7`. The `%3` and
`%7` checks write the SAME two fields (`unk1C`, `unk20`) with the SAME
values -- a genuine "either condition sets the same default" shape,
confirmed by both write blocks being byte-identical in the disassembly.
`%5` writes a disjoint set of fields. All three checks are independent
`if`s (not `else if`), matching `StyleCue03`'s established pattern.

### Proposed learning

None beyond what's already written up in `StyleCue03`'s report.
