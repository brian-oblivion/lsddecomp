# BasicClass__ClearParentRefs

> Renamed from `BasicClass__func_1813c` on 2026-09-24 (tools/rename.py). Address 0x8001813c.

**Unit:** code_8220 · **Size:** 12 instructions · **Status:** MATCHED (12/12 words)

BasicClass vtable slot `+0x028` (`clearParentRefs`) — see
`BasicClass__BasicClass.md` for the class's overall design, and
`BasicClass__Finalize.md` (`finalize`) for the caller that dispatches
through this slot as the last step of tearing an object down.

## What it does

Frees every node in `self->parentRefs` (via `FreeBasicClassList`, still
`asm/TmdRenderer.s`, not yet carved — signature established last round
matching `BasicClass__RemoveAllChildren`: `void FreeBasicClassList(BasicClassListNode
**head)`, walks and frees every node but does not clear `*head` itself),
then nulls the list head. Order matters: the call happens BEFORE the
field is cleared, not after — the walk needs the old head value.

## The C

```c
void BasicClass__ClearParentRefs(BasicClass *self)
{
    FreeBasicClassList(&self->parentRefs);
    self->parentRefs = NULL;
}
```

## Provenance

round 11 (2026-09-03), runner delta, unit code_8220, second pass. Matched
first attempt.

## Naming (round 74)

`BasicClass__ClearParentRefs`, **tier A**: matches vtable slot `+0x028`
(`clearParentRefs`), already documented in `code_8220.h`. Frees every
node in `self->parentRefs` via `FreeBasicClassList`, then clears the
head pointer.
