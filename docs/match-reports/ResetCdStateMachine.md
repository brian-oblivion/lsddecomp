# ResetCdStateMachine

> Renamed from `func_80028864` on 2026-09-18 (tools/rename.py). Address 0x80028864.

**Unit:** code_179d8_r · **Size:** 9 words · **Status:** MATCHED (9/9 words) · **Round 45**

## What it does

"Reset" bookend of the CD-read state machine -- the mirror of
`StartCdOperation`. Zeroes the phase and context globals, marks the machine
idle (`gCdIdle = 1`, the opposite sense of `StartCdOperation`'s `= 0`),
resets the timeout counter, and clears the busy flag. Called from both
`TickCdStateMachine` and `TickCdLoadFileStateMachine` on a successful `CdReadSync`.

## The C

```c
void ResetCdStateMachine(void)
{
    gCdOperation = 0;
    gCdState = 0;
    gCdTickStep = 0;
    gCdIdle = 1;
    gCdTimeoutCounter = 0;
    gCdBusy = 0;
}
```

Closed on the first attempt.

## Naming

**Tier A.** Zeroes the phase/operation/tick-step globals, marks the driver
idle (`gCdIdle = 1`) and clears busy/timeout -- the exact mirror image of
`StartCdOperation`, called from both tick functions on a successful
`CdReadSync` and from `Class6D4E8__CancelRequests` (code_179d8_q.c) when
cancelling the in-flight head request. Corroborated by that unit's own
pre-existing comment on this function: "code_179d8_r: reset the state
machine".
