# Entity__NotifyIfTargetInRange — MATCHED (byte-exact, whole-image `build exit=0`)

> Renamed from `func_8005DF9C` on 2026-09-23 (tools/rename.py). Address 0x8005df9c.

Unit: `Entity` · Size: 36 words · Round 23 (2026-09-07), head. Fresh ground
(no prior report).

## The match

```c
/* arg1 is unused here; the canonical declaration in include/Entity.h has it
 * and func_8005DABC passes 0. Do not drop it -- `conflicting types`. */
void Entity__NotifyIfTargetInRange(Entity *this, s32 arg1) {
    if (gEntityLinkStageTable[this->moodIndex * 0x10] < 0 &&
        sEntityEventVideoTable[this->moodIndex * 0x10] != 0 &&
        Entity__IsTargetInRange(this, sEntityEventVideoTable[this->moodIndex * 0x10] << 9)) {
        this->methods->slot30(this, 0xA);
    }
}
```

with, locally in `src/world/Entity.c`:

```c
extern s32 Entity__IsTargetInRange(Entity *this, s32 arg1);
```

Three gates, all branching to the SAME epilogue, so it is one flat `&&` chain
rather than nested `if`s — read off the fact that all three of `bgez`, `beqz`
and the post-call `beqz` target `.L8005E018`.

`sEntityEventVideoTable[...]` is written three times and GCC common-subexpression-eliminates
it to one `lb`; no local is needed and adding one is not what retail did. Both
table reads share the single `sll $a1, $v0, 4` row index, likewise from CSE.

`Entity__IsTargetInRange`'s return is tested with `beqz` straight off the `jal`, so it is
value-returning, not `void`.

## The one thing that cost an attempt: a canonical declaration with an UNUSED parameter

`Entity__NotifyIfTargetInRange` takes **two** arguments. The second is never read — `$a1` is
clobbered by the row index on the first instruction that uses it — so nothing
in this function's own disassembly reveals it exists. Writing the obvious
one-parameter signature is a **`conflicting types` compile error** against the
long-standing `extern void Entity__NotifyIfTargetInRange(Entity *this, s32 arg1);` in
`include/Entity.h`, put there by whoever matched the caller
(`src/world/Entity.c` passes `Entity__NotifyIfTargetInRange(this, 0)`).

**And that error produces ZERO hits on `error:` and `parse error`** — it was
caught only by the `\*\*\* \[[^]]*\.o\]` alternative added to the oracle grep in
round 21. Without it the build would have exited 2 with no diagnostic and
funcdiff would have reported a full match from the previous build (the function
was still `INCLUDE_ASM` then). The staleness guard did also fire, but as the
documented BACKSTOP.

### Proposed learning

**A function's parameter LIST is not fully recoverable from its own body, and
the canonical declaration is the evidence — match it rather than deriving it.**
An argument the callee never reads leaves no trace: the caller passes it, the
callee's disassembly is silent, and the natural signature is wrong. This is the
argument-count sibling of the existing rule that a byte match tells you nothing
about a one-line wrapper's RETURN type.

So before writing a signature, `grep -rn '<func>' include/ src/` and adopt any
declaration you find verbatim. Round 23 hit this twice in one session — here,
and on `func_80013348` where a `const` qualifier the canonical declaration
lacked produced the identical error class — and both times the error was
invisible to the text patterns in the oracle grep. **That makes this a
compile-time argument for keeping `\*\*\* \[[^]]*\.o\]` in the chain**, not a
style preference: the two cheapest signature mistakes available both fail
silently without it.

## Naming

`Entity__NotifyIfTargetInRange` -- tier B (round 71, runner echo, FINISHING-PLAN track 3). Renamed from `func_8005DF9C`.

Called by `Entity__UpdateDeactivationState` (Entity.c) every active tick, with arg1 = 0 (unused). If the row's `gEntityLinkStageTable` byte is negative and its `sEntityEventVideoTable` byte is non-zero, and `Entity__IsTargetInRange(this, eventVideo << 9)` holds, it calls `notifyParents(this, 0xA)`. Tier B: the mechanics are clear. What event 0xA means is not, and both table names are inherited hypotheses (`Entity__GetEventVideo`/`GetLinkStage`), so the name deliberately does not lean on them.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/Entity.h`: a TodActor subclass whose table and object expand `TODACTOR_SLOTS`/`TODACTOR_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 7 (round 94, delta)

Parameter `arg1` renamed `unused` in the definition (tier A: the body never reads it; the one caller, Entity__UpdateDeactivationState, passes 0). Entity.h's prototype still spells it `arg1`; the head may rename it there (a proposal, since Entity.h is shared this round). The comment this replaced said, verbatim in substance: arg1 is unused, the canonical declaration has it, do not drop it or the unit gets `conflicting types` -- that is the section above. `notifyParents(this, 0xA)` is `ENTITY_EFFECT_LINK_STAGE`. The byte-table index `moodIndex * 16` is the tables' 16-byte stride, decimal; Entity.c spells the same stride `0x10`, so a shared name (`ENTITY_MOOD_ROW_STRIDE` in Entity.h) is proposed rather than added.
