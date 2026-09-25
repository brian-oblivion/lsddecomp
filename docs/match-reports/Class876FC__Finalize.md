# Class876FC__Finalize -- MATCHED (16/16 words)

> Renamed from `func_80056464` on 2026-09-23 (tools/rename.py). Address 0x80056464.

Unit: `class_3bb8c_r` (round 17 continuation). `Class876FCMethods::dtor`
(vtable offset `+0x00C` of `gClass876FCMethods`) -- calls a teardown helper, then
tail-calls the shared base class's own dtor (`GetActorMethods()->dtor`) and
forwards its return.

## Final source

```c
void *Class876FC__Finalize(Class876FC *self) {
    Class876FC__ReleaseByKind(self);
    return GetActorMethods()->dtor(self);
}
```

## Derivation

`Class876FC__ReleaseByKind` (a plain teardown helper, still `INCLUDE_ASM` outside
this unit's range, declared as a local extern) is called first as a
statement, THEN `GetActorMethods()->dtor(self)`'s return value genuinely IS
forwarded here (unlike `Class876FC__Class876FC`'s tail call to `Class876FC__InitByKind` --
confirmed by the disassembly falling straight through the epilogue with
`$v0` untouched after the `jalr`, no extra `move` the way
`Class876FC__Class876FC` had). `FixedBaseTableR::dtor` needed a non-void, checkable
return type in this unit's own local reading (a fresh `void *` field
alongside `ctor`) purely because THIS call site's return value is used;
per the shared getter's already-established per-call-site-typing
precedent (`class_3bb8c_o.c`, and `class_3bb8c.h`/`code_d294.h`'s own
notes on the sibling symbol `GetClass6B5CCMethods`).

### Proposed learning

None -- confirms the tail-call-forwarding case that `Class876FC__Class876FC`'s
report explicitly contrasts against.

## Naming

**Tier A.** `+0x00C` is the "finalize" slot convention this project already uses (`Class6B5CC__Finalize`, `Class866E8__Finalize`, `Viewport__Finalize`), and the body matches: teardown helper then forward the shared base dtor's return.

## Track 4 (2026-09-25, round 82, delta)

The base class is unified as `Actor` (`include/Actor.h`). Its +0x00C is the inherited Class6B5CC `finalize`, whose occupant Class6B5CC__Finalize returns nothing, so this function no longer returns the base call's value: it is `void`, and the base call a plain statement (`GetActorMethods()->finalize((Actor *)self)`). Same bytes: nothing touches $v0 after the jalr either way. Class876FC's own table view (`Class876FCMethods` in class_3bb8c_r.c) still types +0x00C as returning `void *`; that is the subclass's to settle.
