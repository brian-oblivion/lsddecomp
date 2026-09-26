# Get_vtable_TodActor

> Renamed from `Get_vtable_Class65650` on 2026-09-26 (tools/rename.py). Address 0x80066818.

> Renamed from `func_80066818` on 2026-09-24 (tools/rename.py). Address 0x80066818.

**Unit:** code_55dd4 · **Size:** 4 words (0x10 bytes) · **Status:** MATCHED
(4/4 words, whole-image `./build-and-verify.sh` green)

## What it does

Returns the address of `gTodActorMethods`, this class's own 80-slot method table
(header word `0x234`, per `tools/classtable.py gTodActorMethods --vs 0x800878D4`).
The `Get_vtable`-style accessor for this class, same shape as
`Get_vtable_DreamSys` and `GetGameApplicationMethods` (`code_171e0`'s equivalent for
`gGameApplicationMethods`): a plain `lui`/`addiu` address computation, no load — this is
`&gTodActorMethods`, not `*gTodActorMethods`.

```c
TodActorMethods *Get_vtable_TodActor(void)
{
    return &gTodActorMethods;
}
```

Called from both `New_TodActor` and `TodActor__TodActor` — in both
cases retail actually `jal`s this function rather than inlining the
`lui`/`addiu` at the call site, so the callers must write
`Get_vtable_TodActor()`, not `&gTodActorMethods` directly, to reproduce the call
instruction.

### Proposed learning

Same as `GetGameApplicationMethods.md`: a function that only does `lui`/`addiu` to a
symbol with no surrounding `lw`/`sw` is returning `&symbol`. Additionally
worth stating explicitly: when OTHER functions in the unit call this
accessor rather than referencing the symbol directly, write the call in the
caller too (`Get_vtable_TodActor()`), even though `&gTodActorMethods` would be
semantically identical — retail's own bytes are the call, not the inlined
address computation.

## Naming

Round 75 (charlie), track 3.

- `Get_vtable_TodActor` (was `func_80066818`), tier A. Returns &gTodActorMethods (lui/addiu). Named like Get_vtable_Entity, the same accessor for the subclass. Entity.c calls it as its base-table getter.
- `gTodActorMethods` (was `D_8008A6C4`), tier A. The 80-slot method table (header 0x234) this function returns and the ctor installs; `g<Class>Methods` like gClass876FCMethods/gSceneNodeMethods.

## Track 4 (2026-09-25, round 85, alpha)

The class (id 0x234, table `gTodActorMethods`) is unified as `TodActor` in `include/TodActor.h`: an Actor subclass (its ctor chains to Actor's first) and Entity's base. Any source block above is the pre-unification spelling (the local `TodActorMethods` of `include/code_55dd4.h`, `linkCompanion`/`unlinkCompanion`, `companion2`, `Unk5CObj`/`Unk70ElemObj`); the live body in `src/code_55dd4.c` takes the unified types and the inherited slot and field names (`addChild`/`removeChild`, Actor's `ticker`, `Actor *` parts, `ModelData *` modelData), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
