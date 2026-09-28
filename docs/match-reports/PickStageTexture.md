# PickStageTexture -- MATCHED (48/48 words)

> Renamed from `PickDailyVariant` on 2026-09-27 (tools/rename.py). Address 0x80048ea0.

> Renamed from `func_80048EA0` on 2026-09-25 (tools/rename.py). Address 0x80048ea0.

Round 82, runner echo (third echo session), 2026-09-25. Unit `GameFiles`.
Byte-exact on the first build; whole-image SHA1 green, funcdiff 48/48.

## What it does

Picks one of `n = ((day - 1) % 40) / 10 + 1` records (1..4, growing every ten
days of a 40-day cycle) at random from the group `GetStageTextureRecords(index)`.
`$a1` is passed through untouched to SeedAndRandom's unused second parameter.

## Source

```c
FilePathRecord *PickStageTexture(s32 index, s32 arg1, s32 day) {
    s32 n = ((day - 1) % 40) / 10 + 1;
    s32 r = SeedAndRandom(0, arg1) % n;
    return &GetStageTextureRecords(index)[r];
}
```

## Naming

- **Name:** `PickStageTexture`
- **Tier:** A
- **Evidence:** picks one of the first ((day - 1) % 40) / 10 + 1 of the stage's TEX?.TIX records (the record paths are retail's sRecordTable data (0x80081A04), read from disk/SLPS_015.56 in round 100); caller passes ObjM::stage and DreamSys's current day, and hands the record to New_TimBlockSrc.

## Naming history

- Round 100 (bravo, polish): renamed from `PickDailyVariant` with tools/rename.py, on the record paths and callers above; previous tier B (head review, round 82: was A. The mechanics are this body's; the purpose word comes from the callers' inherited names in code_1677c.c / GameApplication.h, which are themselves hypotheses, so the name is consistent but not established).

## Track 10 (2026-09-28, round 104, echo)

GameFiles.h's `FilePathRecord` (an opaque `u8 data[0x1C]`) merged into CdDriver.h's `CdFileEntry` ({name[0x14], CdlLOC pos, u32 size}, 0x1C): sRecordTable goes GetRecordTable -> RegisterRecordTableFiles -> RegisterFileTableEntries -> SetFileTable, so its records are the CD driver's file-table entries. GameFiles.h includes CdDriver.h; sRecordTable is declared `CdFileEntry[]` and GetRecordTable returns `CdFileEntry *`, dropping the casts at its callers, and a record used as a path is spelled `record->name` instead of a `(char *)`/`(const char *)` cast. Byte-identical.
