# func_8003BCF4

**Unit:** code_2c054 · **Size:** 7 instructions · **Status:** MATCHED (7/7 words)

## What it does

StreamTask's override of method-table slot `+0x06C`: unconditionally
stores `a1` into `self->unk40`, then, if `a1` is non-negative, overwrites
it with `a1 * 15` (computed as `(a1 << 4) - a1`, matching retail's
`sll`/`subu` pair rather than an actual multiply instruction).

## The C

```c
void func_8003BCF4(StreamTask *self, s32 a1) {
    self->unk40 = a1;
    if (a1 >= 0) {
        self->unk40 = a1 * 15;
    }
}
```

## How it was found

Raw disassembly: `bltz $a1, .L; sw $a1, 0x40($a0)` (delay slot store
happens regardless of the branch), then on the fallthrough (non-negative)
path `sll $v0,$a1,4; subu $v0,$v0,$a1; sw $v0,0x40($a0)`. Writing the
multiply as `a1 * 15` reproduces the identical shift/subtract sequence
under `-O2`. `classtable.py D_8006E5F8` places this function at slot
`+0x06C`.

## Provenance

round 2026-09-01, runner alpha, unit code_2c054 (unit's first pass).
