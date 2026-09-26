# Actor__AddTranslation -- MATCHED (9/9 words)

> Renamed from `BaseObjO__AddVec14` on 2026-09-25 (tools/rename.py). Address 0x800573a8.

> Renamed from `func_800573A8` on 2026-09-18 (tools/rename.py). Address 0x800573a8.

Unit: `class_3bb8c_o` (round 17). One-line wrapper: `Actor__UpdateTranslation(self, 0,
arg1)`. Already named at `vtable_DreamSys` `+0x0BC` in `DreamSys.h`, typed
`void (*Actor__AddTranslation)(DreamSys *this, DreamSysVec3 *arg1)` there.

## Final source

```c
void Actor__AddTranslation(BaseObjO *self, Vec3O *arg1) {
    Actor__UpdateTranslation(self, 0, arg1);
}
```

## Derivation

Identical shape to `Actor__SetTranslation` (same report applies) except the flag
literal is `0` instead of `1` -- `Actor__UpdateTranslation`'s "accumulate" mode
instead of its "overwrite" mode. `Vec3O` here is this unit's own local
type, structurally identical to (but independently declared from)
`DreamSys.h`'s `DreamSysVec3` -- both are plain `{ s32 x, y, z; }`, per the
project's multiple-independent-local-views convention; this unit does not
include `DreamSys.h`.

### Proposed learning

None -- see `Actor__UpdateTranslation`'s report for the real work.

## Naming

**`Actor__AddTranslation` -- tier A.** One-line forward to
`Actor__UpdateTranslation(self, 0, arg1)` -- the flag literal `0` selects the
accumulate (`vec18.x += v->x` etc.) branch, so "Add" is exactly what this
wrapper causes to happen; mirrors `Actor__SetTranslation`'s naming logic.

## Track 4 (2026-09-25, round 82, delta)

Renamed from `BaseObjO__AddVec14`. Occupant of +0x0BC: Actor__UpdateTranslation(self, 0, v), i.e. coord2->coord.t += v. The class (id 0x34, table `gActorMethods`, formerly `D_800878D4`) is unified as `Actor` in `include/Actor.h`: a SceneNode subclass and the base of TodActor/Entity, DreamSys and Class876FC. Any source block above is the pre-unification spelling; the live body in `src/class_3bb8c_o.c` takes the unified types and field/slot names, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
