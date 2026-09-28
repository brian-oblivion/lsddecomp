# TaskObjF__OnItemListResult -- MATCH

> Renamed from `TaskObjF__OnItemSelected` on 2026-09-26 (tools/rename.py). Address 0x80050730.

> Renamed from `Class86E00_3bb8c_g__OnItemSelected` on 2026-09-23 (tools/rename.py). Address 0x80050730.

> Renamed from `func_80050730` on 2026-09-23 (tools/rename.py). Address 0x80050730.

Unit `title_menu`, round 14. `./build-and-verify.sh` exit 0; whole-image
SHA1 matches retail. `funcdiff.py TaskObjF__OnItemListResult`: 46/46 words match.

## Source

```c
void TaskObjF__OnItemListResult(Class86E00_3bb8c_g *self, GenericSlot9CObj_3bb8c_g *arg1, s32 arg2)
{
    switch (arg2) {
    case 2:
        self->unk80 = arg1->methods->slot9C(arg1);
        self->methods->slotAC(self);
        self->methods->slot7C(self, 0xE);
        break;
    case 3:
        self->methods->slotAC(self);
        self->methods->slot7C(self, 0x17);
        break;
    }
}
```

First attempt, byte-exact. Same two-value `switch` layout lesson as
`TaskObjF__OnTextEntryResult` (last one written, applied directly here without
re-deriving): out-of-line case bodies reached by forward `beq`s, which a
plain `switch` reproduces and an `if`/`else if` chain would not.

This is this unit's last fresh function -- all 12 of `title_menu`'s
non-blocked functions are now matched.

## Struct changes (additive, `include/class_3bb8c.h`)

None new -- `slotAC` and `GenericSlot9CObj_3bb8c_g::slot9C` were both
already declared while surveying the unit (`TaskObjF__ReleaseCardIcon`'s report);
this is the function that exercises both.

### Proposed learning

None new.

## Naming

`TaskObjF__OnItemListResult` (was `func_80050730`), tier B: same
external `arg2 in {2, 3}` dispatch shape as `TaskObjF__OnTextEntryResult`,
but its `2` case additionally reads a return value off its own `arg1`
(a `GenericSlot9CObj_3bb8c_g *`) into `self->unk80` before transitioning --
read as "an item was chosen, and here it is" rather than a plain command,
though which UI concept `arg1` represents is not established.

## Track 4 (2026-09-26, round 89)

Renamed from `TaskObjF__OnItemSelected`. Slot +0x0B0, which TaskObjF__OnNotify calls for a sender of class id 0x20, the ItemList list, whose setState(4) notifies its parents with result 2 or 3. On 2 it reads the list's getCursorIndex (+0x09C) into `selectedIndex` (was `selectedItem`, retyped `s32`: the getter returns cursorIndex and AdvanceState only indexes with it), detaches the list and sets state 0xE; on 3 it detaches and sets 0x17.

## Track 7 (2026-09-27, round 95)

Parameters `arg1`/`arg2` -> `list`/`result`; the cases are
`ITEMLIST_RESULT_CHOSEN`/`_CANCELLED` (enum ItemListResult, added to
include/item_list.h this round), the targets LOAD_WARNING and ABORTED.
Image byte-identical.

## History (source comments moved in track 12, round 106)

From `include/task_objf.h`:

> The prototype and the +0x0B0 slot named the second parameter `sender`;
> track 12 renamed it to the definition's `list` (names only, no bytes).
