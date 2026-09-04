# func_8002CC0C -- MATCHED (4/4 words)

Unit: `code_179d8_e`. Runner: echo, round 17.

## Result

```c
TableDA34 *func_8002CC0C(void) {
    return &D_8006DA34;
}
```

Byte-exact, 4/4 words.

## Notes

The "get methods table" accessor for the `D_8006DA34` class -- called from
`func_8002C480` (this unit) and from `func_8002C4E0`/`func_8002C638` (this
unit, both BLOCKED gp_rel) to obtain the constructor/dispatch table before
indexing a slot. Same idiom as `func_8002C438` (this unit) for `D_8006D9BC`.
