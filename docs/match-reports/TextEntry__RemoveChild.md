# TextEntry__RemoveChild -- MATCHED (32/32 words)

> Renamed from `Obj86ED0__RemoveChild` on 2026-09-26 (tools/rename.py). Address 0x80050db4.

> Renamed from `func_80050DB4` on 2026-09-24 (tools/rename.py). Address 0x80050db4.

Unit `TextEntryItemList`, carved round 14.

`Obj86ED0`'s removeChild override (vtable slot 0x014) -- the mirror image of
`TextEntry__AddChild`'s addChild override. Unlike the add side, the tag check runs
FIRST (clearing whichever of `unk34`/`unk38` matches, unconditionally to
`NULL` rather than checking it was actually the same pointer), then the
BASE class's `removeChild` is dispatched last, in the shared fallthrough of
both tag branches.

```c
void TextEntry__RemoveChild(Obj86ED0 *self, void *arg1)
{
    s32 tag;
    s32 mask;

    if (arg1 != NULL) {
        tag = **(s32 **)arg1;
        mask = tag & 0xF;
        if (mask == 2) {
            self->unk34 = NULL;
        } else if (mask == 5) {
            self->unk38 = NULL;
        }
        GetBasicClassMethods()->removeChild(self, arg1);
    }
}
```

First attempt matched immediately -- the only trick was getting the
tag-check-before-base-call ORDER right (confirmed by reading the raw
disassembly's instruction sequence, not assumed by symmetry with
TextEntry__AddChild).

## Naming

- `TextEntry__RemoveChild` -- tier A. gTextEntryMethods +0x014 (classtable.py), overrides BasicClass's removeChild: symmetric teardown of TextEntry__AddChild's tagging.

## Track 4 (2026-09-26, round 87)

Class unified as `TextEntry` (include/TextEntry.h; table gObj86ED0Methods -> gTextEntryMethods, type Obj86ED0 -> TextEntry). The class name is for what its methods do: setText keeps a caller's string buffer and a working copy, the cursor and char methods edit the copy, command 25 writes it back, 23 closes without writing (banner of include/TextEntry.h). Fields renamed from their accessors: unk14 charCount, unk1C charIndex, unk20 altCommands, nameLen textLen, nameBuf textBuf, unk28 editBuf, unk40 cursorSprite (CharSprite *), unk44 textRow, unk48 panelSprite (ScreenSprite *). Zero bytes changed.

## Track 7 (2026-09-27, round 98, bravo)

TextEntry +0x034/+0x038 `childType2`/`childType5` -> `inputSource`/
`tickSource` (include/TextEntry.h; accessed only in TextEntryItemList, so
renamed in the definition). They are the children of class 2 and 5, which
are Pad and FrameClock (`typeviews.py --tree`), and ItemList and TaskObjF
already call the same pair `inputSource`/`tickSource`. The kind test reads
`((BasicClass *)child)->methods->header & CLASS_ID_ROOT_MASK` (main's define in include/BasicClass.h, added verbatim) and compares with
`PAD_CLASS_ID`/`FRAMECLOCK_CLASS_ID` (new in include/Pad.h and
include/FrameClock.h, TASKOBJF_CLASS_ID's form) instead of
`**(s32 **)child`. Zero bytes changed.
