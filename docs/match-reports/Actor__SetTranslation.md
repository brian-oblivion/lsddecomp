# Actor__SetTranslation -- MATCHED (9/9 words)

> Renamed from `BaseObjO__SetVec14` on 2026-09-25 (tools/rename.py). Address 0x80057384.

> Renamed from `func_80057384` on 2026-09-18 (tools/rename.py). Address 0x80057384.

Unit: `class_3bb8c_o` (round 17). One-line wrapper: `Actor__UpdateTranslation(self, 1,
arg1)`. Already named `Actor__SetTranslation` at `vtable_DreamSys` `+0x0B8` in
`DreamSys.h`, typed `void (*Actor__SetTranslation)(DreamSys *this, void *arg1)`
there (a looser reading than this unit's own, since DreamSys.h only needed
the call site's own arity, not this function's real body).

## Final source

```c
void Actor__SetTranslation(BaseObjO *self, Vec3O *arg1) {
    Actor__UpdateTranslation(self, 1, arg1);
}
```

## Derivation

`addu $a2,$a1,zero` (moves the incoming `arg1` into `$a2`, `Actor__UpdateTranslation`'s
3rd parameter slot) then `jal Actor__UpdateTranslation` with `$a1 = 1` set in the
delay slot -- a plain forward with a literal flag. Matched first try once
`Actor__UpdateTranslation` itself (called forward, still `INCLUDE_ASM` at the time
this was written -- see that function's own report) was declared locally.
`arg1`'s type (`Vec3O *`) comes from `Actor__UpdateTranslation`'s own body (a 3-word
vector, block-assigned or accumulated into `self->unk14->vec18`).

### Proposed learning

None -- see `Actor__UpdateTranslation`'s report for the real work.

## Naming

**`Actor__SetTranslation` -- tier A.** One-line forward to
`Actor__UpdateTranslation(self, 1, arg1)` -- the flag literal `1` selects
`UpdateVec14`'s overwrite (`vec18 = *v`) branch, so "Set" is exactly what
this wrapper causes to happen. `Vec14` names the field it writes
(`self->vecTarget->vec18`, at `SplitCoord2O`'s `+0x018`, reached through
`BaseObjO`'s own `+0x014` `vecTarget` pointer).

## Track 4 (2026-09-25, round 82, delta)

Renamed from `BaseObjO__SetVec14`. Occupant of +0x0B8, the class's first own slot: Actor__UpdateTranslation(self, 1, v). The 'vec14' was coord2 (+0x014, the GsCOORDINATE2) and the vector at its +0x018 is coord.t, so this sets the translation (class_3bb8c_s already called the neighbouring slot addTranslation). The class (id 0x34, table `gActorMethods`, formerly `D_800878D4`) is unified as `Actor` in `include/Actor.h`: a SceneNode subclass and the base of TodActor/Entity, DreamSys and StyleEffect. Any source block above is the pre-unification spelling; the live body in `src/class_3bb8c_o.c` takes the unified types and field/slot names, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
