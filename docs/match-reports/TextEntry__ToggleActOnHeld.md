# TextEntry__ToggleActOnHeld -- MATCHED (10/10 words)

> Renamed from `TextEntry__ToggleAltCommands` on 2026-09-27 (tools/rename.py). Address 0x800517ec.

> Renamed from `Obj86ED0__ToggleFlag20` on 2026-09-26 (tools/rename.py). Address 0x800517ec.

> Renamed from `func_800517EC` on 2026-09-24 (tools/rename.py). Address 0x800517ec.

Unit: `src/class_3bb8c_j.c`. `self` is `Obj86ED0` (ROUND 75 CORRECTION: was misattributed to `Obj866E8`, actually `Obj86ED0` -- gTextEntryMethods, established by class_3bb8c_i; see TextEntry__PrevChar.md for
the class-identity evidence shared across this group).

## Body

```c
void TextEntry__ToggleActOnHeld(Obj86ED0 *self)
{
    if (self->unk48) {
        self->unk20 ^= 1;
    }
}
```

Trivial boolean toggle, matched first try. `self->unk20` is unrelated to
the countdown fields (`unk10`/`unk14`/`unk18`/`unk1C`) the rest of this
group uses -- it is XOR-toggled here and nowhere else read in this unit.

## Naming

- `TextEntry__ToggleActOnHeld` -- tier B. self->unk20 ^= 1, gated on self->unk48. A pure boolean toggle with no further use of unk20 in this unit -- mechanics are the whole of what's known. classtable.py gTextEntryMethods +0x098.

## Track 4 (2026-09-26, round 87)

Class unified as `TextEntry` (include/TextEntry.h; table gObj86ED0Methods -> gTextEntryMethods, type Obj86ED0 -> TextEntry). The class name is for what its methods do: setText keeps a caller's string buffer and a working copy, the cursor and char methods edit the copy, command 25 writes it back, 23 closes without writing (banner of include/TextEntry.h). Fields renamed from their accessors: unk14 charCount, unk1C charIndex, unk20 altCommands, nameLen textLen, nameBuf textBuf, unk28 editBuf, unk40 cursorSprite (CharSprite *), unk44 textRow, unk48 panelSprite (ScreenSprite *). Zero bytes changed.

Renamed from Obj86ED0__ToggleFlag20: flips altCommands (+0x020). HandleCommand's arrow cases act on codes 21/20/18/19 when it is 0 and on 5/4/2/3 when set, so the flag selects which command set moves the cursor. Tier B.

## Round 98: renamed TextEntry__ToggleAltCommands -> TextEntry__ToggleActOnHeld

The field it flips is now `actOnHeld` (+0x020): HandleCommand's arrow cases
act on Pad presses when it is 0 and on held buttons when it is set. The slot
is `toggleActOnHeld` (+0x098). The body is `if (panelSprite) actOnHeld ^= 1;`
and the name claims no more than that. Tier A.
