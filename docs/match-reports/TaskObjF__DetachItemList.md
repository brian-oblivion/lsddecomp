# TaskObjF__DetachItemList -- MATCH

> Renamed from `TaskObjF__DetachChildB` on 2026-09-26 (tools/rename.py). Address 0x80050670.

> Renamed from `Class86E00_3bb8c_g__DetachChildB` on 2026-09-23 (tools/rename.py). Address 0x80050670.

> Renamed from `func_80050670` on 2026-09-23 (tools/rename.py). Address 0x80050670.

Unit `TitleMenuTaskObjF`, round 14. `./build-and-verify.sh` exit 0; whole-image
SHA1 matches retail. `funcdiff.py TaskObjF__DetachItemList`: 48/48 words match.

## Source

```c
void TaskObjF__DetachItemList(Class86E00_3bb8c_g *self)
{
    if (self->unk68 != 0 && self->unk60 != 0 && self->unk7C != NULL) {
        self->unk7C->methods->slot50(self->unk7C);
        self->unk7C->methods->slot48(self->unk7C);
        if (self->unk74 != 0) {
            self->unk7C->methods->release(self->unk7C);
            self->unk7C = NULL;
        }
    }
}
```

First attempt, byte-exact. The exact twin of `TaskObjF__DetachTextEntry`, operating on
`unk7C` instead of `unk78` -- copied the sibling's exact idiom (re-derefed
field, not cached) directly.

## Struct changes (additive, `include/class_3bb8c.h`)

None new -- everything was already declared.

### Proposed learning

None new. A second confirming instance of "when a function closely
resembles an already-matched sibling, copy its exact idiom before
deriving anything" -- cheaper than re-deriving, as already documented.

## Naming

`TaskObjF__DetachItemList` (was `func_80050670`), tier B: the
teardown counterpart of `TaskObjF__AttachItemList`, exact twin of
`TaskObjF__DetachTextEntry` operating on `self->unk7C`.

## Track 4 (2026-09-26, round 89)

Renamed from `TaskObjF__DetachChildB`: the inverse of TaskObjF__AttachItemList on `itemList` (ItemList detachTarget +0x050, releaseResources +0x048, release when `ownsWidget`). Called by TaskObjF__OnItemListResult through +0x0AC.
