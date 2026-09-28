# SceneNode__AddChild

> Renamed from `Class6B5CC__AddChild` on 2026-09-26 (tools/rename.py). Address 0x8001cc48.

> Renamed from `func_8001CC48` on 2026-09-23 (tools/rename.py). Address 0x8001cc48.

**Unit:** SceneNode · **Size:** 27 words · **Status:** MATCHED (27/27 words)

## What it does

`SceneNode` vtable slot `+0x010`, one of the five BasicClass overrides
(`tools/classtable.py gSceneNodeMethods --vs gBasicClassMethods`). Forwards unconditionally
to the base class's own `+0x010` slot (`GetBasicClassMethods()->slot10`), then, if
`other`'s own vtable header tag (`other->methods->header & 0xF`) is `9`,
additionally calls `SceneNode__LinkModel(self, other)` (still uncarved, next
slice). Its sibling `SceneNode__RemoveChild` (`+0x014`) is the mirror-image
"detach" of this "attach".

## The C

```c
void SceneNode__AddChild(SceneNodeObj *self, GenericObj_d294 *other) {
    GetBasicClassMethods()->slot10(self, other);
    if ((other->methods->header & 0xF) == 9) {
        SceneNode__LinkModel(self, other);
    }
}
```

## Provenance

round 11 (2026-09-03), runner charlie, second pass, unit
SceneNode. Matched on the first build. Established
`BasicClassMethodsD294`'s `+0x010` slot (2-arg, `(self, other)`) and the
`GenericObj_d294` generic-dispatch type (header tag comparison), reused by
`SceneNode__RemoveChild`/`SceneNode__DetachAttachedChildren` below.

## Naming

Round 71 (alpha). `func_8001CC48` -> `SceneNode__AddChild`, **tier A**. Overrides BasicClass slot +0x010 `addChild` (include/code_8220.h). Forwards to the base first, then if the child's class tag is 9 (gTmdModelMethods) calls SceneNode__LinkModel on it. SceneNode__AttachToParent reaches this slot on the parent with `self` as the child.

## Proposed field names

For the head to apply by type scope. Each one fails to compile in another unit when renamed in the definition, so this unit did not apply it.

- `SceneNodeObj.unk18` -> `tmd` (tier B): GsDOBJ2.tmd by offset. SceneNode__LinkModel stores the tag-9 child's +0x10 word there and UnlinkModel clears it. Accessors: SceneNode, code_d294_c.
- `SceneNodeObj.unk20` -> `linkedModel` (tier B): SceneNode__LinkModel stores the tag-9 child object itself; code_d294_b passes it to the psyq_fa50 helpers. Accessors: SceneNode, code_d294_b, code_d294_c.

## Round 101 (delta): track 7

Step 4 (constants): The unit-local `CLASS_TAG_MASK` / `TAG_TMDMODEL` (0xF, 9) -> basic_class.h's `CLASS_ID_ROOT_MASK` and a new `TMDMODEL_CLASS_ID` in include/tmd_model.h (gTmdModelMethods word +0x000 is 0x9, tools/classtable.py --scan; tier A). Byte-identical.
