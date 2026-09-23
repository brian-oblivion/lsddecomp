# StreamTaskObj__func_8003BAB4

> Renamed from `func_8003BAB4` on 2026-09-23 (tools/rename.py). Address 0x8003bab4.

**Unit:** code_2c054 · **Size:** 42 words · **Status:** MATCHED (42/42)

## Summary

```c
void StreamTaskObj__func_8003BAB4(StreamTaskObj *self) {
    Get_vtable_TaskCore()->slot4C(self);
    self->unkA4 = 0;
    self->unkB4->methods->slot6C(self->unkB4, self->unkC0);
    if (self->unkB4->methods->slot40(self->unkB4, self->unkB8, self->unkBC, self->unkC4, self->unkC8) != 0) {
        self->methods->slot6C(self, 0);
    }
}
```

## Evidence

- `Get_vtable_TaskCore()->slot4C`: `TaskCoreMethods` slot `+0x04C`. `classtable.py
  gTaskCoreMethods` shows it occupied by `TaskCoreObj__func_8003C238`, this unit's own (still
  queued, larger) function — confirms existence/arity, result discarded here
  so typed `void`.
- `self->unkB4->methods->slot6C(self->unkB4, self->unkC0)`: two-argument
  forward on `StreamTaskUnkB4Methods`, return discarded, typed `void`.
- `self->unkB4->methods->slot40(...)`: five-argument call (4 register args
  plus one stack-spilled 5th, `self->unkC8`), **return value tested directly
  by the following `beqz`, never stored anywhere** — typed `s32`.
- `self->methods->slot6C(self, 0)`: reuses the slot established to be
  `StreamTaskObj__SetUnk40`'s occupied slot (`+0x06C` on `gStreamTaskObjMethods`), called only
  when `slot40`'s result is nonzero.
- New field `self->unkA4` (`+0x0A4`, `s32`): reset to 0 unconditionally here;
  a different function (`StreamTaskObj__func_8003BB5C`, this same round) both reads it and
  assigns a call result to it — this function only zeroes it.

## Pitfall hit and corrected

First attempt wrongly assumed the `slot40` call's return was stored into
`self->unkA4` before the test (`self->unkA4 = ...; if (self->unkA4 != 0)`),
by analogy with a structurally similar pattern in `StreamTaskObj__func_8003BB5C` seen while
reading ahead in the same unit. That produced a **41/42-ish residue**: retail
computes the branch's argument-setup (`move a0, s0` — preparing the call
inside the `if`-body) *in the branch's own delay slot*, which only happens
when the value tested is not first materialized into a store. Rereading the
raw disassembly showed there is in fact **no `sw` of the call result at all**
in this function — the `beqz` tests `$v0` straight off the `jalr`. Removing
the spurious assignment and testing the call expression directly fixed both
residue words at once.

## Proposed learning

**Do not backfill a struct-field write into a call site by analogy with a
different, structurally similar function in the same unit — reread that
function's own disassembly line by line first.** Two calls to the same
vtable slot (`slot40` here) can differ in whether the caller *stores* the
return value at all; only one of `StreamTaskObj__func_8003BAB4`/`StreamTaskObj__func_8003BB5C` does. The
tell, from the delay-slot-scheduling angle already documented for `if`-body
argument setup: if retail schedules an unconditional value (like the callee's
`self` argument) into the guarding branch's own delay slot, the source is not
storing the tested value to a struct field — it is testing a live temporary
directly.
