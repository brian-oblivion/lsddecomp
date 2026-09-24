# New_VabStreamObj -- MATCHED (24/24 words)

> Renamed from `func_8002C480` on 2026-09-18 (tools/rename.py). Address 0x8002c480.

Unit: `code_179d8_e`. Runner: echo, round 17.

## Result

```c
void *New_VabStreamObj(s32 arg0) {
    void *self;

    self = BMemPMgrAlloc(0x64);
    if (self != NULL) {
        GetVabStreamObjMethods()->slot08(self, arg0);
        return self;
    }
    return NULL;
}
```

with a new `slot08` member on `TableDA34` (`void (*slot08)(void *self, s32
arg1);`, offset +0x008) and a forward declaration of `GetVabStreamObjMethods` added
near the top of the unit (it's defined later in ROM order but called here).

Byte-exact, 24/24 words.

## Notes

The `new_class_*`-style constructor for the `gVabStreamObjMethods` class: allocate
0x64 bytes, then dispatch through the table's own slot +0x008
(`VabStreamObj__VabStreamObj`, BLOCKED gp_rel -- not this unit's to write, only its
prototype's shape matters here) as the actual constructor, which is where
`self->methods` gets assigned (inside `VabStreamObj__VabStreamObj`'s own body, not here).
Same idiom as `New_Class6D940` in the sibling `code_179d8_d.c`.

`slot08`'s return value is discarded -- retail overwrites `$v0` with `self`
unconditionally right after the `jalr`, regardless of what the constructor
returned -- so it's typed `void`.

Return type is `void *` rather than `VabStreamObj *` (`ObjDA34` before
round 52's local-type rename): nothing in this function itself dereferences
the allocated object (the field-0/methods assignment happens inside the
constructor), so there's no positive evidence for the more specific type
here.

## Naming

Renamed `func_8002C480` -> `New_VabStreamObj`, tier A. Matches
FINISHING-PLAN track 3's own constructor convention (`New_Class` for the
allocate-and-dispatch entry point, `Class__Class` for the real per-object
init) -- this function's whole body IS "malloc the object, dispatch to the
real constructor," the same idiom already named `New_Class6D940` in the
sibling `code_179d8_d.c`.
