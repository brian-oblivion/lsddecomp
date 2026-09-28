# New_TimedTask

> Renamed from `New_Class86668` on 2026-09-26 (tools/rename.py). Address 0x8004a130.

> Renamed from `func_8004A130` on 2026-09-23 (tools/rename.py). Address 0x8004a130.

**Unit:** DayTaskStageMap · **Status:** MATCHED (27/27 words)

A `New_X` class allocator: allocate 0x38 bytes, and if that succeeds dispatch
the class's constructor slot (`+0x008`) with the caller's arguments forwarded
unchanged. Returns the new object, or NULL.

## The match

```c
Obj865C8 *New_TimedTask(s32 arg1, SubObjB *arg2)
{
    ...

    self = BMemPMgrAlloc(0x38);
    if (self != NULL) {
        GetTimedTaskMethods()->ctor(self, arg1, arg2);
        return self;
    }
    return NULL;
}
```

See `src/DayTaskStageMap.c` for the exact text.

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

Previously filed as **STALLED 26/27** after two attempts, correctly
recognising the precedent in `New_GameApplication` and stopping rather than
burning budget. The precedent was real; the conclusion drawn from it -- that
the class needed a permuter -- was not. Note that `New_GameApplication` is in
fact the OTHER sub-shape of the class and is still open; see the canonical
report for the distinction.

## Provenance

Matched by the head during divergence resolution, 2026-09-02, as part of
closing the epilogue-merge class. `docs/research/epilogue-merge-residue.md` is
updated with what survived and what did not.

## Naming

`New_TimedTask` -- tier A. Allocator (0x38 bytes) for the sibling class, dispatching `GetTimedTaskMethods()->ctor`: matches the `New_X` idiom.

## Track 4

2026-09-25, round 84 (bravo): class unified in `include/TimedTask.h`. Not renamed. Signature `TimedTask *New_TimedTask(char *soundBankPath, BasicClass *sound)`, the ctor's parameters; 0x38 is the object size TIMEDTASK_FIELDS ends at. No C caller. Image byte-identical.
