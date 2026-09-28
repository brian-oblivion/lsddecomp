# TextEntry__SetCharAt — MATCHED (round 45, 45/45 words)

> Renamed from `Obj86ED0__DispatchLookupValue` on 2026-09-26 (tools/rename.py). Address 0x80051998.

> Renamed from `func_80051998` on 2026-09-24 (tools/rename.py). Address 0x80051998.

**Unit:** TextEntryItemList · **Size:** 45 words (0xB4 bytes)

Filed as a `gp_rel`-blocked stub in round 15. That blocker was RESOLVED in
round 42 (`--gp-symbols`, pinned in the Makefile). Matched on the first
attempt once rebuilt against the fixed toolchain, alongside its sibling
`TextEntry__SetCursorPos`.

## ROUND 75 CORRECTION

This report originally typed `self` as `Obj866E8` (gStageMapMethods) and typed
`self->unk44` through a unit-local `Unk44Obj866E8`/`Unk44Obj866E8Methods`
duplicate. Both were wrong, for the same reason as its sibling
`TextEntry__SetCursorPos` (see that report and
`src/ui/TextEntryItemList.c`'s file header comment): `self` is `Obj86ED0`
(`tools/classtable.py gTextEntryMethods` places this function at +0x0A8), whose
shared struct already types `self->unk44` as `ChildObj86ED0 *`. The
`+0x0C4` slot is now `ChildMethods86ED0::slotC4`, added additively next to
the sibling's `slotBC`. Zero bytes affected.

## Derivation

```c
extern u8 *gNameCharTable;

void TextEntry__SetCharAt(Obj86ED0 *self, s32 arg1, s32 arg2, s32 arg3)
{
    ChildObj86ED0 *obj;

    if (self->unk48) {
        self->unk28[arg1] = gNameCharTable[arg2];
        obj = self->unk44;
        obj->methods->slotC4(obj, gNameCharTable[arg2], arg1);
        self->unk18 = arg1;
        self->unk1C = arg2;
        if (arg3) {
            self->methods->slot60(self, 0);
        }
    }
}
```

Same `Obj86ED0` "countdown/flush" group as `TextEntry__SetCursorPos` (see that
report): gated on `self->unk48`, this one copies one byte out of a lookup
table (`gNameCharTable`, VALUE-of `%gp_rel`, ROM image points it at
still-uncarved rodata `D_800115D0`) into `self->unk28[arg1]`, forwards the
same byte plus `arg1` to `self->unk44`'s own method table at slot `0xC4`,
records `self->unk18`/`self->unk1C`, and — if `arg3` is non-zero — notifies
through the same `self->methods->slot60(self, 0)` as its sibling.

`self->unk18 = arg1` and `self->unk1C = arg2` both land in retail's branch
delay slots (of the `beqz arg3, end` branch and its own fallthrough),
executing unconditionally whenever `self->unk48` is set — ordinary
plain-statement-before-`if` C, same shape as `TextEntry__SetCursorPos`.

`unk28`/`unk44` were already present on the shared `Obj86ED0` (established
by TextEntryItemList); only `ChildMethods86ED0::slotC4` (offset 0x0C4) is a
new additive field in `include/class_3bb8c.h`, alongside the sibling's
`slotBC` — see `TextEntry__SetCursorPos`'s report for the full set.

### Proposed learning

None beyond `StampSaveTitleDay`'s this round — this pair matched cleanly once
the stale `gp_rel` verdicts were set aside; the only real content was
identifying the shared `Obj86ED0` "countdown/flush" struct fields and the
notify-on-flag-set tail shape common to both siblings.

## Naming

- `TextEntry__SetCharAt` -- tier B. slotA8 occupant. Looks up gNameCharTable[arg2], stores it into self->unk28[arg1], forwards the same byte to self->unk44's slotC4, records self->unk18/unk1C, optionally notifies via slot60. Same evidence class as its sibling TextEntry__SetCursorPos. classtable.py gTextEntryMethods +0x0A8.

## Track 4 (2026-09-26, round 87)

Class unified as `TextEntry` (include/TextEntry.h; table gObj86ED0Methods -> gTextEntryMethods, type Obj86ED0 -> TextEntry). The class name is for what its methods do: setText keeps a caller's string buffer and a working copy, the cursor and char methods edit the copy, command 25 writes it back, 23 closes without writing (banner of include/TextEntry.h). Fields renamed from their accessors: unk14 charCount, unk1C charIndex, unk20 altCommands, nameLen textLen, nameBuf textBuf, unk28 editBuf, unk40 cursorSprite (CharSprite *), unk44 textRow, unk48 panelSprite (ScreenSprite *). Zero bytes changed.

Renamed from Obj86ED0__DispatchLookupValue: editBuf[pos] = gNameCharTable[charIndex], the same byte to textRow's +0x0C4 (TextRow__SetCellAt) at pos, stores cursorIndex/charIndex, notifyTarget(0) when `notify`. Occupant of slot +0x0A8, called by NextChar/PrevChar/ResetChar/ResetAllChars. Tier A.

## Round 94 (track 6, charlie): textRow is a TextRow

`ChildObj86ED0`/`ChildMethods86ED0` (include/class_3bb8c.h) are deleted: the
object behind them is the `New_TextRow` result, so TextEntry::textRow
(+0x044, include/TextEntry.h) is `struct TextRow *`. The slots map onto
TextRow's table offset for offset: +0x004 `release`, +0x04C `attachToParent`
(position cast to `LongVec3 *`, as ScreenSprite's banner describes), +0x0B8
`setColor` (`gTextEntryTextColor`, the 0x80/0x80/0x00 word, passed as `SpriteRgb *`),
+0x0C4 `setCell`, called through `TextRowSetCellAtFn` because
TextRow__SetCellAt takes the index too. Zero bytes changed.

## History (moved from src/class_3bb8c_j.c, round 100)

The comment on this unit's `extern u8 *gNameCharTable;` read: "VALUE-of
`%gp_rel`, round 45's TextEntry__SetCharAt only -- a byte lookup table (ROM
image initialises it to D_800115D0, still-uncarved rodata)." It now says
what the table is.

## Track 7 (round 100, charlie)

Local `obj` renamed `row` (the TextEntry's textRow). Zero bytes changed.
