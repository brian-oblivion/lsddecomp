> Renamed from `func_80027ED4` on 2026-09-17 (tools/rename.py). Address 0x80027ed4.

# IsCdIdle

**Unit:** code_179d8_q (fresh carve) · **Size:** 3 instructions · **Status:** MATCHED (3/3 words)

## What this function does

A plain `$gp`-relative getter, no arguments. Reads the scalar `s32` global
`D_8008A870` (in `.sdata`, initialized to `0x00000001` per
`asm/data/7B048.sdata.s`) and returns it.

## The C

```c
extern s32 D_8008A870;

s32 IsCdIdle(void)
{
    return D_8008A870;
}
```

## Provenance

round 45 (2026-09-15), runner echo, unit code_179d8_q (fresh carve). See
IsCdBusy.md for the sibling-accessor context.
