# TextEntry__OnNotify -- MATCHED (44/44 words)

> Renamed from `Obj86ED0__Notify` on 2026-09-26 (tools/rename.py). Address 0x80050e78.

> Renamed from `func_80050E78` on 2026-09-24 (tools/rename.py). Address 0x80050e78.

Unit `TextEntryItemList`, carved round 14.

`Obj86ED0`'s slot38 override (vtable slot 0x038, the last slot BasicClass
itself defines). Dispatches the BASE class's own `slot38` first, then reads
the tag word off `arg1` exactly like `TextEntry__AddChild`/`TextEntry__RemoveChild`, and
routes to one of this class's OWN two extra slots (`slot58`/`slot5C`,
0x058/0x05C) via `self->methods` this time (not the base table) --
`slot5C` is itself `TextEntry__HandleCommand`, STALLED in this same unit (addiu-$at /
jump-table blocker, see its own report), so its type only needed naming,
not a body.

```c
void TextEntry__OnNotify(Obj86ED0 *self, void *arg1, s32 arg2)
{
    s32 tag;
    s32 mask;

    GetBasicClassMethods()->slot38(self, arg1, arg2);

    tag = **(s32 **)arg1;
    mask = tag & 0xF;
    if (mask == 2) {
        self->methods->slot5C(self, arg1, arg2);
    } else if (mask == 5) {
        self->methods->slot58(self, arg1, arg2);
    }
}
```

## Residue and how it was closed

First attempt cached `methods = self->methods;` up front (mirroring the
ALREADY-MATCHED `TaskObjF__OnNotify` in `src/ui/title_menu.c`, which does exactly
that for its own 4-way dispatch) and scored 13/44 with the frame GROWN by
8 bytes (`0x28` vs retail's `0x20`) -- an extra callee-saved register
(`$s3`) spilled to hold the cached `methods` pointer across the two
dispatch calls. Retail does NOT cache it: both `self->methods->slot5C(...)`
and `self->methods->slot58(...)` reload `self->methods` fresh from memory
at their own call site (`lw v0,0(a0)` immediately before each dispatch).
Dropping the local variable and writing `self->methods->slotN(...)` inline
at both call sites (two separate reloads, matching two separate register
lifetimes short enough not to need a saved register) closed it to 44/44.

### Proposed learning

Caching `self->methods` into a local is NOT free -- it is only the right
source shape when retail's own disassembly shows ONE load reused across
multiple calls. When each dispatch site reloads independently (two
`lw v0,0(a0)` instructions rather than one load followed by two uses of a
saved register), the source did NOT cache it either, and writing the cache
anyway costs a whole extra callee-saved register + frame growth that shows
up as an 8-byte size regression shifting every later function in the unit.
`TaskObjF__OnNotify`'s own cache was legitimate for THAT function (verify against
its own disassembly, not by analogy); this one was not.

## Naming

- `TextEntry__OnNotify` -- tier A. gTextEntryMethods +0x038 (classtable.py), overrides BasicClass's slot38 (the last BasicClass-defined slot): dispatches the base slot38, then tag-routes to handleCommand/tickState. Named to match the already-matched sibling TaskObjF__OnNotify (src/ui/title_menu.c), which reads the identical tag the identical way.

## Track 4 (2026-09-26, round 87)

Class unified as `TextEntry` (include/TextEntry.h; table gObj86ED0Methods -> gTextEntryMethods, type Obj86ED0 -> TextEntry). The class name is for what its methods do: setText keeps a caller's string buffer and a working copy, the cursor and char methods edit the copy, command 25 writes it back, 23 closes without writing (banner of include/TextEntry.h). Fields renamed from their accessors: unk14 charCount, unk1C charIndex, unk20 altCommands, nameLen textLen, nameBuf textBuf, unk28 editBuf, unk40 cursorSprite (CharSprite *), unk44 textRow, unk48 panelSprite (ScreenSprite *). Zero bytes changed.

Renamed from Obj86ED0__Notify for its slot, BasicClass's onNotify (+0x038). It calls the base onNotify, then handleCommand (sender low nibble 2) or tickState (5) with (sender, event); the bytes pass $a1/$a2 to both, which is why the tickState slot keeps those parameters though its occupant reads only self.

## Track 7 (2026-09-27, round 98, bravo)

TextEntry +0x034/+0x038 `childType2`/`childType5` -> `inputSource`/
`tickSource` (include/TextEntry.h; accessed only in TextEntryItemList, so
renamed in the definition). They are the children of class 2 and 5, which
are Pad and FrameClock (`typeviews.py --tree`), and ItemList and TaskObjF
already call the same pair `inputSource`/`tickSource`. The kind test reads
`((BasicClass *)child)->methods->header & CLASS_ID_ROOT_MASK` (main's define in include/basic_class.h, added verbatim) and compares with
`PAD_CLASS_ID`/`FRAMECLOCK_CLASS_ID` (new in include/pad.h and
include/FrameClock.h, TASKOBJF_CLASS_ID's form) instead of
`**(s32 **)child`. Zero bytes changed.
