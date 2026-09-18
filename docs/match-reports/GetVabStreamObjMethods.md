> Renamed from `func_8002CC0C` on 2026-09-18 (tools/rename.py). Address 0x8002cc0c.

# GetVabStreamObjMethods -- MATCHED (4/4 words)

Unit: `code_179d8_e`. Runner: echo, round 17.

## Result

```c
TableDA34 *GetVabStreamObjMethods(void) {
    return &D_8006DA34;
}
```

Byte-exact, 4/4 words.

## Notes

The "get methods table" accessor for the `D_8006DA34` class -- called from
`New_VabStreamObj` (this unit) and from `VabStreamObj__VabStreamObj`/`VabStreamObj__Close` (this
unit, both BLOCKED gp_rel) to obtain the constructor/dispatch table before
indexing a slot. Same idiom as `GetVabDriverMethods` (this unit) for `D_8006D9BC`.
