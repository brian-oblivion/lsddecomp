# Entity__MoodCue80 -- MATCHED (44/44 words)

> Renamed from `func_80063094` on 2026-09-24 (tools/rename.py). Address 0x80063094.

Unit: `Entity` (round 12). A three-way branch on `this->unkFC` vs
`this->unk80`, with a nested (and, on the surface, logically redundant)
double-guard on `this->unk84` in one arm.
`void Entity__MoodCue80(Entity *this, EntityMoodHandlerArg *out)`.

## Final source

```c
void Entity__MoodCue80(Entity *this, EntityMoodHandlerArg *out) {
    if (this->unkFC < this->unk80) {
        if (this->unk84 != 0) {
            if (this->unk84 == 0x14) {
                out->unk10 = 0;
                out->unk1C = 0x10;
            }
        }
    } else {
        this->methods->slot130(this);
        this->methods->slotBC(this, sTranslateYMinus512);
    }
    SceneNode__FaceTarget(this, this->unk94, 1, 0, 0);
}
```

## Derivation notes

`SceneNode__FaceTarget`'s call shape and `slot130`/`slotBC` were all already
established. The interesting part is the doubled guard on `this->unk84`:
the disassembly has `beqz $v1, L80063118` (skip to the "do nothing extra"
merge point when `unk84 == 0`) immediately followed by a *second*,
independent check `bne $v1, 0x14, L8006311C` (skip the write when `unk84 !=
0x14`). Written as a single `if (this->unk84 == 0x14) { ... }`, this
compiles to *only* the second comparison -- since `0 != 0x14` anyway, the
first check is behaviorally dead code from a pure input/output standpoint.
That single-`if` version came out exactly 2 words (8 bytes) short of
retail's 0xB0, confirmed by comparing `nm` addresses of the surrounding
`INCLUDE_ASM` symbols in the built ELF against their expected retail
addresses (`Entity__MoodCue81`, the next symbol, landed 8 bytes early).

Restoring the outer `if (this->unk84 != 0)` guard around the `== 0x14`
check reproduced retail's exact two-comparison shape and closed the gap.
This reads as `switch`-like source that a human (or an earlier, unoptimized
compilation pass) wrote as nested guards rather than a single combined
test -- plausible given this unit's other mood handlers use a similar
"early-exit on a sentinel value before checking the real case" shape (see
`Entity__MoodCue39` in `entity.c`'s `goto skip48` pattern for a related
early-exit idiom in a sibling handler).

### Proposed learning

**A same-value-but-fewer-comparisons `if` is not free -- GCC does not
automatically eliminate a syntactically dead comparison the source
actually wrote.** When a function's word count comes up short by exactly
the size of one branch instruction (2 words = one branch + delay slot) and
the disassembly shows what looks like a logically-redundant extra
comparison ahead of the "real" one (e.g. an explicit `!= 0` guard
immediately before an `== N` check, where `N != 0` makes the first check
mathematically unnecessary), that redundancy is very likely present in the
retail source as an explicit, separate `if` -- collapsing it into a single
combined condition, however logically equivalent, removes an instruction
retail actually emitted. Cross-check candidate mismatches against `nm`
addresses of the unit's *other*, still-`INCLUDE_ASM` symbols (their
expected vs. built addresses reveal the exact byte delta and which
function introduced it) rather than trusting funcdiff's own per-function
window once a drift warning is present.

## Naming

Why `MoodCueNN`: the function's address sits in `gEntityMoodHandlerTable` row 80 (`asm/data/79528.data.s`, base 0x80089EB0, stride 0x10; row = (slot address - 0x80089EB0) / 0x10), confirmed by reading `disk/SLPS_015.56` directly rather than trusting address proximity (Entity/round 76 measured that row order does not track code address). Tier B: the row-to-function mapping is a compiler fact, not a guess, but which dream object or mood state each row represents is not established -- the row number is kept decimal and zero-padded so the names sort in table order, same convention as Entity.

Reading this function's `out->` writes with the proposed `SoundCueSet` field names (`Entity__MoodCue07.md` `## Proposed field names`, tier A/B, proposal only -- `EntityMoodHandlerArg` is shared with Entity/Entity_d/Entity_g): `attenuation`, `voice0Tone`.

**Data constant renamed this round:** `D_80089D54` -> `sTranslateYMinus512`, tier B. 32-bit value at the Y slot (offset +4) is `0xfffffe00` = -512, matching the `sTranslateYMinus64`/`sTranslateYPlus256` s32-triple format confirmed in `Entity__MoodCue68`'s report.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/entity.h`: a TodActor subclass whose table and object expand `TODACTOR_SLOTS`/`TODACTOR_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 7 (round 93, echo)

Every literal in the live body is in its base: decimal for moodTimer and cue-set ticks, TOD frames, distances, VAB programs and `state` phases (the unit has no hex literal left). Byte-identical (whole image green).
