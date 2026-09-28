# GetMovieFrameCount -- MATCHED (7/7 words), round 81

> Renamed from `GetStreamGroupForType` on 2026-09-27 (tools/rename.py). Address 0x800493c8.

> Renamed from `func_800493C8` on 2026-09-25 (tools/rename.py). Address 0x800493c8.

Round 81, runner echo. Unit `src/cd/game_files.c` (carved from psyq_39094 in
FINISHING-PLAN revision 18). Fresh ground, no prior attempt. Byte-exact on
the first build; whole-image SHA1 green.

- **Where:** not a slot.
- **What:** `return sMovieFrameCounts[index];` over `extern s16 sMovieFrameCounts[]` (sll/addu/lh through `$at`, the resolved addiu_at construct).
- **Levers:** none needed.
- **Name:** kept `func_`; role not yet identified beyond the above.

## Source

```c
s32 GetMovieFrameCount(s32 index) {
    return sMovieFrameCounts[index];
}
```

## Naming

- **Name:** `GetMovieFrameCount`
- **Tier:** A
- **Evidence:** returns sMovieFrameCounts[movieId] (renamed from gStreamTypeToGroupTable); every caller passes it to StreamTask__Init, which hands it to MoviePlayer__Play as `frameCount` (src/graphics/GraphicsResources.c). Its argument is always a movie id from this unit's movie getters.

## Naming history

- Round 100 (bravo, polish): renamed from `GetStreamGroupForType` with tools/rename.py, on the record paths and callers above; previous tier A.
