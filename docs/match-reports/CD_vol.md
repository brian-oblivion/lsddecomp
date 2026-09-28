# CD_vol

> Renamed from `func_8002A378` on 2026-09-23 (tools/rename.py). Address 0x8002a378.

**Unit:** libcd_bios · **Size:** 34 words · **Status:** MATCHED (34/34 words)

## What it does

Stages a 4-byte address into four raw HW pointer variables that this window's
globals reading treats as the link/SIO driver's register staging area (see
`libcd_bios.c`'s header comment). Writes a mode byte (2, then 3), copies
`arg0[0..3]` byte-by-byte through `D_8006D8C8`/`D_8006D8CC`/`D_8006D8C4`, and
finishes with a fixed terminator byte `0x20`.

## The C

```c
s32 CD_vol(u8 *arg0)
{
    *D_8006D8C0 = 2;
    *D_8006D8C8 = arg0[0];
    *D_8006D8CC = arg0[1];
    *D_8006D8C0 = 3;
    *D_8006D8C4 = arg0[2];
    *D_8006D8C8 = arg0[3];
    *D_8006D8CC = 0x20;
    return 0;
}
```

## Notes

`D_8006D8C0`/`D_8006D8C4`/`D_8006D8C8`/`D_8006D8CC` are pointer VARIABLES (each
holds an address, loaded fresh via `lw` before every dereference) -- not
arrays. Declared `volatile u8 *` locally in this unit; the `volatile` on the
pointee matters for later functions that spin-poll through similarly-shaped
pointers (see `CD_getsector`), so the whole family is typed consistently.

Sibling `libcd_bios.c` already carries its own extern for this function as
`s32 CD_vol(void *arg0)` (called from `func_800291C8`). This unit's own
reading uses `u8 *` since the body indexes it byte-wise -- kept local per the
project's multiple-independent-local-views convention, not promoted to a
shared header.

### Proposed learning

None beyond what's already documented (address-drift bites unit-local extern
declarations too, not just struct edits -- see CD_initintr's report).

## History (moved from src/libcd_bios.c, comments pass)

The module declarations above CD_vol carried:

> Still INCLUDE_ASM in THIS unit (not yet converted) -- INCLUDE_ASM leaves no
> C-level prototype of its own, so callers within this file need one.

The module declarations above CD_vol carried:

> Still INCLUDE_ASM elsewhere -- not this unit's to carve.
