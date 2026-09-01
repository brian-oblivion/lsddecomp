# func_8003BDF4

**Unit:** code_2c054 · **Size:** 26 instructions · **Status:** MATCHED (26/26 words)

## What it does

StreamTask's override of method-table slot `+0x094`: when `unkD4` is set,
ticks the sub-object's slot `+0x04C`; otherwise re-enters this object's
own slot `+0x060` (`func_8003BC14`) with a literal 7.

## The C

```c
void func_8003BDF4(StreamTask *self) {
    if (self->unkD4 != 0) {
        self->unkB4->methods->slot4C(self->unkB4);
    } else {
        self->methods->slot60(self, 7);
    }
}
```

## How it was found

Unlike most of this unit's other slots, func_8003BDF4 does NOT chain into
the base task class at all -- there is no `func_8003DFBC()` call anywhere
in its disassembly, just a straight `if`/`else` on `unkD4`. This is the
target func_8003BC14's own `case 0x12` re-enters (see func_8003BC14.md),
and it itself re-enters func_8003BC14's `case 8` path (the sub-object
slot `+0x04C` dispatch) and `case 7` path (via `self->methods->slot60`,
same slot, same argument 7) respectively -- the two functions call into
each other's territory in a small mutual-dispatch cluster.
`classtable.py D_8006E5F8` places this function at slot `+0x094`.

## Provenance

round 2026-09-01, runner alpha, unit code_2c054 (second pass).
