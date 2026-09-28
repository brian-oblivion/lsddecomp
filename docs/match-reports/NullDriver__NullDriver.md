# NullDriver__NullDriver

> Renamed from `VabDriver__VabDriver` on 2026-09-28 (tools/rename.py). Address 0x8002c3c0.

**Unit (by ROM address):** vab_sound · **Semantic owner:** `gNullDriverMethods`
(vab_sound.c) · **Size:** 1 instruction (`jr $ra; nop`, 0x8 bytes) ·
**Status: MATCHED**, whole-image SHA1 green. Splat matched this itself (empty
body); no derivation was spent.

## Role

The `+0x008` (ctor) slot of `gNullDriverMethods`, the 29-slot
FileResource-derived table that is the SPU/VAB sound-streaming backend's own
"generic driver interface" base class (vab_sound.c's unit header
comment). Empty: this backend needs no extra generic setup beyond
FileResource's own base ctor.

```c
void NullDriver__NullDriver(void) {
}
```

Confirmed as this exact slot by `python3 tools/classtable.py 0x8006D9BC`
(`+0x008  0x8002C3C0`, prior to this rename) and by vab_sound.c's own
comment naming this function (by its old placeholder) as
`gNullDriverMethods`'s ctor/dtor pair, "not this unit's to type".

## Naming

Renamed `func_8002C3C0 -> NullDriver__NullDriver`, tier A. The bytes physically
carve into vab_sound.c (splat's ROM-address windowing put them here), but
the function is semantically a slot of a DIFFERENT unit's class
(`gNullDriverMethods`, vab_sound.c) -- confirmed cross-unit by
vab_sound.c's own header/table comments, which already named the old
placeholder as exactly this. Per FINISHING-PLAN track 3, FUNCTION renames are
tree-wide via `rename.py` regardless of which unit's `.s` file the bytes
live in; only FIELD/SLOT ownership is unit-scoped. Named `NullDriver__X` (not
`PlacementGrid__X`) because this table has an established type name already
(`NullDriverMethods`, vab_sound.c) distinct from this unit's own
`gPlacementGridMethods`/`PlacementGrid` table -- confirmed two SEPARATE tables (different
VRAM addresses, different header words: 0x23 vs 0x00000E03).
`rename.py` mechanically updated vab_sound.c's own comments to the new
name; no manual edit was made to that unit's file.

## History (source comments moved in track 12, round 106)

From `include/null_driver.h`:

The class banner in include/null_driver.h read, before it became the header's Doxygen class documentation:

> /*
>  * NullDriver -- the data-source driver whose every method is empty (class
>  * id 0x23, method table gNullDriverMethods), a FileResource subclass and
>  * the CD-ROM driver's (gCdDriverMethods, 0x13) sibling. The id is DATASOURCE_NULL: SetActiveDataSource
>  * (src/app/game_shell.c) binds this table's driver-interface slots into
>  * FileResource's table and every client table whenever the active source is
>  * not DATASOURCE_CD, and GetActiveDataSourceMethods returns it then.
>  *
>  * Every own method is empty. The ctor, the finalize and the eleven
>  * interface slots it overrides (+0x040..+0x058, +0x068..+0x074) are
>  * `jr $ra; nop`, `return 0` or a bare 0x40-byte frame (Open, NoOpSlot40),
>  * so with the VAB source active the file-I/O interface does nothing; the
>  * VAB streaming itself is VabStreamObj's (gVabStreamObjMethods, 0xA03,
>  * src/sound/vab_sound.c), a separate FileResource subclass. Methods in
>  * src/sound/vab_sound.c (ctor through NoOpSlot50) and src/sound/vab_sound.c
>  * (Read onward, and the getter). Each is named for its slot
>  * (`classtable.py gNullDriverMethods --vs gFileResourceMethods`); the slot names are
>  * FileResource's.
>  *
>  * NO FIELDS/SLOTS MACROS: no class lies below 0x23 (`typeviews.py --tree`).
>  * The object has no known own fields: nothing allocates a NullDriver (no
>  * New_NullDriver) and no method reads `self`, so the struct is FileResource's
>  * fields and its size is unmeasured.
>  */

Its "Methods in src/sound/vab_sound.c (ctor through NoOpSlot50) and src/sound/vab_sound.c (Read onward, and the getter)" named one file twice (release review, Track 12), a leftover of the two units the class was split across before they merged; the class doc now names the file once. "The bodies take no arguments: none of them reads a register", above the prototypes, is now worded without the register.
