# Obj86B60__ResetAndRemoveAllChildren — MATCH (17/17 words)

> Renamed from `func_8003E874` on 2026-09-19 (tools/rename.py). Address 0x8003e874.

**Unit:** code_2cc8c_c · **Size:** 17 instructions

## What it does

A ctor-shaped function: zeroes three `Obj86B60` fields (`unk30`, `unk10`,
`unkC` -- all new, only observed here and by `Obj86B60__NotifyTargetReset`, which
dereferences `unkC`), then forwards unconditionally to the shared
`BasicClass` ancestor's own `+0x018` slot, `Get_vtable_BasicClass()->slot18(self)`
-- the same no-argument-getter idiom already established independently in
`include/class_16334.h`, `include/code_171e0.h` and `include/code_d294.h`.

## The C

```c
void Obj86B60__ResetAndRemoveAllChildren(Obj86B60 *self)
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
  `include/code_2cc8c.h`.
- This unit's own local view of the shared `BasicClass` ancestor table
  (`BasicClassMethodsCC8C`, only `slot18` typed) and its getter
  `Get_vtable_BasicClass(void)`.

## Provenance

round 12 (2026-09-03), runner alpha, unit code_2cc8c_c. Matched on the
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
