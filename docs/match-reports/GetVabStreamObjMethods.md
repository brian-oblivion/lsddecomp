# GetVabStreamObjMethods -- MATCHED (4/4 words)

> Renamed from `func_8002CC0C` on 2026-09-18 (tools/rename.py). Address 0x8002cc0c.

Unit: `code_179d8_e`. Runner: echo, round 17.

## Result

```c
VabStreamObjMethods *GetVabStreamObjMethods(void) {
    return &gVabStreamObjMethods;
}
```

(`TableDA34` was this type's name before round 52's hand rename to
`VabStreamObjMethods`.)

Byte-exact, 4/4 words.

## Notes

The "get methods table" accessor for the `gVabStreamObjMethods` class -- called from
`New_VabStreamObj` (this unit) and from `VabStreamObj__VabStreamObj`/`VabStreamObj__Close` (this
unit) to obtain the constructor/dispatch table before indexing a slot. Same
idiom as `GetVabDriverMethods` (this unit) for `gVabDriverMethods`.

## Naming

Renamed `func_8002CC0C` -> `GetVabStreamObjMethods`, tier A. Pure getter --
same reasoning as `GetVabDriverMethods`.
