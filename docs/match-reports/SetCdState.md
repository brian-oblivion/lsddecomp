> Renamed from `func_80028888` on 2026-09-18 (tools/rename.py). Address 0x80028888.

# SetCdState

**Unit:** code_179d8_r · **Size:** 4 words · **Status:** MATCHED (4/4 words) · **Round 45**

## What it does

The state machine's "apply new phase" helper: sets `gCdState` to the
given phase and resets the timeout counter `D_8008A8A0`. This is the
shared call site both `TickCdStateMachine` and `TickCdLoadFileStateMachine` route through
(via a `newstate` local playing `$a0`'s role) every time they advance the
CD-read state machine.

## The C

```c
void SetCdState(s32 arg0)
{
    gCdState = arg0;
    D_8008A8A0 = 0;
}
```

Closed on the first attempt.
