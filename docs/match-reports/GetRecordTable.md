# GetRecordTable -- MATCHED (7/7 words), round 81

> Renamed from `func_80048D48` on 2026-09-25 (tools/rename.py). Address 0x80048d48.

Round 81, runner echo. Unit `src/cd/game_files.c` (carved from psyq_39094 in
FINISHING-PLAN revision 18). Fresh ground, no prior attempt. Byte-exact on
the first build; whole-image SHA1 green.

- **Where:** not a slot; called from GetStageRecords and src/world/dream_day.c.
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

game_files.h's `FilePathRecord` (an opaque `u8 data[0x1C]`) merged into cd_driver.h's `CdFileEntry` ({name[0x14], CdlLOC pos, u32 size}, 0x1C): sRecordTable goes GetRecordTable -> RegisterRecordTableFiles -> RegisterFileTableEntries -> SetFileTable, so its records are the CD driver's file-table entries. game_files.h includes cd_driver.h; sRecordTable is declared `CdFileEntry[]` and GetRecordTable returns `CdFileEntry *`, dropping the casts at its callers, and a record used as a path is spelled `record->name` instead of a `(char *)`/`(const char *)` cast. Byte-identical.

## History (source comments moved in track 12, round 106)

From `src/cd/game_files.c`:

The file banner of src/cd/game_files.c read, before track 12 moved the record table's layout to include/game_files.h's file documentation:

> /*
>  * game_files.c -- the LbdFile class, and the getters over the game's table of
>  * file names.
>  *
>  * LbdFile (include/lbd_file.h, which documents the class): New_LbdFile to
>  * LbdFile__SetAutoLoadData and GetLbdFileMethods, the loader for one stage
>  * map chunk, STGnn\Mnnn.LBD.
>  *
>  * sRecordTable is an array of 0x1C-byte records (CdFileEntry), each a file path
>  * padded with zeros; dream_day.c's RegisterRecordTableFiles hands them to
>  * the CD driver. In order:
>  *  - the seven sound banks' SND\name.VH/VB pairs and SND\SE.VH/VB;
>  *  - each stage's files, from sStageFirstRecord[stage]: its four textures
>  *    (TEXA..TEXD.TIX), five BGM sequences (BGA..BGE.SEQ) and map chunks
>  *    (Mnnn.LBD, laid out as stage_grid.h's grid);
>  *  - from RECORD_TABLE_COUNT, the movies (ETC\OPENINGA..G.STR,
>  *    ETC\ENDING.STR, FILM\EVENTn.STR), then six records per special day
>  *    (FILM\SPDAYnnA/B.STR, IMG1\SPDAYnnC..F.TIM).
>  * The stage getters return a record, used as a path: PickStageBgm's goes to
>  * the WBgm's setSeq, PickStageTexture's to New_TimBlockSrc, and a map
>  * chunk's to an LbdFile. The movie getters also hand back a movie id, the
>  * movie's index in sMovieFrameCounts, whose value GetMovieFrameCount gives
>  * game_shell.c's StreamTasks as the MoviePlayer's frame count.
>  *
>  * The random pickers draw through SeedAndRandom; SetPickOverrides forces
>  * PickSoundBank's and PickStageBgm's choice (1-based, 0 for random).
>  */
