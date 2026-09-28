# Entity__MoodCue40 -- MATCHED (89/89 words)

> Renamed from `func_80060148` on 2026-09-24 (tools/rename.py). Address 0x80060148.

Unit: `Entity_d` (second pass, round 2026-09-03). Mood-dispatch handler:
`void Entity__MoodCue40(Entity *this, EntityMoodHandlerArg *out)`.

## Final source

```c
void Entity__MoodCue40(Entity *this, EntityMoodHandlerArg *out) {
    void *table = NULL;

    if (out->unk4 % 7 == 0) {
        out->unk10 = this->methods->slot148(this);
        out->unk1C = 3;
        out->unk24 = 0x40;
        out->unk28 = 0x40;
    }
    if (this->unkFC == 0xC8) {
        table = ROTATION_YAW_MINUS90;
    } else if (this->unkFC == 0x190) {
        table = ROTATION_YAW_PLUS180;
    } else if (this->unkFC == 0x258) {
        table = ROTATION_YAW_PLUS90;
    } else if (this->unkFC == 0x320) {
        table = ROTATION_YAW_PLUS180;
        this->unkFC = -1;
    }
    if (table != NULL) {
        this->methods->slot44(this, 0, table);
    }
    this->methods->slotD0(this, -0x1E, 0);
    if (this->unk28 != 0) {
        this->methods->slotCC(this, -0xC8, 0);
    }
}
```

## Derivation notes

Three separate residues stacked on this one, each needing its own lever --
the score did not move between fixing #1 and #2, which only became visible
after fixing #3 (per DECOMPILATION_LEARNINGS' "one instruction short
describes the score, not the defect count").

- **New fields on `EntityMoodHandlerArg`: `unk24`/`unk28`** (both `s32`),
  carved out of the existing `pad24[0x0C]` gap between `unk20` and `unk30`.
  Straightforward, no complications.
- **Residue #1 (the size-drift one): `table`'s initializer must be at the
  function's top-level declaration, BEFORE the `out->unk4 % 7` guarded
  block, not after it.** Retail initializes the table pointer to 0 in the
  delay slot of the mod-7 check's branch -- i.e. BEFORE the `slot148`
  vtable call that follows when the mod-7 condition is true -- which forces
  it to survive that call in a callee-saved register (`s2`). Declaring
  `table` and setting it to `NULL` only after the mod-7 block let the
  compiler keep it in a caller-saved register instead, producing a function
  one register-pair (8 bytes: no `sw`/`lw s2`) SHORTER than retail and
  cascading ~125KB of drift into every later function in the unit. Moving
  the `= NULL` into the declaration itself (`void *table = NULL;` as the
  first statement) fixed this in one change, dropping the drift to ~79KB
  (the size of the two remaining residues below).
- **Residue #2: two branches assign the textually IDENTICAL expression
  `table = ROTATION_YAW_PLUS180;`** (the `unkFC == 0x190` and `unkFC == 0x320`
  cases). GCC's cross-jump pass tail-merged them into ONE shared code block
  reached from both branches, which retail's bytes do NOT do -- retail
  keeps two separate, byte-identical `lui`/`addiu` pairs, one per branch.
  This is the same class as the open "identical assignment reaching
  different merge points" residue noted for `Entity__UpdateActivationState` in
  `docs/DECOMPILATION_LEARNINGS.md`, but here it WAS reachable: a bare
  `__asm__("");` placed right after the `unkFC == 0x190` branch's
  assignment blocked the merge and reproduced retail's two separate blocks
  exactly (86/89 immediately after adding it). Worth noting for the open
  `Entity__UpdateActivationState` case -- a scheduling barrier in one of the two merge
  candidates may be worth retrying there too.
- **Residue #3, and it turned out to make the barrier from #2
  unnecessary: statement order inside the `unkFC == 0x320` arm.** Retail
  computes `table = ROTATION_YAW_PLUS180;` (the `lui`/`addiu` pair) BEFORE storing
  `this->unkFC = -1;`, even though the natural narrative order (validate
  the state, THEN update it) suggests writing the store first. My first
  attempt wrote `this->unkFC = -1; table = ROTATION_YAW_PLUS180;` and only the store
  instruction's position was wrong (3 words off from retail, matching
  score 86/89). Swapping the two statements' order fixed it exactly to
  89/89 -- and, checked afterward, ALSO made the `__asm__("")` barrier
  from residue #2 no longer necessary: with the statements in the right
  order, GCC stopped performing the merge on its own. Removed the barrier
  and reconfirmed 89/89 with a clean rebuild. **Always re-test whether an
  earlier barrier is still load-bearing after a later fix** -- CLAUDE.md
  rule 6 requires this, and here it genuinely wasn't needed once the real
  defect (statement order) was fixed.

### Proposed learning

- **"Identical assignment reaching different merge points" (the open
  `Entity__UpdateActivationState` residue class) CAN be reachable with a plain
  `__asm__("")` scheduling barrier placed in one of the two merge
  candidates** -- worth retrying on `Entity__UpdateActivationState` itself. This is the
  first confirmed instance of this class closing.
- **A `void*`/pointer local that must survive an intervening vtable call
  needs its INITIALIZER at the point of declaration, not a separate
  assignment statement positioned after the call** -- same "eager
  initialization" family as `Entity__MoodCue55`'s `mood` field-read, but for a
  local variable's default value rather than a struct field read. The tell
  is identical: a frame missing one callee-saved register pair versus
  retail, and a `funcdiff` "differs outside range" count in the tens- or
  hundreds-of-KB.
- **When a barrier fixes a residue, re-test removing it after fixing any
  OTHER residue in the same function** -- a later fix (here, statement
  reordering) can make an earlier barrier redundant, and the clean version
  without it is the one to commit.

## Naming

`Entity__MoodCue40` -- tier B (round 76, runner delta, FINISHING-PLAN track 3). Same
convention as `Entity__MoodCue00` (round 71): the function's address is the
handler word of `gEntityMoodHandlerTable` (`asm/data/79528.data.s`, base
0x80089EB0, 0x10-byte stride) at row 40, read directly from
`disk/SLPS_015.56` (not inferred from address proximity -- see
`src/Entity_d.c`'s unit header comment, which flags that row order does NOT
track code address once row 115 is reached). Mechanics established
(mood-tick sound-cue-set callback, per `Entity__StartSoundCue`/
`Entity.c`'s own header comment); which dream object owns the row is not.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/Entity.h`: a TodActor subclass whose table and object expand `TODACTOR_SLOTS`/`TODACTOR_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 7 (round 93, bravo)

Every literal in the live body is in its base: decimal for moodTimer ticks, distances, TOD frames, VAB programs, volumes and `state` phases (hex remains only for masks). Byte-identical (whole image green).
