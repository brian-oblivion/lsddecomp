# GetTextEntryMethods -- MATCHED (4/4 words)

> Renamed from `Get_vtable_Obj86ED0` on 2026-09-26 (tools/rename.py). Address 0x80051a4c.

> Renamed from `func_80051A4C` on 2026-09-24 (tools/rename.py). Address 0x80051a4c.

Unit: `src/ui/TextEntryItemList.c`. ROUND 75 CORRECTION: this is NOT
`ItemList_3bb8c_j`'s own getter (an earlier round assumed so, since it was
the only table getter this unit's C had resolved at the time, and named the
whole sibling class after it -- see `ItemList__ItemList.md`
and `include/class_3bb8c.h`'s round-15 HEAD NOTEs on gTextEntryMethods/gItemListMethods
for that history). `tools/classtable.py gTextEntryMethods` places
`TextEntry__PrevChar` .. `TextEntry__SetCharAt` (this same
unit's own first six functions) at that table's +0x094..+0x0A8, and
`include/class_3bb8c.h` already types and shares the WHOLE table as
`Obj86ED0Methods`, established independently by TextEntryItemList from its own
call sites (`New_TextEntry` there is the actual `New_X` for THIS class,
allocating 0x4C bytes and dispatching its ctor through `->ctor(...)` on the
pointer this function returns). So this function is `Obj86ED0`'s own
table getter, simply DEFINED in this unit; `ItemList_3bb8c_j`'s real
table is gItemListMethods, reached instead through `GetItemListMethods()`
(ObjMStyleActor).

## Body

```c
Obj86ED0Methods *GetTextEntryMethods(void)
{
    return &gTextEntryMethods;
}
```

Plain address-of getter for `Obj86ED0`'s own vtable, same shape as
`GetNodeGuardedViewportMethods`/`GetGridCellMethods`/`GetSceneNodeMethods` already documented in
`include/class_3bb8c.h`. Matched first try.

## Naming

- `GetTextEntryMethods` -- tier A. Plain `return &gTextEntryMethods;` -- a table-getter's purpose IS its mechanics (a pure leaf returning a fixed vtable pointer), same shape as the project's other `Get_vtable_*`/`GetClass*Methods` accessors. Identity of gTextEntryMethods as Obj86ED0's table is classtable.py gTextEntryMethods (42 slots) cross-checked against TextEntryItemList's own already-shared struct.

## Track 4 (2026-09-26, round 87)

Class unified as `TextEntry` (include/TextEntry.h; table gObj86ED0Methods -> gTextEntryMethods, type Obj86ED0 -> TextEntry). The class name is for what its methods do: setText keeps a caller's string buffer and a working copy, the cursor and char methods edit the copy, command 25 writes it back, 23 closes without writing (banner of include/TextEntry.h). Fields renamed from their accessors: unk14 charCount, unk1C charIndex, unk20 altCommands, nameLen textLen, nameBuf textBuf, unk28 editBuf, unk40 cursorSprite (CharSprite *), unk44 textRow, unk48 panelSprite (ScreenSprite *). Zero bytes changed.
