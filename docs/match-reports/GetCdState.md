> Renamed from `func_80027EEC` on 2026-09-17 (tools/rename.py). Address 0x80027eec.

# GetCdState

**Unit:** code_179d8_q (fresh carve) · **Size:** 3 instructions · **Status:** MATCHED (3/3 words)

## What this function does

A plain `$gp`-relative getter, no arguments. Reads the scalar `s32` global
`D_8008A878` (in `.sdata`, zero-initialized) and returns it.

## The C

```c
extern s32 D_8008A878;

s32 GetCdState(void)
{
    return D_8008A878;
}
```

## Provenance

round 45 (2026-09-15), runner echo, unit code_179d8_q (fresh carve). See
IsCdBusy.md for the sibling-accessor context.
