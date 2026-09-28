# GetSpecialDayOrEventRecord -- MATCHED (37/37 words)

> Renamed from `ResolveCinematicChannel` on 2026-09-27 (tools/rename.py). Address 0x80049334.

> Renamed from `func_80049334` on 2026-09-25 (tools/rename.py). Address 0x80049334.

Round 82, runner echo (third echo session), 2026-09-25. Unit `GameFiles`.
Byte-exact on the third build; whole-image SHA1 green, funcdiff 37/37,
0 insertions / 0 deletions.

## What it does

Takes a 4-byte struct of two s16 BY VALUE in `$a1` (retail spills `$a1` to
its home slot 0x24(sp) and reads the halves back with `lh 0x24` / `lh 0x26`).
Non-negative `group`: `rec = GetSpecialDayRecords(&count, group)`, writes
`sub + count` if `(u16)sub < 2` else -1, returns `&rec[sub]`. Negative
`group`: tail to `GetEventMovie(countOut, sub)`.

## Source

Declarations: `FilePathRecord` at the top of `src/cd/GameFiles.c`, plus:

```c
typedef struct RecPick {
    s16 group;
    s16 sub;
} RecPick;

FilePathRecord *GetSpecialDayOrEventRecord(s32 *countOut, RecPick pick) {
    s32 count;
    FilePathRecord *rec;
    if (pick.group >= 0) {
        rec = GetSpecialDayRecords(&count, pick.group);
        if (countOut != NULL) {
            *countOut = ((u16)pick.sub < 2) ? pick.sub + count : -1;
        }
        return &rec[pick.sub];
    }
    return GetEventMovie(countOut, pick.sub);
}
```

## Levers (3 builds)

1. `if (group < 0) return GetEventMovie(...);` first: blocks inverted (retail
   branches `bltz` to the tail call at the end), 6/37.
2. Positive case first: 32/37, `$v0`/`$v1` swapped in the `sub + count` /
   `-1` value.
3. MATCH: the if/else store written as ONE store of a ternary.

### Proposed learning

`sw $aN, home(sp)` then `lh $aN, home(sp)` / `lh home+2(sp)` at entry = a
two-halfword struct passed by value. An if/else writing the same lvalue with
the value's register swapped (v0/v1) -> one assignment of a ternary.

## Naming

- **Name:** `GetSpecialDayOrEventRecord`
- **Tier:** B
- **Evidence:** for group >= 0, record `sub` of GetSpecialDayRecords(group) with its movie id, or -1 for the TIM records (sub >= 2); else GetEventMovie(sub). Mechanics from the body and the record paths; the caller (GameApplication__PlayCinematic) streams a movie id and loads a -1 as an image, which agrees. What DreamSys's getCinematic pair selects in the game is not established.

## Naming history

- Round 100 (bravo, polish): renamed from `ResolveCinematicChannel` with tools/rename.py, on the record paths and callers above; previous tier B (head review, round 82: was A. The mechanics are this body's; the purpose word comes from the callers' inherited names in code_1677c.c / GameApplication.h, which are themselves hypotheses, so the name is consistent but not established).

## Track 10 (2026-09-28, round 104, echo)

GameFiles.h's `FilePathRecord` (an opaque `u8 data[0x1C]`) merged into CdDriver.h's `CdFileEntry` ({name[0x14], CdlLOC pos, u32 size}, 0x1C): sRecordTable goes GetRecordTable -> RegisterRecordTableFiles -> RegisterFileTableEntries -> SetFileTable, so its records are the CD driver's file-table entries. GameFiles.h includes CdDriver.h; sRecordTable is declared `CdFileEntry[]` and GetRecordTable returns `CdFileEntry *`, dropping the casts at its callers, and a record used as a path is spelled `record->name` instead of a `(char *)`/`(const char *)` cast. Byte-identical.
