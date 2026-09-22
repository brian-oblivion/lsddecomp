# Class869D8__ForwardIfUnk10AndUnk70

> Renamed from `func_8004D300` on 2026-09-22 (tools/rename.py). Address 0x8004d300.

**Unit:** class_3bb8c_c · **Size:** 23 words · **Status:** MATCHED (23/23)

## What it does

One of `Class869D8`'s own methods. Gates a base-class hook call on two of
the object's own fields both being nonzero, then forwards to the base
ctor table's own `+0x09C` slot with `self` as the only argument.

## The C

```c
void Class869D8__ForwardIfUnk10AndUnk70(Class869D8 *self)
{
    if (self->unk10 != 0 && self->unk70 != 0) {
        func_8003F24C()->slot9C(self);
    }
}
```

## New/changed header content (`include/class_3bb8c.h`)

- `Class869D8` struct: added `unk10` (s32, +0x010) and `unk70` (s32,
  +0x070), both gate fields read here. Padding added to keep the struct at
  its allocation size, 0xDC (from `New_Class869D8`'s `func_80017B34(0xDC)`
  call) -- purely additive, the existing `methods` field at +0x000 is
  untouched.
- `BaseCtorTable_3bb8c_c` (func_8003F24C's return type): added `slot9C`
  (`void (*)(void *self)`, +0x09C). Additive; the existing `ctor` slot
  (+0x008, used by `Class869D8__Class869D8`) is untouched.

## Notes

Matched on the first attempt, no residue. The two-field nonzero gate
(`self->unk10 != 0 && self->unk70 != 0`) compiles straightforwardly to the
retail two-branch sequence (`beqz` on each field in turn, both skipping to
the same tail) with no reordering needed.

## Proposed learning

None beyond what's already documented for this unit's base-ctor-table
pattern.

## Naming

**Class869D8__ForwardIfUnk10AndUnk70** -- tier B. Mechanics are fully
evident from the body (forward to the base ctor table's `slot9C` iff both
`unk10` and `unk70` are nonzero) but the fields' real meaning, and so the
forward's in-game purpose, is not established -- only that both gate the
call (see the struct comments). Named on the "NotifyIfUnkNActive"-style
precedent already used elsewhere in this codebase for a gated-forward shape
(e.g. `Class6B5CC__NotifyIfUnk20Active`, include/code_d294.h) rather than
inventing a semantic verb ("Notify"/"Release"/etc.) the body does not
support. `unk10`/`unk70` themselves are left unnamed -- no evidence beyond
"nonzero gate" exists for either.
