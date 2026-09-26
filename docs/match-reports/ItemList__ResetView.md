# ItemList__ResetView -- MATCHED (4/4 words)

> Renamed from `Class86F88__ResetView` on 2026-09-26 (tools/rename.py). Address 0x80051f14.

> Renamed from `ItemList__ResetCounters` on 2026-09-26 (tools/rename.py). Address 0x80051f14.

> Renamed from `ItemList_3bb8c_j__ResetCounters` on 2026-09-24 (tools/rename.py). Address 0x80051f14.

> Renamed from `func_80051F14` on 2026-09-24 (tools/rename.py). Address 0x80051f14.

Unit: `src/class_3bb8c_j.c`. `self` is `ItemList_3bb8c_j`.

## Body

```c
void ItemList__ResetView(ItemList_3bb8c_j *self)
{
    self->unk20 = 0;
    self->unk24 = 0;
    self->unk28 = 0;
}
```

Trivial three-field reset, an unrelated field group from `ItemList__ClearCachedRefs`'s
(`unk34`/`unk38`/`unk50`). Matched first try.

## Naming

- `ItemList__ResetView` -- tier A. The slot40 occupant (classtable.py gItemListMethods +0x040, the ctor's own tail dispatch): zeroes unk20/unk24/unk28. A pure leaf, tier A by the track-3 rule.

## Track 4 (2026-09-26, round 89)

Renamed from `ItemList__ResetCounters`: the three words it zeroes are
`topIndex`, `column` and `cursorIndex` (+0x020..+0x028, named from
class_3bb8c_k's accessors), the trio ItemList__SetView sets. They are
the list's view position, not counters.
