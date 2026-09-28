# GetStyleEffectMethods -- MATCHED (4/4 words)

> Renamed from `GetClass876FCMethods` on 2026-09-26 (tools/rename.py). Address 0x80056f4c.

> Renamed from `func_80056F4C` on 2026-09-23 (tools/rename.py). Address 0x80056f4c.

Unit: `class_3bb8c_k` (round 17). A sibling class's own method-table
getter, analogous to `GetActorMethods`/`GetTodActorMethods` already documented in
`code_55dd4.h`.

## Final source

```c
extern BaseObjOMethods gStyleEffectMethods;

BaseObjOMethods *GetStyleEffectMethods(void) {
    return &gStyleEffectMethods;
}
```

## Derivation

Whole body is `lui $v0,%hi(gStyleEffectMethods)` / `addiu $v0,$v0,%lo(gStyleEffectMethods)` /
`jr $ra` -- a bare address-of, no dereference, matching CLAUDE.md's
"`lui`/`addiu` to a symbol with no surrounding `lw`/`sw` returns `&symbol`"
idiom.

`gStyleEffectMethods` is typed `BaseObjOMethods` (this unit's own view of the shared
intermediate base class -- see the file banner and `Actor__Actor`'s
report) because `tools/classtable.py gStyleEffectMethods` shows it sharing exactly
this class's `+0x010`/`+0x014`/`+0x018`/`+0x088`/`+0x09C`/`+0x0B8`/`+0x0BC`/
`+0x0C0`/`+0x0C4` slots with the functions this unit defines
(`Actor__AddChild`, `Actor__RemoveChild`, `Actor__RemoveAllChildren`, `Actor__NotifyMove`,
`Actor__DispatchLinkCommand`, `Actor__SetTranslation`, `Actor__AddTranslation`, `Actor__AddLocalTranslation`,
`Actor__MoveLocalZ`) -- i.e. `gStyleEffectMethods` is a SIBLING concrete class built on
the same intermediate base, the same relationship `gTodActorMethods`
(TodActor) and `gDreamSysMethods` (DreamSys) already have to it.

### Proposed learning

None -- see `Actor__Actor`'s report for the class-identification
reasoning this getter's typing depends on.

## Naming

**`GetStyleEffectMethods` -- NOT renamed, tier C.** A pure singleton vtable getter
(`return &gStyleEffectMethods;`), exactly the same shape as this codebase's other
unnamed getters this unit calls but does not define --
`GetSceneNodeMethods`/`GetActorMethods`/`GetTodActorMethods` -- none of which carry a
name either. The getter's own body establishes nothing about the identity
of `gStyleEffectMethods`'s class beyond "it is a sibling of `TodActor`/`BaseObjO`
built on the same base" (see the file header comment); naming the FUNCTION
would require first naming that sibling class, which is out of scope for
this unit (`gStyleEffectMethods`'s own ctor, `StyleEffect__StyleEffect`, is defined elsewhere).
Left as `func_` for consistency with the rest of this getter family.

## Track 4 (2026-09-26, round 88, charlie)

StyleEffect unified (include/StyleEffect.h): the `LinkOwnerObj`/`LinkElemObj` views in class_3bb8c_k.c are deleted and the unit includes include/StyleEffect.h (the getter now returns `StyleEffectMethods *`; it was typed as its parent's `ActorMethods`). Image byte-identical.

## Track 7 (round 99, alpha)

The comment that it sits among Actor's methods by address moved into the unit banner.
