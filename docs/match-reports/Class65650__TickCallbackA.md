# Class65650__TickCallbackA

> Renamed from `func_80066150` on 2026-09-24 (tools/rename.py). Address 0x80066150.

**Unit:** code_55dd4 · **Size:** 29 words (0x74 bytes) · **Status:** MATCHED
(29/29 words, whole-image `./build-and-verify.sh` green)

## What it does

`Class65650Methods` slot `+0x118`. Unconditionally calls the class's own
vtable slot `+0x0C4` with two literal arguments (`-0x1E`, `0`), then, only
if `self->unk64 == 1` **and** `self->unk68` is set, calls `unk68`'s own
vtable slot `+0x088` with the literal `6`.

```c
void Class65650__TickCallbackA(Class65650 *self)
{
    self->methods->slotC4(self, -0x1E, 0);
    if (self->unk64 == 1 && self->unk68 != NULL) {
        self->unk68->methods->slot88(self->unk68, 6);
    }
}
```

The `&&` reproduces retail's two chained `beqz`/`bne` guards exactly: the
first false short-circuits past the second check entirely, matching a
single combined condition rather than nested `if`s (which would still be
semantically equivalent but is not what was tried first — the combined form
matched immediately).

Corrects `self->unk68`'s type from the generic `void *` the first pass gave
it to `Unk68Obj *` (a new minimal type in `include/code_55dd4.h`, typed
only at its `+0x088` slot, the only one this unit calls). Adds `slotC4`
(`+0x0C4`) to `Class65650Methods`.

Matched on the direct translation, no reshaping.

### Proposed learning

Two independent field-guard branches, the first `bne`/`beqz`-style skipping
straight past the second on failure, read naturally as a single `&&`
condition rather than nested `if`s — worth trying `&&` first on this shape
before nesting.

## Naming

Round 75 (charlie), track 3.

- `Class65650__TickCallbackA` (was `func_80066150`), tier B. Occupies +0x118, the callback SelectTickCallback installs for 'A' (InitDefaults' default). Calls slotC4 (BaseObjO__func_5748c) with (-0x1E, 0), and if unk64 == 1 and mainPart is set, mainPart->slot88(6) (BaseObjO__func_571f8). Neither callee has a purpose name yet, hence B.
