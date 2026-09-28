# NullDriver__NullDriver

> Renamed from `VabDriver__VabDriver` on 2026-09-28 (tools/rename.py). Address 0x8002c3c0.

**Unit (by ROM address):** PlacementGridVabSound · **Semantic owner:** `gNullDriverMethods`
(PlacementGridVabSound.c) · **Size:** 1 instruction (`jr $ra; nop`, 0x8 bytes) ·
**Status: MATCHED**, whole-image SHA1 green. Splat matched this itself (empty
body); no derivation was spent.

## Role

The `+0x008` (ctor) slot of `gNullDriverMethods`, the 29-slot
FileResource-derived table that is the SPU/VAB sound-streaming backend's own
"generic driver interface" base class (PlacementGridVabSound.c's unit header
comment). Empty: this backend needs no extra generic setup beyond
FileResource's own base ctor.

```c
void NullDriver__NullDriver(void) {
}
```

Confirmed as this exact slot by `python3 tools/classtable.py 0x8006D9BC`
(`+0x008  0x8002C3C0`, prior to this rename) and by PlacementGridVabSound.c's own
comment naming this function (by its old placeholder) as
`gNullDriverMethods`'s ctor/dtor pair, "not this unit's to type".

## Naming

Renamed `func_8002C3C0 -> NullDriver__NullDriver`, tier A. The bytes physically
carve into PlacementGridVabSound.c (splat's ROM-address windowing put them here), but
the function is semantically a slot of a DIFFERENT unit's class
(`gNullDriverMethods`, PlacementGridVabSound.c) -- confirmed cross-unit by
PlacementGridVabSound.c's own header/table comments, which already named the old
placeholder as exactly this. Per FINISHING-PLAN track 3, FUNCTION renames are
tree-wide via `rename.py` regardless of which unit's `.s` file the bytes
live in; only FIELD/SLOT ownership is unit-scoped. Named `NullDriver__X` (not
`PlacementGrid__X`) because this table has an established type name already
(`NullDriverMethods`, PlacementGridVabSound.c) distinct from this unit's own
`gPlacementGridMethods`/`PlacementGrid` table -- confirmed two SEPARATE tables (different
VRAM addresses, different header words: 0x23 vs 0x00000E03).
`rename.py` mechanically updated PlacementGridVabSound.c's own comments to the new
name; no manual edit was made to that unit's file.
