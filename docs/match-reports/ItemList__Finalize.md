# ItemList__Finalize -- MATCHED (38/38 words)

> Renamed from `Class86F88__Finalize` on 2026-09-26 (tools/rename.py). Address 0x80051c84.

> Renamed from `ItemList_3bb8c_j__Finalize` on 2026-09-24 (tools/rename.py). Address 0x80051c84.

> Renamed from `func_80051C84` on 2026-09-24 (tools/rename.py). Address 0x80051c84.

Unit: `src/ui/TextEntryItemList.c`. `self` is `ItemList_3bb8c_j`. This is its destructor
body (dispatched through `BasicClass`'s inherited `finalize` slot chain,
`GetBasicClassMethods()->finalize`).

## Body

```c
void ItemList__Finalize(ItemList_3bb8c_j *self)
{
    s32 i;

    for (i = 0; i < self->unk10; i++) {
        BMemPMgrFree(self->unk18[i]);
    }
    BMemPMgrFree(self->unk1C);
    BMemPMgrFree(self->unk18);
    GetBasicClassMethods()->finalize(self);
}
```

Frees every entry of the `self->unk18` pointer array (`self->unk10`
entries), then `self->unk1C` (a single pointer), then the array itself,
then chains to the inherited `BasicClass::finalize`. Matched first try --
the `for` loop naturally re-reads `self->unk10` from memory each
iteration (retail does too, since the intervening `BMemPMgrFree` call is
opaque to the compiler and could in principle touch it), matching the
already-documented "must reload after call" idiom.

## Naming

- `ItemList__Finalize` -- tier A. The finalize occupant (classtable.py gItemListMethods +0x00C): frees each unk18[i] buffer, then unk1C and unk18 themselves, then chains GetBasicClassMethods()->finalize(self). Matches the BasicClass finalize-slot convention used throughout this header.
