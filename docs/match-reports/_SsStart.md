# _SsStart -- STALL (exact length 164/164 words; 65/164 raw word match; first real diff at vram 0x80032760)

> Renamed from `SeqTimerControl` on 2026-09-23 (tools/rename.py). Address 0x80032708.

> Renamed from `func_80032708` on 2026-09-23 (tools/rename.py). Address 0x80032708.

Unit `libsnd_ssinit`, carved round 16 (2026-09-04). **Attempted, restored
to `INCLUDE_ASM`.** Callee-saved register screen: 2 (`$s0`, `$s1`) --
well under the deprioritisation band; not the blocker (see "Answering
the head's question" below).

## What it does

A CD-audio/root-counter rate-selection dispatcher, called with `arg0`
from `SsStart`/`SsStart2` (`1`/`0`). Busy-waits ~1000 cycles,
then dispatches on the CD status global `_snd_seq_tick_mode`:

- `2`/`3`: fixed device tag `0xF2000002` with a fixed rate constant
  (`0x44E8`/`0x89D0`), then a shared `_snd_use_interrupt_id = 6`.
- `5`: if `arg0 != 0`, sets tag `0xF2000003`, clears `_snd_use_interrupt_id`, rate
  `1`; if `arg0 == 0`, just increments `_snd_use_vsync_cb` and jumps straight
  to the shared teardown-check near the end (skipping the whole
  SetRCnt/delay-loop/callback body below).
- `0`: returns immediately, doing nothing at all (not even the trailing
  `func_80024CF0()`).
- `1` or anything else (default): guarded by `_snd_seq_no_tick`; computes a
  rate via one of two divisions (`0x204CC0/v1` or `0x409980/v1`,
  selected by `v1 < 0x46`) with the classic PSX `div`+`break 7`/`break 6`
  overflow-trap idiom, and only the `< 0x46` branch additionally ORs `2`
  into the device tag and increments `_snd_1per2`.

Then (except the `5`/`arg0==0` and `0` early-outs): if `_snd_use_vsync_cb` is
set, tears down via `func_80024DA0(func_80033738)` and returns; else
calls `func_80024CE0()`, `ResetRCnt(tag)`, `SetRCnt(tag,
(s16)rate, 0x1000)`, two more ~2000-cycle busy-waits, `StartRCnt(tag)`,
then the SAME callback-(re)registration shape already matched in
`SsEnd` (dispatching on `_snd_use_interrupt_id`/`_snd_1per2` to pick
`_SsTrapIntrVSync`/`_SsSeqCalledTbyT_1per2`/`func_80033738` as the callback for
`func_80024D40`), and finally `func_80024CF0()`.

## Best C reached: 65/164 words, correct dispatch VALUES, wrong physical case-body/test layout

```c
> **ROUND 39 (head): THIS PRESERVED BODY WILL NOT LINK AS WRITTEN.**
> Rename(s) needed before it builds: `func_80024CE0` -> `EnterCriticalSection`; `func_80024CF0` -> `ExitCriticalSection`; `func_80024D40` -> `InterruptCallback`; `func_80024DA0` -> `VSyncCallback`; `func_80033738` -> `SsSeqCalledTbyT`.
> The symbol was retargeted when that function became a linked Psy-Q
> SDK object, so the old name no longer exists in this tree. The NAME
> is stale; the residue this body demonstrates usually is not. Correct
> the name and REBUILD before trusting any figure attached to this
> block -- including one quoted in its own heading.
>
> Found by `python3 tools/stalesyms.py`. Note this warning is placed only
> where the stale name appears in CODE: a block whose prose merely
> discusses the rename is fine and is deliberately not marked.

#if 0
extern s32 _snd_seq_tick_mode;
extern s32 _snd_seq_no_tick;
extern s32 _snd_use_interrupt_id;
extern s32 _snd_1per2;
extern s32 _snd_use_vsync_cb;
extern void (*_snd_vsync_cb)(void);
extern void func_80024CE0(void);
extern void func_80024DA0(void (*cb)(void));
extern void (*func_80024D40(s32 arg0, void (*callback)(void)))(void);
extern void func_80024CF0(void);
extern void _SsTrapIntrVSync(void);
extern void _SsSeqCalledTbyT_1per2(void);
extern void func_80033738(void);
extern void StartRCnt(s32 arg0);
extern s32 ResetRCnt(s32 n);
extern s32 SetRCnt(s32 n, s16 target, u32 mode);

void _SsStart(s32 arg0)
{
    s32 i;
    s32 v1;
    s32 s0, s1;
    s32 rc90;
    void (*cb)(void);

    for (i = 999; i >= 0; i--) {
    }

    v1 = _snd_seq_tick_mode;
    switch (v1) {
    case 2:
        s1 = 0xF2000002;
        s0 = 0x44E8;
        _snd_use_interrupt_id = 6;
        break;
    case 3:
        s1 = 0xF2000002;
        s0 = 0x89D0;
        _snd_use_interrupt_id = 6;
        break;
    case 5:
        if (arg0 != 0) {
            s1 = 0xF2000003;
            _snd_use_interrupt_id = 0;
            s0 = 1;
        } else {
            _snd_use_vsync_cb++;
            goto merge2;
        }
        break;
    case 0:
        return;
    default:
        if (_snd_seq_no_tick != 0) {
            return;
        }
        s1 = 0xF2000002;
        _snd_use_interrupt_id = 6;
        if (v1 < 0x46) {
            v1 = 0x204CC0 / v1;
            _snd_1per2++;
            s0 = v1;
        } else {
            s1 = 0xF2000000;
            v1 = 0x409980 / v1;
            s0 = v1;
        }
        break;
    }

merge2:
    if (_snd_use_vsync_cb != 0) {
        func_80024DA0(func_80033738);
        return;
    }

    func_80024CE0();
    ResetRCnt(s1);
    SetRCnt(s1, (s16)s0, 0x1000);

    for (i = 1999; i >= 0; i--) {
    }
    for (i = 1999; i >= 0; i--) {
    }

    StartRCnt(s1);

    rc90 = _snd_use_interrupt_id;
    if (rc90 != 0) {
        if (_snd_1per2 != 0) {
            cb = _SsSeqCalledTbyT_1per2;
        } else {
            cb = func_80033738;
        }
    } else {
        _snd_vsync_cb = func_80024D40(0, NULL);
        rc90 = _snd_use_interrupt_id;
        cb = _SsTrapIntrVSync;
    }
    func_80024D40(rc90, cb);
    func_80024CF0();
}
#endif
```

## Two real, load-bearing structural findings on the way to 65/164

Starting point (nested `if (v1==2) {...} else if (v1<3) {...} else if
(v1==3) {...} else if (v1==5) {...} else {default}`, using an internal
`goto defaultCase;` for the `v1==1` case): **24/164**, with the dispatch
comparisons themselves already correct in VALUE but a completely
different branch-target layout from retail's -- the if/else chain
placed every branch's body immediately after its own test (standard
if/else layout), where retail's actual layout interleaves bodies in a
different order.

**Switching to a C `switch` statement (`switch (v1) { case 2: ... case
3: ... case 5: ... case 0: return; default: ... }`) is the right
top-level construct** -- it took the diff to 65/164 by fixing every
dispatch COMPARISON to match retail exactly (`beq v1,2`, `slti
v1,3`+`beqz`, `beq v1,3`, `beq v1,5`, fallthrough to default) with the
correct instruction count for the comparison chain. This is a
meaningfully different signal from the `_SsSeqCalledTbyT_1per2`/`SsEnd`
block-order lever (which is about if/else with a RAW value, not a
multi-way dispatch) -- for a genuine multi-way integer dispatch on a
small, non-dense case set, write a C `switch`, not a chain of
`if`/`else if`, even when the retail disassembly superficially looks
like a manual if-chain (as it does here: no jump table, just a sequence
of `beq`/`bne` tests). The two are NOT interchangeable at the source
level despite compiling to similarly-shaped comparison chains for small
case counts -- a `switch` reliably reproduces retail's exact per-case
comparison chain here where an equivalent if/else chain does not.

**What remains after the switch fix is a case BODY layout mismatch**,
not a comparison mismatch: retail's compiled body order is `5, 3, 2,
shared-tail` (physically, in that address order) while this build's
(matching the source's declared case order) is `2, 3, 5, shared-tail`.
The comparison chain tests in ascending value order (2, <3, 3, 5) in
BOTH, so retail's compiler evidently decoupled "order cases are
compared in" from "order case bodies are emitted in" -- not reproduced
by writing the `case` labels in a different declaration order within
one quick trial (not exhaustively tried; see below).

## A regression that's worth recording: an algebraically-equivalent rewrite of the `s1` default-case value made things dramatically worse

The `default:` arm sets `s1 = 0xF2000002` unconditionally, then
overwrites it to plain `0xF2000000` in the `v1 >= 0x46` branch (i.e. the
`|2` never really conditionally applies at the C level in the reached
version -- it's just two full reassignments). Retail's actual
instructions show the OPPOSITE composition: `s1 = 0xF2000000` set once,
with `ori s1,s1,2` applied ONLY inside the `v1 < 0x46` branch (a real
conditional OR, no reassignment in the other branch). These are
algebraically identical. Rewriting the source to match retail's
composition exactly (`s1 = 0xF2000000;` before the branch, `s1 |= 2;` as
the first statement of the `v1 < 0x46` arm, no `else` assignment) was
tried and **regressed hard, 65/164 -> 25/164** -- it flipped which
physical register (`$s0` vs `$s1`) several LATER, unrelated call
argument's rely on (visible as `move a0,$s1` becoming `move a0,$s0` at
the `ResetRCnt`/`SetRCnt` call sites, and the `SetRCnt` target
argument's cast changing from a `andi ...,0xffff` zero-mask to a
`sll/sra` sign-extend). Reverted immediately. Filed as a caution: a
"more literal" transcription of retail's instruction-level composition
is not always closer to retail's SOURCE, and can perturb register
allocation for code far away from the edit. Left in the algebraically-
equivalent-but-differently-composed form that scored 65/164.

## Residue not further chased: register permutation and switch body order

Beyond the case-body-order issue, the surviving diffs are the SAME
class of register permutation seen throughout this unit this round
(`_SsInit`, `func_800323A8`): matching content, swapped physical
registers (`$s0`/`$s1`, `$v0`/`$v1`), plus the two `div`/`break`
overflow-trap blocks and the final callback-dispatch tail (which is
structurally the exact same shape already matched byte-for-byte in
`SsEnd` -- so the logic there is very likely right, and any
remaining diff there is register permutation, not a different C shape).
None of these were separately isolated given the size of the case-body-
order problem still unresolved; a future attempt should fix case body
ordering FIRST (try declaring cases in body order `5, 3, 2, 0, default`
rather than value order, or investigate whether GCC 2.6.3 always emits
switch bodies in reverse-declaration order for this case-count/density)
before spending attempts on the register residue underneath it.

## Answering the head's question: did the register-count screen influence where this stopped?

No. The screen reported 2 (`$s0`/`$s1`), inside the tractable band, and
was never a factor. What actually consumed the budget: this function
combines THREE of this round's hardest problems in one body (a
multi-way dispatch needing the switch-vs-if/else distinction discovered
here, the PSX div/break overflow-trap idiom, and the same register-
permutation class seen in the other two big-three stalls) inside a
164-word function, which is simply a lot of surface area for the
session time available after `_SsInit` and `func_800323A8` had
already consumed substantial budget this round. Stopped at 65/164 (up
from an initial 24/164) with the switch-vs-if/else finding banked as
the header-line result, not because of register count.

### Proposed learning

1. A retail disassembly that LOOKS like a manual if/else chain (no jump
   table, sequential `beq`/`bne` tests) is not reliable evidence against
   a C `switch` -- for small, non-dense integer case sets this compiler
   lowers `switch` to exactly this shape. Prefer `switch` over
   `if`/`else if` whenever dispatching on a single integer value against
   more than two-three named constants; confirmed here to fix an entire
   comparison-chain's instruction selection and count in one shot (24 ->
   65/164) where the if/else equivalent could not.
2. When retail composes a value across a branch as "set unconditionally,
   OR-modify only in one branch" rather than "set differently in both
   branches" (two algebraically equivalent phrasings), the phrasing is
   NOT free to choose based on transcription convenience -- confirmed by
   a regression (65 -> 25/164) here matching the OPPOSITE conclusion of
   several earlier fixes this round (e.g. `SetRCnt`'s `md` variable used
   exactly this "unconditional base + branch OR" composition
   successfully). The generalization is: match retail's COMPOSITION
   (reassignment vs. conditional modify), not just its final values --
   but verify empirically per-function, since here reverting to the
   "two full reassignments" form (a worse composition match, but the
   FIRST one reached) scored better than the "corrected" one.

## Round 19 verification and one more negative (runner charlie)

Per the head's mid-round drift-check broadcast: rebuilt this report's
exact 65/164 body and confirmed via `objdump` the function compiles to
exactly 164 instructions, matching retail's length, with no
outside-range drift. **Recorded score is real and survives the check.**

Read the raw disassembly directly to double check the `s1` composition
claim in "A regression that's worth recording" above, since the delay-
slot mechanics looked worth re-verifying before trusting the prose:
confirmed both `lui`/`ori` halves of `s1 = 0xF2000002` in the `default:`
case DO execute unconditionally (one in the `bnez $v0,.L80032980` delay
slot at function entry to `default`, the other in the `beqz
$v0,.L80032860` delay slot testing `v1 < 0x46`) -- delay slots execute
regardless of branch direction, so at the machine level `s1` reads as
`0xF2000002` on BOTH sides of that branch by the time either path's
body runs. This looked like it might mean the current body's `else: s1
= 0xF2000000;` reassignment is spurious (i.e., retail never actually
resets it). **Tested removing it** (leaving `s1` untouched in the
`v1 >= 0x46` branch) -- **regressed**: 65/164 -> 55/164, and the
compiled length shrank to 163 (1 SHORT of retail, with real
outside-range drift). So the reassignment IS load-bearing for length
despite the delay-slot analysis suggesting it's a no-op at the value
level -- reverted immediately. This is now a THIRD composition tried for
this exact `s1` value (two full reassignments: 65/164, best; base +
conditional-OR: 25/164; base + no modification: 55/164 non-drift-free),
strengthening rather than resolving the existing "match retail's
composition empirically, the delay-slot-level reading of what's
'really' conditional is not a reliable guide" caution already on file.

Did not attempt the case-body-reordering lever this report names as the
concrete next step (`5, 3, 2, 0, default` physical order via
`goto`-per-case) -- the switch-based body already reproduces this
report's own baseline correctly, and reordering it is a genuinely large
restructuring for a 164-word function combining three of this round's
hardest residue classes at once; left for a session with more budget to
spend on one function. No source change kept.

## Round 41 (runner delta): the case-body-reorder lever NEGATIVE, first-ever permuter search NEGATIVE

Round 41's assignment named this function specifically because it (and
its sibling `SsSetTickMode` in the same unit) had **never been
permuter-searched**. Restored the round-16/19/20 65/164 body verbatim
(after renaming the five SDK-retargeted symbols per the round-39 warning
above -- confirmed it still builds clean and reproduces exactly 65/164,
no drift, matching this report's own recorded figure before touching
anything).

**First real diff, read off `tools/asm-differ/diff.py _SsStart`
directly (not inferred):** at file offset `0x22f60` / vram `0x80032760`,
retail has `bne v1,v0,.L800327D8` (branch AWAY to `default` when
`v1 != 5`, so the `v1 == 5` case's own dispatch falls through inline to
the very next instruction) while this build has `beq v1,v0,<case5 body>`
(branch TO the case-5 body when equal, falling through to an explicit
`j <default>` when not). Both encode the identical VALUE comparison
(`v1 == 5`); the polarity and which side is placed inline differs. This
is the exact `if (cond) {A; return;} rest;` vs `if (cond) {...} else
{B; return;}` polarity question that closed the OTHER function in this
round's assignment (`SsSetTickMode`, see that report) -- but attempting
the analogous fix here (below) did not transfer.

### Lever 1 tried: reorder switch cases to retail's physical body order (5, 3, 2, 0, default) -- NEGATIVE, confirms a real project rule the hard way

This is the exact concrete next step this report has named since round
16/20 ("try declaring cases in body order 5, 3, 2, 0, default"). Tried
it directly: reordered the `switch`'s `case` clauses from source order
`2, 3, 5, 0, default` to `5, 3, 2, 0, default`, matching the physical
address order of the case BODIES in retail's `.s` (case5's `arg0!=0`
body at `.L80032790`, case3 at `.L800327A8`, case2 at `.L800327B8`, then
the shared `_snd_use_interrupt_id=6` tail, then `default` at `.L800327D8`).

**Result: regressed hard, 65/164 -> 36/164, WITH real outside-range
drift** (function length changed). `asm-differ` showed the regression
was not a clean reshuffle -- reordering the declaration caused GCC to
swap which physical register (`$s0` vs `$s1`) holds the device-tag vs.
rate value for case 2/3/5 specifically, and case 5's own internal
`arg0`-dependent branch changed polarity AGAIN and relocated its
sub-bodies to different offsets than either the retail layout or the
pre-reorder build. **Also tried swapping the LOCAL VARIABLE declaration
order** (`s32 s1, s0;` instead of `s32 s0, s1;`, following the "sibling
declaration order is a register-allocation lever" idiom recorded
elsewhere in `docs/DECOMPILATION_LEARNINGS.md` for VLA siblings) on top
of the reorder -- **identical regression, still 36/164**, so the
register-role swap is not fixable by pairing the two known levers here.

Both attempts reverted immediately; confirmed back to 65/164 no-drift
before proceeding.

**Why this matters beyond a negative data point:** `docs/DECOMPILATION_LEARNINGS.md`
already states the general rule this collides with (`### A switch's CASE
ORDER is recoverable from the binary`, round 23): **for a JUMP-TABLE
(dense) switch, source order is recoverable from body layout; for a
SPARSE switch lowered to a compare chain (no jump table -- exactly this
function's shape), the compiler "picks its own comparison order" and
source declaration order is NOT reliably recoverable from body layout
either.** `SsSetTickMode` (this round's OTHER assignment, in the SAME
unit) is a jump-table switch and the body-order lever closed it
cleanly. `_SsStart` has no jump table (a straight `beq`/`bne`/`slti`
compare chain, confirmed in the `.s`) and the SAME lever, tried the same
way, made things categorically worse rather than incrementally worse --
strong evidence the two functions are on opposite sides of the
documented jump-table/sparse-switch boundary, not merely that this one
needed a different case order. **Do not retry naive case-declaration
reordering on this function without a new hypothesis for WHY retail's
compare-chain switch chose to inline case 5's body specifically** (e.g.
whether it correlates with which case is a `break` vs which falls into
shared cleanup, or something about `case 5`'s own nested `if` making it
too large to treat as a "trailing" arm).

### Lever 2 tried: first-ever permuter search -- ran to its time bound, best score found is NEGATIVE when translated

Set up with `tools/setup-permuter.sh _SsStart <65/164 seed>`.
`--debug --stack-diffs` validation matched this report's own recorded
residue shape (base score 3735; the visible diff was exactly the
case-5/default polarity region above plus a register-permutation tail
consistent with "Residue not further chased" section above) --
**scaffold confirmed trustworthy before searching.**

Launched: `PATH=.../permuter-work/bin:$PATH .venv/bin/python3
tools/decomp-permuter/permuter.py -j 4 --stop-on-zero --best-only
permuter-work/_SsStart`, bounded at 900s (the machine was also
running alpha's and charlie's own `-j 6` permuter searches concurrently
in sibling worktrees this round -- real contention, not idle).

**Result: ran the full 900s bound, `timeout` fired (rc consistent with
124 -- the wrapping shell's own `echo "permuter rc=$rc"` line did not
flush to the log before the process was reaped, but no permuter process
remained afterward and the log's own tail shows iteration counting
simply stopping mid-batch).** **81,222 iterations** in 900s under load,
**zero reached** (`--stop-on-zero` never fired). Best score found:
**2735**, against a base of 3735 -- a real, repeated improvement under
the permuter's OWN scoring metric, found at iteration 38 and never
improved on in the remaining ~81,000 iterations.

**Translated the 2735 candidate to the project's real C and rebuilt
through the actual pipeline (not the permuter's scaffold) -- it is a
NEGATIVE when measured the way that matters.** The candidate's only
semantic-preserving mutations were (a) routing the `else`-branch's
`s0 = v1;` through a pointless pointer alias (`s32 *new_var2; s0 =
*(new_var2 = &v1);`) and (b) routing the `SetRCnt` rate argument through
an `unsigned char` intermediate instead of a direct `(s16)` cast
(`u8 new_var = (s16)s0; SetRCnt(s1, new_var, 0x1000);`). Applied
verbatim to `src/psyq/libsnd_ssinit_libapi_counter.c` and rebuilt: **14/164 words match,
WITH drift** -- markedly WORSE than the 65/164 baseline, not better.
Reverted immediately; confirmed the file returns to the documented
65/164 no-drift state afterward.

**This is the load-bearing finding of this round's search, and it
generalises past this one function: a LOWER decomp-permuter score is
scored on a weighted edit-distance metric (stack/branch/register diffs,
reorderings, insertions, deletions weighted differently), and improving
that composite score is NOT the same claim as "more words match in the
project's own `funcdiff.py` count."** The other three saved candidates
this search produced (scores 3050, 3245 x2, 3250 -- all close to or
above the 3735 base) were inspected directly (`diff` against `base.c`)
and are pure reformatting/renaming noise with no candidate structural
lead at all; not worth translating. **Never trust a sub-base permuter
score as a real improvement without rebuilding it through
`build-and-verify.sh`/`funcdiff.py` on the actual project pipeline --
this round's one candidate that looked like a genuine lead (a
1000-point improvement, found early and never beaten) was actually a
regression by a factor of four under the metric that is actually the
project's oracle.**

### Disposition

Restored to `INCLUDE_ASM`; `git status --porcelain` clean for this
function. Whole-image build verified green (`./build-and-verify.sh`
passes) with this function as `INCLUDE_ASM` and `SsSetTickMode` (this
round's other assignment) as a genuine byte-exact match -- so the only
thing blocking a fully-matching image in this unit right now is this
function.

### Proposed learning

1. **The "case order is recoverable from body layout" lever is
   confirmed, empirically and by direct A/B regression in this round,
   to be SCOPED to jump-table (dense) switches and not to transfer to a
   sparse switch lowered to a compare chain** -- `SsSetTickMode` (dense,
   jump table) closed cleanly on this lever in this same round;
   `_SsStart` (sparse, compare chain) regressed by nearly half its
   already-matched words on the identical technique, twice (with and
   without also swapping local declaration order). `docs/DECOMPILATION_LEARNINGS.md`
   already states this boundary in prose (round 23); this is a second,
   independent, costed confirmation of it and a caution against treating
   the two switch shapes as interchangeable for this lever even when a
   function LOOKS eligible (small case count, no jump table needed).
2. **A decomp-permuter score improvement is not a proxy for a
   `funcdiff.py` word-count improvement, and the gap can be large enough
   to flip the sign.** A candidate that beat the seed's base score by
   ~1000 points (2735 vs 3735, found early, never improved on in 81k
   further iterations) translated to a REAL regression of more than 4x
   against the project's actual oracle (65/164 -> 14/164, with drift).
   Always rebuild a saved permuter candidate through
   `build-and-verify.sh` + `funcdiff.py` before trusting its score,
   exactly as the existing "sub-base candidate is worth translating"
   guidance already implies for the exhausted-search case -- this
   extends it to the found-improvement case too: a BETTER permuter score
   is not evidence of a better real match either, only a real rebuild is.

## Provenance

round 16 (2026-09-04), runner delta, unit libsnd_ssinit (fresh carve,
second pass, head-directed follow-up on the big-three queue). ~4
attempts (nested if/else baseline, switch conversion, one regression +
revert); restored to `INCLUDE_ASM`.

round 19 (2026-09-05), runner charlie. Verified drift-free; tried a
third `s1`-composition variant (negative, confirms the class); no
change kept.

## ROUND 20 note (runner echo)

Reviewed fresh this round per the coordinator's "re-derive from scratch,
skeptical of any not-fixable verdict" standard (the same standard that
found real progress on `FillRVectors4` in a different unit and on this
unit's own `GetRCnt`). This function's residue (case-body physical
ordering plus a register cascade sensitive to how the default case's
value composition is written) is a different, larger-scoped class than
either of those two fixes -- not a simple register-identity swap or a
commutative-add operand choice, but a genuine case-body layout question
already narrowed by real, load-bearing structural work (the switch-vs-
if/else finding, and the documented regression from rewriting the
default-case value composition). No new lever identified within this
round's time budget; not re-attempted, to avoid burning attempts without
a concrete new idea. Still restored to `INCLUDE_ASM`, 65/164.

round 41 (2026-09-14), runner delta, unit `libsnd_ssinit_libapi_counter` (renamed from
`libsnd_ssinit` in round 33's SDK split). Re-verified the inherited
65/164 body builds clean and matches this report's own figure. Tried the
report's own named next step (case-body reorder to `5, 3, 2, 0, default`,
with and without a paired local-declaration-order swap) -- both
NEGATIVE, regressed to 36/164 with drift, confirming this function sits
on the sparse/compare-chain side of the jump-table body-order boundary
documented elsewhere. Ran this unit's first-ever decomp-permuter search
(900s bound, `-j 4`, 81,222 iterations, zero never reached); its best
saved candidate (permuter score 2735 vs base 3735) translated to a REAL
regression (14/164, with drift) when rebuilt through the actual project
pipeline -- a permuter score improvement did not correspond to a real
one here. No source change kept; restored to `INCLUDE_ASM`, still
65/164, first real diff at vram `0x80032760`. See "Round 41" section
above for full detail.

## Naming

**Superseded, round 71 (track 2):** the track-3 game name `SeqTimerControl`
is replaced by Sony's own name -- fingerprint EXACT masked 1.00 vs
libsnd/ssinit's internal `_SsStart` (disc 3.3). This is Sony's SDK code, not
decompiled game logic; track 2 names those functions and moves them out of
tracks 1/1b/3.

Round 69 (delta), track 3 pass on `libsnd_ssinit_libapi_counter`.

| name | tier | evidence |
| --- | --- | --- |
| `_SsStart` (was `func_80032708`) | B | arms a PSX root counter at the rate `SsSetTickMode` selected (`SetRCnt`), tags a device value, and registers one of two ISR callbacks (`_SsTrapIntrVSync`/`_SsSeqCalledTbyT_1per2`) via `InterruptCallback` -- an "arm the sequencer's software timer" routine. `arg0` is a start/stop-ish switch (see `SsStart.md`/`SsStart2.md`) but its exact in-game trigger is not established, hence B not A. |
| `_snd_use_interrupt_id` (was `D_8006DC90`) | B | the value threaded through `InterruptCallback` as a handle: `-1` is checked as a sentinel ("nothing armed") throughout this function and in `SsEnd`, `0` means "armed but not yet given a real id" (this function then captures one via `InterruptCallback(0, NULL)`), and any other value is passed straight back to `InterruptCallback` to re-target or deregister. |
| `_snd_1per2` (was `D_8006DC94`) | B | set (incremented, never explicitly reset here) only on the branch of the default case that computes the smaller of the two custom rates (`v1 < 0x46`); this function's own callback-selection tail reads it as a boolean to choose `_SsSeqCalledTbyT_1per2` over `_SsTrapIntrVSync` -- i.e. it selects the half-rate ISR variant. `SsEnd` clears it back to 0. |
| `_snd_use_vsync_cb` (was `D_8006DC8C`) | B | when set, this function's shared tail skips arming entirely and instead tears down via `VSyncCallback(SsSeqCalledTbyT)`; `SsEnd` is the other place that reads/clears it, calling `VSyncCallback(0)` first. Named for what it gates (a pending stop/teardown), not for a specific caller's intent. |
| `_snd_vsync_cb` (was `D_8006DC9C`) | B | captured from `InterruptCallback(0, NULL)`'s return value (the previously-installed handler) right before this function installs `_SsTrapIntrVSync` as the new one; `_SsTrapIntrVSync` calls it first, then always calls `SsSeqCalledTbyT` -- the classic "save old handler, chain to it" ISR-hook idiom. |

`_snd_seq_tick_mode`/`_snd_seq_no_tick`/`_snd_video_mode`/`VBLANK_MINUS` are
established in `SsSetTickMode.md`; this function only reads the first two.
