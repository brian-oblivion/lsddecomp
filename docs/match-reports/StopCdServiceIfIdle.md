# StopCdServiceIfIdle — MATCHED (26/26 words)

> Renamed from `func_80028218` on 2026-09-17 (tools/rename.py). Address 0x80028218.

Round 45, runner echo (second sitting), `src/cd/CdDriver.c`.

## Result

Byte-exact on the first attempt.

```c
extern s32 sCdTickStep;
extern s32 sCdCallbackInstalled;
extern s32 sCdUseVSyncCallback;
extern s32 sCdQueueEnabled;
extern void VSyncCallback(void (*cb)(void));

void StopCdServiceIfIdle(void)
{
    LockCd();

    if (sCdTickStep == 0 && sCdCallbackInstalled != 0) {
        if (sCdUseVSyncCallback != 0) {
            VSyncCallback(0);
        }
        sCdCallbackInstalled = 0;
        sCdQueueEnabled = 0;
    }

    UnlockCd();
}
```

## Derivation

Straight read: sets the `sCdLock` latch (`LockCd`), then a guarded
block only entered when `sCdTickStep == 0` AND `sCdCallbackInstalled != 0` (the two
`beqz`/`bnez` gp_rel loads collapse into one `&&`), inside which an optional
`VSyncCallback(0)` fires when `sCdUseVSyncCallback != 0`, then both `sCdCallbackInstalled` and
`sCdQueueEnabled` are cleared; falls through either way to clear the latch
(`UnlockCd`). The `VSyncCallback(0)` idiom (`extern void
VSyncCallback(void (*cb)(void));` then call with a literal `0`) is not new —
it already appears in `src/psyq/libsnd_ssinit_libapi_counter.c:131-153`, reused verbatim here.

Called from `CdDriver__StopService` (this unit, matched alongside it), so it needed a
forward `extern void StopCdServiceIfIdle(void);` in this file since `CdDriver__StopService`
sits earlier in ROM order and therefore earlier in the file (unit must stay
in strict ROM-address order).

### Proposed learning

None new — confirms the existing `VSyncCallback(0)` idiom transfers cleanly
to a second call site in a different unit.

## Naming

Round 51 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_80028218` | `StopCdServiceIfIdle` | A |

**Evidence.** Under the lock, and only when `sCdTickStep == 0` (no
state-machine step is armed) AND the callback is installed: unregister the
`VSyncCallback`, clear `sCdCallbackInstalled` and clear `sCdQueueEnabled`.
The conditional is half the function, so the name carries it -- calling this
`StopCdService` would say it always stops, which it does not. The mirror of
`StartCdService`.

`sCdTickStep` itself is left named: it is `CdDriver`'s "which state-machine
step to tick" selector (1 or 2), written by `CdDriver` and cleared by
that unit's reset, so it belongs to whichever unit's naming pass takes
`CdDriver`. Proposed there: `gCdStep`.

## Track 7 (round 101, echo): comments moved here, and names

`sCdTickStep == 0` is spelled `CD_TICK_NONE`, added to include/CdDriver.h
next to CD_TICK_STATE_MACHINE / CD_TICK_LOAD_FILE: no state machine is
ticking (ResetCdStateMachine's value).
