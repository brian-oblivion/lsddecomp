# New_TimBlockSrc -- MATCHED (24/24 words)

> Renamed from `func_80043008` on 2026-09-25 (tools/rename.py). Address 0x80043008.

Round 82, runner echo (GraphicsResources session, echo #7), 2026-09-25. Unit `GraphicsResources`.
Byte-exact; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 24/24 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

Allocator shape: `BMemPMgrAlloc(size)`, then the constructor (+0x008) through the class's table getter, called through the unit-local `UnprototypedCtorTable` view (an unprototyped `s32 (*ctor)()` at +0x008, declared at the top of the unit with the getter prototypes), since each class's constructor takes different arguments. Shape: `if (obj != NULL) { ctor; return obj; } return NULL;`.

Table slot (`tools/classtable.py`): none (allocator for gTimBlockSrcMethods, object size 0x84).

## Source

The unit-local view `DataSrc33808` (a FileResource subclass built with the unified
`FILERESOURCE_SLOTS`/`FILERESOURCE_FIELDS` macros, plus `slot7C`/`slot80`, and own
fields +0x2C..+0x38) and `SubBlockTable` sit at the top of `src/GraphicsResources.c`.

```c
/* Allocate and construct a gTimBlockSrcMethods object. */
void *New_TimBlockSrc(s32 arg0) {
    void *obj = BMemPMgrAlloc(0x84);

    if (obj != NULL) {
        ((UnprototypedCtorTable *)GetTimBlockSrcMethods())->ctor(obj, arg0);
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

- **New_TimBlockSrc**, tier B. Allocator for gTimBlockSrcMethods; class named for its own mechanics (see TimBlockSrc__TimBlockSrc).

## Track 4 (2026-09-25, round 83, bravo)

The allocator; 0x84 is the class size the header records. It now reaches the ctor through the typed getter (`GetTimBlockSrcMethods()->ctor(obj, (char *)arg0)`) instead of the unit's `UnprototypedCtorTable` cast; its own signature is unchanged because `src/class_3bb8c_k.c` declares it `s32 New_TimBlockSrc(s32)` locally. The class (id 0xF03, table `gTimBlockSrcMethods`) is unified as `TimBlockSrc` in `include/TimBlockSrc.h`. Any source block above is the pre-unification spelling; the live body in `src/GraphicsResources.c` takes the unified types, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Round 93 polish (charlie, track 7)

### Naming

| old | new | tier | evidence |
| --- | --- | --- | --- |
| `arg0` | `name` | A | passed straight to the ctor as its `char *name` (the file TimBlockSrc__TimBlockSrc opens); still `s32` because include/TimBlockSrc.h declares `New_TimBlockSrc(s32)` |
| `0x84` | `sizeof(TimBlockSrc)` | A | each equals the object size the class header records (and the allocation retail makes); the image is byte-identical |

### The unit banner, moved here from src/GraphicsResources.c

Verbatim as it stood before the round-93 comment pass; the new banner says what the file holds.

```c
/*
 * GraphicsResources -- GAME code carved from the head of psyq_33808 on 2026-09-25
 * (FINISHING-PLAN revision 18). 0x33808..0x36654 (vram
 * 0x80043008..0x80045E54). It was counted as Psy-Q SDK by segment name;
 * tools/gameinsdk.py measured it as game (a call into game code, a method-
 * table entry beside game methods, or contiguity with those, and no Sony
 * fingerprint). All 97 functions matched in round 82; named in round 83
 * (track 3 naming pass).
 *
 * Eleven method tables, nine of them FileResource (data-source) subclasses
 * reached through nine of gFileResourceMethods's own `Get...Methods` getter slots
 * (from +0x07C):
 *
 *   - TimBlockSrc  (gTimBlockSrcMethods): a sector-header + block loader with four
 *     CLUT palette-fade channels (FadeClutRow).
 *   - LinkResource (gLinkResourceMethods): a NULL-ended array of TMD models
 *     (New_TmdModel), one per object of a loaded TMD (include/LinkResource.h,
 *     track 4, round 89).
 *   - TimArraySrc  (gTimArraySrcMethods): an array of TimImage objects
 *     (TimImage.c's New_TimImage), one per TimBlockSrc block
 *     (include/TimArraySrc.h, track 4, round 88).
 *   - Tod / TodSet (gTodMethods / gTodSetMethods, TodSet a Tod subclass): one
 *     TOD's packet stream (ScanTodPackets/DecodeTodPacketWord) and an array
 *     of them; named from src/TodActor.c's own "TOD set" (Unk30Obj).
 *   - ModelData / TriggerWorld (gModelDataMethods / gTriggerWorldMethods, TriggerWorld a
 *     ModelData subclass): a LinkResource+TodSet pair, and an array of
 *     those pairs; ModelData named from TodActor.c/.c's own "tmd"/"tods"/
 *     "modelData" fields, TriggerWorld from DreamAux.c's own declared
 *     return type.
 *   - TileMap / TileAtlas (gTileMapMethods / gTileAtlasMethods): a 20x15 grid of
 *     16x16-cell map data (a GsMAP, consumed by BgLayer as its map source)
 *     and the 300-GsCELL texture atlas it indexes; built together and used
 *     together in src/Task.c's TaskCore__TaskCore (include/TileMap.h,
 *     include/TileAtlas.h, track 4, round 88).
 *
 * Two more classes, not FileResource subclasses:
 *
 *   - BgLayer (gBgLayerMethods): a SceneNode subclass wrapping one GsBG
 *     scrolling background layer (its own fields are GsBG's own layout;
 *     include/BgLayer.h, track 4, round 88).
 *   - MoviePlayer (gMoviePlayerMethods): a BasicClass subclass driving CD-streamed,
 *     MDEC-decoded FMV playback (open a CD stream, decode/upload strips,
 *     play/stop/tick controls); called from Task.c
 *     (include/MoviePlayer.h, track 4, round 89).
 *
 * libpress starts right after, at DecDCTReset (now psyq_36654).
 */
```
