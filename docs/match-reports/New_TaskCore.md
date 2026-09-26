# New_TaskCore

> Renamed from `New_TaskCoreObj` on 2026-09-25 (tools/rename.py). Address 0x8003be94.

> Renamed from `func_8003BE94` on 2026-09-23 (tools/rename.py). Address 0x8003be94.

**Unit:** code_2c054 · **Status:** MATCHED (31/31 words)

A `New_X` class allocator: allocate 0xA4 bytes, and if that succeeds dispatch
the class's constructor slot (`+0x008`) with the caller's arguments forwarded
unchanged. Returns the new object, or NULL.

## The match

```c
TaskCoreObj *New_TaskCore(s32 a1, s32 a2, s32 a3)
{
    ...

    self = BMemPMgrAlloc(0xA4);
    if (self != NULL) {
        Get_vtable_TaskCore()->slot08(self, a1, a2, a3);
        return self;
    }
    return NULL;
}
```

See `src/code_2c054.c` for the exact text.

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

The base-class allocator, constructed through `TaskCoreMethods::slot08`. The
object pointer is cast at the dispatch because that slot is typed for
`StreamTaskObj` -- its usual caller -- rather than for the base object.
Arity confirmed from the disassembly: `$a0`-`$a3` and no stack slot, so self
plus three.

## Provenance

Matched by the head during divergence resolution, 2026-09-02, as part of
closing the epilogue-merge class. `docs/research/epilogue-merge-residue.md` is
updated with what survived and what did not.

## Naming

**New_TaskCoreObj** -- tier A. Canonical `New_X` allocator shape for
`TaskCoreObj` (0xA4 bytes), matching `New_StreamTask`'s own shape one
class down; the "TaskCore" name is this unit's own local view, kept
independent of `include/GameApplication.h`'s `LoaderTask` view of the identical
table (per this unit's header comment).

## Track 4 (2026-09-25, round 84, alpha)

Renamed from New_TaskCoreObj (tools/rename.py). The allocator (0xA4 bytes), now `New_TaskCore(TaskCoreTarget *target, char *soundBankPath, BasicClass *sound)`, the ctor's own arguments. The class (id 0x130, table gTaskCoreMethods) is unified as `TaskCore` in `include/TaskCore.h`; `self` is `TaskCore *` (it was the `Obj86B60` or `StreamTaskObj` view). Any source block above is the pre-unification spelling; the live body takes the unified types and slot names, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
