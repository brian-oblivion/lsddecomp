# Obj865C8__ResetState

> Renamed from `func_80049A14` on 2026-09-23 (tools/rename.py). Address 0x80049a14.

**Unit:** class_39e08 · **Size:** 2 words (0x8 bytes) · **Status:** MATCHED (2/2 words)

## What it does

Method-table slot +0x040 of the class whose vtable is `D_800865C8` (resolved
with `tools/classtable.py 0x800865C8`). Zeroes one field of the instance.

## Derivation

```
jr    $ra
 sw   $zero, 0x3C($a0)
```

A one-instruction leaf, the store living in the branch delay slot. Written as:

```c
void Obj865C8__ResetState(Obj865C8 *self) {
    self->unk3C = 0;
}
```

`Obj865C8` and its field `unk3C` are established in the new unit header
`include/class_39e08.h`, added this round.

## Proposed learning

None beyond what's already documented for this residue-free shape.

## Naming

`Obj865C8__ResetState` -- tier A. Pure one-line setter (`state = 0`); mechanics are its purpose. Occupies the class's own +0x040 override; the field it resets is used as a 0-3 state code by `Obj865C8__AdvanceState`/`Obj865C8__OnTag2Notify`.
