> Renamed from `func_80057C84` on 2026-09-19 (tools/rename.py). Address 0x80057c84.

# DreamSys__GetBaseMethods -- MATCHED (4/4)

Unit: `src/class_3bb8c_p.c`. Class: `DreamSys` family -- plain getter, not
a vtable slot itself (checked all 6 method tables reachable from this
unit's addresses, no hit; called directly by symbol name from
`New_D800879C4`, `D800879C4__D800879C4`, and this unit's own already-typed
`extern DreamSysBaseMethods *DreamSys__GetBaseMethods(void);` declaration in
`include/DreamSys.h`, added by an earlier round before this unit existed).

## Body

```c
extern DreamSysBaseMethods D_800878D4;

DreamSysBaseMethods *DreamSys__GetBaseMethods(void) {
    return &D_800878D4;
}
```

A plain no-argument getter for the shared intermediate base-class table
`D_800878D4` -- whole body is `lui`/`addiu`, no `%gp_rel`. Declares this
unit's own `extern DreamSysBaseMethods D_800878D4;` locally (not in the
shared header) since `include/code_55dd4.h` already carries an
INDEPENDENT typed view of the same table (`D800878D4Methods`) for a
different unit, per the project's multiple-independent-local-views
convention.

## Naming

**`DreamSys__GetBaseMethods` -- tier A.** A pure no-argument getter
(`return &D_800878D4;`) for the shared intermediate base-class table --
mechanics ARE the purpose, matching the project's own convention for
such getters (compare `func_80057F58`/`func_800422BC`, still `func_`
because they live in uncarved ground this runner cannot touch).

## Verify

```
./build-and-verify.sh   # build exit=0, OK: build matches retail
tools/funcdiff.py DreamSys__GetBaseMethods   # 4/4
```
