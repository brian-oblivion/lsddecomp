# ItemList__RemoveChild -- MATCHED (32/32 words)

> Renamed from `Class86F88__RemoveChild` on 2026-09-26 (tools/rename.py). Address 0x80051da0.

> Renamed from `ItemList_3bb8c_j__RemoveChild` on 2026-09-24 (tools/rename.py). Address 0x80051da0.

> Renamed from `func_80051DA0` on 2026-09-24 (tools/rename.py). Address 0x80051da0.

Unit: `src/ui/TextEntryItemList.c`. `self` is `ItemList_3bb8c_j`. The "remove child"
counterpart to `ItemList__AddChild`: clears whichever of the two tagged caches
`arg1` matches, THEN unregisters it from the inherited
`BasicClass::removeChild` (order reversed from `ItemList__AddChild`'s
add/tag-check order -- see that function's report).

## Body

```c
void ItemList__RemoveChild(ItemList_3bb8c_j *self, void *arg1)
{
    s32 tag;

    if (arg1) {
        tag = **(s32 **)arg1 & 0xF;
        if (tag == 2) {
            self->unk34 = NULL;
        } else if (tag == 5) {
            self->unk38 = NULL;
        }
        GetBasicClassMethods()->removeChild(self, arg1);
    }
}
```

Matched first try.

## Naming

- `ItemList__RemoveChild` -- tier A. The removeChild occupant (classtable.py gItemListMethods +0x014): clears the matching tag cache then chains the base removeChild. Mirrors ItemList__AddChild.

## Track 7 (round 100, charlie)

The child's kind is read as `((BasicClass *)x)->methods->header &
CLASS_ID_ROOT_MASK` and compared with PAD_CLASS_ID/FRAMECLOCK_CLASS_ID
(were a raw `**(s32 **)x & 0xF` against 2 and 5). Zero bytes changed.
