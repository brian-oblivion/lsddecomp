# RemapTriggerForDreamColor

> Renamed from `AdjustDreamAuxTriggerOffset` on 2026-09-27 (tools/rename.py). Address 0x8005c930.

> Renamed from `func_8005C930` on 2026-09-21 (tools/rename.py). Address 0x8005c930.

**Unit:** DreamAux · **Size:** 29 words · **Status:** MATCHED round 43
(29/29, byte-exact whole-image build, first attempt).

## History

Filed BLOCKED in round 2026-08-30-a on two `%gp_rel` references (the first to
`gDreamAuxStage`). Round 42 resolved the gp-relative blocker
(`--gp-symbols`/`--no-nop-mflo-mfhi`). Never actually attempted -- the
round-42 stub carried no derivation. Round 43 derived and matched it on the
first build, using the `CheckDreamAuxWorldState` sibling (matched immediately before
this one in the same round) as a template for the `gDreamAuxWorld` vtable-0x80
dispatch idiom.

## What it does

```c
s32 RemapTriggerForDreamColor(s32 a0, s32 a1)
{
    s32 val = sDreamAuxStage;

    if (val == 4 && a1 == 0x10) {
        TriggerWorld *w = (TriggerWorld *)sDreamAuxWorld;
        s32 result = ((TriggerWorldFn80)w->vtable[0x80])(w);

        if (result == val) {
            a0 += 0x1E;
        }
    }
    return a0;
}
```

`a0 + 0x1E` (30) only happens when the unit is in state 4 (`sDreamAuxStage == 4`),
the caller passes `0x10` as `a1`, and a re-check of the same vtable-0x80
predicate used by `IsCurrentDreamColor` still reports state 4. Otherwise `a0` is
returned unchanged. `val` is read once into a local and reused both for the
initial `== 4` test and the post-call re-check (`result == val`), matching
retail's single `lw $s1, %gp_rel(sDreamAuxStage)($gp)` cached across the call --
re-reading the global a second time in C, or comparing against the literal
`4` instead of `val`, would very likely still be correct C but was not
tested since the cached-local reading matched on the first build.

`TriggerWorldFn80` (vtable slot 0x80, self-only, `s32` return) was promoted
from a function-local typedef in `IsCurrentDreamColor`'s first draft to a shared
typedef in `include/DreamAux.h`, since this function needed the identical
one immediately after -- two independent call sites is the point past which
sharing beats duplicating for a same-unit type. `TriggerWorldFn` (the
existing two-argument vtable-0x22 alias) and `TriggerWorldFn80` now sit next
to each other in the header with a comment distinguishing slot/arity.

## Proposed learning

None new -- this is a straight application of `IsCurrentDreamColor`'s
newly-derived `TriggerWorldFn80` idiom, and it matched on the first build.
Worth noting as a *process* point rather than a technical one: solving the
smaller sibling first and immediately re-using its vtable-slot typedef paid
off completely here -- zero iteration needed on the second function.

## Naming

**RemapTriggerForDreamColor** — tier B. Adds `0x1E` (30) to its `a0`
(really an entry pointer smuggled through as `s32`, per
`LookupDreamAuxTrigger`) when the unit is in state 4 AND `a1 == 0x10` AND a
re-check of the world's vtable-0x80 predicate still reports state 4;
otherwise returns `a0` unchanged. The mechanics (conditional fixed offset)
are exactly what the name says; WHY 30, and why this particular re-check
gates it, are not established from this unit alone, hence B not A.

## Track 4 (2026-09-26, round 88, bravo)

The view `*sDreamAuxWorld` is cast to is renamed `DreamAuxWorld` /
`DreamAuxWorldFn80` in include/DreamAux.h (was `TriggerWorld` /
`TriggerWorldFn80`, same `{ void **vtable; }` shape, so the call is
unchanged). The name `TriggerWorld` now belongs to the class gTriggerWorldMethods
(include/TriggerWorld.h), whose table is 0x8C bytes: this call loads byte
+0x200 of its object's table (`lw v0,512(v0)`), so sDreamAuxWorld is not a
TriggerWorld. Its real class is unresolved. Bytes unchanged.

## Round 100 (alpha): track 7, moved from src/world/DreamAux.c and include/DreamAux.h

## Naming (round 100)

**RemapTriggerForDreamColor** (was AdjustDreamAuxTriggerOffset) -- tier B.
The `+0x1E` is five 6-byte DreamAuxTriggerEntries, so on stage 4 the chunk
trigger at index 16 is replaced by index 21 when the player's dream colour
(DreamSys getDreamColor, slot +0x200) is DREAM_COLOR_RED (4). The retry
check "still reports state 4" above was the colour compared against the
cached stage, which happens to be 4 too: `color == DREAM_COLOR_RED` is
byte-identical (tested this round), so the enum is the spelling. Tier B: the
mechanics are exact; why that one trigger has a red variant is not known.
Parameters a0/a1 -> trigger/index, locals val/w/result -> stage/player/color;
the function takes and returns DreamAuxTriggerEntry *. Byte-identical.
