# Class866E8__ComputeDivisorSplit

> Renamed from `func_8004C368` on 2026-09-24 (tools/rename.py). Address 0x8004c368.

**Unit:** class_3bb8c · **Size:** 34 words · **Status:** MATCHED (first attempt).

## Result

```c
void Class866E8__ComputeDivisorSplit(Obj866E8 *self, u8 *out, s32 val) {
    out[0] = val % self->unk68->divisor;
    out[1] = val / self->unk68->divisor;
}
```

## Derivation

Retail's body is two near-identical `div`/trap-check sequences (the maspsx
`--expand-div` expansion for a runtime, non-constant divisor: zero-check
`break 7`, `INT_MIN/-1` overflow check `break 6`), one reading `mfhi`
(remainder) and one reading `mflo` (quotient), each reloading
`self->unk68->divisor` fresh from memory. Writing plain `%` and `/` lets the
compiler emit the expansion itself; no manual reconstruction of the trap
sequence was needed.

The double reload of `self->unk68->divisor` (once per statement, not cached
into a local) matches retail exactly -- two separate C statements, two
separate field reads, no explicit sharing.

This is also the function that pinned down `Unk68Struct`'s first field
(`divisor`, a signed halfword at `+0x000`) -- corroborated independently by
`Class866E8__ComputeRateFlags` and `ComputeCellWorldOffsets`/`Class866E8__ComputeCellOffsets`'s call site later in the
same round.

### Proposed learning

None new -- confirms the existing "div by a non-constant expands to a
several-instruction sequence with a zero/overflow check; write plain `%`/`/`"
guidance in CLAUDE.md verbatim, including the double-reload-not-cached shape
for a value read via two independent statements with no intervening
assignment to a local.
