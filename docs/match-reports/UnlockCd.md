# UnlockCd

> Renamed from `func_800280E0` on 2026-09-17 (tools/rename.py). Address 0x800280e0.

**Unit:** code_179d8_q (fresh carve) · **Size:** 3 instructions · **Status:** MATCHED (3/3 words)

## What this function does

Sets the scalar `s32` global `sCdLock` to `0`. No arguments, no return
value. The clear half of the 1/0 latch pair completed by `LockCd`
(see that report) -- `CdDriver__RequestLoadFile` calls `LockCd` on entry and
`UnlockCd` on every exit path; `DisableCdQueue` (queued, later in this
unit) also calls both, `LockCd` first then `UnlockCd`.

## The C

```c
extern s32 sCdLock;

void UnlockCd(void)
{
    sCdLock = 0;
}
```

## Provenance

round 45 (2026-09-15), runner echo, unit code_179d8_q (fresh carve). See
IsCdBusy.md for the sibling-accessor context; see LockCd.md for
its pair.

## Naming

Round 51 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_800280E0` | `UnlockCd` | A |

**Evidence.** Clears `sCdLock`; the release half of the pair documented in
`LockCd.md`, called on every exit path of every entry point that takes it.
Tier A.
