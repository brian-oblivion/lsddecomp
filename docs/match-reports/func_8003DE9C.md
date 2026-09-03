# func_8003DE9C — MATCHED (65/65)

**Unit:** code_2cc8c_b · **Size:** 65 words · **Result:** byte-exact, first attempt

## What it does

The last function in this carve's original 20-function range. Dispatches
two elements of a doubly-indexed pointer array (`self->unk64[idx]` as the
outer array, indexed by both the current ring counter
`self->unk60[idx]` and the caller-supplied `a1`) through their own
`slotB8`, each with a different fixed buffer derived from `self->unk4C`.
Then installs `a1` as the new ring counter, optionally notifies via
`slot70`, and always fires `slot60(self, 9)` — the same closing shape as
`func_8003DA10` and `func_8003D4DC`.

```c
void func_8003DE9C(Obj86B60 *self, s32 a1, void *a2)
{
    s32 idx;
    s32 counter;
    Unk64Elem **arr;
    Unk64Elem *elem1;
    Unk64Elem *elem2;
    u8 *buf;

    idx = self->unk58;
    counter = self->unk60[idx];
    arr = (Unk64Elem **)self->unk64[idx];
    elem1 = arr[counter];
    elem2 = arr[a1];
    elem1->methods->slotB8(elem1, self->unk4C->unk10);
    buf = (u8 *)self->unk4C->unk24[idx] + 8;
    elem2->methods->slotB8(elem2, buf);
    self->unk60[idx] = a1;
    if (a2 != NULL) {
        self->methods->slot70(self, 0);
    }
    self->methods->slot60(self, 9);
}
```

No header changes — reuses every field and slot already modelled
(`unk58`, `unk60`, `unk64`, `unk4C`, `unk4C->unk10`, `unk4C->unk24`,
`Unk64Elem`/`slotB8`, `slot70`, `slot60`).

## Residue

None — matched on the first attempt. Two things from this unit's prior
reports were applied directly rather than re-derived:

- `self->unk60[idx] = a1;` written UNCONDITIONALLY, before the
  `if (a2 != NULL)` — reusing `func_8003D4DC`'s finding that a store which
  only looks relevant inside a following `if` can still belong outside it
  if retail's delay slot shows it executing unconditionally (confirmed
  here too: the store sits in the `beqz`'s delay slot).
- `(u8 *)self->unk4C->unk24[idx] + 8` for the second buffer, reusing
  `func_8003DA10`'s exact derivation for the identical expression.

## Field-read-order note (per head's request)

**This function does NOT follow the `unk58 -> unk64 -> unk5C/unk60` order
established by `func_8003D6D4`/`func_8003DDC8`/`func_8003DE30`/
`func_8003D980`/`func_8003DA10`.** Retail's own base-pointer reads here go
`unk58, unk60, unk64, unk4C` — `unk60` BEFORE `unk64`, the reverse of every
prior instance. Writing the C in that reversed order (`counter =
self->unk60[idx];` before `arr = self->unk64[idx];`) matched immediately
with no residue, so the reversal is a real, source-driven fact about this
function, not noise. **The standing order is therefore per-function, not
a unit-wide invariant** — it tracks whichever field the ORIGINAL SOURCE
happened to reference first, which differs here (this function's first
real use is the ring counter, whereas the four earlier functions all use
`unk64`/`unk5C` as their first substantive computation). Six functions now
observed total: five follow `unk64` before `unk60`/`unk5C`, this one
reverses it. Read the disassembly's own base-pointer load order per
function rather than assuming the majority order.
