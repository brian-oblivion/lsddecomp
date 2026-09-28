# GetStageTextureRecords -- MATCHED (8/8 words), round 81

> Renamed from `GetRecordGroupAlias` on 2026-09-27 (tools/rename.py). Address 0x80048e80.

> Renamed from `func_80048E80` on 2026-09-25 (tools/rename.py). Address 0x80048e80.

Round 81, runner echo. Unit `src/cd/GameFiles.c` (carved from psyq_39094 in
FINISHING-PLAN revision 18). Fresh ground, no prior attempt. Byte-exact on
the first build; whole-image SHA1 green.

- **Where:** not a slot.
- **What:** tail wrapper: `return GetStageRecords(index);` (GetStageRecords returns `GetRecordTable(NULL) + sStageFirstRecord[index] * 0x1C`; its prototype is declared locally as `void *GetStageRecords(s32 index)`). The byte match says nothing about the return type.
- **Levers:** none needed.
- **Name:** kept `func_`; role not yet identified beyond the above.

## Source

```c
void *GetStageRecords(s32 index);

void *GetStageTextureRecords(s32 index) {
    return GetStageRecords(index);
}
```

## Naming

- **Name:** `GetStageTextureRecords`
- **Tier:** A
- **Evidence:** tail call of GetStageRecords: the group's first four records are TEXA..TEXD.TIX (the record paths are retail's sRecordTable data (0x80081A04), read from disk/SLPS_015.56 in round 100); PickStageTexture indexes it and ObjM hands the pick to New_TimBlockSrc (ObjMStyleActor.c).

## Naming history

- Round 100 (bravo, polish): renamed from `GetRecordGroupAlias` with tools/rename.py, on the record paths and callers above; previous tier A.

## Track 10 (2026-09-28, round 104, echo)

GameFiles.h's `FilePathRecord` (an opaque `u8 data[0x1C]`) merged into CdDriver.h's `CdFileEntry` ({name[0x14], CdlLOC pos, u32 size}, 0x1C): sRecordTable goes GetRecordTable -> RegisterRecordTableFiles -> RegisterFileTableEntries -> SetFileTable, so its records are the CD driver's file-table entries. GameFiles.h includes CdDriver.h; sRecordTable is declared `CdFileEntry[]` and GetRecordTable returns `CdFileEntry *`, dropping the casts at its callers, and a record used as a path is spelled `record->name` instead of a `(char *)`/`(const char *)` cast. Byte-identical.
