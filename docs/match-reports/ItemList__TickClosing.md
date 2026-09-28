# ItemList__TickClosing -- MATCH

> Renamed from `Class86F88__TickClosing` on 2026-09-26 (tools/rename.py). Address 0x8005227c.

> Renamed from `func_8005227C` on 2026-09-24 (tools/rename.py). Address 0x8005227c.

Unit `class_3bb8c_k`, round 15. `./build-and-verify.sh` exit 0; whole-image
SHA1 matches retail. `funcdiff.py ItemList__TickClosing`: 24/24 words match.

This is vtable slot `+0x058` of `gItemListMethods` (`ItemListMethods` does not
declare this slot since nothing in this unit dispatches through it).

## Source

```c
void ItemList__TickClosing(ItemList *self)
{
    s32 old;

    if (self->unk2C >= 4) {
        return;
    }
    if (self->unk2C < 2) {
        return;
    }
    old = self->unk30;
    self->unk30 = old + 1;
    if (old == 0) {
        return;
    }
    self->methods->slot54(self, 4);
}
```

## Notes

Two independent guard clauses on `self->unk2C` (proceed only when
`2 <= unk2C < 4`), then an unconditional increment of `self->unk30` (its
old value is stored regardless -- the store sits in the branch's delay
slot in the disassembly, so it always executes), gated only by whether the
OLD value was zero, before dispatching `self->methods->slot54(self, 4)` --
which is `ItemList__SetState`, this unit's own occupant of that slot.

Matched on the first attempt once the two-guard-clause shape and the
unconditional-store-then-test-the-old-value idiom were both in place;
no register or scheduling residue.

## Naming

Round 75 (bravo, track 3). `func_8005227C` -> `ItemList__TickClosing`, **tier B**.

Slot +0x058 (`tools/classtable.py gItemListMethods`), which ItemList__OnNotify (class_3bb8c_i) dispatches for notifications from its tag-5 child. While `result` is 2 or 3 it counts calls in `closeTicks` and on the second call dispatches setState(self, 4). What the tag-5 child is (a per-frame tick?) is not established, hence tier B.

ItemList, per the round-75 pass, is a scrolling list selector: up to 4 visible rows of 26-character item text, a highlighted cursor row, a horizontal column offset (see the unit header comment of `src/class_3bb8c_k.c`).

## Round 99 (delta, track 7)

Local `old` -> `prevTicks`, then the read-increment-test written as `if (self->closeTicks++ == 0) return;`: measured byte-identical, so the two-statement form in Source above was not load-bearing. The bounds are `ITEMLIST_STATE_REPORT` and `ITEMLIST_RESULT_CHOSEN`; the dispatch is `setState(self, ITEMLIST_STATE_REPORT)`.
