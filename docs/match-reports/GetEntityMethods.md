# GetEntityMethods

> Renamed from `Get_vtable_Entity` on 2026-09-28 (tools/rename.py). Address 0x8005e150.

**Unit:** Entity · **Size:** 4 words · **Status:** MATCHED (4/4 words,
whole-image build verified byte-exact)

## What it does

Trivial table-pointer accessor: returns `&gEntityMethods`, the same
`EntityMethods` table `New_Entity`/`Entity__Entity` assign to
`this->methods`. Called directly by name (`jal GetEntityMethods`), not
through a vtable slot itself.

## Final C

```c
EntityMethods *GetEntityMethods(void) {
    return &gEntityMethods;
}
```

Required one new declaration in `include/Entity.h`:

```c
extern EntityMethods gEntityMethods;
```

`gEntityMethods` itself stays a raw asm data blob (`asm/data/79528.data.s`,
offsets `0x000`..`0x180`) — only a correctly-typed `extern` was needed here,
per this unit's own convention for still-uncarved data (same as
`gEntityFadeBoxDefaultSize`/`gEntityFadeBoxDefaultOffset` already in this header).

## Attempt log

Matched on the first attempt — exactly as predicted by the sizing note (an
11-line accessor). The address-of-a-global low-half immediate briefly
disagreed with retail (`-0x652c` vs `-0x6528`) while this unit's other
functions were still wrong sizes and hence causing whole-image address drift;
it resolved to byte-exact as soon as every other function this round matched.

## Proposed learning

None beyond what's already documented (`lui`/`addiu` with no surrounding
`lw`/`sw` is `&symbol`, already in `docs/DECOMPILATION_LEARNINGS.md`).

## Naming

`GetEntityMethods` -- tier A (round 71, runner echo, FINISHING-PLAN track 3).

Kept. A getter returning `&gEntityMethods`. `New_Entity` calls it directly by name, and it follows the project's `Get_vtable_X` accessor convention (`GetDreamSysMethods`). Tier A by definition, since the mechanics of a pure getter are its purpose.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/Entity.h`: a TodActor subclass whose table and object expand `TODACTOR_SLOTS`/`TODACTOR_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 7 (round 94, delta)

Nothing to change: no locals, no literals.
