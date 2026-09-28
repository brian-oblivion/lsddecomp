# GetRecordTable -- MATCHED (7/7 words), round 81

> Renamed from `func_80048D48` on 2026-09-25 (tools/rename.py). Address 0x80048d48.

Round 81, runner echo. Unit `src/cd/GameFiles.c` (carved from psyq_39094 in
FINISHING-PLAN revision 18). Fresh ground, no prior attempt. Byte-exact on
the first build; whole-image SHA1 green.

- **Where:** not a slot; called from GetStageRecords and src/world/DayTaskStageMap.c.
- **What:** `if (out != NULL) *out = 0x230; return sRecordTable;` -- the `ori v0,0x230` in the beqz delay slot is the compiler's own scheduling.
- **Levers:** none needed.
- **Name:** kept `func_`; role not yet identified beyond the above.

## Source

```c
void *GetRecordTable(s32 *out) {
    if (out != NULL) {
        *out = 0x230;
    }
    return sRecordTable;
}
```

## Naming

- **Name:** `GetRecordTable`
- **Tier:** A
- **Evidence:** table getter: returns sRecordTable and writes the record count (0x230) to *out.

## Track 10 (2026-09-28, round 104, echo)

GameFiles.h's `FilePathRecord` (an opaque `u8 data[0x1C]`) merged into cd_driver.h's `CdFileEntry` ({name[0x14], CdlLOC pos, u32 size}, 0x1C): sRecordTable goes GetRecordTable -> RegisterRecordTableFiles -> RegisterFileTableEntries -> SetFileTable, so its records are the CD driver's file-table entries. GameFiles.h includes cd_driver.h; sRecordTable is declared `CdFileEntry[]` and GetRecordTable returns `CdFileEntry *`, dropping the casts at its callers, and a record used as a path is spelled `record->name` instead of a `(char *)`/`(const char *)` cast. Byte-identical.
