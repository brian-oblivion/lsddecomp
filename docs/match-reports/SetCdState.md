# SetCdState

> Renamed from `func_80028888` on 2026-09-18 (tools/rename.py). Address 0x80028888.

**Unit:** CdDriver · **Size:** 4 words · **Status:** MATCHED (4/4 words) · **Round 45**

## What it does

The state machine's "apply new phase" helper: sets `gCdState` to the
given phase and resets the timeout counter `gCdTimeoutCounter`. This is the
shared call site both `TickCdStateMachine` and `TickCdLoadFileStateMachine` route through
(via a `newstate` local playing `$a0`'s role) every time they advance the
CD-read state machine.

## The C

```c
void SetCdState(s32 arg0)
{
    gCdState = arg0;
    gCdTimeoutCounter = 0;
}
```

Closed on the first attempt.

## Naming

**Tier A.** Sets `gCdState` to a new phase and clears `gCdTimeoutCounter` --
the shared "advance to phase N" primitive both tick functions route every
transition through (`goto L_set; ... SetCdState(newstate);`). Named to
parallel `GetCdState` (CdDriver.c's already-established accessor for
the same global).
