# TextRow__SetCellPitch — MATCHED (2/2 words)

> Renamed from `Obj6EAC0__SetChildPitch` on 2026-09-26 (tools/rename.py). Address 0x80040fa8.

> Renamed from `func_80040FA8` on 2026-09-18 (tools/rename.py). Address 0x80040fa8.

Unit: `src/code_2cc8c_f.c`. Trivial setter, first attempt.

```c
void TextRow__SetCellPitch(Obj6EAC0 *self, s32 a1) {
    self->unkB0 = a1;
}
```

Derived-table occupant of `Obj6EAC0Methods::slot0xD4` (derived-only
slot, no base equivalent). Called by `TextRow__Reset` (this unit) as
`self->methods->slotD4(self, 7)`.

### Proposed learning

None beyond what's already recorded.

## Naming

Round 54 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_80040FA8` | `TextRow__SetCellPitch` | A |

**Evidence.** A pure leaf setter (tier A): `self->childPitch = a1`, one
field, no other logic. `childPitch` (renamed from `unkB0`) is the value
`TextRow__SetPosition`/`TextRow__AttachToParent` add into the
running position cursor once per child -- i.e. the per-child advance/
spacing distance, confirmed by those two callers (this same unit).
