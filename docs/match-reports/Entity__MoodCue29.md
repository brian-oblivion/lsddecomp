# Entity__MoodCue29

> Renamed from `func_8005F708` on 2026-09-24 (tools/rename.py). Address 0x8005f708.

**Unit:** Entity_c · **Size:** 62 words · **Status:** MATCHED (62/62 words,
whole-image build verified byte-exact)

## What it does

`(Entity *this) -> void`. Not a mood handler (no `EntityMoodHandlerArg *out`
parameter) -- a periodic re-roll: when `unkFC` wraps to 0, optionally fires a
one-shot effect via `Unk94Obj::slot200`'s return value, then always rerolls
`unk44` from `rand() % 5`; separately, whenever `unk44` is 0 it fires
`slot44`:

```c
void Entity__MoodCue29(Entity *this) {
    if (this->unkFC == 0) {
        if (this->unk94->methods->slot200(this->unk94) == 7) {
            this->methods->slot48(this, 1, SCALE_TRIPLE);
            this->methods->slotCC(this, -0x7800, 0);
        }
        this->unk44 = rand() % 5;
    }
    if (this->unk44 == 0) {
        this->methods->slot44(this, 0, ROTATION_YAW_PLUS1);
    }
}
```

Second confirmed caller of `Unk94Methods::slot200` (previously known only
from `Entity__MoodCue00`, compared against the literal `5`; this compares
against `7`, no new signature information). `SCALE_TRIPLE` and `ROTATION_YAW_PLUS1`
are new rodata pointers, extern-declared alongside this unit's other
`D_80089*` constants -- neither yet dereferenced by any carved code.

The `mult`/`mfhi`/`sra`-`subu` sign-fix chain on `rand()`'s result (magic
constant `0x66666667`, single `sra $a0, $a0, 1` after `mfhi`, unlike
`Entity__MoodCue31`'s divisor-10 chain which shifts by 2) reconstructs `5 *
quotient` via a plain `sll 2` + `addu` (no extra `sll 1` doubling step) --
division by 5, so `rand() % 5` reproduces it directly.

## Attempt log

Matched on the first attempt.

## Proposed learning

None new -- confirms the `mult 0x66666667` magic-multiply family
distinguishes divisor 5 (single post-`mfhi` `sra` by 1, single `sll 2`
multiply-back) from divisor 10 (`Entity__MoodCue31`: `sra` by 2, `sll 2` +
`addu` + extra `sll 1` multiply-back) purely by the sign-fix shift amount
and whether the multiply-back chain has that trailing doubling step.

## Naming

Why `MoodCueNN`: the function's address sits in `gEntityMoodHandlerTable` row 29 (`asm/data/79528.data.s`, base 0x80089EB0, stride 0x10; row = (slot address - 0x80089EB0) / 0x10), read directly off the table (this unit's own row assignment, round 78). Tier B: the row-to-function mapping is a compiler fact, not a guess, but which dream object or mood state each row represents is not established -- the row number is kept decimal and zero-padded so the names sort in table order, same convention as Entity_b/d/e/g.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/Entity.h`: a TodActor subclass whose table and object expand `TODACTOR_SLOTS`/`TODACTOR_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
