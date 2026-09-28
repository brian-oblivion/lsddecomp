# Entity__StepYawInWindowsThenDeactivate — MATCHED (was misdiagnosed as register-store-order; real fix was a wrong conditional grouping)

> Renamed from `Entity__AdvanceWobbleAndDeactivate` on 2026-09-24 (tools/rename.py). Address 0x80064fbc.

> Renamed from `func_80064FBC` on 2026-09-24 (tools/rename.py). Address 0x80064fbc.

**Unit:** Entity · **Size:** 70 instructions · **Attempts:** 5

## Blocker screen

No `gp_rel`/`addiu_at`/`nop_mflo_mfhi` hits.

## What it does

Called by `Entity__MoodCue111` (this unit, also stalled) and already known
cross-unit from `Entity.c`'s own extern
(`extern void Entity__StepYawInWindowsThenDeactivate(Entity *this, EntityMoodHandlerArg *out, s32
arg2, s32 arg3, s32 arg4);`). Sets four `out->` fields when `out->unk4 ==
6`. Tests `this->unkFC` against a cascade of six `arg2`-relative
thresholds (`arg2`, `arg2+0x5B`, `arg2+0x155`, `arg2+0x1B1`, `arg2+0x2BA`,
`arg2+0x317`) that collapse to a single `slot44(this, 0, ROTATION_YAW_PLUS1)` call
when `unkFC` lands in one of three disjoint windows relative to `arg2`
(`[0,0x5B]`, `[0x155,0x1B1]`, `[0x2BA,0x317]`, all offsets from `arg2`);
either way, falls through to `slotC4(this, arg4, 0)`, then `slot160`/
`unk44=1` when `this->unkFC == arg3`.

## The C (best reached, does NOT match -- restored to INCLUDE_ASM)

```c
#if 0
void Entity__StepYawInWindowsThenDeactivate(Entity *this, EntityMoodHandlerArg *out, s32 arg2, s32 arg3, s32 arg4) {
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
    this->methods->slot44(this, 0, ROTATION_YAW_PLUS1);
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

round 13 (2026-09-03), runner alpha, unit Entity. 5 attempts,
`INCLUDE_ASM` restored.

## MATCHED -- round: permuter pass (runner delta)

**The 5-attempt manual round's diagnosis was wrong about the residue's
CAUSE.** It read this as "retail moves `arg3` into `$s1` earlier than any
C form naturally demands" (a register-commit-timing residue). The
permuter (`-j 6 --stop-on-zero --best-only`, zero reached at iteration
6810 of an unbounded-until-zero run) found the actual fix in under a
minute: **`out->unk10 = 0;` is NOT gated by `out->unk4 == 6` -- it runs
UNCONDITIONALLY, every call, before that check.** Every manual attempt
had it nested inside the `if`, alongside `unk1C`/`unk30`/`unk44` (which
genuinely ARE conditional on `unk4 == 6`). Moving just that one store
outside the `if` is the entire fix -- no barrier, no local, no reordering
of anything else.

```c
void Entity__StepYawInWindowsThenDeactivate(Entity *this, EntityMoodHandlerArg *out, s32 arg2, s32 arg3, s32 arg4) {
    s32 unkFC;

    out->unk10 = 0;
    if (out->unk4 == 6) {
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
    this->methods->slot44(this, 0, ROTATION_YAW_PLUS1);
L74:
    this->methods->slotC4(this, arg4, 0);
    if (this->unkFC == arg3) {
        this->methods->slot160(this);
        this->unk44 = 1;
    }
}
```

Verified byte-exact: `./build-and-verify.sh` -- `OK: build matches retail
SLPS_015.56` -- and `tools/funcdiff.py Entity__StepYawInWindowsThenDeactivate` -- `70/70 words
match`. This is now the live body in `src/Entity.c` (`INCLUDE_ASM`
removed).

### New struct knowledge

`EntityMoodHandlerArg::unk10` is reset to 0 on EVERY call into this
handler shape, not just when `unk4 == 6` -- i.e. it is this function's own
"clear the field before any conditional overwrite" idiom, not part of the
`unk4==6` bundle. Worth checking whether sibling mood-handler functions in
this unit/family (`Entity.c`, `Entity_e.c`, `Entity_g.c`) that also
touch `out->unk10` make the same unconditional-vs-conditional mistake if
their own near-miss reports show a similar unexplained residue.

### Proposed learning

**A register/scheduling-shaped residue with no obvious source lever is
worth checking for a wrong CONDITIONAL GROUPING before reaching for a
barrier or a local.** This round's manual attempts (1-5, preserved above)
never questioned whether all four of the `unk4==6` block's stores
actually belonged together in the source -- they treated the block as
verified-correct because the VALUES and the vtable/vslot identities were
right, and hunted for a scheduling explanation for the one register that
wouldn't line up. The permuter doesn't have that bias: it mutates
statement placement freely, and this is a second (after `Entity__MoodCue81`'s
divergence #1 this same round) case of it finding a real logic-grouping
fix inside a body whose control flow and field IDENTITIES were already
confirmed correct. When several C-level reshapes of a small block all
converge on the identical residue, that convergence is evidence the levers
tried are the wrong axis -- not evidence the axis (conditional grouping,
statement scope) has been exhausted.

## Naming

Not a `gEntityMoodHandlerTable` row (no row's `handler` word is
0x80064FBC; confirmed by scanning the whole table against every row, same
method used for the row functions in this unit). It is a shared per-tick
helper called directly (`jal`, not through any vtable) by two different
row handlers: `Entity__MoodCue111` (this unit, twice, with different
`arg2`/`arg3`/`arg4`) and `Entity__MoodCue40` (`Entity.c`, cross-unit,
one call site). Tier B: the mechanics are fully established from the body
-- up to three periodic "wobble" windows relative to `arg2`
(`[arg2,arg2+0x5B]`, `[arg2+0x155,arg2+0x1B1]`, `[arg2+0x2BA,arg2+0x317]`)
trigger a single `updateRotation(this, 0, ROTATION_YAW_PLUS1)` pulse, then
every call unconditionally applies `slotC4(this, arg4, 0)` (a continuous
decay/approach call used identically by many of this unit's own row
handlers) and deactivates + sets `moodState = 1` once `moodTimer == arg3`
-- but why two unrelated mood cues (row 40 and row 111) share exactly this
timed-wobble-then-deactivate shape, or what the wobble represents in the
game, is not established. Named `AdvanceWobbleAndDeactivate` (free-function
`VerbNoun`-adjacent form, `Entity__` prefix kept because it operates
directly on `Entity::moodTimer`/`Entity::moodState` the same way
`Entity__GetOrCreateFadeBox`/`Entity__IsTargetInRange` do) rather than the
tier-C `Entity__func_80064FBC` form, because the mechanics description
above is concrete, not a placeholder.

## Data constant decoded this round

`ROTATION_YAW_PLUS1` (0x80089D18), the wobble-pulse `updateRotation`
argument, decoded from `disk/SLPS_015.56` as four s16 `{num,den}` pairs:
`(0,1, 1,1, 0,1, 0,1)` -- only Y (yaw) nonzero, a whole 1/1 = 1 degree,
matching the existing `ROTATION_YAW_PLUS2` precedent for small whole-degree
per-tick amounts.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/Entity.h`: a TodActor subclass whose table and object expand `TODACTOR_SLOTS`/`TODACTOR_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 7 (2026-09-26, round 94, alpha)

### The goto cascade is three windows

The literal-`goto` transcription (labels `L18`/`L34`/`L50`/`L74`) is now
one `if` over three inclusive windows,
`[windowStart, +91]`, `[+341, +433]`, `[+698, +791]`, that call
`updateRotation(ROTATION_YAW_PLUS1)`. Byte-exact (funcdiff 70/70, whole
image green). The parameters were renamed `windowStart`/`deactivateTimer`/
`zStep` earlier this round.
