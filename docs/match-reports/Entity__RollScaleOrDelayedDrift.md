# Entity__RollScaleOrDelayedDrift -- MATCHED (58/58 words)

> Renamed from `Entity__func_80060710` on 2026-09-26 (tools/rename.py). Address 0x80060710.

> Renamed from `func_80060710` on 2026-09-24 (tools/rename.py). Address 0x80060710.

Unit: `Entity` (fresh carve, round 2026-09-03). Mood-dispatch helper called
by `Entity__MoodCue43`, but itself takes only `Entity *this` -- no `out`
parameter, despite the family convention. Signature: `void
Entity__RollScaleOrDelayedDrift(Entity *this)`.

## Final source

```c
void Entity__RollScaleOrDelayedDrift(Entity *this) {
    s32 r;

    if (this->unkFC == 0) {
        r = rand() % 10;
        if (r >= 8) {
            this->methods->slot48(this, 1, sScaleX3);
        } else if (r >= 5) {
            this->unk44 = 0xA;
        }
    }
    if (this->unk44 == 0xA && this->unkFC >= 0xC9) {
        this->methods->slotBC(this, TRANSLATE_Z_MINUS256);
    }
}
```

## Derivation notes

- **Physical block order is load-bearing, and this is the function that
  proved it for this unit.** The natural C reading is `if (r < 8) { if (r
  >= 5) unk44=0xA; } else { slot48(...); }`, and it compiles -- but with the
  `r < 8` case as the FALLTHROUGH block and the `r >= 8`/`slot48` case as the
  jumped-to block, which is the OPPOSITE of retail's physical layout
  (fallthrough = `slot48` call, jump-target = the small `r<8`/`r>=5` block).
  Same logical condition, wrong branch polarity in the emitted code, and it
  cost one whole instruction (a missing `j` before the small block, since
  retail's `slot48` path exits early via `j 607a4` straight to the OUTER
  join point, skipping the small block entirely). Writing the `r >= 8`
  case as the `if` (first) and `r >= 5` as `else if` reproduced retail's
  layout exactly.
- **This one function's single missing instruction was responsible for the
  ENTIRE unit's other seven functions showing near-zero matches
  simultaneously.** Before fixing it, `funcdiff.py` reported "differs
  OUTSIDE this range" counts around 116,000 bytes for every other function
  in the file, and functions physically after this one in ROM order (e.g.
  `Entity__MoodCue48`, `Entity__MoodCue50`, `Entity__MoodCue56`) scored 0/N despite
  being logically correct, because their comparison window was reading from
  the wrong address entirely. Fixing this function's word count collapsed
  the drift to under 200 bytes project-wide and every other function's score
  became trustworthy again. **When several functions in one unit show
  simultaneous, severe, EVEN-NUMBERED-of-total mismatches with the drift
  warning firing at a five/six-figure byte count, suspect ONE upstream
  function's word count before doubting several unrelated ones.**
- The two data-table arguments (`sScaleX3` to `slot48`, `TRANSLATE_Z_MINUS256` to
  `slotBC`) are plain `extern u8 SYM[];` externs passed directly, no offset
  arithmetic needed (contrast `Entity__MoodCue41`'s `sScaleTemplateZDenom`, which does
  need one).

### Proposed learning

- **When several functions in one unit show simultaneous severe mismatches
  (near-zero words, and `funcdiff`'s "differs outside range" warning firing
  at a five/six-figure byte count for ALL of them at once), look for ONE
  function earlier in ROM order with a word-count defect before treating
  each low score as its own separate bug.** Fixing the one upstream
  size-changing function here collapsed drift from ~116KB to under 200
  bytes and turned six other functions' scores from meaningless into
  accurate, without touching them.
- **A two-way branch's PHYSICAL fallthrough/jump-target assignment is not
  free even when the logical condition is right.** Writing `if (r>=8) {
  call } else if (r>=5) { store }` versus the logically-equivalent `if (r<8)
  { if (r>=5) store } else { call }` picks which arm is the fallthrough
  block and which is reached by an explicit jump -- and only one ordering
  reproduces a given retail layout, independent of which arm reads more
  naturally as the "primary" case.

## Naming

`Entity__RollScaleOrDelayedDrift` -- tier C (round 76, runner delta, FINISHING-PLAN
track 3). Renamed from `func_80060710`; class confirmed Entity by every
call in its body going through `this->methods->...`. NOT itself a
`gEntityMoodHandlerTable` row (checked against every row's handler word in
`disk/SLPS_015.56`, base 0x80089EB0..0x8008A780ish -- no row holds
0x80060710) -- it is a private helper `Entity__MoodCue43`/`Entity__MoodCue44`
both call directly by name (`jal`), never through a vtable or the mood
table. Mechanics: at moodTimer 0, an 80/20-ish dice roll either bumps scale
via `sScaleX3` or arms a delayed effect (`unk44 = 0xA`); once armed and
moodTimer reaches 0xC9, calls `addVec14(TRANSLATE_Z_MINUS256)`. No second
caller or additional context to say WHY MoodCue43/44 share this specific
startup quirk, so the tier-C placeholder form is kept rather than guessing
a purpose.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/Entity.h`: a TodActor subclass whose table and object expand `TODACTOR_SLOTS`/`TODACTOR_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 7 (round 93, bravo)

Every literal in the live body is in its base: decimal for moodTimer ticks, distances, TOD frames, VAB programs, volumes and `state` phases (hex remains only for masks). Byte-identical (whole image green).

## Naming (round 93, bravo, track 7)

`Entity__func_80060710` -> `Entity__RollScaleOrDelayedDrift` -- **tier B**.
What the body does: on the cue's first tick (`moodTimer == 0`) it rolls
`rand() % 10`; 8 or 9 sets the scale to `sScaleX3` ({3/1, 1/1, 1/1}), 5 to 7
sets `state` to 10; while `state` is 10 and `moodTimer >= 201` it calls
addTranslation with `TRANSLATE_Z_MINUS256` every tick. "Roll", "scale" and
"delayed drift" are those mechanics. Its callers are Entity__MoodCue43 and
Entity__MoodCue44, which call it first every tick; what the effect is for in
the game (which dream objects use rows 43/44) is not established, hence B.
The literals are decimal (10, 201); the `state` value 10 is this helper's
own phase, shared with nothing.

## Entity banner before round 93 (moved here, not deleted)

The unit banner was rewritten as documentation in round 93. Its history and
derivation, verbatim:

```text
/* Second 20-function slice of the Entity class's 97-function remainder,
 * 0x5077C..0x52290 (Entity is the first slice, Entity_e the third).
 *
 * 19 of the 20 are gEntityMoodHandlerTable callbacks (Entity.h), named
 * Entity__MoodCueNN for the row they occupy -- rows 39-52 and 55-58 are
 * consecutive with Entity's own tail, row 115 (Entity__MoodCue115, this
 * unit's last function) is not, confirming row order tracks moodIndex
 * assignment, not code address. The names were confirmed by reading
 * disk/SLPS_015.56 directly rather than trusting address proximity:
 * gEntityMoodHandlerTable's own base plus a fixed per-row stride locates
 * each row's `handler` word (the arithmetic and addresses are in each
 * function's docs/match-reports/Entity__MoodCueNN.md), and it was checked
 * against every candidate function's own address. The one
 * exception, `Entity__func_80060710`, is not itself a table row -- it is a
 * private helper Entity__MoodCue43/44 both call directly (`jal`, not
 * through any vtable or table), tier C because its own purpose beyond
 * "sometimes bump scale, sometimes queue a delayed addTranslation" is not
 * established.
 *
 * `Entity__MoodCue45` is a real, matched, genuinely empty function (`{}`,
 * `jr $ra; nop` after splat's own frame elision) -- row 45 of the table is
 * a legitimate "this mood has no per-tick cue effect" entry, not an
 * unfinished stub.
 */
```

(The per-row handler-address arithmetic it cites is in each
`Entity__MoodCueNN.md`'s `## Naming`; the row-115 ordering point is in
`Entity__MoodCue115.md`'s. In the text above the helper appears under its
old name. The old extern comment also said the tables followed "the same
convention as Entity.c's own SCALE_Y2/SCALE_SIX/etc externs (separate
local view per translation unit, not shared via the header)"; round 93
retyped them from `u8[]` to `Ratio16[]`, byte-identical.)
