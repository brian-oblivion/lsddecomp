# Viewport__RemoveAllChildren — MATCH (17/17 words)

> Renamed from `Obj86B60__ResetAndRemoveAllChildren` on 2026-09-25 (tools/rename.py). Address 0x8003e874.

> Renamed from `func_8003E874` on 2026-09-19 (tools/rename.py). Address 0x8003e874.

**Unit:** Task · **Size:** 17 instructions

## What it does

A ctor-shaped function: zeroes three `Obj86B60` fields (`unk30`, `unk10`,
`unkC` -- all new, only observed here and by `IntermediateBase__OnState2`, which
dereferences `unkC`), then forwards unconditionally to the shared
`BasicClass` ancestor's own `+0x018` slot, `Get_vtable_BasicClass()->slot18(self)`
-- the same no-argument-getter idiom already established independently in
`include/class_16334.h`, `include/code_171e0.h` and `include/code_d294.h`.

## The C

```c
void Viewport__RemoveAllChildren(Obj86B60 *self)
{
    self->unk30 = 0;
    self->unk10 = 0;
    self->unkC = NULL;
    Get_vtable_BasicClass()->slot18(self);
}
```

## Struct knowledge established

- `Obj86B60::unkC` (+0x00C, pointer to `Obj86B60UnkC`), `unk10` (+0x010,
  `s32`, meaning unknown beyond "zeroed here"), `unk30` (+0x030, `s32`,
  same) -- all new fields carved out of previously-opaque padding in
  `include/Task.h`.
- This unit's own local view of the shared `BasicClass` ancestor table
  (`BasicClassMethodsCC8C`, only `slot18` typed) and its getter
  `Get_vtable_BasicClass(void)`.

## Provenance

round 12 (2026-09-03), runner alpha, unit Task. Matched on the
first build.

## Naming

**Obj86B60__ResetAndRemoveAllChildren** (renamed from `func_8003E874`,
round 55, runner alpha). Tier A: mechanics fully known -- zeroes
`self->unk30`, `self->unk10` and `self->initArgs`, then forwards to
`Get_vtable_BasicClass()->removeAllChildren` (that base slot's own name,
`+0x018`, matches the canonical `BasicClassMethods::removeAllChildren` at
the identical offset in `include/code_8220.h`). Named for the whole visible
effect (clear the object's own cached pointers, then remove every child)
rather than asserting it is specifically a "finalize" override, since its
own occupant slot on `Obj86B60Methods` is not otherwise identified.

## Track 4 (2026-09-25, round 84, alpha)

NOT TaskCore's: it occupies +0x018 (removeAllChildren) of gViewportMethods (Unk18Obj) and of gNodeGuardedViewportMethods, and never of gTaskCoreMethods, and its three fields are Unk18Obj's child caches (unkC/unk10/unk30, set by Viewport__AddChild). `self` is now `Unk18Obj *` (the Obj86B60 view is gone); byte-identical. The name is left for Unk18Obj's own track-4 job.

## Track 4 (2026-09-25, round 85, bravo)

Renamed from `Obj86B60__ResetAndRemoveAllChildren`. The +0x018 removeAllChildren override (also NodeGuardedViewport's). Its old name was given when TaskCore was still viewed as `Obj86B60`, the table it was thought to belong to; it clears the same three child caches as AddChild/RemoveChild do, so it is named for its slot like them. The class (id 0x7, table `gViewportMethods`, formerly `D_8006E8E4`) is unified as `Viewport` in `include/Viewport.h`, whose banner gives the evidence for the name: its methods hold a GsRVIEW2 (GsSetRefView2), the projection and near clip, a double-buffered GsOT pair, draw the scene tree into it and flip it; IntermediateBase and TaskCore already called the field holding it `viewport`. Any source block above is the pre-unification spelling; the live body takes the unified types and field and slot names, byte-identical.

## Track 7 (round 98, echo)

Pointer stores of 0 spelled NULL. Byte-identical.
