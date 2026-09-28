# GetDayTaskMethods

> Renamed from `GetClass865C8Methods` on 2026-09-26 (tools/rename.py). Address 0x8004a060.

> Renamed from `GetObj865C8Methods` on 2026-09-26 (tools/rename.py). Address 0x8004a060.

> Renamed from `func_8004A060` on 2026-09-23 (tools/rename.py). Address 0x8004a060.

**Unit:** DayTaskStageMap · **Size:** 4 words (0x10 bytes) · **Status:** MATCHED (4/4 words)

## What it does

The `Get_vtable_X`-shaped accessor for the class implemented by most of this
unit's remaining functions: returns `&gDayTaskMethods`, a 33-slot method table
(`tools/classtable.py 0x800865C8`). No parameters, matching the
`GetDreamSysMethods` / `GetGameApplicationMethods` shape from
`docs/research/class-framework.md`.

## Derivation

```
lui   $v0, %hi(gDayTaskMethods)
addiu $v0, $v0, %lo(gDayTaskMethods)
jr    $ra
 nop
```

Written as:

```c
DayTaskMethods *GetDayTaskMethods(void) {
    return &gDayTaskMethods;
}
```

`DayTaskMethods` and `gDayTaskMethods`'s extern declaration are established in
`include/DayTaskStageMap.h`, added this round. The table's DATA itself is still
raw (`asm/data/76DC8.data.s`) -- out of this round's scope; only the pointer
type needed for callers is declared.

## Proposed learning

None beyond what's already documented.

## Naming

`GetDayTaskMethods` -- tier A. Plain accessor, `return &gDayTaskMethods;` -- matches the established `GetXMethods`/`Get_vtable_X` accessor convention used site-wide for vtable getters (e.g. `GetTimedTaskMethods`, `Get_vtable_IntermediateBase`).

## Track 4 (2026-09-26, round 88, DayTask)

The class (table D_800865C8, id 0x1F230, TimedTask's subclass) is unified as DayTask in include/DayTask.h; the Obj865C8/DayTaskMethods views in DayTaskStageMap.h are gone. Renamed from GetObj865C8Methods with the class (returns &gDayTaskMethods, was D_800865C8). Tier A.
