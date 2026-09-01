# func_8003BE7C

**Unit:** code_2c054 · **Size:** 2 instructions · **Status:** MATCHED (2/2 words)

## What it does

One-line field setter: `self->unkD4 = a1`.

## The C

```c
void func_8003BE7C(StreamTask *self, s32 a1) {
    self->unkD4 = a1;
}
```

## How it was found

Same shape as func_8003BE5C, four fields over (`classtable.py` slot
`+0x134`, the last slot in D_8006E5F8's 77-slot table). See
func_8003BE5C.md for the shared context.

## Provenance

round 2026-09-01, runner alpha, unit code_2c054 (unit's first pass).
