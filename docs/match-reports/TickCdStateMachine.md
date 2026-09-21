# TickCdStateMachine

> Renamed from `func_8002858C` on 2026-09-18 (tools/rename.py). Address 0x8002858c.

**Unit:** code_179d8_r · **Size:** 86 words · **Status:** MATCHED (86/86 words) · **Round 45**

## What it does

One tick of a small CD-read state machine. `gCdState` holds the current
phase (0 default, 1 = issue `CdControlF` seek, 2 = poll `CdSync`, 7 = issue
`CdRead`, 8 = poll `CdReadSync`; anything else in `{3,4,5,6}` or `>8` is a
no-op). `gCdTimeoutCounter` is a busy-wait timeout counter, reset by
`SetCdState` whenever the phase advances. Called from
`ServiceCdDriver` (in the sibling unit `code_179d8_q.c`) when `gCdTickStep ==
1`; `TickCdLoadFileStateMachine` is this same state machine's other tick variant
(`gCdTickStep == 2`), differing only in what happens when `CdSync` reports
"still the same phase" and after a successful `CdReadSync`.

## The C

```c
extern void ResetCdStateMachine(void);
extern void SetCdState(s32 arg0);

void TickCdStateMachine(void)
{
    s32 state;
    s32 v1;
    s32 newstate;

    LockCd();
    state = gCdState;

    if (state == 2)
        goto L_state2;
    if (state < 3) {
        if (state == 1)
            goto L_state1;
        goto L_end;
    }
    if (state == 7)
        goto L_state7;
    if (state == 8)
        goto L_state8;
    goto L_end;

L_state1:
    if (CdControlF(2, (u8 *)gCdSeekParam + 0x14) == 0)
        goto L_end;
    newstate = 2;
    goto L_set;

L_state2:
    v1 = CdSync(1, NULL);
    if (v1 == state)
        goto L_reset;
    if (v1 < 3) {
        if (v1 == 0)
            goto L_count;
        goto L_end;
    }
    if (v1 != 5)
        goto L_end;
    newstate = 1;
    goto L_set;

L_count:
    gCdTimeoutCounter++;
    if (gCdTimeoutCounter < 0x259)
        goto L_end;
    newstate = 1;
    goto L_set;

L_state7:
    if (CdRead(gCdReadSectorCount, gCdReadBuffer, 0x80) == 0)
        goto L_end;
    newstate = 8;
    goto L_set;

L_state8:
    v1 = CdReadSync(1, 0);
    if (v1 == -1)
        goto L_pending;
    if (v1 != 0)
        goto L_end;

L_reset:
    ResetCdStateMachine();
    goto L_end;

L_pending:
    CdFlush();
    newstate = 1;

L_set:
    SetCdState(newstate);

L_end:
    UnlockCd();
}
```

## What made this close

The first attempt used an ordinary `if / else if` ladder (`if (state==2)
{...} else if (state<3) {...} else if (state==7) {...} else if
(state==8){...}`), which compiled to correct *logic* but a completely
different branch shape from retail: each `if` in an `else-if` chain
branches *around* its own body, whereas retail's actual layout is a
dispatch of forward `goto`s at the top followed by the bodies placed in
source order -- and, crucially, retail shares **one** call site for
`SetCdState(newstate)` across five different callers (state1-success,
state2's two "retry" exits, state7-success, state8-pending) rather than
five separate call sites. Reading the raw asm label-by-label and
translating it 1:1 into `goto`s targeting a single shared `L_set:`
trampoline (with `newstate` playing the role of retail's `$a0`, set right
before each `goto L_set`) reproduced retail's block order and its call-site
sharing exactly, and this closed on the first attempt at that shape.

### Proposed learning

For a state-machine tick function with a shared "apply new state" tail
called from multiple branches, do not write the natural-looking `if/else
if` ladder -- read the actual retail block order off the asm (labels in
address order) and translate it as a forward-`goto` dispatch into
source-ordered blocks, with the shared tail reached via a plain variable
(`newstate`) set right before each `goto`. GCC 2.6.3 -O2 places labeled
blocks in the order they're *written*, so matching that written order is
what reproduces the exact branch/block layout, not just the logic.

## Naming

**Tier B.** One tick of the CD-read state-machine's phase dispatch (phase 1
= issue `CdControlF(CD_CMD_SETLOC, ...)`, 2 = poll `CdSync`, 7 = issue
`CdRead`, 8 = poll `CdReadSync`), selected by `ServiceCdDriver`
(code_179d8_q.c) when `gCdTickStep == 1`. This is the *default* of the two
tick functions: cross-referencing every `gCdTickStep = 1` assignment in
code_179d8_s.c shows it backs three different request paths --
`func_800272D0` (open/resolve), `func_80027528` (explicit seek) and
`func_800276D0` (straight read from the current position, which starts at
phase 7 directly, so this function's phase-2 branch is never exercised on
that path). Named for the mechanics (a generic state-machine tick); which
of those three call sites is *the* reason for its behaviour (as opposed to
`TickCdLoadFileStateMachine`'s) is not established, so this stays tier B
rather than a name asserting one specific operation.

See `TickCdLoadFileStateMachine`'s report for the paired evidence and the
one call site that needs the other tick function.
