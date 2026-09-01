# func_8003BB5C

**Unit:** code_2c054 · **Size:** 46 instructions · **Status:** MATCHED (46/46 words)

## What it does

StreamTask's override of method-table slot `+0x05C`: chains into the base
task class's own copy first, then -- only when `unkA4` is still zero --
polls the sub-object's slot `+0x048`, stores the result back into
`unkA4`, and, only if that result is non-zero AND `unkD8` is still clear,
re-enters this object's own slot `+0x060` (`func_8003BC14`) with a
literal 7.

## The C

```c
void func_8003BB5C(StreamTask *self, s32 a1, s32 a2) {
    s32 result;
    func_8003DFBC()->slot5C(self, a1, a2);
    if (self->unkA4 != 0) {
        return;
    }
    result = self->unkB4->methods->slot48(self->unkB4);
    self->unkA4 = result;
    if (result == 0) {
        return;
    }
    if (self->unkD8 != 0) {
        return;
    }
    self->methods->slot60(self, 7);
}
```

## How it was found

Retail's own control flow is a chain of early-exit branches, all landing
on the same epilogue label -- NOT nested `if`s. Writing it as three
sequential `if (...) { return; }` guards (rather than nesting) reproduces
that shape exactly; nesting would still be logically equivalent but was
not needed here (unlike func_8003B854/func_8003BE94's `New_X` shape,
where the exact nesting choice DID matter for instruction count -- see
those reports). `classtable.py D_8006E5F8` places this function at slot
`+0x05C`.

## Provenance

round 2026-09-01, runner alpha, unit code_2c054 (second pass).
