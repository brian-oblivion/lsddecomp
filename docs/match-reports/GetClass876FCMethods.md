# GetClass876FCMethods -- MATCHED (4/4 words)

> Renamed from `func_80056F4C` on 2026-09-23 (tools/rename.py). Address 0x80056f4c.

Unit: `class_3bb8c_o` (round 17). A sibling class's own method-table
getter, analogous to `GetActorMethods`/`Get_vtable_Class65650` already documented in
`code_55dd4.h`.

## Final source

```c
extern BaseObjOMethods gClass876FCMethods;

BaseObjOMethods *GetClass876FCMethods(void) {
    return &gClass876FCMethods;
}
```

## Derivation

Whole body is `lui $v0,%hi(gClass876FCMethods)` / `addiu $v0,$v0,%lo(gClass876FCMethods)` /
`jr $ra` -- a bare address-of, no dereference, matching CLAUDE.md's
"`lui`/`addiu` to a symbol with no surrounding `lw`/`sw` returns `&symbol`"
idiom.

`gClass876FCMethods` is typed `BaseObjOMethods` (this unit's own view of the shared
intermediate base class -- see the file banner and `Actor__Actor`'s
report) because `tools/classtable.py gClass876FCMethods` shows it sharing exactly
this class's `+0x010`/`+0x014`/`+0x018`/`+0x088`/`+0x09C`/`+0x0B8`/`+0x0BC`/
`+0x0C0`/`+0x0C4` slots with the functions this unit defines
(`Actor__AddChild`, `Actor__RemoveChild`, `Actor__RemoveAllChildren`, `Actor__NotifyMove`,
`Actor__DispatchLinkCommand`, `Actor__SetTranslation`, `Actor__AddTranslation`, `Actor__AddLocalTranslation`,
`Actor__MoveLocalZ`) -- i.e. `gClass876FCMethods` is a SIBLING concrete class built on
the same intermediate base, the same relationship `gClass65650Methods`
(Class65650) and `gDreamSysMethods` (DreamSys) already have to it.

### Proposed learning

None -- see `Actor__Actor`'s report for the class-identification
reasoning this getter's typing depends on.

## Naming

**`GetClass876FCMethods` -- NOT renamed, tier C.** A pure singleton vtable getter
(`return &gClass876FCMethods;`), exactly the same shape as this codebase's other
unnamed getters this unit calls but does not define --
`GetSceneNodeMethods`/`GetActorMethods`/`Get_vtable_Class65650` -- none of which carry a
name either. The getter's own body establishes nothing about the identity
of `gClass876FCMethods`'s class beyond "it is a sibling of `Class65650`/`BaseObjO`
built on the same base" (see the file header comment); naming the FUNCTION
would require first naming that sibling class, which is out of scope for
this unit (`gClass876FCMethods`'s own ctor, `Class876FC__Class876FC`, is defined elsewhere).
Left as `func_` for consistency with the rest of this getter family.

## Track 4 (2026-09-26, round 88, charlie)

Class876FC unified (include/Class876FC.h): the `LinkOwnerObj`/`LinkElemObj` views in class_3bb8c_o.c are deleted and the unit includes include/Class876FC.h (the getter now returns `Class876FCMethods *`; it was typed as its parent's `ActorMethods`). Image byte-identical.
