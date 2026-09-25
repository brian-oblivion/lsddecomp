# Obj6EAC0__GetDerivedMethods — MATCHED (4/4 words)

> Renamed from `func_80040FB0` on 2026-09-18 (tools/rename.py). Address 0x80040fb0.

Unit: `src/code_2cc8c_f.c`. First attempt.

```c
Obj6EAC0Methods *Obj6EAC0__GetDerivedMethods(void) {
    return &D_8006EB90;
}
```

The override-table twin of `GetBoxFillMethods` (returns `&gBoxFillMethods`).
Called by `New_Obj6EAC0` (this unit, the class's `New_X`-shaped
allocator) to fetch the constructor at `slot0x08`.

### Proposed learning

None beyond what's already recorded.

## Naming

Round 54 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_80040FB0` | `Obj6EAC0__GetDerivedMethods` | A |

**Evidence.** A pure leaf getter (tier A by definition): returns
`&D_8006EB90`, the override method table. Twin of
`GetBoxFillMethods` (returns `&gBoxFillMethods`, the base table).

## Track 4 (2026-09-25, round 85, charlie)

Not a twin of GetBoxFillMethods in the class tree: D_8006EB90 is class 0x11144, below CharSprite (0x1144) and ScreenSprite (0x144), and gBoxFillMethods is class 0x64 (include/BoxFill.h). Round 14's "base/override" pairing of the two tables does not hold.
