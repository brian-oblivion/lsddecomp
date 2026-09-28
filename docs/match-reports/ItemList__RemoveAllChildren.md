# ItemList__RemoveAllChildren -- MATCHED (17/17 words)

> Renamed from `Class86F88__RemoveAllChildren` on 2026-09-26 (tools/rename.py). Address 0x80051e20.

> Renamed from `ItemList_3bb8c_j__RemoveAllChildren` on 2026-09-24 (tools/rename.py). Address 0x80051e20.

> Renamed from `func_80051E20` on 2026-09-24 (tools/rename.py). Address 0x80051e20.

Unit: `src/ui/input_dialogs.c`. `self` is `ItemList_3bb8c_j`. Full reset: clears
both tagged caches and `unk50` (same three fields as `ItemList__ClearCachedRefs`),
then chains to the inherited `BasicClass::removeAllChildren`.

## Body

```c
void ItemList__RemoveAllChildren(ItemList_3bb8c_j *self)
{
    self->unk34 = NULL;
    self->unk38 = NULL;
    self->unk50 = NULL;
    GetBasicClassMethods()->removeAllChildren(self);
}
```

Matched first try.

## Naming

- `ItemList__RemoveAllChildren` -- tier A. The removeAllChildren occupant (classtable.py gItemListMethods +0x018): clears unk34/unk38/unk50 then chains the base removeAllChildren. Matches the BasicClass convention.
