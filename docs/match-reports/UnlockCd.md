> Renamed from `func_800280E0` on 2026-09-17 (tools/rename.py). Address 0x800280e0.

# UnlockCd

**Unit:** code_179d8_q (fresh carve) · **Size:** 3 instructions · **Status:** MATCHED (3/3 words)

## What this function does

Sets the scalar `s32` global `D_8008A88C` to `0`. No arguments, no return
value. The clear half of the 1/0 latch pair completed by `LockCd`
(see that report) -- `Class6D4E8__RequestLoadFile` calls `LockCd` on entry and
`UnlockCd` on every exit path; `DisableCdQueue` (queued, later in this
unit) also calls both, `LockCd` first then `UnlockCd`.

## The C

```c
extern s32 D_8008A88C;

void UnlockCd(void)
{
    D_8008A88C = 0;
}
```

## Provenance

round 45 (2026-09-15), runner echo, unit code_179d8_q (fresh carve). See
IsCdBusy.md for the sibling-accessor context; see LockCd.md for
its pair.
