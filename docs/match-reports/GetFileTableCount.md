# GetFileTableCount

> Renamed from `func_80027FF0` on 2026-09-17 (tools/rename.py). Address 0x80027ff0.

**Unit:** CdDriver (fresh carve) · **Size:** 3 instructions · **Status:** MATCHED (3/3 words)

## What this function does

A plain `$gp`-relative getter, no arguments. Reads the scalar `s32` global
`sFileTableCount` and returns it -- the getter half of the setter/getter pair
completed by `SetFileTableCount` immediately before it in ROM order.

## The C

```c
extern s32 sFileTableCount;

s32 GetFileTableCount(void)
{
    return sFileTableCount;
}
```

## Provenance

round 45 (2026-09-15), runner echo, unit CdDriver (fresh carve). See
IsCdBusy.md for the sibling-accessor context.

## Naming

Round 51 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_80027FF0` | `GetFileTableCount` | A |

**Evidence.** Returns `sFileTableCount` (see `SetFileTableCount.md`). Its one
caller, `GameApplicationFileResource.c`'s `RegisterFileTableEntries`, uses the value as the index of the
first free slot before extending the table -- consistent with a count, not a
capacity. Tier A by the pure-leaf rule.
