# GetCdState

> Renamed from `func_80027EEC` on 2026-09-17 (tools/rename.py). Address 0x80027eec.

**Unit:** CdDriver (fresh carve) · **Size:** 3 instructions · **Status:** MATCHED (3/3 words)

## What this function does

A plain `$gp`-relative getter, no arguments. Reads the scalar `s32` global
`gCdState` (in `.sdata`, zero-initialized) and returns it.

## The C

```c
extern s32 gCdState;

s32 GetCdState(void)
{
    return gCdState;
}
```

## Provenance

round 45 (2026-09-15), runner echo, unit CdDriver (fresh carve). See
IsCdBusy.md for the sibling-accessor context.

## Naming

Round 51 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_80027EEC` | `GetCdState` | A |
| `D_8008A878` | `gCdState` | A |

**Evidence.** The global is the phase variable of the CD read state machine
in `CdDriver`: `TickCdStateMachine` and `TickCdLoadFileStateMachine` both open with
`state = gCdState` and then switch on it (1 -> issue `CdlSetloc` via
`CdControlF`, 2 -> poll `CdSync`, 7 -> issue `CdRead`, 8 -> poll
`CdReadSync`), and `SetCdState` is a one-line "set the phase and reset the
timeout" helper. `StartCdOperation` seeds it per operation; `ResetCdStateMachine`
clears it to 0. That unit's own header comment already called it "the CD
state-machine phase"; this rename records it in the symbol.
