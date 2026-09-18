> Renamed from `func_80027FE4` on 2026-09-17 (tools/rename.py). Address 0x80027fe4.

# SetFileTableCount

**Unit:** code_179d8_q (fresh carve) · **Size:** 3 instructions · **Status:** MATCHED (3/3 words)

## What this function does

A plain `$gp`-relative setter: stores its single `s32` argument into the
scalar global `gFileTableCount` (in `.sdata`). No return value. Paired with the
getter `GetFileTableCount` immediately after it in ROM order, which reads the
same global back.

## The C

```c
extern s32 gFileTableCount;

void SetFileTableCount(s32 a0)
{
    gFileTableCount = a0;
}
```

## Provenance

round 45 (2026-09-15), runner echo, unit code_179d8_q (fresh carve). See
IsCdBusy.md for the sibling-accessor context.

## Naming

Round 51 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_80027FE4` | `SetFileTableCount` | A |
| `D_8008A86C` | `gFileTableCount` | A |

**Evidence.** The loop bound both table scans in `code_179d8_r` stop at
(`while (i < gFileTableCount)`, `if (i >= gFileTableCount) return -1;`) over
the 0x1C-stride array based at `gFileTable`. `code_171e0.c`'s `RegisterFileTableEntries`
sets it to `GetFileTableCount() + n` before resolving `n` new entries, i.e.
the table grows by appending. Setter of the element count: tier A.
