> Renamed from `func_80027EC8` on 2026-09-17 (tools/rename.py). Address 0x80027ec8.

# IsCdBusy

**Unit:** code_179d8_q (fresh carve) · **Size:** 3 instructions · **Status:** MATCHED (3/3 words)

## What this function does

A plain `$gp`-relative getter, no arguments. Reads the scalar `s32` global
`gCdBusy` (in `.sdata`, confirmed a single `.word` in
`asm/data/7B048.sdata.s`) and returns it.

## The C

```c
extern s32 gCdBusy;

s32 IsCdBusy(void)
{
    return gCdBusy;
}
```

## Provenance

round 45 (2026-09-15), runner echo, unit code_179d8_q (fresh carve). One of
a run of identically-shaped `$gp_rel` accessors in this unit
(IsCdBusy/ED4/EE0/EEC are getters, SetFileTable/FE4 are setters,
GetFileTableCount is a paired getter, LockCd/E0 are a 1/0 setter pair) --
see the sibling reports for the same globals block, `gCdAsyncEnabled`..
`gCdQueueEnabled`.
