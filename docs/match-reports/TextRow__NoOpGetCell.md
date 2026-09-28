> Renamed from `Obj6EAC0__NoOpSetter` on 2026-09-26 (tools/rename.py). Address 0x80040f20.

## Naming

Round 54 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_80040F20` | `TextRow__NoOpGetCell` | A |

**Evidence.** An empty function body (`{ }`), splat-generated (`jr $ra;
nop`), the derived table's own occupant of `slotC8` -- the same slot the
base table fills with a real one-field setter (`BoxFill__SetPri`, left
unnamed: `unk44`'s purpose is not established). Same shape as the
project's existing `NoOp`/`NoOpIgnoreArgs` precedent (a pure do-nothing
leaf, tier A by definition), given a class-prefixed name rather than a
bare one since it specifically overrides ONE slot of ONE class's table,
unlike those two shared, class-agnostic fillers.

## Track 4 (2026-09-26, round 88, charlie)

2026-09-26, round 88 (charlie): class 0x11144 unified as TextRow in `include/text_row.h` (a row of CharSprite cells: the ctor makes `count` New_CharSprite cells, setText hands each the next byte of a string, the layout slots step `cellPitch` along x). The view `Obj6EAC0` (named after BoxFill's old table address) is gone; its +0x00C `hasChildren` is SceneNode's `parent` (--merge CONFLICT s32 vs pointer: only tested against 0, bytes unchanged), `children` (+0x0B4) is `CharSprite **cells`, and the per-cell calls go through CharSprite's slots by name. Renamed from `Obj6EAC0__NoOpSetter`: the empty +0x0C8 occupant; the slot is CharSprite's getCell, so the old view's 'setter' reading came from BoxFill's SetPri at the same offset. Image byte-identical; the current source is src/ui/screen_widgets.c.
