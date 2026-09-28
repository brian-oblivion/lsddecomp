# ItemList__ScrollLeft -- MATCH

> Renamed from `Class86F88__ScrollLeft` on 2026-09-26 (tools/rename.py). Address 0x80052498.

> Renamed from `func_80052498` on 2026-09-24 (tools/rename.py). Address 0x80052498.

Unit `ObjMStyleActor`, round 15. `./build-and-verify.sh` exit 0; whole-image
SHA1 matches retail. `funcdiff.py ItemList__ScrollLeft`: 24/24 words match.

This is vtable slot `+0x080` of `gItemListMethods` (`ItemListMethods::slot80`),
dispatched by `ItemList__PlaySound` (this unit, on a DIFFERENT instance reached
through `self->unk3C`) as `other->methods->slot80(other, arg1, 0x60, 0x60)`.

## Source

```c
void ItemList__ScrollLeft(ItemList *self)
{
    s32 count;

    if (!self->unk50) {
        return;
    }
    count = self->unk24 - 1;
    if (count < 0) {
        return;
    }
    self->unk24 = count;
    self->methods->slot94(self, self->unk20, count, self->unk28, 1);
}
```

## Notes

The occupant's own body ignores every argument past `self` -- it never
reads what a caller passes in `arg1`/`arg2`/`arg3` -- which is why it
compiles cleanly as a plain `(ItemList *self)` function even though
`ItemList__PlaySound`'s call site (a DIFFERENT instance's copy of this same
slot) passes 3 more arguments. Per this project's established
per-call-site-arity convention this is not a contradiction: the slot's
declared pointer type in the header carries the fuller signature the call
site needs, while this occupant's own C signature only needs what its own
body reads.

`count = self->unk24 - 1;` (a single fused subtract-and-compare expression,
not a separate load followed by `- 1`) is what let the guard's own
register become the exact value stored back to `self->unk24` and forwarded
to `slot94` with no reload -- matched on the first attempt in this shape.

## Naming

Round 75 (bravo, track 3). `func_80052498` -> `ItemList__ScrollLeft`, **tier A**.

Slot +0x080 (`tools/classtable.py gItemListMethods`). Decrements `column` if it stays >= 0, then refreshRows. Dispatched by HandleInputCode on code 4.

ItemList, per the round-75 pass, is a scrolling list selector: up to 4 visible rows of 26-character item text, a highlighted cursor row, a horizontal column offset (see the unit header comment of `src/world/ObjMStyleActor.c`).
