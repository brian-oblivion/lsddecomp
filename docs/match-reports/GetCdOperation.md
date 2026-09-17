> Renamed from `func_80027EE0` on 2026-09-17 (tools/rename.py). Address 0x80027ee0.

# GetCdOperation

**Unit:** code_179d8_q (fresh carve) · **Size:** 3 instructions · **Status:** MATCHED (3/3 words)

## What this function does

A plain `$gp`-relative getter, no arguments. Reads the scalar `s32` global
`D_8008A874` (in `.sdata`, zero-initialized) and returns it.

## The C

```c
extern s32 D_8008A874;

s32 GetCdOperation(void)
{
    return D_8008A874;
}
```

## Provenance

round 45 (2026-09-15), runner echo, unit code_179d8_q (fresh carve). See
IsCdBusy.md for the sibling-accessor context.
