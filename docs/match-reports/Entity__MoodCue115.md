# Entity__MoodCue115

> Renamed from `func_80061778` on 2026-09-24 (tools/rename.py). Address 0x80061778.

**Unit:** Entity_d · **Size:** 198 words · **Status:** MATCHED (round 68) — 198/198, whole-image oracle green

REVISITED, round 68: MATCHED 198/198 on the first build after retyping
`EntityMethods::slotCC` from `s32` back to `void`; names/types used — the
whole stall WAS a type error, and the revisit's required baseline rebuild is
what exposed it.

## Round 68 revisit — baseline first, as the revisit rule requires

The inherited body (below, under "Attempt history") was rebuilt verbatim
before anything was changed, with only the round-44/67 field and slot renames
applied (`unk94`→`target`, `unkFC`→`moodTimer`, `slot15C`→`activate`,
`slot160`→`deactivate`, `slot168`→`startSoundCue`, `slot16C`→`stopSoundCue`,
`slot148`→`getProximityRatio`, `slot30`→`notifyParents`). It reproduced the
inherited score exactly:

```
Entity__MoodCue115: 115/198 words match (file 0x51F78-0x52290)
Entity__MoodCue115: insertions 8 / deletions 8 (opcode-level, built vs retail;
               positional skeleton diffs 78)
```

**and one figure the inherited report never recorded: the built function was
0x330 bytes = 204 words against retail's 0x318 = 198, i.e. 6 words LONG**
(`build/lsdde.map`: `Entity__MoodCue115` at `0x80061778`, next symbol
`Entity__MoodCue59` at `0x80061aa8`). That number is what cracked it, because it
is exactly +3 in each of the two dispatch chains, and "the same deficit twice"
is the opposite of the inherited report's "opposite-direction misses" reading.

## What the stall actually was

`EntityMethods::slotCC` was typed `s32 (*)(Entity *self, s32, s32)`.
Retyping it `void` matches the function 198/198 on the first build, with the
whole image byte-exact. Nothing else changed — not one line of the body, which
is the inherited body verbatim.

### The mechanism

GCC 2.6.3 emits a value-returning call as `(set (reg:SI v0) (call (mem:SI
(reg v0)) ...))` and a void call as a bare `(call (mem:SI (reg v0)) ...)`.
`find_cross_jump` compares candidate insns with `rtx_equal_p`, so **two calls
whose callees differ in RETURN TYPE can never be tail-merged, however
identical their argument registers and call-target register are.** The call
insn is where a shared call suffix must begin, so the difference does not cost
one instruction — it forecloses the entire merge.

Both of this function's dispatch chains end several arms in an indirect call.
Retail merges them; the `s32` typing split each chain's merge set in two:

| chain | arms | slot | typed | retail | with `slotCC` = `s32` |
| --- | --- | --- | --- | --- | --- |
| 1 | `<3`, `<7` | `EntityMethods::slotCC` | **s32** | all three merge at `.L800619A4` (`jalr $v0`/`nop` — the minimal common suffix, i.e. just the call) | `<3`+`<7` merge pairwise at the *maximal* common suffix (3 words, `nop`/`jalr`/`move $a2,zero`) |
| 1 | `==0xF0` | `Unk94Methods::slot134` | void | (as above) | left out entirely, +3 words |
| 2 | `<0x82` | `EntityMethods::slotCC` | **s32** | all three converge on `.L80061A18` (`nop`/`jalr $v0`/`move $a2,zero` = `(set a2 0)` + the call) | left out entirely, +3 words |
| 2 | `<0xA0`, else | `EntityMethods::slotC4` | void | (as above) | merge pairwise at `.L80061A0C`, 6 words, correctly |

So in each chain, **the one arm that failed to join the merge is precisely the
arm whose slot was typed differently from its siblings** — and that arm is the
majority side in chain 1 and the minority side in chain 2. That asymmetry is
the entire source of the inherited report's "opposite-direction" reading: an
artifact of which side of each chain the mistyped slot happened to sit on, not
two behaviours.

The merge DEPTH follows from the same fact rather than being a second
phenomenon: retail's shared block is always the minimal common suffix of the
arms it unifies, and the scheduling around it (which immediate lands in the
load-delay slot versus the `j` delay slot) is a *consequence* of how many
words the shared block already sets up. With `slotCC` void, all of that falls
out with no source change at all.

### Why the old typing was adopted, and why it was never evidence

`slotCC` was typed `s32` on the strength of `Entity__MoodCue37` (Entity_c) being a
lone one-line wrapper, `return this->methods->slotCC(this, -0x5A, 0);` —
CLAUDE.md's "one-line wrapper" rule, which says a discarded return is never
evidence of `void`, so a wrapper returning the callee's value should be typed
value-returning absent contrary evidence.

**`Entity__MoodCue37` compiles byte-identically either way**, as
`void Entity__MoodCue37(Entity *this) { this->methods->slotCC(this, -0x5A, 0); }`
— verified by the whole-image oracle. Its bytes were never evidence in either
direction. This function's bytes ARE evidence, and they say `void`.

### Blast radius, all re-verified byte-exact

35 `this->methods->slotCC(...)` call sites across `Entity_b/c/d/e/g`. Only two
use the value, both in `src/Entity_c.c`:

- the local function-pointer variable in `func_8005FE1C`'s block
  (`s32 (**slotCC)(Entity *, s32, s32);` → `void (**slotCC)(...)`)
- `Entity__MoodCue37`'s `return` → a bare statement call, and its own return type
  `s32` → `void`. It has no prototype in any header and no other caller, so
  this is a purely local retype.

`./build-and-verify.sh` exits 0 with the header comment updated too.

## What it does

A mood-dispatch handler in this unit's family (`Entity *this,
EntityMoodHandlerArg *out`). On `this->unk44 == 0`, calls
`Entity__IsTargetInRange(this, 0x800)` (matched in `Entity_b.c`; this is its first
cross-unit caller); on a nonzero result it runs a "detach" burst (two
`Class6B5CC__FaceTarget` calls with swapped first/second arguments — see the
round-13 head finding below —, `activate`, `startSoundCue`,
`target->slot130(target, 1)`, resets `out`/`moodTimer`) and sets
`unk44 = 0xB`. A re-check of `this->unk44 == 0` then runs
`deactivate`/`stopSoundCue` and skips to the shared tail.

Note the CFG nesting, which the inherited prose had backwards: retail's
opening `bnez $v0, .L80061888` jumps PAST `.L80061840`'s reload, so the second
`unk44 == 0` test is reached only from inside the first one's body. The
inherited report called it "a SEPARATE, freshly-reloaded check" matching this
unit's independent-`if` idiom. The body nonetheless writes them as two
sequential top-level `if`s and compiles byte-exact, because GCC threads the
second test away on the path where `$v0` is still live and known nonzero — so
the flat spelling is correct here even though the CFG is nested. Worth knowing
before anyone "fixes" it in either direction.

Otherwise: `out->unk4 % 100` reseeds `out`'s fields; a `moodTimer` dispatch
(`<3`, `<7`, `==0x64`, `==0xF0`) drives `slotCC`/`slot130`/`target->slot134`;
a further `unk44 == 0xC` sub-dispatch on `moodTimer` drives
`slotCC`/`slotC4`/a `Class6B5CC__FaceTarget` call. The shared tail (reached
from every path) is a `slot144(this, target) < 0x200` check driving
`deactivate` + `notifyParents(this, 0xA)`.

## Final body

As committed in `src/Entity_d.c` — the inherited near-miss body unchanged
apart from the renames. The fix was entirely in the header.

```c
void Entity__MoodCue115(Entity *this, EntityMoodHandlerArg *out) {
    if (this->unk44 == 0) {
        if (Entity__IsTargetInRange(this, 0x800) != 0) {
            this->unk44 = 0xB;
            Class6B5CC__FaceTarget(this, this->target, 1, 0, 0);
            Class6B5CC__FaceTarget(this->target, this, 1, 1, 0);
            this->methods->activate(this);
            this->methods->startSoundCue(this);
            this->target->methods->slot130(this->target, 1);
            out->unk10 = 0;
            out->unk1C = 0xC;
            this->moodTimer = 0;
        }
    }
    if (this->unk44 == 0) {
        this->methods->deactivate(this);
        this->methods->stopSoundCue(this);
        goto tail;
    }
    if (out->unk4 % 100 == 0) {
        out->unk10 = this->methods->getProximityRatio(this);
        out->unk1C = 0xC;
        out->unk20 = -1;
    }
    if (this->moodTimer < 3) {
        this->methods->slotCC(this, 0x96, 0);
    } else if (this->moodTimer < 7) {
        this->methods->slotCC(this, (this->moodTimer & 1) ? -0x32 : 0x32, 0);
    } else if (this->moodTimer == 0x64) {
        if (rand() & 1) {
            this->unk44 = 0xC;
            this->methods->slot130(this);
        }
    } else if (this->moodTimer == 0xF0) {
        this->target->methods->slot134(this->target, 1, 1);
    }
    if (this->unk44 == 0xC) {
        if (this->moodTimer < 0x82) {
            this->methods->slotCC(this, 0xA, 0);
        } else if (this->moodTimer < 0xA0) {
            this->methods->slotC4(this, -0x1E, 0);
        } else if (this->moodTimer < 0x12D) {
            /* nothing */
        } else {
            Class6B5CC__FaceTarget(this, this->target, 1, 0, 0);
            this->methods->slotC4(this, -0x1E, 0);
        }
    }
tail:
    if (this->methods->slot144(this, this->target) < 0x200) {
        this->methods->deactivate(this);
        this->methods->notifyParents(this, 0xA);
    }
}
```

The `(this->moodTimer & 1) ? -0x32 : 0x32` ternary must stay INLINE in the
call, not hoisted to a local — that part of the inherited attempt history is
real and still load-bearing (it is what puts `this->methods` in `$a2` in the
`<7` arm, matching retail). Splitting it out regresses the arm.

## Proposed learning

### A callee's RETURN TYPE gates cross-jump/tail-merge eligibility

Worth promoting to `docs/DECOMPILATION_LEARNINGS.md`; it is now confirmed in
both directions, on two different slots, by two different rounds.

- **Discriminator.** Retail merges N sibling call sites at their MINIMAL
  common suffix; your build merges only a maximal-common-suffix PAIR (or
  nothing), and the arm left out is the one whose callee you typed with a
  different return type from its siblings. Length comes out N words long,
  roughly 3 per un-merged arm.
- **Mechanism.** `(set (reg v0) (call ...))` and a bare `(call ...)` are not
  `rtx_equal_p`. The call insn is where a shared call suffix must begin, so a
  return-type mismatch does not cost one instruction — it forecloses the whole
  merge.
- **Diagnostic, before reshaping anything.** List the return types of every
  call in the chain. Retail's merge set must be type-uniform. This is a
  30-second check and strictly cheaper than any source reshape.
- **Both directions are measured.** `include/Entity.h`'s `slotC4` comment
  records `void` → `s32` STOPPING a merge and costing the already-matched
  `Entity__MoodCue00` 4 words. This report records `s32` → `void` RESTORING one
  and closing a 6-word, two-round stall.

### Corollary: a lone `return callee(...)` wrapper is not evidence of a value return

CLAUDE.md's "one-line wrapper" rule is a tie-breaker for when there is no
evidence, and it says so ("no positive evidence of void"). The trap is that it
reads like evidence once it has been written into a header comment, and the
header is then what everyone else builds on. `Entity__MoodCue37` compiles
byte-identically both ways: **a tail-call wrapper's bytes are not evidence in
EITHER direction**, so the rule never produced a fact here, only a default.
When a sibling cross-jump elsewhere does produce a fact, it outranks the
default — and the way that fact surfaces is as an unexplained tail-merge stall
in a completely different unit.

### On the inherited diagnosis, and how a wrong CAUSE cost two rounds

The inherited report (rounds 13 and 20) is careful and correct in every local
observation: it traced Chain 2's nested two-level merge structure instruction
by instruction and was right about all of it. What it got wrong was one
inference — that the two chains showed OPPOSITE merge behaviour and that this
"rules out a single uniform fix" — and that inference set the direction for two
rounds of source reshaping and produced the standing instruction "do not
re-stake this function on the same axis".

What would have caught it is the length delta. **A +3-word deficit appearing
twice in one function is a repeated single cause, and "opposite-direction" is
the one reading it rules out.** The inherited report never recorded the built
length. The revisit rule's requirement to rebuild the inherited body and
record `insertions/deletions` before touching anything is what surfaced it —
though not via that figure: 8/8 with 78 positional skeleton diffs was exactly
the false alignment on a repeating call-setup skeleton that the revisit brief
warns about. It was the `build/lsdde.map` lookup done alongside it that paid.

Generalising, and this is the part worth carrying: the inherited report's
"Where it stands" claimed **"every value, every branch target, every register
the CONTROL FLOW and DATA use is confirmed correct"** — and that was true, and
the function still did not match, because the defect was in a TYPE that no
instruction in the function displays. **A residue characterised entirely in
terms of instructions can be complete, accurate, and still point away from the
cause.** When a residue description bottoms out at "the compiler just chose
differently", the next question is what the compiler was told, not what it
emitted.

## HEAD FINDING, round 13: the reversed `Class6B5CC__FaceTarget` arguments are a DIRECTION FLAG

Runner echo (`Entity__MoodCue115`, `Entity_d`) and runner bravo (`Entity__MoodCue81`,
`Entity_e`) each independently flagged a `Class6B5CC__FaceTarget` call site
whose first two arguments are swapped relative to every other known site. Both
verified it against raw disassembly. The head then surveyed **every** call site
in the executable, and the swap is not an outlier convention — it is perfectly
correlated with the FOURTH argument:

| `$a3` | `$a0` | `$a1` | sites |
| --- | --- | --- | --- |
| 0 | `this` | `this->target` | 9 in asm + 19 already matched as C |
| 1 | `this->target` | `this` | 2 (the two flagged sites) |

Reading: `Class6B5CC__FaceTarget` is **symmetric in its first two
parameters**, and `$a3` names which way round the operation runs. The caller
pre-swaps the operands rather than the callee branching on the flag. The
extern signature is CORRECT as it stands; do not "fix" a swapped site back to
the common order, and do not retype the first parameter to `void *` for the
sake of the two outliers (both compile with a non-fatal incompatible-pointer
warning, the same tolerance this codebase already extends elsewhere). If both
objects fit either slot, the real parameter type is most likely a COMMON BASE
the two share — worth resolving with `tools/classtable.py`, and NOT resolved
here.

Method note: two runners in different units flagging the same oddity is what
promoted this from a curiosity to a survey. A single sighting reads as a
transcription error; the second one is what justified the corpus sweep, and
the sweep found the flag correlation neither sighting could see alone.

## Attempt history from the stall rounds — superseded as a diagnosis, but the negatives still stand as negatives

1. **Ternary argument inline in the call rather than hoisted to a local** —
   REAL and still in the final body. Hoisting made the `<7` arm load
   `this->methods` into `$v0` instead of `$a2`. Took the stall score from
   99/198 to 115/198.
2. **Extra nesting around the `==0x64`/`==0xF0` pair** — byte-identical output
   to the flat `else if` chain. Rules out "GCC's crossjump only looks within
   one nesting level".
3. **Nesting the `<3`/`<7` pair inside an outer `if (moodTimer < 7)` guard**
   (round 20) — regressed to 107/198. Reverted.
4. **Isolated reproducer of Chain 1's shape** — did not reproduce the merge
   behaviour at all. Read at the time as evidence of context-sensitivity to
   surrounding register pressure. In hindsight the reproducer declared its own
   structs and so (almost certainly) typed all its slots uniformly, which is
   why it showed no anomaly. A reproducer that fails to reproduce is a result,
   and this one was pointing straight at the answer: the thing it had dropped
   when it dropped the real headers WAS the bug.
5. **`goto` to force retail's literal jump graph** — ruled out by argument
   rather than tried, on the grounds that cross-jumping runs on the CFG after
   front-end lowering. That reasoning is still correct and the conclusion
   still holds.

### On the round-20 "are these one residue class?" question

Round 20 compared this function against `Entity__UpdateActivationState` and
concluded they were two different shapes under one umbrella mechanism, on the
strength of the "opposite-direction" reading. That reading is withdrawn.

**`Entity__UpdateActivationState` itself is NOT a candidate for the lever —
it was matched in round 25** (74/74, `docs/match-reports/` says so in its
first line). I very nearly filed "re-check it with the return-type test" as a
recommendation here on the strength of round 20's prose alone, which would
have been a stale conclusion of exactly the kind CLAUDE.md warns about; the
check that caught it was one `grep` of `src/`. Anyone acting on a
cross-reference inside an old report should confirm the referenced function is
still queued before spending a round on it.

**Where the lever should actually be tried.** 15 live stall reports mention
cross-jumping or tail-merging (`grep -rilE 'cross-?jump|tail[ -]merg'
docs/match-reports/`, then filter to the ones whose first lines say STALL):
`Class6B5CC__TryAttachNearby`, `DreamSys__StepLookYaw`,
`DreamSys__TryInstantTeleportLink`, `cd_read_retry`, `func_80032148`,
`func_80033AB0`, `Snd_decrescendo`, `_SsSetControlChange`, `ContDataEntry`,
`TaskCore__CommitElementScroll`, `Class866E8__SplitFootprintSlot`, `TaskObjF__CheckCardStatus`, `StyleFillEffectKind3`,
`Entity__MoodCue111`, `TaskObjF__BeginSave`. I did not screen these — that is
other units' work and outside this brief — but the ordering heuristic is
cheap: **rank by LENGTH-OFF first**, because an un-merged arm costs words
(~3 here) while a merge that lands in the wrong place at the right size does
not. `TaskObjF__BeginSave` self-describes as a tail-merge stall one word
off, which makes it the nearest thing to a control: one word is too small for
a foreclosed merge, so if the return-type check fires there it would be
evidence the mechanism has a second, cheaper mode, and if it does not fire the
lever stays cleanly scoped.

## Naming

`Entity__MoodCue115` -- tier B (round 76, runner delta, FINISHING-PLAN track
3). Renamed from `func_80061778`. Same convention as `Entity__MoodCue00`
(round 71): the function's address (0x80061778) is the handler word of
`gEntityMoodHandlerTable` (`asm/data/79528.data.s`, base 0x80089EB0,
0x10-byte stride) at row 115 (0x80089EB0 + 0x10*115 = 0x8008A5E0), read
directly from `disk/SLPS_015.56`. Row 115 is NOT contiguous with this
unit's other rows (39-52, 55-58) or with Entity_e's own rows (59-92ish),
confirming the row index tracks moodIndex assignment rather than code
address -- flagged in `src/Entity_d.c`'s unit header comment so the next
reader doesn't assume a typo. Mechanics established (mood-tick sound-cue-set
callback); which dream object owns the row is not.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `ENTITY_METHODS`) is unified as `Entity` in `include/Entity.h`: a Class65650 subclass whose table and object expand `CLASS65650_SLOTS`/`CLASS65650_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
