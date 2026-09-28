# SceneNode__RemoveAllChildren

> Renamed from `Class6B5CC__RemoveAllChildren` on 2026-09-26 (tools/rename.py). Address 0x8001cd20.

> Renamed from `func_8001CD20` on 2026-09-23 (tools/rename.py). Address 0x8001cd20.

**Unit:** SceneNode · **Size:** 16 words · **Status:** MATCHED (16/16 words)

## What it does

`SceneNode` vtable slot `+0x018`, the last of the five BasicClass
overrides. Calls `SceneNode__UnlinkModel(self)` (zeroes `self->unk18`/`unk20`),
then unconditionally forwards to the base class's own `+0x018` slot.

## The C

```c
void SceneNode__RemoveAllChildren(SceneNodeObj *self) {
    SceneNode__UnlinkModel(self);
    GetBasicClassMethods()->slot18(self);
}
```

## Provenance

round 11 (2026-09-03), runner charlie, unit SceneNode, second pass. Matched on the first build.
`SceneNode__UnlinkModel`'s own (measured) body is what confirmed
`SceneNodeObj::unk18`/`unk20` independently of the ctor's own zeroing —
see `include/SceneNode.h`.

## Naming

Round 71 (alpha). `func_8001CD20` -> `SceneNode__RemoveAllChildren`, **tier A**. Overrides BasicClass slot +0x018 `removeAllChildren`: SceneNode__UnlinkModel, then the base implementation.
