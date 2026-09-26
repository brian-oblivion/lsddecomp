# StageMap__SplitChunkIndex

> Renamed from `StageMap__ComputeDivisorSplit` on 2026-09-26 (tools/rename.py). Address 0x8004c368.

> Renamed from `Class866E8__ComputeDivisorSplit` on 2026-09-26 (tools/rename.py). Address 0x8004c368.

> Renamed from `func_8004C368` on 2026-09-24 (tools/rename.py). Address 0x8004c368.

**Unit:** class_3bb8c · **Size:** 34 words · **Status:** MATCHED (first attempt).

## Result

```c
void StageMap__SplitChunkIndex(Obj866E8 *self, u8 *out, s32 val) {
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
`StageMap__ComputeNeighbourMask` and `ComputeCellWorldOffsets`/`StageMap__ComputeCellOffsets`'s call site later in the
same round.

### Proposed learning

None new -- confirms the existing "div by a non-constant expands to a
several-instruction sequence with a zero/overflow check; write plain `%`/`/`"
guidance in CLAUDE.md verbatim, including the double-reload-not-cached shape
for a value read via two independent statements with no intervening
assignment to a local.

## Naming

Round 78 (track 3, naming pass, bravo).

| symbol | name | tier | evidence |
| --- | --- | --- | --- |
| `func_8004C368` | `StageMap__SplitChunkIndex` | A | Takes `self` first (a method). Pure two-line computation: `out[0] = val % self->unk68->divisor; out[1] = val / self->unk68->divisor;` -- a mod/div split against the object's own divisor, no other side effect. Mechanics-only name, tier A by the "getter/clamp" clause (a pure, unconditional computation whose mechanics fully describe it). |
