# DisableCdQueue — MATCHED (11/11 words)

> Renamed from `func_80028280` on 2026-09-17 (tools/rename.py). Address 0x80028280.

Round 45, runner echo (second sitting), `src/cd/CdDriver.c`.

## Result

Byte-exact on the first attempt.

```c
extern s32 sCdQueueEnabled;

void DisableCdQueue(void)
{
    LockCd();
    sCdQueueEnabled = 0;
    UnlockCd();
}
```

## Derivation

Straight call sequence: `jal LockCd`, a gp_rel store of 0 to
`sCdQueueEnabled`, `jal UnlockCd`, then epilogue. Both callees are already
matched in this unit (`LockCd` sets `sCdLock = 1`, `UnlockCd`
sets it back to `0`) — see the header comment above `LockCd` in the
`.c`, which already named this function as the one that "clears it right
back". Confirms that comment: `DisableCdQueue` calls the set-to-1 helper,
clears an unrelated flag `sCdQueueEnabled`, then calls the set-to-0 helper —
net effect is `sCdLock` ends at 0 and `sCdQueueEnabled` is cleared. No new
struct/class knowledge.

### Proposed learning

None — same shape as the rest of this unit's small functions (plain global
touches plus already-matched sibling calls).

## Naming

Round 51 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_80028280` | `DisableCdQueue` | A |

**Evidence.** Under the lock, clears `sCdQueueEnabled` -- and nothing else.
It does not unregister the callback and does not touch the state machine, so
the tick keeps running and only queue draining stops. The narrow name is the
accurate one; `StopCdService` is `StopCdServiceIfIdle`, which is a different
function with a different effect.
