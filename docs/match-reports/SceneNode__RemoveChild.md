# SceneNode__RemoveChild

> Renamed from `Class6B5CC__RemoveChild` on 2026-09-26 (tools/rename.py). Address 0x8001ccb4.

> Renamed from `func_8001CCB4` on 2026-09-23 (tools/rename.py). Address 0x8001ccb4.

**Unit:** code_d294 · **Size:** 27 words · **Status:** MATCHED (27/27 words)

## What it does

`SceneNode` vtable slot `+0x014`, mirror of `SceneNode__AddChild` (`+0x010`).
If `other`'s vtable header tag is `9`, first calls `SceneNode__UnlinkModel(self)`
(zeroes `self->unk18`/`self->unk20` -- MEASURED from its own disassembly,
see `include/code_d294.h`), THEN unconditionally forwards to the base
class's own `+0x014` slot (`Get_vtable_BasicClass()->slot14`). "Detach" to
`SceneNode__AddChild`'s "attach": the pre-work happens before the base call here,
where `SceneNode__AddChild` did its post-work after.

## The C

```c
void SceneNode__RemoveChild(SceneNodeObj *self, GenericObj_d294 *other) {
    if ((other->methods->header & 0xF) == 9) {
        SceneNode__UnlinkModel(self);
    }
    Get_vtable_BasicClass()->slot14(self, other);
}
```

## Note on a local that looks conditionally-set but isn't

Retail's own disassembly sets `s0 = a0` (self) in the delay slot of the
`bne` that decides whether to call `SceneNode__UnlinkModel` -- i.e. that move
executes on EVERY pass through this code, taken branch or not, because a
MIPS delay slot always executes. Reading it as "only set when the branch
is taken" would be the trap; it isn't, and the C above needs no defensive
restructuring to account for an uninitialized-looking path -- `self` is
simply always live in a register here.

## Provenance

round 11 (2026-09-03), runner charlie, unit code_d294, second pass. Matched on the first build.

## Naming

Round 71 (alpha). `func_8001CCB4` -> `SceneNode__RemoveChild`, **tier A**. Overrides BasicClass slot +0x014 `removeChild`. If the child's tag is 9, SceneNode__UnlinkModel first, then forwards to the base. Mirror of SceneNode__AddChild; class_3ac78.h already calls this address `removeChild`.

## Round 101 (delta): track 7

Step 4 (constants): `CLASS_TAG_MASK` / `TAG_TMDMODEL` -> `CLASS_ID_ROOT_MASK` / `TMDMODEL_CLASS_ID` (include/TmdModel.h, new), as in SceneNode__AddChild. Byte-identical.
