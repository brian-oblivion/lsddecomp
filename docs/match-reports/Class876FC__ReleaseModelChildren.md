# Class876FC__ReleaseModelChildren -- MATCHED (12/12 words)

> Renamed from `func_80056B8C` on 2026-09-23 (tools/rename.py). Address 0x80056b8c.

Unit `class_3bb8c_s`. `self` is the owning `LinkNode`.

## Classification

Clean on all four carve-time screens. A guarded release of the fixed
2-element `arr7C` array, using the same `ReleaseBasicClassArray(void **array, s32
count)` already established in `code_8220_b.c` and reused (for the SIBLING
5-element `arr84` array) in `class_3bb8c_o.c`.

## Body

```c
void Class876FC__ReleaseModelChildren(LinkNode *self) {
    if (self->unk6C != 0) {
        ReleaseBasicClassArray((void **)self->arr7C, 2);
    }
}
```

### Proposed learning

None -- a plain guarded release, no residue.

## Naming

Round 70 (alpha). `func_80056B8C` -> `Class876FC__ReleaseModelChildren`, **tier A**.

Body: `if (modelChildLayout != 0) ReleaseBasicClassArray(modelChildren, 2)`,
the exact inverse of Class876FC__PlaceModelChildren's creation guard. Only
caller Class876FC__ReleaseByKind (kind 0), itself called from the dtor.

## Track 4 (2026-09-26, round 88, charlie)

class_3bb8c_s.c's `LinkNode` view (owner and children under one type) is gone: the owner is `Class876FC` (include/Class876FC.h), `modelChildren` are `Actor *`, `sprites` are `VariantSprite *`, and the local `Vec3S` is `LongVec3`. Accessor renames: `kind` is Actor's `pendingExtra` (+0x054, where the ctor stores it); `offset`/`rotation`/`scale`/`modelChildLayout`/`tableIndex`/`color`/`altColor` are `params.*`; slot `slotB8` is Actor's `setTranslation` on the owner and model children and Sprite's `setColor` on sprites; `*coord2 = 0` is `coord2->flg = 0`. Image byte-identical.
