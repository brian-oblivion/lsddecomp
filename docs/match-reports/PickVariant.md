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
Rec1C *PickVariant(s32 index, s32 arg1) {
    u32 r = (u32)SeedAndRandom(0, arg1) % 5;  /* arg1 only forwarded, like PickDailyVariant's */
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

## Arity (round 82, alpha, track 3 externcheck)

Matched first as `PickVariant(s32 index)` passing an uninitialised local as
SeedAndRandom's second argument (cc1: ``'unused' might be used
uninitialized``). The body never writes `$a1` before the `jal SeedAndRandom`,
so the register SeedAndRandom receives is PickVariant's own incoming `$a1`,
and its only caller (ObjM__InitStyleAndWorld, class_3bb8c_l.c) loads it
explicitly (`move a1,zero` at the jal). That is the forwarding idiom:
PickVariant takes a second parameter and forwards it, exactly as
PickDailyVariant does with its `arg1`. Rewritten with the parameter;
byte-identical (whole-image SHA1 green), the warning is gone, and the
caller's 2-parameter extern now agrees with the definition.

## Naming

- **Name:** `PickVariant`
- **Tier:** A
- **Evidence:** leaf picker: 1-of-5 random-or-forced pick (gForcedVariant) from GetVariantBlock(index), with an index==9 special case.
