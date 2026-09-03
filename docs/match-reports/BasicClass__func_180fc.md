# BasicClass__func_180fc

**Unit:** code_8220 · **Size:** 8 instructions · **Status:** MATCHED (8/8 words)

BasicClass vtable slot `+0x020` (`addParentRef`) — see
`BasicClass__BasicClass.md` for the class's overall design, and
`BasicClass__func_17f98.md` (`addChild`) for the caller that dispatches
through this slot on a child object.

## What it does

A one-line wrapper: tail-calls `func_800181AC(&self->parentRefs, parent)`
— pushes `parent` onto `self`'s back-reference list. `$a0` is computed
(`self + 0x8`, i.e. `&self->parentRefs`) but `$a1` (the value to push) is
never touched in this function's own body, forwarded unchanged from the
caller.

## The C

```c
s32 BasicClass__func_180fc(BasicClass *self, BasicClass *parent)
{
    return func_800181AC(&self->parentRefs, parent);
}
```

## Why `s32`, not `void`

Per the one-line-wrapper rule: `func_800181AC` (matched this round, see
its own report) is a real `s32`-returning function (1 on successful push,
0 on pool-allocation failure), and nothing in this function's own body
overwrites `$v0` after the call — a genuine tail call, so `return` is
correct.

## Provenance

round 11 (2026-09-03), runner delta, unit code_8220, second pass. Matched
first attempt.
