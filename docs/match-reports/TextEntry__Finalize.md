# TextEntry__Finalize -- MATCHED (18/18 words)

> Renamed from `Obj86ED0__Finalize` on 2026-09-26 (tools/rename.py). Address 0x80050ce8.

> Renamed from `func_80050CE8` on 2026-09-24 (tools/rename.py). Address 0x80050ce8.

Unit `input_dialogs`, carved round 14.

`Obj86ED0`'s finalize override (vtable slot 0x00C, per
`tools/classtable.py gTextEntryMethods`): frees the owned name buffer, then
dispatches the BASE class's own finalize (`GetBasicClassMethods()->finalize`,
NOT `self->methods->finalize`, since that would just call this same
function again -- confirmed against `GetBasicClassMethods`'s local reduced view
`BasicMethods866E8F`, extended this round with the ctor/finalize/addChild/
removeChild/removeAllChildren slots that were previously opaque padding).

```c
void TextEntry__Finalize(Obj86ED0 *self)
{
    BMemPMgrFree(self->unk28);
    GetBasicClassMethods()->finalize(self);
}
```

First attempt, straight transcription, matched immediately.

### Proposed learning

`BasicMethods866E8F` (`include/class_3bb8c.h`) previously only named its
`slot38` field, leaving `ctor`/`finalize`/`addChild`/`removeChild`/
`removeAllChildren` as opaque `pad000[0x038]`. This round's functions
(`TextEntry__TextEntry`'s base-ctor call, and this function's base-finalize call,
plus `TextEntry__AddChild`/`TextEntry__RemoveChild`/`TextEntry__RemoveAllChildren`'s explicit
`GetBasicClassMethods()->addChild/removeChild/removeAllChildren`) named all five.
Since `GetBasicClassMethods` can only have ONE extern declaration per translation
unit (this header has exactly one), any future unit needing a currently-pad
slot of this same getter must EXTEND `BasicMethods866E8F` in place (shrink
the pad, add the field at its real offset) rather than declaring a second,
differently-typed `GetBasicClassMethods` -- the latter is a straight redeclaration
conflict the moment both land in one `.c` file via this shared header.

## Naming

- `TextEntry__Finalize` -- tier A. gTextEntryMethods +0x00C (classtable.py), overrides BasicClass's finalize slot: frees unk28 then chains to GetBasicClassMethods()->finalize. Standard dtor-shaped override.

## Track 4 (2026-09-26, round 87)

Class unified as `TextEntry` (include/text_entry.h; table gObj86ED0Methods -> gTextEntryMethods, type Obj86ED0 -> TextEntry). The class name is for what its methods do: setText keeps a caller's string buffer and a working copy, the cursor and char methods edit the copy, command 25 writes it back, 23 closes without writing (banner of include/text_entry.h). Fields renamed from their accessors: unk14 charCount, unk1C charIndex, unk20 altCommands, nameLen textLen, nameBuf textBuf, unk28 editBuf, unk40 cursorSprite (CharSprite *), unk44 textRow, unk48 panelSprite (ScreenSprite *). Zero bytes changed.
