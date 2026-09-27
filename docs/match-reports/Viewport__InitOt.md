# Viewport__InitOt -- MATCHED 73/73 (round 71, runner charlie; revisit). Byte-exact whole image.

> Renamed from `Unk18Obj__InitOt` on 2026-09-25 (tools/rename.py). Address 0x8003ecd0.

> Renamed from `func_8003ECD0` on 2026-09-23 (tools/rename.py). Address 0x8003ecd0.

REVISITED, round 71: MATCHED 73/73 in 6 builds; names/types not relevant (the
lever is a local holding the constant, not a type or a name).

## ROUND 71 (runner charlie): the recorded CAUSE was incomplete -- it is fold(), and a local defeats it

**Rebuilt as given first.** The round-36 linkable body (below, `#if 0`), spliced
in place of the `INCLUDE_ASM`: **71/73, insertions 0 / deletions 0,
positional skeleton diffs 2**, first real diff at vram `0x8003ED18` -- the
title's figures exactly, and a true alignment (0/0 at equal length).

**What the old attempts measured without naming it.** Every source grouping
that contained the LITERAL `0x14` came out with the constant applied LAST
(`addu` of the two variable terms, then `addiu 0x14`, or `sllv; addiu 0x14`
on the shift before the `mflo`). That is GCC 2.6.3's `fold()` associating a
constant term outward -- `(x + C) + y` is rebuilt as `(x + y) + C` -- before
RTL exists. So retail's tree `shift + (prod + 0x14)` is unreachable from ANY
spelling with a literal, which is why fourteen hand groupings and ~89k
permuter iterations all failed: they only permuted the spelling fold then
normalised. Splitting into two statements (attempts 5, 8, 13) does dodge
fold, but changes the statement order and therefore the `mult`/`lw`/`sllv`
head schedule -- the "head regresses" signature every round recorded.

**The lever: hide the constant from fold in a local.** `s32 hdrSize = 0x14;`
is a variable at tree level (fold leaves the grouping alone) and a constant
again after RTL cse, so the `addiu` survives, applied where the source put it.

| build | variant | score |
| --- | --- | --- |
| 1 | rebuilt as given | 71/73 (0/0) |
| 2 | `0x14U + prod + (4 << c)` | 69/73 |
| 3 | `(4U << c) + prod + 0x14U` | 68/73 |
| 4 | `prod + 0x14U + (4U << c)`, and `(prod + 0x14U) + (4U << c)` | 69/73 each |
| 5 | two statements, `size = prod + 0x14; size = (4 << c) + size;` | 67/73 |
| 5 | block-local `area = prod + 0x14; size = (4 << c) + area;` (attempt 8 again) | 68/73 |
| 5 | block-local `hdr = 0x14; size = (4 << c) + (prod + hdr);` | **73/73, build exit 0** |
| 5 | block-local `hdr = 0x14; size = prod + hdr + (4 << c);` | 69/73 -- the grouping still matters, the local only lets it through |
| 6 | `hdrSize` hoisted to the function's declarations (committed form) | **73/73, OK: build matches retail** |

The unsigned-constant rows (`0x14U`, `4U`, trying `sizeof`-style
`size_t` arithmetic as a GsOT-header reading) are all negative: the
signedness does not change the fold.

`0x14` is `sizeof(GsOT)` (length, org, offset, point, tag: five words), and
`unk80 = buf + 0x14` is that header's tag array -- consistent with the
buffer being two GsOT headers plus tags plus packet area, and with both
halves being handed to `GsClearOt`. Left as an `s32` field layout (no
struct edit this round).

### Matched body (as committed in `src/code_2cc8c_d.c`)

```c
extern void GsClearOt(s32 a0, s32 a1, s32 a2);
extern void *BMemPMgrAlloc(s32 size);

void Viewport__InitOt(Unk18Obj *self) {
    s32 size;
    s32 buf;
    s32 hdrSize = 0x14; /* sizeof(GsOT) */

    if (self->unk70 != 0) {
        return;
    }

    size = (4 << self->unk3C) + (self->unk48 * self->unk44 + hdrSize);
    /* ... remainder identical to the round-36 body below ... */
}
```

### Proposed learning

**A literal constant in a sum is placed by `fold()`, not by the source
grouping.** GCC 2.6.3 reassociates `(x + C) + y` to `(x + y) + C`, so when
retail shows the constant `addiu` applied to an INNER partial sum (here
`mflo; addiu 0x14; addu`) no parenthesisation of a literal reaches it.
Discriminator: every grouping of the literal yields the constant as the
LAST (or shift-adjacent) operation, and statement splits fix the tail but
disturb the head. Lever: `s32 k = C;` then write retail's grouping with `k`.
Distinct from the scheduling-barrier lever (3x): the expression tree itself
is what changes.

---

## Prior history (the stall, rounds 14-49)

The pre-round-71 title read: STALL: length EXACT (73/73 words, no drift);
71/73 raw word-match; first real diff at vram 0x8003ED18 (the `addu`/`addiu`
pairing). ATTEMPT 6'S VERDICT IS CORRECTED BELOW.


## ROUND 49 (runner delta): a bigger permuter search (~49k more iterations) and the sibling's new lever, both negative -- residue class confirmed to be REGROUPING, not ordering

Re-verified the inherited stall first: the round-36 body (with
`GsClearOt`) reproduces exactly **71/73, zero outside-range drift** --
honest, matching this report's figure precisely.

### A bigger permuter search: same spurious leads, no zero, ~89k combined iterations now

This function was flagged this round as the most lightly-searched
residue relative to its closeness (only ~40,005 iterations on file from
round 36). Re-ran `check 3` first: `--debug --stack-diffs` reproduces
base score **215**, decomposing into 0 stack diffs / 0 branch diffs / 3
register diffs / 0 reorderings / 1 insertion / 1 deletion -- identical
to round 36's own recorded signature, and it agrees with the real
build's residue (verified by splicing and rebuilding in isolation: the
same 71/73, same two words at vram `0x8003ED18`/`0x8003ED1C`). **AGREE
-- search is meaningful.**

Ran `-j 8 --stack-diffs --stop-on-zero --best-only` under `timeout 900`.
The bound fired (`rc=124`, confirmed via its own `rc.txt` file, not the
log). **49,430 iterations** this run (combined with round 36's 40,005:
~89,400 iterations total across two rounds). **No candidate reached
zero.** The only two candidates saved below the 215 baseline (both
scoring 175) are the SAME spurious integer-truncation bug class round 36
already fingerprinted -- diffed against the seed: one narrows `size` to
`unsigned char`, the other to `unsigned short`. Both are real truncation
bugs for a buffer-size computation with no proof the runtime value stays
small, not closer matches; the permuter's heuristic scores them lower
because dead-code-adjacent instruction shuffling happens to resemble
retail's bytes in places the heuristic weights, exactly as round 36's
addendum already documents for this same function. **Not adopted; no new
information from the larger search.**

### The sibling's new lever (named-temp + barrier) does NOT transfer here either

This round's `TaskCore__RefreshSlotView` (same runner, sibling unit `code_2cc8c`)
closed a long-standing `sll`/`lw` INSTRUCTION-ORDERING swap by naming an
independent sub-computation and adding a bare `__asm__("")` barrier
immediately after its declaration. Tried the analogous shape here,
since this residue is also described as "two instructions
paired/ordered differently":

```c
{
    s32 product = self->unk48 * self->unk44;
    __asm__("");
    size = (4 << self->unk3C) + (product + 0x14);
}
```

**Result: 67/73, WORSE than the 71/73 baseline** (no outside-range
drift; a real, length-correct regression). This confirms the two
residues are mechanically different classes even though both LOOK like
"two instructions need reordering, one word out of place": D73C's
residue was the compiler choosing to interleave two independent
computations' SCHEDULE (order-only, no change to what gets computed
first structurally); this residue is an arithmetic REGROUPING (`(a+b)+c`
vs `a+(b+c)`) that changes the shape of the expression tree feeding the
SAME `mult`/`mflo` pair, which is exactly why round 39 already found
every non-naive grouping disturbs the HEAD of the expression (the
`mult`/reload/`sllv` sequence), not just the tail pairing. A barrier
between two named sub-expressions can pin their relative SCHEDULE, but
it cannot change which parenthesization the compiler's constant-folding
and associativity rules already committed to before scheduling ever
runs.

Reverted immediately; baseline 71/73 re-confirmed byte-identical.

### Proposed learning (round 49)

**A lever that closes an instruction-SCHEDULING residue does not
generalize to an instruction-GROUPING residue, even when both present
identically as "two adjacent instructions swapped."** The discriminator
is where in the compiler's pipeline the difference originates:
scheduling residues are decided AFTER the expression tree is fixed (a
barrier can constrain that later pass); regrouping residues ARE the
expression tree (a barrier has nothing to act on, because there is only
one shape being scheduled, just the wrong one). Before trying a
newly-discovered barrier lever on a "looks similar" residue elsewhere,
check which of the two classes it actually is -- round 39's own history
of "every reshape regresses the head, not just the tail" was already the
signature of a regrouping problem, not a scheduling one, and this
round's negative confirms that diagnosis rather than contradicting it.

**And: a permuter search's absolute iteration count is a weaker signal
than whether it is reproducing the SAME spurious leads across
independent runs.** ~89k combined iterations (40k + 49k, different
random seeds) converging on the identical two truncation-bug candidates
is stronger evidence of a converged negative than either run alone,
even though neither run is "exhaustive" in any formal sense.

## ROUND 46 (runner delta): drift-checked fresh, no new lever -- DELIBERATE SKIP, plus a self-caught cross-contamination scare worth recording

Re-spliced the round-36 body (with `GsClearOt`) into `src/code_2cc8c_d.c`
and rebuilt. **First attempt showed a spurious `WARNING: differs OUTSIDE
this range too (22 bytes)`** that had nothing to do with this function --
`cmp -l build/SLPS_015.56 disk/SLPS_015.56` plus
`vram = (N-1) - 0x800 + 0x80010000` pointed at `0x8003e4bc`, which is
`IntermediateBase__SetState` (a DIFFERENT function in a DIFFERENT unit,
`code_2cc8c`) -- I had left that function's own stall body live from an
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
`Snd_setVabAttr` by *dropping* a named local and recomputing inline at its one
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
(`StageMap__PopulateSlotCells`, 130/150 -> 132/150), so the attempt-6 x attempt-7
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
void Viewport__InitOt(Unk18Obj *self);
```

`Unk18ObjMethods`'s own `+0x08C`... no — not a vtable slot found in
`tools/classtable.py gViewportMethods`'s output; called directly by symbol
(caller not yet found in this unit's own queue).

## What it does

One-time allocation/init, guarded by `self->unk70` (the same latch
`Viewport__SetMaxPackets`/`Viewport__SetPacketSize` check, this round): allocates one buffer
sized to fit two internal records plus a `4 << self->unk3C`-sized payload
each, carves it into `unk78`/`unk80`/`unk88` (bases) and
`unk7C`/`unk84`/`unk8C` (bases + size), writes a 2-word header into each of
`unk78`/`unk7C`, then hands both off to `func_8003FC18` (next slice,
uncarved).

```c
/* stalesyms --fix 2026-09-22: func_8003FC18 -> GsClearOt -- names retrofitted so this body links as written; the residue it recorded is unverified until rebuilt. */
void Viewport__InitOt(Unk18Obj *self) {
    s32 size;
    s32 buf;

    if (self->unk70 != 0) {
        return;
    }

    size = self->unk48 * self->unk44 + (4 << self->unk3C) + 0x14;

    buf = (s32)BMemPMgrAlloc(size * 2);
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
sites; added a second, identical declaration ahead of `Viewport__InitOt`
itself (same pattern this unit already uses for its other local externs)
rather than hoisting the existing one. `BMemPMgrAlloc` also needed its own
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
extern void *BMemPMgrAlloc(s32 size);

void Viewport__InitOt(Unk18Obj *self) {
    s32 size;
    s32 buf;

    if (self->unk70 != 0) {
        return;
    }

    size = self->unk48 * self->unk44 + (4 << self->unk3C) + 0x14;

    buf = (s32)BMemPMgrAlloc(size * 2);
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
  value handed to `self->unk7C`/`unk84`/`unk8C` and to the `BMemPMgrAlloc`
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
  `SsUtChangePitch.md`'s round 31/32 addenda: a mutation that scores better
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
`SsUtChangePitch`'s reports already document for a different function. A
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

## Naming

`Unk18Obj__InitOt` -- tier A. One-time allocator/init guarded by `self->unk70`: allocates one buffer sized for two `GsOT` headers (`0x14` bytes each, `sizeof(GsOT)`) plus tag arrays plus packet area, carves it into the `unk78`/`unk80`/`unk88` and `unk7C`/`unk84`/`unk8C` base/size pairs, writes each `GsOT` header, and calls Sony's `GsClearOt` on both halves. "Ot" (ordering table) is Sony's own GPU term for exactly this structure, evident from the `GsClearOt`/`GsOT` shape itself.

## Track 4 (2026-09-25, round 85, bravo)

Renamed from `Unk18Obj__InitOt`. Slot +0x08C `initOt`. Its carving names the fields: `ot[2]` (+0x078, each a 0x14-byte GsOT header, `ViewportOt`, whose `length` and `org` it sets), `otTags[2]` (+0x080, each OT's tag array) and `workBase[2]` (+0x088, each half's packet area). The arithmetic is unchanged: ot[1] is still `(ViewportOt *)(size + (s32)ot[0])`, the int form, and GsClearOt's local prototype takes a `ViewportOt *`. The class (id 0x7, table `gViewportMethods`, formerly `D_8006E8E4`) is unified as `Viewport` in `include/Viewport.h`, whose banner gives the evidence for the name: its methods hold a GsRVIEW2 (GsSetRefView2), the projection and near clip, a double-buffered GsOT pair, draw the scene tree into it and flip it; IntermediateBase and TaskCore already called the field holding it `viewport`. Any source block above is the pre-unification spelling; the live body takes the unified types and field and slot names, byte-identical.

## Sony's headers (round 95, alpha, polish pass)

src/code_2cc8c_d.c now includes `<libgte.h>`, `<libgpu.h>` and `<libgs.h>` and its local prototypes of Sony functions are gone; every call takes Sony's own declaration, byte-identical. Interim casts at this function's call sites, until include/Viewport.h's ViewportOt/ViewportRefView become Sony's GsOT/GsRVIEW2: `GsClearOt(0, 0, (GsOT *)self->ot[0])` and `[1]`: `ViewportOt` is Sony's GsOT under a local name (include/Viewport.h).

The comments that sat on the deleted prototypes, moved here verbatim:

```c
/* One-time allocation of this object's two ordering tables (see
 * docs/match-reports/Viewport__InitOt.md). Each half of the buffer is a
 * 0x14-byte GsOT header, 4 << otLength bytes of OT tags, then unk48 * unk44
 * bytes of packet area. The header size is a LOCAL on purpose: written as a
 * literal, fold() reassociates the constant to the outside of the sum and
 * the final addu/addiu pair swaps (round 71). */
extern void GsClearOt(s32 a0, s32 a1, ViewportOt *ot);
/* Two more, identified in round 78 (FINISHING-PLAN track 2) and moved here
 * from include/code_2cc8c.h. Both prototypes are LIBGS.H's own; PACKET is
 * LIBGS.H's `typedef unsigned char PACKET`.
 *   GsSetNearClip   libgs/gs_101   was func_8003FB0C
 *   GsSetWorkBase   libgs/gs_124   was func_8003FBE4
 * And one identified in round 79, also LIBGS.H's own prototype:
 *   GsSetProjection libgs/gs_106   was Unk18Obj__SetGeomScreen (its argument
 *                                  is the projection distance h, which this
 *                                  unit also passes as SetFogNear's h) */
extern void GsSetNearClip(long clip_near);
extern void GsSetWorkBase(unsigned char *outpacketp);
extern void GsSetProjection(long h);
extern void *BMemPMgrAlloc(s32 size);
```


## Track 7 (round 95, alpha, polish pass)

The 0x14 header is `sizeof(GsOT)` (the unit now includes `<libgs.h>`). The local `hdrSize` stays: re-measured this round, writing `(s32)sizeof(GsOT)` inline in the sum fails the image at word 14 (0x8003ED08: retail `3c00038e`, built `3c00028e`), the same reassociation the old comment described. `buf + 0x14` is `buf + sizeof(GsOT)`, byte-identical. The function comment, before, verbatim:

```c
/* One-time allocation of this object's two ordering tables (see
 * docs/match-reports/Viewport__InitOt.md). Each half of the buffer is a
 * 0x14-byte GsOT header, 4 << otLength bytes of OT tags, then unk48 * unk44
 * bytes of packet area. The header size is a LOCAL on purpose: written as a
 * literal, fold() reassociates the constant to the outside of the sum and
 * the final addu/addiu pair swaps (round 71). */
```

Round 96 (alpha, track 6). include/Viewport.h's local `ViewportOt` (a
0x14-byte view of the GsOT header: length, org, pad) is deleted: `ot[2]` is
Sony's `GsOT *`, `otTags[2]` Sony's `GsOT_TAG *` (each header's `org`) and
`workBase[2]` Sony's `PACKET *` (GsSetWorkBase's argument), and every unit
including Viewport.h takes Sony's headers after common.h. The `(GsOT *)`
casts at GsClearOt, GsSortClear, GsDrawOt and drawNode's five sort calls and
Update's `(PACKET *)` cast are gone. Byte-identical (whole image green).

InitOt's carve, with Sony's types (73/73 on the first build; `buf` stays the
s32 address the allocation arithmetic is written in):

```c
    self->ot[0] = (GsOT *)buf;
    self->otTags[0] = (GsOT_TAG *)(buf + sizeof(GsOT));
    self->workBase[0] = (PACKET *)self->otTags[0] + (4 << self->otLength);

    self->ot[1] = (GsOT *)((PACKET *)self->ot[0] + size);
    self->otTags[1] = (GsOT_TAG *)((PACKET *)self->otTags[0] + size);
    self->workBase[1] = self->workBase[0] + size;

    self->ot[0]->length = self->otLength;
    self->ot[0]->org = self->otTags[0];

    self->ot[1]->length = self->otLength;
    self->ot[1]->org = self->otTags[1];

    GsClearOt(0, 0, self->ot[0]);
    GsClearOt(0, 0, self->ot[1]);
```


## Track 7 (round 100, echo, polish pass)

## Constants

`4 << otLength` -> `sizeof(GsOT_TAG) << otLength` (both uses): the tag array's byte size. Byte-identical although the expression is now unsigned. Fields `unk44`/`unk48` are now `maxPackets`/`packetSize`.
