# func_8001CCB4

**Unit:** code_d294 · **Size:** 27 words · **Status:** MATCHED (27/27 words)

## What it does

`Class6B5CC` vtable slot `+0x014`, mirror of `func_8001CC48` (`+0x010`).
If `other`'s vtable header tag is `9`, first calls `Class6B5CC__UnlinkModel(self)`
(zeroes `self->unk18`/`self->unk20` -- MEASURED from its own disassembly,
see `include/code_d294.h`), THEN unconditionally forwards to the base
class's own `+0x014` slot (`func_80018390()->slot14`). "Detach" to
`func_8001CC48`'s "attach": the pre-work happens before the base call here,
where `func_8001CC48` did its post-work after.

## The C

```c
void func_8001CCB4(Class6B5CCObj *self, GenericObj_d294 *other) {
    if ((other->methods->header & 0xF) == 9) {
        Class6B5CC__UnlinkModel(self);
    }
    func_80018390()->slot14(self, other);
}
```

## Note on a local that looks conditionally-set but isn't

Retail's own disassembly sets `s0 = a0` (self) in the delay slot of the
`bne` that decides whether to call `Class6B5CC__UnlinkModel` -- i.e. that move
executes on EVERY pass through this code, taken branch or not, because a
MIPS delay slot always executes. Reading it as "only set when the branch
is taken" would be the trap; it isn't, and the C above needs no defensive
restructuring to account for an uninitialized-looking path -- `self` is
simply always live in a register here.

## Provenance

round 11 (2026-09-03), runner charlie, unit code_d294, second pass. Matched on the first build.
