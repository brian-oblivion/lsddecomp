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
slot0xCC is `Obj6EAC0__SetText` (this unit, also queued).

### Proposed learning

None beyond what's already recorded.

## Naming

Round 54 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_800408A8` | `Obj6EAC0__SetMask` | A |

**Evidence.** A pure leaf whose mechanics ARE its purpose (tier A):
`self->mask = (1 << a1) - 1`, a bitmask-of-the-low-`a1`-bits computation
and store, with no other logic. Renamed the field too (`unk68` -> `mask`,
this unit's own exclusive field -- see the struct-field rename commit).
