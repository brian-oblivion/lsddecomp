# GameApplication__PlayEndingMovie

> Renamed from `GameApplication__StartStreamTaskWithInit` on 2026-09-27 (tools/rename.py). Address 0x80026900.

> Renamed from `Class6D3C8__StartStreamTaskWithInit` on 2026-09-26 (tools/rename.py). Address 0x80026900.

> Renamed from `func_80026900` on 2026-09-24 (tools/rename.py). Address 0x80026900.

**Unit:** game_shell · **Size:** 56 instructions (0xE0 bytes) · **Status:** MATCHED (56/56 words, whole-image SHA1 green), first attempt

## What it does

`GameApplicationMethods` slot `+0x064`, the last function in this unit's queue.
Gated by `self->arg->unk08` (the same gate `GameApplication__PlayOpeningMovie` and
`GameApplication__PlaySpecialDayMovies` use). Builds a `StreamTask`, runs its `slot12C`, derives a
type code via `GetEndingMovie` (a new library helper with the same
"write-to-`*out`, return-a-separate-value" shape as
`GetAsmkMovie`/`PickOpeningMovie`/`GetSpecialDayMovieSpan`), looks it up via
`GetMovieFrameCount`, initializes the task with it, then starts it -- the same
overall shape as `GameApplication__ShowIntroLogos`/`GameApplication__PlayOpeningMovie`, with a `slot12C` call
added (matching `GameApplication__PlaySpecialDayMovies`'s use of that slot).

## Final C

```c
void GameApplication__PlayEndingMovie(GameApplication *self) {
    StreamTask *task;
    s32 typeCode;
    s32 outerValue;
    s32 typeLookup;

    if (self->arg->unk08 != 0) {
        SetActiveDataSourceDriverMode(0, 0, 0);
        task = New_StreamTask(0, 0, 0, 0);
        task->methods->slot12C(task, 0);
        outerValue = GetEndingMovie(&typeCode, 0);
        typeLookup = GetMovieFrameCount(typeCode);
        task->methods->slot44(task, self->unk1C, outerValue, typeLookup, 1);
        task->methods->slot4(task);
    }
}
```

Matched first attempt: this unit's own established idioms carried straight
across --

- the `task->methods->slot44(..., outerValue, typeLookup, 1)` shape, where
  `outerValue` is a PRECEDING call's return (`GetEndingMovie`, captured via
  the delay slot of the `GetMovieFrameCount` `jal` right after it) and
  `typeLookup` is the REAL 4th argument (the delay slot of `slot44`'s own
  `jalr`, per the idiom `GameApplication__ShowIntroLogos`'s report first documented);
- declaring `typeCode`/`outerValue`/`typeLookup` before `task` (the
  register-allocation-by-declaration-order lesson from `GameApplication__ShowIntroLogos`),
  though here it turned out not to matter -- the natural declaration order
  already put `task` last relative to its own first use, so no reordering
  was needed this time.

## Proposed learning

None beyond what this unit's earlier reports already established; this
function is a clean fourth instance of the "StreamTask init" shape
(`GameApplication__ShowIntroLogos`, `GameApplication__PlayOpeningMovie`, `GameApplication__PlaySpecialDayMovies`, now this one), each
gated by a different `GameApplicationConfig` field and differing only in
which library helper derives the type code and whether extra slots
(`slot12C`) are involved.

## HEAD BROADCAST cross-check (this round's two levers)

- **goto/return early-exit lever:** not applicable -- single guard wraps
  the whole body, void function, no early exit.
- **hand-hoisted loop invariant lever:** not applicable -- no loop.

## Naming history (before round 100)
**`GameApplication__PlayEndingMovie` -- tier B.** Mechanics: gated by
`arg->unk08` (same gate as `GameApplication__PlayOpeningMovie`), builds a
`StreamTask`, runs its `slot12C` (a step none of the other three
StreamTask-launcher siblings besides `GameApplication__PlayCinematic`
perform), derives a type code via `GetEndingMovie`, looks it up, configures
and starts the task. "WithInit" names the one mechanical difference from
its closest sibling `GameApplication__PlayOpeningMovie` (the extra
`slot12C` call); `GetEndingMovie`'s own meaning is not established, so no
stronger, purpose-based name is supported yet.

## Track 4 (2026-09-25, round 84, alpha)

GameApplication.h's StreamTask view names +0x004 `release` (BasicClass's, `void *`), was `start` (track 4 round 84; see GameApplication__PlayCinematic for the bytes that settled the return type). Byte-identical.

## Track 7 polish (round 100, echo)

### Naming

**`GameApplication__PlayEndingMovie` -- tier A** (renamed from `GameApplication__StartStreamTaskWithInit` with tools/rename.py). Evidence: streams GetEndingMovie's record, ETC\ENDING.STR (RECORD_ENDING_MOVIE, MOVIE_ENDING), with no skip on confirm; hook +0x064, which Application__RunMainLoop runs only when RunDayTask reports that a year has gone by.

Body changes, all byte-identical: typeCode/outerValue/typeLookup -> movieId/moviePath/frameCount; GetEndingMovie's extern returns const char * and keeps its arity-ok note, shortened (the full one is in the history section).

### History: code_1677c.c comments before the round-100 polish

Moved here from the source, verbatim (names as they stood then, where the tools had not already rewritten them).

```c
/* GameApplicationMethods slot +0x064. Gated by self->config->playStreams (same gate as
 * GameApplication__StartWeeklyStreamTask/GameApplication__StartGraphRoomStreamTask). Builds a StreamTask, clears its skipOnConfirm,
 * derives a type code via GetEndingMovie, looks it up via GetMovieFrameCount,
 * initializes the task with it, then starts it -- the same shape as
 * GameApplication__LoadIntroLogoSequence/GameApplication__StartWeeklyStreamTask, but with setSkipOnConfirm(0) added and GetEndingMovie
 * in place of GetAsmkMovie/PickOpeningMovie. */

/* GameFiles.c: same "write to *out, return a separate value" shape as
 * GetAsmkMovie/PickOpeningMovie/GetSpecialDayMovieSpan. */
extern s32 GetEndingMovie(s32 *out, s32 unused); /* arity-ok: the definition is 1-parameter and reads only $a0 (it neither reads nor forwards $a1), but the 2nd argument IS byte-load-bearing -- retail emits `move a1,zero` in the jal's delay slot at 0x80026974 */
```

## Track 10 (2026-09-28, round 104, alpha)

`StreamTask::streamName`, StreamTask__Init's parameter and StreamTaskInitFn's are `const char *` (were `s32`): every caller passes a path (GetAsmkMovie's string or a FilePathRecord), so the five `(s32)` casts in game_shell.c are gone. Byte-identical.

## Track 10 (2026-09-28, round 104, echo)

game_files.h's `FilePathRecord` (an opaque `u8 data[0x1C]`) merged into cd_driver.h's `CdFileEntry` ({name[0x14], CdlLOC pos, u32 size}, 0x1C): sRecordTable goes GetRecordTable -> RegisterRecordTableFiles -> RegisterFileTableEntries -> SetFileTable, so its records are the CD driver's file-table entries. game_files.h includes cd_driver.h; sRecordTable is declared `CdFileEntry[]` and GetRecordTable returns `CdFileEntry *`, dropping the casts at its callers, and a record used as a path is spelled `record->name` instead of a `(char *)`/`(const char *)` cast. Byte-identical.
