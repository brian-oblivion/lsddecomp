# BaseObjO__SetVec14 -- MATCHED (9/9 words)

> Renamed from `func_80057384` on 2026-09-18 (tools/rename.py). Address 0x80057384.

Unit: `class_3bb8c_o` (round 17). One-line wrapper: `BaseObjO__UpdateVec14(self, 1,
arg1)`. Already named `BaseObjO__SetVec14` at `vtable_DreamSys` `+0x0B8` in
`DreamSys.h`, typed `void (*BaseObjO__SetVec14)(DreamSys *this, void *arg1)`
there (a looser reading than this unit's own, since DreamSys.h only needed
the call site's own arity, not this function's real body).

## Final source

```c
void BaseObjO__SetVec14(BaseObjO *self, Vec3O *arg1) {
    BaseObjO__UpdateVec14(self, 1, arg1);
}
```

## Derivation

`addu $a2,$a1,zero` (moves the incoming `arg1` into `$a2`, `BaseObjO__UpdateVec14`'s
3rd parameter slot) then `jal BaseObjO__UpdateVec14` with `$a1 = 1` set in the
delay slot -- a plain forward with a literal flag. Matched first try once
`BaseObjO__UpdateVec14` itself (called forward, still `INCLUDE_ASM` at the time
this was written -- see that function's own report) was declared locally.
`arg1`'s type (`Vec3O *`) comes from `BaseObjO__UpdateVec14`'s own body (a 3-word
vector, block-assigned or accumulated into `self->unk14->vec18`).

### Proposed learning

None -- see `BaseObjO__UpdateVec14`'s report for the real work.

## Naming

**`BaseObjO__SetVec14` -- tier A.** One-line forward to
`BaseObjO__UpdateVec14(self, 1, arg1)` -- the flag literal `1` selects
`UpdateVec14`'s overwrite (`vec18 = *v`) branch, so "Set" is exactly what
this wrapper causes to happen. `Vec14` names the field it writes
(`self->vecTarget->vec18`, at `Unk14ObjO`'s `+0x018`, reached through
`BaseObjO`'s own `+0x014` `vecTarget` pointer).
