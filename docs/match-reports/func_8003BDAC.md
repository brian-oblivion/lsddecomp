# func_8003BDAC

**Unit:** code_2c054 · **Size:** 14 instructions · **Status:** MATCHED (14/14 words)

## What it does

StreamTask's override of method-table slot `+0x084`: the identical
passthrough shape as func_8003BD74, one slot over.

## The C

```c
s32 func_8003BDAC(StreamTask *self) {
    return func_8003DFBC()->slot84(self);
}
```

## How it was found

Same shape as func_8003BD74 (see that report for the full derivation).
`classtable.py` confirms D_8006E5F8 and D_8006E730 share the same
function address (`func_8003C9B0`) at offset `+0x084`. Return type is
likewise an unproven `s32` guess, flagged for the same reason as
func_8003BD74.

## Provenance

round 2026-09-01, runner alpha, unit code_2c054 (unit's first pass).
