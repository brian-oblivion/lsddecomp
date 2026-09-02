# func_8003CAEC — MATCH (3/3 words)

**Unit:** code_2cc8c · **Size:** 3 instructions

## What it does

```c
void func_8003CAEC(Obj86B60 *self, void (*a1)(void *ctx), void *a2)
{
    self->unk9C = a1;
    self->unkA0 = a2;
}
```

A pure setter, establishing `Obj86B60::unk9C`/`unkA0` as an
(unconditional) callback/context pair -- confirmed against
`func_8003CA94`'s use of both (see that report).

## Provenance

round 2026-09-02, runner echo, unit code_2cc8c. 1 attempt.
