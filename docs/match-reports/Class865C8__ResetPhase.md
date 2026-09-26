# Class865C8__ResetPhase

> Renamed from `Class865C8__ResetState` on 2026-09-26 (tools/rename.py). Address 0x80049a14.

> Renamed from `Obj865C8__ResetState` on 2026-09-26 (tools/rename.py). Address 0x80049a14.

> Renamed from `func_80049A14` on 2026-09-23 (tools/rename.py). Address 0x80049a14.

**Unit:** class_39e08 · **Size:** 2 words (0x8 bytes) · **Status:** MATCHED (2/2 words)

## What it does

Method-table slot +0x040 of the class whose vtable is `gClass865C8Methods` (resolved
with `tools/classtable.py 0x800865C8`). Zeroes one field of the instance.

## Derivation

```
jr    $ra
 sw   $zero, 0x3C($a0)
```

A one-instruction leaf, the store living in the branch delay slot. Written as:

```c
void Class865C8__ResetPhase(Obj865C8 *self) {
    self->unk3C = 0;
}
```

`Obj865C8` and its field `unk3C` are established in the new unit header
`include/class_39e08.h`, added this round.

## Proposed learning

None beyond what's already documented for this residue-free shape.

## Naming

`Class865C8__ResetPhase` -- tier A. Pure one-line setter (`state = 0`); mechanics are its purpose. Occupies the class's own +0x040 override; the field it resets is used as a 0-3 state code by `Class865C8__AdvancePhase`/`Class865C8__OnObjMNotify`.

## Track 4 (2026-09-26, round 88, Class865C8)

The class (table D_800865C8, id 0x1F230, Class86668's subclass) is unified as Class865C8 in include/Class865C8.h; the Obj865C8/Class865C8Methods views in class_39e08.h are gone. Renamed from Obj865C8__ResetState. The field is `phase` (+0x03C): IntermediateBase already names +0x020 `state`, a different field. This is the +0x040 resetCounters override and clears only phase. Tier A.
