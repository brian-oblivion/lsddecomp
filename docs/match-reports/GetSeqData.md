# GetSeqData -- STALL (register identity)

> Renamed from `func_8003424C` on 2026-09-23 (tools/rename.py). Address 0x8003424c.

**Length exact 172/172. Raw word-match 122/172. First real diff at word 2**
(`tools/asm-differ/diff.py GetSeqData`, which realigns -- confirmed no
address drift: `funcdiff.py` printed no "differs OUTSIDE this range" warning).

Round 31, runner bravo: re-verified, `decomp-permuter` searched, no zero
within budget -- see round-31 update below. Round 33, runner bravo:
re-verified again, extended `decomp-permuter` search (400s) confirms the
same 150-score plateau -- see round-33 update below.

## Round 31 update (runner bravo): re-verified, decomp-permuter searched, no zero within budget

Rebuilt the preserved near-miss body and reproduced the title figures
exactly: 122/172, length exact, register-identity residue (the `$s3`/`$s4`
swap for "channel" vs. the per-case data byte) unchanged from the six
reshape attempts already documented below.

Set up `tools/decomp-permuter` against this body -- the largest function
in this unit's queue and the single biggest register-identity residue, so
worth the budget even knowing a whole-function register-coloring swap is
a much larger search space than the single-instruction residues this
round's other permuter runs closed. Base score confirmed at 773 via
`--debug --stack-diffs` (49 register differences dominate, plus 8 spurious
stack-difference points, 2 reorderings, 2 insertions, 2 deletions -- an
order of magnitude larger than any of this round's five closed near-misses,
consistent with "swap two live ranges across an entire 172-word function"
being a categorically harder target than "one missing/misplaced
instruction").

Ran a 16-way `-j 16` search for the full 240s budget (confirmed exit via
timeout, not an external kill) with `--stop-on-zero --best-only`: **no
zero found in ~52700 iterations.** Best score reached was 150 (down from
the base 773, i.e. the search did make real progress, just not enough to
close in this window) -- the candidate was not inspected in detail given
it did not reach zero and this project's rule is to verify only
zero-or-near-zero candidates through the full pipeline rather than chase
partial permuter-local improvements (`docs/MATCHING-GUIDE.md`'s "a
permuter score drop is a LEAD, never a RESULT" caution, and round 18's
0-for-3 history trusting partial permuter improvements on register-shaped
residues).

**Not exhausted, just not closed within one bounded search of a
categorically larger space than this round's other targets.** A future
round's search with a longer budget, or a hand-found structural trigger
(the six reshapes already tried all held the CALL SIGNATURE and value
FLOW fixed while varying only naming/declaration -- an untried axis per
`docs/MATCHING-GUIDE.md`'s "a long attempt list is not a broad one" is
restructuring which VALUES are computed eagerly vs. lazily at each case
entry, not just how they are named) remains open. This stays a STALL,
correctly classified as register identity per `CLAUDE.md`'s hard rule
(no `register T v asm("$N")`, no operand-constraint fix attempted or
considered).

## What it does

A per-(channel, slot) "sequencer voice" event-stream byte reader. Reads one
byte from `rec->unk4` (advancing the cursor). If the byte has its high bit
set it is a new MIDI-style status byte: the low nibble becomes
`rec->unk12` (byte offset to the active embedded state block) and the high
nibble picks which kind of event follows -- Note On (0x90), Control Change
(0xB0), Program Change (0xC0), Pitch Bend (0xE0), or a Meta/System event
(0xF0, but the CACHED "running status" byte stored back into `rec->unk11`
for this kind is 0xFF, not 0xF0) -- consuming however many further data
bytes that kind needs. If the high bit is clear, the byte just read is
itself the first DATA byte of a new event of whichever kind `rec->unk11`
last recorded (classic MIDI running status) -- same five-way dispatch, one
fewer byte consumed since this byte already stands in for the first data
byte.

**New struct field found**: `Entry90902E8::unk11` (`+0x11`), the cached
running-status byte. It was previously unnamed padding
(`pad11[0x12-0x11]`, exactly 1 byte, so this is a rename, not a shift --
confirmed no other field moved and the whole-image build still verifies
with `GetSeqData` still `INCLUDE_ASM`'d, i.e. before any of this
function's own C was trusted).

## The block-order lever, and why it mattered here

The disassembly's compare chain for the high nibble is NOT emitted in the
case-declaration order you'd guess from "C0 first, since it's checked
first": the compares run `==0xC0`, then `<0xC1` splitting into `==0x90` /
`==0xB0`, else `==0xE0` / `==0xF0` -- a balanced binary-search decision
tree over the SORTED case values (0x90 < 0xB0 < 0xC0 < 0xE0 < 0xF0, with
0xC0 as the pivot). But the physical LAYOUT of the case bodies in memory
follows ASCENDING VALUE ORDER (0x90, 0xB0, 0xC0, 0xE0, 0xF0), which does
NOT match the compare order. Writing the `switch` statement's cases in
that same ascending order (matching retail's presumed source, an
idiomatic ascending list of MIDI status kinds) reproduced BOTH the
decision-tree compares and the body layout exactly -- this is the
project's documented block-order class (`DECOMPILATION_LEARNINGS.md`,
"An arm that must JUMP has to be written NOT-LAST"), just showing up via
switch-statement case order rather than if/else arm order. Writing the
cases in "natural" dispatch order (C0, 90, B0, E0, F0, matching the
compare sequence) put the C0 body FIRST in the layout and cost 30 words
(92/172 instead of 122/172) purely from the resulting branch-offset
cascade -- every branch target downstream of the misplaced block encodes
a different distance, even though the compare instructions themselves
were byte-identical.

The same phenomenon applies to the second (running-status) dispatch on
`rec->unk11`, with the extra wrinkle that its "F0" kind is checked as
`0xFF` (matching the cached value `GetSeqData` itself writes on the
status-byte path) rather than `0xF0`.

## The residue that would not move

After fixing block order, the function is 172/172 words long (exact) and
122/172 raw-match. Every remaining mismatch is one pattern, repeated: the
widened "channel" value (kept live across nearly every call in the
function) sits in `$s4` in retail and in `$s3` here; the per-case "next
data byte" value (note/velocity/controller/program/meta byte -- whichever
one is live across the `ReadDeltaValue` call in the Note-On path, or just
passed straight through in the others) sits in `$s3` in retail and `$s4`
here. Every one of the ~40-odd `move a0,sN` / `sw sN,offset(sp)`
instructions that differ is this SAME pair of registers swapped; there is
no other kind of residue left (no missing/extra instruction, no branch
target disagreement, no immediate-value difference).

**Reshaping tried, in order, none of which moved the swap by even one
line (funcdiff stayed at exactly 122/172 with the identical 50 diff rows
after every one of these):**

1. Aliasing the parameters into named locals (`s16 ch = a0; s16 sl = a1;`)
   used throughout instead of `a0`/`a1` directly.
2. Reordering the declaration list among `raw` / `p` / `note` / `vel` (all
   four permutations tried).
3. Dropping the separate `cursor` local for the first byte-read and
   folding it into the same `p`-based idiom the case bodies use.
4. Introducing a single shared `note` variable used as the argument to
   `_SsSetControlChange`/`SetProgramChange`/`GetMetaEvent` in the 0xB0/0xC0/0xF0
   cases (instead of passing `*p` inline), to raise that pseudo's
   reference count closer to "channel"'s.
5. Declaring `note`/`vel` as `s32` (matching the callee parameter types
   exactly) instead of `u8`, to rule out a byte-vs-word pseudo-class
   difference.
6. Inlining the `low`/`high` locals (`raw & 0xF`, `raw & 0xF0`) directly
   into the switch condition and the `rec->unk12`/`rec->unk11` stores,
   removing them as named pseudos entirely.

Per CLAUDE.md's register-identity rule: this is a **STALL**, not a lever
to keep pulling. `register T v asm("$N")` and an asm operand constraint
are the banned fixes; neither was used or considered viable here since
this is ordinary C control flow, not an unrepresentable GTE/COP2 access.

## Body as reached (172/172 length, 122/172 words, register-identity residue only)

```c
/* New struct field, already applied to the shared Entry90902E8 definition
 * in src/psyq/libsnd_seqread.c (not part of this #if 0 body -- listed here for
 * completeness of what this stall depends on):
 *
 *   u8 unk11;   +0x11: cached MIDI-style running-status byte (0xFF standing
 *               in for a 0xF0 "meta" status) -- read back to interpret a
 *               later event byte that has its high bit clear
 *
 * Forward declarations needed because this function dispatches to siblings
 * defined later in the same unit's ROM-address order:
 *   extern void NoteOn(s16 a0, s16 a1, s32 a2, s32 a3);
 *   extern void SetProgramChange(s16 a0, s16 a1, u8 a2);
 *   extern void _SsSetControlChange(s16 a0, s16 a1, u8 a2);
 *   extern void SetPitchBend(s16 a0, s16 a1);
 *   extern void GetMetaEvent(s16 a0, s16 a1, u8 a2);
 */
void GetSeqData(s16 a0, s16 a1)
{
    Entry90902E8 *rec = &_ss_score[a0][a1];
    u8 *p;
    u8 raw;
    u8 note, vel;

    p = rec->unk4;
    rec->unk4 = p + 1;
    raw = *p;
    if (raw & 0x80) {
        rec->unk12 = raw & 0xF;
        switch (raw & 0xF0) {
        case 0x90:
            p = rec->unk4;
            rec->unk11 = 0x90;
            rec->unk4 = p + 1;
            note = *p;
            rec->unk4 = p + 2;
            vel = *(p + 1);
            rec->unk88 = ReadDeltaValue(a0, a1);
            NoteOn(a0, a1, note, vel);
            return;
        case 0xB0:
            p = rec->unk4;
            rec->unk11 = 0xB0;
            rec->unk4 = p + 1;
            note = *p;
            _SsSetControlChange(a0, a1, note);
            return;
        case 0xC0:
            p = rec->unk4;
            rec->unk11 = 0xC0;
            rec->unk4 = p + 1;
            note = *p;
            SetProgramChange(a0, a1, note);
            return;
        case 0xE0:
            rec->unk11 = 0xE0;
            rec->unk4 = rec->unk4 + 1;
            SetPitchBend(a0, a1);
            return;
        case 0xF0:
            p = rec->unk4;
            rec->unk11 = 0xFF;
            rec->unk12 = raw & 0xF;
            rec->unk4 = p + 1;
            note = *p;
            GetMetaEvent(a0, a1, note);
            return;
        default:
            return;
        }
    } else {
        switch (rec->unk11) {
        case 0x90:
            vel = *rec->unk4;
            rec->unk4 = rec->unk4 + 1;
            rec->unk88 = ReadDeltaValue(a0, a1);
            NoteOn(a0, a1, raw, vel);
            return;
        case 0xB0:
            _SsSetControlChange(a0, a1, raw);
            return;
        case 0xC0:
            SetProgramChange(a0, a1, raw);
            return;
        case 0xE0:
            SetPitchBend(a0, a1);
            return;
        case 0xFF:
            GetMetaEvent(a0, a1, raw);
            return;
        default:
            return;
        }
    }
}
```

## Round 33 update (runner bravo): re-verified, extended permuter search confirms the round-31 plateau

Re-verified: 122/172, length exact, the same `$s3`/`$s4` register-identity
swap unchanged.

**Ran a longer permuter search (400s, up from round 31's 240s) against a
freshly-built scaffold, sanity-checked first.** `--debug --stack-diffs`
reproduced round 31's own base score exactly (773: 49 register differences,
8 stack-difference points, 2 reorderings, 2 insertions, 2 deletions),
confirming the scaffold is sound. The extended search reached ~51841
iterations (essentially the same iteration count as round 31's 240s run,
~52700 -- this machine's per-iteration cost for this function apparently
dominates over the extra wall-clock budget) and again plateaued at **best
score 150**, identical to round 31's figure. No zero, and no improvement
over round 31 despite the longer budget. This is now two independent search
runs (different sessions, same 150 floor) agreeing that this whole-function
register-coloring swap is not reachable by this permuter scaffold at this
search depth -- consistent with `docs/MATCHING-GUIDE.md`'s framing that a
whole-function register-color swap is a categorically harder target than
the single-instruction residues the permuter is proven to close.

**Considered, not attempted: the "dead-value reuse" lever that closed 17
words on `NoteOn` this same round.** That lever needs a value that
goes genuinely dead at some point so its storage can be reused for another
value without adding a competing allocno. This function's parameters (`a0`,
`a1`) are both live for the ENTIRE function body (every case reads `rec` via
`a0`/`a1`, and four of five case bodies pass one or both straight through to
a callee) -- there is no dead parameter or dead intermediate to reuse the way
`NoteOn`'s unused 3rd parameter or dying `speed` local provided.
Confirmed by inspection rather than by trying and failing: this lever's own
documented discriminator ("a local that exists ONLY to carry one branch's
result to a single later use, with a provably dead parameter in scope") is
not satisfied here, so applying it would be guessing rather than following
the lever's stated applicability.

### Round 33 lever checklist (this function)

- **Lever 1 (narrow `volatile`)**: not tried this round -- no fold/fusion to
  defeat; the residue is pure whole-function register coloring, and this
  project's `volatile` lever is documented to act on ONE access, not on an
  allocator's global coloring decision.
- **Lever 2 (register-identity verdict is a hypothesis)**: **checked, DID
  NOT apply.** Looked specifically for the scratch-value-reuse discriminator this round
  already applied successfully on `NoteOn` and found no candidate -- see above. The verdict
  stands as filed: a genuine register-identity STALL, not a mislabeled
  simpler defect.
- **Lever 3 (emission order != source order)**: not newly tested; the six
  reshapes already on file (aliasing, declaration order x4, cursor-folding,
  shared-variable reuse, type widening, low/high inlining) cover this axis
  and none moved the swap.
- **Lever 4 (permuter negative is evidence about one search)**: applied --
  ran a SECOND independent search (longer budget) rather than resting on
  round 31's single negative. It reproduced the same 150 floor, which
  upgrades the finding from "one search didn't find it" to "two independent
  searches at comparable depth agree it isn't shallow", though per the
  project's own rule this still is NOT "permuter-exhausted" -- a
  qualitatively different search (larger `-j`, a different scaffold framing,
  or hours rather than minutes) remains untried.
- **Lever 5 (asm-differ/permuter compare text)**: not implicated; the
  residue is confirmed register-identity via register-symbol-only diffing,
  not an opcode/immediate text ambiguity.

### Proposed learning

The project's documented block-order class ("an arm that must JUMP has to
be written NOT-LAST") applies to `switch` statement CASE ORDER exactly as
it does to if/else arm order: GCC 2.6.3 builds the compare tree from the
sorted case values regardless of source order, but lays out the case
BODIES in source declaration order. Writing a sparse switch's cases in
the wrong order costs the same block-order penalty as a misordered
if/else chain, and is easy to miss because the compare instructions
(which most reviewers check first) can still look identical -- it is the
downstream branch OFFSETS that diverge.

## Round 39 update (runner alpha): re-verified; hoist-both-before-either checked, not applicable

Rebuilt the preserved near-miss body: reproduces exactly, 122/172, length
exact, the same `$s3`/`$s4` whole-function register-color swap unchanged
from rounds 31/33.

**Hoist-both-before-either: not applicable.** This residue is a
whole-function register-coloring swap between two live-for-the-entire-body
parameters (`channel` and the per-case data byte), confirmed by
`asm-differ` register-symbol diffing to be ~40-odd `move`/`sw` instructions
all showing the SAME pair swapped, never a reordered load or multiply pair.
There is no adjacent-load-pair shape anywhere in this residue for the lever
to act on -- consistent with round 33's own finding that the dead-value-
reuse lever (which DID apply to this round's `NoteOn`/
`Snd_setVabAttr`) has no candidate here either, since both `a0`/`a1` are
live across the entire function body with no dead parameter or dying local
to reuse.

Given two independent permuter searches already on file (rounds 31 and 33,
both plateauing at score 150 with no zero across ~52000-104000 combined
iterations) and no new axis found this round, not re-running a third
search against the same categorically-hard whole-function swap; budget
went to functions with fresh permuter history instead (`Snd_setVabAttr`,
below, which the search DID move). `INCLUDE_ASM` unchanged,
`build-and-verify.sh` confirmed byte-exact.

## Round 49 update (runner charlie): third permuter search (163644 iterations, rc=124), same 150 floor; two candidates disqualified as correctness bugs, one safe candidate real-oracle-tested and negative

Rebuilt the preserved near-miss body in isolation first: 122/172, length
exact, the same `channel`($s3 built)/data-byte($s4 built) whole-function
register-color swap unchanged.

**Motivation for a third search despite rounds 31/33's identical 150
floor**: this round's head broadcast noted (from `Snd_setVabAttr`'s own
zero-at-iteration-149 result on a scaffold that had plateaued for 33450
iterations two rounds ago) that decomp-permuter is stochastic and a big
iteration count on an old search is not proof the space is empty --
"the seed axis was real and under-priced". Same scaffold shape as rounds
31/33 (this function's own body has not changed since), so this is a
fresh-SEED test of that exact finding, not a fresh-body one.

**Check 3**: fresh scaffold, base score 773 (49 register differences, 8
stack differences, 2 each of reorderings/insertions/deletions) -- exact
match to rounds 31 and 33's own figures. AGREE; searched with confidence.

**Search: `timeout 900 -j 8 --stop-on-zero --best-only`, rc=124 (900s
bound), 163644 iterations** -- roughly 3x rounds 31+33's combined
iteration count in one run. **Best score: 150 -- the IDENTICAL floor as
both prior searches, not a new low.** Unlike `Snd_setVabAttr`, a bigger
sample from a fresh seed did NOT find anything rounds 31/33 had missed
here; the "seed axis" finding does not guarantee a payoff, only that it
is worth checking.

**Four candidates beat the base score; two are disqualified as genuine
correctness bugs by inspection (never built), one is safe but scores
worse, and the best (150) mixes a safe hunk with a buggy one:**

- **Score 150 (the best)**: three hunks. Two are behaviorally-inert
  syntactic rewrites (`p+2` split into `p; ...+2`, `x=x+1` rewritten
  `x+=1`). The THIRD **reorders `rec->unk4 = rec->unk4 + 1;` to run
  BEFORE `vel = *rec->unk4;` instead of after**, in the case-0x90
  running-status arm. This is NOT behaviorally neutral: retail (and the
  current C) reads `vel` from the OLD `unk4`, then advances the cursor;
  the candidate advances FIRST, so `vel` would read the byte AFTER the
  intended one -- a genuine logic bug the scorer cannot see, matching
  this round's echo/delta forward-trace UB class exactly. **Disqualified
  by inspection, not built.**
- **Score 445**: duplicates `rec->unk4 = rec->unk4 + 1;` (advances the
  cursor by 2 instead of 1) in the `case 0xE0` arm, AND deletes the
  cursor advance entirely from the case-0x90 running-status arm. Both
  are correctness bugs (wrong cursor position on every subsequent event
  read). **Disqualified by inspection, not built.**
- **Score 600**: introduces `new_var = rec->unk4 + 1;` placed
  immediately after `switch (rec->unk11) {` and BEFORE the first `case`
  label -- in C, control jumps straight to the matching case label, so
  this statement is unreachable code and `new_var` is READ
  UNINITIALIZED at its one use (`rec->unk4 = new_var;`). Textbook
  use-before-init, the second of this round's two named UB screens
  (echo's broadcast). **Disqualified by inspection, not built.**
- **Score 745 (safe, tested)**: caches `a1` into the already-declared
  `note` local (`note = a1;`, unread anywhere else on this path) right
  before the case-0x90 arm's `ReadDeltaValue(a0, a1)` call, then passes
  `ReadDeltaValue(a0, note)` instead. `note` is otherwise never assigned
  or read on this control-flow path, so this is behaviorally identical
  to the baseline. Translated to the real function and rebuilt: **118/172
  -- 4 words WORSE than the 122/172 baseline**, with 172/172 length
  unchanged (a same-length, real-oracle-verified regression, not a
  scaffold artifact). Reverted immediately; `build-and-verify.sh`
  reconfirmed byte-exact.

**Disposition**: still 122/172, same residue, same 150-score permuter
floor across three independent searches now (rounds 31, 33, 49) totaling
over 300000 iterations combined. `INCLUDE_ASM` unchanged,
`build-and-verify.sh` confirmed byte-exact throughout (the one real-build
test was reverted before this report was written).

### Proposed learning

The head's round-49 finding that "a big iteration count is one sample,
not proof the space is empty" is correct and worth checking, but this
function is the COUNTEREXAMPLE that keeps it from being read as "always
worth a fresh seed": a 163644-iteration fresh-seed search here reproduced
the identical 150 floor rounds 31 and 33 already found at a combined
~156000 iterations, rather than finding something new the way
`Snd_setVabAttr`'s fresh seed did. The two results compose as the head's
own broadcast already said they would ("where it DID beat base but never
zeroed, a fresh seed is cheap and worth one more run") -- cheap and worth
trying is not the same as likely to pay off, and this function's
whole-function (10-case, ~40-instruction) register-color swap may simply
be too large a search space for decomp-permuter's mutation set to escape
a 150-score local optimum regardless of seed, unlike `Snd_setVabAttr`'s
much smaller (one call, one block) swap.

**NON_MATCHING body promoted, round 66** (runner charlie).

## Round 97 types pass (echo)

code_179d8_k's local `Entry90902E8` view retired onto `include/SsScore.h`:
the same 0xAC-byte (`SS_SEQ_TABSIZ`) `_ss_score[access][seq]` record that
libsnd_cres, libsnd_decre and libsnd_vmanager already use. The header gained
this unit's fields by splitting padding (no offset, size or existing type
moved); field names stay offset-only (`unkNN`) as the header's convention for
Sony-only fields, with each one's mechanics in its comment. The unit's
`(u8 *)rec + unk12 + 0x17/0x2C` and `(s16 *)((u8 *)rec + 0x4E + ch * 2)`
arithmetic became the header's per-channel arrays `unk17[16]` (pan),
`unk2C[16]` (program) and `unk4E[16]` (volume): `unk12` is the event's MIDI
channel (GetSeqData stores a status byte's low nibble), not a byte offset to
an "embedded state block" as the old local comment read it. Byte-exact
unchanged; the NON_MATCHING object is identical too (objdump of
`build/nonmatching/src/code_179d8_k.c.o` before/after).
