# StyleEffect__Finalize -- MATCHED (16/16 words)

> Renamed from `Class876FC__Finalize` on 2026-09-26 (tools/rename.py). Address 0x80056464.

> Renamed from `func_80056464` on 2026-09-23 (tools/rename.py). Address 0x80056464.

Unit: `ObjMStyleActor` (round 17 continuation). `StyleEffectMethods::dtor`
(vtable offset `+0x00C` of `gStyleEffectMethods`) -- calls a teardown helper, then
tail-calls the shared base class's own dtor (`GetActorMethods()->dtor`) and
forwards its return.

## Final source

```c
void *StyleEffect__Finalize(StyleEffect *self) {
    StyleEffect__ReleaseByKind(self);
    return GetActorMethods()->dtor(self);
}
```

## Derivation

`StyleEffect__ReleaseByKind` (a plain teardown helper, still `INCLUDE_ASM` outside
this unit's range, declared as a local extern) is called first as a
statement, THEN `GetActorMethods()->dtor(self)`'s return value genuinely IS
forwarded here (unlike `StyleEffect__StyleEffect`'s tail call to `StyleEffect__InitByKind` --
confirmed by the disassembly falling straight through the epilogue with
`$v0` untouched after the `jalr`, no extra `move` the way
`StyleEffect__StyleEffect` had). `FixedBaseTableR::dtor` needed a non-void, checkable
return type in this unit's own local reading (a fresh `void *` field
alongside `ctor`) purely because THIS call site's return value is used;
per the shared getter's already-established per-call-site-typing
precedent (`ObjMStyleActor.c`, and `class_3bb8c.h`/`SceneNode.h`'s own
notes on the sibling symbol `GetSceneNodeMethods`).

### Proposed learning

None -- confirms the tail-call-forwarding case that `StyleEffect__StyleEffect`'s
report explicitly contrasts against.

## Naming

**Tier A.** `+0x00C` is the "finalize" slot convention this project already uses (`SceneNode__Finalize`, `StageMap__Finalize`, `Viewport__Finalize`), and the body matches: teardown helper then forward the shared base dtor's return.

## Track 4 (2026-09-25, round 82, delta)

The base class is unified as `Actor` (`include/Actor.h`). Its +0x00C is the inherited SceneNode `finalize`, whose occupant SceneNode__Finalize returns nothing, so this function no longer returns the base call's value: it is `void`, and the base call a plain statement (`GetActorMethods()->finalize((Actor *)self)`). Same bytes: nothing touches $v0 after the jalr either way. StyleEffect's own table view (`StyleEffectMethods` in ObjMStyleActor.c) still types +0x00C as returning `void *`; that is the subclass's to settle.

## Track 4 (2026-09-26, round 88, charlie)

The class's view is now include/StyleEffect.h (round 88); body unchanged, image byte-identical.

## Track 7 (2026-09-27, round 96, charlie)

The source comment no longer carries the old view's history; what it said:
"The base finalize is SceneNode__Finalize, which returns nothing: the old
view's `return base->dtor(self)` forwarded a $v0 no one sets." Zero bytes.
