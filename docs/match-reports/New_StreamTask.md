# New_StreamTask

> Renamed from `New_StreamTaskObj` on 2026-09-26 (tools/rename.py). Address 0x8003b854.

> Renamed from `func_8003B854` on 2026-09-23 (tools/rename.py). Address 0x8003b854.

**Unit:** task · **Status:** MATCHED (36/36 words)

A `New_X` class allocator: allocate 0xDC bytes, and if that succeeds dispatch
the class's constructor slot (`+0x008`) with the caller's arguments forwarded
unchanged. Returns the new object, or NULL.

## The match

```c
StreamTaskObj *New_StreamTask(s32 a1, s32 a2, s32 a3, s32 a4)
{
    ...

    self = BMemPMgrAlloc(0xDC);
    if (self != NULL) {
        GetStreamTaskMethods()->slot08(self, a1, a2, a3, a4);
        return self;
    }
    return NULL;
}
```

See `src/app/task.c` for the exact text.

## Why it matched: `return NULL;` goes LAST

This is one of five instances of the `New_X` epilogue-merge residue class
closed in a single pass. **The full account, the measurement table for the
four source shapes that do NOT work, and the sub-shape that this rule does
NOT close, are all in `docs/match-reports/New_StageMap.md`** -- read that
one rather than re-deriving from here.

The one-line version: retail materializes the return value twice, from two
different operands, which means two `return` statements in the source. Put the
`return NULL;` after the success return and GCC 2.6.3 produces retail's
epilogue merge. Put it first and it costs an extra `j`.

## History

Previously filed as **NOT ATTEMPTED**, a predicted instance.

This one is also a **five-argument** dispatch, and worth reading alongside
`docs/match-reports/DayTask__StartObjM.md` for that reason. Retail stores the
fifth argument with `sw $s1, 0x10($sp)` -- the o32 stack-argument slot -- so
`StreamTaskObjMethods::slot08` is typed with self plus four arguments. The
slot did not exist in the header before this match; it was inside `pad04`.

## Provenance

Matched by the head during divergence resolution, 2026-09-02, as part of
closing the epilogue-merge class. `docs/research/epilogue-merge-residue.md` is
updated with what survived and what did not.

## Naming

**New_StreamTask** -- tier A. Canonical `New_X` allocator shape (allocate,
dispatch the ctor slot, return); the class is independently established both
by `gStreamTaskMethods`'s own ctor-slot dispatch (`classtable.py`) and by
`include/game_application.h`'s cross-unit `StreamTask` view of the same call site.

## Track 4 (2026-09-25, round 84, alpha)

StreamTaskObj's table now expands TASKCORE_SLOTS (include/TaskCore.h, round 84): the ctor call is `ctor` and its fifth argument is cast to the ctor's `StreamTaskInitData *`. Byte-identical.

## Track 4 (2026-09-26, round 87)

Renamed with the class unification (gStreamTaskObjMethods -> class StreamTask, include/stream_task.h): the `Obj` suffix is dropped (track 4 step 2; include/game_application.h already viewed the class as `StreamTask`). The 0xDC-byte allocator.


## Track 6 (2026-09-27, round 99, runner bravo)

The fifth parameter is a `DrawRect *` (draw_system.h); `StreamTaskInitData`, its local spelling, is deleted (StreamTask__StreamTask.md). Byte-identical.

## History (moved from src/Task.c, comments pass)

The file's banner carried its edge evidence and the reason for its name:

> Edges (tools/tuboundary.py). The start edge is libgs/gs_122, a placed Sony
> object. Inside, no rodata crossing and no forced boundary, and every edge
> between the carve slices this file was made of is "probably one file" on
> single-user data (0x8006e86c >= 0x8006e730, 0x8008a90c >= 0x8008a8f4);
> content agrees, TaskCore running across the first two slice edges and
> Viewport across the last. The game's file ended at GetRootNode, since
> GsSetProjection is Sony's; no tool splits a unit, so it stays. Named for
> the task classes rather than for one class, whose header already holds
> that name.
