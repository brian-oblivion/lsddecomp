> Renamed from `func_8003E4A4` on 2026-09-19 (tools/rename.py). Address 0x8003e4a4.

# Obj86B60__IncrementFrameCounter — MATCH (5/5 words)

**Unit:** code_2cc8c_c · **Size:** 5 instructions

## What it does

Increments `Obj86B60::unk1C` (already documented). Retail's own delay-slot
fill (`sw` in the `jr $ra` delay slot) happens to leave the incremented
value in `$v0` at return, but that is a side effect of scheduling, not
evidence of a non-`void` return -- no caller was found that reads it, so
this is written as a plain `void` increment. Both a `void` and an
`s32`-returning `return self->unk1C;` form compile identically here (the
value is already live in `$v0` either way); `void` was chosen as the
simpler, unforced reading.

## The C

```c
void Obj86B60__IncrementFrameCounter(Obj86B60 *self)
{
    self->unk1C++;
}
```

## Provenance

round 12 (2026-09-03), runner alpha, unit code_2cc8c_c. Matched on the
first build.
