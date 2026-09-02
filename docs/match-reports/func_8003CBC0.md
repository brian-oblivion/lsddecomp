# func_8003CBC0 — MATCH (27/27 words)

**Unit:** code_2cc8c · **Size:** 27 instructions

## What it does

```c
s32 func_8003CBC0(Obj86B60 *self, s32 a1)
{
    s32 result;

    result = 1;
    if (self->unk88 != NULL) {
        result = self->unk88(self);
    }
    if (result != 0) {
        self->methods->slot60(self, 5);
    }
    return result;
}
```

The consumer of `self->unk88` (set by `func_8003CAF8`). Exactly the
"default value, then conditionally overwritten" idiom already documented
in DECOMPILATION_LEARNINGS.md: the `beqz`'s delay slot sets `result = 1`
UNCONDITIONALLY, then the taken call overwrites it. This IS this class's
own vtable slot `+0x0AC` (per `classtable.py`); `func_8003C51C` (72 insns,
matched separately this round) calls it as `self->methods->slotAC(self,
a1)`, forwarding a live `a1` that this function's own body never reads --
the parameter exists in the signature purely to match the shared vtable
slot type, per CLAUDE.md's "unused parameter in the callee" idiom.

## Struct knowledge established

- `Obj86B60Methods::slotAC` (+0x0AC) -- IS this function; confirmed
  `s32 (*)(Obj86B60*, s32)` from `func_8003C51C`'s call site.

## Provenance

round 2026-09-02, runner echo, unit code_2cc8c. 1 attempt.
