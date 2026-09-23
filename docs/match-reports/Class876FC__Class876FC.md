# Class876FC__Class876FC -- MATCHED (41/41 words)

> Renamed from `func_800563C0` on 2026-09-23 (tools/rename.py). Address 0x800563c0.

Unit: `class_3bb8c_r` (round 17 continuation). `Obj876FCMethods::ctor`
(vtable offset `+0x008` of `gClass876FCMethods`) -- chains to the shared base
class's own ctor (`DreamSys__GetBaseMethods()->ctor`, the SAME shared-base getter
`class_3bb8c_o.c` already used for its own `BaseObjO__BaseObjO`/
`New_BaseObjO` last pass), installs this class's own vtable, sets two
fields, dispatches its own `slot40`, and tail-calls `Class876FC__InitByKind` for
its return value's side effect only.

## Final source

```c
void *Class876FC__Class876FC(Obj876FC *self, void *arg1, void *arg2, void *arg3, void *arg4) {
    if (DreamSys__GetBaseMethods()->ctor(self) == NULL) {
        goto fail;
    }
    self->methods = func_80056F4C();
    self->unk44 = 0;
    self->unk54 = arg1;
    self->methods->slot40(self, arg2);
    Class876FC__InitByKind(self, arg3, arg4);
    return self;
fail:
    return NULL;
}
```

## Derivation

- **`DreamSys__GetBaseMethods()`, not `GetClass6B5CCMethods()`.** Both are fixed-table
  getters `class_3bb8c_o.c` already resolved last pass for the SAME
  shared intermediate base class, but they are DIFFERENT symbols with
  DIFFERENT call sites in that unit (`GetClass6B5CCMethods` for the ctor CHAIN
  inside `BaseObjO__BaseObjO`; `DreamSys__GetBaseMethods` for the plain-allocator
  `New_BaseObjO`'s own ctor dispatch). This function's own disassembly
  calls `DreamSys__GetBaseMethods`, confirmed directly rather than assumed from
  surface similarity to last pass's ctor.
- **`goto fail; ... fail: return NULL;`, not `if (cond) return NULL;`.**
  Same lever as `class_3bb8c_o.c`'s own `BaseObjO__BaseObjO` (documented
  there): with a plain `if`/`return NULL`, the return-`self` path needs
  its own explicit `j` to reach the shared epilogue, costing one word.
  `goto` collapses both exits onto ONE epilogue.
- **The tail call to `Class876FC__InitByKind`'s return value is DISCARDED, not
  forwarded.** The first attempt wrote `return Class876FC__InitByKind(self, arg3,
  arg4);`, which is the "byte match tells you nothing about return type"
  trap's INVERSE mistake -- it assumed the tail call's own return
  propagates, but retail actually issues the call, then explicitly does
  `move v0,s0` (an extra instruction) to force the return value back to
  `self` regardless of what `Class876FC__InitByKind` returned. Caught immediately
  by a real word-count mismatch (28/41 with `return Class876FC__InitByKind(...)`,
  worse yet with the wrong exit-value idiom on top) -- the fix was two
  statements, `Class876FC__InitByKind(self, arg3, arg4); return self;`, not one.
- `func_80056F4C` (installing this class's own vtable) and
  `Class876FC__InitByKind` (this class's own post-init hook, still `INCLUDE_ASM`
  outside this unit's range) are both declared as local externs, per the
  established "calling into a function in another/uncarved unit is fine"
  convention.

### Proposed learning

- **A tail call whose value is visibly DISCARDED (an explicit `move
  v0,<other>` after the call, before the shared epilogue) is the mirror
  image of the "byte match tells you nothing about return type" trap.**
  Don't assume the LAST call in a function is a tail call whose return
  propagates just because nothing else follows it in program order --
  check whether retail's own bytes overwrite `$v0` again afterward. Here
  it cost one build/diff iteration to notice; a direct read of the
  disassembly's trailing instructions (past the `jal`) would have caught
  it without spending the attempt.
