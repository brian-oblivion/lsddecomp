> Renamed from `func_8002C438` on 2026-09-18 (tools/rename.py). Address 0x8002c438.

# GetVabDriverMethods -- MATCHED (4/4 words)

Unit: `code_179d8_e`. Runner: echo, round 17.

## Result

```c
TableD9BC *GetVabDriverMethods(void) {
    return &gVabDriverMethods;
}
```

Byte-exact, 4/4 words (`lui`/`addiu` computing `&gVabDriverMethods`, then `jr`/`nop`).

## Notes

The "get methods table" accessor for the `gVabDriverMethods` class -- same idiom as
`GetVabStreamObjMethods` (this unit) returning `&gVabStreamObjMethods`, and `func_8002C3A8` in
the sibling `code_179d8_d.c` returning `&D_8006D940`. Confirmed void-argument
by checking its two call sites (`code_171e0/func_80026CAC.s`,
`code_171e0/func_80026FE8.s`): both `jal GetVabDriverMethods` with no argument
register set up beforehand.

`TableD9BC` is declared in this unit's own top-of-file scaffolding (added
alongside `func_8002C408`'s match, same commit chain) -- see that struct
definition for the class-framework correction this round made to the unit's
header comment.
