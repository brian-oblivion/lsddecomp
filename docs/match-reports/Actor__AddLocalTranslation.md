# Actor__AddLocalTranslation -- MATCHED (18/18 words)

> Renamed from `BaseObjO__ApplyRotatedVec14` on 2026-09-25 (tools/rename.py). Address 0x80057444.

> Renamed from `func_80057444` on 2026-09-18 (tools/rename.py). Address 0x80057444.

Unit: `class_3bb8c_k` (round 17). Converts a source `s16` triple into a
stack `Vec3O` via `SceneNode__RotateLocalVector`, then forwards it to `self`'s own
`slotBC` (= `Actor__AddTranslation`, this unit) through the vtable.

## Final source

```c
extern void SceneNode__RotateLocalVector(BaseObjO *self, Vec3O *dst, s16 *src);

void Actor__AddLocalTranslation(BaseObjO *self, s16 *arg1) {
    Vec3O buf;

    SceneNode__RotateLocalVector(self, &buf, arg1);
    self->methods->slotBC(self, &buf);
}
```

## Derivation

`SceneNode__RotateLocalVector` is called with `$a0` = this function's own `self`
(untouched since entry), `$a1` = the local buffer's address (freshly set
in the `jal`'s own delay slot, overwriting whatever `$a1` held), and `$a2`
= this function's own SECOND parameter (copied into `$a2` BEFORE the call,
via `addu $a2,$a1,zero` -- note this reads the OLD `$a1`, i.e. `arg1`,
ahead of the delay slot's overwrite). This establishes `arg1`'s type as
`s16 *`, matching `SceneNode__RotateLocalVector`'s own already-decompiled signature
elsewhere (`code_d294_c.c`:
`void SceneNode__RotateLocalVector(SceneNodeObj *self, SceneNodeSub44 *dst, s16 *src)`)
-- declared locally here with this unit's own generic types rather than
pulling in `code_d294.h`'s `SceneNodeObj`/`SceneNodeSub44`, per the
project's per-call-site-typing convention for cross-unit calls.

The second call, `self->methods->slotBC(self, &buf)`, dispatches through
`self`'s OWN vtable (not a fixed/global table) at `+0xBC` -- the exact slot
`Actor__AddTranslation` (this unit) already occupies per `DreamSys.h`'s
`vtable_DreamSys::Actor__AddTranslation`. This is a self-referential virtual call
(the class calling its own overridable slot rather than jumping to
`Actor__AddTranslation` by name), matched by adding `slotBC` to `BaseObjOMethods`
and calling through it rather than `jal`-ing the symbol directly.

### Proposed learning

None -- straightforward once `SceneNode__RotateLocalVector`'s established signature and
`BaseObjOMethods`'s `slotBC` slot were both in place.

## Naming

**`Actor__AddLocalTranslation` -- tier A.** Mechanics ARE the purpose:
rotates a source `s16` triple through the already-established
`SceneNode__RotateLocalVector` into a stack `Vec3O`, then forwards that
buffer to `self`'s own `addVec14` slot (`Actor__AddTranslation`, a
self-referential virtual dispatch to a function this same unit defines).
"Apply a rotated vector" is exactly the composition of "rotate" then
"add".

## Track 4 (2026-09-25, round 82, delta)

Renamed from `BaseObjO__ApplyRotatedVec14`. Occupant of +0x0C0: SceneNode__RotateLocalVector rotates the s16 local vector by the object's orientation, then addTranslation (+0x0BC) adds the result. The class (id 0x34, table `gActorMethods`, formerly `D_800878D4`) is unified as `Actor` in `include/Actor.h`: a SceneNode subclass and the base of TodActor/Entity, DreamSys and StyleEffect. Any source block above is the pre-unification spelling; the live body in `src/class_3bb8c_k.c` takes the unified types and field/slot names, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 7 (round 99, alpha)

Local buf -> delta; a one-line comment.
