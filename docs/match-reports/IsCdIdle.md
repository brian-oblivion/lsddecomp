# IsCdIdle

> Renamed from `func_80027ED4` on 2026-09-17 (tools/rename.py). Address 0x80027ed4.

**Unit:** code_179d8_q (fresh carve) · **Size:** 3 instructions · **Status:** MATCHED (3/3 words)

## What this function does

A plain `$gp`-relative getter, no arguments. Reads the scalar `s32` global
`gCdIdle` (in `.sdata`, initialized to `0x00000001` per
`asm/data/7B048.sdata.s`) and returns it.

## The C

```c
extern s32 gCdIdle;

s32 IsCdIdle(void)
{
    return gCdIdle;
}
```

## Provenance

round 45 (2026-09-15), runner echo, unit code_179d8_q (fresh carve). See
IsCdBusy.md for the sibling-accessor context.

## Naming

Round 51 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_80027ED4` | `IsCdIdle` | B |
| `D_8008A870` | `gCdIdle` | B |

**Evidence.** Written in exactly the same two places as `gCdBusy` and always
to the opposite value: `StartCdOperation` (operation start) sets `gCdBusy = 1`
and `gCdIdle = 0`, `ResetCdStateMachine` (state-machine reset) sets `gCdIdle = 1`
and `gCdBusy = 0`. Its initial value in `.sdata` is 1. `code_171e0.c`'s
wrapper returns 1 when no CD source is selected, matching "idle". The one
reader that is not a getter is `Class6D4E8__CancelRequests`, which only
aborts a transfer in flight when `gCdIdle == 0`.

**Why B and not A.** The mechanics are certain; what is NOT established is
why the driver carries two globals that are exact complements. One of them
presumably means something narrower than the other, and nothing in the three
carved units distinguishes them. A reader should know that `gCdIdle` is
`!gCdBusy` in every write the corpus contains, which is why this is written
down rather than smoothed over.
