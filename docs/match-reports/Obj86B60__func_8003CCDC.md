# Obj86B60__func_8003CCDC — MATCHED (27/27 words)

> Renamed from `func_8003CCDC` on 2026-09-24 (tools/rename.py). Address 0x8003ccdc.

**Unit:** code_2cc8c · **Size:** 27 instructions

> **UPDATE (targeted permuter pass, round 17).** MATCHED. The lever was a
> `goto` to a single shared epilogue instead of an early `return` inside the
> `if (result == 0)` block — reading the disassembly closely enough to
> notice WHERE `move v0,s1` actually lives (the delay slot of the very
> `beqz $s1,...` branch that tests `result == 0`, filled for free because
> it's independent and harmless on both paths) is what suggested it: retail
> reaches its return through the SAME physical code as the `slot60` path,
> not through a private early-exit that duplicates the return sequence.
> Once both paths funnel through one `return result;` reached via `goto`,
> GCC 2.6.3 stops being able to prove `result == 0` is a per-branch constant
> worth substituting with `li v0,0` and reuses the register instead, exactly
> like retail. No permuter run was needed this round — the fix came directly
> from reading the delay-slot placement — but see "Permuter attempt" below,
> which stands as the record of why a blind reshape search didn't find this
> (it never explores a `goto`, and this stall's own conclusion said as much:
> *"seed the permuter with a DIFFERENT base shape... e.g. a `goto`-based
> rewrite"*).

```c
s32 Obj86B60__func_8003CCDC(Obj86B60 *self)
{
    s32 result;

    result = 1;
    if (self->unk8C != NULL) {
        result = self->unk8C(self);
        if (result == 0) {
            goto epilogue;
        }
    }
    self->methods->slot60(self, 8);
epilogue:
    return result;
}
```

## Signature update -- TRIED, did not move the residue

Originally attempted as `s32 Obj86B60__func_8003CCDC(Obj86B60 *self, s32 a1)`, an
unused-but-forwarded parameter matching `Obj86B60__TickFadeCallback`'s ORIGINAL
signature. `Obj86B60__OnTag5Notify`'s own residue (matched separately, see its
report) proved this class of assumption wrong for its sibling slot
`slotAC`/`Obj86B60__TickFadeCallback`: the `s32 a1` at that call site was never a real
argument, just a leftover caller-saved register value from an earlier,
unrelated call. `Obj86B60Methods::slotC0` (this function's own vtable
slot, `+0x0C0`) was retyped to `s32 (*)(Obj86B60*)` (one argument) as part
of matching `Obj86B60__OnTag5Notify`.

**Re-attempted this function itself with the corrected one-argument
signature** (`s32 Obj86B60__func_8003CCDC(Obj86B60 *self)`, body otherwise
unchanged) -- **still 26/27 at the time, IDENTICAL residue** (`move v0,s1`
in retail vs. `li v0,0x0` here, same single instruction). This makes sense
in hindsight: the dropped parameter was never read inside this function's
OWN body either way. The one-argument signature IS the correct, final one
-- it's what's used in the matched body above -- this section is kept for
the record of what did NOT move the residue.

## Permuter attempt -- TRIED against the pre-`goto` shape, no zero found

Ran `tools/setup-permuter.sh` with the 26/27 `return`-based body (see
"Superseded 26/27 body" below) as seed (base score confirmed 5 via
`--debug` -- 0 insertions, 0 deletions, 1 register difference, the
"redundant move" class MATCHING-GUIDE flags as the permuter's best-posed
target). Two runs, ~90s and ~120s, roughly 24,000 combined iterations at
`-j 6`, `--stop-on-zero`. **Best score never dropped below 5 in either
run; zero was never reached.** The permuter's statement/expression
mutations never introduce a `goto`/shared-epilogue restructuring from an
`if`/early-`return` seed, so it could not reach the shape that actually
worked -- confirming this report's own prior conclusion that a different
BASE SHAPE, not more search on this one, was needed.

## What it does

The `self->unk8C` counterpart to `Obj86B60__TickFadeCallback`'s `self->unk88` (both set
by `Obj86B60__func_8003CB30`/`Obj86B60__SetFadeCallbackEnabled` respectively). UNLIKE `Obj86B60__TickFadeCallback`,
this function's control flow has a genuine extra early-exit: if the
callback returns 0, retail branches DIRECTLY to the shared epilogue,
skipping the `slot60` call entirely, rather than reaching the same skip via
a single unified `if (result != 0)` test the way `Obj86B60__TickFadeCallback` does.

## Progression

**Attempt 1: `Obj86B60__TickFadeCallback`'s exact shape** (`result = 1; if (unk8C) result
= unk8C(self); if (result != 0) slot60(...); return result;`) scored
**23/27**, four words off, with a DIFFERENT branch immediate at the first
`beqz` and three more knock-on differences -- confirmed this function's CFG
genuinely differs from `Obj86B60__TickFadeCallback`'s (per DECOMPILATION_LEARNINGS'
"two structurally-similar residues want different C shapes" entry), not
just that the same C compiles slightly differently.

**Attempt 2: explicit early `return` inside the `if` block** scored
**26/27** -- every branch target, every register, every instruction OPCODE
matched retail, except ONE: at the early-return site, retail computes the
return value with `addu $v0,$s1,$zero` (`move v0,s1` -- copy the
already-known-zero register) where a plain `return result;` there compiled
to `li $v0,0x0` (load the immediate 0 directly). Three further isolated
rewrites (a negated `if (!result)`, hoisting the zero-check outside the
`unk8C != NULL` guard, splitting the callback's return into its own `ret`
local) all failed to reproduce `move v0,s1` -- see "Superseded" section.

**Attempt 3 (this round): `goto` to a single shared epilogue** instead of
`return result;` inside the `if` block -- **27/27, byte-exact.** The
insight came from reading exactly WHERE in retail's instruction stream
`move v0,s1` sits: it's the delay slot of `beqz $s1,.L8003CD30` itself (the
very branch that implements `if (result == 0)`), not a separate
instruction sequence for a distinct early-return code path. Retail's
compiled epilogue is ONE physical block (`lw ra / lw s1 / lw s0 / addiu sp
/ jr ra`) reached from TWO places: the early branch (with `v0` already
carrying `s1`'s value via the free delay slot) and the fallthrough after
`slot60` (with `v0 = s1` set explicitly at `2D52C`). A `return` written
inside the nested `if` gives GCC a private, disposable exit that its
constant-propagation pass is free to specialize (substituting the
known-constant `0` for `result`); a `goto` to a label the OTHER path also
reaches removes that freedom, because now the SAME return statement has to
serve a call site where `result` is NOT known to be a compile-time
constant.

## Superseded 26/27 body (kept for the record -- do not resurrect)

```c
#if 0
/* SUPERSEDED -- see the goto-based 27/27 body at the top of this report.
 * Kept only as the permuter's seed record; the `return result;` inside the
 * nested `if` is exactly what let GCC constant-fold this to `li v0,0`
 * instead of retail's `move v0,s1`. */
s32 Obj86B60__func_8003CCDC(Obj86B60 *self)
{
    s32 result;

    result = 1;
    if (self->unk8C != NULL) {
        result = self->unk8C(self);
        if (result == 0) {
            return result;
        }
    }
    self->methods->slot60(self, 8);
    return result;
}
#endif
```

### Proposed learning

**When a residue is "GCC substitutes a known constant instead of reusing a
register that already holds it," check whether retail's instruction lives
in a BRANCH DELAY SLOT shared between two control-flow paths before
assuming it's a value-materialization stall.** A private `return` inside a
nested `if` gives the compiler a disposable, single-use exit it is free to
specialize with constant propagation; routing the same value through a
`goto` to a label ANOTHER path also reaches removes that freedom, because
the return statement now has to work for a caller where the value isn't
constant. This is a genuinely different lever from the ones
DECOMPILATION_LEARNINGS already documents (redundant `__asm__("")`
barriers, `volatile` reads, declaration reordering) — none of which touch
constant-propagation scope — and the permuter's random statement/expression
mutations never reach it from an `if`/`return` seed, so a stall in this
exact shape (single register, a constant vs. a register-copy) is worth a
manual `goto` rewrite before it's accepted as permuter-exhausted.

`Obj86B60__TickFadeCallback`/`Obj86B60__func_8003CCDC` remain the same idiom (a callback stored by
a matching setter, defaulting to 1, invoked and conditionally followed by a
`slot60` reason-code call) at two different offsets in the same class,
needing genuinely different C shapes despite that.

## Provenance

Originally: round 2026-09-02, runner echo, unit code_2cc8c. 11 attempts (4
against the real build across two sessions, 5 in an isolated reproducer,
plus a ~24k-iteration permuter search that found no improvement) — filed as
a stall at 26/27.

Closed: round 17, targeted permuter pass, runner charlie. Matched via a
manual `goto` restructuring after reading the delay-slot placement in the
target disassembly; no fresh permuter run was needed once the base shape
changed.

## Naming (round 78, delta)

**Tier C.** `func_8003CCDC` -> `Obj86B60__func_8003CCDC`. Same
"invoke-optional-callback, transition on nonzero" shape as
`Obj86B60__TickFadeCallback` above, but for the `unk8C`/`slotC4` pair
(`func_8003CD48`, external, unread here -- see `Obj86B60__func_8003CB30`), so
it is not established as a "fade" tick either; transitions to state 8
instead of 5 on completion. Kept tier C for the same reason as `func_8003CB30`.
