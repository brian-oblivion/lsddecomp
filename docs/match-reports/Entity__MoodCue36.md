# Entity__MoodCue36 -- MATCHED (51/51 words)

> Renamed from `func_8005FDFC` on 2026-09-24 (tools/rename.py). Address 0x8005fdfc.

Unit: `Entity_c`. Runner: bravo.

## Shape

```c
void Entity__MoodCue36(Entity *this) {
    u8 *arg2;
    s32 roll;

    if (this->unk44 == 0) {
        roll = rand();
        arg2 = SCALE_SIX;
        if ((roll & 1) != 0) {
            arg2 = SCALE_DOUBLE;
        }
        this->methods->slot48(this, 1, arg2);
        this->unk44 = 0xB;
    }
    SceneNode__FaceTarget(this, this->unk94, 1, 0, 0);
    if (this->methods->slot144(this, this->unk94) < 0x7000) {
        this->methods->slotC4(this, 0x100, 0);
    }
}
```

## Notes

- **Attempt 1 (stored `arg2 = SCALE_SIX;` BEFORE calling `rand()`) scored
  2/51 with an oversized frame** (retail saves only `$s0`/`$ra` in a 0x20
  frame; the first attempt added a spurious `$s1` save). Cause: with the
  default pointer assigned before `rand()`, that pointer local has to
  survive the `jal rand` call, and GCC promoted it to a callee-saved
  register to do so.
- **Fix: capture `rand()`'s result into a plain `s32 roll` FIRST, then set up
  `arg2` afterward.** This matches retail's actual instruction order (`jal
  rand` first, `lui/addiu` for the default address in the delay slot
  *after* the call returns, `andi $v0,$v0,1` testing the raw return value
  directly, THEN the conditional override) and needs no register to survive
  the call at all -- `roll` lives entirely in `$v0`/caller-saved space
  between the call and its one use. Reordering the two statements (call
  first, pointer setup second) fixed the frame and matched first try after
  the fix.
- `SceneNode__FaceTarget(this, this->unk94, 1, 0, 0)` is the established
  five-argument call shape already used throughout `Entity_b.c`.
- `this->methods->slot144(this, this->unk94)` matches the two-argument
  `slot144` signature already established in `include/Entity.h`
  (`Entity__IsTargetInRange`'s residue).
- Extern added: `SCALE_DOUBLE` (already declared as `u8[]` in `Entity_b.c`;
  this unit needs its own file-scope declaration).
- Clean of both open toolchain blockers.

Matched on the 2nd attempt (2/30).

### Proposed learning

**Statement ORDER around a call decides whether a "default value, then
conditionally overridden" local survives the call in a register at all.**
Assigning the default pointer/value BEFORE the call that also determines the
override condition (e.g. `rand()`) forces that value to live across the
call, and GCC 2.6.3 promotes it to a callee-saved register to do so --
growing the frame relative to retail even though the C is logically
equivalent. Capturing the call's result into a plain scalar FIRST, and only
computing the default/override value AFTER, keeps the value entirely in
caller-saved space and reproduces retail's frame exactly. This is a
companion to the project's established "default value, then conditionally
overwritten" idiom: that idiom's delay-slot-fusion benefit only applies
when nothing between the default assignment and its use can clobber the
value's register -- a call in between is exactly the case where it breaks,
and reordering around the call (not adding a barrier) is the fix.

## Naming

Why `MoodCueNN`: the function's address sits in `gEntityMoodHandlerTable` row 36 (`asm/data/79528.data.s`, base 0x80089EB0, stride 0x10; row = (slot address - 0x80089EB0) / 0x10), read directly off the table (this unit's own row assignment, round 78). Tier B: the row-to-function mapping is a compiler fact, not a guess, but which dream object or mood state each row represents is not established -- the row number is kept decimal and zero-padded so the names sort in table order, same convention as Entity_b/d/e/g.

## Proposed field names

`EntityMethods::slot144` -- this unit's `slot144(this, this->unk94) < 0x7000`
is a bare distance-threshold check, consistent with the existing tier-B
proposal `distanceToRegion` (occupant `Entity__DistanceToPeer`,
`Entity__MoodCue11.md`, Entity_d). Not re-proposed here, just corroborated
with a fourth independent call site.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/Entity.h`: a TodActor subclass whose table and object expand `TODACTOR_SLOTS`/`TODACTOR_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 4b (round 93, charlie) — 2026-09-26

The motion templates are declared once, in `include/Entity.h` (`ROTATION_*`/`SCALE_*` as `Ratio16[]`, `TRANSLATE_*` as `LongVec3[]`); the unit-local `u8[]` externs are gone. The local `arg2`, which holds `SCALE_SIX` or `SCALE_DOUBLE` and is passed to `updateScale`, is now `Ratio16 *` (was `u8 *`). A pointer local's pointee type changes no instruction and the slot takes `void *`, so the bytes held: whole image green, 0 new `-Wall` warnings, nonmatching green.
