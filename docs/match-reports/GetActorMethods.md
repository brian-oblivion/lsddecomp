# GetActorMethods -- MATCHED (4/4)

> Renamed from `DreamSys__GetBaseMethods` on 2026-09-25 (tools/rename.py). Address 0x80057c84.

> Renamed from `func_80057C84` on 2026-09-19 (tools/rename.py). Address 0x80057c84.

Unit: `src/class_3bb8c_p.c`. Class: `DreamSys` family -- plain getter, not
a vtable slot itself (checked all 6 method tables reachable from this
unit's addresses, no hit; called directly by symbol name from
`New_VariantSprite`, `VariantSprite__VariantSprite`, and this unit's own already-typed
`extern DreamSysBaseMethods *GetActorMethods(void);` declaration in
`include/DreamSys.h`, added by an earlier round before this unit existed).

## Body

```c
extern DreamSysBaseMethods gActorMethods;

DreamSysBaseMethods *GetActorMethods(void) {
    return &gActorMethods;
}
```

A plain no-argument getter for the shared intermediate base-class table
`gActorMethods` -- whole body is `lui`/`addiu`, no `%gp_rel`. Declares this
unit's own `extern DreamSysBaseMethods gActorMethods;` locally (not in the
shared header) since `include/code_55dd4.h` already carries an
INDEPENDENT typed view of the same table (`D800878D4Methods`) for a
different unit, per the project's multiple-independent-local-views
convention.

## Naming

**`GetActorMethods` -- tier A.** A pure no-argument getter
(`return &gActorMethods;`) for the shared intermediate base-class table --
mechanics ARE the purpose, matching the project's own convention for
such getters (compare `GetVariantSpriteMethods`/`GetSpriteMethods`, still `func_`
because they live in uncarved ground this runner cannot touch).

## Verify

```
./build-and-verify.sh   # build exit=0, OK: build matches retail
tools/funcdiff.py GetActorMethods   # 4/4
```

## Track 4 (2026-09-25, round 82, delta)

Renamed from `DreamSys__GetBaseMethods`. Returns `&gActorMethods`: the class's own table getter (Get<Class>Methods, as GetSceneNodeMethods). The old name put it in DreamSys, a SUBCLASS; Class65650 and Class876FC call it for their base too. The class (id 0x34, table `gActorMethods`, formerly `D_800878D4`) is unified as `Actor` in `include/Actor.h`: a SceneNode subclass and the base of Class65650/Entity, DreamSys and Class876FC. Any source block above is the pre-unification spelling; the live body in `src/class_3bb8c_p.c` takes the unified types and field/slot names, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
