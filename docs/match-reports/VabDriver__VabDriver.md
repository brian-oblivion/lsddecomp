# VabDriver__VabDriver

**Unit (by ROM address):** code_179d8_d · **Semantic owner:** `gVabDriverMethods`
(code_179d8_e.c) · **Size:** 1 instruction (`jr $ra; nop`, 0x8 bytes) ·
**Status: MATCHED**, whole-image SHA1 green. Splat matched this itself (empty
body); no derivation was spent.

## Role

The `+0x008` (ctor) slot of `gVabDriverMethods`, the 29-slot
FileResource-derived table that is the SPU/VAB sound-streaming backend's own
"generic driver interface" base class (code_179d8_e.c's unit header
comment). Empty: this backend needs no extra generic setup beyond
FileResource's own base ctor.

```c
void VabDriver__VabDriver(void) {
}
```

Confirmed as this exact slot by `python3 tools/classtable.py 0x8006D9BC`
(`+0x008  0x8002C3C0`, prior to this rename) and by code_179d8_e.c's own
comment naming this function (by its old placeholder) as
`gVabDriverMethods`'s ctor/dtor pair, "not this unit's to type".

## Naming

Renamed `func_8002C3C0 -> VabDriver__VabDriver`, tier A. The bytes physically
carve into code_179d8_d.c (splat's ROM-address windowing put them here), but
the function is semantically a slot of a DIFFERENT unit's class
(`gVabDriverMethods`, code_179d8_e.c) -- confirmed cross-unit by
code_179d8_e.c's own header/table comments, which already named the old
placeholder as exactly this. Per FINISHING-PLAN track 3, FUNCTION renames are
tree-wide via `rename.py` regardless of which unit's `.s` file the bytes
live in; only FIELD/SLOT ownership is unit-scoped. Named `VabDriver__X` (not
`PlacementGrid__X`) because this table has an established type name already
(`VabDriverMethods`, code_179d8_e.c) distinct from this unit's own
`D_8006D940`/`PlacementGrid` table -- confirmed two SEPARATE tables (different
VRAM addresses, different header words: 0x23 vs 0x00000E03).
`rename.py` mechanically updated code_179d8_e.c's own comments to the new
name; no manual edit was made to that unit's file.
