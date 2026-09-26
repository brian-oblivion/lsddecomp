# ItemList__ReleaseResources -- MATCHED (28/28 words)

> Renamed from `Class86F88__ReleaseResources` on 2026-09-26 (tools/rename.py). Address 0x800520a0.

> Renamed from `ItemList_3bb8c_j__ReleaseResources` on 2026-09-24 (tools/rename.py). Address 0x800520a0.

> Renamed from `func_800520A0` on 2026-09-24 (tools/rename.py). Address 0x800520a0.

Unit: `src/class_3bb8c_j.c`. `self` is `ItemList_3bb8c_j`.

## Body

```c
void ItemList__ReleaseResources(ItemList_3bb8c_j *self)
{
    if (self->unk50) {
        self->methods->slot90(self);
        self->unk50 = self->unk50->methods->slot4(self->unk50);
    }
}
```

Established `self->unk50`'s type as `ItemListHandle_3bb8c_j *` (already
introduced this round from `ItemList__LoadResources`'s evidence) and
`ItemListMethods_3bb8c_j::slot90` (+0x090, self-only). `ItemListHandleMethods_3bb8c_j::
slot4` (+0x004, self-only, returns a handle-typed pointer stored back into
`self->unk50` -- a "release, returns the successor/NULL" shape) was
already declared for `ItemList__LoadResources`'s use; this is the second confirming
call site. Matched first try.

## Naming

- `ItemList__ReleaseResources` -- tier B. The slot48 occupant (classtable.py gItemListMethods +0x048): if unk50 is set, calls slot90(self) then releases unk50 through its own slot4. Mirrors ItemList__LoadResources's load in reverse -- mechanics clear, purpose not established beyond "release what LoadResources acquired".
