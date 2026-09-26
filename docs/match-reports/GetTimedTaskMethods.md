# GetTimedTaskMethods

> Renamed from `GetClass86668Methods` on 2026-09-26 (tools/rename.py). Address 0x8004a4b8.

> Renamed from `func_8004A4B8` on 2026-09-22 (tools/rename.py). Address 0x8004a4b8.

**Unit:** class_3ac78 · **Size:** 4 words · **Status:** MATCHED (4/4 words)

## What it does

Returns the address of `gTimedTaskMethods`, the 28-slot method table for
`TimedTask`. A "Get_vtable" accessor, same shape as `Get_vtable_DreamSys`.

## Derivation

```
lui   $v0, %hi(gTimedTaskMethods)
addiu $v0, $v0, %lo(gTimedTaskMethods)
jr    $ra
```

A leaf that materializes a data address and returns it — no `$gp`-relative
form is used here (this address apparently falls outside whatever range
`-G0`'s relocation picked for it, or the compiler chose `%hi`/`%lo` for
another reason; either way it isn't the gp-relative blocker since there's no
`lui`+`lw` pair reading through `$gp`, just address materialization).
`gTimedTaskMethods` resolved as a 28-slot method table via `tools/classtable.py
gTimedTaskMethods`; declared `extern TimedTaskMethods gTimedTaskMethods;` in
`include/class_3ac78.h` (the table's own data bytes remain unmatched/raw —
this function only takes its address).

## Proposed learning

None beyond what's already documented for `TimedTask`/`Class866E8` in
`TimedTask__PlaySound.md` and `New_Class866E8.md`.

## Naming

Round 67 (track 3, naming pass).

| symbol | name | tier | evidence |
| --- | --- | --- | --- |
| `func_8004A4B8` | `GetTimedTaskMethods` | A | The whole body materializes `&gTimedTaskMethods` and returns it. `tools/classtable.py D_80086668` resolves that address as a 28-slot method table (header 0x230). Same shape and same name form as the already-established `GetSceneNodeMethods` / `Get_vtable_BasicClass`. |
| `D_80086668` | `gTimedTaskMethods` | A | Classtable-verified method table; `gName` per the project's global convention, `Methods` per `gVabDriverMethods` / `gTaskCoreMethods`. The class itself keeps its address-derived placeholder name `TimedTask` (tier C) -- nothing establishes what that class is. |

## Track 4

2026-09-25, round 84 (bravo): class unified in `include/TimedTask.h`. Not renamed. Declared once, in include/TimedTask.h, returning `TimedTaskMethods *`; the local prototypes in class_39e08.h and class_3bb8c_l.c are gone. Image byte-identical.
