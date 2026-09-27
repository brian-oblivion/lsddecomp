# TextRow__Reset — MATCHED (24/24 words)

> Renamed from `Obj6EAC0__FinishConstruct` on 2026-09-26 (tools/rename.py). Address 0x80040a88.

> Renamed from `func_80040A88` on 2026-09-18 (tools/rename.py). Address 0x80040a88.

Unit: `src/ScreenWidgets.c`. First attempt.

```c
void TextRow__Reset(Obj6EAC0 *self, s32 a1) {
    self->methods->slotD4(self, 7);
    self->methods->slotCC(self, a1);
}
```

Derived-table occupant of `Obj6EAC0Methods::slot0x40`. Both calls
dispatch through `self->methods` (virtual, resolves to whichever
class's slot is actually installed) rather than by literal name --
matches the established "a base constructor can dispatch through
self->methods immediately" idiom. `slotD4`'s `s32` return is discarded
as a bare statement.

### Proposed learning

None beyond what's already recorded.

## Naming

Round 54 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_80040A88` | `TextRow__Reset` | B |

**Evidence.** Derived occupant of `slot40`, called as the LAST step of
`TextRow__TextRow` (`self->methods->slot40(self, a3)`, where `a3` is
`Construct`'s own last parameter -- itself forwarded from
`New_TextRow`'s own `name` argument, see that report). Body:
`self->methods->slotD4(self, 7)` (`TextRow__SetCellPitch`, hard-coding
the pitch to 7) then `self->methods->slotCC(self, a1)` (`SetMask` on a
base instance, `SetText` on a derived one). Named for its ROLE in the
construction sequence (the post-allocation finishing step), not for a
guessed purpose -- tier B: the "hard-code pitch=7, then apply
mask-or-text" mechanics are certain, but why 7 specifically is not
established.

## Track 4 (2026-09-26, round 88, charlie)

2026-09-26, round 88 (charlie): class 0x11144 unified as TextRow in `include/TextRow.h` (a row of CharSprite cells: the ctor makes `count` New_CharSprite cells, setText hands each the next byte of a string, the layout slots step `cellPitch` along x). The view `Obj6EAC0` (named after BoxFill's old table address) is gone; its +0x00C `hasChildren` is SceneNode's `parent` (--merge CONFLICT s32 vs pointer: only tested against 0, bytes unchanged), `children` (+0x0B4) is `CharSprite **cells`, and the per-cell calls go through CharSprite's slots by name. Renamed from `Obj6EAC0__FinishConstruct`: the +0x040 reset occupant, `(TextRow *self, char *text)`: setCellPitch(7), setText(text). The slot keeps SceneNode's `reset(self)` type; the ctor casts to TextRowResetFn, as CharSprite's ctor does for its own reset. Image byte-identical; the current source is src/ScreenWidgets.c.
