> Renamed from `func_80028844` on 2026-09-18 (tools/rename.py). Address 0x80028844.

# StartCdOperation

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
