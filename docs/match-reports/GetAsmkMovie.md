# GetAsmkMovie -- MATCHED (7/7 words), round 81

> Renamed from `GetIntroStreamName` on 2026-09-27 (tools/rename.py). Address 0x800490f4.

> Renamed from `func_800490F4` on 2026-09-25 (tools/rename.py). Address 0x800490f4.

Round 81, runner echo. Unit `src/GameFiles.c` (carved from psyq_39094 in
FINISHING-PLAN revision 18). Fresh ground, no prior attempt. Byte-exact on
the first build; whole-image SHA1 green.

- **Where:** not a slot; called from src/code_1677c.c.
- **What:** `if (typeCodeOut != NULL) *typeCodeOut = 0x31; return sAsmkMoviePath;` -- signature copied from `include/GameApplication.h` (`const char *`; rodata `sAsmkMoviePath` referenced as a symbol, no literal).
- **Levers:** none needed.
- **Name:** kept `func_`; role not yet identified beyond the above.

## Source

```c
const char *GetAsmkMovie(s32 *typeCodeOut) {
    if (typeCodeOut != NULL) {
        *typeCodeOut = 0x31;
    }
    return sAsmkMoviePath;
}
```

## Naming

- **Name:** `GetAsmkMovie`
- **Tier:** A
- **Evidence:** returns sAsmkMoviePath (renamed from sAsmkStreamPath), "ETC\ASMK.STR", an STR movie, and writes movie id 49 (MOVIE_ASMK), whose gMovieFrameCounts entry GameApplication__LoadIntroLogoSequence passes to the StreamTask. Named for the file, not for a role.

## Naming history

- Round 100 (bravo, polish): renamed from `GetIntroStreamName` with tools/rename.py, on the record paths and callers above; previous tier A.
