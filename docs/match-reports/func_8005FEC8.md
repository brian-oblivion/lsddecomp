# func_8005FEC8

**Unit:** Entity_c · **Size:** 12 words · **Status:** MATCHED (12/12 words,
whole-image build verified byte-exact)

## What it does

`(Entity *this) -> s32`. A one-line tail-call wrapper: `return
this->methods->slotCC(this, -0x5A, 0);`.

## `slotCC` retyped to `s32`

Same "one-line wrapper" situation as `func_8005FA64`/`slotC4`
(`func_8005FA64.md`, same round) — `slotCC`'s only other known caller
(`func_8005E7F8`, matched the previous round) discards the result, and this
tail call is the first positive evidence either way. UNLIKE `slotC4`,
retyping `slotCC` to `s32` was verified NOT to perturb `func_8005E7F8`'s own
compiled output (isolated `cpp | cc1` recompile of `Entity_b.c`, diffed
line-for-line against the unmodified baseline — zero differences). With no
conflicting evidence, `slotCC` follows the ordinary "one-line wrapper" rule
and is retyped `s32 (*slotCC)(Entity *self, s32 arg1, s32 arg2)`.

## Final C

```c
s32 func_8005FEC8(Entity *this) {
    return this->methods->slotCC(this, -0x5A, 0);
}
```

## Attempt log

Matched on the first attempt.

## Proposed learning

See `func_8005FA64.md` for the general rule this round established: verify
a shared-slot retype against every OTHER known caller's compiled output
before committing to it, since two superficially symmetric slots
(`slotC4`/`slotCC`, same struct, same signature shape) can have opposite
answers.
