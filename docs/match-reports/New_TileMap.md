# New_TileMap -- MATCHED (27/27 words)

> Renamed from `New_GridIndexSrc` on 2026-09-25 (tools/rename.py). Address 0x80044cd4.

> Renamed from `func_80044CD4` on 2026-09-25 (tools/rename.py). Address 0x80044cd4.

Round 82, runner echo (code_33808 session, echo #7), 2026-09-25. Unit `code_33808`.
Byte-exact; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 27/27 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

Allocator shape: `BMemPMgrAlloc(size)`, then the constructor (+0x008) through the class's table getter, called through the unit-local `UnprototypedCtorTable` view (an unprototyped `s32 (*ctor)()` at +0x008, declared at the top of the unit with the getter prototypes), since each class's constructor takes different arguments. Shape: `if (obj != NULL) { ctor; return obj; } return NULL;`.

Table slot (`tools/classtable.py`): none (allocator for gTileMapMethods, object size 0x44, two constructor arguments).

## Source

The unit-local view `DataSrc33808` (a FileResource subclass built with the unified
`FILERESOURCE_SLOTS`/`FILERESOURCE_FIELDS` macros, plus `slot7C`/`slot80`, and own
fields +0x2C..+0x38) and `CountedBuf33808` sit at the top of `src/code_33808.c`.

```c
/* Allocate and construct a gTileMapMethods object. */
void *New_TileMap(s32 arg0, s32 arg1) {
    void *obj = BMemPMgrAlloc(0x44);

    if (obj != NULL) {
        ((UnprototypedCtorTable *)GetTileMapMethods())->ctor(obj, arg0, arg1);
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

- **New_TileMap**, tier A. src/code_2c054.c's TaskCore__TaskCore hands this object to New_BgLayer as its map source; this object's own fields (+0x2C..+0x30) are byte-for-byte Map44294/GsMAP's own layout (cellw/cellh/ncellw/ncellh).

## Track 4 (2026-09-26, round 88, alpha)

Class unified in `include/TileMap.h`. Returns `TileMap *` (was `void *`); the second parameter is `FileResource *atlas` (was `s32 arg1`: a TileAtlas, gTileAtlasMethods, typed by its nearest unified ancestor); the ctor is reached through `GetTileMapMethods()->ctor` (was the unit-local `UnprototypedCtorTable` cast). The prototype `extern StreamTaskUnkB4Obj *New_TileMap(...)` in include/code_2c054.h is gone. Byte-identical.

Later the same round (alpha, third class): TileAtlas unified; the `atlas` parameter is `TileAtlas *` (was `FileResource *`), in the prototype in include/TileMap.h (`struct TileAtlas *`, by tag) and in the ctor slot's parameter list. Byte-identical.

## Round 93 polish (charlie, track 7)

### Naming

| old | new | tier | evidence |
| --- | --- | --- | --- |
| the size literal | `sizeof(TileMap)` | A | each equals the object size the class header records (and the allocation retail makes); the image is byte-identical |
| `arg0` | `source` | C | only its zero test is shown (the ctor builds the default grid when it is 0, the one caller's value); by the other FileResource ctors it is probably a file to load, which no caller shows |
