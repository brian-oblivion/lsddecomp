# Obj86B60__SetFadeCallbackEnabled — MATCH (14/14 words)

> Renamed from `func_8003CAF8` on 2026-09-24 (tools/rename.py). Address 0x8003caf8.

**Unit:** code_2cc8c · **Size:** 14 instructions

## What it does

```c
void Obj86B60__SetFadeCallbackEnabled(Obj86B60 *self, s32 a1)
{
    Obj86B60Methods *methods;

    methods = self->methods;
    switch (a1) {
    case 0:
        self->unk88 = NULL;
        break;
    case 1:
        self->unk88 = methods->slotB0;
        break;
    }
}
```

A setter for the `unk88` callback (see `Obj86B60__TickFadeCallback`, which invokes it):
`a1==0` clears it, `a1==1` sets it to this class's OWN vtable slot `+0xB0`
(`Obj86B60__TickColorFade`) read as a raw function-pointer VALUE (never called through
here), any other `a1` leaves it untouched.

## Residue and fix (2 wasted attempts, then matched)

**Attempt 1 (`if (a1==0) {...} else if (a1==1) {...}`) scored 0/14 with a
totally different instruction sequence** -- GCC compiled the `else if` as a
NEGATED first test (`bnez a1`) feeding into a nested block, which is a
genuinely different branch layout than retail's flat sequential-compare
chain (`beqz a1,CASE0` / `beq a1,1,CASE1` / fallthrough-to-end), and also
came out one word SHORTER (13 vs 14).

**Attempt 2 (`switch (a1) { case 0: ...; case 1: ...; }`) fixed the branch
shape (now the right sequential-compare chain) but still scored 0/14** --
retail loads `self->methods` UNCONDITIONALLY before the dispatch (word 0:
`lw $v1,0x0($a0)`), even though only `case 1` uses it; my `switch` deferred
the `self->methods` load to inside `case 1`, so it never happened on the
`case 0`/default paths and the whole sequence shifted.

**Fix: cache `self->methods` into a local BEFORE the switch, and read the
slot off the local inside `case 1`.** This reproduces retail's
unconditional early load exactly (three attempts total to isolate both
issues; the two residues were independent -- the branch-shape fix and the
early-load fix each moved a different set of words).

### Proposed learning

A `switch`/`if` whose only branch that uses `self->methods` is not the
first one can still need `self->methods` (or any other value) loaded
UNCONDITIONALLY before the dispatch, if retail's own compile hoisted it.
The tell is a load at the very top of the function, before any branch, for
a value only ONE arm actually consumes -- write it as an explicit local
assigned before the `switch`/`if` chain, not read fresh inside the arm that
needs it. This is the same shape as `Obj86B60__OnTag5Notify` (72 insns, matched
separately this round), where `self->methods` is cached into a register the
whole function reuses across a `switch`.

## Struct knowledge established

- `Obj86B60::unk88` (`s32 (*)(Obj86B60*)`, +0x088) -- OBSERVED here as a
  setter target; invoked by `Obj86B60__TickFadeCallback`.
- `Obj86B60Methods::slotB0` (+0x0B0) -- IS `Obj86B60__TickColorFade`; here it is read
  as raw DATA (a function-pointer value), never called through the vtable
  in this unit.

## Provenance

round 2026-09-02, runner echo, unit code_2cc8c. 3 attempts.

## Naming (round 78, delta)

**Tier B.** `func_8003CAF8` -> `Obj86B60__SetFadeCallbackEnabled`. Body:
`switch(a1) { case 0: self->unk88 = NULL; break; case 1: self->unk88 =
methods->slotB0; break; }`. In this unit's evidence, `slotB0` is ALWAYS
`Obj86B60__TickColorFade` (its own IS-occupant, see that report) and nothing
else ever writes `self->unk88`, so "fade callback" is a grounded mechanical
description, not a guess: this is a boolean enable/disable toggle for the
colour-fade tick. Called with a literal boolean-shaped `a1` at every site we
can see (0 or 1), which is why "Enabled" rather than a generic "Set".

## Proposed field names (round 78, delta -- NOT applied, cross-unit)

`Obj86B60::unk88` (`s32 (*)(Obj86B60 *self)`, +0x088) -> `fadeCallback`.
Tier B: in this unit's own evidence the only value ever stored here besides
NULL is `self->methods->slotB0` (`Obj86B60__TickColorFade`), and
`Obj86B60__TickFadeCallback` is its sole invoker. Grep shows `unk88` textual
hits in code_2c054.c/code_179d8_{k,f}.c/code_2cc8c_{d,e}.c/Entity_f.c
(several genuinely this same shared Obj86B60 struct, per code_2cc8c_d/e), so
proposal only -- the head should apply via type scope on `Obj86B60`, not a
whole-tree replace.


**Head disposition, round 78.** `unk88` -> `fadeCallback` APPLIED (type scope, 4 accessors).
