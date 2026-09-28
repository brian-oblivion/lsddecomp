# ObjM__NoOpSlot40

**Unit:** dream_scene · **Status:** MATCHED (splat-generated, `jr $ra; nop`)

## What it does

```c
void ObjM__NoOpSlot40(void) {
}
```

An empty body -- splat generated this stub itself (`jr $ra; nop`), not
work done in this round. It fills vtable slot `+0x040` of `gObjMMethods`
(`tools/classtable.py 0x80087034`), the class whose constructor
(`ObjM__ObjM`, slot `+0x008`) and destructor (`ObjM__Finalize`, slot `+0x00C`)
confirm the table is `ObjM`'s own, the same class as sibling unit
dream_scene's `ObjM`.

## Naming

Round 78 (charlie), FINISHING-PLAN track 3.

| was | now | tier | evidence |
| --- | --- | --- | --- |
| `func_80052DE0` | `ObjM__NoOpSlot40` | A | empty body, splat-generated stub; slot number is the only real content, matching the project's established `Class__NoOpSlotXX` convention (e.g. `TextRow__NoOpSlotD0`, `Actor__NoOpSlotD8`) |


## Track 4 (2026-09-26, round 89, echo)

The class is unified as ObjM in include/ObjM.h (table gObjMMethods, was D_80087034); the dream_scene/_l/_m views (ObjM_3bb8c_k, Obj87034_3bb8c_l, ObjM) and dream_day.h's Obj4C/SubObjB/EventArg are gone. Byte-identical.

## Round 95 (track 7, echo)

The unit banner of `src/world/dream_scene.c` was rewritten as documentation.
Its history, moved here verbatim in substance:

> dream_scene -- sixth carved slice of the dream_day block
> (0x435E0..0x44518, vram 0x80052DE0..0x80053D18), 20 functions, ALL
> MATCHED. Carved round 15; fully matched by round 45.
> This slice is entirely ObjM's own methods (gObjMMethods, include/ObjM.h;
> track 4, round 89 unified the dream_scene/_l/_m views there) ...
> the DreamSys notification dispatcher (OnDreamSysNotify, owning
> `jtbl_8001174C`) ...
> include/class_3bb8c.h is SHARED with every other class_3bb8c_* slice.
> Header edits must be strictly ADDITIVE.

The unit's data symbols were named this round (each its own `rename.py`
commit), all read only by this unit:

| old | new | holds |
| --- | --- | --- |
| `D_8008710C` | `sObjMAcceptedClassIds` | {0x1F34 DreamSys, 0x1F234 Entity, 0}: setAcceptedTags' 0-terminated class-id list |
| `D_80087118` | `sStagePendingExtras` | 14 words, one DreamSys setPendingExtra value per stage (0x80, 0x400, 0x80, 0x100, ...) |
| `D_80087150` | `sStage0Bounds` | CellBounds {0, 0, 8, 9}: setBounds on stage 0 only |
| `D_8008715C` | `sObjMViewPoint` | LongVec3 {0, -1200, 0}: attachViewChild's `vp` |
| `D_80087168` | `sObjMViewRefPoint` | LongVec3 {0, -1200, 10000}: attachViewChild's `vr` |
| `D_8008AB34` | `sObjMProjectionBias` | s32 0 (.sdata), added to setProjection's distance; no writer anywhere |

Tier A for all six: each name says what the data is and where it goes,
read from the bytes and the one site that uses it. `sStagePendingExtras`
names the slot it feeds, not what pendingExtra means in the game (Actor.h:
NotifyMove adds it to |lastOffsetValue|).

## History (moved from src/ObjMStyleActor.c, comments pass)

The ObjM section banner (resetCounters to enterState6, just above this function) carried a note on its shared header:

> include/class_3bb8c.h is shared with the other files of the old class_3bb8c segment;
> edits to it are additive.
