# PickDailyVariant -- MATCHED (48/48 words)

> Renamed from `func_80048EA0` on 2026-09-25 (tools/rename.py). Address 0x80048ea0.

Round 82, runner echo (third echo session), 2026-09-25. Unit `code_39094`.
Byte-exact on the first build; whole-image SHA1 green, funcdiff 48/48.

## What it does

Picks one of `n = ((day - 1) % 40) / 10 + 1` records (1..4, growing every ten
days of a 40-day cycle) at random from the group `GetStageTextureRecords(index)`.
`$a1` is passed through untouched to SeedAndRandom's unused second parameter.

## Source

```c
Rec1C *PickDailyVariant(s32 index, s32 arg1, s32 day) {
    s32 n = ((day - 1) % 40) / 10 + 1;
    s32 r = SeedAndRandom(0, arg1) % n;
    return &GetStageTextureRecords(index)[r];
}
```

## Naming

- **Name:** `PickDailyVariant`
- **Tier:** B (head review, round 82: was A. The mechanics are this body's; the purpose word comes from the callers' inherited names in code_1677c.c / GameApplication.h, which are themselves hypotheses, so the name is consistent but not established)
- **Evidence:** leaf picker: 1-of-n random pick (n grows every ten days of a 40-day cycle, per the existing report's derivation) from GetStageTextureRecords(index).
