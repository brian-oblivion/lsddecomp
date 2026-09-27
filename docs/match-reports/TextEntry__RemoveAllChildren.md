# TextEntry__RemoveAllChildren -- MATCHED (17/17 words)

> Renamed from `Obj86ED0__RemoveAllChildren` on 2026-09-26 (tools/rename.py). Address 0x80050e34.

> Renamed from `func_80050E34` on 2026-09-24 (tools/rename.py). Address 0x80050e34.

Unit `class_3bb8c_i`, carved round 14.

`Obj86ED0`'s removeAllChildren override (vtable slot 0x018): zeroes the
same three fields `TextEntry__ClearChildRefs` zeroes, then dispatches the BASE class's
own `removeAllChildren`. The three `sw zero` stores land BEFORE the `jal`
in program order even though one of them physically sits in the branch/call
delay slot -- that is pure instruction scheduling, not a source-order
question.

```c
void TextEntry__RemoveAllChildren(Obj86ED0 *self)
{
    self->unk34 = NULL;
    self->unk38 = NULL;
    self->unk48 = NULL;
    Get_vtable_BasicClass()->removeAllChildren(self);
}
```

First attempt, straight transcription, matched immediately.

## Naming

- `TextEntry__RemoveAllChildren` -- tier A. gTextEntryMethods +0x018 (classtable.py), overrides BasicClass's removeAllChildren: zeroes the same three fields TextEntry__ClearChildRefs zeroes, then dispatches the base class's removeAllChildren.

## Track 4 (2026-09-26, round 87)

Class unified as `TextEntry` (include/TextEntry.h; table gObj86ED0Methods -> gTextEntryMethods, type Obj86ED0 -> TextEntry). The class name is for what its methods do: setText keeps a caller's string buffer and a working copy, the cursor and char methods edit the copy, command 25 writes it back, 23 closes without writing (banner of include/TextEntry.h). Fields renamed from their accessors: unk14 charCount, unk1C charIndex, unk20 altCommands, nameLen textLen, nameBuf textBuf, unk28 editBuf, unk40 cursorSprite (CharSprite *), unk44 textRow, unk48 panelSprite (ScreenSprite *). Zero bytes changed.

## Track 7 (2026-09-27, round 98, bravo)

TextEntry +0x034/+0x038 `childType2`/`childType5` -> `inputSource`/
`tickSource` (include/TextEntry.h; accessed only in class_3bb8c_i, so
renamed in the definition). They are the children of class 2 and 5, which
are Pad and FrameClock (`typeviews.py --tree`), and ItemList and TaskObjF
already call the same pair `inputSource`/`tickSource`. The kind test reads
`((BasicClass *)child)->methods->header & 0xF` and compares with
`PAD_CLASS_ID`/`FRAMECLOCK_CLASS_ID` (new in include/Pad.h and
include/FrameClock.h, TASKOBJF_CLASS_ID's form) instead of
`**(s32 **)child`. Zero bytes changed.
