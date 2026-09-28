# ItemList__SetState -- MATCH

> Renamed from `Class86F88__SetState` on 2026-09-26 (tools/rename.py). Address 0x800521d4.

> Renamed from `func_800521D4` on 2026-09-24 (tools/rename.py). Address 0x800521d4.

Unit `ObjMStyleActor`, round 15. `./build-and-verify.sh` exit 0 (checked once
the whole unit's batch of matches was in place); whole-image SHA1 matches
retail. `funcdiff.py ItemList__SetState`: 42/42 words match.

This is vtable slot `+0x054` of `gItemListMethods` (`ItemListMethods::slot54`),
dispatched by `ItemList__TickClosing` (this unit) as `self->methods->slot54(self, 4)`.

## Source

```c
void ItemList__SetState(ItemList *self, s32 state)
{
    self->unk30 = 0;
    if (state < 2) {
        goto end;
    }
    if (state < 4) {
        goto case_lt4;
    }
    if (state == 4) {
        goto case_eq4;
    }
    goto end;
case_lt4:
    self->methods->slot14(self, self->unk34);
    self->methods->slot48(self);
    self->unk2C = state;
    goto end;
case_eq4:
    self->methods->slot30(self, self->unk2C);
end:
    return;
}
```

## Notes

A 3-way dispatch on `state` (`<2` no-op, `2..3` one path, `==4` a second
path, `>4` no-op), with `self->unk30 = 0` unconditional at the top (it sits
in the delay slot of the first branch in the disassembly, so it executes
on every call regardless of `state`).

**Neither a nested `if`/`else if` nor an early-return-per-case reproduced
retail's physical block layout or its branch instructions**, even though
both are semantically identical to the final form. Both alternate shapes
compiled 2 words short (a whole `beq`+`nop`+`j`+`nop` 4-instruction group
collapsed into a 2-instruction `bne`), and when the `state==4` code was
written textually before the `state<4` case, GCC swapped which body was
placed first in the physical layout (matching the *source* order, not
retail's ROM order). Only writing the three-way test as explicit
`goto`/label pairs -- with `case_lt4` appearing textually (and therefore
physically) **before** `case_eq4`, replicating retail's own branch
polarities exactly (`bnez` into `case_lt4`, `beq` into `case_eq4`, implicit
fallthrough to `end`) -- reproduced retail's instructions exactly.

### Proposed learning

A three-way dispatch (`if (a) ... else if (b) ... else ...`) where two
branches jump into non-adjacent blocks is not reliably reproduced by
nested `if`/`else if`, even when every value and every condition is
correct -- GCC 2.6.3 chooses its own branch polarity and block order from
the *source's* textual/control-flow shape, not from some canonical
lowering of the semantics. When a residue is "off by a `beq`+`j` pair" or
"the two case bodies are swapped in physical order", try explicit
`goto`/label pairs whose textual order and branch polarity mirror the
disassembly's own `bnez`/`beq`/fallthrough sequence one-for-one, rather
than continuing to reshape nested conditionals.

## Naming

Round 75 (bravo, track 3). `func_800521D4` -> `ItemList__SetState`, **tier B**.

Slot +0x054 of gItemListMethods (`tools/classtable.py gItemListMethods`). Clears `closeTicks`; for state 2 or 3 it removes the cached `inputSource` child, releases resources (slot +0x048, ItemList__ReleaseResources) and stores the state in `result`; for state 4 it calls notifyParents(self, result). Callers: ItemList__HandleInputCode (2 after input code 25, 3 after code 23) and ItemList__TickClosing (4). The one parent-side reader, TaskObjF__OnItemListResult (TitleMenuTaskObjF), takes code 2 as "read the selected item" and 3 as the other outcome. Tier B: `SetState` names the mechanics; the states' game meaning (confirm/cancel) is only suggested by that one caller.

ItemList, per the round-75 pass, is a scrolling list selector: up to 4 visible rows of 26-character item text, a highlighted cursor row, a horizontal column offset (see the unit header comment of `src/ObjMStyleActor.c`).

### Globals and fields named in this pass

Globals (`tools/rename.py`):

| old | new | tier | evidence |
| --- | --- | --- | --- |
| `D_80086F88` | `gItemListMethods` | A | the class's method table (39 slots, header word 0x20), returned by GetItemListMethods |
| `D_8008AB00` | `gItemListRowOriginX` | B | sdata word -0x5C, row 0's x in ItemList__CreateRows's `pos` |
| `D_8008AB04` | `gItemListRowOriginY` | B | sdata word -0xF, row 0's y; each further row +0xA |
| `D_8008AB0C` | `gItemListRowColor` | A | bytes 50 50 50 00, passed to the rows' +0x0B8 (TextRow__SetColor in gTextRowMethods) for every non-cursor row |
| `D_8008AB10` | `gItemListCursorColor` | A | bytes 80 80 00 00, the same colour slot for the cursor row |

Fields and slots (`include/class_3bb8c.h`, renamed definition-first; every
accessor the compiler listed, in both the build and
`tools/check-nonmatching.sh`, was in ObjMStyleActor, so none are proposed):

- ItemList: `itemCount` (+0x10), `maxTextLen` (+0x14), `texts` (+0x18),
  `topIndex` (+0x20), `column` (+0x24), `cursorIndex` (+0x28), `result`
  (+0x2C), `closeTicks` (+0x30), `inputSource` (+0x34), `target` (+0x3C),
  `rows[4]` (+0x40), `resource` (+0x50). itemCount/maxTextLen/texts/
  inputSource/resource are confirmed by the same offsets in TextEntryItemList's
  view (ItemList__ItemList, __AddChild, __LoadResources).
- ItemListMethods: `removeChild`, `notifyParents`, `releaseResources`,
  `setState`, `forwardToTarget`, `scrollRight`/`scrollLeft`/`cursorUp`/
  `cursorDown` (new, +0x07C..+0x088), `refreshRows`, `stepCursorInView`, each
  the method gItemListMethods holds at that offset.
- ItemListElemMethods: `layout` (+0x04C), `setColor` (+0x0B8), `setText`
  (+0x0CC), from gTextRowMethods, the derived Obj6EAC0 table New_TextRow builds.

## Proposed field names

None: every field and slot renamed this round had accessors only in
ObjMStyleActor. For the head: the TYPE name `ItemList` is still an address
name; `ListSelector` or similar would fit the reading above, but renaming it
touches TextEntryItemList's local `ItemList_3bb8c_j` and 20+ symbols, so it is
left for a pass that owns both units.

## Round 99 (delta, track 7)

`state < 2` / `< 4` / `== 4` are `ITEMLIST_RESULT_CHOSEN`, `ITEMLIST_STATE_REPORT` (new in include/ItemList.h, value 4: the state that passes `result` to notifyParents; TickClosing enters it, as TextEntry's `TEXTENTRY_STATE_REPORT`). The gotos carry a `MATCHING:` line (see Notes).
