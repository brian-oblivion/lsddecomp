# StyleEffect__ReleaseByKind -- MATCHED (31/31 words)

> Renamed from `Class876FC__ReleaseByKind` on 2026-09-26 (tools/rename.py). Address 0x80056718.

> Renamed from `func_80056718` on 2026-09-23 (tools/rename.py). Address 0x80056718.

Unit `class_3bb8c_s`. `self` is this unit's local `LinkNode` (see the unit's
own file banner / `class_3bb8c_s.c` for the full type; kept local per the
multiple-independent-local-views convention, not shared with
`class_3bb8c_o.c`'s `LinkOwnerObj`).

## Classification

Clean on all four carve-time screens. Trivial once the dispatch shape was
clear: a `switch` on `self->unk54` (the same state field `StyleEffect__InitByKind` and
`StyleEffect__UpdateByKind`, its two siblings in this unit, both also switch on) with
four cases, two of which forward straight into `class_3bb8c_o.c`'s
`StyleEffect__ReleaseSprites`/`StyleEffect__ReleaseSpritesB`.

## Body

```c
void StyleEffect__ReleaseByKind(LinkNode *self) {
    switch (self->unk54) {
    case 0:
        StyleEffect__ReleaseModelChildren(self);
        break;
    case 2:
        StyleEffect__ReleaseSprites(self);
        break;
    case 3:
        StyleEffect__ReleaseSpritesB(self);
        break;
    default:
        break;
    }
}
```

## Notes

`StyleEffect__InitByKind`/`StyleEffect__UpdateByKind` (still `INCLUDE_ASM`, `gp_rel`-blocked) switch
on the SAME `self->unk54` field with DIFFERENT case->callee mappings -- read
as three separate per-phase handlers (e.g. update/draw/free) sharing one
state selector, not three views of the same table. Do not assume they share a
callee list.

### Proposed learning

None beyond what's already documented -- a clean, ordinary dispatch.

## Naming

Round 70 (alpha). `func_80056718` -> `StyleEffect__ReleaseByKind`, **tier A**.

Body releases exactly the child array each kind built (kind 0 ->
StyleEffect__ReleaseModelChildren, 2 and 3 -> StyleEffect__ReleaseSprites[B],
both `ReleaseBasicClassArray(self+0x84, 5)`), and its only caller is the
class's dtor `StyleEffect__Finalize` (table +0x00C), which calls it before chaining
to the base dtor. Body and caller agree.

## Track 4 (2026-09-26, round 88, charlie)

class_3bb8c_s.c's `LinkNode` view (owner and children under one type) is gone: the owner is `StyleEffect` (include/StyleEffect.h), `modelChildren` are `Actor *`, `sprites` are `VariantSprite *`, and the local `Vec3S` is `LongVec3`. Accessor renames: `kind` is Actor's `pendingExtra` (+0x054, where the ctor stores it); `offset`/`rotation`/`scale`/`modelChildLayout`/`tableIndex`/`color`/`altColor` are `params.*`; slot `slotB8` is Actor's `setTranslation` on the owner and model children and Sprite's `setColor` on sprites; `*coord2 = 0` is `coord2->flg = 0`. Image byte-identical.

## Naming (track 7, round 101)

- Cases are StyleEffectKind's members (include/StyleEffect.h).
