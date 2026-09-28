# GetNullDriverMethods -- MATCHED (4/4 words)

> Renamed from `GetVabDriverMethods` on 2026-09-28 (tools/rename.py). Address 0x8002c438.

> Renamed from `func_8002C438` on 2026-09-18 (tools/rename.py). Address 0x8002c438.

Unit: `PlacementGridVabSound`. Runner: echo, round 17.

## Result

```c
NullDriverMethods *GetNullDriverMethods(void) {
    return &gNullDriverMethods;
}
```

(`TableD9BC` was this type's name before round 52's hand rename to
`NullDriverMethods` -- a local typedef, not a symbol, so `tools/rename.py`
doesn't touch it; updated here to match.)

Byte-exact, 4/4 words (`lui`/`addiu` computing `&gNullDriverMethods`, then `jr`/`nop`).

## Notes

The "get methods table" accessor for the `gNullDriverMethods` class -- same idiom as
`GetVabStreamObjMethods` (this unit) returning `&gVabStreamObjMethods`, and `GetPlacementGridMethods` in
the sibling `PlacementGridVabSound.c` returning `&gPlacementGridMethods`. Confirmed void-argument
by checking its two call sites (`game_shell/func_80026CAC.s`,
`game_shell/func_80026FE8.s`): both `jal GetNullDriverMethods` with no argument
register set up beforehand.

`NullDriverMethods` is declared in this unit's own top-of-file scaffolding
(added alongside `NullDriver__Read`'s match, same commit chain) -- see that
struct definition for the class-framework correction this round made to the
unit's header comment.

## Naming

Renamed `func_8002C438` -> `GetNullDriverMethods`, tier A. Pure getter --
mechanics ARE its purpose (CLAUDE.md/FINISHING-PLAN's own tier-A rule for a
leaf getter). Matches the naming already in use for its sibling accessor
`GetVabStreamObjMethods` and the same-round precedent `GetCdDriverMethods`
(alpha, game_shell.c) for "return the address of a known class's own
methods table."
