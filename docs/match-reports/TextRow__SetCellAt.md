# TextRow__SetCellAt — MATCHED (17/17 words)

> Renamed from `Obj6EAC0__SetChildChar` on 2026-09-26 (tools/rename.py). Address 0x80040edc.

> Renamed from `func_80040EDC` on 2026-09-18 (tools/rename.py). Address 0x80040edc.

Unit: `src/code_2cc8c_f.c`. First attempt.

```c
void TextRow__SetCellAt(Obj6EAC0 *self, s32 a1, s32 a2) {
    Obj6EAC0 *elem = self->unkB4[a2];
    elem->methods->slotC4(elem, a1 & 0xFF);
}
```

Derived-table occupant of `Obj6EAC0Methods::slotC4`, dispatching a
CHILD's own `slotC4` at 2 explicit arguments. This is exactly why
`slotC4` was made unprototyped (K&R style, no parameter list) in the
previous commit -- the base occupant `BoxFill__AttachAbsolute` forwards 4 raw
args through the same named slot, and calling through an unprototyped
function pointer needs no cast for either arity.

### Proposed learning

Confirms the unprototyped-slot lever from `TextRow__AttachToParent`'s header
change actually pays off at a real call site with no cast needed.

## Naming

Round 54 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_80040EDC` | `TextRow__SetCellAt` | B |

**Evidence.** Derived occupant of `slotC4`: `elem = self->children[a2];
elem->methods->slotC4(elem, a1 & 0xFF);` -- dispatches a single 8-bit
value to ONE child selected by index, through the same slot
`TextRow__SetText` (this unit) drives across ALL children one string
byte at a time. See `BoxFill__AttachAbsolute`'s own report for the full
cross-reference; same tier B.

## Track 4 (2026-09-26, round 88, charlie)

2026-09-26, round 88 (charlie): class 0x11144 unified as TextRow in `include/TextRow.h` (a row of CharSprite cells: the ctor makes `count` New_CharSprite cells, setText hands each the next byte of a string, the layout slots step `cellPitch` along x). The view `Obj6EAC0` (named after BoxFill's old table address) is gone; its +0x00C `hasChildren` is SceneNode's `parent` (--merge CONFLICT s32 vs pointer: only tested against 0, bytes unchanged), `children` (+0x0B4) is `CharSprite **cells`, and the per-cell calls go through CharSprite's slots by name. Renamed from `Obj6EAC0__SetChildChar`: the +0x0C4 setCell occupant, but with an index: `(TextRow *self, s32 cell, s32 index)` sets `cells[index]`'s cell. Named SetCellAt rather than SetCell because the body takes the index the slot does not have; the slot keeps CharSprite's `setCell(self, u8)` type and TextRowSetCellAtFn is the override's (TextEntry__SetCharAt calls it with (char, pos) through its own ChildObj86ED0 view). Image byte-identical; the current source is src/code_2cc8c_f.c.
