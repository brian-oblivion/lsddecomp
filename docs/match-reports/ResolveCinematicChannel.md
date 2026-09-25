# ResolveCinematicChannel -- MATCHED (37/37 words)

> Renamed from `func_80049334` on 2026-09-25 (tools/rename.py). Address 0x80049334.

Round 82, runner echo (third echo session), 2026-09-25. Unit `code_39094`.
Byte-exact on the third build; whole-image SHA1 green, funcdiff 37/37,
0 insertions / 0 deletions.

## What it does

Takes a 4-byte struct of two s16 BY VALUE in `$a1` (retail spills `$a1` to
its home slot 0x24(sp) and reads the halves back with `lh 0x24` / `lh 0x26`).
Non-negative `group`: `rec = GetCinematicBank(&count, group)`, writes
`sub + count` if `(u16)sub < 2` else -1, returns `&rec[sub]`. Negative
`group`: tail to `GetStreamPool3Channel(countOut, sub)`.

## Source

Declarations: `Rec1C` at the top of `src/code_39094.c`, plus:

```c
typedef struct RecPick {
    s16 group;
    s16 sub;
} RecPick;

Rec1C *ResolveCinematicChannel(s32 *countOut, RecPick pick) {
    s32 count;
    Rec1C *rec;
    if (pick.group >= 0) {
        rec = GetCinematicBank(&count, pick.group);
        if (countOut != NULL) {
            *countOut = ((u16)pick.sub < 2) ? pick.sub + count : -1;
        }
        return &rec[pick.sub];
    }
    return GetStreamPool3Channel(countOut, pick.sub);
}
```

## Levers (3 builds)

1. `if (group < 0) return GetStreamPool3Channel(...);` first: blocks inverted (retail
   branches `bltz` to the tail call at the end), 6/37.
2. Positive case first: 32/37, `$v0`/`$v1` swapped in the `sub + count` /
   `-1` value.
3. MATCH: the if/else store written as ONE store of a ternary.

### Proposed learning

`sw $aN, home(sp)` then `lh $aN, home(sp)` / `lh home+2(sp)` at entry = a
two-halfword struct passed by value. An if/else writing the same lvalue with
the value's register swapped (v0/v1) -> one assignment of a ternary.

## Naming

- **Name:** `ResolveCinematicChannel`
- **Tier:** B (head review, round 82: was A. The mechanics are this body's; the purpose word comes from the callers' inherited names in code_1677c.c / Class6D3C8.h, which are themselves hypotheses, so the name is consistent but not established)
- **Evidence:** its one caller, Class6D3C8__StartCinematicStream, uses the return value as the cinematic's stream group id, resolved from the DreamSys's own bank/entry pick; matches exactly.
