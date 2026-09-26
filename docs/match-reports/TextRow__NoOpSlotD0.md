> Renamed from `Obj6EAC0__NoOpSlotD0` on 2026-09-26 (tools/rename.py). Address 0x80040fa0.

## Naming

Round 54 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_80040FA0` | `TextRow__NoOpSlotD0` | A |

**Evidence.** An empty function body (`{ }`), splat-generated (`jr $ra;
nop`). `slotD0` is derived-only -- the base table has no occupant for it
at all (see `include/code_2cc8c.h`'s `Obj6EAC0Methods` comment) -- so this
is a reserved/unused slot filled with a do-nothing stub rather than an
override of a real base behaviour, unlike `TextRow__NoOpGetCell`
(`slotC8`, which DOES override a real base setter). Kept the slot number
in the name rather than inventing a guessed purpose for an unused slot.

## Track 4 (2026-09-26, round 88, charlie)

2026-09-26, round 88 (charlie): class 0x11144 unified as TextRow in `include/TextRow.h` (a row of CharSprite cells: the ctor makes `count` New_CharSprite cells, setText hands each the next byte of a string, the layout slots step `cellPitch` along x). The view `Obj6EAC0` (named after BoxFill's old table address) is gone; its +0x00C `hasChildren` is Class6B5CC's `parent` (--merge CONFLICT s32 vs pointer: only tested against 0, bytes unchanged), `children` (+0x0B4) is `CharSprite **cells`, and the per-cell calls go through CharSprite's slots by name. Renamed from `Obj6EAC0__NoOpSlotD0` (class prefix only): +0x0D0, own slot `slotD0`, empty and never called. Image byte-identical; the current source is src/code_2cc8c_f.c.
