# PickOpeningMovie -- MATCHED (36/36 words)

> Renamed from `PickWeeklyStreamChannel` on 2026-09-27 (tools/rename.py). Address 0x8004913c.

> Renamed from `func_8004913C` on 2026-09-25 (tools/rename.py). Address 0x8004913c.

Round 82, runner echo (third echo session), 2026-09-25. Unit `GameFiles`.
Byte-exact on the first build; whole-image SHA1 green, funcdiff 36/36,
0 insertions / 0 deletions.

## What it does

Random pick within the record block at 0x230: `r = (u32)SeedAndRandom(0, ?) % 7`,
`rec = GetOpeningMovieRecords(&count)` (the record at index 0x230, count 0); writes
`r + count` through the optional out pointer and returns `&rec[r]`.

## Source

Declarations: the local views at the top of `src/GameFiles.c` (`FilePathRecord`).

```c
FilePathRecord *PickOpeningMovie(s32 *countOut, s32 arg1) {
    u32 r = (u32)SeedAndRandom(0, arg1) % 7;  /* arg1 only forwarded, like PickStageTexture's */
    s32 count;
    FilePathRecord *rec = GetOpeningMovieRecords(&count);
    if (countOut != NULL) {
        *countOut = r + count;
    }
    return &rec[r];
}
```

## Notes

Retail never sets `$a1` before `jal SeedAndRandom` (whose second parameter is
unused), so the call passes an uninitialised local: it emits no instruction.

## Arity (round 82, alpha, track 3 externcheck)

Matched first as a 1-parameter function passing an uninitialised local as
SeedAndRandom's second argument (cc1: ``'unused' might be used
uninitialized``). The body never writes `$a1` before `jal SeedAndRandom`, so
SeedAndRandom receives PickOpeningMovie's own incoming `$a1`, and its
caller (GameApplicationFileResource.c, via GameApplication.h's 2-parameter extern) loads it
explicitly (`move a1,zero` at the jal). Forwarding idiom: the definition now
takes `s32 arg1` and forwards it, as PickStageTexture does. Byte-identical;
the warning is gone and the extern agrees with the definition.

## Naming

- **Name:** `PickOpeningMovie`
- **Tier:** A
- **Evidence:** random one of the seven GetOpeningMovieRecords (`% OPENING_MOVIE_COUNT`), with its movie id; its caller (GameApplication__PlayOpeningMovie) streams it. The pick is `rand() % 7` with no seed, not a day of the week.

## Naming history

- Round 100 (bravo, polish): renamed from `PickWeeklyStreamChannel` with tools/rename.py, on the record paths and callers above; previous tier B (head review, round 82: was A. The mechanics are this body's; the purpose word comes from the callers' inherited names in code_1677c.c / GameApplication.h, which are themselves hypotheses, so the name is consistent but not established).
