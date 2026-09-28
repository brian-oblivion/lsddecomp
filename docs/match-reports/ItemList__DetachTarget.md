# ItemList__DetachTarget -- MATCHED (22/22 words)

> Renamed from `Class86F88__DetachTarget` on 2026-09-26 (tools/rename.py). Address 0x8005217c.

> Renamed from `ItemList__RemoveCachedChildren` on 2026-09-26 (tools/rename.py). Address 0x8005217c.

> Renamed from `ItemList_3bb8c_j__RemoveCachedChildren` on 2026-09-24 (tools/rename.py). Address 0x8005217c.

> Renamed from `func_8005217C` on 2026-09-24 (tools/rename.py). Address 0x8005217c.

Unit: `src/ui/TextEntryItemList.c`. `self` is `ItemList_3bb8c_j`.

## Body

```c
void ItemList__DetachTarget(ItemList_3bb8c_j *self)
{
    self->methods->slot14(self, self->unk34);
    self->methods->slot14(self, self->unk38);
    self->unk3C = 0;
}
```

Calls `ItemListMethods_3bb8c_j::slot14` (established this round from
`ItemList__AddChild`'s asm, self+one pointer arg) twice, once per tagged-child
cache, then clears `self->unk3C` (also established this round, from
`ItemList__AttachTarget`). Matched first try.

## Naming

- `ItemList__DetachTarget` -- tier A. The slot50 occupant (classtable.py gItemListMethods +0x050): calls slot14 (this class's own removeChild override) on unk34 then unk38, then clears unk3C. Purpose (release the two tag-cached children) follows directly from the mechanics, matching the addChild/removeChild caching pattern established in ItemList__AddChild -- tier A.

## Track 4 (2026-09-26, round 89)

Renamed from `ItemList__RemoveCachedChildren`: the body is
TextEntry__DetachTarget's at the same slot (+0x050), removeChild on the two
cached children then `target = NULL`, and it undoes ItemList__AttachTarget.
Its caller TaskObjF__DetachItemList drives it where TaskObjF__DetachTextEntry drives
TextEntry's detachTarget.
