# StageMap__ResetCellScale — MATCHED (14/14 words)

> Renamed from `StageMap__ResetChildRate` on 2026-09-26 (tools/rename.py). Address 0x8004d108.

> Renamed from `Class866E8__ResetChildRate` on 2026-09-26 (tools/rename.py). Address 0x8004d108.

> Renamed from `func_8004D108` on 2026-09-24 (tools/rename.py). Address 0x8004d108.

Sibling of `StageMap__AddScaleStepToCell` (see that report for how the true call chain —
`StageMap__ForEachElem` forwards to `StageMap__ForEachEntryChild`, which does the actual
`jalr` — was resolved). Same `Unk10ChildMethods_3bb8c_b::slot48` slot,
different literal arguments.

## Disassembly

```
addiu $sp, $sp, -0x18
move  $a0, $a1              ; a0 = item (original arg0, "self", is discarded entirely)
sw    $ra, 0x10($sp)
lw    $v0, 0x0($a0)         ; v0 = item->methods
lw    $v0, 0x48($v0)        ; v0 = item->methods->slot48
lui   $a2, %hi(D_800869CC)
addiu $a2, $a2, %lo(D_800869CC)
jalr  $v0
 ori  $a1, $zero, 0x1
...epilogue
```

Notable: the original first parameter (`self`) is never referenced after
the top-of-function register shuffle overwrites `a0` with `item` — this
function genuinely ignores its own `self` argument, unlike its sibling
`StageMap__AddScaleStepToCell` which uses it (`self->unk1E4`). Confirmed real, not a
missing-parameter bug, by cross-checking the only caller
(`StageMap__EndScaleRamp`, which passes this function's address to
`StageMap__ForEachElem` exactly like `StageMap__StepScaleRamp` passes `StageMap__AddScaleStepToCell`'s —
same call shape, same two-parameter signature required by the eventual
`StageMap__ForEachEntryChild` dispatcher).

## Final C

```c
void StageMap__ResetCellScale(Obj866E8 *self, Unk10ChildObj_3bb8c_b *item) {
    item->methods->slot48(item, 1, D_800869CC);
}
```

## New struct/global knowledge

- `extern s32 D_800869CC[3];` — a 3-word (12-byte) data block, address-of
  only. Never dereferenced in this unit, so left untyped beyond its size.

## Attempts

1 (matched on first attempt).

### Proposed learning

None new beyond `StageMap__AddScaleStepToCell`'s (same call-chain-tracing lesson).

## Naming

**Tier B.** Not a vtable slot -- the `StageMap__EndScaleRamp` callback
sibling of `StageMap__AddScaleStepToCell`. Body: `item->methods->slot48(
item, 1, &D_800869CC)`, the constant "off" entry rather than the parent's
own `rateEntry`. Named to read as the inverse of `ApplyRateToChild`.
