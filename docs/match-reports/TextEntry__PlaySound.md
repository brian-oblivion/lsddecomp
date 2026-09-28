# TextEntry__PlaySound -- MATCH (16/16 words)

> Renamed from `TextEntry__NotifyTarget` on 2026-09-27 (tools/rename.py). Address 0x8005161c.

> Renamed from `Obj86ED0__NotifyTarget` on 2026-09-26 (tools/rename.py). Address 0x8005161c.

> Renamed from `func_8005161C` on 2026-09-24 (tools/rename.py). Address 0x8005161c.

Unit `TextEntryItemList`. Obj86ED0's own method: if `self->unk3C` (a
`TargetObj86ED0 *`, a wholly separate class reached only through this one
field) is non-NULL, dispatch its own `slot80` on it.

```c
void TextEntry__PlaySound(Obj86ED0 *self, s32 arg1)
{
    TargetObj86ED0 *target;

    target = self->unk3C;
    if (target != NULL) {
        target->methods->slot80(target, arg1, 0x60, 0x60);
    }
}
```

## The header fix this function required

The existing `TargetMethods86ED0::slot80` declaration (established from this
exact call site, before this round) had 2 params after `self`
(`arg1, arg2`). The retail disassembly sets `$a2 = 0x60` in the branch's
delay slot (unconditionally) and `$a3 = 0x60` in the `jalr`'s delay slot,
while `$a1` is never touched inside the function body at all.

That is the signature of a THIRD parameter being forwarded unchanged: the
function's own second parameter arrives in `$a1` and is never moved, because
it is already sitting in the exact register the call needs. This project has
the same idiom at several other call sites already (`src/ObjMStyleActor.c`,
`src/app/Task.c`, `src/TitleMenuTaskObjF.c`, `src/world/TodActor.c`:
`obj->methods->slot80(obj, arg1, 0x60, 0x60)` / `(..., 0x7F, 0x7F)` /
`(..., 0x6E, 0x6E)`), so recognizing it here just meant trusting the pattern
instead of the header's (incomplete) prior reading.

Fixed `include/class_3bb8c.h`'s `TargetMethods86ED0::slot80` to
`(TargetObj86ED0 *self, s32 arg1, s32 arg2, s32 arg3)`. `TargetObj86ED0` is
local to this unit's slice (only `TextEntry__PlaySound` reaches it), so this is
safe to retype outright rather than additively.

### Proposed learning

A vtable slot's arity established from a SINGLE call site with no
corroborating siblings is worth cross-checking against this project's
existing `self->methods->slotN(self, arg1, LITERAL, LITERAL)`
forward-and-repeat idiom before trusting it -- an unused/untouched `$a1`
register in the callee is the tell that the slot takes one more argument
than the call appears to set explicitly.

## Naming

- `TextEntry__PlaySound` -- tier B. gTextEntryMethods +0x060 (classtable.py) -- this IS the `slot60` implementation dispatched by both TextEntry__HandleCommand (arg1=0x10) and TextEntryItemList's TextEntry__SetCursorPos/TextEntry__SetCharAt (arg1=0). Forwards arg1 to the attached `target`'s own slot80(target, arg1, 0x60, 0x60) when target != NULL. Mechanics clear (pings the linked TargetObj86ED0 whenever the name cursor or character selection changes); the on-screen meaning of the two 0x60 literals is not established. Since `slot60` is referenced by both this unit and TextEntryItemList, the SLOT NAME is left as `slot60` in the shared header (PROPOSED name below), even though the FUNCTION name is confidently renamed here (function renames are tree-wide, not subject to the field-ownership rule).

## Track 4 (2026-09-26, round 87)

Class unified as `TextEntry` (include/TextEntry.h; table gObj86ED0Methods -> gTextEntryMethods, type Obj86ED0 -> TextEntry). The class name is for what its methods do: setText keeps a caller's string buffer and a working copy, the cursor and char methods edit the copy, command 25 writes it back, 23 closes without writing (banner of include/TextEntry.h). Fields renamed from their accessors: unk14 charCount, unk1C charIndex, unk20 altCommands, nameLen textLen, nameBuf textBuf, unk28 editBuf, unk40 cursorSprite (CharSprite *), unk44 textRow, unk48 panelSprite (ScreenSprite *). Zero bytes changed.

## Round 94 (track 6, charlie): the target is a VabStreamObj

`TargetObj86ED0`/`TargetMethods86ED0` (include/class_3bb8c.h) are deleted.
Both attachTarget callers (TaskObjF, src/TitleMenuTaskObjF.c) pass TaskObjF's
`sound`, already typed `struct VabStreamObj *`, and the one slot the view
named, +0x080, is VabStreamObj's `playTone(self, index, vol, endVol)`
(include/VabStreamObj.h): the `(code, 0x60, 0x60)` call plays tone `code`
at volume 0x60. TextEntry::target and ItemList::target (+0x03C) and both
attachTarget prototypes are `struct VabStreamObj *`, the casts at the call
sites are gone, and `slot80` is `playTone`. Zero bytes changed.

## Track 7 (2026-09-27, round 98, bravo)

Renamed from `TextEntry__NotifyTarget` (`tools/rename.py`), tier A: a leaf
whose mechanics are its purpose, the same body as `TaskObjF__PlaySound`
(TitleMenuTaskObjF): `target->methods->playTone(target, tone, 96, 96)` when a
target is attached. The volumes are decimal now (96, TaskObjF's are 127);
parameter `arg1` -> `tone`. HandleCommand's argument `0x10` is `1 << 4`.
Zero bytes changed.

### Proposed field names (round 98)

- gTextEntryMethods +0x060 `notifyTarget` -> `playSound` (accessors: this
  unit's HandleCommand, TextEntryItemList's SetCursorPos/SetCharAt), so the
  slot is named like its occupant; the prototype's `arg1` -> `tone`.
- TextEntry +0x03C `target` -> `sound`, TaskObjF's name for the same
  object (accessors: this unit's AttachTarget/DetachTarget/PlaySound only;
  left because ItemList, the sibling with the identical layout, calls its
  field `target` too, and the two should move together).

## Round 98: slot +0x060 named `playSound`

This function is the slot's occupant, so the slot `notifyTarget` became
`playSound` and the prototype's `arg1` became `tone` (the definition already
said `tone`). Callers: HandleCommand (`1 << 4`, accept and cancel),
SetCursorPos and SetCharAt (0, when `notify`). Zero bytes changed.
