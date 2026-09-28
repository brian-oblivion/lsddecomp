# TextRow__SetCellPitch — MATCHED (2/2 words)

> Renamed from `Obj6EAC0__SetChildPitch` on 2026-09-26 (tools/rename.py). Address 0x80040fa8.

> Renamed from `func_80040FA8` on 2026-09-18 (tools/rename.py). Address 0x80040fa8.

Unit: `src/ui/ScreenWidgets.c`. Trivial setter, first attempt.

```c
void TextRow__SetCellPitch(Obj6EAC0 *self, s32 a1) {
    self->unkB0 = a1;
}
```

Derived-table occupant of `Obj6EAC0Methods::slot0xD4` (derived-only
slot, no base equivalent). Called by `TextRow__Reset` (this unit) as
`self->methods->slotD4(self, 7)`.

### Proposed learning

None beyond what's already recorded.

## Naming

Round 54 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_80040FA8` | `TextRow__SetCellPitch` | A |

**Evidence.** A pure leaf setter (tier A): `self->childPitch = a1`, one
field, no other logic. `childPitch` (renamed from `unkB0`) is the value
`TextRow__SetPosition`/`TextRow__AttachToParent` add into the
running position cursor once per child -- i.e. the per-child advance/
spacing distance, confirmed by those two callers (this same unit).

## Track 4 (2026-09-26, round 88, charlie)

2026-09-26, round 88 (charlie): class 0x11144 unified as TextRow in `include/TextRow.h` (a row of CharSprite cells: the ctor makes `count` New_CharSprite cells, setText hands each the next byte of a string, the layout slots step `cellPitch` along x). The view `Obj6EAC0` (named after BoxFill's old table address) is gone; its +0x00C `hasChildren` is SceneNode's `parent` (--merge CONFLICT s32 vs pointer: only tested against 0, bytes unchanged), `children` (+0x0B4) is `CharSprite **cells`, and the per-cell calls go through CharSprite's slots by name. Renamed from `Obj6EAC0__SetChildPitch`: +0x0D4, own slot `setCellPitch`; stores `cellPitch` (+0x0B0), which reset sets to 7 and the layout slots add to x between cells. Image byte-identical; the current source is src/ui/ScreenWidgets.c.
