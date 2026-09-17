> Renamed from `func_80028218` on 2026-09-17 (tools/rename.py). Address 0x80028218.

# StopCdServiceIfIdle — MATCHED (26/26 words)

Round 45, runner echo (second sitting), `src/code_179d8_q.c`.

## Result

Byte-exact on the first attempt.

```c
extern s32 D_8008A898;
extern s32 gCdCallbackInstalled;
extern s32 gCdUseVSyncCallback;
extern s32 gCdQueueEnabled;
extern void VSyncCallback(void (*cb)(void));

void StopCdServiceIfIdle(void)
{
    LockCd();

    if (D_8008A898 == 0 && gCdCallbackInstalled != 0) {
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

Straight read: sets the `gCdLock` latch (`LockCd`), then a guarded
block only entered when `D_8008A898 == 0` AND `gCdCallbackInstalled != 0` (the two
`beqz`/`bnez` gp_rel loads collapse into one `&&`), inside which an optional
`VSyncCallback(0)` fires when `gCdUseVSyncCallback != 0`, then both `gCdCallbackInstalled` and
`gCdQueueEnabled` are cleared; falls through either way to clear the latch
(`UnlockCd`). The `VSyncCallback(0)` idiom (`extern void
VSyncCallback(void (*cb)(void));` then call with a literal `0`) is not new —
it already appears in `src/code_179d8_c_b.c:131-153`, reused verbatim here.

Called from `Class6D4E8__StopCdService` (this unit, matched alongside it), so it needed a
forward `extern void StopCdServiceIfIdle(void);` in this file since `Class6D4E8__StopCdService`
sits earlier in ROM order and therefore earlier in the file (unit must stay
in strict ROM-address order).

### Proposed learning

None new — confirms the existing `VSyncCallback(0)` idiom transfers cleanly
to a second call site in a different unit.
