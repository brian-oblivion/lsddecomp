# Class86AA0__ForwardIfTag34

> Renamed from `func_8004D434` on 2026-09-22 (tools/rename.py). Address 0x8004d434.

**Unit:** class_3bb8c_c · **Size:** 18 words · **Status:** MATCHED (18/18)

## What it does

One of `Class86AA0`'s own methods (this table's slot +0x09C). Reads a
second argument's own vtable header-tag byte (the low byte of the header
word at the argument's `methods` pointer's own +0x000) and, if it equals
the literal `0x34`, forwards to `self`'s own slot +0x0B8 with no other
arguments.

## The C

```c
void Class86AA0__ForwardIfTag34(Class86AA0 *self, GenericTagInst_3bb8c_c *arg1)
{
    if (arg1->methods->tag == 0x34) {
        self->methods->slotB8(self);
    }
}
```

## New type: GenericTagInst_3bb8c_c / GenericTagMethods_3bb8c_c

Every vtable in this codebase opens with a header word whose low byte
carries what looks like a small per-class tag (e.g. `gClass869D8Methods`'s header
is `0x17`, `gClass86AA0Methods`'s is `0x24`) -- this function is the first one in
this unit that actually reads that byte back out and compares it against a
literal, rather than just following the header past it to a named slot.
Modeled as a minimal generic "any class instance" shape (`methods` pointer
at +0x000, whose own first byte is the tag) since nothing in this unit
identifies which specific class `0x34` names.

## Notes

Matched on the first attempt. No residue -- straightforward branch, single
dispatch call, no register-identity surprises.
