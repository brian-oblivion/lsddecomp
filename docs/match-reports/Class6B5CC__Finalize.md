# Class6B5CC__Finalize

> Renamed from `func_8001CBA4` on 2026-09-23 (tools/rename.py). Address 0x8001cba4.

**Unit:** code_d294 · **Size:** 41 words · **Status:** MATCHED (41/41 words)

## What it does

`Class6B5CC`'s own destructor — vtable slot `+0x00C` of `gClass6B5CCMethods`
(confirmed via `tools/classtable.py gClass6B5CCMethods`). Calls three of its own
virtual teardown hooks in order (`slot50` = `Class6B5CC__DetachFromParent`, `slot54` =
`Class6B5CC__DetachAttachedChildren`, `slot5C` = `Class6B5CC__func_1d33c` — all three still queued or,
for `Class6B5CC__func_1d33c`, already a matched no-op stub elsewhere), frees the two
sub-blocks the constructor allocated (`self->unk14->unk44`, then
`self->unk14` itself), then tail-calls the BasicClass base destructor
(`Get_vtable_BasicClass()->dtor(self)`).

## The C

```c
void Class6B5CC__Finalize(Class6B5CCObj *self) {
    self->methods->slot50(self);
    self->methods->slot54(self);
    self->methods->slot5C(self, 0);
    BMemPMgrFree(self->unk14->unk44);
    BMemPMgrFree(self->unk14);
    Get_vtable_BasicClass()->dtor(self);
}
```

## Note: `slot5C`'s local declared arity does not match its current occupant

This call site passes 2 arguments (`self`, `0`) to `self->methods->slot5C`.
That slot's CURRENT occupant, `Class6B5CC__func_1d33c`, is already matched
elsewhere in this unit as a no-argument `void(void)` body (`{}`, a bare
`jr $ra`). Both are right about their own codegen — the callee ignores
every argument it's given, so the caller's arity is unconstrained. Typed
`slot5C` as `void (*)(Class6B5CCObj *, s32)` in `include/code_d294.h`
to match THIS call site; did not touch `Class6B5CC__func_1d33c`'s own declaration.
Same precedent as `GetClass6B5CCMethods`, documented in `include/class_3bb8c.h`.

## Provenance

round 11 (2026-09-03), runner charlie, unit code_d294 (fresh carve, first attempt).
Matched on the first build once `Class6B5CC__Class6B5CC`'s size-drift bug (see its
own report) was fixed — this function's own diff was already 41/41 before
that point; the WARNING about out-of-range bytes was entirely
`Class6B5CC__Class6B5CC`'s doing.

## Naming

Round 71 (alpha). `func_8001CBA4` -> `Class6B5CC__Finalize`, **tier A**. Table slot +0x00C, which BasicClass names `finalize` (include/code_8220.h). Body: detachFromParent, detachAttachedChildren, the empty +0x05C slot, frees the GsCOORD2PARAM and GsCOORDINATE2, then forwards to BasicClass finalize. Does not free self, matching the base slot's meaning; same name as Viewport__Finalize/Class866E8__Finalize on their tables.
