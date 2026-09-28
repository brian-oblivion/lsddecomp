# GetItemListMethods -- MATCH

> Renamed from `GetClass86F88Methods` on 2026-09-26 (tools/rename.py). Address 0x80052b60.

> Renamed from `func_80052B60` on 2026-09-24 (tools/rename.py). Address 0x80052b60.

Unit `ObjMStyleActor`, round 15. `./build-and-verify.sh` exit 0; whole-image
SHA1 matches retail. `funcdiff.py GetItemListMethods`: 4/4 words match.

## Source

```c
ItemListMethods *GetItemListMethods(void)
{
    return &gItemListMethods;
}
```

## Notes

A plain no-argument accessor returning `&gItemListMethods`, the same shape as
`GetTimedTaskMethods`/`gTimedTaskMethods` documented elsewhere in this project (base
class table getters). `gItemListMethods` is this unit's own class's vtable
(39 slots, `tools/classtable.py gItemListMethods`); the words at `+0x054`,
`+0x058`, `+0x07C`..`+0x09C` of that table are this unit's own
`ItemList__SetState`/`ItemList__TickClosing`/`ItemList__ScrollRight`.../`ItemList__GetCursorIndex`, all
matched this round. Matched first attempt.

## Naming

Round 75 (bravo, track 3). `func_80052B60` -> `GetItemListMethods`, **tier A**.

Returns &gItemListMethods. Used as the ctor table by New_ItemList and ItemList__ItemList (TextEntryItemList). Named like GetTimedTaskMethods/GetObjMMethods.
