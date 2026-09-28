# StyleEffect__ReleaseSprites -- MATCHED (9/9 words)

> Renamed from `Class876FC__ReleaseSprites` on 2026-09-26 (tools/rename.py). Address 0x80056df8.

> Renamed from `LinkOwnerObj__ReleaseLinks` on 2026-09-26 (tools/rename.py). Address 0x80056df8.

> Renamed from `func_80056DF8` on 2026-09-18 (tools/rename.py). Address 0x80056df8.

Unit: `ObjMStyleActor` (round 17). A one-line wrapper releasing a 5-element
`BasicClass *` array inline at `self+0x84`.

## Final source

```c
typedef struct LinkElemObj LinkElemObj;
typedef struct LinkElemMethods {
    u8 pad0[0x48];
    void (*slot48)(LinkElemObj *self, s32 arg1, Vec3O *arg2); /* +0x048 */
} LinkElemMethods;
struct LinkElemObj {
    LinkElemMethods *methods; /* +0x000 */
    u8 pad4[0x80];             /* +0x004 .. +0x083, unknown */
    s32 unk84;                   /* +0x084 */
};

typedef struct LinkOwnerObj {
    u8 pad0[0x84];              /* +0x000 .. +0x083, unknown */
    LinkElemObj *arr84[5];        /* +0x084 .. +0x097 */
} LinkOwnerObj;

extern void ReleaseBasicClassArray(void **array, s32 count);

void StyleEffect__ReleaseSprites(LinkOwnerObj *this) {
    ReleaseBasicClassArray((void **)this->arr84, 5);
}
```

## Derivation

`addiu $a0, $a0, 0x84` then `jal ReleaseBasicClassArray` with `$a1 = 5` -- the
address of `this+0x84` is passed directly (not loaded through it), so
`+0x84` is an INLINE array field, not a pointer field. `ReleaseBasicClassArray` is
already established elsewhere (`TmdRenderer.c`) as
`void ReleaseBasicClassArray(BasicClass **array, s32 count)` -- a release-all-N loop.
Kept generic `void **` here rather than pulling in `BasicClass` from
`code_8220.h`, matching this project's existing looser per-unit reading of
the same symbol (`Task.h`'s `void ReleaseBasicClassArray(void *a0, void *a1)`).

`this` is NOT the same class as `BaseObjO` (the shared intermediate base
class the rest of this unit implements, see the file banner) -- `+0x84`
would overflow that class's 0x58-byte allocation (`New_Actor`). It is
kept as its own independent local type, `LinkOwnerObj`, established
together with `StyleEffect__RandomizeSprites` (which walks indices 1..4 of the SAME
5-element array) and `StyleEffect__ReleaseSpritesB` (byte-identical body to this
function).

### Proposed learning

None beyond what's already documented -- straightforward wrapper.

## Naming

**`StyleEffect__ReleaseSprites` -- tier A.** Mechanics ARE the purpose: the
whole body is `ReleaseBasicClassArray(this->links, 5)`, i.e. "release [all
of] this object's links". `LinkOwnerObj` is this unit's own established
local view of the class `ObjMStyleActor.c` independently calls `LinkNode`
(same object, same `arr84`/`links` array, per that unit's own header
comment) -- kept distinct per the multiple-independent-local-views
convention rather than importing that name here.

## Track 4 (2026-09-26, round 88, charlie)

Renamed from the `LinkOwnerObj__` family to `StyleEffect__` with the class's
unification (`include/StyleEffect.h`). Evidence: the only caller is
StyleEffect's own per-kind dispatch in `ObjMStyleActor.c`
(`StyleEffect__InitByKind` kind 3, `StyleEffect__UpdateByKind` kind 3,
`StyleEffect__ReleaseByKind` kinds 2/3), each passing its own `self`; the
five-element array at +0x084 ("links") is `StyleEffect::sprites`, filled by
`StyleEffect__SpawnSprites` with `New_VariantSprite` objects. The old
`LinkOwnerObj`/`LinkElemObj` views were StyleEffect and VariantSprite under
another name; RandomizeSprites' `slot48` is VariantSprite's inherited
`updateScale` and its `angle` (+0x084) is `sprite.rotate` (Sprite, +0x064 +
0x020, 4096 per degree -- the `(rand() % 360) << 12` it stores).

View replaced the same day: the `LinkOwnerObj`/`LinkElemObj` views in ObjMStyleActor.c are deleted and the unit includes include/StyleEffect.h (`this` is `StyleEffect *self`; `links` is `sprites`, `slot48` is `updateScale`, `angle` is `sprite.rotate`). Image byte-identical.

## Track 7 (round 99, alpha)

The count is ARRAY_COUNT(self->sprites) (5); a comment says kind 2 releases through it and kind 3 through the identical StyleEffect__ReleaseSpritesB.
