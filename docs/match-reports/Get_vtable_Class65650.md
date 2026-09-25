# Get_vtable_Class65650

> Renamed from `func_80066818` on 2026-09-24 (tools/rename.py). Address 0x80066818.

**Unit:** code_55dd4 · **Size:** 4 words (0x10 bytes) · **Status:** MATCHED
(4/4 words, whole-image `./build-and-verify.sh` green)

## What it does

Returns the address of `gClass65650Methods`, this class's own 80-slot method table
(header word `0x234`, per `tools/classtable.py gClass65650Methods --vs 0x800878D4`).
The `Get_vtable`-style accessor for this class, same shape as
`Get_vtable_DreamSys` and `GetClass6D3C8Methods` (`code_171e0`'s equivalent for
`D_8006D3C8`): a plain `lui`/`addiu` address computation, no load — this is
`&gClass65650Methods`, not `*gClass65650Methods`.

```c
Class65650Methods *Get_vtable_Class65650(void)
{
    return &gClass65650Methods;
}
```

Called from both `New_Class65650` and `Class65650__Class65650` — in both
cases retail actually `jal`s this function rather than inlining the
`lui`/`addiu` at the call site, so the callers must write
`Get_vtable_Class65650()`, not `&gClass65650Methods` directly, to reproduce the call
instruction.

### Proposed learning

Same as `GetClass6D3C8Methods.md`: a function that only does `lui`/`addiu` to a
symbol with no surrounding `lw`/`sw` is returning `&symbol`. Additionally
worth stating explicitly: when OTHER functions in the unit call this
accessor rather than referencing the symbol directly, write the call in the
caller too (`Get_vtable_Class65650()`), even though `&gClass65650Methods` would be
semantically identical — retail's own bytes are the call, not the inlined
address computation.

## Naming

Round 75 (charlie), track 3.

- `Get_vtable_Class65650` (was `func_80066818`), tier A. Returns &gClass65650Methods (lui/addiu). Named like Get_vtable_Entity, the same accessor for the subclass. Entity.c calls it as its base-table getter.
- `gClass65650Methods` (was `D_8008A6C4`), tier A. The 80-slot method table (header 0x234) this function returns and the ctor installs; `g<Class>Methods` like gClass876FCMethods/gClass6B5CCMethods.

## Track 4 (2026-09-25, round 85, alpha)

The class (id 0x234, table `gClass65650Methods`) is unified as `Class65650` in `include/Class65650.h`: an Actor subclass (its ctor chains to Actor's first) and Entity's base. Any source block above is the pre-unification spelling (the local `Class65650Methods` of `include/code_55dd4.h`, `linkCompanion`/`unlinkCompanion`, `companion2`, `Unk5CObj`/`Unk70ElemObj`); the live body in `src/code_55dd4.c` takes the unified types and the inherited slot and field names (`addChild`/`removeChild`, Actor's `ticker`, `Actor *` parts, `ModelData *` modelData), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
