# func_8003CBC0 — MATCH (27/27 words)

**Unit:** code_2cc8c · **Size:** 27 instructions

## What it does

```c
s32 func_8003CBC0(Obj86B60 *self)
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
own vtable slot `+0x0AC` (per `classtable.py`).

**Addendum after `func_8003C51C` was matched (its only caller in this
unit):** originally typed `(Obj86B60*, s32 a1)`, assuming the caller
forwarded a genuine (if unused) `a1` argument, per CLAUDE.md's "unused
parameter in the callee" idiom. `func_8003C51C`'s own residue proved this
wrong: its `jalr` to this slot sets up ONLY `a0=self`, and `$a1` at that
call site is a caller-saved register left over from an EARLIER, unrelated
call (`slot60(self, 6)`) a few instructions before -- not a value the
source is deliberately forwarding. Retyped to `s32 (*)(Obj86B60*)`
(one argument). This function's OWN compiled bytes are unaffected by the
retype (the parameter was never read in its body either way); the earlier
27/27 match stands unchanged. See `func_8003C51C`'s report for the full
account and the generalized lesson (an argument register surviving an
intervening CALL, not just the next instruction, is not reliably a real
argument).

## Struct knowledge established

- `Obj86B60Methods::slotAC` (+0x0AC) -- IS this function; `s32
  (*)(Obj86B60*)`, one argument (retyped, see addendum above).

## Provenance

round 2026-09-02, runner echo, unit code_2cc8c. 1 attempt (plus a
same-round retype with no rebuild-affecting change, see addendum).
