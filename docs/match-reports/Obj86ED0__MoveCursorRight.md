# Obj86ED0__MoveCursorRight -- MATCH (25/25 words)

> Renamed from `func_8005165C` on 2026-09-24 (tools/rename.py). Address 0x8005165c.

Unit `class_3bb8c_i`. Obj86ED0's own "advance frame counter, clamped at
`unk10`" method. Mirror pair with `Obj86ED0__MoveCursorLeft` (decrement/clamp-at-zero,
matched alongside it) and `Obj86ED0__AdvanceCharSelect` (a second increment/clamp pair on
different fields, also matched this round).

```c
void Obj86ED0__MoveCursorRight(Obj86ED0 *self)
{
    s32 old;
    s32 v;

    if (self->unk48 != NULL) {
        old = self->unk18;
        v = old + 1;
        self->unk18 = v;
        if (v < self->unk10) {
            self->methods->slotA4(self, v, 1);
        } else {
            self->unk18 = old;
        }
    }
}
```

## Reading the shape

Retail's disassembly stores the incremented value to `self->unk18`
UNCONDITIONALLY, in the delay slot of the bound-check branch, then
overwrites it back to the OLD value in the taken (out-of-bound) branch. That
is not something to write directly in C ("store new value, then
conditionally restore old") -- it falls out naturally from writing the
ordinary shape above (`self->unk18 = v;` unconditionally before the `if`,
with `self->unk18 = old;` only in the else). GCC's delay-slot filler hoists
the first true-path-independent store into the branch delay slot on its own;
the source doesn't need to anticipate that.

Matched on the first attempt with this shape -- no residue, no header
changes (`slotA4`'s existing 2-arg-after-self signature already matched
this call site: `self->methods->slotA4(self, v, 1)`).

### Proposed learning

For a "compute new value / clamp against a bound / restore old value (or
reset) on overflow" pattern, write the unconditional store BEFORE the `if`
(not inside the true branch) even though the retail asm shows it living in
a branch delay slot -- the delay-slot placement is the scheduler's doing,
not something the C needs to spell out. `Obj86ED0__AdvanceCharSelect` in this same round
uses this exact same placement despite ALSO forwarding the new value on as
a live call argument -- moving the store into the true branch there (by
analogy, reasoning that a live call argument needed to be visibly tied to
the branch) was tried and made the score worse, not better. The
"unconditional store before the `if`" shape held for all three siblings in
this batch; that function's actual defect was an arity mismatch on the
callee vtable slot, not statement placement. See its report for the full
account.
