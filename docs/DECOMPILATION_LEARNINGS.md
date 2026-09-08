# Decompilation learnings

Source-shape idioms and open questions for this executable. **Nothing goes here
during a parallel round** — runners put discoveries in their match report under
`### Proposed learning` and the head promotes them after merging (see
docs/PARALLEL-RUNS.md).

This file is deliberately short. It starts with what has actually been proven
against this binary, not with imported folklore; a learnings file that opens
with fifty unverified claims teaches sessions to skim it.

## Toolchain facts (proven)

- **GCC 2.6.3, Psy-Q patched, reproduces retail byte-for-byte.** Confirmed on a
  full-file rebuild with the `gcc-2.6.3-psx` tarball from decompals/old-gcc
  (sha1 `2051de9d…`), through `maspsx --aspsx-version=2.34 --dont-force-G0
  --expand-div`, assembled by binutils 2.43.1 `mipsel-linux-gnu-as`.
- **`cpp` and `cc1` do not take the same flags.** `-fno-builtin` is a cc1 flag;
  2.6.3's `cpp` rejects it outright and exits 33. This is why the Makefile keeps
  `CPP_FLAGS` and `CC_FLAGS` separate rather than sharing one variable.
- **C89 only.** A `//` comment is `unterminated character constant` /
  `parse error` from cpp, and the error points at a line number that is often
  nowhere near the actual comment. If cpp reports a parse error in a header you
  did not touch, grep that header for `//` first.
- **`-no-pad-sections` is required.** Without it gas pads `.text` and the match
  is lost. Retail's own alignment is what the linker script expects.
- **splat regenerates `include/include_asm.h`, `include/macro.inc`,
  `include/labels.inc` and `include/gte_macros.inc`** on extract when they are
  missing. Do not hand-edit them expecting the edit to survive; if you need a
  different `INCLUDE_ASM` expansion, that is a splat option, not a file edit.
- **The Psy-Q inline macro layer is INERT — do not reach for `gte_*` or the
  LIBGPU `set*` macros (round 12).** Eight headers under `include/psyq/` have
  CRLF line endings, and 2.6.3's `cpp` splices `\` only when `LF` follows
  immediately. 1134 multi-line macros (1043 in `INLINE.H`, 87 in `LIBGPU.H`,
  4 in `LIBGS.H`) therefore expand to `{\ ;` — an empty block plus a null
  statement. **That is valid C: it compiles clean, warns nothing, and emits
  nothing.** So a call to `gte_stsxy3(...)` silently does NOTHING and the
  function scores a mismatch with the stores simply absent. There are zero
  `gte_*` call sites in `src/` today, so nothing is broken yet; it is a
  landmine. Census, reproducer and disposition:
  `docs/research/psyq-header-crlf-blocker.md`. **Escalated, not fixed** —
  un-breaking 1134 macros in pinned vendored headers is an operator call.
- **A COP2/GTE store leaf has no plain-C form, and hand-rolled inline asm for
  it is NOT banned by the register rule (round 12).** There is no C expression
  that emits `swc2`. The form that reproduces retail is
  `__asm__ volatile("swc2 $12, 0x8(%0)" : : "r"(ptr) : "memory")`, and it
  passes CLAUDE.md's test: `"r"` leaves the GPR to the allocator, and
  `$12`/`$13`/`$14` are COP2 *data* registers named in the instruction text
  with no GPR identity to pin. Sony's own `INLINE.H` is built out of exactly
  this construct. Do carry the SDK's fuller clobber list
  (`"$12","$13","$14","$15","memory"`) rather than the `"memory"`-only form:
  the thin version matches for a standalone leaf whose whole body is the asm,
  but does not tell GCC the COP2 registers are live inputs and would break if
  anyone made it `static inline`.
  (`func_800196D4`, `func_800196E8`, `func_800196FC`, `func_80019710`,
  `func_80019724`, `func_8001974C`)
- **A negative result about a MACRO is only evidence once you have proved the
  macro EXPANDED (round 12).** Testing whether the SDK could express retail's
  GTE sequence produced an objdump with the macro emitting nothing at all,
  which reads exactly like a clean negative and was actually the CRLF bug
  above. Check the preprocessed output, not just the objdump:
  `cpp ... | sed -n '/yourfunc/,/^}/p'`.

### BLOCKED: no C function can reach a small-data global (2026-08-29)

**Full evidence and reproducer: `docs/research/gp-relative-blocker.md`. This is
an open operator escalation; do not attempt a toolchain change yourself.**

Retail reaches globals in the `.sdata` region gp-relatively, in one
instruction (`lw $v0, 0x40($gp)`). The pinned pipeline emits the
two-instruction absolute `lui`/`lw` form instead, and the extra instruction
shifts every later function in the same unit.

The measured discriminator: gp-relative addressing needs a non-zero `-G` at
**both** cc1 and `as`. Either alone still gives the absolute form. The project
pins `-G0` at both and passes maspsx no `-G` at all, though maspsx's README
says a `$gp` project must be passed one.

**Flipping `-G` globally was tried on 2026-08-29 with operator authorisation
and REJECTED.** It does produce retail's exact instruction shape for a blocked
function — the diagnosis is right — but a clean rebuild at `-G8` differs from
retail by 19148 bytes across 3203 runs, and `-G4` gives byte-identical damage,
which rules out the size threshold as the cause. The pin stays at `-G0`. Do
not re-propose a global `-G` change without reading
`docs/research/gp-relative-blocker.md` first.

**Before spending attempts on any function, check whether it touches a
small-data global:**

```sh
grep -l 'gp_rel' asm/nonmatchings/<unit>/*.s
```

A hit means the function is blocked by this, not by anything you can write.
Nine functions across two unrelated units were confirmed blocked in round
2026-08-29-a. It went unnoticed for 20 matches because every function matched
before that round happens to touch no global at all — a fact worth
remembering, because it is exactly why "the build is green" did not catch it.

### BLOCKED: no C function can load through a runtime-indexed global (2026-08-30)

**Full evidence, reproducer and census: `docs/research/addiu-at-blocker.md`.
Open operator escalation; do not attempt a toolchain change yourself.**

Retail resolves a `sym[reg]` address fully into `$at` before loading from it
(`lui` / `addiu %lo` / `addu` / `lb 0x0($at)`, four instructions). The pinned
pipeline folds `%lo` into the load's own displacement instead (three
instructions), and the missing instruction shifts every later function in the
unit -- which is why the affected functions score like 8/53, not "one word off".

cc1 has no opinion here: it emits a single `lbu $2,SYM($4)` pseudo-op. The
choice is maspsx's `addiu_at` flag, off at the pinned `--aspsx-version=2.34`
and on below 2.30.

Census over the whole disassembly, counting indexed accesses only: retail uses
the unfolded form **502 times across 39 files and the folded form 0 times**.
There is no counterexample in the executable.

**The remedy is not a version bump.** Below 2.30 four flags flip together --
`addiu_at` plus three nop-insertion rules (`nop_at_expansion`, `nop_mflo_mfhi`,
`nop_lw_lw`) that affect constructs inside the 84 functions that already match.
maspsx exposes no per-flag override. Same shape as the rejected `-G`
experiment.

Routing rule, effective immediately:

```sh
grep -n 'addiu *\$at, *\$at, *%lo' asm/nonmatchings/<unit>/<func>.s
```

A hit means blocked: file a stub report citing the research doc and move on.
**Address-only table arithmetic is safe** -- `Entity__GetMoodEffect` matched
6/6 against the same table because it only forms `&arr[i]` and never loads
through it. Only the load is exposed.

### BLOCKED: the `nop_mflo_mfhi` screen runs FORWARD, and the backward reading is not a blocker (2026-09-04)

**Measured this round with two reproducers through the pinned pipeline.** The
third blocker grep (Gate 1 in `docs/PARALLEL-RUNS.md`, evidence in
`docs/research/addiu-at-blocker.md`) is written as

```sh
grep -A2 -nE '\b(mflo|mfhi)\b' asm/nonmatchings/<unit>/<func>.s \
  | grep -qE '\b(mult|multu|div|divu)\b'
```

`grep -A2` prints the `mflo`/`mfhi` line **plus the two FOLLOWING lines**, so
the construct it screens for is *`mflo`/`mfhi` followed within two instructions
by another multiply-unit op*. That direction is correct and it is the one the
blocker is about. The head of round 15 re-implemented this screen in Python,
inverted it to "a `mflo`/`mfhi` within two instructions **after** a
`mult`/`div`", and got four false blockers — including a 252/258 near-miss
where a wrong CAUSE is exactly the expensive kind of error.

Both directions were then reproduced in isolation, and they behave oppositely:

```c
int div20(int a) { return a / 20; }                              /* CLEAN */
int mulplain(int a, int b) { return a * b; }                     /* CLEAN */
int mul_then_div(int a, int b, int c) { int p = a*b; return p/c; }  /* BLOCKED */
int div_then_mul(int a, int b, int c) { int q = a/b; return q*c; }  /* BLOCKED */
```

- **`mult` -> `mfhi` (the hazard-slot direction) is NOT blocked.** The pinned
  pipeline reproduces retail's signed-divide-by-constant idiom exactly —
  `mult a0,magic` / `sra a0,a0,31` / `mfhi v0` / `sra` / `subu`, with the `sra`
  filling the slot and **no `nop` inserted**. Same for a bare `mult` / `mflo`
  with a zero-instruction gap. So every `mult; ...; mfhi` in retail is ordinary
  matchable code, however tight the gap looks.
- **`mflo`/`mfhi` -> `mult`/`div` IS blocked.** The pipeline inserts **two
  `nop`s** between the result read and the next multiply-unit op, which retail
  does not have. This is the case `addiu-at-blocker.md`'s 2026-09-01 addendum
  already recorded for `func_8005950C` (`mult; mflo; div`); the reproducers
  above show it fires for `mflo -> mult` as well, not just `mflo -> div`.

Corpus census with the screen the right way round, over all 285 queued
functions: **7 hits**, and all seven already have match reports naming the
blocker. So the existing stall corpus has no mis-filed cause here — and 7 of
285 keeps Gate 1's decision to leave this grep with the HEAD rather than
promoting it to a per-runner screen correct on measured grounds.

The generalisable part is not about multiplies. **A screen is a claim with a
direction, and an inverted screen fails in the worst way available**: it
manufactures blockers on functions that are actually workable, and CLAUDE.md's
own asymmetry applies — a wrong score gets corrected the next time anyone
measures, a wrong CAUSE is what the next round acts on. If you re-implement one
of the three greps in another language, reproduce a known-positive and a
known-negative through the pinned pipeline before believing its output.

#### Round 24 addendum: the TWO in "two nops" is load-bearing, and Gate 1 contradicted this entry

Re-measured independently 2026-09-08, same reproducer, same result: the pinned
pipeline emits `mflo $a0` / `nop` / `nop` / `mult $a0, $a2`. **Exactly two.**
That number is what makes the screen's two-instruction window a discriminator
rather than a heuristic: a `mult`/`div` landing within two instructions of the
`mflo`/`mfhi` is precisely the case where retail cannot absorb the insertion.

**The entry above had this right and Gate 1 in `docs/PARALLEL-RUNS.md` had it
wrong** — it defined the blocker as an `mflo`/`mfhi` followed within two
instructions by a `mult`/`div` *"with no `nop` between them in retail's own
bytes"*. That qualifier does not survive the reproducer: retail's single `nop`
still leaves the sequence one word short, so `mflo` / `nop` / `mult` is
blocked like any other. Gate 1 is corrected; this entry is the one that was
already faithful.

Round 24's head acted on the wrong half. Taking the qualifier literally, it
"refined" the canonical grep with a nop test and concluded that two flagged
functions were assignable — one of them `func_8005D864`, which round 23 had
just carefully re-adjudicated *onto* this blocker. The reproducer overturned
the refinement in under a second and it was discarded; the corpus count stands
at **12 flagged, all 12 genuinely blocked** (253 queued functions,
2026-09-08 — up from 7 of 285 because round 24's carve added three).

Three things worth keeping:

- **A doc's prose is not a specification of a toolchain behaviour.** Two
  project documents described one mechanism and only one of them was right,
  with nothing local to tell them apart. The pipeline is the tiebreaker and it
  costs a second.
- **This screen has now been broken in both directions it can break in** —
  inverted window (rounds 15, 16) and false qualifier (round 24) — by four
  different heads, every one of whom had read the warning not to re-implement
  it. Treat "improve the mflo screen" as a smell, not a task.
- The failure mode here is the MIRROR of the usual one: not a false blocker
  (which deletes matchable ground permanently) but a false CLEARANCE, which
  staffs a runner into a real wall on a function whose report correctly said
  not to. Cheaper than a false blocker, and still paid by somebody who did
  nothing wrong.

## Build hygiene (proven, the hard way)

### Address drift's most convincing disguise is a PLAUSIBLE WRONG CONSTANT in the function you are editing (round 24)

The existing entry below says drift "looks exactly like a broken symbol". Here
is the sharpest instance measured so far, because the wrong value is not
garbage -- it is off by a round number, in the one instruction a reader is most
likely to trust.

`func_80059814` (`DreamSys`) re-measured as:

```
retail:  addiu at,at,0x7e50        built:  addiu at,at,0x7e40
retail:  addiu at,at,0x7e5c        built:  addiu at,at,0x7e4c
```

Both table bases exactly 0x10 low, on two independent symbols, alongside a
101043-byte out-of-range diff. That reads as a symbol or a linker-script
problem, and `D_80087E50` *is* a real symbol at a name-matching address.

`build/lsdde.map` settles it in one grep: `D_80087E50` was linked at
`0x80087e40`. **The function was 4 words (0x10 bytes) short, so its object's
text was 0x10 small and every symbol after it -- including all of `.data` --
slid down by 0x10.** The wrong-looking immediate IS the length bug, observed
from the far end.

Two things worth carrying:

- **A uniform offset shared by several unrelated symbols is drift, not a
  symbol bug.** One wrong symbol is a symbol bug; three wrong by the same
  amount is your function's size.
- **Check `build/lsdde.map` for the symbol's linked address before believing
  any symbol-shaped diff**, and read funcdiff's out-of-range byte count in the
  same breath -- it was printed by the same command that produced the score.

- **Address drift looks exactly like a broken symbol, and TWO runners lost
  time to it independently in one round.** If your function is the wrong
  length, every symbol linked after it shifts, so an unrelated and already
  correct data symbol resolves to the wrong address in the map, and a byte
  diff shows a "wrong" immediate. Both runners suspected symbol or table
  layout; in both cases the real fault was their own function's word count.
  **Check your own function's size first, and check the plain `.o`'s
  relocations (`objdump -dr`) before suspecting anything about symbols.**
  funcdiff's "differs OUTSIDE this range" count firing at the tens-of-KB
  scale is the tell that you are looking at drift, not at a symbol bug.
  (`func_8005FB6C`, `func_8005AB2C` — found separately, in different units.)
- **Do not share a scratch path between parallel runners.** The runner prompt
  in PARALLEL-RUNS.md used to hardcode `/tmp/b.log`, so every runner in a
  round wrote and grepped the same file, and one runner read another's build
  result before noticing. That is a false oracle in the one place the workflow
  cannot afford one — the log a runner greps to decide whether its score means
  anything. Runner-unique paths are now mandatory; see the runner prompt.

- **A header edit rebuilds everything, on purpose.** The Makefile makes every
  C object depend on every header. Without that, editing a struct layout
  rebuilt nothing, `make` reported success, and the next funcdiff scored the
  new layout against an object that had never seen it — a stale number that is
  plausible and self-consistent. A full build is under a second here;
  correctness is cheaper than fine-grained dependencies.
- **Data is decompiled like code, one slot at a time.** splat's dot-prefixed
  segment form (`.data, DreamSys`) means "this data is DEFINED IN
  src/DreamSys.c", so it emits nothing and the link fails on every symbol in
  the slot until someone writes the arrays out as C. Slots start as plain
  `data`/`rodata` segments and flip to the dot form in the same commit that
  writes their C.
- **Splicing a salvaged body into another checkout's file drops its
  `#include`s, and the resulting parse error reads as a match.** Recovering
  runner/echo's 14 bodies, the head spliced each into main's `src/DreamSys.c`,
  which lacked the `#include "DreamSys.h"` echo had added. Every `DreamSys *`
  became `parse error before '*'`, the compile failed, the previous build
  stayed in place — and all 14 funcdiff reads came back full matches from that
  stale build. They were plausible, self-consistent, and entirely fictional.
  funcdiff's STALE BUILD guard is what caught it. **When splicing a body from
  another tree, take its file preamble too, and confirm the base file builds
  GREEN on its own before scoring anything against it.**
- **A FLAG change rebuilds NOTHING, so every flag experiment starts out
  falsely green.** Every object depends on every source and header — but not
  on the Makefile. Edit `CC_FLAGS`/`AS_FLAGS`/`MASPSX_FLAGS` and
  `./build-and-verify.sh` reports OK, because it is still the previous build
  compiled with the old flags. This is nastier than the ordinary stale-build
  trap: there is no failed compile anywhere, nothing is newer than anything,
  and funcdiff's mtime guard cannot see it either. **`rm -rf build` before
  timing or trusting any flag experiment.** A `-G8` trial reported a clean
  green this way and the real answer, from scratch, was 19148 differing bytes.
- **`build exit=` is not advice you get to weigh against a good-looking
  number.** In the incident above the exit status was printed next to every
  one of the 14 scores and was `2` every time. Printing it is not the control;
  refusing to read the number unless it is `0` is the control.
- **One exception: a rodata slot holding JUMP TABLES must stay attached to its
  unit.** Its words point at `.L8005....` labels *inside* the unit's functions,
  which are local to each function's `.s` file, so a standalone rodata object
  cannot resolve them and the link dies. `migrate_rodata_to_functions` puts each
  table in the same `.s` as the switch that uses it. `0x1F88` (DreamSys) is the
  one such slot today.

### `funcdiff.py`'s byte range is not a stalled function's true length

**Round 10.** For a function that is still `INCLUDE_ASM` or whose C compiles
to a different size, the range `funcdiff.py` prints is the range it COMPARED,
not evidence of what your C actually produced. `func_8004BB3C` scored 14/105
while being structurally 104 of 105 instructions identical to retail — one
missing `addiu $s4,$s4,0xc` shifted everything after it, and the score
understated the near-miss by an order of magnitude.

**So a low score is not evidence of a distant shape.** Cross-check the real
compiled length before concluding a body is wrong:

```sh
tools/binutils/bin/mipsel-linux-gnu-objdump -d build/src/<unit>.c.o
```

This is the same family as "address drift" in CLAUDE.md's four-ways-a-score-
lies list, seen from the other side: drift makes a good score untrustworthy,
and it also makes a bad score uninformative.

## Source-shape idioms

### A dense `switch` must reproduce retail's JUMP-TABLE WIDTH, not just its arms (round 25)

**`func_80058C58` (`DreamSys`) matched at 79/79 once a no-op high `case` was
added to widen GCC's jump table.** Its thirteen real arms compile to a
**33-entry** table; retail's is **48 entries**. The arms were all correct and
the body was not defective — the table was simply narrower than retail's, so
everything after it shifted.

GCC 2.6.3 sizes a dense-switch table from the span of the case values it can
see. If retail's table is wider than yours, retail's source had a case label
your source does not — most likely an empty or `break`-only arm that compiles
to nothing but still extends the span. Adding `case 47:` (a no-op falling into
the default behaviour) widened the table to retail's 48 and closed the
function.

**The diagnostic, which is the reusable part: check `funcdiff`'s first-diff
offset against the RODATA TABLE BASE before assuming a body defect.** A first
diff that lands at or just after the jump table's own address is a table-width
problem, not a code problem, and reading it as a code problem sends you into
the arms — which are fine. This is the third member of the "retail's own
layout is readable off the binary" family, alongside the case-ORDER idiom
below and the block-ORDER entry above: here it is the case *span*.

### An arm that must JUMP has to be written NOT-LAST (round 25)

**GCC 2.6.3 lets one arm of an if/else fall through into the join, and it
always picks the LAST one. So retail's block ORDER is a source decision, and
a residue caused by it does not respond to any amount of expression
reshaping.** This closed `func_8005CBC8` (`code_4cd08`) at 100/100 after
three rounds pinned at 99/100, thirteen hand attempts and ~77k permuter
iterations — every one of which was searching the wrong axis.

Retail's preamble there is laid out
`[test] [return false] [idx = sel arm] [idx = ~sel + 1 tail] [join]`. The
`idx = sel` arm sits BETWEEN the fail path and the negate tail, so it needs
an explicit `j` over the join. The obvious C puts that arm last, where it
falls through and no jump is emitted:

```c
    /* 1 word SHORT, forever, for every expression-level reshape */
    if (sel < 0) {
        if (record->unk0 != 0) {
            return false;
        }
        idx = ~sel + 1;
        goto have_idx;
    }
    idx = sel;
have_idx:
```

The fix is to invert the inner guard so the negate arm becomes the forward
`goto`, and then place the jumping arm textually ABOVE the label:

```c
    if (sel < 0) {
        if (record->unk0 == 0) {
            goto negate;
        }
        return false;
    }
    idx = sel;
    goto have_idx;

negate:
    idx = ~sel + 1;

have_idx:
```

cc1 then emits retail's five blocks in retail's order, `j` included, and
`~sel` still lands in the `beqz` delay slot as `nor $v0, $zero, $a0` without
being asked to. No `__asm__`, no barrier, no operand constraint.

**The tell in the disassembly:** a lone unconditional `j` (not a conditional
branch) whose target is the next join a few instructions below, and whose
delay slot carries real work rather than a `nop`. That jump exists because
retail's compiler had a block after it, so the source has to put a block
there too.

**This is the block-order sibling of the "shared `return` block's PLACEMENT
follows the FIRST return in source order" entry immediately below, and of
the "case order follows the jump table" entry further down.** All three are
cases where retail's own layout is readable off the binary and must be
reproduced rather than reasoned about; this one generalises them from
`return` blocks and `switch` arms to ordinary if/else arms.

#### Instance 2, a DIFFERENT shape: a duplicated assignment retail keeps and GCC merges

**This is what makes the entry a rule rather than an anecdote, and it is the
more COMMON of the two shapes.** `func_8005DBF0` (`Entity`) closed at 74/74
after four rounds at 2 words short, through ELEVEN structural variants and a
`__asm__("")` barrier. Not one character of its logic changed.

Retail keeps two textually distinct `doDetach = 1;` writes reaching one merge
point, and spends two extra instructions to keep them separate:

```
; the detachKind==2 arm
bne  $v1, $v0, MERGE
 nop
j    MERGE
 li  $s2, 0x1        ; its OWN write, in the delay slot of an otherwise-pointless j
; ... the rand-check arm, elsewhere ...
li   $s2, 0x1        ; a SECOND, textually distinct write
MERGE:
```

The build instead lets the kind==2 arm fall through into the rand arm's `li`
— fewer instructions, correct, not retail. In the stalled body the
rand-check block was nested INSIDE an earlier arm, textually before the
kind==2 write, so kind==2 was last and took the fallthrough. Lifting the
rand-check block OUT of the nesting to the end of the gated block, behind an
explicit `goto merge`, makes the rand write last and forces the kind==2 arm
into retail's shape.

So both shapes reduce to one fact: **GCC 2.6.3 gives the fallthrough to
whichever candidate is LAST in source order.** Shape 1 is an if/else arm that
must jump over a join; shape 2 is a duplicated assignment retail keeps in two
copies. The fix in both is textual: make the block you want to JUMP not-last,
using an explicit `goto` over the block you want to fall through. Moving a
statement out of the nesting looks like it changes the meaning, which is why
eleven variants that all permuted things INSIDE the nesting could not find it
— it does not change the meaning once the `goto` is explicit.

#### "A barrier had no effect" is positive evidence FOR this lever

`func_8005DBF0`'s report had the mechanism right before the fix existed:
*"a scheduling barrier placed both before and after the arm's `doDetach = 1;`
— no effect, confirming this is a cross-basic-block CFG/tail-merge decision,
not an intra-block scheduling one."* That reasoning is correct and the
missing step is that a cross-basic-block decision has a SOURCE lever too — it
is textual placement rather than a barrier. So when a report says a barrier
did nothing, that is a positive indicator for block order, not a sign the
function is exhausted. Two of the two functions closed by this lever had
exactly that note in their history.

Corollary for isolated reductions: a reduction usually has only ONE candidate
for the fallthrough, so the merge does not arise and a barrier appears to fix
it. The real function has two or more, and placement decides. That is the
shape of "the barrier works in the reduction and transfers to nothing".

#### Why the permuter cannot find this, which is the routing consequence

The permuter mutates expressions, declarations and statement order WITHIN a
control-flow shape. It does not restructure control flow into a different
basic-block LAYOUT. So a layout residue is invisible to it and presents as a
flat plateau — which is exactly how this one presented for two rounds
(77,264 iterations, score never below base). **A residue that survives both
hand reshaping and a long search is therefore evidence FOR block order, not
evidence that the function is exhausted.**

#### THE LEVER IS NOT STRICTLY DOMINANT — diagnose per instance, do not pattern-match

**Two runners reached this independently in round 25, from different units,
and it is the correction to how the lever was first broadcast.** The head's
mid-round broadcast described one shape and one fix; runners who
pattern-matched it onto a superficially similar residue lost ground.

- **`func_80059BE0` (`DreamSys`, echo): applying the lever made it WORSE.**
  Its entry guard already had its default value living in the guard's own
  delay slot — a *different* and already-correct idiom. Forcing the explicit
  `goto` there reintroduced a tail-merge defect the function had previously
  escaped. **Check which of the shapes the disassembly ALREADY shows before
  applying anything.**
- **`func_80036230` (`code_179d8_f`, delta) needed a THIRD variant**, not
  either of the head's two. Its residue was a guard whose instruction order
  was backwards; the fix was swapping *which side owns the `if` body* — put
  the success case inside `if (success) { ... return 0; }` and demote
  `return -1;` to a bare trailing statement. Closed on the first structural
  variant tried after that reading, at 115/115.
- **`func_80059814`/`func_800598E8` (`DreamSys`, echo) needed a fourth**: an
  explicit `goto` to a SHARED label rather than if/else, applied whenever
  retail shows one arm reaching a merge point via a real jump and the other
  via fallthrough. That took `func_80059814` from 14/53 to 49/53 and exact
  length, and made `func_800598E8`'s whole CFG match.
- **`GenerateInitialSpawn` (`DreamSys`, echo) needed the OPPOSITE of a shared
  merge:** giving each `if`/`else` arm its own `return stage;` instead of one
  shared return is what cleared a register-identity cluster. So "share the
  tail" and "duplicate the tail" are both levers, and which one applies is
  read off retail, never assumed.

So the transferable content is the QUESTION, not any of the four fixes:
*which block does retail place where, and which candidate did my source make
last?* Answer it from the disassembly per function. Round 25 closed three
functions with four different placement fixes, which is the ratio to expect.

#### "A barrier does not transfer" has (at least) THREE causes; this lever explains ONE

**Round 25's charlie tested the block-order hypothesis against a whole unit's
stalls instead of adopting it, and decomposed the symptom.** `code_179d8_g`'s
long-standing puzzle — a bare `__asm__("")` fixes the "redundant-raw-copy
elision" in an isolated reduction and transfers to none of the real functions
— is not one mechanism:

1. **Block-order / shared-join fallthrough.** CONFIRMED, and found in
   `func_8002AEE0` INDEPENDENTLY, before the head's broadcast naming it
   arrived. An early `return -1;` written inside a timeout arm compiles
   SHORTER than retail, which keeps a "redundant" `move $v0,$zero` /
   `bnez $v0,<epilogue>` pair at the join. Restructuring to
   diagnostic-block-FIRST, success-label-SECOND, one shared `result` checked
   once took it **61/174 -> 153/174 in a single fix** — the largest single
   gain of that session. The same shape found twice independently in two
   functions is stronger evidence than either instance.
2. **Pure intra-block scheduling tie-breaks.** `func_8002A75C`'s residue is
   confirmed via the permuter's `--debug` breakdown (zero insertions, zero
   deletions, zero register differences — pure reordering) to have no second
   block at all, and a barrier STILL fails to move it: the right category of
   instrument, no position found that works.
3. **Dead-code elimination.** `func_8002B94C`'s redundant-looking check is
   removed by the optimizer BEFORE scheduling runs, which no barrier
   placement can rescue.

So diagnose from the real `.s`'s block/label structure and, where available, a
permuter `--debug` breakdown, before reaching for a barrier as a first move.

#### The NAIVE form of the fix is wrong once block order is already right

**This is a twice-reproduced counter-example and it corrects the way the lever
was first broadcast.** "Retail keeps two materializations, so give the
duplicate its own C variable" is NOT the fix. On `func_8002B4D4`, whose block
order already matches retail exactly (its if-branch tail carries the explicit
`j` and its else-branch correctly falls through — verified against the raw
`.s`), splitting the shared pointer into a second named variable was tried
independently in TWO rounds and **regressed both times** (to ~17/91, and
60 -> 50/91). A second named pointer changes register allocation for the WHOLE
function, because the allocator's pressure budget is shared across the entire
body, not local to the one store.

Both things are true at once, and the order matters:

- The MECHANISM — retail keeps two independent materializations, and block
  order decides which one earns the fallthrough — is real and worth checking
  first.
- "Split it into two C variables" only helps **while the block order is
  WRONG**. Once it is right, that lever is actively harmful, and what remains
  is ordinary register allocation.

#### A negative that bounds it: the delay-slot-hoist family is NOT this

Echo screened five `DreamSys` stalls against both shapes and found none:
`func_8005A82C`, `func_8005A9CC`, `DreamSys__InstanceEffectsOnJournal` and
`func_80059BE0`'s remaining word are all the same *different* residue — the
compiler schedules an independent instruction into a delay slot that retail
leaves empty or fills differently. Delta likewise found **no bare `j`
anywhere** in `func_80065A5C`, which confirms that function's residue is
genuinely pure scheduling rather than a layout question the permuter is blind
to (144,658 iterations, best score never moved). That family remains open and
is worth its own investigation; do not spend block-order attempts on it.

#### The mechanical screen for this has NO measured precision — do not build one

The obvious next move is to grep the queue for the tell. That screen was
written and run in round 25 over every blocker-clean live `INCLUDE_ASM`
(bare `j`, target within ~8 entries below, non-`nop` delay slot): **51 of
the queue's functions match it.** It was then tested on one candidate,
`func_8004CFB8` (`class_3bb8c_b`), and the hit landed in that function's
ALREADY-BYTE-EXACT half — the `j`-to-join shape there is one cc1 reproduces
naturally from plain nested `if`/`else`, so it was a false lead.

Precision on the sample actually tested is therefore **1 tried, 0 genuine**,
and the screen is not in `tools/` deliberately. `j`-to-join is the *normal*
if/else shape; the signature is not the jump, it is the jump PLUS a block
after it that your natural C would place last, and nothing greppable
distinguishes those. Use the tell when you are already reading a specific
function's disassembly. Do not use it to rank the queue, and do not promote
it to a standing screen without measuring precision first — this project has
paid four times for screens whose scope was reasoned instead of measured
(see PARALLEL-RUNS.md, Gate 1).

#### One measured counter-indication: an `mflo`/`mfhi` residue is NOT this

`func_8004CFB8`'s real residue is a deferred single `mflo` where cc1 extracts
eagerly. Round 25 tried the cross-jumping reading of it — give both arms an
identical `mflo`/`sw` tail so GCC merges them and absorbs the shorter arm's
lone `mult` into the `bgez` delay slot, which is precisely retail's shape —
and it came out **5 words too long**; GCC emitted an `mflo` and an `sw` in
each arm and merged nothing. cc1 expands a `mult`/`mflo` pair together during
RTL expansion, BEFORE any block-layout decision, so block order cannot move
them apart. When you are weighing whether a residue is a layout one, a
`mult`/`div`/`mflo`/`mfhi` in it is a negative indicator rather than a
neutral one. (`docs/match-reports/func_8004CFB8.md`, variant 7.)


### A shared `return` block's PLACEMENT follows the FIRST return in source order (round 24)

**N separate `return X;` statements and one `goto` with a trailing label are
NOT equivalent, even though GCC merges the duplicates either way.** It puts
the merged block where the FIRST one was. So a function whose failure paths
all `return 0` gets that block early if the first `return 0` is early, and
retail's shape -- failure block sitting immediately before the epilogue, with
the success path `j`umping OVER it -- is the `goto`-with-trailing-label form:

```c
    if (early_bail) {
        goto fail;
    }
    ...
    for (...) {
        if (nothing_found) {
            goto fail;
        }
    }
    /* success */
    return 1;

fail:
    return 0;
```

Measured on `func_800585B4` (`class_3bb8c_t`): worth 2 words directly, and it
took the function from **5/56 to 15/56** because it also re-shaped the entry
branch -- every word of the prologue became exact.

**The diagnostic is specific and cheap, and it is worth knowing because the
symptom points at the wrong thing.** If the entry test's branch POLARITY is
inverted against retail (`beqz` where retail has `bnez`) *and* there is a
stray `j` + `move v0,zero` pair early in the function, the failure block is in
the wrong PLACE. Do not go looking for a condition you spelled backwards --
the condition is right and the block ordering is wrong.

Related and separate: a loop counter's SIGNEDNESS is directly visible.
Retail's `sltiu` against a small constant means the counter is unsigned;
`slti` means signed. Worth 1 word on the same function, and it is invisible
in any other part of the output. This is the signedness sibling of the
round-23 finding that a local's declared WIDTH is a codegen decision.

### A symbol accessed at TWO WIDTHS by one function needs a pointer CAST, not a scalar truncation (round 24, bravo)

Where one function writes a symbol with `sh` and reads the same address back
with `lbu`, the reproducing source is a **plain pointer-cast dereference** --
`*(u8 *)&sym` -- and NOT a scalar truncation cast of the loaded value. The
truncation form widens back to a full-width load plus a mask, which costs
words.

Three related findings from the same runner, all from `code_179d8_m`:

- **`volatile` on a POINTER TYPE and `volatile` on the OBJECT are different
  levers with OPPOSITE effects.** A volatile-qualified pointer cast defeats
  the compiler's address fold and ADDS instructions. A volatile OBJECT read
  through a plain non-volatile pointer or cast still folds fine, and is what
  stops this compiler proving a narrow-range store-then-reload redundant. The
  existing caution about `volatile`-as-a-lever does not distinguish these, and
  reaching for the wrong one looks like the lever failing.
- **A recomputed-vs-reused array offset -- same index, different narrowing --
  is diagnostic of two different WIDTH VIEWS of one variable at two source
  sites.** Reproduced by casting the index at only the SECOND occurrence.
  Complements round 23's "a local's declared WIDTH is a codegen decision":
  the width can differ per USE, not just per declaration.
- **When a cross-unit callee already has a documented prototype -- even a
  guessed one -- type the CALLER's parameters so that prototype's implicit
  conversions emit the exact per-call narrowing seen in the disassembly.** The
  narrowing instructions belong to the call, so they are evidence about the
  caller's declared parameter types, not about its body.

### A block of stores far ahead of a branch may be entirely UNCONDITIONAL (round 24, bravo)

A run of stores that precedes a branch by many instructions reads as though
the branch gates it. Verify there is no branch BETWEEN the stores and the
point you assume gates them, by reading the `.s` file's own label and branch
structure -- not an earlier paraphrase of it, and not the shape m2c produced.

Related, and a genuine addition to the register-identity toolkit: **a pure
register-identity swap between two INDEPENDENT accumulator pairs (two
OR/AND-NOT sequences) can be fixed by INTERLEAVING the writes** -- both ORs
before either AND-NOT -- rather than by declaration-order or block-order
reshaping, which is where the existing levers point. Worth trying before
filing a register-identity stall on a function with two parallel accumulators.

### Two globals a few bytes apart with a COMMON stride are one array; separate `lui`/`addiu` pairs prove nothing (round 24)

`func_8002BC40` (`code_179d8_d`) reads `D_8008B9F4` and `D_8008B9FC`, each
with its own `lui`/`addiu` materialisation, each advancing by 0x2C per
iteration. They are **one** array: the delta is 8, which is the second
member's offset, folded into the symbol at compile time. One `for` loop over
one array of 0x2C-byte structs produced both.

The split is explained by what each member is USED for, not by the source:

- A member that is only **tested** gets the indexed-global form
  (`lui $at` / `addiu $at` / `addu $at,$at,<idx>` / `lw`), needs no register
  live across the loop, and so has its base **re-materialised inside the
  loop**.
- A member whose **address is passed to a call** must be a real register
  value, so loop strength reduction turns it into an induction variable
  initialised to `base + offset` and bumped by the stride.

**Read the STRIDE first.** A common non-power-of-two stride on both symbols is
the tell, because it means one `sizeof`.

This is the converse half of round 23's `func_80031CF0` adjudication, which
established that a SHARED base with a small positive fold (`addiu $t2, $a3, 2`)
is conclusive FOR one object. The pair now covers both directions:

| evidence | conclusion |
| --- | --- |
| shared base register, small positive fold | ONE object -- conclusive |
| separate `lui`/`addiu` per symbol | **nothing either way** |

The second row is the one that misleads, and round 23 already warned why: "each
field gets its own `lui`/`addiu`" measures whether GCC CHOSE to share a base
register in that one function -- a register-pressure question -- not whether
the symbols ARE one object.

### A "register-identity" verdict is the least reliable class in this corpus (round 18)

**Two functions filed as unfixable register-identity stalls were overturned in
one round, and a census then found the class is 26% of everything queued.** Put
this near the top because it is the largest pool of possibly-recoverable ground
the project has, and because the verdict is self-sealing: a function filed this
way stops attracting attempts, so nobody re-measures it.

The two overturns, both by ordinary C:

- **`func_8004EEA0` (matched 51/51).** Filed as "register-identity
  permutation, not fixable by reshaping". The actual cause was **a masked byte
  parameter mistyped `s32`** — only ever used as `a3 & 0xFF`. Retyping it to
  `char` closed the whole function. Nothing was ever a rotation of independent
  values; one wrong type cascaded into a register-shaped appearance across the
  entire body.
- **`func_8004042C` (25 words, now 22/25 — see the correction below).** Filed as
  a whole-function register-bank swap (`self`/`a1` -> `$a3`/`$t0`), with its
  report explicitly recording that the permuter was never tried *because of*
  that filing. Rewriting a field-copy pair as one whole-struct assignment fixed
  a real structural defect **and** flipped `self`'s allocation to `$a3` to match
  retail.

  **CORRECTION (round 19): this entry originally read "now 1 residue word" and
  that figure was wrong.** It came from reading the permuter's weighted penalty
  score (210 = 2 register-diffs x5 + 1 insertion x100 + 1 deletion x100) as a
  word count. Rebuilt against the real oracle the body is **22/25 — three wrong
  words.** The missing `move $t0,$a1` is one missing *instruction*, but its
  absence forces two downstream loads to read `$a1` instead of `$t0`. A
  single-instruction cause is not a single-word residue. The wrong figure
  propagated from the function's own report title into PROGRESS, into round
  19's staffing, and back to a runner as an instruction to "close the one
  remaining word" before anyone rebuilt it. See "A permuter number is in
  PERMUTER units, not retail words" below. The function is now
  permuter-exhausted across two independent searches (28k + 48k iterations).

**The mechanism to internalise: a wrong TYPE produces register-shaped
symptoms.** Signedness, width, and a parameter's declared type all change
allocation and coalescing, so the visible diff is "the values are in the wrong
registers" while the fault is upstream in a declaration. `func_8003FCFC` is a
third instance found the same round: its "register bank differs" verdict was a
**discarded return value** — retail copies `dst` into `$v0` for a function
declared `void`; retyping to `s16 *` with `return dst;` moved `--debug` 290 ->
135.

**But do NOT over-apply the type lever — it closed exactly one function and was
a clean negative everywhere else it was tried.** Measured the same round:
alpha's five variants across two functions (signed bitfield markedly worse,
widened copy and concrete struct types all unchanged), charlie's two exhaustive
searches (432-combo and 24-combo, no movement), echo's direct test on
`func_8004C93C` (**1/109 with major drift — much worse**), delta's checks
(fields already word-width, axis inapplicable). It is one axis worth trying
early because it is cheap, not the answer.

**The epistemics trap, which is the transferable part.** `func_8004EEA0`'s
wrong verdict had been *corroborated by a sibling* — `func_8004C93C`, seven
variants, zero movement. **Two functions agreeing did not validate the class,
because both shared the same unexamined assumption.** A stall class several
reports agree on is not thereby confirmed; it may be one error copied. Echo
wrote the correction into `func_8004C93C`'s own report in the right words:
*`func_8004EEA0`'s fix does not validate this function's classification — same
symptom, different function, not yet the same diagnosis.*

**And a register-shaped SYMPTOM is not a register-shaped RESIDUE.** Screen it,
do not read it. `func_80066340` sits in the census and turned out to have
`Register Differences: 0` — six words of pure instruction *reordering* in a
divide-by-360 magic-multiply setup. `--debug`'s bucket breakdown answers this
in one command; the report's prose does not.

Axes that DID close register-shaped residues in round 18, none of them exotic:

- **Retype a parameter to its real width** (`func_8004EEA0`).
- **Whole-struct assignment instead of a field-copy pair** (`func_8004042C`).
- **Split one combined expression into two sequential statements** — how
  `func_8001EDAC` matched 22/22 after seven failed manual attempts: splitting
  `*word = (*word & ~mask) | (value << shift);` in two closed a register-pair
  swap. A genuinely distinct axis from order/naming/operand-order.
- **`return dst;` on a wrongly-`void` function** (`func_8003FCFC`, partial).

Confirmed INERT for this class, so do not spend attempts on it: **declaration
and introduction ORDER**, across 3 functions and 9 attempts, byte-identical
every time.

#### Round 19 confirmed the class and added four more mechanisms

A second full round against this corpus (5 runners, 67 blocker-clean
register-shaped functions) closed **17** functions. The verdict remains the
least reliable in the corpus, and the list of things that hide behind it is now:

- **A missing field-offset term.** `func_80040C00` (matched 52/52) was filed for
  rounds as a same-length "register permutation from word 1". The body was
  simply missing a `+ self->unkAC`. A wrong *value* looks exactly like a wrong
  *register*.
- **A statement-order swap between two independent stores.** `func_80066214`
  (matched 37/37) — swapping two unrelated stores freed the register retail
  needed and closed a 23-word gap. No barrier required.
- **A stale helper signature.** `func_8004A7C0` (matched 113/113) — the
  inherited body used no-argument declarations for two helpers that an
  already-matched sibling constructor had since corrected. **Check what matched
  siblings in the unit already do before re-deriving anything**; echo closed
  most of this function by adopting the sibling ctor's own pointer-walk idiom.
- **A cross-call cached local masquerading as register-file SATURATION.**
  `func_8003D73C` — removing one took it from 9 registers (saturated-plus-one)
  to retail's exact 8-register file, 144 of 145 words. It did not close, but the
  *saturation* verdict dissolved entirely. If a report blames a saturated
  register file, look here first.

Also worth carrying: a **misread of GCC's own scheduling as source order**.
`func_80058228` (matched 56/56) had a preserved body writing three colour-field
decrements as `r, b, g`, on the theory that retail's write pattern demanded it.
That apparent order is GCC interleaving three independent byte operations; the
plain declared order `r, g, b` reproduces retail. Same family of error as
reading a permuter bucket label as a word count — **a compiler artifact is not
evidence about the source.**

Genuine register rotations do exist and several were re-confirmed this round
(`func_8004BB3C` at 7 registers — not saturated; `func_8004CAF0` narrowed to a
clean 3-register rotation; `func_80066340` a clean 3-instruction rotation with
`Register Differences: 0`). The class is real. It is just heavily contaminated,
and every contaminant so far has been ordinary C.

### A near-miss word count describes WORD COUNT, not the number of divergences (round 18)

`func_80063144` was carried as a **216/217** one-word residue — the most
attractive target of the round. `permuter.py --debug` scored its preserved body
at **465**, not the ~200 a genuine one-instruction residue gives, and the
reason was that **two divergences of equal size were hiding behind a word-count
coincidence.** Closing the first moved funcdiff 69/217 -> 77/217 and `objdump`
confirmed it closed that one and only that one; the second is still open.

So: **before believing "everything else matches", cross-check the word count
against `--debug`'s bucket breakdown** (`Register Differences`, `Reorderings`,
`Insertions`, `Deletions`). They measure different things and only the second
one tells you how many independent problems you have.

**File this under how to READ a number, not as a fifth way a score lies.** The
four in CLAUDE.md are cases where the oracle hands you a wrong green or a wrong
number. Here funcdiff's number was true; someone read information into it that
it does not carry. Round 13 had a proposed fifth rejected on exactly this
distinction, and misfiling this one would teach the next runner to distrust the
oracle precisely where it is reliable.

Corollary, measured the same round: **a caller's score never validates a
callee.** `func_8001EE98`'s report carried both a 19/31 and a 29/29; the 29/29
belongs to `func_8001E58C`, an already-matched *caller*. A `jal` encodes only a
symbol address, so a caller's bytes are invariant to whatever the callee's body
does.

### The permuter on libc functions: three data points, three different shapes (round 18)

Round 8's `strcat` produced a zero that was a **dead store**, needing
translation to an idiomatic form that scored identically. It was tempting to
generalise that into "expect a dead store". Two more data points say the
pattern is about the AXIS, not the artefact:

- **`strstr` (30/30)** — permuter zero at iteration 1068 via ordinary hoisting
  (`matchStart = haystack;` lifted above the empty-haystack guard). Went in
  **verbatim**, no translation. Notably this was an axis seven prior hand
  attempts never touched: all seven had varied `cursor`'s placement.
- **`strcpy` (17/17)** — the permuter **never reached zero** in 52148
  iterations (best 100, base 315). The non-zero diff was not committable, but
  its *axis* — "an extra copy of `dest`, distinct from both the guard and the
  write cursor" — closed the function by hand in two more iterations.

**The claim that survives: a permuter result, zero or not, is a lead about
WHICH AXIS moved.** Whether it needs translation, goes in verbatim, or is
merely directional has to be decided per instance. A non-zero result is still
worth reading for its axis — that is what closed `strcpy`.

### A third escape from the `sltiu` boolean-materialization fold (round 18)

Round 17 established two ways out of GCC 2.6.3's branchless `sltiu`
materialization (a side effect in an arm; return values that are not a bare
0/1 pair). `func_80028C54` (matched 27/27) adds a third, and it is the one that
explains retail's genuine two-branch tail:

**Nest the final decision inside its own guard with a direct early `return`,
rather than flattening it.** A trailing `if (cond) return X; return Y;` pair and
a single accumulator assigned then conditionally overwritten **both fold
identically** — round 17 tried both, six attempts. The un-flattened nested-guard
form does not fold at all. The test going forward: if a trailing boolean return
sits inside an `if` that also guards other retail-required control flow, try the
nested form before concluding the fold is unavoidable.


### A struct-layout claim must be settled CORPUS-WIDE — a shared-base `+2` is conclusive, counting address computations is not (round 23)

`func_80031CF0`'s report modelled `D_8008D7F0`/`D_8008D7F2` as two INDEPENDENT
16-byte-stride arrays, on the discriminator "retail computes each field's
address through its OWN `lui`/`addiu` pair rather than one cached pointer with
`+0`/`+2` displacements". Round 23's alpha found the counter-evidence in a
SIBLING function, and the head verified it against the `.s`:

```
lui   $a3, %hi(D_8008D7F0)
addiu $a3, $a3, %lo(D_8008D7F0)
addiu $t2, $a3, 0x2          <- +2 off the FIRST symbol's base
```

**A compile-time `+2` off another symbol's materialised base is only emittable
if the compiler knows the two are ONE object at a known offset.** Two separate
`extern` arrays have, as far as cc1 is concerned, unrelated addresses; it cannot
fold one into the other. So they are two fields of one struct.

**Why the original discriminator failed, and this is the generalisable part:**
"one address computation with two displacements" versus "two full address
computations" measures whether GCC **chose** to share a base register in *that
one function* — which register pressure and distance between the uses decide —
not whether the symbols ARE one object. In `func_80031CF0` the two stores are
far apart, GCC materialised each base separately, and **both models emit
identical bytes there**, so the wrong one was locally indistinguishable and
locally harmless.

Two consequences:

- **Settle a struct-layout question from the function that SHARES a base, not
  from the one that does not.** A shared base with a small displacement is
  positive evidence; separate materialisations are no evidence either way.
  `grep` the corpus for the symbol pair before committing to a model.
- **The cost lands on a DIFFERENT function.** Left standing, the two-arrays
  model makes `func_80030404` unmatchable by construction — no pair of
  independent arrays can produce that `addiu $t2, $a3, 0x2`. This is the same
  shape as the shared-header prototype hazard: the author sees nothing wrong,
  and the function that breaks is one that never touched the declaration.

### Four narrower confirmations from round 23 (runner alpha, `code_179d8_j`)

- **`__asm__("")` is an ORDER-only barrier and does not block VALUE
  forwarding.** A store to a global immediately followed by a read of it, where
  the stored value is still in a live register, gets fused by GCC 2.6.3 — and a
  bare barrier does not stop that, because nothing is being reordered. Only
  `volatile` on the global does. This is a real scope limit on the barrier and
  it belongs next to the barrier's own entry: the test "does removing it change
  which register holds a value" already says a barrier cannot move data, and
  value-forwarding is a data question.
- **Two early-return paths tail-merge into one shared fail block only via an
  explicit `goto` to a hand-placed label**, not from naive sequential
  `if (...) return X;` statements. Retail's shared-stub shape is authored, not
  emergent. (Bravo reached the same conclusion independently in
  `func_80032148`, where the LAST guard being phrased positively — `goto
  success` — while earlier guards are plain early returns is what produced
  retail's exact stub placement, and took a 9/53-shaped structural mismatch to
  48/53.)
- **A `(u8)` mask on a loop induction variable used as an ARRAY INDEX — not
  just in the exit test — independently controls whether GCC strength-reduces
  the per-iteration address multiply.** So the mask's placement is a codegen
  lever, and masking only the comparison is a different program from masking
  the index.
- **Compute a value LAZILY, at the point retail's control flow first needs it,
  rather than eagerly at its declaration.** Confirmed twice in one unit
  (`func_8003069C`'s `key`, `func_80030404`'s `recIdx`/`key`). An eager
  computation lengthens the value's live range, which is the same allocno-
  ranking mechanism behind the `s16`-width and fresh-local entries above.

### A wholly-unused STACK parameter, diagnosed from a FIXED OFFSET rather than a register gap (round 23)

The corpus already carries "silent ABI waste" for an unused REGISTER argument
(`func_800302DC`, `func_800319B4`) — diagnosed from the last USED argument's
register number leaving a gap. Round 23's alpha found the **stack** form, and
its diagnostic is different enough that treating them as one lever costs the
attempt.

`func_80031A44` (`code_179d8_j`) reads its two `u16` stack arguments from
`0x3c($sp)` / `0x40($sp)` against a measured `-0x28` frame. The o32 formula for
a four-register-argument function at that frame size puts the first stack
argument at **`frame_size + 0x10` = `0x38($sp)`** — retail's is four bytes
further out. The real source has a **fifth parameter, a plain 32-bit value
between the last used register argument and the first used stack argument, never
referenced in the body.** Adding it (pushing the two `u16`s to argument
positions six and seven) reproduces retail's offsets exactly.

**Three things make this worth its own entry:**

- **The diagnostic is a fixed-offset disagreement, not a gap in a sequence.**
  Compute `frame_size + 0x10` and compare it against the FIRST stack argument's
  actual `$sp` offset. A register-argument gap is invisible here because all
  four registers are used.
- **The symptom is subtle and reads like something else entirely.** Before the
  fix the function measured 49/88 with *five single-bit-different words*
  scattered through the body — every embedded branch-target address off by one
  nibble, because the two loads were being generated four bytes from retail's
  positions. **A handful of low-nibble-only diffs on branch targets, rather
  than missing or extra instructions, is what an argument-offset mismatch looks
  like.** It is easy to misread as instruction selection.
- **The dead-LOCAL trick does not substitute for it.** A scalar `s32 dead;` and
  a one-element `s32 dead[1];` under `if (0) { ... }` — this project's
  established idiom for forcing a frame onto an otherwise-frameless leaf — had
  **zero effect** on the stack-argument offsets. The function already has a
  real frame from five callee-saved registers plus `$ra`, and the dead-local
  trick only sizes a frame that would otherwise be zero. **The fix was an extra
  ARGUMENT, not an extra local.** So the entry below (an oversized outgoing-arg
  area from a dead CALL) and this one are siblings, not the same lever: that one
  is about the area a function reserves for calls it MAKES, this one about the
  offsets at which it READS what it was passed.

### An oversized outgoing-arg frame is EVIDENCE OF DEAD CODE in the original source (round 19)

**GCC 2.6.3 sizes the outgoing-argument area from every call expression's
argument count during RTL expansion — BEFORE dead-code elimination removes an
unreachable branch.** So a call inside `if (0) { ... }` emits zero instructions
and still enlarges the frame.

Found by echo closing `func_8001EE98` (**31/31**) after the head reframed a
stalled measurement. Retail reserved `0x18` (24 bytes = six words) where the
o32 minimum is `0x10`, with exactly one surviving `jal` to a callee that uses
only `a0`/`a1`/`a2`, and **nothing ever written or read in those 24 bytes**. The
reproducing source is an unprototyped `func_80015618()` called with three live
arguments in a loop, plus a **dead six-argument call** to it inside `if (0)`.

**This makes an unexplained frame size a readable signal rather than a dead
end.** The reasoning chain, in the order to apply it:

1. Compute the outgoing area = lowest saved-register offset (`sw $sN/$ra`).
   Anything above `0x10` is non-minimal.
2. Grep every `$sp` access. If nothing touches the region, it is outgoing-arg
   reservation, not locals.
3. `outgoing / 4` is the argument count of the **widest call expression the
   compiler saw** — not necessarily one that survives in the disassembly.
4. If no visible `jal` needs that many arguments, the missing width belonged to
   a call that was compiled away.

**Do not over-fit to `if (0)`.** That is the shape that reproduced here; any
construct the optimiser folds away after RTL expansion has the same effect. What
is established is the *ordering* (frame sizing precedes DCE), not one spelling
of dead code.

**A caution the same round supplies:** echo checked `func_80065AE0`, which has
the same numeric signature, and reports its frame is already correctly sized
with a parameter-copy *timing* residue instead. So the signature is a
**screening** signal, not a verdict — the same status as the register-shaped
census. Confirm per function.

#### The live census (2026-09-05, 222 queued)

Functions whose outgoing-arg area exceeds `0x10` with **nothing ever stored or
loaded in it**. Re-derive rather than trusting this table — it goes stale as
functions match:

| `jal` | outgoing | frame | unit / function | blocker screen |
| --- | --- | --- | --- | --- |
| **0** | 0x18 | 0x30 | `code_55dd4/func_80065A5C` | CLEAN |
| **0** | 0x18 | 0x30 | `code_55dd4/func_800662BC` | CLEAN |
| 1 | 0x18 | 0x20 | `code_4cd08/func_8005C8AC` | gp_rel, addiu_at |
| 1 | 0x18 | 0x30 | `code_55dd4/func_80065AE0` | CLEAN (echo: not this class) |
| 1 | 0x18 | 0x38 | `class_3bb8c/func_8004BB3C` | CLEAN |
| 2 | 0x14 | 0x38 | `code_4cd08/func_8005C508` | addiu_at |
| 3 | 0x20 | 0x30 | `DreamSys/func_8005A82C` | CLEAN |
| 4 | 0x20 | 0x30 | `DreamSys/func_8005A9CC` | addiu_at |
| 4 | 0x30 | 0x38 | `code_179d8_e/func_8002C6FC` | gp_rel |
| 4 | 0x820 | 0x838 | `code_179d8_h/func_80028A84` | CLEAN |
| 7 | 0x30 | 0x40 | `class_3bb8c_e/func_8004EA38` | CLEAN |
| 9 | 0x18 | 0x38 | `class_3bb8c_j/func_80051AC8` | CLEAN |
| 10 | 0x30 | 0x40 | `code_179d8_e/func_8002C4E0` | gp_rel |
| 12 | 0x30 | 0x58 | `class_3bb8c_f/func_8004EF6C` | CLEAN |

**The two `0`-`jal` rows are the strongest leads in the table and should be
worked first: a function that makes no calls at all cannot justify ANY outgoing
area from live code**, so the entire `0x18` is unexplained. Both are
blocker-clean and both are small.

`func_80028A84`'s `0x820` is a different animal — that is a ~2KB stack buffer,
almost certainly a real local array the screen mis-classifies because it is
addressed through a register rather than a literal `$sp` offset. Listed for
completeness, not as a candidate.

Regenerate with the script in `docs/match-reports/func_8001EE98.md`.

#### ROUND 20: the signature is a screen for FRAME SIZE ONLY, and the census's two best rows are now falsified

**The lever and a callee-save STORE-ORDER residue are unrelated GCC decisions,
even when a function's numeric signature matches this census exactly.** Runner
alpha established this in isolation on the two "0-`jal`" rows above —
`func_80065A5C` and `func_800662BC`, the strongest-looking candidates in the
table — and on `func_80066340`.

The method is the part worth copying: rather than spending real builds, alpha
compiled two variants through the pinned pipeline and diffed the `cc1` output.
Variant (a) supplied the extra frame bytes with a genuinely unused local;
variant (b) with a dead six-argument call. **The assembly is byte-identical.**
The only difference is `cc1`'s own `.frame` comment (`vars=8, args=16` vs
`vars=0, args=24`) and internal label numbers that do not survive to machine
code — including, unchanged in both, the exact prologue store permutation those
functions' whole stall history is about (`s2,s3,ra,s1,s0` against retail's
`s2,s3,s0,ra,s1`).

So the reasoning chain above is sound and its conclusion is narrower than it
reads: **the outgoing area tells you the width of the widest call expression
the compiler saw, and nothing whatsoever about the ORDER or IDENTITY of the
registers around it.** Where a body's frame is already the right SIZE, this
lever has nothing left to fix — which is the case for every function alpha
tested, since their compiled lengths were already exact.

Round 19 recorded this as a one-function caution (`func_80065AE0`, "frame
already correctly sized"). It is now a general limit with a reproducer, and it
means **the remaining rows of the census are weaker candidates than the table
makes them look.** Confirm the frame is actually the wrong SIZE before
spending attempts; if the length is already exact, this is not your residue.

### An inherited report's PROSE can be wrong while its NUMBER is right — three independent instances in one round (round 20)

`funcdiff.py` scores bytes. It has no opinion whatever about a report's
*narrative* — what the residue is made of, which side does what, or which
earlier lever is supposed to have fixed what. Round 20 had **four runners, in
four unrelated units, each independently find a false mechanism claim in a
report they inherited**, in a single round:

| runner | report | the false claim |
| --- | --- | --- |
| alpha | `func_80065AE0` | a padding local was said to also fix the callee-save store order "for free"; it does not — the same unfixed permutation is still there, compounded with an `arg`-copy deferral |
| bravo | `func_8002B3F4` family | a prose description of "which side does what" disagreed with a fresh `objdump` read, and the mismatch was **hiding a real fix** |
| charlie | `func_8004C470` | retail and built were **swapped** in the operand-order description |
| echo | `func_80061778` | described as a flat tail merge; the retail bytes are a **nested two-level cross-jump** |

Four in one round, found by four agents who did not talk to each other, is
not bad luck — it is the base rate for prose that nobody re-measures. Compare
`docs/PARALLEL-RUNS.md`'s standing lesson that a claim about MACHINE STATE
decays differently from a claim about the BINARY: a wrong score gets corrected
the next time anyone measures, because measuring is the job. **A wrong
narrative is nobody's job to re-measure, so it survives, and the next runner
builds on it.**

Two practical rules, both cheap:

- **Re-derive the residue from `asm-differ` / `objdump` before acting on any
  report's description of it**, exactly as you already re-derive a preserved
  body's drift claim. Charlie's swapped retail/built would have sent a runner
  reshaping in precisely the wrong direction.
- **When you correct one, correct it in the report** rather than only in your
  summary. All three of these are now fixed in place.

### A lever's NEGATIVE is scoped to the state it was tested under (round 20)

Every stall report accumulates "tried X, inert" lines, and they read as
permanent. They are not. **Re-check cheap levers after any structural fix that
changes the function's shape** — register pressure and scheduling state are
inputs to whether a lever can bite.

Measured by bravo on `func_8002B4D4`: round 19 tested the dispatch-code guard
polarity and found it inert. Round 20 applied an unrelated structural fix (the
unfolded-address local-pointer idiom for `D_8006D8F8`'s store), **re-tested the
same guard polarity, and it worked** — matching retail's delay-slot-sharing
trick byte-for-byte. That function went 34 → 60/91 with its compiled length
becoming exact.

The corollary for how to write a report: an inert-lever line is worth much more
with the surrounding state named ("inert at 34/91, before the `D_8006D8F8`
fix") than as a bare "tried, no effect".

### The permuter can produce a SEMANTICALLY WRONG candidate that scores 176/177 (round 20)

**ROUND 24: two more instances, found INDEPENDENTLY by two runners in one
round, and both were caught by HAND-CHECKING rather than by any tool.** The
class below is not rare and is not specific to call hoisting.

| runner | function | candidate | what was actually wrong |
| --- | --- | --- | --- |
| delta | `func_8005CBC8` | permuter score 330, best of 34,180 iterations | reordered one statement; translated to real C it **regressed to 98/100** |
| delta | `func_8005CBC8` | permuter score 202, best of 43,084 iterations | a `volatile` dead-store trick; compiled and objdumped for real it **grew the stack frame** (`-0x20` vs retail's `-0x18`) |
| echo | `func_800662BC` | permuter lead 625 -> 280, best of 41,561 iterations | **changed the loop's trip count** -- a real semantic bug, not a scheduling artifact |

Echo's is the same shape as round 20's original: a control-flow change whose
static word-count cost is near zero. **The generalisation both runners reached
separately: a permuter score improvement is not evidence of a usable candidate
until the candidate's control flow is traced by hand.** A candidate can compile
with zero errors, score better, and silently change how many times a loop body
executes.

Delta's second instance adds an axis the round-20 entry does not cover: the
candidate was not wrong about semantics at all, it was wrong about the
**frame**. So the check is not only "does the control flow still match" but
"does the prologue still match" -- compile the candidate and objdump it, do not
score it and stop.

And the practical rule both rounds now support: **translate every candidate to
idiomatic C and re-measure with `funcdiff.py` before recording anything.** Two
of the three above scored better and measured worse.


The existing caution — *a permuter zero is a LEAD, not an answer* — implicitly
puts the danger at zero. **The danger is not at zero.** Runner delta, working
`func_8003F848`, hit a candidate that scored **176 of 177 words** against the
real oracle and was outright incorrect C.

The mechanism is what makes this general rather than an anecdote. The candidate
**hoisted a call out of a loop**, so `func_80012AF8` executes once instead of
once per node. GCC does not unroll that loop, so the call's instructions appear
exactly once in the binary either way: moving it across the loop's back-edge
changes **only the backward branch's target immediate**, which is a ONE-WORD
effect. A call-hoisting bug and a genuine one-word near-miss are therefore
indistinguishable by static word count — to the permuter's own scorer *and* to
`funcdiff.py`.

**So a near-perfect score is not evidence of near-correct semantics, and the
closer the score the more this matters.** Before believing any candidate, check
its CONTROL FLOW against the original disassembly's own branch targets — the
project's existing "branch targets disagree = outranks everything" rule is the
right instrument and it is not implied by the score. Delta verified and
rejected this one on semantics regardless of score, which is the behaviour to
copy.

### Commutative-operand-order canonicalization: 6 instances, 3 units — SETTLED as a project-wide class (round 20)

**cc1 canonicalizes the register operand order of a commutative op
independently of the order written in C.** Confirmed independently by two
runners who never communicated, in unrelated units:

- **echo, `code_179d8_c` (3 instances)** — e.g. retail `addu v1,v1,v0` against
  built `addu v0,v0,v1`; survived five reshapes plus a 64631-iteration bounded
  permuter search with the floor stuck.
- **charlie, `class_3bb8c` (2 instances, `addu` and `or`)** — retail
  `addu v0,s4,v1` against built `addu v0,v1,s4`, and the decisive datum:
  **both C operand orders (`tol + r->unk18` and `r->unk18 + tol`) produced the
  SAME wrong order.** That is what rules out source control and makes it an
  RTL canonicalization.

**Screen for the shape:** same opcode, same immediate/operands, only register
POSITIONS swapped. File it under this class and do not spend attempts
reordering commutative operands hoping to match.

**SCOPE BOUNDARY, round 24 — the class holds for SYMMETRIC addends and NOT
for a re-associable base pointer, and one round produced both cases side by
side.** `func_80034D90` (`code_179d8_k`) was filed under this class with an
accurate screen: retail `addu v0,s0,v0` against built `addu v0,v0,s0`, twice.
It closed at **51/51** by REGROUPING, not reordering:

```c
((u8 *)rec)[rec->unk12 + 0x2C] = ...    /* rec + (off + 0x2C)  -> 49/51 */
*((u8 *)rec + rec->unk12 + 0x2C) = ...  /* (rec + off) + 0x2C  -> 51/51 */
```

That is the already-documented `&arr[i + j]` versus `arr + i + j` lever
reaching the very `addu` this class calls unreachable. The two only conflict
if "operand order" and "grouping" are conflated:

- **Symmetric addends: the class holds.** Round 20's decisive datum stands —
  both C operand ORDERS produced the same wrong output. Nothing in source
  reaches it.
- **One addend is a base pointer and the expression can be re-associated:
  the class does NOT hold.** Grouping decides which value becomes `rs`, and
  grouping is not order. This is why the failing reshape on `func_80034D90`
  (`u8 *ptr = rec->unk12 + (u8*)rec;`) regressed — it changed the ORDER,
  the one axis the class correctly rules out.

**The boundary then predicted the sibling correctly, which is why it is a
boundary and not an anecdote.** `func_80035E80`, same unit, same round, same
runner, also filed under this class: its commutative word is
`addu a1,v0,v1` — two COMPUTED values, no base pointer, nothing to
re-associate — and alpha had tested both C orderings. It stays a stall (and
carries three register-identity words besides). Same class, same screen, two
opposite dispositions, separated by whether an addend is a base.

**Practical tell, cheaper than any of the above:** when a function disagrees
with its own ALREADY-MATCHED siblings' idiom, try the siblings' idiom before
accepting a class verdict. Three of `code_179d8_k`'s matched functions write
`(u8 *)rec + rec->unk12` base-first and match; the stall used the subscript
form.

**And a second, independent source-reachable axis was found the same round**
(charlie, `func_8002DF7C`, `code_179d8_l`): **a local's declared WIDTH can
flip which operand becomes `rs`.** Narrowing a small clamp local from
`s32`/`u32` to `u8` -- the narrowest width the value can hold -- flipped a
commutative `addu` to retail's order and closed that function at 47/47. So
before filing under this class, there are now TWO things to try that are not
operand reordering: re-associate the grouping, and narrow an addend's type.

**And the scope boundary, which is the more useful half.** echo then tested a
third unrelated unit (`class_3ac78`, three functions) and found **no instance**
— those residues are a load-delay-slot scheduling swap, a register-role
asymmetry between two structurally identical byte blocks, and a backward
delay-slot hoist across a call. So the class is real and cross-unit but **not
universal**, and "is this that class?" remains a question to answer per
function rather than assume. Both halves are recorded deliberately: "this class
is everywhere" and "this class is real in some units and absent in others" lead
to very different next rounds.

*(Note on provenance: echo's own report concluded the class was confined to its
origin unit and "not yet promotable". That was correct on echo's information
and wrong on the round's — echo could not see charlie's parallel work. The head
compared the two residue descriptions directly before merging them into one
class. This is the adjudication `docs/PARALLEL-RUNS.md` assigns to the head,
and it is why runners are told to describe a residue rather than only name it.)*

**SETTLED later the same round, by the decisive test rather than by tallying.**
echo took the both-orders test into a THIRD unrelated unit (`DreamSys`) and
applied it to `CalcDreamColor`: reversing the C-level operand order of the
final addition produced a **byte-identical wrong result**. That is a sixth
instance, in a third unit, re-derived from the raw disassembly before
cross-referencing anything. The class is project-wide and no longer needs
further confirmation.

**What earns a screen's promotion is the DISCRIMINATING test, not the count.**
Five instances across two units left the scope genuinely open; one application
of "reverse the operands in C and see whether the output changes" closed it.
When you find a candidate class, look for the test that could falsify it and
run that, rather than accumulating agreeing observations — and note that this
one is cheap enough to run per function.

**A neighbouring family, deliberately kept separate: delay-slot-fill choice.**
echo screened five functions across three units against the commutative class
and got two positives and three negatives — and the negatives were not
formless. `func_8005A82C`, `func_80059BE0` and `class_3ac78`'s `func_8004B100`
all diverge on **which instruction fills a delay slot** (retail placing real
work or a `nop` where the build hoists something else), with no commutative op
involved in either diverging word. That looks like a second class, currently at
three instances across two units. It is recorded here as an OPEN observation,
not a promoted class: nobody has yet found its discriminating test, and the
lesson directly above says that is what would settle it.

### A `for`-loop keeps a status value register-resident where `goto`/labels folds it away (round 20)

**Spelling a retry loop as a genuine `for` loop rather than `goto`/labels
changes whether GCC 2.6.3 keeps a loop-carried status value in a register.**
With the `goto` form the compiler folds the value away; with the `for` form it
stays register-resident, which is what retail does.

Found by bravo on `func_80028DF0` (10/82 → 45/82) and **transferred to the
sibling `func_80028F38` with zero per-function tuning** (11/79 → 46/79). Both
compiled lengths became exact — `func_80028DF0` had been two words short and
now matches retail's 82 words byte-for-byte. Moving the status value's `-1`
reset inside the loop body closed the remaining length gap.

This closes a question rounds 16 and 19 both left open on these functions, and
the clean transfer is what promotes it from a one-function accident to an
idiom: **the two siblings' reports had carried a shared-root-cause hypothesis
since round 16, and this confirmed it.**

Add it to the reshaping repertoire next to the existing `do { } while (0)` and
nested-guard entries: **when a loop-carried value's residency looks wrong, try
the other loop spelling before concluding the residue is allocation.**

### Two DISTINCT permuter false-lead patterns, both caught in round 20

The permuter earns its place, but round 20 produced two different ways it
misleads, and they need different defences. Both were caught by runners who
verified against the real oracle rather than the permuter's own scorer.

**1. An isolated-scorer improvement that REGRESSES the real build.** bravo, on
`func_80029074`: a candidate scored 835 against a base of 1170 — a large
apparent gain — and verified *worse* against the real build, at 2/85 words with
the length grown from 85 to 89. bravo caught two more of these in the same
pass. The permuter scores in isolation and cannot see what the linked image
does; a score improvement is a hypothesis, and the real oracle is the only
test.

**2. A candidate that is SEMANTICALLY WRONG yet scores almost perfectly.**
delta, on `func_8003F848`: 176/177 words, and incorrect C — it hoisted a call
out of a loop. See that entry above for the mechanism; the short version is
that moving code across an un-unrolled loop's back-edge changes only the branch
target immediate, a one-word effect.

**The defences are different and you need both.** Pattern 1 is caught by
re-verifying every candidate against `build-and-verify.sh` + `funcdiff.py`.
Pattern 2 survives that check — it *is* a near-perfect score — and is caught
only by reading the candidate's CONTROL FLOW against the disassembly's own
branch targets. **A high real-oracle score is not evidence of correct
semantics.**

### A residue next to a just-fixed defect may be the same root cause wearing a different face (round 20)

bravo's load-bearing proposed learning, and it is the practical form of the
"re-check cheap levers after a structural fix" entry above. After closing one
defect, **test whether the adjacent residue moves under the same structural
lever before assuming it needs its own fix.** On `func_80028F38` the sibling's
entire fix transferred with no tuning at all.

The counterweight, so this does not become over-applied: echo established the
opposite result on the Entity tail-merge pair, where two residues under one
label were genuinely different shapes and neither lever transferred. **Test the
transfer; do not assume it in either direction.**

### "N words short" and "N/M words match" are DIFFERENT measurements that read identically — the head got this wrong in round 20

`docs/PARALLEL-RUNS.md` Gate 1b already says to rank from title/verdict lines
and to rebuild any figure before putting it in an assignment. Round 20's head
did the first and **not the second**, and it is worth recording because the
sub-case is new.

**ROUND 23 TURNS THIS INTO A REPORT-FORMAT REQUIREMENT, because it recurred
and because fixing it recovered a near-miss that was being buried.** Alpha
filed `func_80031A44` with the sentence *"the function is 51/88, matches
retail's exact instruction count is off by exactly one word, and the residue is
precisely one missing nop"* — internally contradictory, and 51/88 means 37
words differ. The head flagged it rather than fixing it (the runner held the
unit and the build); alpha re-measured and the corrected title reads:

> length 1 word SHORT at 87/88; 51/88 raw word-match, but that 37-word gap is
> almost entirely the SHIFT from the one missing word, not independent residue

**That is a one-word near-miss and a prime permuter target, and "51/88" hides
it completely.** The two possibilities — "mostly shift" versus "independent
register residue" — send the next round to opposite places, which is why the
ambiguity is not cosmetic.

So a stall report's TITLE must carry three things, not one:

1. **LENGTH**: exact, or N words short/long.
2. **RAW WORD-MATCH**: M/N.
3. **WHERE THE FIRST REAL DIFF IS**, read off `tools/asm-differ/diff.py`
   (which realigns) — never inferred. If it sits at or just past the length
   gap, the word-match deficit is mostly ripple; if it is early, there is
   independent residue and the length gap is only half the story.

Alpha's four corrected titles are the model to copy, and they show the range
the format distinguishes: `func_80031890` (length EXACT 73/73, 51/73, first
diff at word 37 — genuine scattered residue) versus `func_80031A44` (1 short,
51/88, gap is ripple) versus `func_80030404` (5 short, 21/96, first diff at
word 0 — residue independent of the length gap). Three very different states
that all render as "≈50%" under a single figure.

`func_8004F8A4`'s title read:

> best 0x130 (1 word / 4 bytes short), zero logic/CFG miss

That is a **LENGTH** statement: the compiled body was 0x130 bytes against
retail's 0x134, i.e. one word short in SIZE. It says nothing whatever about
how many words MATCH — which was **33 of 77**. The head read "1 word" as a
match residue and staffed it as *"the closest unmatched function in the whole
queue apart from one 258-word outlier."* It was not; runner charlie found the
real position on arrival.

**This is a different failure from the three already recorded.** Rounds 18 and
19 pulled figures out of report BODIES, where variants that were tried and
discarded contaminate the text. This one came from the TITLE — the place the
guidance calls safe — and the title was not wrong. Two honest measurements of
the same body simply read the same way in prose:

| phrase | what it measures | good news or bad |
| --- | --- | --- |
| "1 word short" | compiled LENGTH vs retail | says nothing about correctness |
| "76/77 words match" | per-word agreement | says nothing about length |

A body can be the exact right length and match almost nothing (delta's
`func_8003D73C`: 144/145 words compiled, 50/145 raw), or be one word short
while matching a third of its words (this case).

**So the Gate 1b rule needs its second half enforced, not just its first:**
ranking from titles is necessary and not sufficient. Before a figure goes into
an assignment, say out loud which of the two quantities it is — and if the
title does not make that unambiguous, open the report or re-run
`funcdiff.py`. When you write a title, give both: *"best 36/77 words, compiled
length 1 word short"* leaves nothing to infer. That is how
`func_8004F8A4`'s title now reads.

### One named C variable gets ONE storage location — retail's transient-rematerialization shape has no C spelling (round 20)

**GCC 2.6.3's allocator is not SSA: a named C variable has one fixed storage
location for its whole scope.** So the moment any reachable path requires that
variable to survive a call, the WHOLE variable is promoted to a callee-saved
register — no matter how many redundant assignment sites you write.

Retail sometimes does something a C author cannot ask for: several
**independent, transient, scratch-register recomputations** of the same value,
converging on one physical merge point, with no single variable ever living
across the calls.

Established by echo on `func_80064E34` (98 words) with six structural variants
deliberately **bracketing retail's length from both directions** — four
independent call sites (110 words, 12 over), two call sites (104, 6 over), a
combined `||` guard (drift), a `goto`-based shared label (92, 6 under), and a
named variable reassigned at each of retail's four rematerialization sites
(97/99 — one word off in *either* direction, the closest reached). Bracketing
like this is the right way to argue a shape is unreachable: it shows the target
sits between two adjacent expressible forms rather than merely that several
guesses missed.

**Three things this is NOT, and they matter:**

- **Not a toolchain lead.** The pinned compiler is behaving as designed. There
  is nothing to escalate and nothing to reproduce in isolation; a
  register-allocation policy is not a bug.
- **Not grounds for raw asm.** CLAUDE.md's "no C form EXISTS" exception is
  scoped to INSTRUCTIONS with no C spelling — GTE `rtpt`/`nclip`, COP2
  `swc2`/`lwc2`. "The allocator will not produce this arrangement" is
  emphatically not that, and round 13 already had one whole-function `__asm__`
  reworked into six lines of ordinary C for making a weaker version of this
  argument.
- **Not a reason to stop measuring.** It is a characterised STALL, and the
  characterisation is what makes it cheap to recognise next time.

**The recognisable signature:** retail recomputes a value at several sites and
never keeps it in a callee-saved register, while every C form you write either
hoists it into one (too few words) or duplicates surrounding setup (too many).
If bracketing puts you one word out on both sides, you are probably here.

### The `__asm__("")` barrier in round 20: two wins, four regressions, and the discriminator (round 20)

The bare scheduling barrier is the one lever CLAUDE.md permits for an
order-only residue, and round 20 exercised it hard enough to say something
useful about when it bites. **Do not read this entry as "barriers are
harmful" — it closed real ground twice this round.**

| runner | function | outcome |
| --- | --- | --- |
| bravo | `func_8002AA6C` | **WIN** — a missing barrier on a reused tail block was part of getting 119 → 202/223 |
| alpha | `func_8001DA28` | **WIN** — first-statement barrier fixed a deferred-self-materialization residue |
| alpha | `func_80065AE0`, `func_8001E4A4` | inert at two positions |
| delta | `func_8003FCFC` | regressed badly |
| delta | `func_8003DAD4` | regressed sharply, 114/118 → 23/118 |

**The discriminator is what the residue actually IS, not how it looks.** The
two wins were both cases where a genuine ORDERING decision was in play — a
statement whose placement relative to a block boundary was wrong. Every
regression was a case where the residue was a register CHOICE or a
cross-basic-block CFG decision wearing an order-shaped appearance; there the
barrier does not merely fail, it perturbs delay-slot filling and can add real
`nop`s or shift allocation.

This is the round-14 `func_8001E6F8` lesson at a larger sample: **a barrier is
not guaranteed to be a pure no-op even when CLAUDE.md's register-identity test
says it is allowed** — it can grow the function, which disqualifies it for a
different reason than the banned register pinning does. Delta's phrasing is the
right one to carry: for a register-choice or cross-block residue the barrier is
**presumptively harmful rather than a cheap first lever**. For a genuine
placement/ordering residue it remains the correct first thing to try.

Test a barrier's effect on WORD COUNT, not just on position, before trusting
what it did.

**Round 21 adds a PRECONDITION that sits upstream of the discriminator, and
it is cheaper to check than the discriminator is.** A barrier reorders
instructions GCC has already scheduled from real C statements. It cannot
CREATE one. So if the residue is an instruction retail has and your build
does not — an ABSENT instruction rather than a misplaced one — the barrier
is not a candidate at all, whatever the residue looks like, and no
placement of it can help.

Read the residue's direction off `funcdiff` before reaching for the lever:

| `funcdiff` row shape | residue | barrier a candidate? |
| --- | --- | --- |
| `retail=<insn> built=<different insn>` | substitution — order or register | maybe: apply the discriminator |
| `retail=<insn> built=00000000` | **absent** — retail has a word you do not | **no** |

Measured on `func_8001A064` (`code_8220_c`), whose residue is one of each:
word 12 is a register choice (`$a2` vs `$a1`) and word 21 is absent
(`retail=34002226 built=00000000`, i.e. retail's `addiu $v0,$s1,0x34`
against our `nop`). Four barrier placements, whole-image oracle each time:
three inert at 104/112, one regressing to 18/112. **Zero moved word 21**,
which is what the precondition predicts and what makes it worth stating
separately — the discriminator alone would have sent you to test it.

The reason this earns its own paragraph rather than a footnote: round 21's
charlie reached the same conclusion for this family by ANALOGY to round
20's `func_8003DAD4` regression, and was right. But the project's own rule
is that a lever's scope is measured, not reasoned — round 7 found two
superficially symmetric vtable slots needing OPPOSITE answers — so the
head ran it. Same answer, now evidence, and the precondition above is the
part that generalises beyond the family. **An absent-instruction residue
needs a source-shape change that makes the compiler COMPUTE the missing
value; there is no scheduling lever for it.**

**Round 22 adds a THIRD condition, and it is positional rather than
diagnostic — where in the statement stream the barrier sits.** Two runners hit
opposite sides of it in the same round, which is what makes it separable from
the discriminator above:

| runner | placement | outcome |
| --- | --- | --- |
| delta | between two straight-line stores (`func_80056BBC`) | **WIN** — closed a scheduling residue |
| head | between three straight-line stores (`func_80031BA4`) | **WIN** — 47/61 → 61/61 |
| charlie | immediately before a cast-and-call statement (`DreamSys__InstanceEffectsOnJournal`) | inert, optimized away, zero effect |
| charlie | at a branch/label boundary (`func_8005A9CC`) | regressed — suppressed unrelated scheduling, 136KB of drift |
| alpha | first statement, ahead of a guard (`func_80031BA4`) | **regressed** — moved two stack-argument loads and manufactured the residue it then filed as a stall |

**A barrier between straight-line statements in one basic block does what it
says. A barrier at or across a block boundary does not** — it is either
eliminated outright (nothing to reorder across a boundary the compiler already
treats as a barrier) or it perturbs the delay-slot filling that spans the
boundary. Round 20's discriminator says *what kind of residue* to use it on;
this says *where it can physically act*. Both have to hold.

### Levers do not commute — a residue that MOVES is a signal to subtract (round 22)

Runner alpha filed `func_80031BA4` at 57/61 as a source-unreachable scheduling
stall: "stack-passed argument loads sit outside ordinary scheduling-barrier
reach." The naive body — no barriers, no named intermediates — reproduced
retail's supposedly unreachable ordering **on the first try** through the
pinned pipeline. The only real divergence was a missing empty 8-byte frame,
which moves both stack-argument displacements from `0x10/0x14` to retail's
`0x18/0x1C`: a displacement, not a reorder.

Alpha owned every lever it needed (its own `s32 dead[2];` + `if (0)` frame
idiom, plus two order-only barriers between the three stores) — that
combination gives **61/61**. What broke it was the scaffolding stacked on top:
a third barrier ahead of the guard, and named intermediates for `idx`, `p4`
and `p5`. Those four lines moved the two `lhu`s, and the stall it filed was a
description of damage done by its own earlier fixes.

**The rule: add one lever, measure, and if the residue MOVES rather than
SHRINKS, revert it before trying the next.** A body carrying three barriers
and three named intermediates is no longer testing a hypothesis about the
function; it is testing the interaction of six edits, and the residue it
reports is attributable to none of them. Each lever here was individually
sound and individually documented — the failure was purely additive.

**Corollary, and the cheaper half: a "source-unreachable scheduling residue"
verdict requires an isolation reproducer of the NAIVE body.** CLAUDE.md
already demands this of a toolchain escalation ("never escalate a lead you
have not tried and failed to reproduce in isolation"). Extend it to this
verdict, because it closes a function to future rounds exactly the same way a
blocker classification does. It costs under a second, and here it would have
falsified the hypothesis before it was ever formed.

### A delay-slot residue writing `$a0`-`$a3` is a CALL ARGUMENT until proven otherwise (round 22)

Runner delta filed `func_80056BBC` at 86/87 as the documented "redundant move"
class: an `ori $a1, $zero, 1` filling a branch delay slot, with — it argued —
no reader on either path. That is right for the fallthrough (`$a1` is
overwritten in the very next instruction) and **wrong for the branch-taken
path**, which is the half that decides:

```
    bnez  $v0, .L80056C78
     ori  $a1, $zero, 0x1     <- the residue
.L80056C78:
    lw    $s0, 0x88($s1)
    nop
    lw    $v0, 0x0($s0)
    nop
    lw    $v0, 0x64($v0)
    nop
    jalr  $v0                 <- slot64, and NOTHING wrote $a1 in between
     addu $a0, $s0, $zero
```

The instruction is that call's second argument, hoisted into the delay slot in
the ordinary way. The unit's local vtable view typed the slot
`void (*slot64)(LinkNode *)` — one parameter — so no `li $a1, 1` existed
anywhere in the C for the scheduler to hoist, and the slot came out a `nop`.
Typed `(LinkNode *, s32)` and called `slot64(child, 1)`: **87/87**.

Two rules out of it, and the second is the cheap one:

- **Walk FORWARD from the residue along the branch-TAKEN path to the first
  `jal`/`jalr`. If nothing writes the register in between and it is
  `$a0`-`$a3`, it is an argument.** It is a dead store only if BOTH successors
  redefine it before any call. A single-path liveness check is not a liveness
  check.
- **A function-pointer slot's arity is invisible at the call site** — `jalr` is
  the same bytes either way — so a slot typed with too few parameters produces
  exactly this signature and nothing else. **Cross-check arity against every
  other unit that calls the slot before classifying such a residue**
  (`grep -rn 'slotNN' src/`). Here six already-matched call sites in four
  units all passed a second argument; this unit's one-argument view was the
  outlier, and one grep would have said so. The project's
  multiple-independent-local-views convention is what makes that disagreement
  cheap to find, and is itself the evidence.

### A same-size pointer cast in a FUNCTION-SCOPE local can cost a callee-saved register (round 22)

`DreamSys__InstanceEffectsOnJournal` (charlie): a
`DreamSysEntityObj *obj = (DreamSysEntityObj *)entity;` assigned once at the
top and reused by five cases compiled to a **third** callee-saved register
(`$s2`) where retail needs two — an 8-byte-larger frame that cascaded into
85287 bytes of whole-image drift. Casting `entity` inline at each of the five
use sites instead took the function from wildly wrong to 106/110 in one step.

**The cost is about LIFETIME, not about the existence of a named local** — a
*case-local* temp of the same type was byte-identical. Prefer casting inline at
each use site; promote to a local only when removing it demonstrably changes
nothing.

**The head closed the third form of this axis, negatively:** retyping the
PARAMETER itself to the view type and deleting every cast leaves the residue
unchanged. Three forms are now tested (function-scope local, case-local,
parameter retype) and the cast/typing axis is closed for that function — what
`asm-differ` showed instead is that the `move a0,s1` / `lw v0,0(a0)` ordering
is specific to its **case 4 alone**, the only site there passing a second
argument. Recorded because a negative result nobody wrote down is one the next
round pays for again.

### Four narrower confirmations from round 22

Each backed by a byte-exact match, one line each:

- **A named local can change a load's SIGNEDNESS, not just its register.**
  `s16 speed = e->unk44;` emitted `lhu` plus a re-sign-extend where retail has
  a plain `lh`; reading the field directly at each use site fixed it
  (`func_80033AB0`, bravo). Same family as the `sltiu`-vs-`slti` symptom
  charlie hit — a wrong type shows up as an instruction choice, not only as a
  register.
- **Comparison operand ORDER, not just polarity, decides which side's load is
  emitted first.** `if (a < b)` and `if (b > a)` are semantically identical and
  compile differently (bravo).
- **A masked-and-shifted extraction folds `sra` → `srl` when stored through a
  named intermediate, and stays `sra` written inline at the use site** —
  verified in isolation through the pinned pipeline before use
  (`func_800305F4`, alpha).
- **An empty stack frame SMALLER than 0x10 is evidence of a dead local-array
  write, not a dead call** — a dead call forces 0x10 minimum for the outgoing
  argument area, a dead local array does not (`func_80031CF0`, alpha; used by
  the head to close `func_80031BA4`).

### `make extract` is match-status-aware — it deletes the `.s` for a function that is live C (round 20)

Splat reads `src/*.c` to decide which functions still need a generated
`asm/nonmatchings/<unit>/<func>.s`. So while you have a function spliced in as
real C mid-investigation, `make extract` will **not regenerate — and will
remove — that function's stub**, and the build then fails on a missing `.s` the
moment you restore its `INCLUDE_ASM`.

This is expected behaviour, not a bug, and it is the same mechanism behind
`progress.py`'s "stale asm/nonmatchings/**/*.s with no live INCLUDE_ASM"
warning after a match. Delta lost time to it looking like a tooling failure.
The fix is one command — restore the `INCLUDE_ASM` first, then `make extract`
— and the rule is: **do not run `make extract` while a function is spliced in
as live C.**

### "Tail merge" is an UMBRELLA, not a class — two shapes, and they do not transfer (round 20)

Several reports label a residue "tail-merge". They are not all the same thing,
and treating them as one costs attempts. echo was given two Entity-family
functions filed under that label and asked what test would falsify "these are
the same class". The answer is that they are **two distinct shapes under one
umbrella mechanism (GCC's cross-jump pass)**:

| function | shape | the question retail answers differently |
| --- | --- | --- |
| `func_8005DBF0` | single-level, whole-statement | merge **COUNT** — three identical predecessors, retail merges only two |
| `func_80061778` | multi-level, partial-suffix | merge **DEPTH** — retail builds a two-level hierarchy of nested shared tails (a 6-word merge between two arms, with a 3-word suffix carved out and shared with a third, differently-set-up arm) |

**The falsifying test, run:** neither lever transfers. The type-retype that the
`func_8005E160`/`slotC4` precedent suggests has no applicable target in
`func_80061778` (no discarded-return call anywhere in its merge chains), and
the nesting nudge has no equivalent shape in `func_8005DBF0` (nothing
hierarchical about a flat three-way merge onto one instruction). Both were
tried and both regressed.

**So: check a third instance against WHICH of these two shapes it matches
before inheriting either report's attempt history.** Count-vs-depth is the
discriminator, and it is readable straight off the disassembly.

Note also that `func_80061778`'s own report described its merge as flat; the
retail bytes are the nested two-level structure above. That is the fourth
inherited-prose correction of the round — see the entry on that above.

### A fresh local that only carries one branch's result to one later use is a register-identity risk — reuse a dead PARAMETER instead (round 23)

Two confirmed instances in one unit, both closing a function that had a
register swap and nothing else wrong (runner charlie, `Entity`):

- `func_8005D560` (62/62): a fresh `s32 code` local produced an `$s0`/`$s1`
  swap against retail. Reusing the function's own **semantically dead `arg2`
  parameter** as the carrier closed it.
- `func_8005D714` (58/58): same shape — mutating the `arg3` parameter in place
  instead of declaring a fresh `dist` local, which additionally fixed a
  speculative-hoist branch-shape mismatch.

**The mechanism is the one already documented for `s16` widths and for
`for`-loop status values: a named C variable gets one storage location, and a
NEW name creates a NEW allocno competing for a callee-saved register.** A
parameter that is already live on entry and never read again is free storage
the allocator has already committed to — reusing it adds no allocno, so it
cannot perturb the ranking.

The discriminator, so this does not become "delete locals at random": it
applies when the local exists **only** to carry one branch's result to a single
later use, and there is a parameter in scope that is provably dead. Those two
conditions together are what make the rewrite semantics-preserving and
allocation-neutral. Contrast the existing entries on the opposite direction
(round 20's "two levers that closed long-standing near-misses by DELETING a
named value", and "one named C variable gets ONE storage location") — this is
the same family, arriving as *substitute* rather than *delete*.

Related, from the same unit and the same round: **branch-polarity residue
recurs at multiple nesting levels INDEPENDENTLY in one function.** Fixing the
outermost `if`/`else` polarity says nothing about an inner one;
`func_8005D560` needed it applied twice, and the head's `func_80049EB4` needed
it at two separate ternary sites. Five instances across three runners and the
head this round. **The tell is that only the branch MNEMONIC and the two
literal immediates differ, with everything else exact** — retail's
`beq`/`bgez` means the source condition was `!=`/`< 0`, because GCC tests the
NEGATION and falls through to the first arm. Never scheduling.

And a triage caution charlie paid one attempt for: **a table read that
resembles a neighbouring function's near-identical table read is not evidence
of the same expression shape.** Check the actual instructions rather than
pattern-matching from a function read minutes earlier (`func_8005D278`).

### Cross-jump shape THREE: a declared RETURN TYPE can block a merge that should happen (round 23)

The umbrella entry above names two shapes, both about what retail does
differently from us (merge COUNT and merge DEPTH). Round 23 adds a third that is
categorically different, because **the defect is in our HEADER, not in the
compiler's decision.**

`func_8003C63C` (`code_2cc8c`, matched) has four indirect calls that retail
cross-jumps into ONE `jalr $v0` site. The first attempt merged them **2 + 2**,
costing exactly 3 words, and the split fell precisely along the declared return
types of the four vtable slots: `slot90`/`slot94` were `void`, `slot10C`/
`slot110` were `s32`. **GCC 2.6.3 will not cross-jump a `void` call against a
value-returning call whose result is discarded**, because the RTL differs
(`(call …)` versus `(set (reg) (call …))`) even though the emitted instructions
are byte-identical. Retyping the two `s32` slots to `void` merged all four and
closed the function.

**The discriminator, and it makes this diagnosable instead of a guessing game:
when N identical-shaped calls should merge into one site and instead merge into
GROUPS, the grouping PARTITIONS THEM BY DECLARED RETURN TYPE.** Count the
groups, line them up against the slot declarations, and the odd one out is the
wrong type. A 2+2 split is a TYPE mismatch; no barrier, reordering or nesting
nudge will touch it.

Two things that make this cheap to act on and easy to justify:

- **A slot whose return type came from an UNATTEMPTED function's disassembly is
  a hypothesis with no evidence behind it.** A discarded return value is
  invisible in the bytes — the same reason the runner prompt says a one-line
  wrapper's byte match tells you nothing about its return type. Both slots here
  were annotated `OBSERVED: func_8003C63C (STALL, not attempted)`, i.e. read off
  the disassembly of a function nobody had ever compiled. **Treat any slot
  annotated that way as an open variable.**
- **Prefer evidence from the slot's OCCUPANT over its call site.** `slot110`'s
  occupant `func_8003DCAC` was already matched as
  `void func_8003DCAC(Obj865C8 *self)` in a sibling unit — positive, independent
  evidence for the retype. `tools/classtable.py` resolves the occupant; if it is
  already C, its signature settles the question.

Retyping a shared slot is still the round-7 hazard, so re-verify the whole image
and every other caller individually (done here; also done for the `slot1B8`
`void` -> `s32` retype in `func_80049EB4`, where the other caller discards the
value — the configuration that cost round 7 four words — and which came back
clean).

### A `switch`'s CASE ORDER is recoverable from the binary — promoted from a one-liner on four instances in one round (round 23)

The idioms list has carried this as a single line since round 11 ("GCC 2.6.3
lays out case bodies in TEXTUAL source order but picks its own comparison
order"). Round 23 hit it **four times in four different units, three of them
independently**, and it was worth 6+ words each time — so here is the full
recipe, plus the inverse case that makes it safe to apply.

**For a JUMP-TABLE (dense) switch: block layout follows SOURCE order.** The jump
table is indexed by case VALUE and does not care about layout, but the arm
blocks are emitted in the order the `case` labels appear in the source. So:

1. Read the arm labels out of the target's `.s` and sort them **by address**.
2. Map each label back to its case value(s) through the jump table's words.
3. Write the `case` clauses in that order — **not ascending numeric order.**
4. **The arm that FALLS THROUGH into the shared tail (no `j` of its own) is the
   LAST case in source order.** That is a free anchor available before you match
   anything.

Measured instances, all closed:

| function | unit | what it cost |
| --- | --- | --- |
| `func_8003C48C` | `code_2cc8c` | 6 words; order was `0x12, 0x13, 0x21, 0x17, 0x19` |
| `func_8004FBE4` | `class_3bb8c_g` | closed the function; ascending order was wrong |
| `func_80032588` | `code_179d8_c` | order `4,1,3,2,5,0`; took a **9/53-shaped structural mismatch to a 48/53 near-miss** |
| `func_80049EB4` | `class_39e08` | order `4, {5,6,7,8,0xA}, {0xC,0xD}` confirmed off the labels |

**The tell that you are looking at this and not at scheduling: the diff shows
whole ARM BODIES swapped — and their jump-table words permuted with them —
rather than instructions changed.**

**THE INVERSE, and you must keep the two straight because they point opposite
ways.** For a SMALL, SPARSE switch that does NOT become a jump table, GCC
lowers it to a balanced decision tree and **normalises the comparison order to
ascending value regardless of source order** (echo measured this in
`func_80050034`; a two-value nested switch had to become explicit `if`/`else if`
to reproduce retail). So source order is recoverable from a dense switch's block
layout and is NOT recoverable from a sparse switch's compare order. Round 11's
one-liner said exactly this ("picks its own comparison order"); it is repeated
here because two runners this round rediscovered half of it each.

**Related, and the reason a sparse switch is worth identifying at all: a
range-split compare in the middle of what looks like a compare chain means
`switch`, not `if`.** A three-way `if / else if / else` on one variable emits
three sequential `beq`s. A three-way `switch` on the same variable compares the
MEDIAN first, then emits an `slti`/`sltiu` range split to choose which half to
test. `func_8003C63C`'s nested three-way dispatch is `beq 0xF` -> `slti …,
0x10` -> `beq 0xB` / `beq 0x11`; that `slti` is the whole signal, and it is what
distinguishes two source constructs whose logic is identical.

**SCOPE, measured — the tell needs at least THREE explicit case values, and
below that `switch` and `if`-chain are the SAME codegen.** The head proposed
this lever to round 23's bravo as a way to explain a `bne`-vs-`beq` divergence
between two logically identical switch arms in `func_80032588`. Bravo tested
both assignments (switch in one arm, if-chain in the other, and the reverse)
and both rebuilt **byte-identical to the if-chain baseline** — same
`beqz`/`li`/`bne`/`li`/`j`/`li` sequence either way, and no change in word
count. Its inner dispatch has **two** explicit values plus a default, which is
never enough for a balanced tree to beat sequential compares, so the two
constructs converge and the `slti` never appears.

So the boundary sits between 2 and 3: **3 explicit values produced a tree
(`func_8003C63C`'s inner switch), 2 values plus a default did not
(`func_80032588`).** Do not reach for this lever to explain a divergence
between two- or one-value dispatches — there is nothing to distinguish, and
`switch` there is a rewrite that changes no bytes. (`func_80032588`'s own
report carries the negative and the reasoning; the head's original framing
omitted the threshold, which is the correction.)

### Two VLAs, an `$fp` frame, and a rounding immediate that carries the array bound (round 23)

`func_8004109C` (`code_2cc8c_f`) was filed round 13 as unattempted on a
7-callee-saved-register census. **The `$fp` is not register pressure — it is a
variable-length array**, and the function has two:

```
addiu $v0, $s2, 0xf     # ((n) + 15) & ~7  -- GCC's VLA size rounding
srl   $v0, $v0, 3
sll   $v0, $v0, 3
subu  $sp, $sp, $v0     # the allocation
addiu $s3, $sp, 0x10    #   -> the array's pointer
lw    $v1, 0x0($sp)     #   dead load: part of the expansion, no source construct
```

Recognise it by: two or more `subu $sp, $sp, <reg>`, `move $fp, $sp` in the
prologue, and `move $sp, $fp` restoring it. Each `subu` is one VLA; identical
`$v0` for several means identical size expressions, CSE'd.

**The rounding immediate is the array BOUND, and it is the only place a `+ 1` is
visible.** GCC emits `addiu <t>, <n>, 0xf` for a VLA of `n` bytes, so the
register holds the element count and the immediate is always 15: `0xf` against a
register holding `width` means `char buf[width + 1]`; `0xe` means
`char buf[width]`. Getting it wrong is a LENGTH mismatch, not anything that
reads like a size bug. Here the `+ 1` is the NUL byte that `strcpy` writes past
the `width` characters `memset` fills.

### A struct RETURNED BY VALUE reads as a call with its arguments shifted (round 23)

`func_80049EB4`'s call site looked like an argument-order anomaly — the object
in `$a1` and a stack address in `$a0`:

```
lw    $a1, 0x38($s0)      # the OBJECT, in a1
lw    $v0, 0x0($a1)
lw    $v0, 0x1BC($v0)
jalr  $v0
 addiu $a0, $sp, 0x10     # a LOCAL's address, in a0
```

**GCC 2.6.3 returns every struct through a hidden pointer passed as the
invisible FIRST argument**, so each real argument shifts one register right. The
source is an ordinary assignment, `dest = obj->methods->slotNN(obj);`.

Two things to establish before writing it that way:

- **Recognise the pair**: an `addiu $aN, $sp, <local offset>` in the `jalr`'s
  delay slot together with the object one register later.
- **Size the destination from the FRAME.** The gap between the four-word
  outgoing-argument spill area and the first saved register is the local area,
  and it gives the returned struct's size directly. Here frame `0x28` with
  `$s0`/`$s1`/`$ra` at `0x18`/`0x1C`/`0x20` leaves `0x10`-`0x17` — 8 bytes. A
  4-byte local would have let `$s0` sit at `0x14`.

Declaring the slot `void (*)(Dest *, Obj *)` instead matches the bytes at this
call site and is WRONG about the type, which then propagates to every other
caller.

### A local's DECLARED WIDTH is a codegen decision, and `s16` is the expensive default (round 23)

Two of `GetStageChunkFromMood`'s five excess words, and four of its five in
total, were `s16` locals.

An `s16`/`u16` local that is compared or used as an index gets
**re-sign-extended at each use, inside loops included** — spurious
`sll <r>, <r>, 0x10` / `sra <r>, <r>, 0x10` PAIRS the target does not have.
Declaring the LOCAL `s32` folds the extension into the `lh` at the point of
load. **The struct FIELD stays `s16`** — it really is 2 bytes in the data; only
the local copy widens. (Widening the field would be the non-local struct edit
that breaks an already-matched function elsewhere with a clean compile.)

**The trap that delayed the diagnosis: two locals of the SAME declared type can
come out differently.** `rows` and `columns` were both `s16`, both read from
adjacent fields of one struct; `rows` compiled to a single clean `lh` and
`columns` to `lhu` plus a sign-extend pair at each use. **One of them looking
right does not clear the type.** Widen both.

The tell is specifically the `sll 0x10` / `sra 0x10` pair. That is never a
scheduling residue and no barrier will move it.

Smaller sibling from the same function: **`sltiu` on a loop bound means an
UNSIGNED counter.** Retail's `sltiu $v0, $t2, 0xE` against a signed counter's
`slti` is a one-word residue with a one-token fix (`u32`), and it survives
`return counter;` from an `s32` function unchanged (still a bare `move`).

### `&arr[i + j]` and `arr + i + j` are one instruction apart (round 23)

The subscript form builds ONE index expression, so GCC sums `i + j` as integers
and scales the sum once. The pointer form scales **each addend separately**,
because each `+` on a pointer is its own scaled addition rather than a term in
one index.

So a one-word-short residue on a pointer-returning accessor, with an `addu`
sitting on the wrong side of an `sll`, is a SPELLING question and not a
scheduling one — try the other form before reaching for anything else.
Measured on `GetMoodFromStageChunk`: 19/20 with the subscript form, 20/20 with
the pointer form, no other change.

### An allocated-but-unused stack frame is not a residue (round 23)

Retail brackets `GetStageChunkFromMood` — a 43-word LEAF function — with
`addiu $sp, $sp, -0x10` / `addiu $sp, $sp, 0x10` and never touches the frame: no
store, no load, no `$ra` save. **It reproduces automatically** from ordinary C
with enough simultaneously-live locals.

So an unused frame is not a signal that the original source declared an array
that got optimised away, it is not something to reproduce deliberately, and it
is not worth an attempt to explain. Do not go hunting for it.

### The two-independently-live-locals lever, and the discriminator that predicts it (round 20)

Splitting a table-address computation into two independently-live locals —
`base = D_8006DCB0; entry = &base[idx];` instead of one combined expression —
closed 5 of 7 residue words on `func_80032BB8` (7/14 → 12/14).

**It does not generalise, and echo measured exactly where it stops.** One win,
two regressions (`func_80032C60` 13/14 → 2/14 *with drift*; `func_8004B100`
95/117 → 92/117), and two functions with no applicable shape at all.

**The discriminator:** the lever helps only when the starting shape is a single
expression built from an *unnamed global folded directly into one line*. Where
the "base" is already a named separate value — a parameter, or a local already
split for other reasons — it costs register pressure for no benefit.

A lever reported with its measured failure cases is worth far more than one
reported only where it worked; this entry exists in that form on purpose.

### A rodata `D_XXXXXXXX` holding a STRING is a symbol to REFERENCE, never a string to retype (round 20)

Splat has already emitted those bytes. Writing the string literal in your C
emits a **second** copy, and since `section_order` puts `.rodata` first, the
duplicate shifts the whole image.

**The diagnostic signature is what makes this worth its own entry: a clean
compile, a red whole-image SHA1, and a first differing byte in RODATA
thousands of bytes AHEAD of anything you edited** — carrying no hint of which
unit caused it. It is a sibling of the forgotten-`padNN` struct hazard in
CLAUDE.md, and it presents the same way.

Found closing `func_8003FC70` (35/35), a stall that had stood since round 14.
`D_80011194` *is* the format string `"not supported light mode %d\n"`. Round 14
had correctly identified that the function could not be C while its rodata slot
stayed attached, prescribed splitting the slot, tried boundary `0x1994`,
measured **339541 bytes** off, and filed "likely an alignment constraint on
where a rodata section may begin" as the open lead. **There is no alignment
constraint** — the split is byte-neutral on its own. The 339541 was the
duplicate string. Both halves are required: split the slot *and* write
`extern const char D_XXXXXXXX[];`.

**Before writing any string literal, grep `asm/data/*.rodata.s` for the
symbol.** If it is there, the extern is the only correct spelling.

### Two levers that closed long-standing near-misses by DELETING a named value (round 21)

Both of round 21's delta matches closed by removing a C-level name rather
than by adding or reordering anything, and both had survived multiple prior
rounds of operand-order and declaration-order reshaping. They are worth
stating together because the shared shape is the useful part: **a named
local is a promise to the allocator that a value must stay live, and the
lever is to withdraw the promise.**

- **Eliminate a pointer local that lives across loop iterations; index the
  array directly instead.** `func_8003F848` sat at 173/177 for two rounds
  with the residue described as "a single register-pair swap in one
  region". The fix was to delete the tail loop's decrementing `p`/`node`
  pointer locals and write `D_8009025C[i]` at the point of use. Round 20
  had rewritten the operand order and the declaration order of those same
  locals repeatedly; **it never removed the pointer itself**, and no amount
  of reshaping around a live pointer reaches the allocation decision the
  pointer forces. 177/177.

  Note this is the DUAL of the existing "take an explicit intermediate
  element pointer in an array loop" bullet, which fixes the opposite
  failure. They are not in conflict — one adds a name to stop GCC
  repurposing `self` as the induction variable, the other removes a name to
  stop GCC keeping a register pinned across iterations. The discriminator
  is what the residue is: a diverging INDUCTION variable wants the added
  pointer; a register-pair swap in a loop body wants it deleted. Try the
  one matching your residue, and try the other if it fails — between them
  they cover both directions and each is one edit.

- **Write a division in place into its dying dividend.** `func_80040154`'s
  last 2-word residue was the destination register of a third `mflo`.
  `self->unk84 = q2 / self->unk80;` and `q2 = q2 / self->unk80;
  self->unk84 = q2;` are the same computation; only the second lets the
  allocator reuse the now-dead dividend's register for the quotient, which
  is what retail does. 103/103.

  Generalising past division: **when a residue is "the result went to the
  wrong register" and one operand is dead after the operation, assigning
  the result back into that operand names the register you want without
  naming a register** — which keeps it on the legal side of the ban in
  HARD RULE 6, because removing it changes instruction selection rather
  than pinning a register by hand.

### A same-size sibling family is a single unit of work, not N units (round 21)

Round 21's bravo matched ten of eleven functions in a freshly carved unit,
and the shape that made it cheap was that four of them were same-size
21-word siblings of one "validate an index, then read or write a global
slot table" idiom. **The first sibling took real derivation; the rest were
one-attempt matches once its shape was known.**

Two practical consequences for a head and for a runner:

- **Order the queue by SIBLING GROUP, not by size.** Sorting a fresh unit
  smallest-first is the right default only until a family is visible. Once
  two functions have the same word count and the same call shape, do the
  family together while its idiom is in hand.
- **Same size plus same shape is a hypothesis worth stating in the report
  either way.** Round 21's bravo confirmed it for the slot-table four;
  the negative — a same-size group that does NOT share an idiom — is
  equally worth recording, because the next runner will otherwise form the
  hypothesis again from the same evidence. Round 21's alpha had the
  complementary case: six flag-bit dispatches in one function split into
  four nested and two flat, distinguished by branch TARGETS rather than by
  delay slots.

### Aggregate assignment vs scalar field-copy, and the rule that predicts which helps (round 19)

**Whole-struct/aggregate assignment and field-by-field scalar copy are not
interchangeable to GCC 2.6.3.** Writing a run of adjacent field copies as one
aggregate assignment closed **seven functions across three unit families** in
one round. It is the single highest-yield source-shape lever the project has
found.

But the round also measured where it does NOT help, from four independent
directions, and the negatives are what make it usable:

| runner | unit family | candidate | result |
| --- | --- | --- | --- |
| alpha | `code_2cc8c_f`/`code_2cc8c` | five, incl. a 3-byte RGB struct | **closed 5** |
| delta | `DreamSys` | 3-word out-parameter copy, 9 instructions vs retail's 6 | **closed 1** |
| echo | `code_55dd4` | `arr18` -> three contiguous same-type `s32`s, via a wrapper struct | **identical score, no movement** |
| delta | `code_8220_c` | the OT-splice family's `u16` tail copies | **no movement, with a mechanism** |
| charlie | `code_179d8_*` | none — shape absent from the units | n/a |

**The predictive rule, from delta's mechanism rather than from the tally:**

> The lever helps when the address being copied through is a **genuinely
> runtime-only value that resists constant folding** — a second pointer, an
> array index, an out-parameter. It does nothing when the address is a
> **compile-time-constant `self + literal`**, because that folds to the same
> single instruction however you spell it, leaving nothing for the scheduler's
> delay-slot filler pass to hoist.

Echo's negative fits the same rule from the other side: a naturally-aligned run
of same-type `s32`s already compiles optimally as scalars, so there is no
suboptimality for the aggregate form to fix. Restated as a screen: **try it
where the natural scalar compile is not already optimal (padded, odd-sized, or
runtime-addressed aggregates); do not try it on naturally-aligned same-type runs
at constant offsets.**

Delta closed the obvious escape route on the `code_8220_c` family too: making
the offset runtime-valued would mean writing the tail copies as a loop, and
retail's own disassembly shows them manually unrolled and branch-free on every
sibling. So that family is not a candidate at all, rather than a candidate that
failed.

Two extensions worth keeping:

- It applies to an **out-parameter**, not only to a struct-to-struct copy
  (delta, `func_8005942C`): `*(Vec3 *)out = *(Vec3 *)local;` replaced a 3-word
  element copy and removed a 3-word overshoot. The idiom had previously been
  written down only as "whole-struct assignment for a block copy".
- Arrays are not assignable in C89, so an array-shaped candidate needs a
  wrapper struct to test — echo did this correctly and it is the right way to
  get a clean negative rather than a false one.

**Scope honesty:** alpha declined to claim beyond `code_2cc8c`-descended units
from its own evidence, which was right at the time. The promotion above rests on
delta reaching the same idiom independently in `DreamSys` — a different unit,
different class, different route — which is what two independent families buys
that five siblings in one do not.


### `do { } while (0)` wrapping is a SMALL-BODY lever, and round 19 narrowed it (round 19)

Confirmed load-bearing for delay-slot scheduling a third time (alpha,
`func_80040854`, 19/19). **But two confirmed negatives the same round show it
actively REGRESSES larger functions containing loops or branches** (alpha,
`func_8003DAD4` and `func_8003E4B8`).

This NARROWS an existing learning rather than broadening it. Reach for it on a
small straight-line body; do not reach for it on a big one. A third data point
in the same direction from charlie: barrier/wrapper placement is
**non-monotonic** — moving a barrier one statement can go from 30/33 to 5/33
*with drift*, so "closer to the residue" is not a gradient you can climb.


### A preserved body's "clean / drift-free" claim must be RE-VERIFIED, not inherited (round 19)

**Five preserved stall bodies out of roughly thirty checked carried a
word-count claim that was false**, found independently by three runners in three
unit families:

- `func_80059BE0` — report said "clean (drift-free) build" at 45/79; the body
  compiles to **81 words, 2 longer** than retail's 79.
- `func_80063144` — "divergence #2" described as a zero-cost pure reordering; it
  is **1 word longer**. A net insertion.
- `func_80040C00` — report said "correct total length, no outside-range
  warning"; the literal body compiles to **4/52 with 191204 bytes of drift**.
  Alpha closed the function anyway, but only because it re-derived from
  `objdump` instead of trusting the report.

**The mechanism, and the part that generalises: a permuter `--debug` bucket
label such as `"Reorderings: 2"` names the SCORER'S INTERNAL EDIT-DISTANCE
OPERATION. It says nothing about word count against retail.** Two of the three
false claims trace directly to reading a bucket label as a size guarantee.

Once a body drifts, its in-range `funcdiff` score is meaningless — that is
CLAUDE.md's third way a score lies, arriving through an *inherited* body rather
than through your own edit, which is why the usual discipline does not catch it.

**Round 21 found a sixth, and it is the worst-behaved one yet because the
false claim survived THREE rounds of being acted on.** `func_800400B0`
(`code_2cc8c_e`) carried "29/41, same total instruction count, same
registers, purely reordered" through rounds 18, 19 and 20. It is not a
reordering: the body compiles to **40 words against retail's 41**, and
`build/lsdde.map` shows the next function in ROM order, `func_80040154`,
linking at `0x80040150` instead of retail's `0x80040154`.

**The guard was firing the whole time.** Reproduced by the head from the
report's own verbatim body:

```
func_800400B0: 20/41 words match (file 0x308B0-0x30954)
WARNING: the build differs OUTSIDE this range too (224685 bytes) — a size change may have
         shifted linked addresses, so this per-function read is NOT trustworthy.
```

224685 bytes of drift, printed in the same output as the score, three
rounds running, and each round wrote the in-range number into the corpus
anyway. **So this is NOT a new way a score lies and must not be filed as
one** — CLAUDE.md is explicit that recording a fired-guard case as an
oracle defect teaches the next runner to distrust the oracle exactly where
it worked. It is round 20's drift-attribution lesson in a worse form: there
the drift was misattributed to the wrong function, here it was not read at
all.

Two things follow that the round-19 entry above does not already say:

- **The failure is in READING, not in detection, so a better check will not
  fix it.** The check already exists, already runs by default, and already
  says "NOT trustworthy" in words. What failed is that a plausible in-range
  number sat directly above the warning and got copied out. Treat any
  `funcdiff` output containing the word `WARNING` as having NO score in it.
- **A wrong figure in a report title is self-propagating in a way a wrong
  prose claim is not**, because Gate 1b ranks from title lines. `29/41`
  read as a near-miss and kept attracting attempts across three rounds;
  the true 20/41-with-drift would have ranked it nowhere near the top. This
  is the third distinct mechanism by which a title-line figure has misled
  a head (rounds 18, 19, 21) — see "A permuter number is in PERMUTER units"
  and "An inherited report's PROSE can be wrong while its NUMBER is right".

**The check, before building on any preserved body** (delta's wording, and it
is seconds):

```sh
# paste the preserved body in, build, then BOTH of:
.venv/bin/python3 tools/funcdiff.py <fn>     # non-zero OUTSIDE-range count == drift
tools/binutils/bin/mipsel-linux-gnu-objdump -d build/.../<unit>.o   # literal word count
# compare against the .s header's own declared size, e.g. `nonmatching func_X, 0x4C`
```

A trustworthy answer is: outside-range count zero, **and** the word count equal
to the `.s`'s declared size. If either fails, the recorded in-range score is not
a starting point and the residue the report describes is not the real residue.

**Run it on bodies you actually resume, not as a sweep.** The other twenty-five
checked clean, so this is a real hazard at roughly one in six, not a reason to
distrust the corpus.


### A permuter zero is validated in ISOLATION and cannot see cross-TU damage (round 19)

Charlie found a **genuine zero** on `func_8002B3F4` — and rejected it. Reaching
zero required declaring a shared global `volatile`, which shifted a neighbouring
symbol's linked address and **corrupted an already-matched sibling**
(`func_8002A510`).

The permuter scores the target function compiled in isolation. It has no view of
the link, so a change that is locally perfect and globally destructive scores as
a win. This is a distinct failure from the known "a permuter score drop is a
LEAD, not a RESULT" rule: there the local score was misleading about the same
function; here the local score was *correct* about the target and wrong about the
image.

**So a zero earns the same whole-image oracle run as anything else**, and
specifically: any candidate that retypes, qualifies, or moves a symbol with
external linkage must be checked against `./build-and-verify.sh`, not just
`funcdiff` on the target.

Related, from echo: **a permuter run that never improves off the base score even
once is a stronger negative than the usual partial improvement** — 122K
iterations flat on `func_8004B100` says something different from 122K iterations
that wandered.


### Tooling defects found in round 19 (two since FIXED, one is not a bug)

- **FIXED.** `tools/setup-permuter.sh`'s scaffold validation ran the seed
  through the real `cc1`, which cannot parse `PERM_GENERAL`/`PERM_VAR` — those
  are read by the permuter's own pycparser front end, which substitutes concrete
  variants before any compile happens. It died with `parse error before 'int'`
  under `set -e` and no explanation, which is why the "hinted PERM_GENERAL
  search" several reports recommend could not be followed. The script now detects
  a PERM_* seed, skips the scaffold build, and names the permuter's own
  `--debug` as the correct validator (trap 5 in its header).
- **NOT A BUG, and not fixable in the harness: permuter scaffolds disagree with
  the real in-context build, three times now, all inside `class_3bb8c.h`.** `func_8004BB3C`'s scaffold reports base 460 with
  2 insertions/2 deletions against a residue that has none, because the isolated
  single-function compile schedules a pointer computation early where the real
  build defers it past a `blez` guard. Bravo hit two more. Possibly systemic to
  that class's register pressure. An isolated single-function compile genuinely
  does not reproduce the surrounding register pressure — that is what compiling
  one function alone *means*, so no scaffold generator can patch it out.
  **Before spending a search, check the scaffold's base score agrees with
  `funcdiff`'s residue** — if it does not, the scaffold is scoring a different
  problem and its results will not transfer. `setup-permuter.sh` now prints that
  check as its handover instruction.
- **`make clean` + re-extract desync.** Extracting `asm/` while a `src/` file
  still holds a non-`INCLUDE_ASM` body leaves that function's `.s` missing and
  hard-fails the next revert. Restore the `INCLUDE_ASM` *before* re-extracting.
- **FIXED.** The `block-raw-make.py` hook blocked a REDIRECTED `make extract`,
  which it advertises as allowed, because `SEPARATORS` omitted redirection
  operators — so `>` and the log path were judged as make *targets*. It also
  blocked writing prose about itself via a heredoc, since a heredoc body is
  command position to the tokeniser. Round 18 escalated this as an unexplained
  per-checkout difference; it was neither unexplained nor per-checkout (round 18
  simply used redirected forms in main and bare ones in the worktrees).

  Note for anyone touching this again: adding the operators to `SEPARATORS` does
  **not** work, because `shlex(punctuation_chars=True)` splits a leading file
  descriptor into its own token *before* the operator (`2>&1` -> `"2"`, `">&"`,
  `"1"`), leaving a stray `"2"` that still reads as a target. The fix is a
  `strip_redirections()` pass. The heredoc fix recurses into bodies fed to a
  *shell* rather than discarding them, so `bash <<'EOF' / make build / EOF` is
  still blocked — verified across 28 allow/block cases.

### A permuter number is in PERMUTER units, not retail words — three distinct misreadings in one round (round 19)

Put this next to the drift check, because it is the same failure wearing three
different costumes and it produced the round's only wrong *published* figures.
Every one of these numbers is printed by the permuter, looks like a measurement
of the function, and is not a word count against retail:

| what was read | what it actually is | cost |
| --- | --- | --- |
| `"Reorderings: 2"` | a **bucket label** naming the scorer's internal edit-distance operation | `func_80063144`'s "zero-cost pure reordering" was a net **+1 word** insertion |
| `210` (a base score) | a **weighted penalty**: 2 register-diffs x5 + 1 insertion x100 + 1 deletion x100 | `func_8004042C` was published as **"1 word remaining"** when it is 22/25, i.e. **3** |
| `Stack Differences: 0` without `--stack-diffs` | a field that was **never filled in** | `func_80065AE0` scored a false **zero** on an 8-byte frame overshoot (round 18) |

**The `func_8004042C` case is the one to internalise, because the arithmetic is
seductive.** One missing `move $t0,$a1` really is one missing instruction — so
"one word" reads as a faithful translation of the diff. It is not: the absent
`move` forces two downstream loads to read `$a1` instead of `$t0`, so the byte
comparison shows **three** wrong words. A single-instruction *cause* is not a
single-word *residue*, and nothing in the permuter's output distinguishes them.

That figure propagated from the report's title line into `PROGRESS.md`, into the
round-19 assignment built from it, and back to the runner as an instruction to
"close the one remaining word" — **three consumers, none of whom could have
caught it without rebuilding the body.** Alpha rebuilt it and corrected the
title, the prose, and every downstream claim.

**The rule: a permuter number is only ever a LEAD about a direction. The only
statements about word count come from `funcdiff.py` and `objdump` against a real
build.** When you write a score into a report's verdict line, write where it came
from.

**And a corollary for whoever ranks the queue** (a head job): build the Gate 1b
ranking from report **title/verdict lines only**, never from figures parsed out
of report bodies. Round 18 wrote this down after conflating a caller's score with
its callee's; round 19's head did it anyway and pulled `func_80032BB8`'s "0/14"
out of a sentence comparing a **rejected** variant — the real residue is 7/14, as
charlie established and corrected. The guidance as round 18 phrased it ("treat
figures near correction/superseded wording as retracted") is necessary but not
sufficient, because this figure sat in an ordinary attempt narrative with no
warning keyword anywhere near it. Only the title line is safe, and it is safe
only once someone has verified it — which is what this section is about.

### "Unscoreable" describes a SESSION's state, not a function's — compile a salvaged body as-is FIRST (round 19)

Two independent instances in one round, both of bodies recovered under
PARALLEL-RUNS 4c from runners killed mid-function:

- **`func_8005942C`** (delta) — filed as a mid-attempt snapshot at 19/56 with
  **140723 bytes of outside-range drift**, explicitly labelled "far from
  correct, not a near-miss", and carrying a note that the region needed naming
  in `include/` before it was worth retrying. Delta **matched it 56/56.** The
  control flow was already correct; it had three separable expression-shape
  bugs.
- **`func_8002AA6C`** (charlie) — no score had **ever** been recorded, because
  no author survived long enough to run the oracle on it. Charlie compiled the
  salvaged body unchanged, with no reshaping at all, and got **119/223, one
  instruction short.**

**The rule: before reshaping a salvaged body, compile it exactly as it is and
take a number.** A 4c salvage carries no stop rule, so its recorded state
reflects where a session was interrupted, not where the function resists. The
head's own 4c procedure already says to score a rescued body — these two cases
show the score is not just bookkeeping for a future round, it is frequently the
cheapest match available *right now*.

**Why this is easy to get backwards:** a mid-attempt snapshot's paperwork looks
exactly like a well-worked stall's — preserved body, a score, prose about what
is wrong — so it inherits the authority of a considered plateau it never earned.
`func_8005942C`'s own report said the right thing ("this is not a considered
plateau") and it still read as discouraging, because the accompanying number was
terrible. The number was terrible because nobody had finished the work, which is
the one reading the number cannot convey.

Corollary for whoever writes a 4c salvage report: say **"no stop rule was
applied"** in the verdict line, not only in the body, and put the drift figure
next to the score so the next reader can see the in-range number is meaningless
rather than merely bad.


### How to read a one-instruction residue (round 8, three stalls closed by it)

Put this first because it retired more standing stalls in one round than any
other idea here, and because the thing it replaces — a detailed, plausible
theory about GCC's scheduler — is what those stalls had been written up as for
three rounds.

- **When your diff is ONE redundant or ONE missing `move`, count how many
  times your SOURCE mentions the value.** It is almost never a scheduling
  choice. Two instances, closed the same round, pointing opposite ways:
  - `new_class_6d3c8` mentioned its return value once too **many** — a
    `return self;` on a path where the allocator had already left the value in
    `$v0`. The fix moved the `return` inside the `if` and let the null path
    fall off the end of a non-void function. Removing a mention removed a
    `move`.
  - `strcat` mentioned `dest` once too **few** — a null-guard spelled
    `return NULL;` where retail spelled `return dest;`. Identical value (dest
    *is* null there), but returning it USES it, keeping it live so the
    compiler establishes it in `$a0`. Adding a mention added a `move`.

  In both cases the surplus/missing copy landed in a branch delay slot, which
  is exactly why both read as delay-slot filler choices. **A surplus value has
  to go somewhere, and a free delay slot is where the scheduler puts it — so
  "different filler" and "one value too many" are indistinguishable in a
  diff.** Prefer the surplus-value reading: it has a source-level fix and the
  scheduling reading does not.

- **A delay-slot instruction belongs to the TAKEN path too.** MIPS runs it
  before control transfers, so when a branch's delay slot writes a register the
  TARGET reads, evaluate the value at the target, not at the branch.
  `func_80026698` had `li $v0, 0x1` in the delay slot of its case-3 branch and
  `sw $v0, 0x24(s1)` at the target — so the store writes **1**, not the `3`
  that `$v0` held for the comparison. Two rounds read it as `= 3` and that one
  wrong value manufactured two separate compiler mysteries: a "materialised
  unused default-arm constant" (not unused — it IS the stored value) and a
  "wrong register" store (with the right value, `$v0` is simply where the 1
  already is).

- **Do not transcribe a lowering back into C.** If your C reproduces retail's
  instruction sequence rather than the expression behind it, it usually costs a
  word. GCC 2.6.3 lowerings met so far:
  - `xori $r,$r,K` then `sltiu $r,$r,1` is `x == K` — *not*
    `(u32)(x ^ K) < 1`. That transcription is arithmetically correct and two
    instructions long. (`func_80026698`)
  - `sltu $r,$zero,$r` is `x != 0`; `sltiu $r,$r,1` is `x == 0`.
  - A `sll`/`sra` pair by the same amount is a cast to a narrower signed type.
  - A `mult`/`mfhi`/sign-fix chain is `%` or `/` by a constant — see the
    entry below.

- **The constructive direction: when retail HAS a redundant `move` and you do
  not, mention the source expression TWICE, dependent-quantity first.** For the
  common loop shape — a pointer and a bound derived from one loaded field —
  write

  ```c
  end = item->unk10 + N;     /* bound first, from the field */
  p   = item->unk10;         /* then the loop variable, from the field again */
  ```

  rather than caching the field in `p` and deriving `end` from `p`. CSE still
  emits a single load, but this order is what makes the compiler materialise
  the extra `move` into the callee-saved register instead of folding the loop
  variable into the loaded one. (`func_8004C0AC` derived it, `func_8004D1D0`
  confirmed it 29/29 a round later.)

  **This entry is here because not writing it down cost a stall.** It was
  derived in round 8 and left in one match report; round 9 stalled a function
  four addresses away on the identical residue, because the runner looked in
  this file, did not find it, and followed MATCHING-GUIDE's "best-posed
  permuter target" advice instead — which was correct behaviour given what was
  written down. An unpromoted learning does not exist.

- **Still the first discriminator, and it comes before all of the above: do the
  branch TARGETS agree?** A differing delay slot is a scheduling artifact; a
  differing branch target is a differing control-flow graph, and a differing
  CFG always comes from the source. (`strcat`, where reading past this cost 25
  words.)

### Confirmed on this game (each backed by a byte-exact match)

- **A two-armed `if`/`else`'s LAYOUT and its VALUE-PER-ARM are independently
  wrong-able, and sometimes both need fixing.** Which arm falls through and
  which is the branch target is one degree of freedom; which value each arm
  produces is another. Fixing only the one you noticed leaves the other and
  reads as an unrelated residue. (`func_8004C620`, `func_8004CE24`)
- **A `for` loop's multi-variable increment clause has a load-bearing order**,
  even with no data dependency between the variables — `for (; c; a++, b++)`
  and `for (; c; b++, a++)` are not interchangeable in the output.
  (`func_8004CE24`)
- **Walk an array with an incrementing pointer and dereference it directly.**
  `(*cell)->field` beats caching `*cell` in a local, and pointer increment
  beats array indexing — the cached form changes register assignment.
  (`func_8004CE24`; the same family as the explicit-element-pointer entry
  below, which is about the opposite mistake, so read both.)
- **An unused parameter in the callee shows up in the CALLER as a genuinely
  uninitialised local.** If retail's call site passes a register nothing ever
  set, do not invent a value for it: declare the local and pass it unset. The
  callee ignores it. Reading this as a bug in your own derivation is the trap.
  (`func_8004CC74`, whose callee `func_8004CDA4` ignores its 2nd argument.)
- **A getter's vtable can be found even when `classtable.py` does not know
  it**, by tracing the getter's own `lui`/`addiu` `%hi`/`%lo` pair back into
  `asm/data/*.s`. The fingerprint of a class method table is a count/header
  word followed by `BasicClass__func_17eb0` at `+0x004`. This found two
  previously unknown sibling classes in one round. (`func_8004D37C`,
  `func_8004D508`)
- **`sizeof` is not how this game allocates.** `New_X` wrappers pass a
  literal byte count to the allocator, so extending a struct's tail in a
  header cannot change an allocation size — which makes appending newly
  discovered trailing fields a safe, non-invasive edit. The one exception in
  `src/` is `DreamSys`, which does use `sizeof`. Check before you rely on it.

- **Write a small early exit as an inverted guard clause, not as the `else` of
  a big `if`.** `if (cond) { lots } else { return k; }` stops being reproduced
  once the `lots` side grows past some size threshold; `if (!cond) return k;`
  followed by the body reproduces it. Confirmed three times in one round
  (`DreamSys__TimerTick`, `func_8005AB2C`, `func_8005AE40`).
- **But the inverted guard clause is a DEFAULT, not a rule, and round 21
  found the counter-shape.** For a small bounds-check-then-body function,
  retail's polarity can be the plain `if (valid) { body } return -1;` —
  i.e. the un-inverted form the bullet above warns against. The size
  threshold cuts both ways: below it, the plain shape is what reproduces.
  **Check which arm actually falls through in the disassembly before
  assuming a polarity**, rather than applying the inversion reflexively
  (bravo, `func_80031C98` 22/22).
- **And nesting order, not the truth table, decides BLOCK order.** For
  `if (A) X; else if (B) Y; else Z;`, which branch is written as the outer
  `if` versus the `else` changes the order the blocks are emitted in, even
  where the two spellings are logically identical. On a near-miss that is
  short by one instruction or has blocks in the wrong order, swap the
  outer/inner nesting before suspecting anything subtler (alpha,
  `func_800336CC` 16/16).
- **An early-exit guard may have to leave the whole FUNCTION, not just the
  block it appears to wrap.** Check where the failing branch actually lands
  rather than what it visually encloses — if it lands on the epilogue, an
  unconditional tail call later in the function must not fire on the failure
  path. (`DreamSys__WallLink`)
- **Take an explicit intermediate element pointer in an array loop.**
  `Elem *e = &self->arr[i];` rather than repeated `self->arr[i].field`, or GCC
  2.6.3 repurposes `self` itself as the induction variable and the loop's
  register assignment diverges. Confirmed twice (`func_8004C588`,
  `func_8004C5D0`).
- **Statement order around a call decides whether a "default, then
  conditionally overridden" local survives it.** Assign the default AFTER the
  intervening call, not before: assigned before, the value has to live across
  the call and GCC promotes it to a callee-saved register, growing the frame.
  (`func_8005FDFC`; and the inverse negative — the same lever applied to a
  compile-time LITERAL rather than a struct-field read makes things worse, see
  `func_8005F544`.)
- **Do not cache a `this->field` across an intervening vtable call.** Re-read
  it at each use site; caching forces the same spurious callee-saved
  promotion. (`func_8005FB6C`)
- **A bare `andi $v0,$v0,N` with no sign-fix sequence is `& (N-1)`, not
  `% N`.** The `%` form always drags in the `mult`/`mfhi` sign-fix chain, so
  its absence is the discriminator. (`func_8005EFF4`)
- **A struct whose members are all `s8`/`s16` has alignment 2, and that is
  load-bearing.** It is what makes a whole-struct copy compile to unaligned
  `lwl`/`lwr` + `swl`/`swr` instead of aligned `lw`/`sw`. One stray `s32`
  member changes the alignment and the copy no longer matches.
  (`func_8004B38C`, and `FlashbackRotation` in `include/DreamSys.h` earlier.)

- **`x % N` for a compile-time-constant `N`: just write `%`.** Retail's
  `mult`/`mfhi`/`sra`-`subu` sign-fix followed by a `sll`+`addu` chain that
  rebuilds `N * quotient` and subtracts it is GCC 2.6.3's ordinary expansion.
  Reconstructing that sequence by hand is wrong; the plain operator reproduces
  it. (`func_800260A4`)
- **"Default value, then conditionally overwritten by an `if` with no `else`"
  is a real shape, and the alternatives are not equivalent.** 2.6.3 slides the
  default assignment into the guarding branch's delay slot for free. The
  `if/else` and `||` forms cost an extra instruction or change the comparison
  codegen. Reach for this when the residue is "one extra instruction" or "a
  `beq` where retail has `xor`+`sltu`". (`func_8005C9A4`)
- **When a branch seems to vanish where two sibling blocks share an identical
  tail call, write the literal jump graph with `goto`.** 2.6.3's
  cross-jump/tail-merge can merge two blocks whose *guards* differ, dropping a
  comparison. Do not reach for `||` or nested `if/else` first.
  (`func_8005C714`, second instance in the same unit)
- **A whole-struct assignment, not an indexed `for`, for a block copy.** Where
  retail's first loop batches 4 words per iteration cycling four temp
  registers (`v0,v1,a0,a1`), that is 2.6.3's inlined block-move for a struct
  assignment. A per-word loop cannot produce it and silently shifts every
  later function. (`func_80025E1C`)
- **`lui`/`addiu` to a symbol with no surrounding `lw`/`sw` returns
  `&symbol`**, not a value read from it. Check the callers before guessing a
  dereferencing signature. (`func_800269E0`)
- **Calling into a function that is still `INCLUDE_ASM` in another unit is
  fine.** Add a local `extern` prototype at the call site, typed from the
  registers loaded before the `jal` and from whether the caller consumes
  `$v0`. It need not be authoritative — only the call site's own bytes depend
  on it. (`SetTeleportsEnabled`)

- **An early exit returning a DIFFERENT value from the main path: try `goto`
  and `return` both, they are not interchangeable.** With `return OTHER;` GCC
  places the exit block after the main path, leaves the branch delay slot as
  `nop`, and needs a `j` to reach the shared epilogue -- one extra word. With
  `goto fail; ... fail: return OTHER;` the exit value is stolen into the delay
  slot and the branch retargets to the epilogue. (`func_80025B34`)
  **This lever is narrower than it first looked, and three runners bounded it
  in one round.** It does NOT apply when the allocator also tests the
  constructor's return (`New_class_65650` matched 31/31 with plain
  `if`/`return`); it does NOT apply when the normal path contains a loop
  (`func_80026410` needed the whole body wrapped in the positive condition
  instead); and a superficially similar residue can want the plain early
  return (`func_80059AEC`). Read the asm; do not apply it by reflex.
- **`while (*p) { p++; }` and `while (*p++) { } p--;` are different code.** The
  post-increment idiom increments unconditionally and backs up at the merge, so
  its guard branch targets the `addiu rN,rN,-1` fixup; the pre-test idiom skips
  it. When retail's guard branch jumps TO a decrement, the source used
  post-increment. Worth 25 words on one function. (`strcat`, 16/42 -> 41/42)
- **A guarded `do { } while` where the source looks like it wants `while`** --
  promoted out of "unconfirmed" below. (`func_8005CAB4`, 10/69 -> 69/69)
- **Let GCC hoist its own loop invariants.** Naming an array directly in the
  loop condition (`for (p--; p >= events; p--)`) and hand-hoisting a `base`
  local are not equivalent: the hand-hoisted form is live at the guard, so GCC
  allocates the callee-saved register immediately and compares against it,
  losing retail's temp/compare/copy-in-the-delay-slot preheader. Worth 23
  words. (`func_80025D10`, 38/65 -> 61/65)
- **Prologue callee-save store ORDER is not reachable from C.** Six
  declaration-order permutations produced one identical score; statement order
  and guard spelling did not touch it either. A bare `__asm__("")` as the
  function's FIRST statement is the lever, and it is the permitted form -- but
  verify rather than assert: remove it, rebuild, and confirm the register
  ALLOCATION is unchanged. That is exactly the test CLAUDE.md rule 6 states.
  (`func_80025D10`, 61/65 -> 65/65). It did NOT generalise: two runners tried
  it on unrelated residues and worsened them, and on `func_8005C76C` explicit
  assignment *statements* in the desired order worked instead.
- **There are at least three `New_X` allocator sub-shapes.** (1) ignores the
  constructor's return, one early exit -- needs `goto` (`func_80025B34`);
  (2) returns the allocation unconditionally, no early exit -- open stall
  (`new_class_6d3c8`); (3) tests BOTH the allocation and the constructor's
  return, freeing on constructor failure -- plain `if`/`return`
  (`New_class_65650`). Identify which from the asm before choosing a spelling.
- **A call through a vtable slot needs no forward `extern` prototype** -- only
  the struct field's type has to be right. This differs from the direct
  `jal`-by-name case above. (`New_class_65650`)
- **`return self->field = N;`** reproduces retail's single-`ori`-reused-for-
  store-and-return shape for "set a literal, return the same literal" setters.
- **For a `switch`, GCC 2.6.3 lays out case bodies in TEXTUAL source order but
  picks its own comparison order** (binary-search pivot). Do not transcribe the
  observed comparison order as the case order. (`func_8005966C`,
  `func_800596E8`) **PROMOTED round 23 to its own section — see "A `switch`'s
  CASE ORDER is recoverable from the binary" above** for the recipe (sort arm
  labels by address, map through the jump table, fall-through arm is last), the
  four measured instances, and the INVERSE for a sparse non-table switch. Do
  not act on this line alone; half of it was independently rediscovered twice
  in one round because the recipe was missing.
- **A delay slot after a `jalr` captures the PRECEDING call's return value, not
  the upcoming call's argument.** A real misread, easy to commit with two
  adjacent calls. (`func_80026170`, `func_8002677C`)
- **Solve a magic-multiply divisor arithmetically rather than guessing it.**
  Given the multiplier and shift, solve `constant * N == 2^(32+shift) +
  remainder` across candidate divisors; close constants at different shifts
  render guessing unreliable. One function's assumed "divide by 9" was actually
  15. (`func_8002658C`)

- **A value in an ARGUMENT register that is live at the next call IS an
  argument — even when its only visible use is a branch condition.** This is
  the round-2026-08-30-c headline lever: it converted two functions that had
  been filed as unreachable *register-identity* stalls into ordinary matches,
  in two different units. `func_80059E3C` stalled at 21/23 with retail doing
  `lw $a1, 0xBC($s0)` / `bltz $a1` where the natural codegen produced
  `lw $v0` / `bltz $v0` — same branch target, everything else byte-identical.
  Tracing forward, nothing overwrote `$a1` before the following `jalr`, so it
  was still live at the call: retail's source read
  `obj->vt->slot0x84(obj, this->unk_0xBC)`, and the body under test was one
  parameter short. GCC only picked an argument register *because* the value was
  an argument; the register choice was a consequence of the missing parameter,
  not an independent codegen quirk.

  **The test, before ever classifying a residue as register-identity:** trace
  forward from the load to the next `jal`/`jalr`. If the register is `$a0`–`$a3`
  and nothing overwrites it in between, the parameter list is wrong and this is
  an ordinary match.

  **The negative half matters just as much, and is what keeps the test safe.**
  Three of the round's residues looked similar and were NOT this class:
  `New_DreamSys`'s residue register was `$v0` (a return value, never live into
  a following call) and closed with an ordinary reshape; `func_80065A5C`'s
  residue is prologue callee-save STORE ORDER, not a value in the wrong
  register; `func_800662BC`'s is a loop-carried value never passed to the call
  at all. A `$v0` residue, or a register dead at the next call, is genuinely
  not this class — do not go hunting a parameter that was never there.
  (`func_80059E3C`, `func_8005DD18`; negatives `New_DreamSys`, `func_80065A5C`,
  `func_800662BC`)
- **A missing argument can be invisible from the callee's own disassembly.**
  `func_8005DD18` passes a literal `0` that `func_8005DF9C` never reads — the
  callee overwrites the register as scratch on entry. A signature derived by
  reading the callee alone is therefore wrong with nothing to flag it, and only
  the CALLER's register setup recovers it. Type a slot from its call sites, not
  from its body. (`func_8005DD18`)
- **A discarded return value is never evidence of `void`.** A previous round
  recorded `Class65650Methods` slot `+0x134` as returning `void` because the one
  known call site threw the result away; it actually returns `u8 *`. This is the
  same trap the runner prompt already flags for one-line tail-call wrappers,
  in a second guise — a byte match constrains the return type only where the
  caller USES the value. (`func_80065FD8`)
- **When two structurally-similar residues want different C shapes, retail's
  own instruction count is the tell.** Two "get old value, conditionally set
  new" functions in DreamSys needed opposite spellings — `func_8005A168` a
  single hoisted load, `func_8005BA20` a load duplicated into each branch.
  Nothing in the surrounding code distinguishes them; the word count does.
  Count before reshaping blind. (`func_8005A168`, `func_8005BA20`)
- **`__asm__("")` is not a local lever — it perturbs the WHOLE function's
  register allocation.** It fixed a store/branch-ordering residue in
  `func_80065E1C` and simultaneously introduced a full `$s1`/`$s2` identity swap
  between `self` and the array-walk pointer across the entire function. The
  blast radius scales with function size, so reach for it late and revert it
  fast. This does not make it banned — it still only reorders — but "it helped
  here" and "it is safe here" are different claims. Related: an 8-byte padding
  local fixed callee-save store order in `func_80065AE0` for free, with no
  barrier at all. (`func_80065E1C`, `func_80065AE0`)
- **Verify struct offsets with a host `-m32` `offsetof` build, not a native
  one.** Native 64-bit pointers silently widen every pointer field and the
  cross-check passes while the real layout is wrong. A runner lost real time to
  this before catching it; `sizeof(DreamSys)` turned out to be 0x928, some 0x98
  bytes past where it had been modelled — recovered from the allocator's own
  literal `ori $a0, $zero, 0x928`. An allocation size in the caller is
  load-bearing evidence about a struct's true extent. (`New_DreamSys`)
- **Vtable slot order follows function ADDRESS order, exhaustively.** Confirmed
  against `DREAMSYS_METHODS` with `tools/classtable.py`: a run of five
  consecutive slots (`+0x180`..`+0x190`) maps onto five functions at consecutive
  addresses. Useful for resolving unnamed slots — but keep resolving with the
  tool rather than by counting; the ordering tells you where to LOOK, and it
  corrected a previously-wrong comment about a gap that did not exist.
- **Six residues from one 258-word function, each closed independently.**
  `func_80066340` reached 252/258 and is the project's most detailed partial
  derivation; the levers generalize to any large body. Do not cache a struct
  field that is read in several places (`self->unk5C`) — it costs an extra
  register; a multi-way tag dispatch must be a real `switch`, not an
  `if`/`else if` chain; do not cache a flags byte across separated tests; a
  tail copy wants batched loads into a dedicated local; inner loops want
  INCREMENTING POINTERS, not array indexing; and loop setup sometimes wants an
  explicit `i = 0;` statement between two pointer initialisations rather than a
  `for`'s implicit initializer. (`func_80066340`)

**From round 2026-09-02 (5 runners: DreamSys, Entity_b, class_39e08, class_3ac78,
code_2c054).**

- **A dedicated local pins an address-of expression's SCHEDULING.** Assigning
  `&this->unk14->x` to its own local, in the statement position retail computes
  it, pins GCC's placement. Written inline as a call argument, the compiler
  defers it arbitrarily. (`func_8005DE18`)
- **An over-narrow parameter type forces a spurious sign-extend at the CALL
  SITE.** `func_8005D714`'s `arg2`/`arg3` were modelled `s8` because every known
  caller happens to pass a byte-range value. The callee's own body treats them
  as full words with no narrowing on entry, and the `s8` declaration cost an
  extra sign-extend at any call site whose argument was an already-computed
  `s32`. **Read the callee's body for the width it actually uses, not the
  callers for the width they happen to pass.** Retyped with no regression to the
  existing matched caller. (found via `func_8005DE18`'s residue)
- **The same width rule governs STACK arguments, which was not previously
  recorded.** An unsigned narrow stack argument loads as a single `lhu`;
  a signed one as `lw` + `sll` + `sra`. Identical in principle to the
  register-argument rule above, but the stack case had never been written
  down here, and reading a `lhu` as "unsigned" is the fast way to fix a
  three-instruction residue that looks structural (bravo, round 21).
- **Passing a `u8` lvalue straight to an `int`/`s32` parameter costs a
  redundant `andi $x, $y, 0xff` at the call site** — including when you
  have just assigned it a small literal, where the mask is provably
  useless. Introduce a plain `s32` temp and pass that. This is the
  call-site mirror of the over-narrow-parameter bullet above: there the
  narrow type was on the callee, here it is on the local (alpha,
  `func_800336CC`).
- **A negative mask materialised as `addiu $vN, $zero, -N` is `~(N-1)`,
  not `~N`.** `-3` is `~2`. Convert to the explicit two's-complement bit
  pattern before writing the C, rather than reading the decimal off the
  disassembly and complementing it (alpha, `func_800339AC` 40/40).
- **A `void`-typed vtable slot that fails to compile against a
  `return callee(...)` wrapper is itself evidence the slot typing is wrong.** A
  free signal — the compile error arrives before any attempt budget is spent.
- **MIPS o32 fills argument registers strictly left to right.** So a vtable call
  that sets `$a2`/`$a3` to literals while leaving `$a1` untouched PROVES `$a1`
  carries a real forwarded parameter; there is no "skip a register" call shape.
  (`class_3ac78`)
- **The converse does NOT hold.** A `jalr` with a plain `nop` delay slot and no
  argument setup is not proof of a zero- or one-argument call. Check the
  callee's own body, or another caller, for its true arity before treating an
  untouched register as leftover garbage. (`class_3ac78`)
- **Establish a sibling-class relationship by diffing both candidates against a
  COMMON BASE, not against each other.** A long run of identical slots can come
  from two independent overrides that share an implementation, not from
  inheritance. (`tools/classtable.py <t> --vs <base>`, `class_39e08`)
- **A function shared verbatim between two sibling vtable slots is only safely
  reusable if every instance field it touches sits at the same offset in BOTH
  classes.** Same code at the same slot offset does not imply the same layout
  behind `self`. (`class_39e08`)
- **The "a byte match tells you nothing about the return type" trap applies to
  an INTERMEDIATE link in a delegation chain, not just to an outermost
  wrapper.** A slot typed `void` on the strength of one unit's discarding caller
  records what that CALLER does with the value, not what the occupant computes.
  Read the occupant's own disassembly. This corrected
  `LoaderTaskMethods::slot44` in `include/Class6D3C8.h` to `s32`, ABI-neutrally
  and at zero byte cost. (`func_8003C1DC`)

- **A repeated read of an unchanging pointer field is NOT reliably CSE'd across
  statements when a whole-struct assignment sits between the reads.** GCC 2.6.3
  reloads it. Cache the pointer in an explicit local instead. Note this is the
  OPPOSITE prescription to `func_80066340`'s "do not cache a struct field read
  in several places" — the discriminator is what sits BETWEEN the reads: an
  intervening aggregate assignment defeats the compiler's aliasing analysis and
  forces the reload, whereas plain straight-line reads are CSE'd fine and the
  cache then costs a register. Read the intervening statements before choosing.
  (`func_8005B904`)

- **A register-identity residue can be a MISSING CALL ARGUMENT. Cross-check
  another caller of the same vtable slot before you accept the
  classification.** The strongest result of round 7, and it retired a stall
  that fourteen attempts across two authors had confirmed. `func_8005E02C`
  kept landing `this->unk94` in `$v0` where retail has `$a1`, unmoved by
  every reshape of the code computing it. The cause was that
  `EntityMethods::slot144` takes a SECOND argument — `this->unk94` itself —
  so retail parks the value in `$a1` for the whole function *because that is
  the register the call needs it in*. With the two-argument prototype it
  matched first try, no barrier and no reshape.

  The trap is that the call site under test looked like positive evidence
  for the one-argument signature: a plain `nop` in the delay slot and no
  fresh `$a1` load. There was no fresh load because the value had been
  resident in `$a1` since the top of the function. A *different* caller
  (`func_8005EA94`) loads it explicitly right before the `jalr`, which is
  unambiguous. **CLAUDE.md rule 6's test is still right as written** —
  reshaping genuinely could not move that register — but "reshaping" has to
  include the call's own ARGUMENT LIST, not just the statements feeding it.
  A slot's arity is a property of the slot; derive it from whichever caller
  makes it visible. (`func_8005E02C`, `func_8005EA94`)

- **Retyping a shared vtable slot is NOT a local change.** `void` -> `s32` on
  `EntityMethods::slotC4` fixed the wrapper under test and silently broke an
  already-matched function in another unit: with `slotC4` `void`, GCC
  tail-merges two of `func_8005E160`'s identical discarded `slotC4` calls
  into one; typed `s32` it stops merging, costing 4 words and shifting every
  later function in the file. `slotCC` faced the identical question in the
  same round, was checked the same way, came back clean, and was retyped —
  **two superficially symmetric slots, opposite answers.** Check every other
  caller (rebuild; a broken slot retype is a RED BUILD, not a diff in your
  own function) rather than reasoning by analogy from a sibling slot.
  (`func_8005FA64` kept `void`, `func_8005FEC8` retyped)

- **A value reused after an intervening indirect call needs an explicit
  local.** GCC 2.6.3 has no aliasing guarantee that a `jalr` through an
  unknown function pointer did not write back through the object, so a bare
  repeated field access forces a reload, which changes register allocation
  and can shift the frame size. **The fast tell is a frame with the wrong
  number of callee-saved registers versus retail.** Confirmed independently
  several times in one round, and it generalizes past `self->field`: it
  applies to a call's own return value, and to a value whose next use is
  many calls later rather than the next one. (`func_8003C3D0`,
  `func_8003C11C`, `func_8003BF10`, `func_8003C238`)

- **An arity conflict at an already-typed vtable offset is real
  counter-evidence — trust it over an earlier positive-but-circumstantial
  slot match.** It is what legitimately *un*-unifies two fields previously
  modelled as the same class. Round 7 used it twice in one function to split
  a wrongly-shared method table and to revert a wrongly-unified field type,
  both as pure header relabeling with zero compiled-byte impact.
  (`func_8003C238`)

- **An empty-bodied vtable occupant is not evidence the SLOT takes no
  arguments** — only that this occupant ignores them. The parameter-side twin
  of the established "a discarded return value is never evidence of `void`".
  Relatedly, a slot's *field name* is whatever the first-resolved occupant
  suggested; it describes layout and signature, never which function runs.
  That risk is narrow, though — tested against ten functions across two
  units with no second instance found, and it only bites when a dispatch
  through `self->methods` sits BETWEEN two writes to it. A mere double-write
  is not sufficient.

- **GCC 2.6.3's switch pivot tree depends on the exact case-value SET,
  including otherwise-empty cases — not on declaration order.** `{1,3}` and
  `{0,1,3}` both produced the wrong comparison tree where `{1,2,3}` with an
  empty `case 2:` matched. Read the other way round: a GAP in the case values
  you can see is itself a signal that the original had an empty case there.
  (`func_80049CA8`, 9 attempts)

- **Split a value's COMPUTATION from its STORE through a named local** when
  retail defers the store into a later instruction's delay slot (a tail
  call's, for instance). Textual adjacency alone does not predict this.
  (`func_8004AFE0`)

- **Stack-local DECLARATION ORDER decides which local lands at which `$sp`
  offset**, and `if`/`else` versus its logical inverse compile to different
  branch polarities (`beqz` vs `bnez`) — neither is a free choice.
  (`func_8004AEA4`)

- **A source-level re-test of an already-established condition is not dead
  code** — 2.6.3 compiles it literally. Conversely a chain of
  mutually-exclusive-*looking* literal checks may be independent `if`s rather
  than `else if`: check whether retail's bytes re-test the later conditions
  after an earlier one already matched. (`func_8005E160`, `func_8005E7F8`)

- **Statement order, not data dependency, decides register class and store
  order** for independent assignments. This compiler does not reorder either
  on its own. (`func_8005ED30`, `func_8005E7F8`)

- **Counting slots forward from a `classtable.py`-confirmed neighbour often
  resolves a "new" slot to an ALREADY-MATCHED function**, turning apparent
  new-header work into none. Worth trying before typing a slot as new: in one
  DreamSys batch every raw offset touched but two turned out to be already
  named. (round 7, DreamSys)

- **`bool` is `typedef int bool` here — a full word, not a byte.** Reading a
  raw struct offset as byte-granular because a field is boolean produces a
  layout error that looks like a struct-size mistake rather than a typedef
  mistake.

- **The `__asm__("")` barrier is a scheduling nudge, not a fence — and its
  scope is narrower than previously assumed.** Two round-7 results bound it
  from both sides. It DID change which instruction fills a load-delay slot at
  its own position (forcing retail's order, `move` before the barrier and an
  explicit `nop` after). It did NOT stop a loop-offset increment from being
  hoisted BACKWARD across it into an earlier delay slot, past several
  intervening independent statements. So: use it for local delay-slot fill,
  do not expect it to pin anything across statements, and never expect it to
  move a register choice (it operates in a later pass than register
  allocation). (`func_8005E02C` positive, `func_8004ABD0` negative)

- **GCC reserves stack space for a completely dead, unreferenced local**, so
  `u8 unused[N];` closes a pure frame-size gap when every instruction already
  matches. Third project instance, so it is a reliable idiom — but treat the
  gap as EVIDENCE, not explanation: a 24-byte hole is most likely a real
  local aggregate the original source passed somewhere, and the padding
  reproduces the bytes without explaining them. Say so in the report.
  (`func_8004ADD8`)

- **Multiple independent local views of the SAME method table, one per unit,
  is the established convention** — do not edit another unit's header to
  unify them. Round 7 had two units both describing `D_800866E8` under
  different type names, deliberately. (`class_3ac78`, `class_3bb8c`)

- **"One instruction short" describes the SCORE, not the defect count.** The
  round's clearest diagnostic lesson. `func_8005F544` sat at 30/49 with a
  report describing a single-instruction residue; it was FOUR independent
  residues stacked, each needing a different lever, and closing three of them
  moved the score not at all until the fourth went. Diagnose incrementally
  and re-measure after each change rather than looking for one explanation
  that accounts for the whole gap. (`func_8005F544`, closed 49/49 after a
  six-attempt stall)

- **Retail's PHYSICAL BLOCK ORDER is part of the match, and nested
  `if`/`else if` chooses its own.** Where retail dispatches into a shared
  continuation with one arm's failure path relocated *past* the shared block,
  no arrangement of nested conditionals reproduces the layout — explicit
  `goto` and labels, laid out in the disassembly's own physical order, do.
  The conditional form gets every VALUE right, which is what makes this hard
  to spot. (`func_80058B08`; same family as the two-armed-`if` layout entry
  above, but about a merge point rather than two arms)

- **To duplicate a compile-time constant across the predecessors of a CFG
  merge, write it as its own statement in each predecessor and `goto` a
  shared label.** A single expression at the merge point never produces the
  duplication, because there is only one of it. Retail does this whenever a
  caller-saved register holding a constant is clobbered by an intervening
  call and the value is still needed after the merge. Expect to need an
  `__asm__("")` alongside, to stop GCC floating the statement past unrelated
  stores. (`func_8005F544`)

- **Cache a vtable pointer into an explicit local BEFORE any intervening
  call, or every use after that call reloads it from memory.** The cost is
  not one word: it can consume a whole callee-saved register and produce the
  wrong frame size. This is the opposite-direction companion to the
  address-of-slot entry below, so read both — one is about caching too
  little, one about caching the wrong thing. (`func_8003CAF8`,
  `func_8003C51C`)

- **`&obj->vtable->slotNN` — the ADDRESS of the slot, not the pointer value
  — assigned to a local before a branch, reproduces GCC 2.6.3's split-load
  scheduling** (base pointer early, slot value adjacent to the call). A bare
  cached pointer VALUE is free to float arbitrarily far up the block and
  will. Give each call site its own local rather than sharing one; sharing
  forces a coarser allocation and costs a `move`. (`func_8005FC58`, found by
  permuter; reused by hand on `func_8005F544`)

- **Keep a pointer computation that is one retail expression as ONE C
  statement.** Splitting it across two statements — even where that reads
  more clearly — can make GCC commit the intermediate to a permanent
  callee-saved register one statement early. This single change moved
  `func_8004B100` from 47/117 to 95/117. (`func_8004B100`)

- **Unconditional-then-overwrite, not a ternary, for a two-constant
  selection.** `d = -50; if (cond) d = 50;` reproduces retail's branch
  polarity where `d = cond ? 50 : -50;` does not. Related but distinct: a
  lazy-init store of a small integer into a struct field also prefers
  `if`/`else` over `?:`, which can put the literal in a different register
  entirely. (`func_8005AC24`, `func_8005F800`)

- **A byte copy is `lbu` under `-funsigned-char` no matter what you write —
  unless the value passes through a wider (`s32`) local first, which forces
  `lb`.** C-level signedness of the source or destination does not reach the
  load; the promotion does. (`func_8003CB68`, where it was the one residue
  that DID yield)

- **`addu`/`+` operand order in a byte-truncated sum follows the source's
  left-to-right expression order.** (`func_8003CC2C`)

- **When a function closely resembles an already-matched sibling in the same
  file, copy the sibling's exact local-variable-versus-repeated-field-access
  idiom before deriving anything.** `func_8005AC24` matched on the first
  attempt this way — cheaper than the permuter and cheaper than manual
  derivation, and the idiom is not recoverable from the disassembly alone.
  (`func_8005AC24`, from `func_8005AB2C`/`func_8005AD68`/`func_8005AE40`)

- **An argument register that survives an intervening CALL is not reliably an
  argument.** Trace forward past every call, not just to the next `jalr`,
  before deciding a register is a parameter. (`code_2cc8c`, round 10)

- **A base constructor that sets `self->methods` directly can still dispatch
  through `self->methods` immediately afterwards in the same function** —
  plain sequential assignment then dispatch, no re-fetch through a getter and
  no caching. (`func_8004D578`)

- **Two residues can be ENTANGLED, so a change with independent evidence is
  not refuted by scoring worse alone.** An `if`/`else` branch-polarity fix
  scored *worse* in isolation and was the key unlock once combined with an
  unrelated scheduling fix. A/B testing one change at a time is the right
  default, but it silently discards any fix whose benefit only appears in
  combination — so if a change has independent evidence behind it, keep it
  and keep looking rather than reverting on the score. (`func_8004B700`,
  52/140 -> 125/140)

- **GCC 2.6.3's strength reduction collapses two array-field accesses into
  ONE induction variable once it can prove they share a base and index, and
  C-level grouping does not stop it.** Plain indexing, an intermediate
  element pointer, a nested sub-struct and a manual byte-cast dual-pointer
  walk all collapse identically. Retail walking an array with two
  independently-incrementing pointers therefore needs the relationship
  genuinely severed — or, as in `func_8004B700`, the second walk to be over
  an unrelated array. (`func_8004BB3C`, missing exactly one
  `addiu $s4,$s4,0xc`)

- **A store-then-reread of a narrow SIGNED field can compile as an unsigned
  reload plus a manual sign-extend rather than retail's single `lb`**,
  costing an instruction. Isolated reproducer in the report. Note this is a
  different mechanism from the `-funsigned-char` byte-copy entry above,
  which is about the load width; this one is about the sign-extension
  strategy after a reload. (`func_8004C1C0`)

#### Round 11 (62 matches across four fresh carves)

- **GCC 2.6.3's CROSS-JUMP / tail-merge pass folds two source-distinct but
  RTL-identical statements into one shared instruction, and a bare
  `__asm__("")` does NOT stop it.** Cross-jump operates at block level, not on
  local instruction scheduling, so the barrier has nothing to bite on. Two
  independent runners hit this in different units in one round: two `i++;`
  statements merged into one (`func_8003D3B0`), and an `if`-guard plus a
  `do/while`'s own test — two textually identical calls — folded to a single
  call site (`BasicClass__func_18040`). The fix is to remove the RTL identity
  rather than to suppress the pass: fold the increment into the
  branch-deciding expression (post-increment array index), or write the two
  calls in the shape that keeps them distinct.

- **THE DISCRIMINATOR for whether `__asm__("")` can help at all.** These two
  look alike in a diff and need opposite responses:
  - *Same instructions, different ORDER* — two independent, already
    identically-registered instructions swapped. A bare `__asm__("")` fixes
    it, including as a loop body's own last statement after a manual
    increment. (`func_8003D2CC`)
  - *Missing or duplicated instruction* from cross-jump merging. The barrier
    does nothing; reshape the source. (`func_8003D3B0`,
    `BasicClass__func_18040`)

  This refines the project's standing register-vs-order rule rather than
  replacing it: a barrier that changes WHICH REGISTER holds a value is still
  banned, and a register-identity mismatch is still a stall.

- **A single function that is one word short makes EVERY later function in
  ROM order score near-zero at once — and the fingerprint is specific.**
  `funcdiff`'s "differs outside range" count shows the SAME six-figure value
  across all the affected functions. Check for that shared value BEFORE
  debugging each function as its own bug, then fix the earliest function in
  ROM address order; the rest resolve themselves. CLAUDE.md's "four ways a
  score lies" already names address drift, but had no stated signature for
  it. Three instances in one round, one of them caught and applied
  deliberately by the runner that found it. (`func_80060710`, `func_80061070`,
  `func_80060148`)

- **Eager initialization, and its converse — neither is a default.** A struct
  field or local whose value must survive an intervening call often has to be
  read/initialized as the function's FIRST statement, because retail keeps it
  in a callee-saved register across the call; the tell is a frame missing one
  callee-saved register pair. (`func_80061070`, `func_80060148`,
  `func_800605D0`) But the converse is equally real: `func_80060D80` matched
  only by initializing at retail's actual delay-slot init point rather than
  reflexively at the top. **Read the disassembly's init point per function.**

- **A "unit-wide field-read order" is a per-function property, not an
  invariant — measured, and falsified.** An `unk58 -> unk64 -> unk5C` read
  order held across five functions in `code_2cc8c_b` and looked like a unit
  convention worth assuming. `func_8003DE9C` violates it outright
  (`unk58 -> unk60 -> unk64 -> unk4C`) and matched immediately when written in
  its own literal disassembly order. The order tracks whichever field that
  function's source references first. Recorded because the head explicitly
  invited the generalization and the runner correctly reported the negative.

- **A value that is only READ inside an `if` is not evidence it is only
  ASSIGNED there.** Retail frequently computes and stores unconditionally and
  gates only the call. Check whether the suspect store sits in a branch's
  delay slot — which always executes — before trusting the naive C guard
  scope. (`func_8003D4DC`, two independent instances in one function)

- **Caching a loop bound that the source actually re-reads costs a whole
  extra callee-saved register**, and it is diagnosable by a FRAME SIZE
  mismatch (word count / saved-register count) rather than a register-identity
  swap. The general rule the round converged on, from both ends: cache a
  re-read struct field only across a CALL-FREE span, and reload it after any
  intervening call. Charlie reached the same rule from the opposite direction
  and it is stated once here rather than as two observations. (`func_8003D2CC`,
  `func_8001CAF4`)

- **Asymmetric vtable-pointer caching is not a contradiction.** One function
  can legitimately cache `this->methods` for one call site and reload it fresh
  after an intervening call. Read each call site independently rather than
  imposing one policy on the function. (`func_80060B34`)

- **GCC 2.6.3 at `-O2` does not eliminate a dead store into an address-taken
  local**, even when a callee overwrites it unconditionally immediately after.
  An initializer that looks logically redundant may still be load-bearing.
  (`func_8001D204`)

- **The same magic-multiply constant can encode two different divisors**
  depending on an extra post-`mfhi` shift, so the hex constant alone does not
  identify the divisor. (`func_8005FF7C`, `0x2AAAAAAB`)

- **Branch-polarity is not reflexively "invert the small early exit".** When
  a function returns a plain literal on both paths rather than reusing a
  just-nulled register, the POSITIVE `if` can be the correct shape. Check
  which block is retail's actual fall-through. A two-armed `if`/`else` can
  also have correct per-arm VALUES with swapped LAYOUT — three residue words
  vanished from one polarity flip. (`func_800181AC`, `func_80018208`)

- **Per-call-site arity and typing, reconfirmed twice more.** A call site can
  pass two arguments into a slot whose current occupant is a no-arg
  `void(void)` no-op, and two functions can pass the same two stack slots
  while treating one as a boolean and the other as a list cursor. This is the
  established per-call-site convention, not a bug in either.
  (`func_8001CBA4`, `func_8001D204`/`func_8001D280`)

#### Round 12 (32 byte-exact matches across four units)

- **Do not hand-decode a magic-multiply divisor — probe `cc1` for it.** Write
  a one-line `int f(int x){return x % N;}`, run it through the pinned pipeline,
  and compare constants. A first hand-decode of `func_80061E60` mis-guessed the
  divisor outright. This is the cheap, reliable half of the already-recorded
  fact that one constant can encode two divisors depending on a post-`mfhi`
  shift. (`func_80061E60` `% 300`, `func_8006204C` `% 30`)
- **A statement sitting in a branch's DELAY SLOT is unconditional** — it is not
  part of the guarded body, and reading it as guarded gets the logic wrong
  *and* changes the function's length, cascading address drift into every later
  function in the unit. Two failures for the price of one, and the second one
  looks like unrelated regressions elsewhere. (`func_80062660`)
- **`cond ? A : B` and `!cond ? B : A` are value-equivalent but not
  byte-equivalent.** When a ternary leaves a same-size residue that looks like
  inverted branch polarity, flipping the ternary is the one-character fix to
  try before reshaping anything. (`func_800624BC`, `== 0 ? -0x100 : 0x100`)
- **A syntactically redundant outer guard is not free — GCC 2.6.3 does not
  eliminate it.** An `if (x != 0)` wrapped around a test that already implies
  it costs real instructions, which means retail's source wrote it out
  explicitly and yours must too. The instinct to simplify it away is what
  leaves the residue. (`func_80063094`)
- **A `bne`-guards-a-store shape reads backwards on a quick skim.** Check the
  equality direction explicitly rather than trusting the first reading; a
  polarity slip here produces a logically-inverted function that still has the
  right instruction count. (`func_8006204C`)
- **Crossjump/tail-merge is sensitive to a vtable slot's DECLARED RETURN TYPE,
  and a LOCAL function-pointer variable is the safe lever.** When two branches
  call differently-typed slots that retail tail-merges into one call site,
  assign both into one local `void (*fn)(...)` (casting at the assignment) and
  call `fn`. This forces the merge **without retyping a shared slot** — which
  is the round-7 `slotC4` hazard, where a retype silently changed an
  already-matched function in another unit. Prefer the local every time; the
  shared retype needs a per-slot check that cannot be reasoned by analogy.
  (`func_80062970`, merging `slot44`/`slot48`)
- **Source statement ORDER decides register allocation for a value that
  crosses two dependent loads.** With an independent store sandwiched between
  them, write the store where retail's delay-slot fill puts it. Assigning the
  local before the independent store cost one extra register move.
  (`func_8003E538`, 11/16 → 16/16 on statement reorder)
- **A sparse `switch` can beat an `if`/`else` chain that is logically
  identical.** GCC 2.6.3 -O2's block placement for a 3-way dispatch was
  reproduced by a `switch` after `if`/`else`/early-return shapes would not
  converge. **This does NOT contradict the dense-switch blocker** — the
  discriminator is DENSITY, not the keyword. `func_8001D6B4`'s switch compiles
  to `slti`/`bnez` compares with ZERO jump tables (verified: no `jlabel`, no
  `%lo(jtbl`), whereas a dense run of cases becomes a jump table and is
  blocked. Screen the result, do not assume from the source form.
  (`func_8001D6B4`)
- **A "struct knowledge established" line naming a slot's OCCUPANT is a claim
  about DATA, and matching does not check it.** A caller that dispatches
  through a slot and discards the result matches byte-exactly regardless of
  which function the slot actually holds — so a wrong occupant name survives
  the only verification the round performs, and gets copied forward. Resolve
  with `tools/classtable.py` before reusing another report's attribution. First
  recorded instance: `Obj86B60Methods::slot118` was attributed to
  `func_8003DFA0` (really at `+0x120`; `+0x118` is `func_8003DE30`) and
  propagated one round. (found while matching `func_8003DFA0`; corrected in
  `func_8003C944.md`)
- **A folded immediate offset and a precomputed pointer at zero offset can
  produce identical ADDRESSES but different INSTRUCTIONS.** When retail's delay
  slot computes an address unconditionally ahead of a branch
  (`addiu $v0, $a0, 0x14`), reproduce that as an explicit pointer local before
  the `if` — not as an inline literal offset inside each arm. "Same effective
  address" is not the test. (`func_80019724`, `func_8001974C`)
- **Re-dereference; do not cache in a named local.** A value read from memory,
  used, and then read again for a second use should be written as two separate
  dereferences. Caching it in a local forces the value to survive across a call
  boundary, which consumes a callee-saved register and can renumber registers
  through the whole function. **This is the primary lever for the
  saturated-callee-saved-file stall class below.** (`func_8003CE98`,
  `func_8003D050`)
- **…but that fix is shape-specific, and applying it to both shapes is wrong.**
  A loop bound that is a cheap single-field read (`self->unk50`) stays uncached
  in the loop condition; a bound reached through an array index
  (`self->unk5C[idx]`) is expensive enough that retail DOES cache it. Two
  superficially identical "hoist the bound?" questions with opposite answers —
  same trap shape as round 7's `slotC4`/`slotCC`. (`func_8003D194`)
- **-O2 tail-merges identical trailing statements out of `if`/`else` arms.**
  Write the repeated statement in EACH arm rather than hoisting it after the
  `if`; GCC produces the single shared block itself, and hoisting it in source
  gives different code. Read together with bravo's crossjump entry above — same
  optimizer, two directions. (`func_8003D194`)
- **A `__asm__("" ::: "memory")` barrier anchors MEMORY operations only.** It
  can force a genuine store-then-reload where the optimizer would otherwise
  forward the value in a register — useful for reproducing a retail double
  store. It does **not** pin a pure register assignment: an `i = 0` with no
  memory side effect has nothing for the barrier to anchor and floats across it
  freely. So a barrier is not a general scheduling lever, and a residue on a
  register-only statement will not yield to one.
  (`func_8003DAD4`, `func_8003D73C`)

#### Round 13 (2026-09-03)

- **`~x + 1` and `-x` are NOT interchangeable.** `-x` compiles to a single
  `negu`. `~x + 1` compiles to `nor $vN, $zero, $rX` followed by
  `addiu $rY, $vN, 1`. So a retail `nor`+`addiu` pair IS the source saying
  `~x + 1`, and no amount of reshaping around a `-x` will reach it.
  Generalises past this one operator: **read retail's CHOICE OF INSTRUCTIONS
  as evidence about the source EXPRESSION**, not only about its control flow.
  A two-instruction encoding of something the compiler can do in one is
  usually the source spelling it out. (`func_8005CBC8` — seven prior attempts
  all used `-sel`, so none of them could reach it.)

- **N independently-incrementing walkers over one array need N differently
  BASED view types.** This SUPERSEDES the round-12 conclusion that GCC
  2.6.3's strength reduction is uncontrollable from source once it can prove
  two accesses share a base+index. It is controllable — but only by changing
  the BASE, never by regrouping the fields. Measured both directions on one
  function: every shape that keeps ONE base collapses to one induction
  variable (`arr[i].fieldA`/`arr[i].fieldB`, an `&arr[i]` element pointer, a
  nested `arr[i].sub.field`, a per-iteration sub-pointer — all identical
  output). The recipe when retail shows N walkers over one array:

  ```c
  /* stride is 0xC; BOTH types are the full stride so a natural ++ works */
  SetupEntry866E8 *ep = arr1;                              /* based at +0 */
  SetupSub866E8   *sp = (SetupSub866E8 *)((u8 *)arr1 + 4); /* based at +4 */
  ...
  ep++; sp++;
  ```

  Keep the second type OUT of the real element struct — embedding it corrupts
  the element struct's size, and that false blocker is what stopped the
  previous attempt. Seed each walker with ONE cast outside the loop; a
  byte-cast `+= stride` INSIDE the loop reaches the same CFG but costs extra
  addressing instructions and scores worse. (`func_8004BB3C`, 14/105 → 90/105
  at correct length; residue then a pure `$s3`/`$s4` identity swap.)

- **Never size a per-function residue with the whole-image byte count when
  the function contains a `switch`.** Compiling the switch from C replaces
  the `.s` file's embedded jump table with GCC's own, relocating rodata and
  shifting the ENTIRE image, so the whole-image count is dominated by drift
  and is not a residue measurement at all. Use asm-differ's
  inserted/deleted instruction markers, or `funcdiff.py`'s in-range word
  score. (`func_8005CBC8` was sized at one word this way and is two.)

- **A preserved body's own "untried direction" note outperforms a fresh
  derivation — INCLUDING when the stated reason it went untried is wrong.**
  `func_8004BB3C` gained 76 words in three builds because the previous author
  wrote down exactly which lever they had not pulled and why they believed
  they could not. The belief (struct-size corruption) was mistaken, and
  reading it carefully is what showed the lever was available. When you stop,
  record the direction you did not take and your reason for not taking it,
  even if the reason feels obvious.

- **The shift amount after `mfhi` fingerprints a division's divisor** faster
  than decoding the magic multiplier constant. (`Entity_e`, several)

- **Hoist a `rand() % K` into a named local before scaling or reusing it.**
  A `(rand() & 1) * K`-shaped expression used inline leaves a
  register-identity residue through the `mult`/`mfhi` expansion; the named
  local matches. Note this cuts the opposite way from the usual
  "drop the intermediate" advice — `func_80061198` needed its intermediate
  local REMOVED for the same idiom. The lever is real in both directions, so
  try both rather than assuming which. (`func_80062570`, `func_80061198`)

- **An unsigned range check needs an EXPLICIT cast to get `sltiu`.** Write
  `(u32)(x - LO) < N`; without the cast you get `slti`. Same word count,
  wrong opcode — so it survives a length check and shows up only as a diff.
  Relatedly, `x == 0` / `!x` on a masked expression lowers differently by
  signedness: only a named **unsigned** local compared with `<` reaches
  `sltiu`. (`func_80062730`, `func_800621A8`)

- **A comparison reachable from two converging branches, with its operand
  re-materialised on one path, is `A && B` short-circuit form**, not nested
  `if`s. (`func_80061C2C`)

- **Reuse one counter for a guard and its loop test.** Retail's paired
  guard-then-loop shape comes from `if (count--) { ... while (count--) ... }`
  on the SAME variable; a fresh `remaining` local does not reproduce it.
  (`func_800183DC`)

- **Two adjacent updates to the same byte are independent statements, each
  reloading from memory.** Do not share a "new flags" temp between them.
  (`func_8001934C`)

- **In a free-list walk, copy the about-to-be-freed value into a temp BEFORE
  advancing the cursor**, not after — that ordering is what matches retail's
  register reuse. Separately, test the advanced pointer directly rather than
  giving it a second `next` name. (`func_80018288`)

- **A wrong GPR clobber on an `lwc2`/`swc2` block cascades.** Naming
  `$2`-`$5` when the instructions target COP2 data registers — a disjoint
  numbering — forces spurious evictions through the whole surrounding
  function's allocation, and presents as a register-identity residue
  somewhere else entirely. See CLAUDE.md HARD RULE 6 for the GTE exception
  this sits inside, and `docs/research/maspsx-noreorder-lead.md` for the
  `.set noreorder` bracket a branch-containing block also needs.
  (`func_800195EC`)

- **`func_8001EACC`'s first two arguments are symmetric and `$a3` names the
  direction.** All 9 asm call sites and all 19 matched C call sites with
  `a3 == 0` pass `(this, this->unk94)`; both `a3 == 1` sites pass
  `(this->unk94, this)`. The caller pre-swaps rather than the callee
  branching. Do NOT "fix" a swapped site back, and do NOT retype the extern
  on the strength of one. Open, deliberately unasserted: if both objects fit
  either slot, the real parameter type is probably a common base shared by
  `Entity` and `Unk94Obj` rather than `Entity *` — resolve with
  `tools/classtable.py` before declaring it.
  (`func_80061778`, `func_80063144`)

  Method note, which is the transferable part: two runners in different units
  flagged the same oddity independently. One sighting reads as a
  transcription error; the second is what justifies a corpus sweep, and the
  sweep is what found the flag correlation neither sighting could see alone.
  **When two runners report the same anomaly, sweep the corpus before writing
  either report up.**

#### Round 14 CORRECTION: the register threshold is 7, not 5

**The round-13 threshold below is wrong and was written by the head that
promoted it. Round 14 more than tripled the sample and it did not survive.**

Pooled over rounds 13 and 14 — 81 matched functions and 10 stalled:

| band | matched | stalled | verdict |
| --- | --- | --- | --- |
| 5-6 registers | **5** | 1 | **83% MATCHED** — not a stall signal at all |
| 7+ registers | **1** | 4 | see the round-16 correction below |

> **ROUND 16 CORRECTION — the 7+ band is no longer 0-matched, and the
> sentence that used to sit here ("No function needing 7 or more distinct
> callee-saved registers has ever matched") is now FALSE.** `func_8004A534`
> (`class_3ac78`) matched **163/163 words with 8 distinct callee-saved
> registers** — `$s0`–`$s7`, saturated bar `$fp`. The head sent that runner
> in *expecting a stall* on the strength of the old wording, told it a
> measured stall report was the deliverable, and independently re-ran the
> census (8) and re-verified the match byte-exact after merging. Corroborated
> the same round by runner delta, which attempted all three of its large
> bodies and reported that **register count was not the blocker in any of the
> three** — the real costs were struct-shape reconstruction,
> induction-variable/multiply behaviour, and switch-vs-if/else. Two of those
> three turned out to need 0 and 1 s-registers anyway.
>
> **Read the count as a CORRELATE, not a cause.** High saturation travels
> with "large function needing deep struct reconstruction", and it is the
> reconstruction that costs. `func_8004A534`'s load-bearing insight was a
> struct-shape one (a whole-struct copy misread as field-by-field), nothing
> to do with register pressure.
>
> Still a real deprioritisation signal at 1-of-5 — **but never a reason to
> skip a function, and never sufficient grounds on its own to stop.** If you
> stop in this band, the report must name a residue, not a register count.
>
> **This is the SECOND time this threshold has been corrected** (round 14
> moved it from 5 to 7), and round 14's own stated lesson is why: *"A
> threshold needs samples on BOTH sides of it before it is a threshold; until
> then it is just the edge of what you have seen."* The 7+ band had four
> stalls and zero attempted-and-matched samples, so it could not distinguish
> "7 is fatal" from "nobody has tried". One deliberate attempt settled it.
> The general rule this keeps re-teaching: **a screen built only from the
> failures it predicted needs a deliberate attempt on its wrong side before
> it earns a number.**

Prior to that correction the reading was that no function needing 7 or more
distinct callee-saved registers had ever matched, across 81 samples. No
function at a full 9 has matched yet either — but that is now a statement
about a handful of samples with one known match at 8 next to them, so do not
lean on it. But the 5-6 band, which round 13 treated as the
danger zone, is where `func_8004EB88` (6), `func_8004F4C8` (6),
`func_8004ED40` (5), `func_8004EDC0` (5) and `func_8004F40C` (5) all matched
— four of them on functions a runner had been told to expect a stall on.

**Why the original number was wrong, since the mistake is repeatable.**
Round 13's sample had 22 matched functions and *none* of them happened to
land at 5 or above. "None of 22 matched needed 5+" is a true statement about
that sample and says almost nothing about the threshold: with a max observed
demand of 4, the data could not distinguish "5 is fatal" from "9 is fatal".
The head read the boundary of the observed range as the boundary of the
possible, promoted it, and then used it to set expectations for a whole
round. **A threshold needs samples on BOTH sides of it before it is a
threshold; until then it is just the edge of what you have seen.**

The screen itself is still good and still one-directional — it is only the
number that moved. Use **7+** to deprioritise. Treat 5-6 as ordinary work.
**And after round 16, "deprioritise" is the whole of it: the 7+ band has a
byte-exact match in it, so ordering work by this screen is fine and declining
to attempt on it is not.**

#### Round 13: retail's callee-saved-register demand is a VALIDATED pre-work screen

Round 12 opened "retail saturates the callee-saved register file" as a residue
class with a screening command. Round 13's echo used it *predictively* --
running it before writing any C -- and correctly called both of its functions'
difficulty class in advance. So the head validated it against the round's
actual outcomes, which is the check that turns a plausible screen into a tool.

Count the DISTINCT callee-saved registers retail saves in the function's
prologue:

```sh
grep -oE 'sw +\$(s[0-7]|fp),' asm/nonmatchings/<unit>/<func>.s | sort -u | wc -l
```

**Mind the register naming, it differs by tool.** `$30` is `$fp` and `$s8` and
the same register. splat's `.s` writes `$fp`; `objdump` renders it `s8`. A
screen written for one and run over the other silently undercounts by exactly
one register --- which is the difference between "8 of 9" and "fully
saturated". The head made this error while validating and echo's figure of 9
was the correct one.

Measured over round 13's 27 worked functions:

| | n | mean regs | max | >= 5 | fully saturated (9) |
| --- | --- | --- | --- | --- | --- |
| matched byte-exact | 22 | **1.86** | 4 | **0** | 0 |
| stalled | 5 | 5.00 | 9 | 2 | 1 |

**It is a ONE-DIRECTIONAL screen and must be used as one.** A high count
predicts a register-allocation stall: not one of 22 matches needed 5 or more,
and both functions demanding 8+ stalled. A LOW count predicts nothing --- three
of the five stalls sat at 2-3 registers and failed for unrelated reasons
(tail-merge granularity, a redundant constant, `nop_mflo_mfhi`). So use it to
DEPRIORITISE and to set expectations, never to promise a match.

This is the first screen that gives `progress.py`'s `fresh` column the thing it
explicitly cannot see --- "large body, deep reconstruction, low cold-runner
yield" (`docs/PARALLEL-RUNS.md`, Gate 1). Run it per unit at triage time to rank
the queue, not just per function.

**Two distinct residue classes live above the threshold, and they do not share
a fix** --- echo established this by hitting both in one pass:

- **Full register-identity PERMUTATION with zero address drift.** The word
  count and total length are exactly right and every instruction matches
  opcode-for-opcode; ~8 simultaneously-live values are shifted across
  `$s0`-`$s7`. `func_8004C93C`: 7 source-shape variants (declaration order,
  boolean-vs-requery, read order, pointer-assignment timing) failed to move
  `self` off `$s3` onto retail's `$s2`.
- **Missing-register / FRAME-SIZE gap.** Retail saves 9 (all of `$s0`-`$s7`
  plus `$fp`); every C shape reaches 8, so the frame is smaller and everything
  after it drifts. `func_8004C6A8` and the pre-existing `func_8004CAF0`.

Three instances now sit in the `class_3bb8c` / `class_3bb8c_b` /
`class_3bb8c_c` header family, which is what makes it a class rather than three
coincidences. **Levers proven elsewhere do not transfer into it** --- Entity_d's
"collapse into one call expression" lever moved `func_8004C6A8` by one word,
not by a register.

Triage consequence, live at the time of writing: `class_3ac78`'s single
remaining fresh function, `func_8004A534`, demands **8** registers. It is the
last item in the reserve pool and it should be handed out with the expectation
of a documented stall, not of a match.

#### Round 13, second batch (later passes)

- **GCC 2.6.3 does NOT cross-jump-merge two syntactically identical
  call+assignment sequences reached from different branches.** Confirmed in
  both directions in one unit. So the reliable lever for "one call reached
  from several guards" is an explicit `goto` to a single physical call site —
  whether the calls are identical or different. Cascading range/threshold
  chains that collapse onto one call site transcribe reliably by matching the
  disassembly's own labels with literal `goto`s.
  (`Entity_g`, several; and see the counter-direction note below.)

  **Read that together with the opposite finding from the same round**, or it
  will mislead: `goto` is *not* a universal fix for 2.6.3 tail-merging.
  `func_8001D714` reproduced a documented `goto` lever bit-identically and
  still did not match. `goto` controls whether there is ONE physical call
  site; it does not control how many trailing bytes the cross-jump pass
  decides to share once there is.

- **A `funcdiff` "differs outside range" warning does not mean "too long".**
  It means "a different length", and the sign is not in the message. Check the
  direction with `objdump` before chasing a fix — a body that is four words
  SHORT and one that is four words long need opposite changes, and the warning
  reads the same for both. (`func_80064E34`.)

- **Retail redundantly re-materializing a literal across several branches
  that reach a shared call is a real shape, and every leaner C form compiles
  SHORTER.** Related to the "redundant move" class, generalised to constant
  materialisation; treat it as a permuter target rather than spending attempts.
  (`func_80064E34`, `func_80063144`.)

- **Branch polarity is a real, separate residue from branch targets.** An
  `if`/`else` pair can carry correct values on both arms and still compile with
  the test inverted relative to retail; the fix is to swap which arm is written
  first and negate the condition. Cheap to try, and it does not perturb
  anything else. (`func_800634A8`, `func_80063874`, `func_8004C93C`.)

- **In a multi-link `bne` chain, a constant sitting next to one branch may
  belong to the NEXT link.** Reading each `ori` as though it were its own
  test's operand is the natural mistake and it silently reassigns whole case
  bodies: in `func_80063874` it swapped which of two bodies belonged to
  `unk44 == 0xC` versus `== 0xD`. Re-trace every link's carried value against
  the raw hex rather than the mnemonic column.

- **Pin a divisor by the reconstruction arithmetic, not by the magic
  constant.** The same magic multiplier is shared across divisors; what
  disambiguates is the `sll`/`subu` chain that rebuilds `N * quotient`.
  `func_800636E4` is `% 15` (`quotient*16 - quotient`), where the constant
  alone suggested the more obvious `% 8`.

- **Dump the shared table BEFORE reading any function in isolation.**
  `tools/classtable.py <addr>`, or a row-alignment check against
  `asm/data/*.s`, resolves a whole unit's slot identities and argument shapes
  in one pass. Independently proposed by two runners in different units this
  round, both after doing it the slow way first.

- **A `jal`'s delay slot carries the value from the PRECEDING call's return,
  not the callee's own.** Misreading this is plausible and produces silently
  wrong field derivations. (`func_8003E628`, caught before committing.)

- **`arg->methods->header & 0xF` — a double dereference through a method
  table's offset-0 word — is a runtime type ID, not a struct field.** Learn the
  shape on sight; it is how this game's hand-rolled class framework does
  dispatch-on-concrete-type.

- **An "unused-looking" local can still need real stack space** sized to match
  retail's frame, independently of what the function reads back from it. Two
  instances in one unit. (`func_8001D568`, `func_8001D714`.)

- **An all-`s16` struct whole-assignment reliably reproduces retail's
  `lwl`/`lwr` codegen** — now the third and fourth confirming instances,
  including one where the head used it to replace a whole-function `__asm__`
  transcription with six lines of C (`func_8001A3EC`). Reach for it whenever
  retail shows unaligned load/store pairs over a small fixed-size payload.

- **A 24-bit BITFIELD write is how the Psy-Q OT linked-list splice is
  spelled, and hand-written masks are not equivalent.** Assigning
  `unsigned addr : 24` (Psy-Q `P_TAG`, via `setaddr`/`getaddr`/`addPrim`) is a
  read-modify-write that GCC 2.6.3 emits as `& 0xFF000000`, `& 0x00FFFFFF`,
  `or`. Writing those masks by hand produces the identical VALUE with
  different register allocation, and the difference presents as an
  unreachable register-identity residue. Note `include/psyq/LIBGPU.H` does not
  compile standalone under this toolchain (it needs the `LIBGTE`/`RECT`
  chain), so declare a minimal local view of the tag instead — `OtTag` in
  `include/code_8220.h` is the worked example. (`func_800197C4` and its seven
  siblings.)

- **A macro's argument is evaluated once per expansion, and that is visible in
  the bytes.** `addPrim(ot, p)` expands to
  `setaddr(p, getaddr(ot)), setaddr(ot, p)`, so `ot` is loaded TWICE. Caching
  it in a local is two instructions short. This is the same family as the
  existing "do not cache a `this->field` across an intervening vtable call"
  rule, one step further out: here there is no call at all.

- **GCC 2.6.3 hoists an EXISTING instruction into a load-delay slot; it never
  invents one.** So a filler instruction whose result looks dead is evidence
  that the real source computes that value somewhere. Do not write it off as a
  "dead `addiu`" — it is a lead about the source. (`func_800197C4`.)

- **A whole-function `__asm__` is for constructs with NO C form, not for
  constructs that are hard to type.** See CLAUDE.md HARD RULE 6. GTE
  `rtpt`/`nclip`/`cfc2` and COP2 `swc2`/`lwc2` qualify; an awkward unaligned
  struct copy does not, and one was reworked into six lines of C this round
  after being matched as a transcription. If you cannot name the instruction
  that has no C spelling, it is not the exception.

- **Two register-saturation residues that look alike need opposite fixes**, and
  neither responds to the other's: a full register-identity PERMUTATION at
  zero address drift (word count and length exactly right, ~8 live values
  shifted across `$s0`-`$s7`) versus a missing-register FRAME-SIZE gap (retail
  saves 9 including `$fp`, every C form reaches 8). Three instances in the
  `class_3bb8c` header family. Levers proven elsewhere do not transfer in:
  the "collapse into one call expression" lever moved `func_8004C6A8` by one
  word, not by a register.

- **Declaration-order and indirection tricks for steering register mapping are
  non-monotonic and function-specific.** Two register-identity stalls in one
  unit contradict each other as a predictive rule, and a third elsewhere in
  the round found that the same trick helped in one direction and hurt in the
  other. There is no general rule here; try both directions and keep the
  measurement, do not reason from a sibling.

#### Round 13, final batch

- **A field can be read SIGNED at its writer and UNSIGNED at a different
  reader, and both are right.** Two instances in one unit (`unk5B` via
  `func_8003EEC0`, `unk58` via `func_8003F04C`). Do not "fix" one reader to
  agree with the other; type the field where it is written and cast at the
  reader that disagrees.

- **The range-check fold's desirability is READ OFF THE DISASSEMBLY, per
  function, never assumed.** `(unsigned)(x - LO) < N` needed an explicit cast
  to reach `sltiu` in one unit this round, and in another unit a natural
  `||` chain reproduced the fold correctly and the explicit form was wrong.
  Two units, opposite answers, same round. There is no default here.

- **A delay-slot store can be UNCONDITIONAL even though it sits inside what
  reads as a guarded assignment.** Check whether the store is in a branch's
  delay slot before concluding the source guards it. (`func_8003F1A8`.)

- **Watch for a "leftover register" implicit argument to a vtable slot** — a
  register still live from earlier code that the callee reads, with nothing
  at the call site setting it. It looks like a 3-argument call with a garbage
  third argument. (`func_8003EEC0`'s `slotA0`.)

- **`x / N` and `x >> log2(N)` are not interchangeable even for power-of-two
  `N`.** Signed division rounds toward zero and needs the sign-fix chain; a
  shift floors. If retail has the sign-fix, the source said `/`.

- **A delay-slot filler whose value looks dead is CODE MOTION of a real
  computation, not an arithmetic artifact.** Confirmed on the `func_800197C4`
  family: the filler's value is an address the OTHER branch genuinely
  computes. So do not try to reconstruct the NUMBER — reconstruct a source
  expression that makes that value live at the branch. This is the
  operational form of "2.6.3 hoists an existing instruction into a
  load-delay slot; it never invents one".

- **Count trivial functions separately from real ones when reporting.** A
  freshly carved unit can be a third `jr $ra; nop` setters and getters, and
  splat generates some of them as C itself. 14 of one unit's 26 matches this
  round were one-liners. They are real matches and they are not real work;
  folding them into a headline number misrepresents both the round and the
  unit's remaining difficulty. Size a carve by NON-TRIVIAL count.

#### Round 14

- **A leftover argument register is not always a real argument — and you can
  act on that WITHOUT retyping the shared slot.** Where a vtable slot's
  canonical type has N arguments but one call site genuinely passes N-1 (the
  extra register being untouched leftover from the incoming parameter), cast
  the slot down to a narrower function-pointer type **at that one call site**
  and leave the struct field alone for the callers that do use all N. This
  closed `func_80040948` exactly, and it is strictly better than the
  alternatives: retyping the field breaks the other callers, and leaving the
  extra argument in emits a redundant `move`.

- **Two entangled residues can each be WORSE alone and correct together.**
  `func_80040D74` went 36/40 → 38/40 only when a pre-loop operand order that
  measurably regressed the score on its own was combined with a specific
  named-temp split of the in-loop recompute. If two candidate levers each
  make things worse, that is not proof either is wrong — try them together
  before writing both off.

- **A `switch` and its "equivalent" `if`/`else if` chain differ in PHYSICAL
  LAYOUT, not just comparison order** — inline versus out-of-line case bodies
  — and they diverge even for two values. (`func_800504D0`.)

- **Splitting a fused boolean condition into separate `if`-`goto` statements
  can defeat a GCC cross-jump merge.** Useful against a shared-tail residue.
  Conversely, a crossjump-mergeable tail needs the full call statement
  written out in each branch rather than deferred through a function-pointer
  local, when the branches share one slot with different arguments.
  (`func_8004E6B8`, `func_80050280`.)

- **Retyping an already-matched `void`-shaped function to a real return type
  by ADDING explicit `return` statements can grow its compiled size** (21 →
  23 words) even though the change is semantically a no-op. Retype the
  DECLARATION and leave the body's implicit-`$v0` shape alone.
  (`func_8004E77C`.)

- **The `goto fail;` idiom is REQUIRED, not stylistic, for a `New_X`
  allocator whose null and success paths share one return variable.** A bare
  early `return NULL;` duplicates the epilogue and regressed a 26/27
  near-miss to 16/27. (`func_8004E2E0`.)

- **A combined `&&` and the equivalent nested `if` are not interchangeable at
  `-O2`, and the direction is not fixed.** In `func_8004E230` the combined
  form compiled SHORTER than retail — the reverse of the usual "simplifying a
  guard costs instructions" case. Try both.

- **A do-while with a post-decrement loop condition, and reusing a callee's
  own return value instead of re-deriving it**, both closed
  register-heavy functions this round. So did computing seek-offset
  arithmetic as one whole expression rather than transcribing the
  disassembly's around-a-call interleaving. (`func_8004ED40`,
  `func_8004EDC0`.)

- **Finding an unknown method table: grep the retail binary for the raw
  pointer values of the functions you already have.** Echo found
  `D_80086E00`, a 29-slot table unrelated to anything known, by searching for
  its unit's own function addresses — each appeared exactly once, in one
  contiguous run. That is a table, and its extent falls out of the same scan.

- **When other runners' branches are unmerged, suffix new type names with
  your unit.** Echo named everything `_3bb8c_g`, class name included,
  specifically because two other runners were live on the same header and
  might name the same class. It costs nothing and removes a whole category of
  merge adjudication. Adopt it whenever more than one runner shares a header.

- **A vtable slot genuinely called at several arities can be declared
  unprototyped (K&R, `void (*slot)()`)** — a legitimate escape hatch in this
  codebase, since the class framework does reuse slots. But it silently
  disables argument checking for every other caller in that header, so it is
  a last resort, it belongs only in a header no other live runner is editing
  if that can be arranged, and it should be narrowed the moment one
  consistent signature is established. (`code_2cc8c_f`'s `slot4C`/`slotC4`,
  re-checked and confirmed genuinely multi-arity.)

#### Round 15 (2026-09-04, five runners on five class_3bb8c slices)

**Branch shape and polarity — the round's densest cluster, four runners
independently.**

- **GCC 2.6.3's `if`/`else` codegen is MECHANICAL, so solve for the written
  condition instead of guessing from semantics.** The compiled branch test is
  always `NOT(the condition you wrote)`; the `if`-body always lands at the
  fallthrough and the `else`-body at the branch target. So when retail puts a
  particular block at the branch target, that tells you the polarity of the
  source condition directly. Confirmed 3x in one unit (`func_80052E7C`,
  `func_800531CC`, `func_80053358`), and it turns polarity from guesswork into
  arithmetic.

- **A redundant-looking condition can be LOAD-BEARING; measure before
  simplifying.** `func_80053F84` matches byte-exact on
  `if (x != 5 && x != 8 && x == 0xA)`, where the first two conjuncts are
  provably dead — `x == 0xA` implies both. The head hypothesised the real
  source was a sparse `switch (x) { case 5: case 8: break; case 0xA: ... }`,
  which would explain the three compares honestly, and **measured it: whole
  image red, 109627 bytes different.** GCC 2.6.3 emits the three sequential
  compares (all branching to one target) from the short-circuit `&&` chain
  and something structurally different from the switch. Do not "clean up" a
  condition like this.

- **For a genuine multi-way (>2 arm) dispatch, transcribe retail's CFG
  literally with `goto`/labels in its own physical block order.** Both
  `func_80053358` and `func_800521D4` needed this after nested `if`/`else if`
  picked the wrong shape and would not converge. This generalises the
  project's existing `goto fail;` idiom: the labels are not a hack, they are
  how you express a block order the compiler will not otherwise choose.

- **A two-armed dispatch on ONE scrutinee, where each arm does unrelated
  work, is a `switch` and not two independent `if`s.** Two separate `if`s make
  GCC re-materialize and re-compare the scrutinee, costing an instruction and
  cascading address drift (`func_80053F84`, first pass). Note this sits
  *beside* the round-14 finding that a `switch` and its equivalent `if`/`else
  if` chain differ in physical layout — neither form is "the" answer; they are
  two distinct levers and the disassembly says which.

- **`if`/`else-if` and a `return`-terminated sequential-`if` compile
  IDENTICALLY, so switching between them is not a lever — but a `switch` is.**
  Useful negative result: it stops you spending attempts on a reshape that
  cannot move a single byte (`func_800512C8`).

- **`if`/`else` ARM ORDER is load-bearing independently of logical polarity.**
  Which arm is the fallthrough and which is the branch target matters even
  when you have the condition right (`func_80051784`, call-as-fallthrough vs
  reset-as-branch-target).

**Register identity — three new levers, and all three are ordinary C.**

These matter because a register-identity residue is otherwise a STALL by
project rule. Try all three before filing one.

- **A redundant `local2 = local1;` double-assignment can be load-bearing for
  register allocation, with no UB involved.** Confirmed 3x in one unit
  (`func_80052430`, `func_800524F8`, `func_80052598`). **Check this before
  accepting a `$v0`/`$v1` swap as a stall.**

- **Mutating a parameter in place (`a3 -= a1;`) rather than introducing a
  fresh local** is a legitimate fix for a register-identity residue around a
  call argument (`func_800529FC` — which a ~6500-iteration permuter run had
  failed to close).

- **Hoisting an unconditional store to BEFORE an unrelated loop guard** closes
  the documented "redundant move, resists everything" class, which had been
  recorded as source-unfixable. `func_80051858`: manual attempts (barrier,
  explicit re-mention) failed or made it worse; the permuter found the hoist in
  14 iterations. **The class has a source-level fix in at least one case** —
  do not treat it as permuter-only.

- **Conversely, a deferred-call pattern that matched in one function is NOT
  safe to reuse by analogy in a sibling** with different register pressure — it
  can swap register identity between two variables (`func_80053458`). Same
  shape as round 7's two-symmetric-slots lesson: check per site.

**Return types, from the opposite side of the standing rule.**

- **A multi-exit function that stores a literal into a field and returns that
  same literal right after a `jalr` CANNOT be typed `s32`, however you phrase
  it — try `void`.** `func_800542D0`, confirmed with **18 isolated reproducers**
  through the pinned pipeline, none matching: keeping `$v0` consistent across
  the multi-exit merge evicts the store's operand to `$v1`, costing one real
  instruction. Typed `void`, there is no caller-visible return register and the
  eviction never happens.

  Read this together with the standing runner rule ("a `void` wrapper around
  an `s32` tail call is byte-identical, so write `return callee(...)` absent
  evidence"). Both say the same thing: **the bytes do not pin the return type.**
  The rule is not "prefer `s32`" — here `s32` was unreachable and `void` was
  the answer.

**Struct and vtable discipline.**

- **A vtable slot that resolves by address to an already-named C function must
  STILL be called through the struct's function-pointer field**, never by that
  name — `jal` and `jalr` are different bytes. Use the name only to cross-check
  identity and signature (`func_80053C94`).

- **Do not cache `self->methods` in a local unless retail's own disassembly
  shows one load reused.** Writing the cache when retail reloads (or vice
  versa) is a whole-instruction difference.

  **Round 23 adds that the answer tends to be constant PER CLASS, which makes
  it worth checking once per unit rather than once per function.** In
  `class_3bb8c_g`'s class (`D_80086DC4`, 44 slots — verified with
  `tools/classtable.py --scan`) retail loads the vtable pointer ONCE into a
  callee-saved register before every run of `->slotXX` calls, and echo needed
  the cached-local form to close both `func_8004FE24` (71/71) and
  `func_8004FBE4` (144/144). The head's `func_8003C48C`/`func_8003C63C` in
  `Obj86B60` behaved the same way, matching the idiom the already-matched
  sibling `func_8003C51C` had established. **So determine it once from any
  already-matched function in the same class and carry it across the unit** —
  but the rule stays conditional, because it is retail's disassembly that
  decides, not the class's reputation.

- **A call result that is BOTH stored into a struct field and reused later
  needs a named local** (round 23, echo, `func_8004FE24`). Writing
  `self->field = f(); ... use(self->field);` makes GCC re-read the field from
  memory, where retail keeps the value register-resident across both uses.
  This is the mirror image of the no-cache rule directly above, and the two
  are not in tension: do not cache a field you only READ, and do cache a value
  you WROTE and then use again.

- **A `New_X` allocator's null check must use the proven idiom verbatim** —
  `if (self) { ctor(...); return self; } return NULL;`. Two plausible
  rephrasings compile to non-matching shapes (`func_80050BA8`). Relatedly, the
  `goto`/`return`-with-a-different-value lever applies to pointer-vs-NULL early
  exits, not only integer constants (`func_80051A5C`).

- **A delay-slot store is not necessarily conditional.** Reading one as part of
  the branch it follows produces a wrong CFG that then resists every reshape.

- **Compile immediately after drafting any struct with more than two fields.**
  A missing `u8 padNN[...]` right after the first pointer field is easy to miss
  because the struct still "looks" right, and it presents only as a whole-image
  SHA1 failure — see the shared-struct hazard above.

**Permuter craft.**

- **A permuter zero reached via UB can have an unrelated, purely idiomatic fix
  buried in the same diff — isolate and test each part.** `func_80052430`'s
  permuter zero carried a `(float)1` cast that was a red herring; the actual
  lever was the double-assignment above, which is ordinary C. Do not discard a
  UB-tainted zero without decomposing it, and do not adopt the UB either.

- **A permuter lead that scores well against the permuter's own stripped
  scaffold does not always transfer to the real build.** `func_80051F24`'s
  score-30 lead did not survive translation back. Run `--debug` on the
  CANDIDATE, not only on the base, before trusting a promising non-zero score
  — otherwise you spend the translation effort to discover the scaffold was
  what made it work.

- **A `do { ... } while (0)` wrapper around an otherwise-unconditional body can
  be load-bearing for delay-slot scheduling.** `func_80052110`: the identical
  statements scored differently inside the wrapper versus outside it,
  confirmed by an isolated A/B test. **Mechanism unexplained** — recorded as a
  measured lever, not as understood behaviour. Worth trying against a
  delay-slot residue; worth investigating if it recurs.

**Proposed refinement to the register-saturation screen (one instance, NOT yet
validated).** The existing validated predictor is a *saturated* callee-saved
demand (8-9 of 9) — see the round-13/14 threshold note above, which corrected
5 to 7. `func_80051AC8` suggests the risk zone is wider: **6-7 of 9 s-registers
combined with several independent long-lived values** also produced an
intractable register-identity rotation. Treat this as a hypothesis with a
single data point. It needs a corpus census over already-matched functions in
that band before it becomes a screen, exactly as the 7-not-5 correction did —
if many matched functions sit at 6-7 with long-lived values, the refinement is
wrong and the distinguishing factor is elsewhere.

#### Round 16 (2026-09-04, five runners; two class_3bb8c slices plus three fresh code_179d8 carves)

**Confirmed, each backed by a byte-exact match:**

- **The two-exit allocator shape, now confirmed TWICE across unrelated
  blocks.** For "allocate; construct on success; return NULL on failure", put
  the success-path `return` INSIDE the `if`-body and the failure
  `return NULL;` as the trailing unconditional statement — not an early-return
  guard, not a single shared result variable. Only that shape folds into one
  branch with no extra jump. Established on `func_80052B70`
  (`class_3bb8c_k`), then reused verbatim on `new_class_6d940`
  (`code_179d8_d`, a different block entirely) and matched **first try**.
- **GCC 2.6.3 does not dedupe an explicit `if` guard against a `for`/`while`
  loop's own implicit entry test**, even when the two conditions are provably
  identical — you get a redundant duplicate bounds check retail does not have.
  Recurred twice in one round in unrelated units (`func_8005281C`,
  `func_8002C014`). Its inverse is also a real fix: where retail HAS only one
  check, `guard + do-while` is what expresses that (`func_800323A8`, verified
  with a standalone toolchain reproducer).
- **A loop-carried multiplicand must be recomputed from the loop counter, not
  accumulated with `+=`.** cc1 strength-reduces a constant multiply over an
  induction variable, so a `+=`-accumulated value produces the wrong shape;
  recomputing it by multiplication each iteration reproduces retail.
  (`func_800323A8`, verified with a standalone reproducer.)
- **A genuine C `switch` is not interchangeable with an if/else chain**, and
  the difference is worth attempts: switching `func_80032708` from if/else to
  a real `switch` moved it 24 -> 65/164. This is a DISTINCT finding from the
  block-order lever below, confirmed separately.

**Framed wrappers — promoted, with the negative check that earns it.**

- **A framed wrapper is not evidence against tail-call elimination on this
  target.** A function that allocates and restores a stack frame, saves and
  restores `$ra`, calls exactly one function and never touches `$v0`
  afterwards is still just `return callee(fwd-args);` — GCC 2.6.3 here always
  pays for the frame. Read it that way and type it from whether the callee
  sets `$v0`, not from the presence of a frame.

  **Promoted because the negative was checked, on request:** every
  single-call forwarding wrapper across two passes of `code_179d8_b`
  (`func_80028D68`, `func_80028D88`, `func_80029234`, `func_80029254`) pays
  for a full frame, and no `.s` sibling in the unit shows a bare `jal` plus
  immediate return with no `addiu $sp`/`sw $ra`. Contrast the entry below,
  which was NOT promoted for want of exactly this.

**NOT promoted — two patterns at one or two sightings, recorded so the next
round can sweep rather than re-derive.** Both runners were asked for a
negative check and both answered honestly that they had none, which is why
these are here and not above.

- **An unexplained register near a call may mean a vtable slot's arity is
  short by one.** Two positive sightings (`slot80`, `slotA8` in
  `class_3bb8c_i`), zero negative checks — nobody looked for an untouched
  register that was *not* a forwarded argument. A near-cousin found the same
  round sharpens what the tell actually is: `new_class_6d940`'s
  untouched-looking `$a0` was a fresh `move a0, zero` for an explicit literal
  `0` argument, not a forwarded value. So the real signal is **unexplained
  register activity near a call**, and "forwarded argument" is only its most
  common cause. Needs a corpus sweep before it is a rule.
- **`$a1` set by its own `lui` before its only read means scratch, not a
  forwarded argument** (`func_8002B198`, giving a genuine arity of 1 where
  the call site looked like 2). One sighting, explicitly re-checked in a
  second pass with no further instance found. An oddity worth a sweep, not a
  rule.

**The block-order lever, WITH the boundary that a checked negative put on it
— read both halves.**

- **Where it works:** when a raw, uncompared value feeds `beqz`/`bnez`
  directly, the `==0` vs `!=0` spelling controls which block GCC places INLINE
  (fallthrough) and which OUT-OF-LINE (jumped to), not merely the polarity.
  Matched `func_80032AD0` on a one-flip change, and generalised correctly to a
  full two-sided `if`/`else` in `func_800329D8` (41/41).
- **Where it does NOT: guard-clause range checks.** The head hypothesised the
  same lever would move two stalls written `if (idx >= 3) return 0;` and asked
  for the flip to be tried on both. **It regressed both** — `func_80032BB8`
  gained a wrong branch direction and a duplicated inline block on top of its
  register residue, and `func_80032C60` lost its clean one-instruction
  residue for the same regression. So the lever is scoped to raw truthy/flag
  values reaching a branch directly; for a guard-clause range check,
  negative-first is already what retail's compiled form is.

  **This entry exists in this shape deliberately.** The negative was
  explicitly requested and explicitly reported, and it is what stops the
  positive half from being over-generalised into "always try flipping the
  guard" — which is what would have happened had only the two matches been
  written up. **Ask runners for the negative answer; it is cheap, and it is
  what turns a lever into a scoped lever.**

**Method lessons about ATTEMPT BREADTH — the round's most transferable
finding, seen in two runners independently.**

- **A long attempt list is not the same as a broad one.** Two stalls this
  round were argued with impressive attempt counts that all varied ONE axis
  and left the deciding axis untested. `func_80032BB8`: seven reshapes, all
  varying the *expression* form (array index vs pointer arithmetic, temp vs no
  temp, declaration split, parameter type) — none changed the CONTROL FLOW.
  `func_8002C048`: twenty-five variations, all keeping the cached `c1`/`c2`
  locals — none tested whether those locals should exist at all, which is what
  the documented no-cache idiom (see "Do not cache a `this->field`…" above)
  points straight at. **Before filing a stall, list the axes you varied, not
  the number of attempts.** If one axis has all the entries, that is the tell.
- **A `volatile` cast is a codegen lever, not a statement about the program.**
  It "worked" on `func_8002C048` (fixing a CSE'd-away reload) and paid for it
  with a defensive `andi` re-mask, because a volatile-qualified load's value
  is not trusted already-zero-extended for the following comparison, whereas
  every plain `lbu` in the same function is. Two consequences: the mask is a
  *symptom* of the lever, not an independent residue; and a preserved body
  needing `volatile` would put source in the tree that misrepresents what the
  game did, so prefer removing the thing being CSE'd.

  **ROUND 23 CARVES OUT THE ONE CASE WHERE `volatile` IS NOT A LEVER BUT THE
  CORRECT DECLARATION: a genuine hardware register.** Runner bravo matched
  `func_80032BF0` and `func_80032C28` (14/14 each, `code_179d8_c`) — an
  IRQ-mask set/clear pair over the `D_8006DCAC`/`D_8006DCB4` shadow
  `I_STAT`/`I_MASK` globals — and `volatile` on those struct fields is what
  closed them. Without it GCC hoisted the mask store into the `jr $ra` delay
  slot, a one-word drift that misaligned everything after it.

  **The discriminator is EVIDENCE that the location is memory-mapped I/O, not
  whether `volatile` helps.** Bravo got this right in both directions in one
  round: it declared the IRQ pair `volatile` (a documented PSX register pair,
  written then read back), and it explicitly **declined** to declare
  `D_8009024C` `volatile` in `func_80032588` — where it also would have been a
  candidate — on the grounds that nothing shows that global is MMIO. It
  measured the negative too: `volatile` there changed nothing.

  So the rule is not "avoid `volatile`", it is: **`volatile` because the
  hardware says so is source that describes the game; `volatile` because the
  diff says so is a lever, and the existing caution above applies to that one
  only.** Delta reached the same place from the other side the same round
  ("a write-then-immediate-readback of the same global needs that global
  `volatile`, or GCC's CSE elides the reload") — two independent
  confirmations, and the shape to look for is a store followed by a load of
  the same address with no intervening call.

- **NEW, round 17, reproduced in isolation: GCC 2.6.3 `-O2` merges
  per-branch constant stores to one global into a SINGLE shared store.**
  Each arm materialises its constant into a register and one `lui`/`sw` pair
  follows the join; retail sometimes keeps a store in each arm instead.
  Reproducer, straight through the pinned pipeline:

  ```c
  extern int g;
  void probe(int x) { if (x > 0) { g = 1; } else { g = 2; } }
  ```

  ```
  bgtz  a0, .L      li v0,1      li v0,2      lui at,0x0     sw v0,0(at)
  ```

  The three-way form (`if`/`else if`/`else`) merges the same way. Levers that
  moved it in real functions: a bare `__asm__("")` barrier, and a local
  `volatile T *` forcing unfolded addressing — but **the pointer lever
  backfires inside a loop** over a loop-invariant target, where it defeats to
  loop-invariant code motion instead. (runner delta, `func_8002ADE8`,
  `func_8002B4D4`; head-verified with the reproducer above.)
- **A repeated `x & IMM` is CSE'd on the VALUE, not on the immediate**, and
  the distinction matters because the wrong reading sends you hunting for an
  immediate-matching pass that does not exist. Measured: two `x & 0xFF` on
  the same `x` produce ONE `andi` reused; `x & 0xFF` and `y & 0xFF` produce
  TWO. It is ordinary common-subexpression elimination. When retail shows two
  independent `andi` with equal immediates, the question is what makes the
  two operands different, not how to defeat an immediate-matcher. (runner
  charlie, `func_80035F3C` — the fix there was real and permuter-found; this
  corrects the mechanism it was filed under.)

### Proposed learnings that did NOT earn promotion (round 17)

Recorded because an unpromoted claim that leaves no trace gets re-derived,
and because both failures share one shape: **a claim of "unconditional"
compiler behaviour that was only ever tested along one axis.**

- **"A `for`-header increment is not interchangeable with the same statement
  as the body's second line; GCC ties the header form to the loop tail."**
  Does not reproduce. Three spellings — increment in the `for` header,
  increment as the body's last line, increment as the body's second line —
  emit **byte-identical** code through the pinned pipeline. Whatever moved in
  the originating function had another cause. Note this does NOT contradict
  the confirmed multi-variable entry above (`a++, b++` vs `b++, a++` inside
  ONE header clause), which is a different question and still holds.
- **"GCC unconditionally folds `x - (x/N or x>>k)*N` into `x & (N-1)` once it
  can prove `x >= 0`."** False. Seven probes supported it, all varying `/` vs
  `>>`, guard vs no guard, and barrier vs none — while holding the
  intermediate's TYPE fixed at `s32`. Type is the axis that matters:
  narrowing the multiplicand (`lo = index - (short)hi * 16;`) blocks the fold
  and reproduces retail's whole sequence bar one operand. See the head
  adjudication in `docs/match-reports/func_8002CA3C.md`.

  The transferable rule, and the reason both of these are worth the space:
  **retail came out of THIS compiler, so "no C reaches these bytes" is a
  claim about the entire shape space, not about the axis you happened to
  vary.** It earns the same standard CLAUDE.md already sets for the
  no-C-form exception — name the construct that has no C spelling, or it is
  not that exception. A blocker filed on one axis becomes a stub, and a stub
  removes the function from `fresh` permanently.

### New residue classes opened this round (not yet closed)

- **NEW, round 16: the "retry-loop driver" cluster — three instances of ONE
  shape, stalled on two cross-confirmed residues.** `func_80028DF0`,
  `func_80028F38` and `func_80029074` (`code_179d8_b`) are the same
  retry-loop driver, fully reverse-engineered (control flow, the
  `D_8006D5FC` save/restore bracket, tag/flag semantics, all confirmed
  against m2c and the raw bytes) and stalled at 10/82, 1/79 and 55/85 — the
  last with an EXACT length match. About 90 manual attempts plus ~70k
  permuter iterations across three seeded searches went into them. **Neither
  residue is register count** — the runner was asked directly and answered
  that the 7+ screen did not influence where it stopped.

  1. **A status-value fold.** A local set to `0`, reassigned `-1` in the
     loop-exhausted tail, returned as `value + 1`. Retail keeps it in a real
     callee-saved register across the whole loop; **a from-scratch reproducer
     of the identical shape through the pinned pipeline has GCC 2.6.3 fold it
     away entirely.** That the reproducer was built and does not reproduce
     retail's shape is itself the finding — it says the divergence is not a
     toolchain defect to escalate but something contextual still missing from
     the C. Open question for whoever picks this up.
  2. **A prologue register-mapping puzzle, in all three.** Retail processes
     `arg1` into the lowest persistent register *before* `arg0`, even though
     `arg0` is referenced first in every natural C reading. Six
     variable-ordering experiments each produced a *different* wrong mapping
     and never retail's; the full table is in `func_80029074`'s report.

  The cluster is the useful artifact: three instances of one shape with the
  same two residues is what turns "a hard function" into a named class, and
  it is why a zero-match pass that files reports is not a wasted pass.

- **NEW, round 16: commutative-operand SLOT order in `addu`, not reachable
  from C.** In `func_800323A8` every one of sixteen field accesses has retail
  encoding `addu $v0,$a0,$v0` where the build encodes `addu $v0,$v0,$a0` —
  same two registers, same values, same order, confirmed identical at every
  instance, so this is **not** the register-identity class. It is purely which
  operand slot (`rs` vs `rt`) the encoder picked for a commutative add.
  Rewriting all 15 source sites from `(u8 *)(*rowPtr) + colOff` to
  `colOff + (u8 *)(*rowPtr)` produced a **byte-identical build**: cc1
  canonicalises pointer+integer addition to a fixed internal order regardless
  of source order. This is the documented "prologue callee-save stores in the
  wrong order — not reachable from C" class in MATCHING-GUIDE, generalised
  from a register save to a commutative pointer addition. **Not a toolchain
  escalation:** no flag is implicated, and the 15-site mechanical swap is the
  isolation experiment that shows source cannot reach it.

- **NEW, round 15: "PURE register rotation" — a residue with no
  instruction-level difference at all.** `func_80051F24` stalled at 75/95 with
  the permuter's `--debug` reporting **0 reorderings, 0 insertions, 0
  deletions — only register differences.** `self` and `arg1` already sit
  exactly where retail puts them; three *locally introduced* values (two
  global addresses and one handle) rotate among themselves. `func_80051AC8`
  is the same class at 9/107, where the correct total LENGTH was reached but
  every reshape rotated differently.

  This is worth its own name because of what it rules out. There is nothing to
  reorder, nothing missing and nothing extra, so every lever that works by
  changing instruction selection or scheduling is inapplicable by
  construction — including the `__asm__("")` barrier, which only reorders. The
  three register-identity levers found this round (redundant
  double-assignment, in-place parameter mutation, hoisting a store above a
  loop guard) are the candidate set, and all three were tried on
  `func_80051AC8` without success.

  Diagnostic, and it is cheap: run the permuter with `--debug` and read
  whether the diff is register-only. **A register-only diff is a STALL under
  the project's own rules** (fixing register identity with
  `register T v asm("$N")` or an operand constraint is banned), so
  establishing that early is what stops the attempt budget being spent on
  reshapes that cannot possibly move it.

- **NEW, round 12: "retail saturates the callee-saved register file."**
  Diagnosable in one command *before* spending an attempt budget:

  ```sh
  grep -oE 'sw +\$s[0-9]' asm/nonmatchings/<unit>/<func>.s | sort -u | wc -l
  ```

  **A count of 8 means retail uses all of `$s0..$s7` and has NO spare
  callee-saved register.** Any C shape that needs a 9th value live across a
  call must spill into `$s8`/`$fp`; the epilogue then restores one register too
  many and every register number downstream shifts. The result *looks*
  catastrophic and is not: exact total length, zero inserted/deleted
  instructions, purely `r`-tagged renames — `func_8003D73C` scored **40/145**
  in exactly this state.

  **Why it matters that this is its own class:** it is NOT the unfixable
  register-identity stall it resembles, and it has a specific permitted lever —
  **reduce the number of values live across each call** (re-dereference instead
  of caching in a local; narrowly scope post-loop re-derivations). Target a
  budget of 8; do not chase the register numbers.

  It is also NOT the scheduling class. `func_8003DAD4`, the sibling stall filed
  the same round and initially reported as "the same shift", saves only 6
  callee-saved registers with two s-regs and `$fp` spare — no pressure at all,
  and its 114/118 residue is delay-slot placement plus one temp choice.
  Its fixes were tried on `func_8003D73C` and did not transfer, which is the
  expected result once the census is run. **Run the census before assuming two
  register-flavoured stalls are the same class.**
  (`func_8003D73C` 40/145; contrast `func_8003DAD4` 114/118)

- **"Identical assignment reaching different merge points."** GCC tail-merges
  two identical `doDetach = 1;` statements that retail keeps separate.
  Unreached by reshaping, by `__asm__("")` barriers, or by the argument-register
  test. (`func_8005DBF0`, 72/74 — that round's closest near-miss)

  **UPDATE (round 11): this class is reachable, and the lever is not the one
  it looks like.** In `func_80060148` a bare `__asm__("")` placed in one of
  the two merge candidates DID block the merge and reproduce retail's two
  separate blocks (86/89 the moment it was added) — the first confirmed
  instance of this class yielding to anything. But the function did not match
  until a separate residue was fixed: retail computed a table address BEFORE
  an adjacent store, where the natural narrative order writes the store first.
  With the statements in retail's order, **GCC stopped performing the merge on
  its own and the barrier was no longer needed** — it was removed and 89/89
  reconfirmed on a clean rebuild.

  So the merge was a SYMPTOM of wrong statement order, not an independent
  optimiser quirk to be suppressed. When retrying `func_8005DBF0`, hunt the
  statement-order cause first; reach for the barrier only as a diagnostic to
  confirm the merge is what you are fighting. **And re-test removing any
  barrier after fixing any other residue in the same function** — a later fix
  can make an earlier barrier redundant, and the version without it is the one
  to commit. (`func_80060148`, 89/89)
- **Magic-multiply constant load POSITION.** A GCC-synthesized multiplier
  constant whose load placement has no direct C-source counterpart; two
  symmetric 3-word clusters, unmoved by six reshapes and by a late barrier
  (which made it worse). Flagged as a candidate "not a source-shape question"
  case. (`func_80066340`, 252/258)
- **Register-identity EXCHANGE of two parameters, driven by reference-count
  priority.** Retail assigns `a0->$s1, a1->$s2, a2->$s0` where the compiler
  gives `self->$s0, arg1->$s2, arg2->$s1` — `arg1` is stable and only two
  values trade, so this is an exchange rather than a rotation. GCC 2.6.3's
  local allocator prioritises pseudos by reference count and `$s0` goes to
  the most-referenced, which retail's `arg2` is (four reads against two
  each). **So count references in RETAIL versus in your C before calling it
  unreachable** — equalising them is the lever. It was not applicable here
  only because the surplus reference sat in a call whose declaration is
  shared with an already-matched neighbour, and changing it regressed that
  neighbour. Unresolved, not refuted. (`func_8004D47C`, 23/33)

- **Load-delay-slot fill at a loop tail.** Retail schedules the loop
  increment into an `lh`'s delay slot; every C shape tried (combined `for`,
  `while`, explicit `do`/`while`, `__asm__("")` at three positions) emits a
  `nop` there instead. (`func_8004B100`, 95/117)

- **`li` versus `move` for a known-zero return value.** Retail materialises
  it as `move v0,s1` from a register that already holds zero; every C form
  produces `li v0,0`. Confirmed with an isolated reproducer, and NOT fixed by
  correcting the callee signature (the dropped parameter was never read in
  the body, so the callee's codegen could not change — a reasonable head
  lever, closed). (`func_8003CCDC`, 26/27 — the round's closest near-miss)

- **Pure instruction-scheduling residue in a branch-free, call-free body.**
  Neither of this round's two resolved-stall levers can apply, since both need
  a branch or a call to reason about. Nine reshapes, best 8/14.
  (`DreamSys__LogMood`)

- **The `New_X` epilogue-merge residue — the project's DOMINANT stall class, at
  24 instances.** Written up in full, with a corpus census and every attempt
  already spent, in **`docs/research/epilogue-merge-residue.md`**. Read that
  before touching any `New_X` allocator. The short form:

  > **GCC 2.6.3 (Psy-Q) `-O2` will not merge two function exits carrying
  > DIFFERENT values into one epilogue.**

  Retail plainly does merge them, so the compiler can be made to — we have not
  found the source form. Every hand-reachable shape either returns the pointer
  on both paths (one epilogue, but no materialized constant: 26/27) or forces a
  single exit and then grows a second epilogue or an extra callee-saved
  register. Two runners on different units reached this independently in one
  round and classified it identically. **Not a toolchain blocker and not an
  operator escalation** — the toolchain is innocent, the input is unknown.

- **Shared-literal early-exit delay-slot placement.** A function with several
  early-exit points all returning the SAME literal can have that constant's
  delay-slot placement scheduled differently from retail while matching on
  instruction count AND control-flow graph. Not reachable by goto/return
  spelling, statement reorder, explicit locals, or an `__asm__("")` barrier
  (which made it worse). Note this is a DISTINCT class from the epilogue-merge
  residue above: there the two exits carry *different* values and the epilogue
  count differs; here the values are identical and the CFG already matches.
  (`func_8005A82C`, 58/63)

- **Asymmetric codegen on structurally identical sibling blocks.** Two
  byte-extraction blocks against sibling struct fields, written identically,
  get different codegen; fixing one symptom (a redundant sign-extend) trades
  it for another (a 16-byte-oversized frame). Best 19/52, correct size, no
  drift. (`func_8004B030`)
- **Loop-offset increment scheduling.** The increment lands in the wrong
  delay slot regardless of where in the loop body it is written — six
  positions tried, plus a barrier, which did not block the backward hoist.
  The structural part (two recomputed registers vs one incrementing pointer)
  WAS reproduced; only that one instruction's placement resists. Best 9/74,
  correct size. (`func_8004ABD0`)
- **Duplicated loop test in a destructor scan loop.** A conditional
  skip-to-continue-check duplicates the loop condition. Three
  control-flow-equivalent spellings (`do`/`while`, `goto`, `while`) compile
  **byte-identically to each other** and none matches retail — a stronger
  negative than a single failed reshape, because it rules out the whole
  spelling family rather than one member. (`func_8004A7C0`)

All eight are permuter candidates rather than reshape candidates; see Gate 3
in docs/PARALLEL-RUNS.md.

**Round 10 ran the permuter in anger for the first time, and the results
split sharply. Read this before assuming "permuter candidate" means
"solvable".** Five searches on four functions, ~140,000 iterations total:

| target | base | best | outcome |
| --- | --- | --- | --- |
| `func_8005FC58` | — | **0** | MATCHED — found the address-of-slot lever |
| `func_8005F544` | 60 | **0** | MATCHED in 23 iterations, re-seeded from a near-miss |
| `func_8005A82C` | 460 | 200 | exhausted; convergent form is UB |
| `CalcDreamColor` | 610 | 120 | UB-only at 120; a clean 135 candidate left untried |
| `func_8003CCDC` | 5 | 5 | no movement in ~24k iterations |
| `func_8003CB68` | 70 | 70 | **no improvement over the seed at all** |

What separates the two halves is **what you seed it with**, not how close the
score is:

- **Re-seed from your closest MANUAL near-miss, never from the original flat
  form.** `func_8005F544` went to zero in 23 iterations once seeded from a
  body whose remaining defect was purely a register difference — after three
  other residues had been closed by hand first. Seeded earlier it would have
  been searching several problems at once.
- **`--debug`'s base-score COMPOSITION predicts viability better than its
  magnitude.** A base score made of register differences with zero
  insertions or deletions is a live search. One dominated by insertions and
  deletions means the shape is still wrong, and the permuter will grind
  without converging — which is exactly what `func_8003CB68` (no movement
  whatsoever) and `func_8003CCDC` (base 5, best 5) look like.
- **A base score of 5 that will not move is a stronger negative than a base
  of 460 that halves.** Do not read a low base score as "nearly there".

**And judge what it returns.** Three of the five searches converged on forms
that are not real C: a spare local read through stale-register reuse on a
path that never assigns it, and a return retyped `volatile unsigned int` in
place of the true enum. Per Gate 3 those are exhausted-class verdicts, not
leads. **Rejecting them is the discipline** — a zero reached by UB fails the
next reader, not the SHA1.

**Mark exhaustion at the right GRANULARITY.** `CalcDreamColor`'s report says
permuter-exhausted *for that region* and explicitly not overall, because the
same search surfaced a legitimate candidate at 135 — correct return type, no
UB — that was never applied. A blanket "exhausted" would have buried it.

**Correction to an earlier round's advice:** "the permuter closes
single-register residues fast" does not generalise. It did for
`func_8005F544` and `func_8005FC58`; `func_8003CB68` is a single-register
residue on which the permuter found nothing in 13.5k iterations.

**One class was CLOSED this round, and how it closed is the transferable
part.** The "vacate-then-reuse-argument-register" residue was written up with
a corpus census as a rare, poor permuter target. It was not a scheduling or
allocation class at all — it was a missing call argument (see the
cross-check-another-caller entry above), and the census had been measuring a
shape that merely *co-occurs* with it: a value sitting in an argument
register because it is a future argument. **A census of a residue's surface
SHAPE is not a census of its CAUSE**, and a plausible mechanism attached to a
real measurement is still a hypothesis. The head confirmed that stall twice
before a runner overturned it.

**The cheap tell that you are in the wrong shape FAMILY, not one reshape from
a match:** funcdiff's *"differs OUTSIDE this range"* warning carrying a
six-figure byte count. That means your function changed SIZE and every later
address shifted. Two independent runners plus the head each hit it this round;
recognising it early is worth several attempts. Read it as "wrong family, start
over", never as "close, keep pushing".

### Still unconfirmed here

Candidates from other GCC 2.x projects — treat each as a thing to test:

- Member loads hoisted into a local at the top of a loop.
- A struct pointer to a global block, rather than several separate globals.
- ~~Comma expressions and assignment-in-condition~~ — **TESTED 2026-09-02, and
  for the two-exit case the answer is NO.** A comma-ternary
  (`return c ? (f(x), p) : NULL;`) compiled byte-for-byte identically to the
  separated early-return form, down to the same outside-range byte count, so
  2.6.3 does NOT schedule it differently there — it lowers both to the same
  RTL. Still untested as a scheduling lever in a SINGLE-exit body, which is a
  different question and remains open. (head adjudication of `func_8004A130`,
  see `docs/research/epilogue-merge-residue.md`)

## The class framework (SETTLED — the game is plain C)

**Proven 2026-08-28, full evidence and reproducer in
docs/research/class-framework.md.** The game is plain C with a HAND-ROLLED
class framework. It is not C++: nothing in the binary was built by a C++ front
end.

The trap this nearly walked into: the suggestive symbol names (`New_DreamSys`,
`DreamSys__DreamSys`, `BasicClass__*`) are all **FirecatFG's hypotheses**,
inherited with the symbol file. Reading them as evidence for C++ is circular —
they look like C++ because someone who suspected C++ chose them. Every finding
below is from the bytes.

- **Constructors are called THROUGH the method table** (slot `+0x008`), and so
  are base-class constructors. No C++ compiler can do that: the object has no
  vtable pointer until the constructor stores one, which is why the language
  forbids virtual constructors. This alone settles it.
- **Table entries are 4 bytes.** Compiling C++ with this repo's own
  `tools/gcc263/cc1plus` emits **8-byte** entries — `{short delta; short index;
  void *pfn}` with an entry-count header. The game's tables are flat pointers.
- **Tables contain null slots mid-table.** g++ fills an unimplemented virtual
  with `__pure_virtual`, never zero, and never leaves a hole.
- **The vptr is at object offset 0**; g++ 2.6.3 places it after the base's data
  members.
- **A scan of the whole executable finds ZERO 8-byte-stride vtables** against
  **128 flat pointer tables**. Nothing here is compiler-generated C++.

Practical consequences:

- The pipeline is correct as-is. `cc1`, not `cc1plus`; units stay `.c`.
- Methods are ordinary C functions with an explicit `this` first parameter.
- Method tables are DATA — a `static` struct-of-function-pointers initializer,
  decompiled like any other data slot.
- `lw $v0, 0x0($reg)` then `lw $v0, <off>($v0)` then `jalr` is a method call.
  Resolve `<off>` with `tools/classtable.py <table> [--vs <base>]`; the `--vs`
  diff is the subclass's behaviour in one screen.
- **60 classes, ~1425 method slots.** This is the game's backbone, not a corner.
- **`BasicClass` is the ROOT of the framework, and its internals are now
  matched** (round 11, `code_8220`, 16 functions byte-exact). Every object in
  the game inherits this layout, so it is worth reading before working any
  class:
  - `BMemPMgrInit` builds a pool header (`freeListHead`, `poolSize`) over a
    heap allocation. The pool seeding itself is in a `gp_rel`-blocked function.
  - A `BasicClass` object owns **two pool-allocated singly-linked lists**:
    `children` at `+0x004` and `parentRefs` at `+0x008` (back-references).
  - `addChild`/`removeChild` dispatch **bidirectional notifications through
    the CHILD's own vtable**, not the parent's — which is why a child class's
    slots get called from code that appears to belong to the parent.
  - Two underlying list primitives do the real work: `func_800181AC` (push)
    and `func_80018208` (find-unlink-free).

  The design was reconstructed from the callers alone BEFORE the primitives
  were matched, and every extern declaration predicted that way turned out
  correct against the real bodies — worth noting as evidence that reading a
  class's callers is a reliable way into it. Layout in `include/code_8220.h`.
- **A patchy `--vs` diff may mean you picked the wrong ancestor, not that
  there is no inheritance.** If a derived table's *high* slots line up
  byte-for-byte with some other candidate table's high slots, re-run `--vs`
  against that one; the longer identical run is the real parent. This is how
  `code_1677c`'s class was found to descend from `D_8006E4F0`, an intermediate
  between `BasicClass` and itself, rather than directly from `BasicClass`.
  (`func_80026108`)
- **`New_X` allocator wrappers are a recurring shape, and there are TWO
  variants — only one of them is stalled.** Both start malloc → null check →
  constructor through the class's own slot `+0x008`. They differ in what they
  do with the constructor's return:
  - *Checked variant* — test the ctor's return, free the block and return
    `NULL` on failure, else return the allocation. **This one matches.**
    `New_Entity` (`src/Entity.c`) and `New_Class6B5CC` (`func_8001CA94`,
    24/24, round 11, first attempt) are both this shape.
  - *Return-regardless variant* — ignore the ctor's return and hand back the
    allocation unconditionally. `new_class_6d3c8` stalls here on a single
    delay-slot residue that no `if`/`goto`/temp-variable reshaping and no
    `__asm__("")` barrier closed.

  **This entry previously said `New_X` as a whole was the highest-value single
  permuter target, on the reading that the shape was unmatched anywhere.** That
  was too broad: the checked variant was already matched in `Entity.c` when the
  claim was written, and round 11 matched a second one cold on the first
  attempt. The permuter target is specifically the RETURN-REGARDLESS variant,
  which is a smaller population — so before spending a permuter round on it,
  count how many `New_X` sites actually ignore the ctor return rather than
  assuming all ~60 classes do. Do not re-derive the same 20+ manual attempts
  per class in the meantime.

## Open questions

- **What is the class-table header word at `+0x000`? PARTLY ANSWERED, and the
  first answer was TOO NARROW.** It holds a class identifier, but **the field is
  wider than 12 bits.**
  - 2026-09-01, `func_80058E8C`: the first confirmed in-game READ. Masks the
    word with `0xFFF` and compares against a literal class id — a runtime type
    check. This was written up as "the low 12 bits are a class identifier".
  - **2026-09-02, `func_80058F18` (the sibling check) CORRECTS that**: it masks
    with **`0xFFFFF`** — 20 bits — and compares against **`0x1F234`**, which is
    `D_80089AD4`'s own header value. A 20-bit comparand cannot fit the 12-bit
    reading, so `0xFFF` was **one function's mask, not the field's width**. Two
    call sites, two different masks, both against real class ids.

  The generalizable error is worth keeping: a single masked read tells you an
  identifier is *at least* that wide, never that it is *exactly* that wide.
  Treat the widest observed mask as the current lower bound. The next step is
  unchanged but better posed: find the WRITES, in the framework code at
  `class_16334` / `code_179d8`, and see what composes the field — and whether
  anything ever uses the top 12 bits of the word.
- **What are the four dead-looking data slots** at `0x57070`, `0x76DC8`,
  `0x79528` and the `sbss` runs? They assemble and link fine as plain data, so
  nothing is blocked, but their owners are unidentified. **Partly answered for
  `0x57070` (2026-08-29):** it holds at least `D_8006D370`, one class's method
  table, and probably `BASICCLASS_METHODS` (`D_8006B58C`) next to it. Before
  calling any of these slots unidentified, cross-reference all 60 addresses
  from `classtable.py --scan` against the segment ranges — the "dead" slots may
  simply be the method tables, already understood, seen from the data side.
  Nobody has run that cross-reference yet; it is cheap.
- **How much of the 724 `psyq_*` functions is really SDK?** The blocks were
  identified by lsddecomp and inherited wholesale. The boundary between
  `psyq_memset` (260 functions) and game code at `0x39c80` in particular is a
  big claim resting on one name. Worth a spot check before anyone relies on the
  game-code percentage.
