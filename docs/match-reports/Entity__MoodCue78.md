# Entity__MoodCue78 -- MATCHED, round 59 (2026-09-20)

> Renamed from `func_80062C58` on 2026-09-24 (tools/rename.py). Address 0x80062c58.

REVISITED, round 59: MATCHED 213/213 byte-exact; names/types not relevant

**Unit:** `Entity_e` · **Size:** 213 words · **Result:** 213/213 words,
`insertions 0 / deletions 0`, `./build-and-verify.sh` green
(`OK: build matches retail SLPS_015.56`), `Entity__MoodCue79` back at its retail
address `0x80062fac`.

Previously: STALL at 61/213 with the built function **+2 words long**
(round 45); before that, filed as `gp_rel`-blocked (round 12, blocker
resolved round 42). Closed in **11 builds** this round.

## On the revisit hypothesis (the measurement this round was buying)

The unit's names and types **did change** since round 45 — `unkFC` is now
`moodTimer`, `unk94` is `target`, `slot148` is `getProximityRatio`,
`slot16C` is `stopSoundCue` — and `tools/rename.py` had already rewritten
the preserved `#if 0` body in `src/Entity_e.c` accordingly. **None of it
mattered.** Every renamed field was already correctly identified in round
45's derivation; the new names made the body easier to read and changed
nothing about what compiled. Re-reading the callers likewise produced
nothing the report did not already have.

**What the revisit actually bought was re-measuring, not re-reading.** The
round-45 report's central claim —

> The three-way `unkFC` dispatch ... reproduces retail's branch STRUCTURE
> exactly -- every branch target and every call ... matches ... the logic
> itself is settled.

— is **false**, and one `objdump` of the built object against the `.s`
falsifies it. So the revisit rule earned its keep here, but not through the
mechanism it names: the stale thing was a *measurement*, not a vocabulary.

## The stall was THREE independent defects, not the one its report named

Round 45 described a single residue ("GCC schedules a free register copy
into a far-upstream delay slot") and proposed a lever for it. There were
three, and that one was the last of them.

### Defect 1 -- branch POLARITY / block order (the +2 words)

Retail gives the **fall-through to the `else` arm**, not to the `if` body:

```
53554:  slt   v0,a1,v0          ; v0 = moodTimer < half+quarter
53558:  bnez  v0,53580          ; TRUE jumps away
5355c:   slt  v0,a1,a0          ; delay slot
53560:  lw    v0,0(s0)          ; <-- fall-through IS the slot12C arm
```

The round-45 body wrote `if (moodTimer < half+quarter) { nest } else
{ slot12C }`, which gives the fall-through to the nest and inverts both
outer branches. Same for the second test. The innermost test (`< quarter`,
empty then-arm) was already right. The fix is to invert the two outer
conditions so the arms swap places textually:

```c
if (this->moodTimer >= half + quarter) {
    this->methods->slot12C(this);
} else if (this->moodTimer >= half) {
    if (this->moodTimer == half) { out->unk1C = 0x10; }
    this->methods->slotC4(this, 0x6E, 0);
} else if (this->moodTimer < quarter) {
    /* nothing */
} else {
    this->methods->slot130(this);
    this->methods->slotC4(this, -0x6E, 0);
}
```

This is `MATCHING-GUIDE.md`'s "branch TARGETS disagree, not just delay
slots" discriminator, and it outranks everything else in the residue list
exactly as that entry says.

### Defect 2 -- the reroll block's `old` local and store placement

Round 45 wrote the `unk44 >= 0xC` reroll arms as

```c
old = this->unk44;
this->unk44 = 0xC;
if (old == 0xD) { tmp = -0x190; }
table = D_80089E14;
```

Retail has no `old` at all, loads the table first, and puts the **store
after the test**, which is what frees the `bne` delay slot:

```
536fc:  lui   a2,0x8009
53700:  addiu a2,a2,-0x61ec     ; table = D_80089E14
53704:  lw    v1,0x44(s0)
53708:  li    v0,0xd
5370c:  bne   v1,v0,53718
53710:   li   v0,0xc            ; delay slot -- not a nop
53714:  li    s1,-0x190
53718:  sw    v0,0x44(s0)       ; the store, at the join
```

so:

```c
table = D_80089E14;
if (this->unk44 == 0xD) { tmp = -0x190; }
this->unk44 = 0xC;
```

With the store written first, GCC has nothing to sink into the delay slot
and emits a `nop` there -- a whole wasted word.

### Defect 3 -- the closer: **delete the locals, inline the expressions**

With 1 and 2 fixed the function was down to `insertions 1 / deletions 1`,
one word SHORT, and the entire residue was a single missing instruction:

```
TARGET                              CURRENT
53524:  bne  a1,v0,53538           53524:  bne  a1,v0,53538
53528:   move a2,v1          |     53528:   srl  v0,v1,0x1f
53538:  srl  v0,v1,0x1f      <     (absent)
53548:  addiu a2,v1,3        r     53544:  addiu v1,v1,3
5354c:  sra  a2,a2,0x2       r     53548:  sra  v1,v1,0x2
```

GCC 2.6.3 lowers signed `x / 4` to `t = x; if (x >= 0) goto L; t = x + 3;
L: q = t >> 2`, and that leading `t = x` copy is the missing word. With
`half` and `quarter` as named locals the allocator **coalesces `t` into
`x`'s register** and the copy vanishes (`addiu v1,v1,3` destructively
reusing `v1`). Retail keeps them apart (`a2` vs `v1`), so the copy
survives -- and reorg then hoists it into the far-upstream `bne` delay
slot, which is the behaviour round 45 correctly observed and then
mis-attributed to scheduling.

**The fix is to delete the two locals and inline the divisions at all four
use sites.** CSE still computes each division once, so the arithmetic is
identical, but the live ranges are not, and the copy is no longer
coalesced away:

```c
if (this->moodTimer >= this->unk80 / 2 + this->unk80 / 4) {
    ...
} else if (this->moodTimer >= this->unk80 / 2) {
    if (this->moodTimer == this->unk80 / 2) { out->unk1C = 0x10; }
    ...
} else if (this->moodTimer < this->unk80 / 4) {
```

That build was byte-exact on the whole image.

## What was measured, in order (11 builds)

| # | body | words | ins/del | `Entity__MoodCue79` links at |
| --- | --- | --- | --- | --- |
| 1 | round-45 inherited body (control) | 61/213 | 27/27 | `0x80062fb4` (+2w) |
| 2 | +polarity fix, quarter before half | 56/213 | 11/11 | `0x80062fb4` (+2w) |
| 3 | +polarity fix, half before quarter | **91/213** | 7/7 | **`0x80062fac` (exact)** |
| 4 | 3 + reroll-block fix | 54/213 | **1/1** | `0x80062fa8` (-1w) |
| 5 | 4, quarter before half | 52/213 | 4/4 | `0x80062fb0` |
| 6 | 4 + staged `s32 n = this->unk80` | 54/213 | 1/1 | `0x80062fa8` |
| 7 | 4, declaration order `quarter, half` | 54/213 | 1/1 | `0x80062fa8` |
| 8 | 4 + named `sum = half + quarter` | 54/213 | 1/1 | `0x80062fa8` |
| 9 | **4, locals deleted and inlined** | **213/213** | **0/0** | **`0x80062fac`** |
| 10 | 4 + `quarter = unk80; quarter /= 4;` | 210/213 | 0/0 | `0x80062fac` |

Build 1 reproduced round 45's recorded 61/213 and its +2-word drift
exactly, so the inherited body was verified before anything was built on
it (project rule).

Builds 6, 7 and 8 are the negatives that matter: staging the numerator in
its own local, reversing the declaration order, and naming the sum all
leave the residue *bit-for-bit unchanged*. The knob is not naming
something extra, it is naming something LESS.

## Why the round-45 report's own experiments pointed away from the answer

Round 45 tried "half first" and recorded it as **worse** (56/213 against
61/213) -- and it was, *while the block order was still wrong*. Two levers
that only pay off together, with the word score moving the wrong way for
one of them alone, is a trap the word score cannot see.

**`insertions/deletions` could see it, and did.** Across the sequence the
word score went 61 -> 56 -> 91 while ins/del went 27/27 -> 11/11 -> 7/7 --
monotone, and monotone in the right direction, through the step where the
word score said "worse". Round 45 had no ins/del line (funcdiff gained it
later, for Gate 3 check 3); this round it was the primary signal for every
decision after the first.

## New struct knowledge

None. `Entity::unk90` was already carved by round 45 and is confirmed
correct by the byte-exact match. No header was edited.

### Proposed learning

**When a residue is a missing register-to-register COPY, try DELETING the
named local and inlining the expression.** This is the exact inverse of the
project's well-worn "name the subexpression" lever (`Entity__MoodCue71`,
`func_8002C048`, the named-remainder technique), and it is not in
`DECOMPILATION_LEARNINGS.md` in either direction.

The mechanism is specific and checkable, not a superstition. GCC 2.6.3's
signed `x / 2**k` (k > 1) lowering opens with a copy `t = x`. Whether that
copy survives to the output is a coalescing decision: if `x`'s pseudo is
dead at the end of the division, `t` and `x` get the same hard register and
the copy is deleted. Holding the results in named locals shortened `x`'s
live range enough for the coalesce to fire; inlining the divisions at the
use sites did not. So the discriminator is: **a one-word residue that is a
`move`/copy between two registers, where your build reuses one register
destructively (`addiu v1,v1,3`) and retail uses a second (`addiu a2,v1,3`),
is a LIVE-RANGE question, and the source knob for live ranges is whether a
value is named.** Try both spellings before classifying it as the documented
"redundant move" permuter class -- that class is for copies that survive
every *reordering*, and naming is not a reordering.

Corollary worth keeping separately: **ins/del is the right signal on a
length-defective function, and the word score is actively misleading
there.** A structural fix that corrects block order will move ins/del
monotonically while the word score jumps around, because every instruction
after the first shifted byte scores as a miss regardless of how right it
is. Round 45 rejected a correct lever on a 5-word score drop.

## Naming

Why `MoodCueNN`: the function's address sits in `gEntityMoodHandlerTable` row 78 (`asm/data/79528.data.s`, base 0x80089EB0, stride 0x10; row = (slot address - 0x80089EB0) / 0x10), confirmed by reading `disk/SLPS_015.56` directly rather than trusting address proximity (Entity_d/round 76 measured that row order does not track code address). Tier B: the row-to-function mapping is a compiler fact, not a guess, but which dream object or mood state each row represents is not established -- the row number is kept decimal and zero-padded so the names sort in table order, same convention as Entity_d.

Reading this function's `out->` writes with the proposed `SoundCueSet` field names (`Entity__MoodCue07.md` `## Proposed field names`, tier A/B, proposal only -- `EntityMoodHandlerArg` is shared with Entity_b/Entity_d/Entity_g): `tick`, `attenuation`, `voice0Tone`.

**Data/global renamed this round:** `D_80089D0C` -> `ROTATION_XPLUS90` (first {num,den} pair = (90,1), the X slot by the same X/Y/Z decoding as `Entity__MoodCue68`'s report), tier B. `D_8008ACCC` -> `sMoodCue78TransitionDone`, tier B: a one-shot s32 flag local to this function -- cleared at `out->unk4==0`, set when the `moodState==0xB` branch fires at `unk4==0x1FE`, read once more at `unk4==0x208` to gate a second `stopSoundCue`/`moodState` reset. No other file in `src/` references it.

**`D_80089E14` left unnamed this round.** s16-pair-decoded it reads (1,1, 1,1, 1,1, 1,8) -- X=Y=Z=1 (no scale change on the three named axes), only the 4th/W pair differs (1,8). Every named `SCALE_*` table so far is named for its X/Y/Z content and ignores W (e.g. `SCALE_HALF`'s own W is (4,5), `SCALE_SIX`'s is (2,5)), so this table reads as an X/Y/Z-identity scale and there is no precedent for naming one on its W value alone.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/Entity.h`: a TodActor subclass whose table and object expand `TODACTOR_SLOTS`/`TODACTOR_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 7 (round 93, echo)

Every literal in the live body is in its base: decimal for moodTimer and cue-set ticks, TOD frames, distances, VAB programs and `state` phases (the unit has no hex literal left). Local `tmp` is `rollOrDy`: it holds the rand() % 3 roll and later the y move. Splitting it into `roll` and `dy` was measured and changes the register allocation (whole image red), so it stays one local with a `MATCHING:` line. Both `state = 1` after stopSoundCue are ENTITY_STATE_DONE. `sMoodCue78TransitionDone` got a comment on its declaration (what sets and reads it). `D_80089E14` ({1,1} x3, the identity scale) stays a symbol: Entity_g declares it too, so a rename moves a declaration in another unit; proposed to the head. Byte-identical (whole image green).
