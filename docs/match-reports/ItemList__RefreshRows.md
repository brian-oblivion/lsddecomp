# ItemList__RefreshRows

> Renamed from `Class86F88__RefreshRows` on 2026-09-26 (tools/rename.py). Address 0x8005281c.

> Renamed from `func_8005281C` on 2026-09-24 (tools/rename.py). Address 0x8005281c.

**Unit:** class_3bb8c_k · **Size:** 68 instructions (0x110 bytes) ·
**Status: MATCHED 68/68**, whole-image SHA1 green.

## Role

Refreshes the active window's display text: for each of up to 4 active
`self->unk40[]` elements, formats a fixed-width label via `ItemList__FormatRowText`
(this unit, matched this round) into a local stack buffer and dispatches
`elem->methods->slotCC(elem, buf)`; then forwards `(arg1, arg2, arg3, 0)`
to `ItemList__SetView` (already matched) and optionally notifies `self` via
`slot60` (new this round, shared with `ItemList__StepCursorInView`).

```c
void ItemList__RefreshRows(ItemList *self, s32 arg1, s32 arg2, s32 arg3, s32 arg4)
{
    s32 count;
    s32 i;
    char buf[0x20];
    ItemListElem **p;

    if (!self->unk50) {
        return;
    }
    count = self->unk10;
    p = &self->unk40[0];
    if (count >= 5) {
        count = 4;
    }
    for (i = 0; i < count; i++) {
        ItemList__FormatRowText(self, buf, i, arg1, (char *)arg2);
        (*p)->methods->slotCC(*p, buf);
        p++;
    }
    ItemList__SetView(self, arg1, arg2, arg3, 0);
    if (arg4) {
        self->methods->slot60(self, 0);
    }
}
```

## Three separate reshaping steps, each fixing a distinct mismatch

This one needed three rounds of adjustment, each isolated by comparing
frame size / instruction count against retail before touching anything
else -- worth recording as a sequence, since fixing them in the wrong
order or bundled together would have been much harder to diagnose.

1. **Stack-buffer size, not padding.** The local formatting buffer only
   ever needs indices `0..0x1A` (27 bytes, `ItemList__FormatRowText`'s own fixed
   width), but declaring `char buf[0x28]` (matching a first guess at "the
   gap between the outgoing-args area and the saved registers") produced
   an 8-byte-OVERSIZED frame (`addiu $sp,$sp,-0x70` vs retail's `-0x68`) --
   funcdiff's whole-image shift warning caught it immediately (170KB
   outside-range diff). `char buf[0x20]` (32 bytes) is what actually
   reproduces retail's frame size exactly. The lesson: back into a local
   array's size from the TOTAL FRAME SIZE budget empirically (try a size,
   check `addiu $sp,$sp,-N` in the compiled `.o`), not from "how many bytes
   are unaccounted for in the `.s`'s offset arithmetic" -- the two are not
   the same number here.
2. **`p = &self->unk40[0]` needs to be computed BEFORE the count-clamp
   `if`, not inside the `if (count > 0)` guard.** Retail computes this
   address in the DELAY SLOT of the `count < 5` clamp's `bnez` -- i.e.
   unconditionally, before the clamp even resolves, simply because nothing
   stops the compiler from scheduling an independent computation into an
   available delay slot early. Moving the `p = ...` statement earlier in
   the C (right after `count = self->unk10;`, before the clamp) let GCC
   make the same scheduling choice.
3. **No explicit `if (count > 0) { for (...) }` guard -- the bare `for`
   loop's own entry check already reproduces retail's SINGLE `blez`.**
   Wrapping the loop in an extra `if` produced a REDUNDANT DUPLICATE
   `blez` (two back-to-back, functionally-identical zero/negative checks)
   -- GCC 2.6.3 does not recognize that the `if` guard and the `for`
   loop's own bounds check are the same condition and dead-code-eliminate
   one of them. Retail's single check is what a bare `for (i = 0; i <
   count; i++)` compiles to on its own (a standard single-entry-check
   do-while lowering) -- no separate guard needed or wanted.

## Notes

`ItemList__FormatRowText`'s call site here passes this function's own `arg2`
(established as a plain `s32`, since it is ALSO forwarded unmodified to
`ItemList__SetView`'s already-typed `s32 a2` parameter) through an explicit
`(char *)` cast to match `ItemList__FormatRowText`'s own `base` parameter type
(`char *`, fixed by that function's internal pointer arithmetic -- see its
own report). Both typings are correct for their own function; the cast is
the bridge, not a contradiction.

`ItemListElemMethods::slotCC` (new slot, `+0x0CC`,
`void (*)(ItemListElem *, char *)`) added additively after the existing
`slotB8`.

## Naming

Round 75 (bravo, track 3). `func_8005281C` -> `ItemList__RefreshRows`, **tier A**.

Slot +0x094 (`tools/classtable.py gItemListMethods`). (self, top, column, cursor, notify): re-formats every visible row's text for the new top/column (setText), SetView without highlight, and if `notify` calls forwardToTarget(0). Callers: ScrollRight, ScrollLeft, CursorUp, CursorDown (all notify=1).

ItemList, per the round-75 pass, is a scrolling list selector: up to 4 visible rows of 26-character item text, a highlighted cursor row, a horizontal column offset (see the unit header comment of `src/class_3bb8c_k.c`).

## Round 99 (delta, track 7)

Local `p` -> `row`; buffer 32 decimal; clamp `ARRAY_COUNT(self->rows)`. The buffer size and the early `row = &self->rows[0]` keep `MATCHING:` lines. The notify call is now `playSound(self, 0)` (slot renamed from forwardToTarget).
