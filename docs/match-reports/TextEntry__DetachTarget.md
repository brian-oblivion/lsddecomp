# TextEntry__DetachTarget -- MATCHED (22/22 words)

> Renamed from `Obj86ED0__DetachTarget` on 2026-09-26 (tools/rename.py). Address 0x80051270.

> Renamed from `func_80051270` on 2026-09-24 (tools/rename.py). Address 0x80051270.

Unit `class_3bb8c_i`, carved round 14.

`Obj86ED0`'s vtable slot 0x050: detaches both typed child slots via
`self->methods->removeChild` (the class's own override, `TextEntry__RemoveChild`),
then zeroes `unk3C`. Note `unk34`/`unk38` themselves are NOT zeroed here
(only `unk3C` is) -- transcribed as observed, not assumed symmetric with
`TextEntry__ClearChildRefs`/`TextEntry__RemoveAllChildren`.

```c
void TextEntry__DetachTarget(Obj86ED0 *self)
{
    self->methods->removeChild(self, self->unk34);
    self->methods->removeChild(self, self->unk38);
    self->unk3C = NULL;
}
```

First attempt, matched immediately once the earlier in-unit drift was
resolved.

## Naming

- `TextEntry__DetachTarget` -- tier A. gTextEntryMethods +0x050 (classtable.py). Symmetric teardown of TextEntry__AttachTarget: removes childType2/childType5 via self->methods->removeChild, clears target.

## Track 4 (2026-09-26, round 87)

Class unified as `TextEntry` (include/TextEntry.h; table gObj86ED0Methods -> gTextEntryMethods, type Obj86ED0 -> TextEntry). The class name is for what its methods do: setText keeps a caller's string buffer and a working copy, the cursor and char methods edit the copy, command 25 writes it back, 23 closes without writing (banner of include/TextEntry.h). Fields renamed from their accessors: unk14 charCount, unk1C charIndex, unk20 altCommands, nameLen textLen, nameBuf textBuf, unk28 editBuf, unk40 cursorSprite (CharSprite *), unk44 textRow, unk48 panelSprite (ScreenSprite *). Zero bytes changed.
