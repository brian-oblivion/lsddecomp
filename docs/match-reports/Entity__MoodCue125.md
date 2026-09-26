# Entity__MoodCue125 — MATCH (77/77 words)

> Renamed from `func_8006536C` on 2026-09-24 (tools/rename.py). Address 0x8006536c.

**Unit:** Entity_g · **Size:** 77 instructions

## Blocker screen

No hits.

## What it does

`gEntityMoodHandlerTable` handler row. Dispatches `slot160(this)` + `unk44=1` (ONE
shared call site) when `(this->unkFC==0 && (rand()&3)==0) || this->unkFC
== 0xE10`; then unconditionally `slot48(1,D_80089E44)`, `out->unk10 =
slot148(this)`, the established `out->unk4 % (this->unk80/2) == 0` gate
(setting `out->unk1C=0xA`/`out->unk20=1`), and `slotC4(this,-0xA,0)`.

## The C

```c
void Entity__MoodCue125(Entity *this, EntityMoodHandlerArg *out) {
    if (this->unkFC == 0) {
        if ((rand() & 3) == 0) {
            goto trigger;
        }
    }
    if (this->unkFC != 0xE10) {
        goto merge;
    }
trigger:
    this->methods->slot160(this);
    this->unk44 = 1;
merge:
    this->methods->slot48(this, 1, D_80089E44);
    out->unk10 = this->methods->slot148(this);
    if (out->unk4 % (this->unk80 / 2) == 0) {
        out->unk1C = 0xA;
        out->unk20 = 1;
    }
    this->methods->slotC4(this, -0xA, 0);
}
```

## Residue chased: two paths reaching an identical call, not merged by GCC

First attempt wrote the trigger as two separate `if`/`else if` bodies with
IDENTICAL statements (`slot160(this); this->unk44=1;`) -- 11/77, GCC 2.6.3
did NOT collapse them into one call site even though the bodies were
byte-for-byte the same C, instead compiling a genuine second copy plus an
extra `j` to route around it. Rewriting with `goto` to a single shared
`trigger:` label (mirroring retail's own `L800653B4` reached from two
different branch instructions) closed it to 77/77 on the second build.

## Struct/table knowledge established

None new; corroborates `out->unk4 % (this->unk80/2) == 0` (same idiom as
`Entity__MoodCue09`/`Entity__MoodCue125`'s siblings) now also pairs with
`out->unk20=1`.

### Proposed learning

Do not assume GCC 2.6.3 will cross-jump-merge two SYNTACTICALLY IDENTICAL
call+assignment sequences reached from different branches just because the
BODIES match -- `Entity__MoodCue106`'s report already showed it merges a shared
TRAILING INSTRUCTION SEQUENCE across DIFFERENT calls when written as one
call through a local function-pointer; this function shows the opposite
near-miss: two IDENTICAL statements in two different `if` bodies did NOT
merge automatically. The reliable lever for "one call reached from
multiple guards" is always the same regardless of whether the calls are
identical or different: write ONE physical call site and route to it with
`goto`, matching `DECOMPILATION_LEARNINGS`'s "write the literal jump
graph" guidance -- do not expect the "obviously identical, surely GCC
merges it" shortcut to work.

## Head-broadcast levers: applicability

- **Lever 1 (negation idiom):** does not apply.
- **Lever 2 (dual-based-type array walkers):** does not apply.

## Provenance

round 13 (2026-09-03), runner alpha, unit Entity_g. 2 attempts.


## Naming

Why `MoodCue125`: the function's address sits in `gEntityMoodHandlerTable`
row 125 (base 0x80089EB0, stride 0x10, the row's own `handler` word),
confirmed by reading `disk/SLPS_015.56` directly rather than trusting
address proximity (Entity_d/Entity_e, rounds 76-77, measured that row
order does not track code address). Tier B: the row-to-function mapping is
a compiler fact, not a guess, but which dream state or object each row
represents is not established -- the row number is kept decimal, matching
the existing `MoodCueNN` siblings (Entity_b through Entity_f), so the
names sort in table order.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/Entity.h`: a TodActor subclass whose table and object expand `TODACTOR_SLOTS`/`TODACTOR_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
