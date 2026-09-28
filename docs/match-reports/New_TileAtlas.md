# New_TileAtlas -- MATCHED (24/24 words)

> Renamed from `func_80044F30` on 2026-09-25 (tools/rename.py). Address 0x80044f30.

Round 82, runner echo (graphics_resources session, echo #7), 2026-09-25. Unit `graphics_resources`.
Byte-exact; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 24/24 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

Allocator shape: `BMemPMgrAlloc(size)`, then the constructor (+0x008) through the class's table getter, called through the unit-local `UnprototypedCtorTable` view (an unprototyped `s32 (*ctor)()` at +0x008, declared at the top of the unit with the getter prototypes), since each class's constructor takes different arguments. Shape: `if (obj != NULL) { ctor; return obj; } return NULL;`.

Table slot (`tools/classtable.py`): none (allocator for gTileAtlasMethods, object size 0x38).

## Source

The unit-local view `DataSrc33808` (a FileResource subclass built with the unified
`FILERESOURCE_SLOTS`/`FILERESOURCE_FIELDS` macros, plus `slot7C`/`slot80`, and own
fields +0x2C..+0x38) and `SubBlockTable` sit at the top of `src/graphics/graphics_resources.c`.

```c
/* Allocate and construct a gTileAtlasMethods object. */
void *New_TileAtlas(s32 arg0) {
    void *obj = BMemPMgrAlloc(0x38);

    if (obj != NULL) {
        ((UnprototypedCtorTable *)GetTileAtlasMethods())->ctor(obj, arg0);
        return obj;
    }
    return NULL;
}
```

## Notes

- Byte-exact on the first build.
- Not referenced by any data word (`grep` of asm/data finds no pointer to it); called from code elsewhere or unused.
- No shared header was edited; prototypes for other units' functions are local
  to the unit.
- Types of arguments and returns are readings of the registers used, not proven.

## Naming

- **New_TileAtlas**, tier A. src/app/task.c builds this object first and hands it to New_TileMap; its BuildCells lays out exactly the 300-cell (20x15) atlas TileMap__BuildMap indexes.

## Track 4 (2026-09-26, round 88, alpha)

Class unified in `include/TileAtlas.h` (gTileAtlasMethods, 0x303, a FileResource subclass, 0x38 bytes). Returns `TileAtlas *` (was `void *`; include/task.h's local view returned `StreamTaskUnkB4Obj *` and is deleted); the ctor is reached through the typed `TileAtlasMethods` ctor slot `(TileAtlas *self, s32 arg1)` instead of the unit's `UnprototypedCtorTable` cast. One caller, TaskCore__TaskCore (src/app/task.c), which stores the result in TaskCore::tileAtlas and passes it to New_TileMap. No rename. Byte-identical.

## Round 93 polish (charlie, track 7)

### Naming

| old | new | tier | evidence |
| --- | --- | --- | --- |
| the size literal | `sizeof(TileAtlas)` | A | each equals the object size the class header records (and the allocation retail makes); the image is byte-identical |
| `arg0` | `source` | C | as New_TileMap's |
