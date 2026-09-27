# SsUtChangePitch -- STALL: length EXACT at 88/88 (no drift); 84/88 raw word-match; first real diff at in-range word 49 (vram 0x80031B08) -- HEAD SALVAGE round 31, reconfirmed rounds 32 and 36

> Renamed from `func_80031A44` on 2026-09-24 (tools/rename.py). Address 0x80031a44.

**CONTRADICTS THE TITLE -- REBUILD BEFORE USING EITHER FIGURE (flagged by
the head, round 57, not resolved).** The note below is undated and states
length "87 words, ONE word short" with "51/88 words match", while the title
above states length EXACT 88/88 with 84/88 raw. Both cannot describe the
same body. The title carries the round-31/32/36 reconfirmation, so the note
most likely predates it and was never retired -- but nobody has re-measured,
and this head did not either. PARALLEL-RUNS.md §3.3 screen 4 ranks from
TITLE lines only, so a stale paragraph ABOVE a correct title is invisible to
the screen and is read as current by a human. Found by sweeping for this
shape after runner bravo hit the identical pathology on `SpuVmSetSeqVol`,
where the stale note had stood for 34 rounds.

**Measurement note (both numbers, not conflated -- this is the one to
read carefully):** compiled LENGTH is 87 words against retail's 88 (ONE
word short), and funcdiff's "51/88 words match" is a SEPARATE, raw
hex-equality count that does NOT by itself say how much of the 37-word
gap is caused by that one missing word shifting everything after it
sideways versus how much is independent residue. Checked with
`tools/asm-differ/diff.py`, which realigns the two instruction streams:
**every single word from the top of the function through file offset
0x022320 (retail vram 0x80031B20) matches exactly** once realigned --
that is 46 of the function's 47 leading words. The realigned FIRST real
content difference is the missing `nop` itself, at retail address
`0x80031B24`. Everything after that point in the raw (non-realigned) diff
-- the bulk of the 37 "different" words funcdiff reports -- is the SAME
one-word shift echoing through every subsequent branch-target immediate,
not a second class of residue. **So the correct characterisation is: ONE
missing instruction, not thirty-seven wrong ones.** This is a strong
near-miss and a good permuter target precisely because of that, which the
original draft of this report (correctly identifying the missing `nop`
but stating the two numbers -- "51/88" and "off by exactly one word" -- in
the same breath without saying how they relate) left ambiguous.

Unit `code_179d8_j`, round 23 (2026-09-07). Not a class method. Bounds-checks
`idx`, validates it against three parallel 52-byte-stride records
(`D_8008D99E`/`D_8008D99A`/`D_8008D994`, same family as `SsUtChangeADSR`/
`SsUtKeyOff`), then on a full match calls `func_80032148`, sets the
"current channel" mode globals, copies a per-channel byte into
`D_8008EA18`, stores `note2pitch2`'s return into a 16-byte-stride table,
and ORs a flag bit into `_svm_sreg_dirty`.

## What it is

> ## STOP -- THE `#if 0` BLOCK BELOW IS THE **SUPERSEDED** BODY. DO NOT SPLICE IT.
>
> **The current figures are in this file's TITLE: length EXACT at 88/88, no
> drift, 84/88 raw word-match.** They come from the round-31 head salvage and
> were reconfirmed by a stale-symbol rebuild in round 36.
>
> The body immediately below is the OLDER one: 87/88 words (one word SHORT),
> 51/88 raw match. Spliced as-is it does not reproduce even that -- it lands
> around 42/88 with large out-of-range drift, because it does not splice
> cleanly at all (see "HEAD, round 36 -- the salvage body's NAMES are fixed;
> it still does not splice, and the reason is a THIRD failure mode", near the
> end of this file).
>
> **This warning is here because the trap fired again in round 38.** Runner
> charlie, explicitly briefed to rebuild every inherited figure before
> trusting it, spliced this block, measured 42/88, compared it against the
> `51/88` in the heading this warning replaced, and read the result as an
> unexplained regression. It reverted immediately and spent no further budget,
> which was the right call -- but it had already paid for the detour.
>
> The mechanism is structural, not a reading failure: **the superseded body is
> the FIRST `#if 0` in the file**, and `#if 0` is the project's mandated
> preservation form, so the block a reader or a tool reaches first is the
> wrong one. The correction lived 500 lines below it. See
> `docs/DECOMPILATION_LEARNINGS.md`, "A report's MANDATED preservation form
> can point at the WRONG body (round 36)".

**Superseded body (87/88 words compiled, one word SHORT of retail's length;
51/88 raw match; 46/47 leading words exact once realigned). Kept for its
derivation only -- it is not the best-reached body and it does not splice.**

```c
#if 0
/* Round 34's carve dropped these three from code_179d8_j_c.c. Runner delta
 * recovered them into libsnd_vm_vol_ut_key_ut_keyv.c for ITS two stalls and did not carry
 * them here, so this body was still un-compilable after its names were
 * corrected: `D_8008EA22'/`D_8008EA26'/`D_8008EA18' undeclared. Carried
 * explicitly so the body is self-sufficient, per CLAUDE.md's requirement to
 * inline a preserved body "with every declaration it needs". */
extern volatile u16 D_8008EA26;
extern volatile u8 D_8008EA18;
extern u16 D_8008EA22;

extern s16 note2pitch2(u16 a0, u16 a1);

s32 SsUtChangePitch(s16 idx, s16 p1, s16 p2, s16 p3, s32 unusedP, u16 p4, u16 p5)
{
    s16 v1, v2, v3;

    if ((u16) idx >= 0x18)
        return -1;
    v1 = D_8008D99E[idx].unk0;
    if (v1 != p1)
        return -1;
    v2 = D_8008D99A[idx].unk0;
    if (v2 != p2)
        return -1;
    v3 = D_8008D994[idx].unk0;
    if (v3 != p3)
        return -1;
    SpuVmVSetUp(p1, p2);
    D_8008EA22 = 0x21;
    D_8008EA26 = idx;
    D_8008EA18 = *(u8 *) &D_8008D99C[idx].unk0;
    D_8008D7F4[idx].unk0 = note2pitch2(p4, p5);
    _svm_sreg_dirty[idx] |= 4;
    return 0;
}
#endif
```

## Two CLOSED findings

### 1. A wholly-unused STACK parameter, discovered from a fixed-offset mismatch, not a register gap

Retail reads its two stack arguments (`p4`, `p5`, both `u16`) from
`0x3c($sp)`/`0x40($sp)` against a measured `-0x28` (40-byte) frame. The
naive o32 formula for a 4-register-argument function with that frame size
puts the first stack argument at `0x38($sp)` (`frame_size + 0x10`) --
retail's is 4 bytes further out. This is the SAME "silent ABI waste" idiom
already documented for a wasted REGISTER argument (`SpuVmSeKeyOn`,
`func_800319B4`'s reports) but here the wasted slot is on the STACK: the
real source has a fifth argument, between `p3` and `p4`, that is a plain
32-bit value (`s32 unusedP` here) never referenced anywhere in the body.
Adding it as a 5th parameter (pushing `p4`/`p5` to the 6th/7th argument
positions) reproduces retail's exact `0x3c`/`0x40` offsets. Before this fix
the function measured 49/88 with FIVE single-bit-different words scattered
through the body (every embedded branch-target address was off by
one nibble because the two loads were being generated at 4-byte-different
positions than retail's, which is a subtler symptom than the usual
"whole function shifted" -- worth remembering that a handful of low-nibble-
only diffs on branch targets, not just outright missing/extra instructions,
is what an argument-offset mismatch looks like).

**Corollary for stall triage:** don't assume "arg 4 unused" (register case)
and "arg N unused" (stack case) are the same lever just because they're
both "silent ABI waste" -- the register case is diagnosed from the FINAL
argument's register number; the stack case is diagnosed from a FIXED
OFFSET disagreement against the `frame_size + 0x10` formula, and is easy to
miss because the function still builds and even scores plausibly (49/88)
without it.

### 2. `if(0){dead[..]=1;}`/scalar dead-local tricks do NOT reproduce this gap, and are not the same lever

Before finding the unused-parameter explanation, a scalar `s32 dead;` and
a one-element `s32 dead[1];`, both guarded by an unreachable `if (0) { ... }`
statement (this unit's established idiom for forcing a frame on an
otherwise-frameless leaf, see `SsUtChangeADSR`'s report), were tried here and
had **zero effect** on the stack-argument offsets -- this function already
has a real, non-zero frame from five callee-saved registers plus `$ra`, and
the dead-local trick appears to only matter for sizing a frame that
would OTHERWISE be zero. Once a genuine frame already exists from register
saves, a small dead local does not visibly grow it further (at least not at
this size); the real fix was an actual extra ARGUMENT, not a local.

## The unresolved residue: one missing `nop`

With the above two fixes in place, the compiled function is 87 words
against retail's 88 (one word short), and -- per the measurement note at
the top of this report -- `tools/asm-differ/diff.py` confirms that single
missing word accounts for essentially the ENTIRE gap: every word up to
retail address `0x80031B20` matches once realigned, and the residue is
precisely one missing `nop` late in the body: retail's `D_8008EA18`
byte-copy sequence is

```
lui  $at, %hi(D_8008D99C)
addiu $at, $at, %lo(D_8008D99C)
addu $at, $at, $s0
lbu  $v0, 0x0($at)
nop
lui  $at, %hi(D_8008EA18)
sb   $v0, %lo(D_8008EA18)($at)
```

-- this build's compiled form omits the `nop` between the `lbu` and the
following (unrelated, non-dependent) `lui`. It is not a MIPS-I load-delay
hazard (the next instruction doesn't read `$v0`), so it reads as a pure
scheduler artifact: retail's scheduler had nothing to fill that slot with
and left a `nop`; this build's scheduler filled it with something (net
zero -- one fewer instruction, not a different one). Tried and rejected:

- A bare `__asm__("");` immediately after the `D_8008EA26 = idx;` statement
  (before the byte copy) -- regressed to 45/88.
- A bare `__asm__("");` between a temporary holding the loaded byte and its
  store to `D_8008EA18` (splitting the one-line read-then-store into two
  statements with an explicit `u8 tmp` local) -- regressed to 41/88.
- Rewriting the address computation as `((u8 *) D_8008D99C)[idx * 0x34]`
  instead of `*(u8 *) &D_8008D99C[idx].unk0` -- **no effect**, byte-identical
  output; both spellings compile the same way.
- Passing `func_80032148`'s two arguments as the raw parameters (`p1`, `p2`)
  instead of the just-validated record fields (`v1`, `v2`) -- **no effect**,
  confirming (a second time, after `SsUtChangeADSR`'s report) that GCC 2.6.3's
  cse pass treats a value proven equal by an `if (x != y) return;` guard as
  fully interchangeable for a later call, regardless of which name the
  source uses.

Per CLAUDE.md's rule this is squarely a STALL: a single-instruction
scheduling gap that every legal reshape tried either left unchanged or
made worse, never better.

## Axes tried (for the next attempt)

- Stack argument layout: naive 6-parameter signature (49/88, offset
  mismatch) vs. 7-parameter with a wasted 5th arg (51/88, offsets fixed) --
  **closed**.
- Frame sizing: scalar dead local, one-element dead array, both under
  `if (0)` -- **no effect** once a real register-save frame already exists.
- Scheduling around the missing `nop`: barrier before the byte copy,
  barrier mid-way through a split byte copy, alternate address-computation
  spelling, alternate `func_80032148` argument spelling -- **no effect or
  regression** in all four cases tried.

**Untested axis:** whether reordering the THREE preceding record checks
(`D_8008D99E`/`D_8008D99A`/`D_8008D994`) changes downstream register
liveness enough to reintroduce the `nop` as a genuine scheduling
consequence, the way it did for `SsUtSetDetVVol`'s (unresolved) delay-slot
residue. Not attempted this round for lack of remaining budget.

### Proposed learning

**A handful of branch-target words differing only in their low nibble,
scattered through an otherwise-matching function, is the signature of a
STACK-ARGUMENT OFFSET mismatch, not a scattered set of independent
residues.** It looks superficially like "several small unrelated bugs"
(five separate diff lines, none adjacent) but collapses to one cause (a
missing stack parameter) the moment you check the fixed-offset arithmetic
(`frame_size + 0x10 + 4*stack_arg_index`) against what retail actually
reads. Worth checking before spending attempts on each diff line
individually.

**Second, and this is the one that cost a head review cycle:** "N words
short" (a LENGTH measurement) and "M/N words match" (a raw hex-equality
count) are different measurements that read as if they agree when they
don't. This report's own first draft said both "51/88" and "off by
exactly one word" in the same sentence, which is true but leaves the
READER to guess whether the 37-word gap is mostly the one-word shift
rippling through every later branch target (it is, confirmed with
`tools/asm-differ/diff.py`) or a second, independent pile of residue (it
is not). State which one it is, in the report's title line -- that is the
only place a busy reader is guaranteed to look.


---

## HEAD SALVAGE -- round 31 (2026-09-11). THIS SUPERSEDES THE TITLE FIGURES ABOVE.

Runner delta was stopped by the orchestration (its session ended while a
permuter search was still running and could not be resumed) with this
function live as C in its worktree and **never committed**. The head
recovered the body per docs/PARALLEL-RUNS.md 4c, scored it in `main`, and
restored the `INCLUDE_ASM`.

**Label: this is a MID-ATTEMPT SNAPSHOT, not a considered plateau.** No
author applied a stop rule to it and delta never reported on it, so it
carries none of the "this is as far as reshaping got" implication a
normally-preserved body does.

### Measured by the head, in `main`, this round

| measurement | value |
| --- | --- |
| build | clean compile, `build exit=2` (ordinary SHA1 mismatch), zero grep hits |
| length | **EXACT, 88/88 words** -- funcdiff reported NO out-of-range bytes |
| raw word-match | **84/88** |
| first real diff | in-range word 49, file 0x022308 / vram 0x80031B08 |

**The length gap that defined this function's whole prior history is
CLOSED.** Every earlier figure here (87/88 length, 51/88 raw) described a
one-word-short body whose low word-match was mostly shift ripple. This body
is exact length, so its 84/88 is a real 4-word residue and is NOT
comparable to the old 51/88.

### What the four residual words are

Two adjacent effects, both instruction PLACEMENT rather than value or
control flow:

```
word 49  off=0x022308 vram=0x80031B08  retail=22ea22a4  built=26ea32a4
word 51  off=0x022310 vram=0x80031B10  retail=26ea32a4  built=22ea22a4   <- swapped pair
word 56  off=0x022324 vram=0x80031B24  retail=00000000  built=21288002
word 60  off=0x022334 vram=0x80031B34  retail=21288002  built=00000000   <- moved across a nop
```

Words 49/51 are the same two instructions in the opposite order. Words
56/60 are one instruction (`21288002`) that retail places four words later
than the build does, with a `nop` filling the other slot. That is a
scheduling/code-motion residue at exact length -- the shape a permuter
closes -- and it is explicitly NOT a register-identity residue, so it is
not in banned territory.

### The body, as literal source

Positioned where it would compile: immediately after `func_800319A4`'s
closing brace and before the `extern Rec16D7F0 D_8008D7F8[];` block, in
strict ROM-address order. The declarations it needs already exist in
`src/code_179d8_j.c` at the lines noted; they are repeated here so the body
travels complete.

```c
/* Declarations this body needs (already present in the unit): */
typedef struct {
    s16 unk0; /* +0x0 */
    u8 pad2[0x34 - 0x2];
} Rec34D994;

typedef struct {
    s16 unk0; /* +0x0 */
    s16 unk2; /* +0x2 */
    u8 pad4[0x10 - 0x4];
} Rec16D7F0;

extern u16 D_8008EA22;
extern volatile u16 D_8008EA26;
extern volatile u8  D_8008EA18;
extern Rec34D994 D_8008D994[];
extern Rec34D994 D_8008D99A[];
extern Rec34D994 D_8008D99C[];
extern Rec34D994 D_8008D99E[];
extern Rec16D7F0 D_8008D7F4[];
extern u8 _svm_sreg_dirty[];
extern void SpuVmVSetUp(s16 p1, s16 p2);   /* was func_80032148 until round 34 */
extern volatile u16 D_8008EA26;
extern volatile u8 D_8008EA18;
extern u16 D_8008EA22;
extern s16  note2pitch2(u16 p4, u16 p5);

/* The salvaged body -- scored 84/88, exact length: */
s32 SsUtChangePitch(s16 idx, s16 p1, s16 p2, s16 p3, s32 unusedP, u16 p4, u16 p5)
{
    s16 v1, v2, v3;

    if ((u16) idx >= 0x18)
        return -1;
    v1 = D_8008D99E[idx].unk0;
    if (v1 != p1)
        return -1;
    v2 = D_8008D99A[idx].unk0;
    if (v2 != p2)
        return -1;
    v3 = D_8008D994[idx].unk0;
    if (v3 != p3)
        return -1;
    SpuVmVSetUp(p1, p2);
    D_8008EA26 = idx;
    D_8008EA22 = 0x21;
    D_8008EA18 = *(u8 *) &D_8008D99C[idx].unk0;
    D_8008D7F4[idx].unk0 = note2pitch2(p4, p5);
    _svm_sreg_dirty[idx] |= 4;
    return 0;
}
```

### Next round

This is now one of the best permuter targets in the corpus: exact length,
four words, all of them instruction placement. Seed the search from the
body above rather than from anything earlier in this report -- the earlier
bodies are a word short and start from a strictly worse place.

## ROUND 31 (runner delta): correction, and the salvaged body IS the search that was live

**One factual correction to the salvage note above: the session was not
stopped and did not need resuming.** The "84/88, exact length" state the
head recovered was not an abandoned mid-attempt sitting idle -- it was
this same runner's own live work, captured mid-flight while a bounded,
`--stack-diffs`-correct permuter search against exactly that body was
still running in the background (per this project's own guidance that
such a search is background work and not a reason to end a turn). The
head's own measurements (88/88 length, 84/88 raw, first diff at word 49)
are exact and match this runner's independent measurement of the same
state a few minutes earlier, so nothing about the RESULT needed
correcting -- only the narrative of how it came to be sitting there
uncommitted.

**The search the salvage note calls for was already run, from this exact
body, and did not close it.** ~9,087 iterations, `-j 4`, `--stack-diffs`
(confirmed present -- the first attempt, without the flag, was caught and
discarded per this project's own trap warning before any score from it
was trusted), wall-clock-limited to 600s via `timeout` (the run ended on
its own within that budget; no PID cleanup was required for a runaway
`-j`4 forkserver, verified by `pgrep` against this worktree's `cwd`
afterward). Best score found: **150, i.e. the base itself -- no
candidate anywhere in the run beat the 84/88 state already reached by
hand.** One superficially-better-scored candidate (130, then later
recurring at the same score) turned out on inspection to be the
IDENTICAL function body, differing only in unused local-declaration order
and in the permuter's own randomizer dropping the `volatile` qualifier
off a local re-declaration of `D_8008EA18` that has nothing to do with
the four residual words (that global is only ever WRITTEN in this
function, never re-read, so `volatile` cannot affect its codegen here) --
not a real lead, and not adopted.

**Per this project's phrasing rule, this is "not closed in ~9,000
iterations under load," not "permuter-exhausted."** Four other runners'
permuter searches were independently confirmed live on the same host
during this window (`ps aux` showed workers rooted in
`lsddecomp2-wt-charlie`, `lsddecomp2-wt-echo`, and `lsddecomp2-wt-alpha`,
scoped-killed rather than touched), so throughput here was shared, not
dedicated, and a longer or less-contended run could still find the
remaining four-word placement fix. **Also worth recording plainly: the
two swapped words (49/51) are `D_8008EA22`/`D_8008EA26`'s store order
reversed relative to retail's own literal source order** (retail:
`D_8008EA22` first per the raw `.s`; the 84/88 body has `D_8008EA26`
first) **yet the reversed order is what scores better overall** -- a
direct instance of this unit's already-documented "independent global
stores get freely reordered/batched by GCC unless barred" finding
(`SsUtKeyOn`'s finding 3), here cutting the OTHER way: matching
retail's own writing order for these two statements, verified as the
FIRST body ever tried on this function (41/88, with drift), scored worse
than reversing it. A `__asm__("")` barrier between the two statements
(forcing the reversed C order to actually emit in that order rather than
let the scheduler re-decide) was not tried this round; it is the most
promising untested next lever, ahead of a longer permuter run, since it
targets a mechanism this unit's own reports already have precedent for.

**Restored to `INCLUDE_ASM`** (already done by the head's salvage;
confirmed still true and the whole-image build still green after this
round's additional, unsuccessful search). Classification unchanged: STALL,
scheduling/code-motion residue, not register-identity, not banned-lever
territory -- a genuine permuter target for whichever round has spare
search budget.

## ROUND 32 (runner alpha): bounded permuter search, ~6x round 31's iteration count, same result

Seeded the permuter directly from the round-31 salvaged body (the "84/88,
exact length" state above) via `tools/setup-permuter.sh`. The scaffold's
own base-score sanity check reproduced **base score = 150** with
`--debug --stack-diffs`, and the debug diff it printed matches this
report's own four-word residue description exactly: the two swapped
`D_8008EA22`/`D_8008EA26` stores (words 49/51) and the `move a1,s4` /
`nop` pair around the `note2pitch2` call (words 56/60) -- confirming the
seed is scoring the right function in the right state before spending any
search budget.

Ran the search itself under `timeout 600` (a hard wall-clock bound, per
this round's instructions), `-j 4` (chosen slightly below the `-j 6` other
runners' concurrent searches were using on this same host, to avoid
starving them), `--stack-diffs` (confirmed present in the invocation, not
omitted the way round 31's first, discarded attempt did):

```
timeout 600 env PATH=.../permuter-work/bin:$PATH \
  .venv/bin/python3 tools/decomp-permuter/permuter.py -j 4 --stack-diffs \
  --stop-on-zero --best-only permuter-work/SsUtChangePitch
```

**Reached iteration 54507 before the wall-clock bound ended the run** --
about 6x round 31's ~9087 iterations (this run had less contention: only
two other runners' searches were confirmed live on the host at various
points during this window, against four during round 31's). **No
iteration scored below 130, and no iteration reached 0.** The one
sub-150 candidate found (`output-130-1/`, score 130) is confirmed **the
same spurious non-lead round 31 already identified and rejected**: diffed
its saved `source.c` against the seed directly (`diff base.c
output-130-1/source.c`) and it differs only in (a) an unused-local
declaration named `new_var` that aliases `idx` for the `D_8008D99C`
index expression, and (b) the permuter's randomizer dropping `volatile`
from a local re-declaration of `D_8008EA18`'s type -- neither touches any
of the four residual words, both are cosmetic to the compiled output.
Compiled the candidate directly through the pinned pipeline
(`compile.sh output-130-1/source.c -o /tmp/cand130.o`) and confirmed by
inspection it is the same instruction sequence as the base, not a genuine
improvement; the permuter's own internal score (130 vs. 150) reflects its
weighted textual-diff heuristic reacting to the reformatted/expanded C
(this candidate's source is pycparser's own re-serialization, with
braces added to every single-statement `if`, not hand-written), not a
real change in the assembled bytes' distance from retail.

**On which mechanism ended the run:** the background job's own completion
signal reported the wrapping shell's exit as 0, which is expected
regardless of whether `timeout` fired (124) or the search reached some
other stopping condition, because the invocation appended an unconditional
`echo` after the `timeout` command and shell `;`-chaining reports the LAST
command's exit code. That appended diagnostic line did not end up in the
log file for a reason not fully understood (raced against the permuter's
own atexit/semaphore-cleanup output, most likely) -- so this report
cannot independently distinguish "wall-clock timeout fired" from "the
search process ended on its own" from the log alone. What IS certain,
independent of that ambiguity: the run reached 54507 iterations (an order
of magnitude more than round 31's, well past the point where round 31's
smaller sample had already stabilized on the same 130-score plateau), and
no process for this search remained running afterward (`ps` showed none),
so the search is not silently still consuming host resources.

**Verdict: unchanged from round 31.** A ~6x larger, less-contended search
still finds nothing below the same spurious 130 plateau and no zero.
This strengthens (does not just repeat) round 31's own framing -- "not
closed in ~9,000 iterations under load" now reads as "not closed in
~54,000 iterations with less contention" -- but the same caveat applies:
a still-longer or fully-dedicated run remains untried. Restored to
`INCLUDE_ASM` (was already so; the permuter operates in its own
`permuter-work/` scratch directory and never touched `src/`).
`git status --porcelain` is empty; whole-image build re-verified green.
Classification unchanged: STALL, scheduling/code-motion residue at exact
length, not register-identity, not banned-lever territory.

### Proposed learning

**A recurring spurious low-score candidate across independent permuter runs
(here: 130, in both round 31's ~9k-iteration run and this round's
~54k-iteration run) is worth fingerprinting once and then recognizing by
its shape, not re-diffing from scratch each time.** This candidate's
signature -- an added `new_var` alias for an already-live index expression,
plus a dropped `volatile` on a write-only global's local re-declaration --
reproduced identically across two rounds' independent searches, confirming
it is a stable local optimum in the permuter's mutation space rather than
a fluke. Worth a one-line note in this project's permuter tooling docs (not
done here, out of scope for a per-function match report) so the next
runner recognizes it on sight instead of re-verifying it is spurious.

## ROUND 36 (runner delta): stale-symbol rebuild reconfirms 84/88 exactly; one more untested lever tried and rejected

Round 34's SDK-object conversion linked `libsnd/vm_vsu.o` and renamed this
function's callee from `func_80032148` to `SpuVmVSetUp` (confirmed against
`config/symbols.slps01556.lsdde.txt:456`). The round-31/32 salvaged body
preserved above (both the "body, as literal source" section and the two
in-prose call sites) still used the OLD name -- exactly this round's
stale-symbol trap: the 84/88 figure was correct as measured at the time,
but had never been rebuilt since the rename, so it was unverified against
the current tree.

`src/code_179d8_j_c.c` already declares `extern s32 SpuVmVSetUp(s16 a0,
s16 a1);` at file scope (added when this unit was split off in round 34),
so restoring the salvaged body only needed the callee's NAME corrected,
not a new declaration. Rebuilt: clean compile (`build exit=2`, zero grep
hits), **`funcdiff.py` reports 84/88 words, exact length (no drift when
this is the only in-progress edit)** -- an exact reproduction of the
round-31/32 figure. The four-word residue is unchanged: the swapped
`D_8008EA22`/`D_8008EA26` store pair (words 49/51) and the `note2pitch2`
call's argument-setup instruction placed four words later than retail
(words 56/60).

### The one specifically-flagged untested lever: tried, and it is not a lever

Round 31 flagged "a `__asm__("")` barrier between the two [D_8008EA26/
D_8008EA22] statements... the most promising untested next lever, ahead of
a longer permuter run" but never tried it. This round did:

```c
D_8008EA26 = idx;
__asm__("");
D_8008EA22 = 0x21;
```

**Result: catastrophic regression, not a fix.** `funcdiff.py` reported
**42/88** words matching with **272099 bytes differing OUTSIDE the
function's own range** -- a massive length increase, not the targeted
2-word reorder. Removing the barrier (reverting to the plain, unbarred
84/88 body) restored the exact prior state (`build/lsdde.map` confirms
`SsUtChangePitch` and its own length are back to the 84/88 baseline). So the
barrier does not "block a reorder here", it forces the compiler into an
entirely different, much larger code shape for this specific function --
a bare scheduling barrier is not free of side effects on every function,
and this is a case where CLAUDE.md's own test ("if removing it changes
WHICH REGISTER holds a value, it is banned; if it only changes
instruction ORDER, it is allowed") is moot because the barrier here changed
neither in a useful way -- it changed the function's overall SIZE.

No further reshaping attempted this round: the two rounds of permuter
search already on record (round 31: ~9k iterations; round 32: ~54k
iterations, both plateauing at the same spurious 130 candidate and finding
no zero) plus this round's one specifically-flagged lever failing sharply
leaves no untested axis with a plausible chance of closing the residue
cheaply. Restored to `INCLUDE_ASM` (unchanged from prior rounds).
Classification unchanged: STALL, scheduling/code-motion residue at exact
length, not register-identity, not banned-lever territory.

### Proposed learning

**A bare `__asm__("")` scheduling barrier is not risk-free just because it
adds no real instructions.** This project's own rule treats it as always
"allowed" (as opposed to a register-pinning constraint, which is banned),
but "allowed" is not "harmless": on this function it blew the compiled
length up by hundreds of instructions system-wide, for a function that
was otherwise EXACT LENGTH and four words from a match. Test a barrier's
effect on the FULL build (`build-and-verify.sh`'s out-of-range byte count),
not just the targeted words, before treating "try a barrier here" as a
low-cost experiment -- on this evidence it can be the opposite of
low-cost.

## HEAD, round 36 -- the salvage body's NAMES are fixed; it still does not splice, and the reason is a THIRD failure mode

Runner delta reported this function as "84/88 (exact reproduction)" after
correcting `func_80032148` -> `SpuVmVSetUp`. It corrected that name in its
working tree to take the measurement and did not carry the correction into
this report, so the durable artifact still held the un-linkable spelling --
round 35's trap, committed by a runner that had been briefed on it with a
worked example. Both copies are corrected now (the superseded `#if 0` block
and the round-31 salvage fence).

**Two things were found doing that, and the second is new.**

**1. This report defeats any positional rule for "which body is live."** It
holds a superseded 87/88 body in the `#if 0` block near the top and the
authoritative 84/88 salvage body in a fenced block six sections lower. The
project's mandated preservation form is `#if 0`, so a reader -- or a tool --
that trusts the form picks the WRONG body here. `stalesyms.py` briefly had a
rule that did exactly that and cleared this report while its live body was
still stale; the rule was removed the same afternoon. The supersession is
stated in PROSE ("THIS SUPERSEDES THE TITLE FIGURES ABOVE") and nothing
lexical follows prose.

**2. A preserved body can be correctly-named, complete, and STILL not splice,
because the UNIT grew declarations underneath it.** Spliced over the
`INCLUDE_ASM` with its names corrected, this body does not compile:

```
src/code_179d8_j_c.c:132: conflicting types for 'D_8008D99C'
src/code_179d8_j_c.c:86:  previous declaration of 'D_8008D99C'
src/code_179d8_j_c.c:136: conflicting types for 'SpuVmVSetUp'
src/code_179d8_j_c.c:55:  previous declaration of 'SpuVmVSetUp'
```

The body travels with its own `extern` preamble, as CLAUDE.md requires ("with
every declaration it needs"). Round 34's carve and delta's own round-36
recovery have since added declarations for the same symbols to the unit, at
different types. The preamble that made this body self-sufficient in round 31
is now a duplicate-at-conflicting-type against the unit it belongs to.

So the recipe "correct the names and rebuild" is necessary and NOT sufficient.
Three distinct things can be wrong with an inherited body, and they need
different fixes:

| failure | symptom | fix |
| --- | --- | --- |
| stale name | `undefined reference` | rename to the current symbol |
| missing declaration | `'D_8008EA22' undeclared` | carry the declaration in |
| preamble duplicates the unit's | `conflicting types` | RECONCILE -- drop or retype the preamble |

Every one of those three is a `*** [....o]` hit, and only the first produces a
hit on `error:`/`parse error`. The middle and bottom rows are the round-21
table in CLAUDE.md arriving in practice: `undeclared` and `conflicting types`
are fatal and textually silent.

**Not re-measured, deliberately.** Reconciling the preamble means deciding
which type is right for `D_8008D99C` and `SpuVmVSetUp` -- the report's or the
unit's -- and that is a real question about the data layout, not a mechanical
edit. It is the first thing the next runner on this function should settle,
and it is why delta's "84/88, exact reproduction" should be read as measured
against delta's working tree rather than against anything this report can
currently reproduce.

## Track 2 (round 86, 2026-09-26, alpha)

This function is still `INCLUDE_ASM` and its C was not touched, but the per-field symbols this report uses (`D_8008D988`..`D_8008D9BA` at a 0x34 stride) are ONE Sony table: libsnd/vmanager.o's `_svm_voice` (0x8008D988, 24 x 0x34 = 0x4E0 bytes), typed in `include/SvmData.h` with fields by offset (`D_8008D98C` is `_svm_voice[i].unk04`, `D_8008D9A3` is `unk1B`, and so on: address minus 0x8008D988). The next attempt should write `_svm_voice[i].unkNN`: in every converted accessor (libsnd_vm_vol_ut_key_ut_keyv/j_c/l/m/p) the struct spelling compiled byte-identically to the separate symbols, and two NON_MATCHING bodies moved closer to retail. The other `D_` spellings in preserved bodies below still link (splat keeps them as auto-symbols).

## Round 97 (bravo, track 6): the unit banner and comments, moved here

code_179d8_j_c.c took <libsnd.h> and its banner was rewritten as documentation. The history it carried, verbatim:

```c
/*
 * ROUND 42 CORRECTION (2026-09-15) -- READ BEFORE ANY "BLOCKED" LINE BELOW:
 * every claim in this comment that a function is BLOCKED by `gp_rel`,
 * `nop_mflo_mfhi` or `addiu_at` is STALE.  All three constructs are RESOLVED
 * by pinned maspsx flags (CLAUDE.md, "Open toolchain blockers");
 * `tools/nearmiss.py` reports them tagged (RESOLVED-not-a-blocker) and counts
 * none of them.  Any "do NOT spend attempts on these" directive below is
 * therefore RETRACTED: those functions are ordinary matching work, and most
 * carry a mechanism-correct partial derivation already.  The rest of this
 * comment still stands -- only the blocker verdicts are withdrawn.
 * Screen: `python3 tools/nearmiss.py`, round 43 (2026-09-15).
 *
 * code_179d8_j_c -- the TAIL of the old code_179d8_j slice, split off in round
 * 34 (2026-09-12) when Sony's `libsnd/ut_pb.o` was linked into the middle of
 * `libsnd_vm_vol_ut_key_ut_keyv`.  Now 0x22244..0x2273C (vram 0x80031A44..0x80031F3C), eight
 * functions (SsUtChangePitch .. SsUtAutoPan).
 *
 * WHY THE SPLIT EXISTS.  `func_800319B4` is Sony's `SsUtPitchBend`
 * (`libsnd/ut_pb`, Psy-Q 3.6 -- the only disc carrying the module; 0x90 text
 * covering exactly that one function).  It had been MATCHED as C;
 * reclassifying it out of the game count is the correction CLAUDE.md asks for,
 * not a regression.  A placed object cannot live inside a `c` segment, so
 * `libsnd_vm_vol_ut_key_ut_keyv` became [c][o][c] and this half needed its own name.
 *
 * This is the SECOND split of the same original slice in the same round --
 * `libsnd/vm_prog` took 0x20FF0..0x21180 first, which is what created
 * `libsnd_vm_vol_ut_key_ut_keyv`.  Hence the `_c` suffix: `_b` was already taken.  The
 * precedent for a second-generation split name is the yaml's own note on
 * `<unit>_b` / `<unit>_c`.
 *
 * NO RODATA ATTACH CAME WITH THIS HALF, and that is measured, not assumed: the
 * old code_179d8_mid_c monolith contains zero `jtbl_` and zero `.word .L`
 * across its whole extent, and the splat yaml's rodata slot list names none of
 * `code_179d8_j`, `_j_b` or `_j_c`.  Unlike round 33's libsnd_ssinit_libapi_counter there
 * was nothing to move, and a link failure of the form
 * `undefined reference to '.L8003....'` would mean something else.
 *
 * DECLARATIONS: this file carries its own copy of what its functions use,
 * split out of the old shared block.  Keep it that way -- do NOT create a
 * shared code_179d8*.h.  The sibling slices are staffed independently and a
 * shared header is what makes their merges collide; see
 * `python3 tools/headercontention.py`.
 *
 * BLOCKER PROFILE: screen with `python3 tools/nearmiss.py`, never by
 * re-implementing the greps and never for `addiu_at` (resolved round 21).
 * `nearmiss.py` runs `tools/sdkstalls.py` for you.  SsUtChangePitch carries a
 * HEAD SALVAGE body in its report (84/88 words, round 31) -- read the report
 * before starting, it is not cold ground.
 *
 * Expect this slice to span more than one class; identify each with
 * tools/classtable.py rather than assuming the unit has one.  Keep every
 * function in strict ROM-address order.
 */


----
/* ------------------------------------------------------------------------
 * Cross-unit calls, typed per-call-site from the registers loaded before
 * each `jal` -- none of these callees have an established prototype yet, so
 * these are local guesses, not authoritative.  (This note used to add "several
 * are themselves addiu-$at blocked in their own units"; that is stale as of
 * round 21 and was removed rather than left to be believed.)  See CLAUDE.md's note on this.
 * ------------------------------------------------------------------------ */
/* SpuVmKeyOn (round 76, was StartNote) is Sony libsnd/vmanager INTERNAL --
 * unlike SsUtKeyOn (code_179d8_d.c), it has no public prototype in
 * LIBSND.H (grep confirms no `Vm`-prefixed extern anywhere in that
 * header), so this stays the byte-exact local-guess signature rather
 * than a header copy. */

----
/* STALL -- see docs/match-reports/SsUtChangePitch.md.  HEAD SALVAGE, round 31,
 * confirmed round 32 (permuter, ~54k iterations, not closed). Round 36:
 * rebuilt with SpuVmVSetUp's real name (was func_80032148 in the report's
 * preserved body -- round 34's SDK conversion renamed the callee, and the
 * report's body was never corrected). Measured 84/88 words, exact length,
 * matching the prior figure exactly; a barrier between the D_8008EA26/
 * D_8008EA22 stores (the one untested lever the report flagged) blows the
 * function up drastically instead of fixing the swap -- see this round's
 * report update. */
```
