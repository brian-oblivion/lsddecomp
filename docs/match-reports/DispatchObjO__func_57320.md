> Renamed from `func_80057320` on 2026-09-18 (tools/rename.py). Address 0x80057320.

# DispatchObjO__func_57320 -- MATCHED (25/25 words)

Unit: `class_3bb8c_o` (round 17). A tag-gated double dispatch: reads
`arg1`'s own vtable header LOW BYTE and calls one of two of `self`'s own
vtable slots depending on its value.

## Final source

```c
typedef struct DispatchObjO DispatchObjO;
typedef struct DispatchObjOMethods {
    u8 pad0[0xDC];                        /* +0x000 .. +0x0DB, unknown */
    void (*slotDC)(DispatchObjO *self);      /* +0x0DC */
    void (*slotE0)(DispatchObjO *self);        /* +0x0E0 */
} DispatchObjOMethods;
struct DispatchObjO {
    DispatchObjOMethods *methods;
};

void DispatchObjO__func_57320(DispatchObjO *self, TagByteObjO *arg1) {
    if (arg1->methods->tag == 0x34) {
        self->methods->slotDC(self);
    } else if (arg1->methods->tag == 0x24) {
        self->methods->slotE0(self);
    }
}
```

## Derivation

`lbu $v1,0($v0)` where `$v0 = *$a1` (i.e. `arg1->methods`) reads a BYTE,
not a masked word -- a DIFFERENT tag check than `BaseObjO__LinkCompanion`'s/
`BaseObjO__UnlinkCompanion`'s (which read the full header word and mask it). Kept as a
separate type, `TagByteObjO`, matching this. Both tag values (`0x34`,
`0x24`) are compile-time constants set up before the first comparison
(`ori $v0,$zero,0x24` sits in the FIRST branch's delay slot, ahead of where
it's actually used in the SECOND comparison -- ordinary delay-slot filler,
required no special handling once the `if`/`else if` was written in
retail's own textual order).

`self`'s own vtable dispatch (`self->methods->slotDC`/`slotE0`) is called
with NO explicit argument setup beyond `self` itself -- the call forwards
whatever was already in `$a0` (this function's own first parameter),
unchanged.

This function's OWN `self` type (`DispatchObjO`) is kept independent of
`BaseObjO` -- nothing else in this unit calls `DispatchObjO__func_57320`, and while
its "self" MIGHT be the same shared base class (the offsets `0xDC`/`0xE0`
are plausible padding gaps in `BaseObjOMethods` too), nothing here confirms
that relationship, so it is not asserted. `TagByteObjO` (the tag argument)
IS shared with `BaseObjO__func_571f8`'s `self->unk28` field, since both are
independently confirmed to compare the same byte against the same `0x34`
constant.

### Proposed learning

None -- see `BaseObjO__func_571f8`'s report for the shared `TagByteObjO` tag
convention.

## Naming

**`DispatchObjO__func_57320` -- tier C.** Mechanics are fully known (reads
`arg1`'s vtable tag byte, dispatches to one of `self`'s own two vtable
slots depending on whether it is `0x34` or `0x24`), but the report's own
class-identification section explicitly declines to assert a relationship
between `DispatchObjO` (this function's `self`) and `BaseObjO`, since
nothing in this unit calls `func_80057320` to test that relationship.
Kept `DispatchObjO` (this unit's own type for this unresolved class) as
the prefix rather than guessing `BaseObjO`.
