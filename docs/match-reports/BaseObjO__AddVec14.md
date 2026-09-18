> Renamed from `func_800573A8` on 2026-09-18 (tools/rename.py). Address 0x800573a8.

# BaseObjO__AddVec14 -- MATCHED (9/9 words)

Unit: `class_3bb8c_o` (round 17). One-line wrapper: `BaseObjO__UpdateVec14(self, 0,
arg1)`. Already named at `vtable_DreamSys` `+0x0BC` in `DreamSys.h`, typed
`void (*BaseObjO__AddVec14)(DreamSys *this, DreamSysVec3 *arg1)` there.

## Final source

```c
void BaseObjO__AddVec14(BaseObjO *self, Vec3O *arg1) {
    BaseObjO__UpdateVec14(self, 0, arg1);
}
```

## Derivation

Identical shape to `BaseObjO__SetVec14` (same report applies) except the flag
literal is `0` instead of `1` -- `BaseObjO__UpdateVec14`'s "accumulate" mode
instead of its "overwrite" mode. `Vec3O` here is this unit's own local
type, structurally identical to (but independently declared from)
`DreamSys.h`'s `DreamSysVec3` -- both are plain `{ s32 x, y, z; }`, per the
project's multiple-independent-local-views convention; this unit does not
include `DreamSys.h`.

### Proposed learning

None -- see `BaseObjO__UpdateVec14`'s report for the real work.
