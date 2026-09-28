# GetPadMethods

> Renamed from `Get_vtable_Pad` on 2026-09-28 (tools/rename.py). Address 0x80025e9c.

> Renamed from `func_80025E9C` on 2026-09-24 (tools/rename.py). Address 0x80025e9c.

**Unit:** `src/app/Pad.c` (runner ALPHA, `runner/alpha`)
**Status:** MATCHED (4/4 words, full build verified byte-exact)
**Vtable slot:** none -- this function *returns* the table itself.

## Context

Trivial accessor: returns the address of this class's own 21-slot method
table, `gPadMethods` (see `tools/classtable.py gPadMethods`). Called by
`New_Pad` (the `New_`-style allocator+constructor for this class) to
fetch the constructor slot, and again internally by `Pad__Pad` (the
constructor itself) to install `self->methods`.

## Final C

```c
PadMethods *GetPadMethods(void) {
    return &gPadMethods;
}
```

`gPadMethods` and `PadMethods` are declared in `include/Pad.h`. The
table's bytes are NOT owned by this unit -- its file offset (0x5DB70) falls
inside the anonymous `data` segment starting at file offset 0x57070 in
`config/splat.slps01556.lsdde.yaml` (still `type: data`, not `.data,
Pad`), i.e. it is auto-emitted raw data, not something this unit
needs to author as C. `extern PadMethods gPadMethods;` is enough; the linker
resolves it against splat's own auto-generated label for that address (the
same way the pre-existing raw `.s` for this function already referenced it
via `%hi`/`%lo(gPadMethods)`).

## Derivation notes

First-try match, no iteration needed -- straight address-of a global.

### Proposed learning

`config/DECOMPILATION_LEARNINGS.md`'s "four dead-looking data slots" open
question (`0x57070`, `0x76DC8`, `0x79528`, sbss runs) is at least partly
explained for `0x57070`: it holds (at least) this class's method table,
`gPadMethods`, and very likely the neighbouring `BASICCLASS_METHODS` table
(`gBasicClassMethods`) and others of the 60 tables `classtable.py --scan` finds --
worth cross-referencing all 60 table addresses against that data segment's
range before calling it "unidentified."

## Naming

**Tier A.** Renamed from `func_80025E9C` to match the project-wide
`Get_vtable_X` convention for a class's own no-argument vtable-table
accessor (`GetBasicClassMethods`, `GetEntityMethods`, `GetDreamSysMethods`,
`Get_vtable_TaskCore`, `GetTodActorMethods`, etc. -- all the identical
`return &gXMethods;` shape). The body itself (`return &gPadMethods;`) is a
pure leaf whose mechanics are its whole purpose.
