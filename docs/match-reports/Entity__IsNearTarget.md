# Entity__IsNearTarget -- MATCHED (58/58 words)

> Renamed from `func_8005D714` on 2026-09-19 (tools/rename.py). Address 0x8005d714.

**Unit:** Entity · Runner: charlie, round 23.

## What it does

Copies a raw 3-word vector (`pos`, pointing directly at x/y/z -- NOT an
`EntityPos*`, see `include/Entity.h`'s own extensive comment on this
function, already written up before this round from its external callers)
onto its own stack, applies a mood-table-driven Y offset, converts `arg3`
into a fixed-point-11 distance value (either `arg3 << 11` or `0x800 /
abs(arg3)` depending on sign), and tail-calls `this->unk94`'s vtable slot
`+0x120` with the scaled vector, `arg2 << 11`, and that distance. This
function was already extensively cross-validated by header comments and
five other units' match reports (`Entity__UpdateTargetProximity`, `Entity__UpdateSoundCueStart`,
`Entity__UpdateActivationState`, `Entity__MoodCue57`, `Entity__UpdateSoundCueStop`, `Entity__MoodCue07`, plus
`Entity_b`/`Entity_d`) that all already call it with `s32`-not-`s8`
parameters and treat its return as a real value compared against 0 -- this
round only had to supply the BODY, the signature was already settled.

## Final C

```c
typedef struct EntityVec3 EntityVec3;
struct EntityVec3 {
    s32 x;
    s32 y;
    s32 z;
};

s32 Entity__IsNearTarget(Entity *this, void *pos, s32 arg2, s32 arg3) {
    EntityVec3 local;
    s32 kind;

    local = *(EntityVec3 *)pos;
    kind = ((u8 *)gEntityUnlockKindTable)[this->moodIndex * 0x10];
    if ((u32)((kind + 9) & 0xFF) < 9) {
        local.y += (s8)kind * 1024;
    }
    if (arg3 < 0) {
        arg3 = 0x800 / (~arg3 + 1);
    } else {
        arg3 <<= 11;
    }
    return this->unk94->methods->slot120(this->unk94, 0, arg2 << 11, &local, arg3);
}
```

Byte-exact, whole-image build verified.

## Attempt log (3 attempts)

1. First attempt used `local = *pos;` cast to a local `EntityVec3`, a
   `-arg3` negation, and a fresh `s32 dist` local holding the branch result,
   passed as the call's 5th argument. Scored 27/58 with 79368 bytes of
   whole-image drift outside the range -- wrong on TWO independent axes at
   once (see below), which is why the raw funcdiff table looked like a
   total mismatch rather than a near miss.
2. **Negation idiom.** `docs/DECOMPILATION_LEARNINGS.md`'s round-13 entry
   (`~x + 1` and `-x` are NOT interchangeable) applied directly: retail's
   `nor $v0,$zero,$a3` / `addiu $v0,$v0,1` is the two-instruction encoding of
   `~x + 1`, not GCC's single-instruction `negu` for a plain `-x`. Switched
   to `~arg3 + 1` for the negative-arg3 case.
3. **The `dist` local itself was the second, bigger problem.** Even after
   fixing the negation, the branch delay slot filled with a *speculatively
   hoisted* `sll v1,arg3,0xb` (the else-arm's cheap shift, computed
   unconditionally before the branch, then overwritten by `mflo v1` on the
   division path) -- a structurally different control-flow shape from
   retail, which keeps the two arms genuinely separate and joins with an
   explicit `j`. Retail's own register choice was the tell: BOTH arms write
   the result back into `$a3` itself (`sll a3,a3,0xb` / `mflo a3`), i.e. the
   parameter register, not a fresh temporary. Rewriting the C to mutate
   `arg3` in place (`arg3 = 0x800 / (~arg3+1);` / `arg3 <<= 11;`) instead of
   assigning a separate `dist` local removed the incentive for GCC to
   pre-materialize a shared temporary, and the branch fell into retail's
   exact two-armed structure (fallthrough = division, explicit `j` over the
   shift arm) on the next build.

One new `Unk94Methods` vtable slot: `slot120` (`self, s32, s32, void*,
s32`), a tail call whose return value is forwarded (`Entity__IsNearTarget` itself
returns `s32`, matching every already-matched caller's `!= 0`/`== 0`
comparison of this function's result).

### Proposed learning

**A local variable that exists ONLY to carry a branch's result into a
single later use is itself a codegen decision, not a style choice.**
Reusing the PARAMETER register in place (`arg3 = ...;` in both arms) versus
introducing a fresh named local for the same value produced two genuinely
different instruction sequences here -- the fresh local let GCC treat the
cheap arm's computation as safe to hoist into the branch's delay slot
unconditionally (changing which arm supplies the delay-slot filler and
adding an extra register), while mutating the parameter in place reproduced
retail's real two-armed, explicitly-joined structure. When retail's two
branch arms both write back into the SAME register that one of the
function's own parameters started in, that register identity is itself
evidence for how the source names the value -- prefer reusing the parameter
over inventing a new local for a single-use branch result.

None of the round-23 head broadcast's three levers apply directly here (no
`s16` locals, no `sltiu`-gated loop, no `&arr[i+j]` shape), though LEVER 1's
underlying principle -- a local's declared identity/width is a codegen
decision, not just documentation -- is exactly the axis this stall turned
on, just for "does this value get its own register" rather than "how wide
is it".

## Naming

**Tier B.** Renamed from `func_8005D714` this round (tools/rename.py).
Every known caller (5+ units, per this report and `Entity.h`) compares its
return against 0, i.e. treats it as a boolean predicate; it applies a mood-
scaled Y offset and a fixed-point-11 distance conversion, then tail-calls
`this->unk94`'s own `slot120`. "Target" is not a fresh guess for
`this->unk94` -- it is `SceneNode__FaceTarget`'s (code_d294.c, a different
unit) OWN established name for dereferencing this exact field, cited
already in `Entity.h`'s `Unk94Obj` comment before this round.

## Proposed field names

- `Entity::unk94` -> `target` -- **tier B.** Same evidence as above
  (`SceneNode__FaceTarget` treats it as its own "target" argument).
  CROSS-UNIT (Entity_b/c/d/e/f/g all dereference `->unk94`, grep -rn --
  '->unk94\b'), so proposed rather than applied.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/Entity.h`: a TodActor subclass whose table and object expand `TODACTOR_SLOTS`/`TODACTOR_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
