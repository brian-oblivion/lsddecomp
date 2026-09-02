# func_8003CBB8 — MATCH (2/2 words)

**Unit:** code_2cc8c · **Size:** 2 instructions

## What it does

```c
void func_8003CBB8(Obj86B60 *self, s32 a1)
{
    self->unk84 = a1;
}
```

The unit's smallest real function (a single `sw`, plus its `jr $ra` delay
slot). Confirms `Obj86B60::unk84` (already established as a multiplier by
`func_8003CC2C`) is a plain setter target, not computed internally.

## Provenance

round 2026-09-02, runner echo, unit code_2cc8c. 1 attempt.
