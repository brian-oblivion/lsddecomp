# SceneNode__Finalize

> Renamed from `Class6B5CC__Finalize` on 2026-09-26 (tools/rename.py). Address 0x8001cba4.

> Renamed from `func_8001CBA4` on 2026-09-23 (tools/rename.py). Address 0x8001cba4.

**Unit:** SceneNode · **Size:** 41 words · **Status:** MATCHED (41/41 words)

## What it does

`SceneNode`'s own destructor — vtable slot `+0x00C` of `gSceneNodeMethods`
(confirmed via `tools/classtable.py gSceneNodeMethods`). Calls three of its own
virtual teardown hooks in order (`slot50` = `SceneNode__DetachFromParent`, `slot54` =
`SceneNode__DetachAttachedChildren`, `slot5C` = `SceneNode__NoOpSlot5C` — all three still queued or,
for `SceneNode__NoOpSlot5C`, already a matched no-op stub elsewhere), frees the two
sub-blocks the constructor allocated (`self->unk14->unk44`, then
`self->unk14` itself), then tail-calls the BasicClass base destructor
(`GetBasicClassMethods()->dtor(self)`).

## The C

```c
void SceneNode__Finalize(SceneNodeObj *self) {
    self->methods->slot50(self);
    self->methods->slot54(self);
    self->methods->slot5C(self, 0);
    BMemPMgrFree(self->unk14->unk44);
    BMemPMgrFree(self->unk14);
    GetBasicClassMethods()->dtor(self);
}
```

## Note: `slot5C`'s local declared arity does not match its current occupant

This call site passes 2 arguments (`self`, `0`) to `self->methods->slot5C`.
That slot's CURRENT occupant, `SceneNode__NoOpSlot5C`, is already matched
elsewhere in this unit as a no-argument `void(void)` body (`{}`, a bare
`jr $ra`). Both are right about their own codegen — the callee ignores
every argument it's given, so the caller's arity is unconstrained. Typed
`slot5C` as `void (*)(SceneNodeObj *, s32)` in `include/SceneNode.h`
to match THIS call site; did not touch `SceneNode__NoOpSlot5C`'s own declaration.
Same precedent as `GetSceneNodeMethods`, documented in `include/class_3bb8c.h`.

## Provenance

round 11 (2026-09-03), runner charlie, unit SceneNode (fresh carve, first attempt).
Matched on the first build once `SceneNode__SceneNode`'s size-drift bug (see its
own report) was fixed — this function's own diff was already 41/41 before
that point; the WARNING about out-of-range bytes was entirely
`SceneNode__SceneNode`'s doing.

## Naming

Round 71 (alpha). `func_8001CBA4` -> `SceneNode__Finalize`, **tier A**. Table slot +0x00C, which BasicClass names `finalize` (include/code_8220.h). Body: detachFromParent, detachAttachedChildren, the empty +0x05C slot, frees the GsCOORD2PARAM and GsCOORDINATE2, then forwards to BasicClass finalize. Does not free self, matching the base slot's meaning; same name as Viewport__Finalize/StageMap__Finalize on their tables.

## Round 97 (alpha): Sony's GsCOORDINATE2

SceneNodeSub14 is deleted: SceneNode.coord2 is Sony's GsCOORDINATE2 (flg; MATRIX coord, whose t is the offset from the parent; MATRIX workm, whose t is the world position; param, super, sub -- 0x50 bytes, offset for offset). Accessors here follow the compiler's list: tx/ty/tz -> coord.t[0]/[1]/[2], unk38 -> workm.t; a local that holds coord.t or workm.t is `long *` (MATRIX.t is long[3]; s32 is int); any cast to GsCOORDINATE2 * is gone. Byte-identical.

## Round 101 (delta): track 7

Step 3 (locals and parameters): `sub` -> `coord2`. Byte-identical.

Step 5 (comments): The `coord2` local (a copy of self->coord2 used once) is gone: `BMemPMgrFree(self->coord2->param)` builds byte-identical, measured.
