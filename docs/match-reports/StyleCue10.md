# StyleCue10 -- MATCHED (36/36 words)

> Renamed from `func_80056054` on 2026-09-23 (tools/rename.py). Address 0x80056054.

Unit: `class_3bb8c_r` (round 17 continuation). Slot occupant #11 of
`gStyleCueCallbacks`. Dispatches on the ACTUAL REMAINDER of `self->unk4 % 20`
(not just divisibility) -- the first sibling in this family to keep the
remainder value itself rather than only testing it against zero.

## Final source

```c
void StyleCue10(ParamObj *ctx, ParamObj *self) {
    s32 rem;

    self->unk10 = ComputeStyleCueFalloff(ctx);
    rem = self->unk4 % 20;
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

Retail computes `q = unk4/20` (same `%20` magic as `StyleCue03`) and
then `rem = unk4 - q*20` explicitly (the SUBTRACTION result itself is
kept and compared against 1 and 16, rather than the earlier siblings'
pattern of comparing the dividend against `q*N` directly). Writing
`self->unk4 % 20` and caching it in a `rem` local reproduces this --
GCC's ordinary `%` codegen computes exactly this quotient-then-subtract
sequence, and caching it avoids two redundant re-derivations for the two
comparisons.

### Proposed learning

None -- a natural extension of the `%20` idiom already confirmed in
`StyleCue03`'s report, this time keeping the remainder value itself.
