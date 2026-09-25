# PickVariant -- MATCHED (46/46 words)

> Renamed from `func_80048F84` on 2026-09-25 (tools/rename.py). Address 0x80048f84.

Round 82, runner echo (third echo session), 2026-09-25. Unit `code_39094`.
Byte-exact on the second build; whole-image SHA1 green, funcdiff 46/46.

## What it does

Random pick of one of five records in the block `GetVariantBlock(index)`
(record 4 of the group): `r = rand % 5`; for group 9, r 2 becomes 3 and an
override gForcedVariant of 3 becomes 4. Returns `&rec[gForcedVariant - 1]` when the
override is set, else `&rec[r]`.

## Source

```c
Rec1C *PickVariant(s32 index) {
    s32 unused;
    u32 r = (u32)SeedAndRandom(0, unused) % 5;
    Rec1C *rec;
    if (index == 9) {
        if (r == 2) {
            r = 3;
        }
        if (gForcedVariant == 3) {
            gForcedVariant = 4;
        }
    }
    rec = GetVariantBlock(index);
    return &rec[gForcedVariant != 0 ? gForcedVariant - 1 : r];
}
```

## Levers (2 builds)

1. `if (D) return &rec[D-1]; return &rec[r];` -- length changes (per-branch
   full index computation).
2. MATCH: the ternary index. Retail computes `x*7` per branch and shares the
   final `sll 2; addu` after the join: that is ONE index expression with a
   conditional operand (contrast PickWeeklyGroup, where retail keeps separate
   `sll`s per branch and needs per-branch pointers). Read which instructions
   sit after the join to choose between the two shapes.
