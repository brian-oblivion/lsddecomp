# ItemList__CursorUp -- MATCH

> Renamed from `Class86F88__CursorUp` on 2026-09-26 (tools/rename.py). Address 0x800524f8.

> Renamed from `func_800524F8` on 2026-09-24 (tools/rename.py). Address 0x800524f8.

Unit `dream_scene`, round 15. `./build-and-verify.sh` exit 0; whole-image
SHA1 matches retail. `funcdiff.py ItemList__CursorUp`: 40/40 words match.

This is vtable slot `+0x084` of `gItemListMethods` (not declared in
`ItemListMethods` since nothing in this unit dispatches through it by
name; the arity used below is fixed by the two `slot94`/`slot98` call
sites, which ARE declared).

## Source

```c
void ItemList__CursorUp(ItemList *self, s32 arg1, s32 arg2, s32 arg3)
{
    s32 count;
    s32 newUnk20;
    s32 newUnk28;

    if (!self->unk50) {
        return;
    }
    count = self->unk28;
    if (count - 1 < 0) {
        return;
    }
    if (count - self->unk20 > 0) {
        self->methods->slot98(self, 0, 1, arg3);
    } else {
        self->unk20--;
        newUnk20 = self->unk20;
        self->unk28--;
        newUnk28 = self->unk28;
        self->methods->slot94(self, newUnk20, self->unk24, newUnk28, 1);
    }
}
```

## Notes

`arg1`/`arg2` are genuinely unused inside this function -- overwritten by
`self`-derived values before ever being read, matching this project's
"unused parameter forwarded verbatim" idiom; `arg3` survives untouched and
is forwarded as `slot98`'s 4th argument on the one path that reaches it.

**Branch polarity was inverted on the first attempt and had to be
flipped.** The natural `if (diff <= 0) { decrement; slot94 } else {
slot98 }` reproduced the right VALUES but the wrong CFG -- retail's
`blez`/fallthrough pair places the `slot98` call as the physical
fallthrough (untaken) path and the decrement+`slot94` call as the branch
TARGET, which is the opposite of what that natural phrasing produces.
Flipping to `if (diff > 0) { slot98 } else { decrement; slot94 }` fixed
the branch instruction (`blez` vs `bgtz`) and the physical block order in
one change.

**A second, independent residue remained after the polarity fix: a
register-identity swap in the `slot94` call's arguments, in the SAME
family as `ItemList__ScrollRight`'s.** `self->unk20--; self->unk28--;` followed
directly by `self->methods->slot94(self, self->unk20, self->unk24,
self->unk28, 1);` reproduced every value but landed the post-decrement
`unk20`/`unk28` in the wrong registers relative to retail. `permuter.py`
(zero found at iteration 1815 of a ~150s run) confirmed the fix is a
double-read: assign the just-decremented field into its own named local
(`newUnk20 = self->unk20;` right after `self->unk20--;`, same for
`unk28`) and pass the LOCALS to the call rather than re-reading
`self->unk20`/`self->unk28` inline. No UB or barrier needed here, unlike
`ItemList__ScrollRight`'s permuter run which also surfaced a `(float)` cast that
turned out to be unnecessary.

### Proposed learning

See `ItemList__ScrollRight`'s report for the general form of this lever
(double-assign a value the permuter reads twice). This function is the
second confirmed instance in the same round, on a POST-DECREMENT value
rather than a freshly-loaded field, which broadens the pattern: it is not
specific to "reading a struct field for a guard", it applies to "storing
a locally-mutated value back to the struct, then immediately reusing that
same value as a call argument."

## Naming

Round 75 (bravo, track 3). `func_800524F8` -> `ItemList__CursorUp`, **tier A**.

Slot +0x084 (`tools/classtable.py gItemListMethods`). Decrements `cursorIndex`: inside the window via stepCursorInView(self, 0, 1), or at the top row by decrementing both `topIndex` and `cursorIndex` and redrawing. Rows are laid out 0xA apart in increasing y (ItemList__CreateRows), so a lower index is higher on screen. Dispatched by HandleInputCode on code 18.

ItemList, per the round-75 pass, is a scrolling list selector: up to 4 visible rows of 26-character item text, a highlighted cursor row, a horizontal column offset (see the unit header comment of `src/world/dream_scene.c`).

## Round 99 (delta, track 7)

Parameters `arg1`, `arg2`, `arg3` -> `unused1`, `unused2`, `forwarded` (also in include/item_list.h's prototype): the first two are never read, the third is passed to stepCursorInView as a fourth word its occupant does not read (include/item_list.h banner). The branch polarity and the newTop/newCursor locals keep a `MATCHING:` line.
