# ItemList__ScrollRight -- MATCH

> Renamed from `Class86F88__ScrollRight` on 2026-09-26 (tools/rename.py). Address 0x80052430.

> Renamed from `func_80052430` on 2026-09-24 (tools/rename.py). Address 0x80052430.

Unit `class_3bb8c_k`, round 15. `./build-and-verify.sh` exit 0; whole-image
SHA1 matches retail. `funcdiff.py ItemList__ScrollRight`: 26/26 words match.

This is vtable slot `+0x07C` of `gItemListMethods` (not declared in
`ItemListMethods` since nothing in this unit dispatches through it).

## Source

```c
void ItemList__ScrollRight(ItemList *self)
{
    ItemListMethods *methods;
    s32 tmp;
    s32 count;

    if (!self->unk50) {
        return;
    }
    tmp = self->unk24;
    count = tmp;
    if (count + 0x1A >= self->unk14) {
        return;
    }
    methods = self->methods;
    count++;
    self->unk24 = count;
    methods->slot94(self, self->unk20, count, self->unk28, 1);
}
```

## Notes

Straightforward once the register-identity residue below was closed: two
guard clauses (`self->unk50`, then `self->unk24 + 0x1A < self->unk14`),
then increment-and-store `self->unk24`, then dispatch `slot94` with the
new value and a literal `1` as the 5th (stack) argument.

**A permuter-confirmed register-identity residue, closed with a
`double-assignment` idiom, no barrier and no UB.** The natural forms
(`self->unk24++` inline, or a single `count = self->unk24;` local) both
scored 22/26 -- the same two words (`self->methods` and the literal `1`
swap which of `$v0`/`$v1` each lands in) no matter how the surrounding
code was reshaped. `tools/setup-permuter.sh` found a zero-score candidate
using `count = (new_var = self)->unk28;`-style double reads and a
`(float) 1` UB cast on the literal; **the UB cast turned out to be a red
herring** -- translating only the double-read idiom into plain, legal C
(`tmp = self->unk24; count = tmp;` instead of `count = self->unk24;`)
closed the residue on its own, with the literal argument left as an
ordinary `1`. See CLAUDE.md's warning that a permuter zero reached only
through UB is a LEAD, not an answer -- here the idiomatic half of the
permuter's fix was the real lever and the UB half was incidental.

### Proposed learning

**A redundant `local2 = local1;` double-assignment (reading a struct field
into one local, then copying it to a second) can be load-bearing for
register allocation, independent of any UB.** Seen once before this round
in the "mention the source expression twice" family, but this instance is
narrower and cheaper to check: before reaching for a scheduling barrier or
accepting a `$v0`/`$v1` swap as an unfixable register-identity stall, try
literally doubling the assignment of a value the permuter's zero-scoring
candidate reads twice, and drop any UB (float casts, mismatched types)
from that same candidate first -- the two are often independent, and the
double-read is far more often the real lever.

## Naming

Round 75 (bravo, track 3). `func_80052430` -> `ItemList__ScrollRight`, **tier A**.

Slot +0x07C (`tools/classtable.py gItemListMethods`). If rows exist (`resource`) and column+26 < maxTextLen, increments `column` (the character offset into every item string) and redraws via refreshRows. Mechanics are the purpose. Dispatched by HandleInputCode on code 5.

ItemList, per the round-75 pass, is a scrolling list selector: up to 4 visible rows of 26-character item text, a highlighted cursor row, a horizontal column offset (see the unit header comment of `src/class_3bb8c_k.c`).
