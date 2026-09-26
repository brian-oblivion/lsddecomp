# AttachWithRotScale -- MATCHED (33/33 words)

> Renamed from `func_800567D4` on 2026-09-23 (tools/rename.py). Address 0x800567d4.

Unit `class_3bb8c_s`. `self` here is a `LinkNode` in the CALLEE role (a
child, e.g. `self->arr7C[i]`), not the owning node -- see
`StyleEffect__InitByKind`/`StyleEffect__PlaceModelChildren`'s call sites, which both pass one of the
owner's own child pointers as `self` here.

## Classification

Clean on all four carve-time screens. Reads as a pure forwarder: it hands its
own `arg1`/`arg2` straight to `self->methods->slot4C` UNCHANGED (they are
still sitting in the registers the caller put them in, which is why the
disassembly shows no reload for them before that first call), then makes two
more calls with a literal flag `1` and its own `arg3`/`arg4`.

## Body

```c
void AttachWithRotScale(LinkNode *self, void *arg1, void *arg2, s32 arg3, void *arg4) {
    self->methods->slot4C(self, arg1, arg2);
    self->methods->slot44(self, 1, arg3);
    self->methods->slot48(self, 1, arg4);
}
```

## Notes

`arg3`/`arg4` are preserved across the first call in `$s1`/`$s2` (delay-slot
saves, standard caller-saved-register preservation across a `jalr`) -- this
does NOT mean `slot4C` takes 4 arguments; the save happens regardless of
whether the callee reads `$a3`, because the C variable holding `arg3` is live
past the call either way. `slot4C` is modeled here as taking only
`(self, arg1, arg2)`, which is enough to reproduce retail exactly; there is
no evidence either way for a 4th parameter from this call site alone.

### Proposed learning

Confirms (again) that "a register is preserved across a call" is NOT evidence
that the callee reads it -- see the `StyleEffect__SpawnSprites` read-only cross-check
(still `INCLUDE_ASM`/`gp_rel`-blocked in this same unit), which calls the same
kind of `slot4C` with a completely different, and clearly unrelated, `$a3`
value sitting stale in the register from far earlier in that function.

## Naming

Round 70 (alpha). `func_800567D4` -> `AttachWithRotScale`, **tier A**.

Pure forwarder: slot +0x04C (SceneNode__AttachToParent: sets the parent link and
coord2 `super`, writes `trans` into coord.t), then slots +0x044 and +0x048
with set = 1 (SceneNode__UpdateRotation assigns GsCOORD2PARAM.rotate from a degree ratio
triple; SceneNode__UpdateScale assigns .scale). Both callers
(StyleEffect__InitByKind on the owner, StyleEffect__PlaceModelChildren on
each BaseObjO child) pass objects whose tables resolve those three slots to
exactly those functions (tools/classtable.py on gStyleEffectMethods and gActorMethods).
Free function because `node` is not always the owner.

## Track 4 (2026-09-26, round 88, charlie)

Retyped with StyleEffect's unification: `Vec3S` is `LongVec3`; `node` is `Actor *` (the owner, upcast, and each model child) and `rotation` is `void *`, the type of SceneNode's updateRotation table. Image byte-identical.
