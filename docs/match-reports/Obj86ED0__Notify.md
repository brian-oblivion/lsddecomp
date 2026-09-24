# Obj86ED0__Notify -- MATCHED (44/44 words)

> Renamed from `func_80050E78` on 2026-09-24 (tools/rename.py). Address 0x80050e78.

Unit `class_3bb8c_i`, carved round 14.

`Obj86ED0`'s slot38 override (vtable slot 0x038, the last slot BasicClass
itself defines). Dispatches the BASE class's own `slot38` first, then reads
the tag word off `arg1` exactly like `Obj86ED0__AddChild`/`Obj86ED0__RemoveChild`, and
routes to one of this class's OWN two extra slots (`slot58`/`slot5C`,
0x058/0x05C) via `self->methods` this time (not the base table) --
`slot5C` is itself `func_800513D0`, STALLED in this same unit (addiu-$at /
jump-table blocker, see its own report), so its type only needed naming,
not a body.

```c
void Obj86ED0__Notify(Obj86ED0 *self, void *arg1, s32 arg2)
{
    s32 tag;
    s32 mask;

    Get_vtable_BasicClass()->slot38(self, arg1, arg2);

    tag = **(s32 **)arg1;
    mask = tag & 0xF;
    if (mask == 2) {
        self->methods->slot5C(self, arg1, arg2);
    } else if (mask == 5) {
        self->methods->slot58(self, arg1, arg2);
    }
}
```

## Residue and how it was closed

First attempt cached `methods = self->methods;` up front (mirroring the
ALREADY-MATCHED `TaskObjF__Notify` in `src/class_3bb8c_f.c`, which does exactly
that for its own 4-way dispatch) and scored 13/44 with the frame GROWN by
8 bytes (`0x28` vs retail's `0x20`) -- an extra callee-saved register
(`$s3`) spilled to hold the cached `methods` pointer across the two
dispatch calls. Retail does NOT cache it: both `self->methods->slot5C(...)`
and `self->methods->slot58(...)` reload `self->methods` fresh from memory
at their own call site (`lw v0,0(a0)` immediately before each dispatch).
Dropping the local variable and writing `self->methods->slotN(...)` inline
at both call sites (two separate reloads, matching two separate register
lifetimes short enough not to need a saved register) closed it to 44/44.

### Proposed learning

Caching `self->methods` into a local is NOT free -- it is only the right
source shape when retail's own disassembly shows ONE load reused across
multiple calls. When each dispatch site reloads independently (two
`lw v0,0(a0)` instructions rather than one load followed by two uses of a
saved register), the source did NOT cache it either, and writing the cache
anyway costs a whole extra callee-saved register + frame growth that shows
up as an 8-byte size regression shifting every later function in the unit.
`TaskObjF__Notify`'s own cache was legitimate for THAT function (verify against
its own disassembly, not by analogy); this one was not.
