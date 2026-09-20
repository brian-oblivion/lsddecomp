> Renamed from `func_800404C0` on 2026-09-20 (tools/rename.py). Address 0x800404c0.

# Class6E99C__GetMethods -- MATCH (4/4 words, first attempt, trivial)

Unit `code_2cc8c_e`, carved round 14. `Class6E99CObj`'s own bare
no-argument table getter, same idiom as `GetClass6B5CCMethods`
(`include/code_d294.h`) for `Class6B5CCObj`: `Class6E99CMethods
*Class6E99C__GetMethods(void) { return &D_8006E99C; }`.

## Naming (round 61, track 3)

**`Class6E99C__GetMethods`** -- tier A. Bare no-argument getter,
`return &D_8006E99C;` -- the exact same idiom as `code_d294.h`'s
`GetClass6B5CCMethods` for `Class6B5CCObj`'s own table. Mechanics fully
determine the name.
