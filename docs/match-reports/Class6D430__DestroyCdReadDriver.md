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

## Naming (round 64, runner alpha)

`func_800288E0` -> `Class6D430__DestroyCdReadDriver`, tier B. Named as the
paired teardown for `Class6D430__InstallCdReadDriver` (adjacent in ROM
order, same `Class6D430*` parameter, the established "ctor chains base then
installs a derived vtable / dtor chains directly to the base's own dtor"
idiom this codebase already uses at the `DestroyChained` level for
`BasicClass`). The function's OWN body only chains to `Class6D430`'s real
dtor slot -- it does not itself reference `D_8006D4E8` or
`GetClass6D4E8Methods` -- so the "CdReadDriver" half of the name is
justified by the PAIRING with the ctor, not by this function's own body;
flagged explicitly in case a future runner finds a caller that shows these
two are NOT actually a matched pair.
