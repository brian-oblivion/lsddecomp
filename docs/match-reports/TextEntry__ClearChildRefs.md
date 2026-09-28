# TextEntry__ClearChildRefs -- MATCHED (4/4 words)

> Renamed from `Obj86ED0__ClearChildRefs` on 2026-09-26 (tools/rename.py). Address 0x80050cd8.

> Renamed from `func_80050CD8` on 2026-09-24 (tools/rename.py). Address 0x80050cd8.

Unit `input_dialogs`, carved round 14.

Trivial reset helper on `Obj86ED0` (see `include/class_3bb8c.h`): zeroes the
two child-slot fields and the release-gated pointer. Called both from this
class's own (STALLED, gp_rel-blocked) ctor `TextEntry__TextEntry` and standalone.

```c
void TextEntry__ClearChildRefs(Obj86ED0 *self)
{
    self->unk34 = NULL;
    self->unk38 = NULL;
    self->unk48 = NULL;
}
```

First attempt, straight transcription of the three `sw zero` stores in
order, matched immediately.

## Naming

- `TextEntry__ClearChildRefs` -- tier A. Pure leaf: nulls childType2/childType5/unk48. Called once from the ctor to establish the initial (empty) child-tracking state -- mechanics are the purpose, tier A by the pure-leaf rule.

## Track 4 (2026-09-26, round 87)

Class unified as `TextEntry` (include/TextEntry.h; table gObj86ED0Methods -> gTextEntryMethods, type Obj86ED0 -> TextEntry). The class name is for what its methods do: setText keeps a caller's string buffer and a working copy, the cursor and char methods edit the copy, command 25 writes it back, 23 closes without writing (banner of include/TextEntry.h). Fields renamed from their accessors: unk14 charCount, unk1C charIndex, unk20 altCommands, nameLen textLen, nameBuf textBuf, unk28 editBuf, unk40 cursorSprite (CharSprite *), unk44 textRow, unk48 panelSprite (ScreenSprite *). Zero bytes changed.

## Track 7 (2026-09-27, round 98, bravo)

TextEntry +0x034/+0x038 `childType2`/`childType5` -> `inputSource`/
`tickSource` (include/TextEntry.h; accessed only in input_dialogs, so
renamed in the definition). They are the children of class 2 and 5, which
are Pad and FrameClock (`typeviews.py --tree`), and ItemList and TaskObjF
already call the same pair `inputSource`/`tickSource`. The kind test reads
`((BasicClass *)child)->methods->header & CLASS_ID_ROOT_MASK` (main's define in include/basic_class.h, added verbatim) and compares with
`PAD_CLASS_ID`/`FRAMECLOCK_CLASS_ID` (new in include/pad.h and
include/frame_clock.h, TASKOBJF_CLASS_ID's form) instead of
`**(s32 **)child`. Zero bytes changed.
