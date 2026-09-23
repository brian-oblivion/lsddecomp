# Class869D8__Class869D8

> Renamed from `func_8004D2A4` on 2026-09-22 (tools/rename.py). Address 0x8004d2a4.

**Unit:** class_3bb8c_c · **Size:** 21 words · **Status:** MATCHED (21/21)

## What it does

The constructor (`ctor`, slot +0x008) for `Class869D8`. Standard
class-framework shape, same as e.g. class_39e08.c's `func_8004A19C`: chain
to a base class's ctor (fetched via `GetUnk18ObjMethods()`), install this
class's own vtable, then call the just-installed vtable's own
post-construct hook (slot +0x040, currently `func_8004D2F8` -- already
matched, an empty body).

## The C

```c
void Class869D8__Class869D8(Class869D8 *self)
{
    GetUnk18ObjMethods()->ctor(self);
    self->methods = GetClass869D8Methods();
    self->methods->slot40(self);
}
```

## Notes

`GetUnk18ObjMethods` is a base-ctor-table getter this unit has no other
evidence about beyond this one call site: the retail instruction sequence
sets up only `self` for the ctor call (no second argument register is
touched), so it's declared minimally as
`extern BaseCtorTable_3bb8c_c *GetUnk18ObjMethods(void);` with
`BaseCtorTable_3bb8c_c` holding only a `ctor(void *self)` field at +0x008
(see `include/class_3bb8c.h`). This is a fresh, file-local extern
declaration -- it does not need to agree with any other unit's own
(possibly differently-typed) declaration of the same external symbol,
since each translation unit's local prototype only has to reproduce ITS
OWN call site's register usage.

Built and verified byte-for-byte as part of a 9-function batch across this
whole unit; `./build-and-verify.sh` reports a clean whole-image SHA1 match.

## Naming

**Class869D8__Class869D8** -- tier A. Canonical ctor (`Class__Class`
convention, e.g. `Class866E8__Class866E8`): chains the base ctor, installs
this class's own vtable via `GetClass869D8Methods()`, then dispatches the
freshly-installed table's own post-construct hook. The identity of the
class and the fact that this occupies its own `ctor` slot (+0x008) are both
evident from the body and from the vtable dump (`tools/classtable.py
0x800869D8`).
