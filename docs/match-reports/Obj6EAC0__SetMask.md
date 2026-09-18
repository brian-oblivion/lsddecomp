> Renamed from `func_800408A8` on 2026-09-18 (tools/rename.py). Address 0x800408a8.

# Obj6EAC0__SetMask — MATCHED (5/5 words)

Unit: `src/code_2cc8c_f.c`. First attempt.

```c
s32 Obj6EAC0__SetMask(Obj6EAC0 *self, s32 a1) {
    return self->unk68 = (1 << a1) - 1;
}
```

`$v0` already holds the computed value at the point of `jr $ra` in
retail, so `return self->unk68 = expr;` (reusing the "set a value,
return the same value" idiom already documented for other setters)
reproduces it with no extra `move`.

Base-table occupant of `Obj6EAC0Methods::slot0xCC`; the derived table's
slot0xCC is `func_80040F28` (this unit, also queued).

### Proposed learning

None beyond what's already recorded.
