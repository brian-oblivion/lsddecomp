# func_80064FBC — STALL (register-store order, best 65/70)

**Unit:** Entity_g · **Size:** 70 instructions · **Attempts:** 5

## Blocker screen

No `gp_rel`/`addiu_at`/`nop_mflo_mfhi` hits.

## What it does

Called by `func_80064E34` (this unit, also stalled) and already known
cross-unit from `Entity_d.c`'s own extern
(`extern void func_80064FBC(Entity *this, EntityMoodHandlerArg *out, s32
arg2, s32 arg3, s32 arg4);`). Sets four `out->` fields when `out->unk4 ==
6`. Tests `this->unkFC` against a cascade of six `arg2`-relative
thresholds (`arg2`, `arg2+0x5B`, `arg2+0x155`, `arg2+0x1B1`, `arg2+0x2BA`,
`arg2+0x317`) that collapse to a single `slot44(this, 0, D_80089D18)` call
when `unkFC` lands in one of three disjoint windows relative to `arg2`
(`[0,0x5B]`, `[0x155,0x1B1]`, `[0x2BA,0x317]`, all offsets from `arg2`);
either way, falls through to `slotC4(this, arg4, 0)`, then `slot160`/
`unk44=1` when `this->unkFC == arg3`.

## The C (best reached, does NOT match -- restored to INCLUDE_ASM)

```c
#if 0
void func_80064FBC(Entity *this, EntityMoodHandlerArg *out, s32 arg2, s32 arg3, s32 arg4) {
    s32 unkFC;

    if (out->unk4 == 6) {
        out->unk10 = 0;
        out->unk1C = 4;
        out->unk30 = 4;
        out->unk44 = 4;
    }
    unkFC = this->unkFC;
    if (unkFC < arg2) {
        goto L18;
    }
    if (!(arg2 + 0x5B < unkFC)) {
        goto L50;
    }
L18:
    if (unkFC < arg2 + 0x155) {
        goto L34;
    }
    if (!(arg2 + 0x1B1 < unkFC)) {
        goto L50;
    }
L34:
    if (unkFC < arg2 + 0x2BA) {
        goto L74;
    }
    if (arg2 + 0x317 < unkFC) {
        goto L74;
    }
L50:
    this->methods->slot44(this, 0, D_80089D18);
L74:
    this->methods->slotC4(this, arg4, 0);
    if (this->unkFC == arg3) {
        this->methods->slot160(this);
        this->unk44 = 1;
    }
}
#endif
```

The literal-`goto` transcription of the six-way range cascade above is
CONFIRMED CORRECT -- every branch target and threshold matches the
disassembly exactly, and the entire body from the first `unkFC` load
onward (word 7 through word 70) matches byte-for-byte. The residue is
confined entirely to the first 6 words of the prologue.

## The residue

Best score 65/70 (with a since-rejected barrier, see attempt 3 below); the
clean form above scores 63/70. Retail moves `arg3` (`$a3`) into its
callee-saved home (`$s1`) as the SECOND real instruction, before even
loading `out->unk4` -- i.e. before it is known whether the `out->unk4==6`
block will run at all. Every build reached here defers that move until
AFTER the `out->unk4==6` branch decision, because `arg3` is not read again
until the function's last statement and nothing in the C obviously demands
committing it to a register that early.

## Attempts (5)

1. Plain transcription as shown above -- 63/70, `move s1,a3` scheduled
   after the branch instead of before.
2. `s32 arg3Cached = arg3;` declared and assigned as the very FIRST
   statement (before the `unk4==6` check) -- no change, still 63/70:
   confirms C declaration ORDER alone does not move where GCC 2.6.3
   commits a parameter to its callee-saved register.
3. A bare `__asm__("");` as the function's first statement -- 65/70, but
   REJECTED: it changed WHICH REGISTER holds `out->unk4` (`$v0` in retail
   becomes `$v1` in this build, or vice versa), which is exactly
   CLAUDE.md's hard-rule-6 test for a banned register-identity change, not
   a permitted order-only barrier. Confirmed by inspecting the diff
   directly rather than trusting the raw score, which is the entire point
   of that rule.
4. The same `arg3Cached` local declared but assigned via a separate
   statement (`s32 arg3Cached; arg3Cached = arg3;`) rather than an
   initializer -- identical 63/70 to attempt 1.
5. Reverted to the clean, minimal form (attempt 1's body) as the one
   preserved here, since attempts 2 and 4 added complexity for zero gain
   and attempt 3's gain is disqualified by the register-identity rule.

## Struct/table knowledge established

None beyond confirming `slot44`, `slotC4`, `slot160`, `unk44`, and
`EntityMoodHandlerArg::unk4/unk10/unk1C/unk30/unk44` (all already known).

### Proposed learning

A parameter moved into its callee-saved home EARLIER than its first C-level
use, with no intervening call that would force a spill, is a genuine
register-identity-ADJACENT residue that neither reordering a local's
declaration nor a scheduling barrier (safely) closes -- the barrier
attempt here demonstrates the ambiguity DIRECTLY: the SAME `__asm__("")`
insertion that moved `arg3`'s home earlier ALSO reassigned an unrelated
value's register, which is the exact signature CLAUDE.md's test is
designed to catch. Treat "retail commits a value early for no visible
reason" as a distinct residue flavour from the already-documented
"prologue store ORDER" class -- that class keeps the same final register
assignments and only reorders the STORES to stack; this one changes WHEN a
value enters a register at all, and forcing it earlier via source tricks
risks exactly the kind of collateral reassignment attempt 3 hit.

## Head-broadcast levers: applicability

- **Lever 1 (negation idiom):** does not apply.
- **Lever 2 (dual-based-type array walkers):** does not apply -- no array
  walk, only scalar comparisons.

## Provenance

round 13 (2026-09-03), runner alpha, unit Entity_g. 5 attempts,
`INCLUDE_ASM` restored.
