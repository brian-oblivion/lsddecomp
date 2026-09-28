# TextEntry__ResetChar -- MATCHED (17/17 words)

> Renamed from `Obj86ED0__ResetCountdown` on 2026-09-26 (tools/rename.py). Address 0x80051814.

> Renamed from `func_80051814` on 2026-09-24 (tools/rename.py). Address 0x80051814.

Unit: `src/ui/TextEntryItemList.c` (was `src/class_3bb8c_j.c`). `self` is `Obj86ED0` (ROUND 75 CORRECTION: was misattributed to `Obj866E8`, actually `Obj86ED0` -- gTextEntryMethods, established by TextEntryItemList; see TextEntry__PrevChar.md for
the class-identity evidence shared across this group).

## Body

```c
void TextEntry__ResetChar(Obj86ED0 *self)
{
    if (self->unk48) {
        self->unk1C = 0;
        self->methods->slotA8(self, self->unk18, 0, 1);
    }
}
```

An "immediate fire" sibling of `TextEntry__PrevChar`'s countdown: resets
`self->unk1C` to 0 and calls `slotA8` unconditionally with a literal 0
where `TextEntry__PrevChar` would pass the live decremented countdown. Matched
first try -- no reshaping needed.

## Naming

- `TextEntry__ResetChar` -- tier B. self->unk1C = 0; slotA8(self, unk18, 0, 1) -- the same dispatch as TextEntry__PrevChar's expiry arm, called directly/unconditionally instead of reached by counting down. classtable.py gTextEntryMethods +0x09C.

## Track 4 (2026-09-26, round 87)

Class unified as `TextEntry` (include/TextEntry.h; table gObj86ED0Methods -> gTextEntryMethods, type Obj86ED0 -> TextEntry). The class name is for what its methods do: setText keeps a caller's string buffer and a working copy, the cursor and char methods edit the copy, command 25 writes it back, 23 closes without writing (banner of include/TextEntry.h). Fields renamed from their accessors: unk14 charCount, unk1C charIndex, unk20 altCommands, nameLen textLen, nameBuf textBuf, unk28 editBuf, unk40 cursorSprite (CharSprite *), unk44 textRow, unk48 panelSprite (ScreenSprite *). Zero bytes changed.

Renamed from Obj86ED0__ResetCountdown: charIndex = 0 and setCharAt(cursorIndex, 0, 1), i.e. table byte 0 at the cursor. Tier B (what byte 0 of sNameCharTable is on screen is not checked).
