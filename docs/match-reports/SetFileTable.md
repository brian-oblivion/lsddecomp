> Renamed from `func_80027FD8` on 2026-09-17 (tools/rename.py). Address 0x80027fd8.

# SetFileTable

**Unit:** code_179d8_q (fresh carve) · **Size:** 3 instructions · **Status:** MATCHED (3/3 words)

## What this function does

A plain `$gp`-relative setter: stores its single `s32` argument into the
scalar global `gFileTable` (in `.sdata`). No return value.

## The C

```c
extern s32 gFileTable;

void SetFileTable(s32 a0)
{
    gFileTable = a0;
}
```

## Provenance

round 45 (2026-09-15), runner echo, unit code_179d8_q (fresh carve). See
IsCdBusy.md for the sibling-accessor context.
