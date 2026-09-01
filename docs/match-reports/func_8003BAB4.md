# func_8003BAB4

**Unit:** code_2c054 · **Size:** 42 instructions · **Status:** MATCHED (42/42 words)

## What it does

StreamTask's override of method-table slot `+0x04C`: chains into the base
task class's own copy of the same slot first, resets `unkA4` to 0, ticks
the sub-object's (`unkB4`) slot `+0x06C`, dispatches its slot `+0x040`
(capturing the s32 status), and, if that status is non-zero, re-enters
this object's own slot `+0x06C` (`func_8003BCF4`) with a literal 0.

## The C

```c
void func_8003BAB4(StreamTask *self) {
    s32 status;
    func_8003DFBC()->slot4C(self);
    self->unkA4 = 0;
    self->unkB4->methods->slot6C(self->unkB4, self->unkC0);
    status = self->unkB4->methods->slot40(self->unkB4, self->unkB8, self->unkBC, self->unkC4, self->unkC8);
    if (status != 0) {
        self->methods->slot6C(self, 0);
    }
}
```

## How it was found

Same "chain to base, then extra work" shape already established in this
unit's first pass (func_8003BA58, func_8003B9DC). Field order in the C
matches retail's own instruction order exactly (unkA4 reset happens
between the base-class call and the sub-object dispatch, not before or
after as a batch). `classtable.py D_8006E5F8` places this function at
slot `+0x04C`; `classtable.py D_8006E730` confirms the base class's own
unoverridden copy at that slot is `func_8003C238` (not yet matched by
this unit).

## Provenance

round 2026-09-01, runner alpha, unit code_2c054 (second pass).
