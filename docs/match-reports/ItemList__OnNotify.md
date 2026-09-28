# ItemList__OnNotify -- MATCHED (44/44 words)

> Renamed from `Class86F88__OnNotify` on 2026-09-26 (tools/rename.py). Address 0x80051e64.

> Renamed from `ItemList__NotifyChild` on 2026-09-26 (tools/rename.py). Address 0x80051e64.

> Renamed from `ItemList_3bb8c_j__NotifyChild` on 2026-09-24 (tools/rename.py). Address 0x80051e64.

> Renamed from `func_80051E64` on 2026-09-24 (tools/rename.py). Address 0x80051e64.

Unit: `src/ui/TextEntryItemList.c`. `self` is `ItemList_3bb8c_j`.

## Body

```c
void ItemList__OnNotify(ItemList_3bb8c_j *self, void *arg1, s32 arg2)
{
    s32 tag;

    GetBasicClassMethods()->slot38(self, arg1, arg2);
    tag = **(s32 **)arg1 & 0xF;
    if (tag == 2) {
        self->methods->slot5C(self, arg1, arg2);
    } else if (tag == 5) {
        self->methods->slot58(self, arg1, arg2);
    }
}
```

Calls the inherited `BasicClass::slot38` (already declared in
`include/class_3bb8c.h`'s `BasicMethods866E8F`, established by
TitleMenuTaskObjF) unconditionally first, then dispatches through ItemList_3bb8c_j's
OWN vtable (`slot5C`/`slot58`) based on the same tag-nibble convention as
`ItemList__AddChild`/`ItemList__RemoveChild`. Established `ItemListMethods_3bb8c_j::slot5C`
(+0x05C, "tag==2") and `slot58` (+0x058, "tag==5") from this function.
Matched first try.

## Naming

- `ItemList__OnNotify` -- tier B. The slot38 occupant (classtable.py gItemListMethods +0x038, the offset every sibling class in this header uses for its own "OnNotify"-shaped override): chains the base slot38 unconditionally, then dispatches to this class's own slot5C/slot58 by the child's tag nibble. Mechanics (notify then tag-dispatch) are clear; the in-game meaning of the notification is not.

## Track 4 (2026-09-26, round 89)

Renamed from `ItemList__NotifyChild`: it occupies gItemListMethods
+0x038, BasicClass's `onNotify` slot (`classtable.py gItemListMethods --vs
gBasicClassMethods`: OVERRIDDEN BasicClass__OnNotify), and it chains
`GetBasicClassMethods()->onNotify` first, so the override takes the slot's
name (FINISHING-PLAN track 4 step 6). The tag-2 child's notifications go to
`handleInputCode` (+0x05C), the tag-5 child's to `tickClosing` (+0x058).

## Track 7 (round 100, charlie)

The child's kind is read as `((BasicClass *)x)->methods->header &
CLASS_ID_ROOT_MASK` and compared with PAD_CLASS_ID/FRAMECLOCK_CLASS_ID
(were a raw `**(s32 **)x & 0xF` against 2 and 5). Zero bytes changed.
