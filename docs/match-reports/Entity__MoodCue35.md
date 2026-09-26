# Entity__MoodCue35

> Renamed from `func_8005FC58` on 2026-09-24 (tools/rename.py). Address 0x8005fc58.

**Unit:** Entity_c · **Size:** 105 words · **Status:** MATCHED (105/105 words,
whole-image build verified byte-exact)

## What it does

`(Entity *this) -> void`. Not a mood handler. Four independent nudges, each
gated on a different modulus of `unkFC`:

```c
void Entity__MoodCue35(Entity *this) {
    s32 rem500;
    s32 arg1a;
    s32 arg1b;
    s32 arg1c;
    s32 (**slotCC)(Entity *self, s32 arg1, s32 arg2);
    void (**slotC8)(Entity *self, s32 arg1, s32 arg2);
    void (**slotC4)(Entity *self, s32 arg1, s32 arg2);

    rem500 = this->unkFC % 500;

    slotCC = &this->methods->slotCC;
    if (this->unkFC % 6 < 3) {
        arg1a = -0x40;
    } else {
        arg1a = 0x40;
    }
    (*slotCC)(this, arg1a, 0);

    slotC8 = &this->methods->slotC8;
    if (this->unkFC % 12 < 6) {
        arg1b = -0x40;
    } else {
        arg1b = 0x40;
    }
    (*slotC8)(this, arg1b, 0);

    slotC4 = &this->methods->slotC4;
    if (this->unkFC % 64 < 0x20) {
        arg1c = -0x80;
    } else {
        arg1c = 0x80;
    }
    (*slotC4)(this, arg1c, 0);

    if (rem500 < 0x20) {
        this->methods->slotBC(this, TRANSLATE_Y_MINUS64);
    } else if (rem500 < 0x40) {
        this->methods->slotBC(this, D_80089D60);
    }
}
```

New vtable slot: `EntityMethods::slotC8` (`void (*)(Entity*, s32, s32)`),
added to `include/Entity.h` between the existing `slotC4` and `slotCC`, same
`(self, arg1, arg2)` shape as its neighbors. Only one caller so far
(discards the return), kept `void` by default like `slotC4`.

Three of the four gates go through the `mult`/`mfhi`/sign-fix magic-multiply
family: `%6` and `%12` share the same `0x2AAAAAAB` magic constant (only the
post-`mfhi` `sra` shift differs, 0 vs 1, doubling the effective divisor —
same discriminator documented for `Entity__MoodCue29`'s `%5` vs `Entity__MoodCue31`'s
`%10`), and `%500` uses a distinct constant (`0x10624DD3`) kept live in `$s1`
across all three nudge calls since its comparison happens only at the very
end. The fourth, `%64`, is a power of 2 and compiles to the classic
`bgez`-adjust-then-`sra` truncating-division idiom rather than a `mult` --
GCC 2.6.3's ordinary signed-division-by-power-of-2 expansion, not a new
mechanism.

## Attempt log (~9 attempts)

The control-flow shape (four independent gated nudges) was right from
attempt 1 and never changed; every attempt from then on was chasing where
`this->methods` gets loaded relative to the `if`/`else` that computes each
call's `arg1`.

1. **Direct `this->methods->slotCC(this, arg1, 0)` calls, `arg1` computed by
   plain `if`/`else` (48/105, drifted).** Retail loads `this->methods`
   (`lw v0,0(s0)`) in the ONE instruction slot between the modulus
   comparison (`slti`) and the branch (`beqz`) -- i.e. between deciding the
   condition and acting on it. My version loaded it one instruction later,
   right before the call, after the branches had already merged. Same
   registers, same branch targets, just this one load shifted by exactly
   one word per call site (3 call sites -> cascading drift).
2. **Cached `this->methods` into a local `EntityMethods *methods;` assigned
   right before each `if` (13/105, much worse -- register reallocation, not
   just a shift).** Placing the assignment as its own statement gave GCC's
   local scheduler room to hoist the independent load anywhere within the
   whole extended basic block (it has no data dependency on the `mult`
   sequences), and it floated the load past the ENTIRE division chain,
   changing which registers held the division's intermediates entirely --
   worse than attempt 1's clean 1-word shift.
3. **Explicit `mod = this->unkFC % 6;` temp computed first, `methods =
   this->methods;` second, `if (mod < 3)` third (also much worse, register
   reallocation).** Forcing the modulus into its own named temp changed
   which register held the dividend throughout the whole division sequence
   -- a different failure mode than the scheduling drift, and just as bad.
4. **"Default, then conditionally overridden" idiom** (`arg1 = 0x40; if
   (...) arg1 = -0x40;` instead of `if/else`) **produced byte-identical
   output to attempt 1.** This project's existing "default value, then
   conditionally overwritten" learning didn't apply here -- GCC 2.6.3 chose
   the same codegen either way for this particular shape.
5. **Permuter, seeded from attempt 1's body** (`tools/setup-permuter.sh`,
   base score 665) **found a lead at score 360 in under a minute:** for the
   THIRD call only, it introduced `void (**new_var)(Entity*, u16, s32) =
   &this->methods->slotC4;` assigned BEFORE that call's `if`/`else`, then
   called through `(*new_var)(...)`. Taking the ADDRESS of the vtable slot
   (not its value) computes `this->methods` (a real load) plus a constant
   offset (pure arithmetic, no second load) -- exactly splitting the two
   loads retail's disassembly keeps separate: the base-pointer load early,
   the slot-value load adjacent to the call.
6. **Applied the same address-of-slot pattern to all three calls, still
   sharing one `s32 arg1;` across all three (56/105) -- the first call
   (`slotCC`) matched EXACTLY**, but the second (`slotC8`) showed a
   register-identity residue: retail computes that arm's literal directly
   into `$a1`, mine computed it into `$a2` then `move $a1,$a2` (one extra
   instruction). Sharing one `arg1` variable across all three calls,
   despite matching the first, didn't reproduce the second's tighter
   allocation.
7. **Split into three separate locals (`arg1a`/`arg1b`/`arg1c`), one per
   call (105/105) -- matched.** Letting the register allocator treat each
   call's literal independently (rather than reusing one variable across
   three call sites with different live ranges) removed the second call's
   spurious `move`.

## Proposed learning

**Taking `&obj->vtable_ptr->slotNN` (the slot's ADDRESS, not its value) into
a local function-pointer-to-pointer, assigned before an `if`/`else` that
computes the call's other argument, splits a vtable dispatch into its two
constituent loads the way GCC 2.6.3 sometimes does on its own.** A plain
`obj->vtable_ptr->slotNN(...)` call bundles "load the vtable pointer" and
"load the slot from it" into one expression evaluated adjacent to the call;
retail's disassembly here (and in `Entity__MoodCue23`, `Entity__MoodCue35`)
sometimes shows the FIRST of those two loads scheduled well before the
call, at the point right after an unrelated computation finishes and
before a branch that doesn't affect it. A bare local `EntityMethods
*methods = this->methods;` placed earlier does NOT reliably reproduce
this -- GCC's scheduler is free to hoist an ordinary pointer-value
assignment anywhere in the enclosing block (verified: it floated past an
entire `mult`/`mfhi` division chain, attempt 2 above), because nothing
about that local ties it to the offset lookup. Taking the slot's ADDRESS
instead (`&this->methods->slotNN`) encodes both the intended position (the
address computation is inseparable from the value it produces, so it can't
be trivially rescheduled the same way as a bare load can) and the delayed
dereference (the actual slot fetch stays a `(*p)(...)` call, adjacent to
where retail keeps it) in one declaration. **When several such calls
share this pattern, give each its own local for the varying literal
argument** -- one variable shared across multiple call sites can force a
looser, one-size-fits-all register allocation, costing an extra `move` at
whichever call site the allocator treats as secondary (confirmed:
attempt 6's second call site).

The permuter (`tools/setup-permuter.sh`) surfaced the address-of-slot lever
directly; it is worth reaching for on a "same shape, off-by-one-word,
several manual attempts failed" residue like this one rather than
continuing to guess source shapes by hand.

## Naming

Why `MoodCueNN`: the function's address sits in `gEntityMoodHandlerTable` row 35 (`asm/data/79528.data.s`, base 0x80089EB0, stride 0x10; row = (slot address - 0x80089EB0) / 0x10), read directly off the table (this unit's own row assignment, round 78). Tier B: the row-to-function mapping is a compiler fact, not a guess, but which dream object or mood state each row represents is not established -- the row number is kept decimal and zero-padded so the names sort in table order, same convention as Entity_b/d/e/g.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/Entity.h`: a TodActor subclass whose table and object expand `TODACTOR_SLOTS`/`TODACTOR_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
