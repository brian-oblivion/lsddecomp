# LockCd

> Renamed from `func_800280D0` on 2026-09-17 (tools/rename.py). Address 0x800280d0.

**Unit:** code_179d8_q (fresh carve) · **Size:** 4 instructions · **Status:** MATCHED (4/4 words)

## What this function does

Sets the scalar `s32` global `gCdLock` to `1`. No arguments, no return
value. Paired with `UnlockCd` a few functions later in ROM order,
which clears the same global back to `0`. Reads as a 1/0 "active" latch:
`Class6D4E8__RequestLoadFile` (this unit's first function, a slot of the `D_8006D4E8`
class -- see `GetClass6D4E8Methods.md`) opens with `jal LockCd` (sets the
latch on entry) and closes with `jal UnlockCd` on every exit path
(clears it). Nothing in this unit's own decompiled bodies dereferences
`gCdLock` directly, so its reader lives elsewhere (not yet carved from
this monolith, or already carved in a sibling unit).

## The C

```c
extern s32 gCdLock;

void LockCd(void)
{
    gCdLock = 1;
}
```

## Provenance

round 45 (2026-09-15), runner echo, unit code_179d8_q (fresh carve). See
IsCdBusy.md for the sibling-accessor context; see UnlockCd.md for
its pair.

## Naming

Round 51 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_800280D0` | `LockCd` | A |
| `D_8008A88C` | `gCdLock` | A |

**Evidence, and it corrects this report's round-45 reading.** That reading
said "nothing in this unit's own bodies dereferences `D_8008A88C`, so its
consumer lives elsewhere". The consumer is in this unit and was queued at the
time: `ServiceCdDriver` (`func_800280EC`) returns immediately when the flag
is set. With that reader in hand the pattern is unambiguous -- every public
entry point in this unit and in `code_179d8_r`/`code_179d8_s` brackets its
body with set-then-clear, and the one thing that reads the flag is the
VSync-driven service tick, which skips its turn rather than walk a
half-updated queue. That is a re-entrancy latch against an interrupt-time
callback, not a mutex: nothing spins or blocks on it.

`code_179d8_r`'s header comment had already called this pair lock/unlock;
this rename records it in the symbols.
