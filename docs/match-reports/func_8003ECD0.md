# func_8003ECD0 -- STALL: length EXACT (73/73 words, no drift); 71/73 raw word-match; first real diff at vram 0x8003ED18 (the `addu`/`addiu` pairing). ATTEMPT 6'S VERDICT IS CORRECTED BELOW.

## ROUND 46 (runner delta): drift-checked fresh, no new lever -- DELIBERATE SKIP, plus a self-caught cross-contamination scare worth recording

Re-spliced the round-36 body (with `GsClearOt`) into `src/code_2cc8c_d.c`
and rebuilt. **First attempt showed a spurious `WARNING: differs OUTSIDE
this range too (22 bytes)`** that had nothing to do with this function --
`cmp -l build/SLPS_015.56 disk/SLPS_015.56` plus
`vram = (N-1) - 0x800 + 0x80010000` pointed at `0x8003e4bc`, which is
`func_8003E4B8` (a DIFFERENT function in a DIFFERENT unit,
`code_2cc8c_c`) -- I had left that function's own stall body live from an
earlier hand-lever test in this same round instead of restoring its
`INCLUDE_ASM` before moving on. Restored it (`grep -c '^INCLUDE_ASM'`
checked across all four of this runner's units to confirm exactly one
function live at a time) and rebuilt again: **71/73, zero outside-range
drift** -- the figure this report already documents, honest and
reproducible in isolation.

Posted this as a general caution to the round's broadcast channel: with
four units live in one worktree, a drift warning on the function you're
CURRENTLY testing can come from a sibling you forgot to revert two
functions ago, and `build/lsdde.map` will point straight at the real
culprit if you check it rather than assuming the function in front of
you is at fault.

**Deliberate skip, no new attempt.** This residue is now 14 hand
attempts plus one ~40,000-iteration permuter search deep across four
prior rounds (23, 36, 44), converged on the same 2-word `addu`/`addiu`
operand-pairing swap with every reshape other than the naive
left-to-right parse regressing the HEAD of the expression too (see the
round-39/44 sections below). No new source-level grouping occurred to me
that isn't already in that table, and per this round's own guidance not
to re-run an already-negative unguided search without a new angle, I am
not repeating the 40k-iteration search. Restored to `INCLUDE_ASM`; full
oracle re-confirmed green.

### Proposed learning (round 46)

Restates and reinforces the standing CLAUDE.md guidance rather than
adding new content: **with four INCLUDE_ASM units in one runner's brief,
`grep -c '^INCLUDE_ASM' src/<unit>.c` against the count you started with
is cheap enough to run before EVERY build once you've touched more than
one function this session, not only when a diff already looks
structurally wrong.** The 22-byte drift here looked small enough to be a
real, narrow residue rather than an obvious whole-image corruption,
which is exactly the case where skipping the check costs the most time.

## ROUND 39 (head): baseline re-verified, five more variants, and ATTEMPT 6 WAS MEASURING THE WRONG HALF OF ITSELF

Baseline re-spliced against `main` and rebuilt: **exactly 71/73, zero
outside-range drift.** The inherited figure is honest.

### The correction, and it is the useful part of this pass

Attempt 6 reads:

> A named `s32 shiftVal = 4 << self->unk3C;` local, **reused at both call
> sites** (this and `unk88`'s own computation, which ALSO computes
> `4 << self->unk3C`) -- **no change from attempt 1's score either way**;
> ruled out as the lever.

**"Either way" collapses two variants that differ by 55 words.** Measured
here, separately, each spliced and built through the full oracle:

| variant | score |
| --- | --- |
| `shiftVal` named, used ONLY in the `size` computation; `unk88` recomputes `4 << self->unk3C` | **71/73** (inert -- this is the half attempt 6 measured) |
| `shiftVal` named, **reused at BOTH sites** as attempt 6's own text describes | **16/73, with whole-image drift** |

So the reuse is not inert, it is catastrophic, and attempt 6 recorded it as
inert. The verdict "ruled out as the lever" happens to survive -- naming the
local does not close the residue -- but the reasoning under it did not measure
what it says it measured.

**Why the reuse is destructive is the part worth carrying forward: retail
RECOMPUTES `4 << self->unk3C` at the `unk88` site rather than keeping one
value live.** Naming it once and reusing it forces GCC to hold it across the
allocation and rewrites the whole function. This is round 39's alpha lever
arriving as a CONSTRAINT rather than an opportunity -- alpha closed 8 words on
`func_800357B0` by *dropping* a named local and recomputing inline at its one
real use, on the same principle. Here the recomputation is already correct and
naming it is what breaks.

### The other four variants (all negative)

| variant | score |
| --- | --- |
| baseline (attempt 1) | 71/73 |
| multiply operand order flipped (`unk44 * unk48`) -- **not in the prior attempt list** | 69/73 |
| flipped multiply + bare `__asm__("")` first statement | 67/73 |
| flipped multiply + `shiftVal` at both sites | 14/73, drift |
| `__asm__("")` + `shiftVal` at both sites (the attempt-6 x attempt-7 conjunction) | 16/73, drift |

**And a note on how NOT to read that last row.** Charlie measured this round
that two individually-inert levers can combine productively
(`func_8004BE54`, 130/150 -> 132/150), so the attempt-6 x attempt-7
conjunction looked like the prescribed next move here. It scored 16/73, and
the tempting conclusion -- "conjunctions can also combine destructively" --
**is not supported by this data.** Once attempt 6's reuse half is measured
alone at 16/73, the conjunction's score is fully attributable to that single
component, and the barrier contributes nothing. The conjunction was never
tested against two genuinely inert levers, because one of them was not inert.

That is the project's own attribution discipline applied to a lever rather
than to a blocker: a combined result says nothing until each component has
been measured alone.

### Standing

The residue itself is unchanged and is now **14 attempts deep across four
rounds**, plus a ~40k-iteration permuter search. Every restructuring of the
`size` expression regresses the HEAD of it (the `mult` / `unk3C`-reload /
`sllv` scheduling), not merely the tail pairing. The multiply operand order,
tried here for the first time, joins that pattern.

### Proposed learning (round 39)

**An attempt log entry that says "either way" or "in both positions" is a
single figure standing for two measurements, and the project has no way to
tell which one was actually run.** This one hid a 55-word difference under
"no change either way" for four rounds, and the next reader inherits it as a
closed axis. When an attempt has two spellings, record two rows.

Unit: `code_2cc8c_d`. Round 14, runner delta. Best score: 71/73 words
in-range, build clean at that score. ~14 real attempts, all on the SAME
2-word residue. Restored to `INCLUDE_ASM` per project rule.

## Signature (as attempted)

```c
void func_8003ECD0(Unk18Obj *self);
```

`Unk18ObjMethods`'s own `+0x08C`... no — not a vtable slot found in
`tools/classtable.py D_8006E8E4`'s output; called directly by symbol
(caller not yet found in this unit's own queue).

## What it does

One-time allocation/init, guarded by `self->unk70` (the same latch
`func_8003EA2C`/`func_8003EA48` check, this round): allocates one buffer
sized to fit two internal records plus a `4 << self->unk3C`-sized payload
each, carves it into `unk78`/`unk80`/`unk88` (bases) and
`unk7C`/`unk84`/`unk8C` (bases + size), writes a 2-word header into each of
`unk78`/`unk7C`, then hands both off to `func_8003FC18` (next slice,
uncarved).

```c
void func_8003ECD0(Unk18Obj *self) {
    s32 size;
    s32 buf;

    if (self->unk70 != 0) {
        return;
    }

    size = self->unk48 * self->unk44 + (4 << self->unk3C) + 0x14;

    buf = (s32)func_80017B34(size * 2);
    if (buf == 0) {
        return;
    }

    self->unk78 = buf;
    self->unk80 = buf + 0x14;
    self->unk88 = (4 << self->unk3C) + self->unk80;

    self->unk7C = size + self->unk78;
    self->unk84 = size + self->unk80;
    self->unk8C = size + self->unk88;

    *(s32 *)self->unk78 = self->unk3C;
    *(s32 *)(self->unk78 + 4) = self->unk80;

    *(s32 *)self->unk7C = self->unk3C;
    *(s32 *)(self->unk7C + 4) = self->unk84;

    func_8003FC18(0, 0, self->unk78);
    func_8003FC18(0, 0, self->unk7C);

    self->unk70 = 1;
    self->unk74 = 0;
}
```

Every field write, every branch, the allocation call, both `func_8003FC18`
calls, and the whole tail all match byte-for-byte. **The only residue is
2 words (out of 73) in the very first arithmetic expression** — the
initial `size` computation.

## The residue, exactly

Retail's own instruction sequence for `size`:

```
mult  $v1, $v0        ; self->unk48 * self->unk44  (into HI:LO)
lw    $v1, 0x3C($s0)  ; v1 = self->unk3C  (independent, fills mult latency)
sllv  $v1, $s2, $v1   ; v1 = 4 << self->unk3C
mflo  $v0              ; v0 = product (mult now resolved)
addiu $v0, $v0, 0x14   ; v0 = product + 0x14
addu  $s1, $v1, $v0    ; size = shiftVal + (product + 0x14)
```

Every attempt that reproduces the first 4 of those 6 instructions
byte-for-byte (confirmed — `mult`, the independent `unk3C` reload, the
`sllv`, and `mflo` all land exactly where retail has them) ends the
combination as `(product + shiftVal) + 0x14` instead of retail's
`shiftVal + (product + 0x14)` — same VALUE, different pairing, 2
instructions swapped.

## What was tried (all real attempts, all reverted)

1. `self->unk48 * self->unk44 + (4 << self->unk3C) + 0x14;` (plain
   left-to-right) — **71/73, the best reached.** Gets the first 4
   instructions exactly right; only the final pairing differs.
2. `(4 << self->unk3C) + (self->unk48 * self->unk44 + 0x14);` (retail's
   own final grouping, written explicitly) — 68/73, WORSE, and the
   regression starts at instruction 1 (the `mult`/reload/`sllv` ordering
   itself changes), not just the tail. This is the most informative
   negative result: naming retail's own target grouping does not
   reproduce it, because the earlier instructions are sensitive to the
   SAME expression shape in a way that isn't obvious from the final value.
3. `self->unk48 * self->unk44 + 0x14 + (4 << self->unk3C);` — 69/73,
   same regression pattern.
4. `(self->unk48 * self->unk44 + 0x14) + (4 << self->unk3C);` (explicit
   parens forcing retail's inner grouping) — 69/73.
5. Two-statement splits (`size = mult; size = shift + size;`, `size =
   mult + 0x14; size = shift + size;`, `size = mult; size += 0x14;`) —
   67–68/73 each, all regressing the early instructions too.
6. A named `s32 shiftVal = 4 << self->unk3C;` local, reused at both call
   sites (this and `unk88`'s own computation, which ALSO computes `4 <<
   self->unk3C`) — no change from attempt 1's score either way; ruled out
   as the lever.
7. A bare `__asm__("");` scheduling barrier, tried both mid-expression
   (via a statement split) and as the function's literal first statement
   — no improvfement in either position; the "prologue callee-save order"
   precedent's lever does not transfer to this residue.

## Round 23 (head): two further attempts, both 68/73 — the verdict is firmer, not changed

The baseline above was re-verified first: the preserved body splices in and
builds clean, and reproduces **exactly 71/73 with no address drift**, so this
report's inherited score is honest (round 19 measured roughly one preserved body
in six carrying a false drift-free claim; this is not one of them).

Two groupings not in the attempt list above were then tried:

| # | attempt | result |
| --- | --- | --- |
| 8 | a separate `s32 area = self->unk48 * self->unk44 + 0x14;` local, declared FIRST, then `size = (4 << self->unk3C) + area;` | **68/73** |
| 9 | `size = (4 << self->unk3C) + (0x14 + self->unk48 * self->unk44);` — the constant written on the LEFT inside the parens, so `fold`'s canonicalisation produces the `addiu` rather than the source | **68/73** |

Both regress to **the same 68/73 and in the same place** as attempts 2–5: the
`mult` / `unk3C`-reload / `sllv` scheduling at the HEAD of the expression breaks,
not just the tail pairing. Attempt 8 matters because it is the one shape that
should have decoupled the two halves — a distinct local with its own live range,
computed in its own statement — and it does not. Attempt 9 matters because it
rules out the remaining hypothesis that `addiu $v0, $v0, 0x14` came from a
source-level `const + var` that `fold` canonicalised.

**So the residue is now 9 attempts deep across two rounds, with every attempt
except the naive left-to-right parse landing on 68 or below.** That is a
converged negative, not an unfinished search: the two instructions cannot be
re-paired by any expression shape tried without breaking four earlier ones.
The next lever here is the permuter, not another hand-written grouping.

The final `addu`'s operand ORDER is worth recording because it is what pins the
target grouping and rules out the cheap readings: retail's `addu $s1, $v1, $v0`
has the shift (`$v1`) as `rs` and the `product + 0x14` sum (`$v0`) as `rt`, so
the shift is genuinely the LEFT operand of the outer `+` and the sum is a single
right-hand operand. The best build inverts this into `addu $v0, <shift>, <product>`
followed by `addiu $s1, $v0, 0x14`.

**Every attempt other than #1 disturbs instructions BEFORE the residue
even starts**, which is the real finding here: this isn't a case where the
tail can be reshaped independently of the head. The `mult`/`mflo` pair's
own scheduling (which independent work the compiler interleaves to hide
multiply latency) is apparently entangled with the FULL expression's shape
in a way where only the naive left-to-right parse reproduces retail's own
early scheduling, and that same naive parse is what gets the tail's pairing
wrong.

## Header changes kept

`include/code_2cc8c.h` — all MEASURED from the disassembly, independent of
the stall:
- `Unk18Obj::unk70`'s own comment updated: this function is its set site
  (previously "not itself written by any function this unit attempted").
- `Unk18Obj` gains `unk74` and `unk78`/`unk7C`/`unk80`/`unk84`/`unk88`/
  `unk8C` (all `s32`, deliberately not pointer-typed — see the struct's own
  comment on why: retail computes every one of them via plain word
  arithmetic, and `unk78`/`unk7C` are ALSO dereferenced directly as raw
  2-word records via explicit casts, which a pointer-typed field would not
  reproduce cleanly).
- New extern `func_8003FC18(s32 a0, s32 a1, s32 a2)` (next slice,
  uncarved).

## ROUND 36 (runner delta): stale-symbol rebuild confirms 71/73 exactly; bounded permuter search finds no zero, two spurious sub-baseline candidates fingerprinted

Round 34's SDK-object conversion linked `libgs/gs_113.o` and renamed this
function's callee from `func_8003FC18` to `GsClearOt` (`config/symbols.
slps01556.lsdde.txt:476`). This report's preserved body (both copies above)
still used the old name, so per this round's stale-symbol trap it had never
actually been rebuilt in its current form since that rename — the 71/73
figure was correct but unverified against the current tree.

**Rebuilt with the corrected name.** `extern void GsClearOt(s32, s32, s32);`
already existed in `src/code_2cc8c_d.c` (added when round 34 retyped this
unit's other Sony calls), just declared after this function's own call
sites; added a second, identical declaration ahead of `func_8003ECD0`
itself (same pattern this unit already uses for its other local externs)
rather than hoisting the existing one. `func_80017B34` also needed its own
extern (not previously declared in this unit). Build: clean compile
(`build exit=2`, zero grep hits on the compile-error patterns, ordinary
SHA1 mismatch). **`funcdiff.py` reports 71/73 words, no out-of-range
drift when this is the only in-progress edit** -- exactly reproducing the
title figure. The residue is unchanged from the description above (the
same 2-word `addiu`/`addu` pairing).

### The corrected, LINKABLE body (head, round 36) — measured 71/73 in the current tree

The prose above describes this rebuild; it did not carry the source, so the
report still preserved only the pre-rename copies and `stalesyms.py` still
flagged it. That is round 35's trap exactly — the names get fixed in the
working tree to take the measurement, and the durable artifact keeps the
version that cannot link. Spliced in place of the `INCLUDE_ASM` below, this
builds and measures **71/73, no out-of-range drift**; the two differing words
are the documented instruction-order residue at vram `0x8003ED18`/`0x8003ED1C`.

```c
#if 0
extern void GsClearOt(s32 a0, s32 a1, s32 a2);
extern void *func_80017B34(s32 size);

void func_8003ECD0(Unk18Obj *self) {
    s32 size;
    s32 buf;

    if (self->unk70 != 0) {
        return;
    }

    size = self->unk48 * self->unk44 + (4 << self->unk3C) + 0x14;

    buf = (s32)func_80017B34(size * 2);
    if (buf == 0) {
        return;
    }

    self->unk78 = buf;
    self->unk80 = buf + 0x14;
    self->unk88 = (4 << self->unk3C) + self->unk80;

    self->unk7C = size + self->unk78;
    self->unk84 = size + self->unk80;
    self->unk8C = size + self->unk88;

    *(s32 *)self->unk78 = self->unk3C;
    *(s32 *)(self->unk78 + 4) = self->unk80;

    *(s32 *)self->unk7C = self->unk3C;
    *(s32 *)(self->unk7C + 4) = self->unk84;

    GsClearOt(0, 0, self->unk78);
    GsClearOt(0, 0, self->unk7C);

    self->unk70 = 1;
    self->unk74 = 0;
}
#endif
```

### Permuter search: ~40,000 iterations, no zero, two spurious leads

Per round 23's own verdict ("the next lever here is the permuter, not
another hand-written grouping"), seeded `tools/setup-permuter.sh` from this
exact 71/73 body (with `GsClearOt`) and ran `-j 4 --stack-diffs
--stop-on-zero --best-only` under `timeout 400`. Base score (via `--debug
--stack-diffs`) reproduced as **215**. The search reached **iteration
~40,005** before the wall clock ended it (no leaked process afterward).
**No candidate reached score 0, and the search found no legitimate
improvement.**

Two candidate directories scored below the 215 baseline (120 and 175,
the latter appearing twice) but both are spurious, confirmed by diffing
each `source.c` against the seed:

- **Score 120 is a semantic bug, not a fix.** It moves the `+ 0x14` term
  out of the live `size` computation and into an unreachable dead branch
  (`if (buf == 0) { size = size + 0x14; if (1) { return; } }`) that always
  returns before the modified `size` is ever read. The SURVIVING path's
  `size` therefore never gets `+0x14` added at all, which changes the
  value handed to `self->unk7C`/`unk84`/`unk8C` and to the `func_80017B34`
  allocation call relative to the confirmed-correct semantics (the leading
  4 instructions of the 71/73 body already reproduce retail's own
  `mult`/reload/`sllv`/`mflo` sequence exactly, which only happens when
  `+0x14` is folded into `size` before the allocation, not after). The
  permuter's textual/instruction-diff heuristic scored this lower anyway
  because dead code still gets compiled and can coincidentally resemble
  retail's instruction stream in places the heuristic weights, without the
  candidate being behaviorally equivalent. **Not adopted.**
- **Score 175 (x2) narrows `size` to `unsigned char`.** A real
  truncation bug for a buffer-size computation that is very plausibly
  >255 in general (this specific call site's actual runtime values are
  unknown, so nothing here proves it safe). Same class of permuter
  artifact as the fingerprinted 130-scored candidate in
  `func_80031A44.md`'s round 31/32 addenda: a mutation that scores better
  on the tool's own heuristic without being a real candidate.

**Verdict: unchanged.** 71/73 remains the best HONEST score; the residue
is the same 2-word `mult`/`mflo`-adjacent instruction-order pairing
documented above, now reconfirmed against the current tree and against a
~40k-iteration search that found nothing better. Restored to
`INCLUDE_ASM`. Classification unchanged: STALL, not register-identity, not
banned-lever territory.

### Proposed learning

**A permuter score below the baseline is not evidence of progress by
itself -- diff the candidate's source against the seed before trusting the
number.** Both sub-baseline candidates found here were real bugs (a value
computed differently, silently, in a way the assembled bytes partially
disguise) rather than closer matches, mirroring the pattern
`func_80031A44`'s reports already document for a different function. A
scoring heuristic built on instruction/textual distance to the target has
no way to know the source changed what the function COMPUTES, only that
the resulting bytes moved closer on some weighted metric; that gap is
exactly where a permuter can manufacture a plausible-looking false lead,
and it is cheap to catch (one `diff` against the seed) but easy to skip
under time pressure.

## Proposed learning

**A `mult`/`mflo` pair's instruction scheduling can be entangled with the
FULL shape of the expression it feeds, not just the sub-expression
adjacent to the multiply.** Every reshaping that targeted only the TAIL of
this expression (the final `+0x14`/`+shiftVal` pairing) changed the HEAD
too (the `mult`→reload→`sllv`→`mflo` ordering), even though those
instructions look, by inspection, independent of how the result is later
combined. This contradicts the intuitive model of "reshape locally, get a
local effect" and is worth flagging for any future multiply-latency-
adjacent residue: verify the WHOLE instruction range after every attempt,
not just the words immediately around the change, because a fix attempt at
the tail can regress the head silently.

## ROUND 44 (echo): re-verified 71/73 exactly; two more negative variants, neither a new direction that helps

Re-spliced the round-36 body (with `GsClearOt`) in place of the
`INCLUDE_ASM` and rebuilt clean before touching anything: **71/73 words,
no out-of-range drift**, first real diff at vram `0x8003ED18`/`0x8003ED1C`
-- identical to every prior measurement. The inherited score is honest.

Per this round's instruction to prefer a CHANGED approach over re-running
a search, tried two variants not in the existing attempt list (11 attempts
across three prior rounds, one ~40k-iteration permuter search):

| # | attempt | result |
| --- | --- | --- |
| 12 | swap the two local declarations (`buf` before `size`, reversing attempt 1's order) | **71/73, unchanged** -- declaration order is not the lever |
| 13 | shift value computed in its OWN statement, assigned to `size` FIRST, then the product added in a second statement (`size = 4 << self->unk3C; size = size + (self->unk48 * self->unk44 + 0x14);`) -- a shape not tried by attempt 5's two-statement splits, all of which put the MULTIPLY first | **69/73**, same regression pattern as attempts 2-5/8-9: the `mult`/reload/`sllv` head breaks, not just the tail |

**Verdict unchanged: 71/73 remains the best reached.** Fourteen hand
attempts and one large permuter search across four rounds have now
converged on the same two instructions (`addu`/`addiu` operand pairing)
as the sole residue, with every attempted reshape either reproducing
attempt 1's score exactly or regressing the HEAD of the expression too.
Restored to `INCLUDE_ASM`. No new lever found this round; not escalating
the permuter search again per this round's guidance to change approach
rather than re-run one that already ran ~40k iterations.

### Proposed learning

**Declaration order for two SCALAR locals with no aliasing relationship
(here, `size`/`buf`, never both live across a call in a way that would
make order matter) is not a lever worth trying by default** -- it is a
zero-cost check (one swap, one build) but this is the second function this
project's reports show it being fully inert on (the "prologue callee-save"
class is different: THAT residue is about STORE order in the prologue
itself, not general local declaration order). Worth demoting from "try
early" to "cheap dead end" for this specific residue shape (multiply/shift
combined into one word-sized outer sum) unless a future case shows
otherwise.
