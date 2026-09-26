# FileResource__DestroyCdReadDriver -- MATCHED (14/14 words)

> Renamed from `Class6D430__DestroyCdReadDriver` on 2026-09-26 (tools/rename.py). Address 0x800288e0.

> Renamed from `func_800288E0` on 2026-09-21 (tools/rename.py). Address 0x800288e0.

Unit: `code_179d8_h`. Runner: echo, round 17 (second assignment).

## Result

```c
void *FileResource__DestroyCdReadDriver(FileResource *self) {
    return ((FileResourceMethods *)GetFileResourceMethods())->dtor(self);
}
```

Byte-exact, 14/14 words.

## Notes

Chains directly to `D_8006D430`'s own dtor slot (`GetFileResourceMethods()->dtor`,
i.e. `FileResource__Finalize`, matched in `code_171e0.c`) rather than through
`self->methods` -- same "call the base class's own copy of a slot, not the
possibly-overridden one on `self`" idiom `FileResource__Release` in that same file
uses for `Get_vtable_BasicClass()->dtor(this)`. Presumably this unit's own
subclass's chain-up dtor, mirroring `FileResource__InstallCdReadDriver`'s chain-up ctor.

## Naming (round 64, runner alpha)

`func_800288E0` -> `FileResource__DestroyCdReadDriver`, tier B. Named as the
paired teardown for `FileResource__InstallCdReadDriver` (adjacent in ROM
order, same `FileResource*` parameter, the established "ctor chains base then
installs a derived vtable / dtor chains directly to the base's own dtor"
idiom this codebase already uses at the `FileResource__Release` level for
`BasicClass`). The function's OWN body only chains to `FileResource`'s real
dtor slot -- it does not itself reference `gCdDriverMethods` or
`GetCdDriverMethods` -- so the "CdReadDriver" half of the name is
justified by the PAIRING with the ctor, not by this function's own body;
flagged explicitly in case a future runner finds a caller that shows these
two are NOT actually a matched pair.
