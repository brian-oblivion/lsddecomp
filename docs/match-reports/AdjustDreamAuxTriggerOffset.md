> Renamed from `func_8005C930` on 2026-09-21 (tools/rename.py). Address 0x8005c930.

# AdjustDreamAuxTriggerOffset

**Unit:** code_4cd08 · **Size:** 29 words · **Status:** MATCHED round 43
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
s32 AdjustDreamAuxTriggerOffset(s32 a0, s32 a1)
{
    s32 val = gDreamAuxStage;

    if (val == 4 && a1 == 0x10) {
        TriggerWorld *w = (TriggerWorld *)gDreamAuxWorld;
        s32 result = ((TriggerWorldFn80)w->vtable[0x80])(w);

        if (result == val) {
            a0 += 0x1E;
        }
    }
    return a0;
}
```

`a0 + 0x1E` (30) only happens when the unit is in state 4 (`gDreamAuxStage == 4`),
the caller passes `0x10` as `a1`, and a re-check of the same vtable-0x80
predicate used by `CheckDreamAuxWorldState` still reports state 4. Otherwise `a0` is
returned unchanged. `val` is read once into a local and reused both for the
initial `== 4` test and the post-call re-check (`result == val`), matching
retail's single `lw $s1, %gp_rel(gDreamAuxStage)($gp)` cached across the call --
re-reading the global a second time in C, or comparing against the literal
`4` instead of `val`, would very likely still be correct C but was not
tested since the cached-local reading matched on the first build.

`TriggerWorldFn80` (vtable slot 0x80, self-only, `s32` return) was promoted
from a function-local typedef in `CheckDreamAuxWorldState`'s first draft to a shared
typedef in `include/code_4cd08.h`, since this function needed the identical
one immediately after -- two independent call sites is the point past which
sharing beats duplicating for a same-unit type. `TriggerWorldFn` (the
existing two-argument vtable-0x22 alias) and `TriggerWorldFn80` now sit next
to each other in the header with a comment distinguishing slot/arity.

## Proposed learning

None new -- this is a straight application of `CheckDreamAuxWorldState`'s
newly-derived `TriggerWorldFn80` idiom, and it matched on the first build.
Worth noting as a *process* point rather than a technical one: solving the
smaller sibling first and immediately re-using its vtable-slot typedef paid
off completely here -- zero iteration needed on the second function.
