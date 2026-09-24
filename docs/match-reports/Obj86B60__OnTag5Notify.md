# Obj86B60__OnTag5Notify — MATCH (72/72 words)

> Renamed from `func_8003C51C` on 2026-09-24 (tools/rename.py). Address 0x8003c51c.

**Unit:** code_2cc8c · **Size:** 72 instructions -- the second-largest function
in the unit (after the two `addiu_at`-blocked switch dispatchers, see
`Obj86B60__OnTag2Notify`/`Obj86B60__SetState`'s reports).

## What it does

```c
void Obj86B60__OnTag5Notify(Obj86B60 *self, s32 a1, s32 a2)
{
    Obj86B60Methods *methods;

    methods = self->methods;
    Get_vtable_IntermediateBase()->slot5C(self, a1, a2);
    if (self->unk3C != 0) {
        u32 bound;

        bound = self->unk1C;
        if ((u32)self->unk40 < bound) {
            methods->slot60(self, 6);
        }
    }
    switch (self->unk20) {
    case 2:
        methods->slot60(self, 4);
        break;
    case 4:
        methods->slotAC(self);
        break;
    case 7:
        methods->slotC0(self);
        break;
    case 8:
        methods->slot60(self, 3);
        break;
    }
}
```

First forwards to the shared "IntermediateBase" utility class
(`Get_vtable_IntermediateBase()->slot5C(self, a1, a2)`, same idiom already used in
`src/code_2c054.c`/`src/class_39e08.c`). Then, if `unk3C` is set and
`unk40` is (unsigned) less than `unk1C`, calls `slot60` with reason `6`.
Finally switches on `unk20` and forwards to one of four more vtable slots
depending on its value -- two of which (`slotAC`/`slotC0`) ARE this unit's
own `Obj86B60__TickFadeCallback`/`Obj86B60__func_8003CCDC`.

## Residues found, and what closed each

Went through several iterations (this function was initially left
unattempted by mistake this round -- see Provenance -- and picked up again
after the rest of the unit was done, with the benefit of lessons already
learned on smaller functions in the same unit).

**Residue 1: unused `s32 a1` argument on `slotAC`/`slotC0` was WRONG, not
just unused.** First attempt called `methods->slotAC(self, a1)` /
`methods->slotC0(self, a1)`, matching the "unused parameter in the callee"
idiom already used for these two slots elsewhere in the unit. This scored
only 15-24/72 with a totally different frame size (`-0x20`/`-0x28` sp
delta mismatch) -- retail's `jalr` at those two call sites sets up ONLY
`a0=self` in its delay slot, never touching `$a1` at all. Tracing forward
from the earlier `slot60(self, 6)` call (which explicitly sets `$a1=6`) to
these later `jalr`s, nothing between them writes `$a1` again -- but `$a1`
is CALLER-saved and the intervening call (`slot60` itself) is free to
clobber it, so whatever value reaches `slotAC`/`slotC0` is NOT a
meaningful forwarded argument, just physically-whatever's-left-in-the-
register. **Fix: call `slotAC(self)`/`slotC0(self)` with ONE argument**,
and retype the shared vtable slots (and `Obj86B60__TickFadeCallback`'s own definition,
already matched earlier this round) down to `s32 (*)(Obj86B60*)` -- this
does not change `Obj86B60__TickFadeCallback`'s own compiled bytes (it never read the
parameter either way) so the earlier match stays intact.

**Residue 2: `self->methods` must be cached into a local BEFORE the
`Get_vtable_IntermediateBase()` call, or every `self->methods->slotNN` after it reloads
from memory instead of reusing retail's single early `lw $s3,0($s2)`.**
Same lesson as `Obj86B60__SetFadeCallbackEnabled`'s report, but here the stakes are an entire
missing callee-saved register (`s3`) and hence a wrong stack-frame size
(`-0x20` instead of `-0x28`) rather than one extra word -- GCC cannot
prove `self->methods` is unchanged across an opaque call, so without an
explicit local it reloads at every use site after the call, using MORE
total instructions and needing FEWER saved registers, which is why this
residue manifests as widespread structural drift rather than a narrow
diff. Fix: `Obj86B60Methods *methods = self->methods;` before the
`Get_vtable_IntermediateBase()` call, then `methods->slotNN(...)` everywhere after.

**Residue 3: `sltu` (unsigned) where a plain `<` on two `s32` fields gives
`slt` (signed).** `self->unk40 < self->unk1C` compiles to signed `slt`;
retail uses `sltu`. Fix: cast at least one operand,
`(u32)self->unk40 < bound`.

**Residue 4: load order for the two comparison operands.** Retail loads
`unk1C` BEFORE `unk40` (`lw v1,0x1c(s2)` then `lw v0,0x40(s2)`), even
though the comparison is "is `unk40` less than `unk1C`". Plain
`(u32)self->unk40 < self->unk1C` evaluates `unk40` first (matching C's
left-to-right operand evaluation for `<`), producing the wrong load order.
Fix: read `unk1C` into a named local FIRST, then write the comparison
against that local -- `u32 bound = self->unk1C; if ((u32)self->unk40 <
bound)`. This is the exact "mention the dependent quantity first" shape
from DECOMPILATION_LEARNINGS' loop-bound entry, applied here to a plain
comparison rather than a loop bound.

All four fixed together closed the function to 72/72 on the first attempt
after residue 2 (the register-cache fix) was identified; residues 3 and 4
were found and fixed in the SAME pass by reading the remaining diff after
residues 1-2 were already fixed.

## Struct knowledge established

- `Obj86B60Methods::slotAC`/`::slotC0` RETYPED from `(Obj86B60*, s32)` to
  `(Obj86B60*)` -- the `s32 a1` parameter in the original signature was
  never a real argument at this (their only) call site, just a leftover
  caller-saved register value. `Obj86B60__TickFadeCallback`'s own C definition updated
  to match (no effect on its already-matched bytes).
- `Obj86B60::unk20` (s32, +0x020) -- confirmed as a real dispatch/state
  value (previously only known as "set to 5" by `Obj86B60__SetState`, STALL).

### Proposed learning

Two generalizable levers, both already present in the codebase but easy to
under-apply on a function this size:
1. **A shared "cache the vtable pointer" local is not optional once ANY
   call happens between two `self->methods->slotNN` uses** -- it is not
   just a word-count nicety (as it looked in `Obj86B60__SetFadeCallbackEnabled`, a 14-insn
   function) but can cost an entire callee-saved register and a wrong
   frame size on a larger function, which then reads as "everything after
   word 6 differs" rather than a narrow diff. Screen for this FIRST on any
   stalled function whose residue includes a frame-size (`addiu $sp`)
   mismatch.
2. **An argument register that survives an intervening CALL to the callee
   (not just to the next `jalr`) is NOT reliably a real argument** --
   CLAUDE.md's existing "argument register live at next call" test needs
   this addendum: trace forward past every call between the register's
   last known write and the `jalr` in question, not just to the very next
   instruction. A caller-saved register can look "live" across a call
   syntactically while actually holding garbage after it returns.

## Provenance

round 2026-09-02, runner echo, unit code_2cc8c. This function was
overlooked during the main pass through the unit (the runner worked the
smaller queued functions first per the assigned cheapest-first order,
intended to circle back, and the circle-back was initially missed --
caught only by `tools/progress.py` still showing 1 `fresh` function left in
the unit after the rest were matched/stalled). Picked up and matched in a
follow-up pass, ~6 attempts total (3 against the full build, plus the
struct/slot-typing analysis carried over from the initial, pre-drafted
design for this function).

## Naming (round 78, delta)

**Tier A.** `func_8003C51C` -> `Obj86B60__OnTag5Notify`. Occupies slot5C in
`gTaskCoreMethods` and `gClass86B60Methods` identically (only `GraphRoomObj`
overrides this slot, with its own `GraphRoomObj__UpdateFromLog`).
`Obj86B60__OnNotify` dispatches `EventArg`s with `target->header & 0xF == 5`
through `self->methods->slot5C`, same evidence shape as `OnTag2Notify` above.
Tier A.
