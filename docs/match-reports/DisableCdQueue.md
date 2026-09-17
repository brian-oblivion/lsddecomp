> Renamed from `func_80028280` on 2026-09-17 (tools/rename.py). Address 0x80028280.

# DisableCdQueue — MATCHED (11/11 words)

Round 45, runner echo (second sitting), `src/code_179d8_q.c`.

## Result

Byte-exact on the first attempt.

```c
extern s32 gCdQueueEnabled;

void DisableCdQueue(void)
{
    LockCd();
    gCdQueueEnabled = 0;
    UnlockCd();
}
```

## Derivation

Straight call sequence: `jal LockCd`, a gp_rel store of 0 to
`gCdQueueEnabled`, `jal UnlockCd`, then epilogue. Both callees are already
matched in this unit (`LockCd` sets `gCdLock = 1`, `UnlockCd`
sets it back to `0`) — see the header comment above `LockCd` in the
`.c`, which already named this function as the one that "clears it right
back". Confirms that comment: `DisableCdQueue` calls the set-to-1 helper,
clears an unrelated flag `gCdQueueEnabled`, then calls the set-to-0 helper —
net effect is `gCdLock` ends at 0 and `gCdQueueEnabled` is cleared. No new
struct/class knowledge.

### Proposed learning

None — same shape as the rest of this unit's small functions (plain global
touches plus already-matched sibling calls).
