# SceneNode__DetachFromParent

> Renamed from `Class6B5CC__DetachFromParent` on 2026-09-26 (tools/rename.py). Address 0x8001d1a4.

> Renamed from `func_8001D1A4` on 2026-09-23 (tools/rename.py). Address 0x8001d1a4.

**Unit:** code_d294 · **Size:** 24 words · **Status:** MATCHED (24/24 words)

## What it does

`SceneNode` vtable slot `+0x050`, the "detach" half of the attach/detach
pair with `SceneNode__AttachToParent` (`+0x04C`). If `self->unkC` (the owner) is set,
calls `owner->methods->slot14(owner, self)` (the owner's own "unregister a
child" hook), clears `self->unk14->unk48`, and clears `self->unkC`.
Always returns `self`.

## The C

```c
SceneNodeObj *SceneNode__DetachFromParent(SceneNodeObj *self) {
    UnkOwner_d294 *owner;

    owner = self->unkC;
    if (owner != NULL) {
        owner->methods->slot14(owner, self);
        self->unk14->unk48 = 0;
        self->unkC = NULL;
    }
    return self;
}
```

Unlike `SceneNode__AttachToParent` (`docs/match-reports/SceneNode__AttachToParent.md`), this one's
early/late "return self" DON'T need merging into a single exit -- retail's
own disassembly sets `v0 = self` in the branch delay slot for BOTH the
"owner is NULL" skip-ahead path and the normal path, i.e. retail itself
computes it twice here (once per path), matching a plain `if (...) {...}
return self;` shape with no restructuring needed. Contrast with
`SceneNode__AttachToParent`, where retail computed it only ONCE for both paths and the
C had to be reshaped to match. Same observation, opposite answer -- read
each function's own disassembly rather than assuming last function's fix
generalizes unchanged.

## Provenance

round 11 (2026-09-03), runner charlie, unit code_d294, second pass. Matched on the first build (no
iteration needed). Cross-checks `SceneNode__AttachToParent`'s `UnkOwner_d294` type:
both functions dispatch through the identical `+0x010`/`+0x014` slot
shape on whatever `self->unkC` points to.

## Naming

Round 71 (alpha). `func_8001D1A4` -> `SceneNode__DetachFromParent`, **tier A**. Table slot +0x050. If attached: parent->removeChild(self) (BasicClass slot +0x014), coord2->super = 0, self->unkC = NULL. Exact inverse of SceneNode__AttachToParent. Finalize calls it first.

## Round 101 (delta): track 7

Step 3 (locals and parameters): `owner` -> `parent`. Byte-identical.
