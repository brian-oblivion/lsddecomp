# Entity__MoodCue48 -- MATCHED (58/58 words)

> Renamed from `func_80060A4C` on 2026-09-24 (tools/rename.py). Address 0x80060a4c.

Unit: `Entity_d` (fresh carve, round 2026-09-03). Mood-dispatch handler:
`void Entity__MoodCue48(Entity *this, EntityMoodHandlerArg *out)`.

## Final source

```c
void Entity__MoodCue48(Entity *this, EntityMoodHandlerArg *out) {
    s32 a1val;
    EntityMethods *methods;

    if (this->unk84 == 0x26) {
        Class6B5CC__FaceTarget(this, this->unk94, 1, 0, 0);
        out->unk10 = this->methods->slot148(this);
        out->unk1C = 6;
    }
    methods = this->methods;
    a1val = (this->unkFC % 10 < 5) ? -0x1E : 0x1E;
    methods->slotCC(this, a1val, 0);
    this->methods->slotC4(this, -0x1E, 1);
}
```

## Derivation notes

- `Class6B5CC__FaceTarget(this, this->unk94, 1, 0, 0)` reuses the exact call shape
  already documented on that extern's declaration in `include/Entity.h`
  (second arg `this->unk94`, third literal `1`, fourth `0`, fifth `0` on the
  stack) -- same pattern as `Entity__UpdateTargetProximity` elsewhere in the Entity family.
- `this->unkFC % 10 < 5 ? -0x1E : 0x1E` must be written as ONE expression
  (no intermediate `r = this->unkFC % 10;` local) to get the magic-multiply
  divide-by-10 sign-fix chain's registers to land the way retail's does --
  same register-identity trap as `Entity__MoodCue41`'s divide-by-5, confirming
  it's general to this idiom, not a one-off.
- **The final residue, and the only one that needed something beyond
  inlining an expression:** retail loads `this->methods` into a register
  BEFORE the `r<5` ternary's branch, even though the loaded value is only
  used AFTER the branch merges (for the `slotCC` call). A direct
  `this->methods->slotCC(this, a1val, 0);` at the call site left the load
  scheduled AFTER the branch instead. Introducing an explicit `EntityMethods
  *methods = this->methods;` statement positioned BEFORE the ternary --
  even though there is no intervening CALL for it to survive, just a
  conditional -- moved the load early enough to match. The later
  `this->methods->slotC4(...)` call was left as a fresh `this->methods`
  read (matching retail's own fresh reload there), not routed through the
  same `methods` local -- retail genuinely reloads it a second time.

### Proposed learning

- **An independent load can need an explicit early local to get scheduled
  ahead of a BRANCH, not just ahead of a CALL.** Every existing "cache a
  vtable pointer early" entry in DECOMPILATION_LEARNINGS is framed around
  surviving an intervening CALL. Here the hazard was a conditional
  (ternary) with no call in it at all, and the fix was the same shape
  anyway: name the load as its own statement, positioned before the branch,
  even though nothing would clobber it if left inline. Two calls to the
  same vtable slot pointer in one function do not need to share the cached
  local either -- only the one whose retail position is early needs
  hoisting; a later use can stay a fresh reload.

## Naming

`Entity__MoodCue48` -- tier B (round 76, runner delta, FINISHING-PLAN track 3). Same
convention as `Entity__MoodCue00` (round 71): the function's address is the
handler word of `gEntityMoodHandlerTable` (`asm/data/79528.data.s`, base
0x80089EB0, 0x10-byte stride) at row 48, read directly from
`disk/SLPS_015.56` (not inferred from address proximity -- see
`src/Entity_d.c`'s unit header comment, which flags that row order does NOT
track code address once row 115 is reached). Mechanics established
(mood-tick sound-cue-set callback, per `Entity__StartSoundCue`/
`Entity_b.c`'s own header comment); which dream object owns the row is not.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `ENTITY_METHODS`) is unified as `Entity` in `include/Entity.h`: a Class65650 subclass whose table and object expand `CLASS65650_SLOTS`/`CLASS65650_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
