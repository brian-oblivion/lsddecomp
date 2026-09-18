> Renamed from `func_80040A88` on 2026-09-18 (tools/rename.py). Address 0x80040a88.

# Obj6EAC0__FinishConstruct — MATCHED (24/24 words)

Unit: `src/code_2cc8c_f.c`. First attempt.

```c
void Obj6EAC0__FinishConstruct(Obj6EAC0 *self, s32 a1) {
    self->methods->slotD4(self, 7);
    self->methods->slotCC(self, a1);
}
```

Derived-table occupant of `Obj6EAC0Methods::slot0x40`. Both calls
dispatch through `self->methods` (virtual, resolves to whichever
class's slot is actually installed) rather than by literal name --
matches the established "a base constructor can dispatch through
self->methods immediately" idiom. `slotD4`'s `s32` return is discarded
as a bare statement.

### Proposed learning

None beyond what's already recorded.
