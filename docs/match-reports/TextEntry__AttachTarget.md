# TextEntry__AttachTarget -- MATCHED (28/28 words)

> Renamed from `Obj86ED0__AttachTarget` on 2026-09-26 (tools/rename.py). Address 0x80051200.

> Renamed from `func_80051200` on 2026-09-24 (tools/rename.py). Address 0x80051200.

Unit `TextEntryItemList`, carved round 14.

`Obj86ED0`'s vtable slot 0x04C: adds two children via `self->methods->
addChild` (the class's OWN overridden addChild, `TextEntry__AddChild`, reached
through `self->methods` rather than the explicit base-table call this
time), stashes a third pointer verbatim into `unk3C` (typed `TargetObj86ED0
*`, the same opaque type `TextEntry__PlaySound` dispatches through), and resets
two counters.

```c
void TextEntry__AttachTarget(Obj86ED0 *self, void *arg1, void *arg2, TargetObj86ED0 *arg3)
{
    self->methods->addChild(self, arg1);
    self->methods->addChild(self, arg2);
    self->unk3C = arg3;
    self->unk2C = 0;
    self->unk20 = 0;
}
```

First attempt, transcribed directly (order matters: both `addChild` calls
before any of the three stores), matched immediately once the earlier
in-unit drift (from `TextEntry__OnNotify`/`TextEntry__SetText`, fixed first) was
resolved -- this function's own C never changed.

## Naming

- `TextEntry__AttachTarget` -- tier B. gTextEntryMethods +0x04C (classtable.py). Adds two children via self->methods->addChild, stores a third pointer as `target`, resets closeState/unk20. Mechanics clear (bind two tagged children plus a dispatch target); the game-level reason this bundle is attached together is not established.

## Track 4 (2026-09-26, round 87)

Class unified as `TextEntry` (include/TextEntry.h; table gObj86ED0Methods -> gTextEntryMethods, type Obj86ED0 -> TextEntry). The class name is for what its methods do: setText keeps a caller's string buffer and a working copy, the cursor and char methods edit the copy, command 25 writes it back, 23 closes without writing (banner of include/TextEntry.h). Fields renamed from their accessors: unk14 charCount, unk1C charIndex, unk20 altCommands, nameLen textLen, nameBuf textBuf, unk28 editBuf, unk40 cursorSprite (CharSprite *), unk44 textRow, unk48 panelSprite (ScreenSprite *). Zero bytes changed.

## Round 94 (track 6, charlie): the target is a VabStreamObj

`TargetObj86ED0`/`TargetMethods86ED0` (include/class_3bb8c.h) are deleted.
Both attachTarget callers (TaskObjF, src/class_3bb8c_g.c) pass TaskObjF's
`sound`, already typed `struct VabStreamObj *`, and the one slot the view
named, +0x080, is VabStreamObj's `playTone(self, index, vol, endVol)`
(include/VabStreamObj.h): the `(code, 0x60, 0x60)` call plays tone `code`
at volume 0x60. TextEntry::target and ItemList::target (+0x03C) and both
attachTarget prototypes are `struct VabStreamObj *`, the casts at the call
sites are gone, and `slot80` is `playTone`. Zero bytes changed.

## Track 7 (2026-09-27, round 98, bravo)

Parameters `arg1`/`arg2`/`arg3` -> `inputSource`/`tickSource`/`target`:
the first two are the Pad and the FrameClock addChild files into the
fields of those names. Zero bytes changed.
