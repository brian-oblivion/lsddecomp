> Renamed from `func_80040FB0` on 2026-09-18 (tools/rename.py). Address 0x80040fb0.

# Obj6EAC0__GetDerivedMethods — MATCHED (4/4 words)

Unit: `src/code_2cc8c_f.c`. First attempt.

```c
Obj6EAC0Methods *Obj6EAC0__GetDerivedMethods(void) {
    return &D_8006EB90;
}
```

The override-table twin of `Obj6EAC0__GetBaseMethods` (returns `&D_8006EAC0`).
Called by `New_Obj6EAC0` (this unit, the class's `New_X`-shaped
allocator) to fetch the constructor at `slot0x08`.

### Proposed learning

None beyond what's already recorded.
