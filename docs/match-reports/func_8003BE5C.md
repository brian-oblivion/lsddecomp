# func_8003BE5C

**Unit:** code_2c054 · **Size:** 2 instructions · **Status:** MATCHED (2/2 words)

## What it does

One-line field setter: `self->unkC4 = a1`. StreamTask instance field at
offset 0xC4.

## The C

```c
void func_8003BE5C(StreamTask *self, s32 a1) {
    self->unkC4 = a1;
}
```

## How it was found

Raw disassembly is `sw $a1, 0xC4($a0); jr $ra`. `classtable.py D_8006E5F8`
places this function at slot `+0x124` of StreamTask's own method table.
Field 0xC4 is also one of the five fields `func_8003BA38` (StreamTask's
slot `+0x040` override, also matched this round) resets in bulk, which is
where the field's role as part of a related quintet (0xC4/0xC8/0xCC/
0xD0/0xD4) comes from.

## Provenance

round 2026-09-01, runner alpha, unit code_2c054 (unit's first pass).
