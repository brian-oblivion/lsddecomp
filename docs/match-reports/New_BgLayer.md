# New_BgLayer -- MATCHED (27/27 words)

> Renamed from `func_800441B4` on 2026-09-25 (tools/rename.py). Address 0x800441b4.

Round 82, runner echo (graphics_resources session, echo #7), 2026-09-25. Unit `graphics_resources`.
Byte-exact; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 27/27 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

Allocator shape: `BMemPMgrAlloc(size)`, then the constructor (+0x008) through the class's table getter, called through the unit-local `UnprototypedCtorTable` view (an unprototyped `s32 (*ctor)()` at +0x008, declared at the top of the unit with the getter prototypes), since each class's constructor takes different arguments. Shape: `if (obj != NULL) { ctor; return obj; } return NULL;`.

Table slot (`tools/classtable.py`): none (allocator for gBgLayerMethods, object size 0x68, two constructor arguments).

## Source

The unit-local view `DataSrc33808` (a FileResource subclass built with the unified
`FILERESOURCE_SLOTS`/`FILERESOURCE_FIELDS` macros, plus `slot7C`/`slot80`, and own
fields +0x2C..+0x38) and `SubBlockTable` sit at the top of `src/graphics/graphics_resources.c`.

```c
/* Allocate and construct a gBgLayerMethods object. */
void *New_BgLayer(s32 arg0, s32 arg1) {
    void *obj = BMemPMgrAlloc(0x68);

    if (obj != NULL) {
        ((UnprototypedCtorTable *)GetBgLayerMethods())->ctor(obj, arg0, arg1);
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

- **New_BgLayer**, tier B. Allocator for gBgLayerMethods, a SceneNode subclass whose own fields (bgAttribute, x/y/w/h, scrollx/scrolly, scalex/scaley, a 20.12 fixed-point rotate word) are GsBG's own layout.

## Track 4 (2026-09-26, round 88, alpha)

Class unified in `include/bg_layer.h`. Returns `BgLayer *` and takes `(struct Map44294 *src, s32 mode)` (was `void *` / `s32, s32`); the ctor is called through the typed `GetBgLayerMethods()->ctor` (was the unit-local unprototyped `UnprototypedCtorTable` view). The slot keeps SceneNode's `void *` return; the value is ignored, as before. The local views of it in include/task.h are gone. Byte-identical.

Later the same round (alpha, second class): TileMap unified too (`include/tile_map.h`, same round): `src` is `TileMap *` (was `struct Map44294 *`). Byte-identical.

## Round 93 polish (charlie, track 7)

### Naming

| old | new | tier | evidence |
| --- | --- | --- | --- |
| the size literal | `sizeof(BgLayer)` | A | each equals the object size the class header records (and the allocation retail makes); the image is byte-identical |
