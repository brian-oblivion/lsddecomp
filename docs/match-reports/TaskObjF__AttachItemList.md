# TaskObjF__AttachItemList -- MATCH

> Renamed from `TaskObjF__AttachChildB` on 2026-09-26 (tools/rename.py). Address 0x800505a8.

> Renamed from `Class86E00_3bb8c_g__AttachChildB` on 2026-09-23 (tools/rename.py). Address 0x800505a8.

> Renamed from `func_800505A8` on 2026-09-23 (tools/rename.py). Address 0x800505a8.

Unit `title_menu`, round 14. `./build-and-verify.sh` exit 0; whole-image
SHA1 matches retail. `funcdiff.py TaskObjF__AttachItemList`: 50/50 words match.

## Source

```c
void TaskObjF__AttachItemList(Class86E00_3bb8c_g *self)
{
    if (self->unk68 != 0 && self->unk60 != 0) {
        if (self->unk7C == NULL) {
            self->unk7C = New_ItemList(self->unk38, 1);
            self->unk74 = 1;
        }
        self->methods->slot10(self, self->unk7C);
        self->unk7C->methods->slot44(self->unk7C, self->unk68);
        self->unk7C->methods->slot4C(self->unk7C, self->unk60, self->unk64, self->unk6C);
    }
}
```

First attempt, byte-exact. The exact twin of `TaskObjF__AttachTextEntry` (same guard
shape, same lazy-init-then-attach sequence), operating on `unk7C` instead
of `unk78` and calling `New_ItemList` instead of `New_TextEntry`. Unlike
`TaskObjF__AttachTextEntry`, `New_ItemList`'s second argument (`1`) is materialized
right in the `jal`'s own delay slot -- an ordinary, unremarkable argument
setup, not the "surprise 2nd parameter hoisted several instructions
early" residue `TaskObjF__AttachTextEntry` needed to diagnose. Applying that
function's already-corrected `slot44` arity (`self, s32 arg1`) here
directly is what made this one match cold.

## Struct changes (additive, `include/class_3bb8c.h`)

None new -- every slot and the `New_ItemList` extern were already
declared correctly (the `slot44` arity fix came from `TaskObjF__AttachTextEntry`'s
report, applied here without needing its own derivation).

### Proposed learning

None new. Worth noting as a case where reading a SIBLING function's match
report before starting paid off directly -- no repeat of the arity
mistake that cost `TaskObjF__AttachTextEntry` an extra attempt.

## Naming

`TaskObjF__AttachItemList` (was `func_800505A8`), tier B: the
"B" twin of `TaskObjF__AttachTextEntry` -- identical guard and
attach sequence, operating on `self->unk7C` via `New_ItemList` instead of
`self->unk78` via `New_TextEntry`. See `AttachChildA`'s naming note: the
A/B suffixes are positional labels, not an established functional split.

## Track 4 (2026-09-26, round 89)

Renamed from `TaskObjF__AttachChildB`. The child it makes is a ItemList, the list selector (New_ItemList(titles, 1), include/ItemList.h), kept in `itemList` (+0x07C, was `childB`), the slot TaskObjF__AddChild fills for a child of class id 0x20 (gItemListMethods). Its calls are ItemList's loadResources (+0x044) and attachTarget (+0x04C). SetState(0x12) calls it through +0x0A8.
