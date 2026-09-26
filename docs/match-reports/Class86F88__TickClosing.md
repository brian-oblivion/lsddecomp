# Class86F88__TickClosing -- MATCH

> Renamed from `func_8005227C` on 2026-09-24 (tools/rename.py). Address 0x8005227c.

Unit `class_3bb8c_k`, round 15. `./build-and-verify.sh` exit 0; whole-image
SHA1 matches retail. `funcdiff.py Class86F88__TickClosing`: 24/24 words match.

This is vtable slot `+0x058` of `gClass86F88Methods` (`Class86F88Methods` does not
declare this slot since nothing in this unit dispatches through it).

## Source

```c
void Class86F88__TickClosing(Class86F88 *self)
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
which is `Class86F88__SetState`, this unit's own occupant of that slot.

Matched on the first attempt once the two-guard-clause shape and the
unconditional-store-then-test-the-old-value idiom were both in place;
no register or scheduling residue.

## Naming

Round 75 (bravo, track 3). `func_8005227C` -> `Class86F88__TickClosing`, **tier B**.

Slot +0x058 (`tools/classtable.py gClass86F88Methods`), which Class86F88__OnNotify (class_3bb8c_j) dispatches for notifications from its tag-5 child. While `result` is 2 or 3 it counts calls in `closeTicks` and on the second call dispatches setState(self, 4). What the tag-5 child is (a per-frame tick?) is not established, hence tier B.

Class86F88, per the round-75 pass, is a scrolling list selector: up to 4 visible rows of 26-character item text, a highlighted cursor row, a horizontal column offset (see the unit header comment of `src/class_3bb8c_k.c`).
