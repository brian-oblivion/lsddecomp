# TextEntry__AddChild -- MATCHED (33/33 words)

> Renamed from `Obj86ED0__AddChild` on 2026-09-26 (tools/rename.py). Address 0x80050d30.

> Renamed from `func_80050D30` on 2026-09-24 (tools/rename.py). Address 0x80050d30.

Unit `TextEntryItemList`, carved round 14.

`Obj86ED0`'s addChild override (vtable slot 0x010). Dispatches the BASE
class's own `addChild` first, then reads the new child's method-table
`header` word (matches `TaskObjFMethods::header`/`BasicClassMethods::header`
elsewhere in this project -- a per-class type tag, first word of every
BasicClass-family vtable) and stashes the child pointer into one of two
typed slots depending on the tag, mirroring the ALREADY-MATCHED
`TaskObjF__OnNotify` (`src/class_3bb8c_f.c`) which reads the identical tag the
identical way (`**(s32 **)arg1`, masked `& 0xF`).

```c
void TextEntry__AddChild(Obj86ED0 *self, void *arg1)
{
    s32 tag;
    s32 mask;

    if (arg1 != NULL) {
        Get_vtable_BasicClass()->addChild(self, arg1);
        tag = **(s32 **)arg1;
        mask = tag & 0xF;
        if (mask == 2) {
            self->unk34 = arg1;
        } else if (mask == 5) {
            self->unk38 = arg1;
        }
    }
}
```

First attempt, mirroring `TaskObjF__OnNotify`'s proven idiom, matched
immediately.

## Naming

- `TextEntry__AddChild` -- tier A. gTextEntryMethods +0x010 (classtable.py), overrides BasicClass's addChild: calls the base addChild, then reads the new child's type tag and stashes it into childType2/childType5 by mask. Mirrors the already-matched TaskObjF__OnNotify idiom.

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
