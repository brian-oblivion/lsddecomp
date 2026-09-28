# Entity__MoodCue58

> Renamed from `func_80061400` on 2026-09-24 (tools/rename.py). Address 0x80061400.

**Unit:** Entity · **Size:** 222 words · **Status:** MATCHED (222/222 words)

## What it does

Another mood-dispatch handler (`Entity *this, EntityMoodHandlerArg *out`).
On `this->unkFC==0`, rolls `this->unk94`'s `slot200` against `6` (setting
`unk44=0xB` on a hit) else a `%3` roll (setting `unk44=0xC` on a hit), always
seeding `out->unk10=0`/`out->unk1C=0xC` first. Independently, on
`out->unk4 % 100 == 0`, overwrites `out->unk10`/`unk1C`/`unk20` from
`slot148`. Then three independent top-level `if`s (NOT one `else-if` chain —
see below) drive state-machine transitions on `this->unk44`: `0xB`/`0xC` (each
gated by `slot144(this, this->unk94) < 0x400`, transitioning to `0xD`/`0xE`
respectively and clearing `unkFC`), `0xD` (an `unkFC`-range dispatch into
`unk94`'s `slotCC`/new `slotC8`/`Entity`'s own `slot30`), and `0xE` (an
`unkFC`-range dispatch that either returns early via `slotCC` or, on
`unkFC==0xA`, runs a whole burst of setup calls and sets `unk44=1`).

## Derivation

Read directly off the disassembly (module family already established by
`Entity__MoodCue57`, same file). Verified each `this->unk44 == 0x*` group's
register reload pattern before writing the C: the `0xB`/`0xC` pair shares one
`lw $v1, 0x44($s0)` (confirmed no reload between them in the raw bytes) — so
they're a genuine `if`/`else if`; the `0xD` and `0xE` groups each carry their
own fresh `lw $v1, 0x44($s0)` right before their own compare — so they're
independent top-level `if` statements, not chained. Writing `0xD`/`0xE` as
`else if` off the `0xB`/`0xC` chain would have been a plausible-looking but
wrong CFG (it also wouldn't match the reload count either way once built —
this was checked before writing, not after a failed build).

Two real residues, one of them costly to chase:

### `%N == 0` and `x % N` throughout: bare `%`, no manual magic-constant work

Per the project's established idiom, every modulus in this function
(`out->unk4 % 100`, `rand() % 3`, `this->unkFC % 40`) is written as a plain
`%` expression; GCC 2.6.3 -O2 reproduces retail's magic-multiply expansion
(including the specific shift amounts) on its own. No hand-derivation of the
magic constants was needed or attempted.

### `unk94->methods->slotC8(unk94, ternary, 0)`: one call expression, not two statements

The `0x32 <= unkFC < 0x1F4` arm computes an `a1` value via
`(this->unkFC % 40 < 0x14) ? -5 : 5` and passes it to `unk94`'s `slotC8`.
Splitting this into two statements —

```c
a1val = (this->unkFC % 40 < 0x14) ? -5 : 5;
this->unk94->methods->slotC8(this->unk94, a1val, 0);
```

— scores 2 words short: retail loads `this->unk94` into `$a0` **immediately
after the `%40` remainder is computed** (reusing the register that just went
dead holding `this->unkFC`), *before* the `slti` that decides the ternary,
and loads `unk94->methods` into `$v1` right after that, ahead of the
call. The two-statement form defers both loads to the call site instead — a
different, longer instruction sequence (confirmed via
`tools/asm-differ/diff.py`, not just word-count).

**What did NOT work, tried in order, each verified against a full rebuild
(and separately confirmed in an isolated `cpp|cc1|maspsx|as` reproducer,
CLAUDE.md's "Escalate, do not experiment" pipeline, to iterate without the
full project build):**

1. Hoist `this->unk94` into a local `unk94`, assigned as the block's first
   statement, ternary using the field expression directly:
   `unk94 = this->unk94; a1val = (this->unkFC % 40 < 0x14) ? -5 : 5; unk94->methods->slotC8(unk94, a1val, 0);`
   — the `unk94` load moved to the *earliest* possible point (right after
   the `mult`, before `mfhi`/`sra`/`subu`), into a **fresh** register (`$a2`)
   instead of reusing `$a0`, and `unk94->methods` was still deferred to the
   call site with an extra `move`. Worse (2 extra words), not just
   differently wrong.
2. Add a second local for the remainder itself
   (`rem = this->unkFC % 40; unk94 = this->unk94; a1val = (rem < 0x14) ? -5 : 5; ...`)
   to try to force the `unk94` load between the remainder computation and the
   compare — this fixed the *timing* (unk94 landed in the right slot,
   reusing `$a0`) but reintroduced the register-identity swap from
   `Entity__MoodCue57`'s `%3` residue (`sra $v1`/`mfhi $v0` instead of
   `sra $v0`/`mfhi $v1`, etc.) in the `%40` expansion, because `rem` as a
   named local is exactly the shape that residue is about. Net loss.

**What worked: write it as ONE call expression**, no intermediate locals at
all:

```c
this->unk94->methods->slotC8(this->unk94, (this->unkFC % 40 < 0x14) ? -5 : 5, 0);
```

This reproduces retail exactly. The generalizable read: GCC 2.6.3 evaluates
a call's **receiver expression** (here, `this->unk94`, needed both as the
call's first argument and to resolve `->methods->slotC8`) as soon as its
operand register frees up, *when the whole thing is one expression* — but a
prior, textually-separate statement computing another argument first
(`a1val = ...;`) makes the receiver's load a *fresh*, independent piece of
work item scheduled after that statement completes, landing at the call site
instead of interleaved with the still-live division. The two-statement
version isn't merely "the same code, reordered" to GCC 2.6.3 -- it changes
which instruction-scheduling window the receiver's load competes in.

## Header additions (`include/Entity.h`, additive only)

- `Unk94Methods::slotC8` — new slot at `+0xC8` (before the existing
  `slotCC` at `+0xCC`, immediately adjacent, no arithmetic overlap).
- `Unk70Obj`/`Unk70Sub`/`Unk70SubMethods` — a new two-level object chain
  (`Entity::unk70 -> Unk70Obj::unk4 -> Unk70Sub::methods->slot60`), same
  class-framework idiom as `Unk94Obj`/`Unk100Obj`, one hop deeper. Only the
  one slot this function reaches is named.
- `Entity::unk70` split out of the existing `pad5C[0x80-0x5C]` padding
  (narrowed to `pad5C[0x70-0x5C]` + `pad74[0x80-0x74]`) — a pad split, not a
  retype of any named field.
- Comment-only additions to three EXISTING `Unk94Methods` slot doc-comments
  (`slotCC`, `slot130`, `slot200`) noting this function as an additional
  caller, alongside the callers already on record. No type or name changed
  on any of them.

`extern u8 sRotationZMinus90[];` added to `src/Entity.c` (file-local, same
convention as this unit's other raw data-table externs).

## Proposed learning

- **A call `recv->method(recv, ternary_or_computed_arg, ...)` written as ONE
  expression can compile to different, SHORTER code than the same call
  preceded by a separate statement that pre-computes the argument** — even
  though the two forms are semantically and (at first glance)
  instruction-count identical. When a residue is "callee's receiver/self
  loaded too late, right at the call site" (extra `move`s, or a load
  deferred past an unrelated intervening computation), try collapsing an
  `arg = expr; recv->m(recv, arg, ...);` pair back into
  `recv->m(recv, expr, ...);` before reaching for an intermediate local
  timed to "fix" the load position — the local's presence (not just its
  position) is often the thing perturbing the schedule, per this project's
  established `%3`/`%N` local-vs-inline lesson from `Entity__MoodCue57`, now
  confirmed to generalize past pure `field = expr` assignments to full call
  expressions.
- **Before writing up a "which `if`-chain is it" derivation, check whether
  each branch group reloads the discriminant register from memory.** A
  shared load across two `bne`/`beq` tests against the same value means a
  true `else if`; a fresh `lw` before each group's own first test means
  independent top-level `if`s, even when the groups are textually adjacent
  and superficially look like one chain (four `this->unk44 == 0x*` checks in
  a row, here, are actually two `else if` plus two independent `if`s).

## Naming

`Entity__MoodCue58` -- tier B (round 76, runner delta, FINISHING-PLAN track 3). Same
convention as `Entity__MoodCue00` (round 71): the function's address is the
handler word of `gEntityMoodHandlerTable` (`asm/data/79528.data.s`, base
0x80089EB0, 0x10-byte stride) at row 58, read directly from
`disk/SLPS_015.56`. Mechanics established (mood-tick sound-cue-set
callback); which dream object owns the row is not.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/Entity.h`: a TodActor subclass whose table and object expand `TODACTOR_SLOTS`/`TODACTOR_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 7 (round 93, bravo)

Every literal in the live body is in its base: decimal for moodTimer ticks, distances, TOD frames, VAB programs, volumes and `state` phases (hex remains only for masks). Named: `ENTITY_EFFECT_END_DREAM`, `ENTITY_STATE_DONE`, `DREAM_COLOR_YELLOW` (evidence on each definition: EntityEffect and ENTITY_STATE_DONE in include/Entity.h, SOUND_CUE_STOP in include/SoundCueSet.h). clearTickCallbacks' bool clearLook is `false`; getDreamColor's 6 is DREAM_COLOR_YELLOW (DreamColors, DreamSys.h). Byte-identical (whole image green).
