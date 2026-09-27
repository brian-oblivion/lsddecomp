# PickStageBgm -- MATCHED (46/46 words)

> Renamed from `PickVariant` on 2026-09-27 (tools/rename.py). Address 0x80048f84.

> Renamed from `func_80048F84` on 2026-09-25 (tools/rename.py). Address 0x80048f84.

Round 82, runner echo (third echo session), 2026-09-25. Unit `code_39094`.
Byte-exact on the second build; whole-image SHA1 green, funcdiff 46/46.

## What it does

Random pick of one of five records in the block `GetStageBgmRecords(index)`
(record 4 of the group): `r = rand % 5`; for group 9, r 2 becomes 3 and an
override gForcedStageBgm of 3 becomes 4. Returns `&rec[gForcedStageBgm - 1]` when the
override is set, else `&rec[r]`.

## Source

```c
FilePathRecord *PickStageBgm(s32 index, s32 arg1) {
    u32 r = (u32)SeedAndRandom(0, arg1) % 5;  /* arg1 only forwarded, like PickStageTexture's */
    FilePathRecord *rec;
    if (index == 9) {
        if (r == 2) {
            r = 3;
        }
        if (gForcedStageBgm == 3) {
            gForcedStageBgm = 4;
        }
    }
    rec = GetStageBgmRecords(index);
    return &rec[gForcedStageBgm != 0 ? gForcedStageBgm - 1 : r];
}
```

## Levers (2 builds)

1. `if (D) return &rec[D-1]; return &rec[r];` -- length changes (per-branch
   full index computation).
2. MATCH: the ternary index. Retail computes `x*7` per branch and shares the
   final `sll 2; addu` after the join: that is ONE index expression with a
   conditional operand (contrast PickSoundBank, where retail keeps separate
   `sll`s per branch and needs per-branch pointers). Read which instructions
   sit after the join to choose between the two shapes.

## Arity (round 82, alpha, track 3 externcheck)

Matched first as `PickStageBgm(s32 index)` passing an uninitialised local as
SeedAndRandom's second argument (cc1: ``'unused' might be used
uninitialized``). The body never writes `$a1` before the `jal SeedAndRandom`,
so the register SeedAndRandom receives is PickStageBgm's own incoming `$a1`,
and its only caller (ObjM__InitStyleAndWorld, class_3bb8c_l.c) loads it
explicitly (`move a1,zero` at the jal). That is the forwarding idiom:
PickStageBgm takes a second parameter and forwards it, exactly as
PickStageTexture does with its `arg1`. Rewritten with the parameter;
byte-identical (whole-image SHA1 green), the warning is gone, and the
caller's 2-parameter extern now agrees with the definition.

## Naming

- **Name:** `PickStageBgm`
- **Tier:** A
- **Evidence:** random 1-of-5 (or gForcedStageBgm, renamed from gForcedVariant) of GetStageBgmRecords; its caller hands the record to the WBgm's setSeq (class_3bb8c_l.c). Stage 9 skips index 2, BGC.SEQ.

## Naming history

- Round 100 (bravo, polish): renamed from `PickVariant` with tools/rename.py, on the record paths and callers above; previous tier A.
