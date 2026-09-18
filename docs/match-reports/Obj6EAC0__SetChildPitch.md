> Renamed from `func_80040FA8` on 2026-09-18 (tools/rename.py). Address 0x80040fa8.

# Obj6EAC0__SetChildPitch — MATCHED (2/2 words)

Unit: `src/code_2cc8c_f.c`. Trivial setter, first attempt.

```c
void Obj6EAC0__SetChildPitch(Obj6EAC0 *self, s32 a1) {
    self->unkB0 = a1;
}
```

Derived-table occupant of `Obj6EAC0Methods::slot0xD4` (derived-only
slot, no base equivalent). Called by `func_80040A88` (this unit) as
`self->methods->slotD4(self, 7)`.

### Proposed learning

None beyond what's already recorded.
