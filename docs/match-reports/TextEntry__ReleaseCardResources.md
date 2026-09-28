# TextEntry__ReleaseCardResources -- MATCHED (35/35 words)

> Renamed from `Obj86ED0__ReleaseCardResources` on 2026-09-26 (tools/rename.py). Address 0x80051174.

> Renamed from `func_80051174` on 2026-09-24 (tools/rename.py). Address 0x80051174.

Unit `TextEntryItemList`, carved round 14.

`Obj86ED0`'s vtable slot 0x048. Releases three owned BasicClass-family
child pointers (`unk48`/`unk44`/`unk40`), all gated on a single guard
(`self->unk48 != NULL`) -- only `unk48` gets its return value stored back
(nulling it); `unk44`/`unk40` are released with the return value discarded,
matching retail's asymmetric store pattern exactly (`sw v0,0x48(s0)` after
the first `release` call, no corresponding store after the other two).

```c
void TextEntry__ReleaseCardResources(Obj86ED0 *self)
{
    if (self->unk48 != NULL) {
        self->unk48 = self->unk48->methods->release(self->unk48);
        self->unk44->methods->release(self->unk44);
        self->unk40->methods->release(self->unk40);
    }
}
```

First attempt, transcribed directly off the disassembly's asymmetry rather
than assumed-symmetric, matched immediately.

### Proposed learning

`ChildObj86ED0` (`include/class_3bb8c.h`, new this round) is a minimal
generic BasicClass-family type exposing only `release` at +0x004 -- same
policy as the project's other generic-child types (e.g.
`Class86E00SubObj_3bb8c_g`). Reach for it (or add to it) rather than
inventing a new minimal type per field when a function only needs `release`
dispatched on an opaque child pointer.

## Naming

- `TextEntry__ReleaseCardResources` -- tier A. gTextEntryMethods +0x048 (releaseCardResources slot, classtable.py). Symmetric teardown of TextEntry__LoadCardResources -- releases unk48/unk44/unk40.

## Track 4 (2026-09-26, round 87)

Class unified as `TextEntry` (include/TextEntry.h; table gObj86ED0Methods -> gTextEntryMethods, type Obj86ED0 -> TextEntry). The class name is for what its methods do: setText keeps a caller's string buffer and a working copy, the cursor and char methods edit the copy, command 25 writes it back, 23 closes without writing (banner of include/TextEntry.h). Fields renamed from their accessors: unk14 charCount, unk1C charIndex, unk20 altCommands, nameLen textLen, nameBuf textBuf, unk28 editBuf, unk40 cursorSprite (CharSprite *), unk44 textRow, unk48 panelSprite (ScreenSprite *). Zero bytes changed.

## Round 94 (track 6, charlie): textRow is a TextRow

`ChildObj86ED0`/`ChildMethods86ED0` (include/class_3bb8c.h) are deleted: the
object behind them is the `New_TextRow` result, so TextEntry::textRow
(+0x044, include/TextEntry.h) is `struct TextRow *`. The slots map onto
TextRow's table offset for offset: +0x004 `release`, +0x04C `attachToParent`
(position cast to `LongVec3 *`, as ScreenSprite's banner describes), +0x0B8
`setColor` (`sTextEntryTextColor`, the 0x80/0x80/0x00 word, passed as `ColorRgb *`),
+0x0C4 `setCell`, called through `TextRowSetCellAtFn` because
TextRow__SetCellAt takes the index too. Zero bytes changed.
