> Renamed from `func_800280D0` on 2026-09-17 (tools/rename.py). Address 0x800280d0.

# LockCd

**Unit:** code_179d8_q (fresh carve) · **Size:** 4 instructions · **Status:** MATCHED (4/4 words)

## What this function does

Sets the scalar `s32` global `D_8008A88C` to `1`. No arguments, no return
value. Paired with `UnlockCd` a few functions later in ROM order,
which clears the same global back to `0`. Reads as a 1/0 "active" latch:
`Class6D4E8__RequestLoadFile` (this unit's first function, a slot of the `D_8006D4E8`
class -- see `GetClass6D4E8Methods.md`) opens with `jal LockCd` (sets the
latch on entry) and closes with `jal UnlockCd` on every exit path
(clears it). Nothing in this unit's own decompiled bodies dereferences
`D_8008A88C` directly, so its reader lives elsewhere (not yet carved from
this monolith, or already carved in a sibling unit).

## The C

```c
extern s32 D_8008A88C;

void LockCd(void)
{
    D_8008A88C = 1;
}
```

## Provenance

round 45 (2026-09-15), runner echo, unit code_179d8_q (fresh carve). See
IsCdBusy.md for the sibling-accessor context; see UnlockCd.md for
its pair.
