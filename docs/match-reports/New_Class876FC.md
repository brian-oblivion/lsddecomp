# New_Class876FC -- MATCHED (40/40 words)

> Renamed from `func_80056320` on 2026-09-23 (tools/rename.py). Address 0x80056320.

Unit: `class_3bb8c_r` (round 17 continuation). `New_X` allocator (0x98
bytes) for the `gClass876FCMethods` class, dispatching through
`func_80056F4C()->ctor` -- a CROSS-UNIT call into the already-matched
`class_3bb8c_o.c` (previous pass, same round) rather than calling
`Class876FC__Class876FC` (this unit's own ctor) by name.

## Final source

```c
void *New_Class876FC(void *arg0, void *arg1, void *arg2, void *arg3) {
    Obj876FC *self = func_80017B34(0x98);

    if (self != NULL) {
        if (func_80056F4C()->ctor(self, arg0, arg1, arg2, arg3) != NULL) {
            return self;
        }
        func_80017CFC(self);
        return NULL;
    }
    return NULL;
}
```

## Derivation

The `New_X` allocator sub-shape #3 from `DECOMPILATION_LEARNINGS.md`
("tests BOTH the allocation and the constructor's return, freeing on
constructor failure -- plain `if`/`return`"), same shape as
`class_3bb8c_o.c`'s own `New_BaseObjO` from the previous pass. The
constructor is reached THROUGH THE VTABLE (`func_80056F4C()->ctor(...)`,
where `func_80056F4C` is `class_3bb8c_o.c`'s already-matched getter for
`&gClass876FCMethods`) rather than by a direct `jal` to `Class876FC__Class876FC` -- both
resolve to the same function at runtime, but the disassembly's own
`jal func_80056F4C` / `lw v0,8(v0)` / `jalr v0` sequence requires the
vtable form, not a direct call. `Obj876FCMethods::ctor` (declared in this
unit, see `Class876FC__Class876FC`'s report) is 5-argument (`self` + 4 forwarded
parameters), matching this allocator's own 4 incoming parameters plus
`self`.

### Proposed learning

None -- confirms the `New_X` sub-shape #3 pattern established last pass,
this time reached through a vtable dispatch rather than a bare `jal`.
