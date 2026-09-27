# GetVabDriverMethods -- MATCHED (4/4 words)

> Renamed from `func_8002C438` on 2026-09-18 (tools/rename.py). Address 0x8002c438.

Unit: `code_179d8_d`. Runner: echo, round 17.

## Result

```c
VabDriverMethods *GetVabDriverMethods(void) {
    return &gVabDriverMethods;
}
```

(`TableD9BC` was this type's name before round 52's hand rename to
`VabDriverMethods` -- a local typedef, not a symbol, so `tools/rename.py`
doesn't touch it; updated here to match.)

Byte-exact, 4/4 words (`lui`/`addiu` computing `&gVabDriverMethods`, then `jr`/`nop`).

## Notes

The "get methods table" accessor for the `gVabDriverMethods` class -- same idiom as
`GetVabStreamObjMethods` (this unit) returning `&gVabStreamObjMethods`, and `GetPlacementGridMethods` in
the sibling `code_179d8_d.c` returning `&gPlacementGridMethods`. Confirmed void-argument
by checking its two call sites (`code_171e0/func_80026CAC.s`,
`code_171e0/func_80026FE8.s`): both `jal GetVabDriverMethods` with no argument
register set up beforehand.

`VabDriverMethods` is declared in this unit's own top-of-file scaffolding
(added alongside `VabDriver__Read`'s match, same commit chain) -- see that
struct definition for the class-framework correction this round made to the
unit's header comment.

## Naming

Renamed `func_8002C438` -> `GetVabDriverMethods`, tier A. Pure getter --
mechanics ARE its purpose (CLAUDE.md/FINISHING-PLAN's own tier-A rule for a
leaf getter). Matches the naming already in use for its sibling accessor
`GetVabStreamObjMethods` and the same-round precedent `GetCdDriverMethods`
(alpha, code_171e0.c) for "return the address of a known class's own
methods table."
