# StartCdOperation

> Renamed from `func_80028844` on 2026-09-18 (tools/rename.py). Address 0x80028844.

**Unit:** code_179d8_r · **Size:** 8 words · **Status:** MATCHED (8/8 words) · **Round 45**

## What it does

"Start" bookend of the CD-read state machine: takes a context value and an
initial phase, stashes them into the state-machine globals, marks the
machine busy, and flags the current head-of-list node (`gCdRequestQueue`,
shared with `AllocCdRequestNode`/`FreeCdRequestNode`'s list) as active. No
lock/unlock bracketing (unlike its siblings in this unit) and no NULL
check on `gCdRequestQueue` -- retail dereferences it unconditionally, so this is
presumably only ever called with a live list head.

## The C

```c
void StartCdOperation(s32 arg0, s32 arg1)
{
    gCdBusy = 1;
    gCdOperation = arg0;
    gCdState = arg1;
    gCdIdle = 0;
    gCdRequestQueue->unk0 = 1;
}
```

Closed on the first attempt.

## Naming

**Tier A.** Stores an operation code and initial phase into
`gCdOperation`/`gCdState`, marks the driver busy (`gCdBusy = 1`,
`gCdIdle = 0`) and marks the queue's head node `active`. Exact mirror image
of `ResetCdStateMachine`; the "start the operation the head queue node
represents" purpose is evident from the body (every field it writes is one
this unit's other functions later read to drive or unwind that operation)
and confirmed by five call sites across code_179d8_s.c, each passing a
distinct `(op, state)` pair for a distinct request type (open, close, seek,
read, load-file).
