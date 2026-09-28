# StopCdServiceIfIdle — MATCHED (26/26 words)

> Renamed from `func_80028218` on 2026-09-17 (tools/rename.py). Address 0x80028218.

Round 45, runner echo (second sitting), `src/code_179d8_q.c`.

## Result

Byte-exact on the first attempt.

```c
extern s32 gCdTickStep;
extern s32 gCdCallbackInstalled;
extern s32 gCdUseVSyncCallback;
extern s32 gCdQueueEnabled;
extern void VSyncCallback(void (*cb)(void));

void StopCdServiceIfIdle(void)
{
    LockCd();

    if (gCdTickStep == 0 && gCdCallbackInstalled != 0) {
        if (gCdUseVSyncCallback != 0) {
            VSyncCallback(0);
        }
        gCdCallbackInstalled = 0;
        gCdQueueEnabled = 0;
    }

    UnlockCd();
}
```

## Derivation

Straight read: sets the `sCdLock` latch (`LockCd`), then a guarded
block only entered when `gCdTickStep == 0` AND `gCdCallbackInstalled != 0` (the two
`beqz`/`bnez` gp_rel loads collapse into one `&&`), inside which an optional
`VSyncCallback(0)` fires when `gCdUseVSyncCallback != 0`, then both `gCdCallbackInstalled` and
`gCdQueueEnabled` are cleared; falls through either way to clear the latch
(`UnlockCd`). The `VSyncCallback(0)` idiom (`extern void
VSyncCallback(void (*cb)(void));` then call with a literal `0`) is not new —
it already appears in `src/libsnd_ssinit_libapi_counter.c:131-153`, reused verbatim here.

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

**Evidence.** Under the lock, and only when `gCdTickStep == 0` (no
state-machine step is armed) AND the callback is installed: unregister the
`VSyncCallback`, clear `gCdCallbackInstalled` and clear `gCdQueueEnabled`.
The conditional is half the function, so the name carries it -- calling this
`StopCdService` would say it always stops, which it does not. The mirror of
`StartCdService`.

`gCdTickStep` itself is left named: it is `code_179d8_r`'s "which state-machine
step to tick" selector (1 or 2), written by `code_179d8_s` and cleared by
that unit's reset, so it belongs to whichever unit's naming pass takes
`code_179d8_r`. Proposed there: `gCdStep`.
