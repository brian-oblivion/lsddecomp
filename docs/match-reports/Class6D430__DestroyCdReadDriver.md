# Class6D430__DestroyCdReadDriver -- MATCHED (14/14 words)

> Renamed from `func_800288E0` on 2026-09-21 (tools/rename.py). Address 0x800288e0.

Unit: `code_179d8_h`. Runner: echo, round 17 (second assignment).

## Result

```c
void *Class6D430__DestroyCdReadDriver(Class6D430 *self) {
    return ((Class6D430Methods *)GetClass6D430Methods())->dtor(self);
}
```

Byte-exact, 14/14 words.

## Notes

Chains directly to `D_8006D430`'s own dtor slot (`GetClass6D430Methods()->dtor`,
i.e. `Class6D430__Destroy`, matched in `code_171e0.c`) rather than through
`self->methods` -- same "call the base class's own copy of a slot, not the
possibly-overridden one on `self`" idiom `DestroyChained` in that same file
uses for `Get_vtable_BasicClass()->dtor(this)`. Presumably this unit's own
subclass's chain-up dtor, mirroring `Class6D430__InstallCdReadDriver`'s chain-up ctor.
