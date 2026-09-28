# BgLayer__SetColor -- MATCHED (10/10 words)

> Renamed from `func_8004464C` on 2026-09-25 (tools/rename.py). Address 0x8004464c.

Round 82, runner echo (graphics_resources session, echo #6), 2026-09-25. Unit `graphics_resources`.
Byte-exact on the SECOND build (first-build miss described below); whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 10/10 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

`if (enable) self->unk54 = *src;` with a 3 x s8 struct. All three `lb` issue before any `sb`, and the three loads need v0/v1/a0, which is why self is moved to a3 in the branch delay slot. FIRST build used three explicit s8 field copies: `lbu`/nop/`sb` interleaved, 3 words LONGER (13 vs 10); the whole-struct assignment matched on the second build.

Table slot (`tools/classtable.py`): gBgLayerMethods +0x0B8 (a SceneNode subclass, not a data source).

## Source

The unit-local view `DataSrc33808` (a FileResource subclass built with the unified
`FILERESOURCE_SLOTS`/`FILERESOURCE_FIELDS` macros, plus `slot7C`/`slot80`, and own
fields +0x2C..+0x38) and `SubBlockTable` sit at the top of `src/graphics/graphics_resources.c`.
Slot +0x078 is `void *slot78` in the unified macro, so calls cast it.

```c
/* gBgLayerMethods (a SceneNode subclass) +0x0B8: when `enable`, copy a
 * three-byte vector to +0x54. */
typedef struct Vec3S8 {
    s8 x;
    s8 y;
    s8 z;
} Vec3S8;

typedef struct Obj6F2C4 {
    SCENENODE_FIELDS(SceneNodeMethods);
    /* +0x044 */ u8 pad44[0x10];
    /* +0x054 */ Vec3S8 unk54;
} Obj6F2C4;

void BgLayer__SetColor(Obj6F2C4 *self, s32 enable, Vec3S8 *src) {
    if (enable) {
        self->unk54 = *src;
    }
}
```

## Notes

- No shared header was edited. `FileResource.h`, `scene_node.h`, `basic_class.h` are
  included; prototypes for other units' functions (GetActiveDataSourceMethods,
  ReleaseBasicClassArray, BMemPMgrFree) are local to the unit.
- Types of arguments and returns are readings of the registers used, not proven.

### Proposed learning

A 3-byte copy whose three `lb` all issue before the three `sb` (and which
evicts a0 to make room) is a whole-struct assignment of a `{ s8 x, y, z; }`
struct; three field-by-field copies compile to `lbu`/nop/`sb` triples, three
words longer. Extends the "four lw then four sw" struct-copy idiom to byte
structs, where the signed load is the tell.

## Naming

- **BgLayer__SetColor**, tier B. Slot +0x0B8: conditionally copies a 3-byte vector into the GsBG's colour field.

## Track 4 (2026-09-26, round 88, alpha)

Class unified in `include/BgLayer.h`: this is its own slot +0x0B8 `setColor`. `self` is `BgLayer *`, the source is `BgLayerRgb *rgb` (was `Vec3S8 *src`), the field `color` (was unk54). The TaskCore callers (TaskCore__OnInit, TaskCore__TickFadeIn, TaskCore__TickFadeOut) now call it by name, casting their u8[3] buffers to `BgLayerRgb *` (no code). Byte-identical.
