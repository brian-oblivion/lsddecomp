# Matching guide

The per-function loop, in detail. Read CLAUDE.md first for the hard rules and
the toolchain facts.

## The loop

### 1. Pick a function

From the `INCLUDE_ASM(...)` entries in your assigned `src/<unit>.c`.
`python3 tools/progress.py` shows which units have fresh queue. Prefer leaves
(functions that call little) on a cold start — they establish struct layouts
that the callers then reuse.

### 2. Read the disassembly

`asm/nonmatchings/<unit>/<func>.s`. Read it before reaching for m2c. Things
worth noticing on the first pass, all of which change how you write the C:

- **The frame.** `addiu $sp, $sp, -N` at the top and the matching restore at
  the bottom. More than one such instruction means the symbol covers more than
  one function — stop and tell the head; it is a carve bug, not a hard function.
- **Which saved registers are used** (`$s0`–`$s7`). Roughly, one per long-lived
  local. A body that needs six of them is not a three-line function.
- **`%gp_rel(sym)($gp)`** — an ordinary global access. `$gp` is `0x8008A808`.
- **Delay slots.** `-mips1`, so every branch and jump has one, and it executes
  before the branch is taken. The instruction indented under a `jal` is the
  argument setup for that call, not the next statement.
- **Soft float.** There is no FPU. Float arithmetic becomes calls into the
  libgcc-style helpers; a `jal` to something like `__adddf3` is `a + b` on
  doubles, not a function the game wrote.

### 3. Seed with m2c (optional, but pass `--sig`)

```sh
.venv/bin/python3 tools/m2ctx.py <unit> --sig 'void <func>(Foo *a, s32 b)' --run
```

Without `--sig` you get `void *arg0` and `->unk8` regardless of context quality,
because m2c types arguments from a declaration of the target and `INCLUDE_ASM`
leaves none. With one, the failures become informative: a `padNN[0x..]` access
means the struct you guessed is missing a field at that offset. Iterate on
`--sig` until the padding accesses disappear — that iteration IS the struct
derivation.

m2c output is a seed, not an answer. It is frequently correct about control
flow and frequently wrong about types.

### 4. Write the C

- **C89.** Declarations at block top. `//` comments are a parse error in GCC
  2.6.3's cpp.
- **Strict ROM-address order** within the unit. Out-of-order definitions
  miscompile the whole image while the link still succeeds.
- Real struct fields, not pointer arithmetic. If you find yourself writing
  `*(s32 *)((u8 *)p + 0x24)`, you have found a field — name it in the header.
- `char` is unsigned here (`-funsigned-char`). A signed byte is `s8`.

### 5. Verify

```sh
./build-and-verify.sh > /tmp/b.log 2>&1; echo "build exit=$?"
grep -nE 'error:|parse error|undefined reference|\*\*\* \[[^]]*\.o\]' /tmp/b.log | head -8
.venv/bin/python3 tools/funcdiff.py <func>
```

`build exit=` is the only line that decides whether the score means anything.
See CLAUDE.md "The three ways a score lies".

**In a parallel round, put your runner name in the log path** —
`/tmp/<name>_b.log`. `/tmp` is shared between worktrees, and in round 8 two
runners both writing `/tmp/b.log` crossed over: one read the other's build
result. A shared scratch path turns the log you grep to validate your score
into a fourth way a score can lie.

To read a diff rather than score it:

```sh
.venv/bin/python3 tools/asm-differ/diff.py <func>
```

### 6. Iterate, or stop

**Hard stop at 30 attempts.** On a stall: restore the `INCLUDE_ASM`, inline the
best body you reached into the match report as literal source with its
declarations, classify the residue, and move on. A stall with a good report is
a contribution; a stall that burns a whole session is not.

On a match: tidy the C, name what you learned into `include/`, write the report,
commit.

## Reading a residue

When the body is right but a few instructions differ, the residue usually falls
into one of these. This list is short because this project is young — add to it.

- **Register identity.** Same instructions, different registers. Often a
  declaration-order or lifetime difference in the C. **Never** fix it with
  `register T v asm("$N")` or an asm operand constraint; both are banned. If
  reshaping does not move it, it is a stall.
- **Instruction order only.** Sometimes a scheduling barrier — a bare
  `__asm__("")` — is legitimate. The test: if removing it changes WHICH
  REGISTER holds a value it is banned; if it only changes ORDER it is allowed.
- **Off-by-one instruction count, short.** Frequently a missing volatile, a
  hoisted member load the compiler did not hoist, or a `do/while` written as
  `while`.
- **A `nop` retail has and you do not** (or vice versa) around a branch. Delay
  slot filling differs when the source statement order differs.
- **maspsx-expanded macros.** `div` by a non-constant expands to a several
  instruction sequence with a zero check. If your division differs structurally
  from retail's, check whether retail divided at all — a shift may be the
  source form.
- **A `lui`+`lw` pair where retail has a single `lw ..($gp)`.** This is the
  gp-relative blocker, and **no source-level reshaping will move it** — the
  addressing mode comes from the toolchain's `-G` value. Check for it before
  spending attempts: `grep -l 'gp_rel' asm/nonmatchings/<unit>/*.s`. A hit
  means STOP and file the report; see `docs/research/gp-relative-blocker.md`.
  It is by far the largest obstruction in the project — derive the current
  figure with `python3 tools/nearmiss.py | head -2` rather than trusting any
  number written here. (This line used to say "nine functions across two
  units", which was one round's snapshot and understated it by more than an
  order of magnitude by round 25.)

- **Branch TARGETS disagree, not just delay slots.** This one is a
  discriminator, not a residue, and it outranks everything else in this list.
  A differing delay slot is a scheduler choice; a differing branch *target* is a
  different control-flow graph, and a different CFG always comes from the
  source. **Line up the branch targets before classifying anything as a
  scheduling stall, a delay-slot choice or a toolchain lead.** In round
  2026-08-30-a this turned a confidently-filed "unreachable compiler-internal"
  stall at 16/42 into 41/42 with a one-line source change, and a second runner
  credited it with closing a function outright (10/69 -> 69/69).
- **One instruction short, everything after it shifted.** Suspect **BLOCK
  ORDER** first — see the entry below. Do **NOT** screen for `addiu_at`: it
  was RESOLVED in round 21 (maspsx `--addiu-at`,
  `docs/research/addiu-at-blocker.md`), and screening for it now INVENTS a
  blocker, which is the strictly worse failure — a false blocker becomes a
  stub report, which `progress.py` counts as a documented stall, which
  removes the function from `fresh` permanently.

  *This bullet previously read "Suspect the `addiu_at` blocker before
  suspecting your C ... a hit means STOP and file the stub", and it survived
  four rounds after the blocker died. It is kept as a corrected bullet rather
  than deleted because a stale line in a guide is not merely wrong, it is a
  DIRECTIVE, and this one told runners to stop working matchable functions.
  Screen with `python3 tools/nearmiss.py`, which runs the two live screens
  (`gp_rel`, `nop_mflo_mfhi`) and reports the resolved construct WITHOUT
  counting it.*

- **BLOCK ORDER: retail has a bare unconditional `j` you do not, or keeps a
  duplicated assignment you merged.** Round 25 closed four functions on this
  across three units after they had absorbed 13 hand attempts, ~77k permuter
  iterations, eleven structural variants and several barriers between them.

  **GCC 2.6.3 gives the fallthrough to whichever candidate is LAST in source
  order.** If retail's *jumping* block is the one your source puts last, no
  expression reshape and no `__asm__("")` will reach it. The fix is textual:
  make that block not-last, with an explicit `goto` over the block you want
  to fall through.

  The tell is a bare `j` (not a conditional branch) to a nearby join whose
  delay slot carries REAL WORK. Two strong secondary indicators:
  **you are SHORT by a small number of words**, and **a barrier had no
  effect** — the latter is positive evidence FOR this class, not evidence of
  exhaustion, because a barrier moves instructions within a block and this is
  a cross-block decision.

  Three cautions, all measured:
  - **It is not strictly dominant.** Applied to a guard that already has the
    correct idiom it makes things WORSE (`func_80059BE0`). Read which shape
    the disassembly ALREADY shows first.
  - **"Give the duplicate its own C variable" is NOT the fix** once block
    order is already right — twice-reproduced regressions on
    `func_8002B4D4`. A second named variable re-allocates the WHOLE function.
  - **An `mflo`/`mfhi` residue is a counter-indication.** cc1 expands
    `mult`/`mflo` together during RTL expansion, before any layout decision.

  Full entry, with all four placement variants: `DECOMPILATION_LEARNINGS.md`,
  "An arm that must JUMP has to be written NOT-LAST".
- **Prologue callee-save stores in the wrong ORDER**, same registers and same
  offsets. Not reachable from C — six declaration-order permutations produce
  one identical score. A bare `__asm__("")` as the function's first statement is
  the lever and is the permitted form; prove it by removing it and confirming
  register allocation is unchanged. It does NOT generalise to other residues:
  three runners tried it elsewhere and worsened them.
- **A redundant `move` retail emits and you do not**, same register, same
  value, branch targets agreeing. This is the genuine one-instruction class, now
  with three confirmed instances (`new_class_6d3c8` 23/24, `strcat` 41/42,
  plus a third found this round). It resists `goto`/`return` spelling, temp
  placement, barriers and `volatile`. It is the project's best-posed permuter
  target — do not spend a fresh attempt budget re-deriving it.

## Unit and segment state

**Not recorded here.** It changed every round, this file said "keep this
current", and it still went stale — naming units that had been carved and
segments that no longer existed under those names. A snapshot that is wrong
half the time is worse than no snapshot, because a reader cannot tell which
half they are in. Derive it instead:

```sh
python3 tools/progress.py     # matched / queued / stalled / fresh, per unit
```

`fresh` is a ceiling, not a work order — it cannot see "large body, deep
reconstruction, low cold-runner yield". Three things tell you that, and all
three are live rather than transcribed:

- **Size.** The remaining queue for a unit, biggest first. A unit whose cheap
  seam is exhausted shows it here as a floor of 35+ instruction bodies:

  ```sh
  wc -l asm/nonmatchings/<unit>/*.s | sort -rn | head -20
  ```

- **`docs/match-reports/<func>.md`.** Every stalled or blocked function has
  one, and it carries what a column cannot: the residue, how many attempts have
  been spent, whether it is a permuter target, and whether it is toolchain-
  blocked. **Read the report before staffing anyone onto a function** — several
  carry an explicit do-not-re-staff finding, and re-deriving one costs a round.

  **But a report is the best available account, not a verdict.** Round 7
  retired a stall whose report carried fourteen attempts by two authors, a
  head re-audit that CONFIRMED the classification, and a corpus census
  supporting it — and it was still the wrong diagnosis. Trust a report to
  tell you what has been tried; do not let it tell you what is impossible.
  The specific way it went wrong is worth knowing, because it is cheap to
  repeat: every attempt varied the code that COMPUTED a value while holding
  the call's signature fixed, so the whole attempt history explored one
  branch of the space and read as if it had explored all of it. When a report
  shows many attempts along one axis, that is a reason to look for the axis
  nobody varied.

  **Round 24 produced the strongest instance of this yet, and the shape is
  worth carrying: the report was not sloppy, it was AUTHORITATIVE and stale.**
  Three `Entity` getters (9, 14 and 15 words — the cheapest functions in the
  corpus) each opened with a HEAD ADJUDICATION certifying that the head had
  reproduced the diagnosis independently from scratch and written it up
  project-wide with a 502-of-502 corpus census. All three matched on the first
  attempt, with the C their own reports had already derived. Nothing in the
  adjudication was wrong — it verified the MECHANISM, which never changed.
  What expired was the premise that the mechanism was unfixable, stated
  explicitly in each report as "maspsx exposes no `--addiu-at` flag", which
  round 21 falsified by adding exactly that flag.

  So: **a report's confidence is not evidence about its currency, and a
  resolved blocker invalidates the best-argued reports as thoroughly as the
  worst.** Two practical habits follow. Rank from `python3 tools/nearmiss.py`,
  which screens from the ASM, and treat any disagreement between it and a
  report's verdict as the report being the wrong half. And when one stale
  report turns up, read every report that shares its preamble, its unit, or
  its adjudicating round — these are written in families, and round 24's
  detector found one of those three siblings and missed the other two.

  **Round 26 adds the case the two above do not cover: a report that was
  never a verdict at all — a banked DERIVATION — and it was wrong in two
  places.** `func_80032D34` carried a round-25 report marked
  `DERIVATION ONLY -- ASSIGNABLE`: the head had derived the algorithm from the
  disassembly but never written C. That is the most trustworthy-looking kind
  of report, because it makes no claim about what is impossible and reads as
  pure fact. Round 26's echo turned it into C at 271/274 and, having been
  asked explicitly to flag errors rather than work around them, found two:

  - The per-VAG shift test reads `hdr->ver` (offset 4) — the SAME field the
    surrounding decision already uses. The derivation named `hdr->attr`, a
    field that **does not exist in the struct**.
  - The table advance is a FIXED `+ 0x200`, not `+ numVags*2` as derived. The
    proof is delay-slot semantics: the increment sits in the loop-continuation
    branch's own delay slot, so it executes on all 256 iterations regardless
    of the inner cutoff.

  Both are the kind of error that survives review indefinitely, because the
  pseudocode is plausible and nobody re-reads the `.s` to check a field name.
  The second one is the more instructive: **a derivation written by reading
  instructions in source order will get delay-slot placement wrong**, and a
  delay slot is precisely where an increment hides.

  Two habits follow, and the first is an ASSIGNMENT habit rather than a
  reading one. **When you staff someone onto a banked derivation, tell them
  explicitly that finding the derivation wrong is a result you want** — echo
  did this because it was asked to, and the default posture toward a report
  from the head is deference. And **verify a derivation's field names and
  offsets against the raw `.s`, not against its own prose**, before building
  on them.

  **Round 27 adds the surface none of the above covers, and it is the one no
  tool can see: a note about where NOT to work.** Everything above concerns
  match REPORTS, which `progress.py` and `nearmiss.py` both index. The
  equivalent staleness in a splat carve comment, a `src/*.c` unit header, or
  a previous round's triage recommendation is invisible to every tool — and
  it is worse in kind, because those are phrased as **directives** rather
  than findings.

  `class_3bb8c_h` was screened in round 17 as **15 of 17 "clean"** and
  correctly judged the least matchable ground in the executable: 13 of its
  functions are PSX BIOS trampolines no C compiles to, and the other four
  were `addiu_at`-blocked. The note concluded "the cheapest next slice
  therefore starts AFTER the trampolines". Round 21 resolved `addiu_at`,
  which silently made all four of those functions blocker-clean — the
  **densest clean ground left uncarved**, on zero live blockers. Nobody
  re-measured for four rounds, and two consecutive rounds' triage repeated
  "avoid `class_3bb8c_h`" on the strength of it. Round 27 carved it and
  matched two of the four functions in one head sitting (8/8 and 25/25,
  first and fourth attempt).

  Note what was and was not wrong, because it is the round-24 shape exactly:
  every factual claim in the note was true and stayed true. The trampolines
  are still trampolines. What expired was the **premise** that the other four
  were unreachable — and the note's operational advice outlived it. The same
  note also carried a claim nobody had checked, that all 13 trampolines
  occupied the span and "nothing else does"; four ordinary functions are
  interleaved among them in three clusters.

  Three habits follow, and the first is the cheap one:

  - **Rank uncarved ground with `python3 tools/uncarved.py`, not by reading
    notes.** It screens per function from the ASM, the same way
    `nearmiss.py` does for the queue, and it exists because this gate's
    older `grep -c '^glabel'` one-liner counts FUNCTIONS where what matters
    is WORKABLE functions — `class_3bb8c_h` counted 17 and held 4.
  - **When a blocker is retired, sweep the DIRECTIVES too, not just the
    reports.** `grep -rn 'addiu' src/*.c` and the splat yaml's carve
    comments. Round 23 found four such units; round 27 found a fifth
    (`class_3bb8c_k`, whose comment still said "do not attempt either" of two
    functions, one of which had been matchable since round 21 and was in fact
    already matched).
  - **Treat a "do not work here" verdict as having a shelf life tied to the
    blocker it rests on.** A note that says *why* it is closed can be
    re-checked in seconds; one that only says *don't* cannot be, and will be
    obeyed indefinitely. Record the method and the date next to any such
    verdict you write.

  **Round 16 hit this twice in one round, which makes it a standing check
  rather than an anecdote: a long attempt list is not a broad one.**

  - `func_80032BB8` — seven reshapes, every one varying the *expression* form
    (array index vs pointer arithmetic, temp vs no temp, declaration split,
    parameter type). None changed the CONTROL FLOW.
  - `func_8002C048` — twenty-five variations, every one keeping the cached
    `c1`/`c2` locals. None tested whether those locals should exist, which is
    exactly where the already-documented no-cache idiom points.

  Both were sent back on the untested axis. One improved into a cleaner stall
  (dropping a `volatile` that had been fighting a CSE, and the spurious mask
  it caused, at the cost of one nominal word); the other confirmed its stall
  and, in doing so, bounded the lever that had been proposed for it.

  **So when you file a stall, list the AXES you varied, not the number of
  attempts.** If every entry sits on one axis, that is the tell — and it is
  the tell whether you are the author or the reader.

- **Retail's callee-saved-register demand** (added round 13, after a runner
  used it predictively and the head validated it against that round's
  outcomes):

  ```sh
  grep -oE 'sw +\$(s[0-7]|fp),' asm/nonmatchings/<unit>/<func>.s | sort -u | wc -l
  ```

  Pooled over rounds 13 and 14 — 81 matched functions, 10 stalled:

  | band | matched | stalled |
  | --- | --- | --- |
  | 5-6 registers | **5** | 1 |
  | 7+ registers | **1** (round 16) | 4 |

  **The threshold is 7, not 5** — round 13 published 5 on a sample whose
  maximum observed demand among matches was 4, so it could not tell "5 is
  fatal" from "9 is fatal", and round 14 matched five functions in the 5-6
  band, four of them after a runner had been told to expect a stall.

  **ROUND 16: the 7+ band is no longer 0-matched, so it is a priority order
  and nothing more.** `func_8004A534` matched **163/163 with 8 distinct
  callee-saved registers** — sent in *expecting* a stall, on the strength of
  the 0-matched figure this table used to print. Two other runners the same
  round attempted six more large bodies between them and reported, when asked
  directly, that **register count was not the blocker in any of them**; two of
  those needed 0 and 1 s-registers anyway.

  Read the count as a **correlate** of "large function needing deep struct
  reconstruction" — that is what actually costs. `func_8004A534`'s
  load-bearing insight was a struct-shape one (a whole-struct copy misread as
  field-by-field), nothing to do with register pressure. **Never skip a
  function on this screen, and never stop on it: if you stop in this band,
  your report must name a residue, not a register count.**

  Twice corrected now, and the reason is the same both times: a screen built
  only from the failures it predicted needs a deliberate attempt on its wrong
  side before it earns a number.

  **Use it in ONE direction only.** A high count deprioritises; it never
  promises. And a LOW count predicts nothing at all — stalls at 2-4
  registers are common and fail for entirely unrelated reasons (tail-merge
  granularity, redundant constant materialisation, the `nop_mflo_mfhi`
  blocker, shared-tail dispatch).

  Mind the register naming: `$30` is `$fp` and `$s8` and the same register.
  splat's `.s` writes `$fp`; `objdump` renders it `s8`. A screen written for
  one and run over the other undercounts by exactly one, which is the
  difference between "8 of 9" and fully saturated. The head made that error
  in round 13 and the runner's figure was the correct one.

  **Round 8 retired three more, and the pattern repeated exactly.** All three
  were written up as compiler-internal with mechanically detailed reasoning —
  one cited GCC's `reorg.c` by function name — and all three fell to a
  source-level change:

  | function | had stood as | actually was |
  | --- | --- | --- |
  | `new_class_6d3c8` | delay-slot filler choice, 3 rounds, 20+ attempts | one surplus `return` on the null path |
  | `strcat` | ditto, plus a head adjudication | `return dest;` vs `return NULL;` on a guard |
  | `func_80026698` | switch-lowering internals, 2 rounds | the store's value misread as 3; it is 1 |

  The common failure was not laziness — every one of those reports was
  careful. It was that **a plausible mechanism attached to a real measurement
  still has to be checked against the instructions.** Two of the three were
  fixed by re-reading four instructions; the third by noticing that a delay
  slot had changed a register before the branch target read it. Before you
  believe any "compiler-internal" verdict, re-read the residue's immediate
  neighbourhood and ask the two questions in DECOMPILATION_LEARNINGS' "How to
  read a one-instruction residue": did a delay slot move a value, and did the
  author transcribe a lowering instead of the expression behind it?

  **Round 10 retired a fourth, and the variant is different enough to name.**
  `func_8005F544` had stood at 30/49 with a careful six-attempt report
  describing **a single-instruction residue** — and it was FOUR independent
  residues stacked. The report was not misdiagnosed; it was under-counted.
  Closing the first three moved the score not at all, which is exactly what
  had made "one problem" the natural reading in the first place.

  So add a third question to the two above: **is this one defect, or several
  whose scores do not add?** The tell is a residue description that accounts
  for the shape but not the arithmetic — a report saying "one instruction
  short" while the diff shows differences in several unrelated places. Fix
  incrementally and re-measure after each change rather than looking for the
  single explanation that covers the whole gap, and do not revert a change
  with independent evidence behind it just because the score did not move
  (see DECOMPILATION_LEARNINGS on entangled residues, where a fix that scored
  *worse* alone was the key unlock in combination).

  **And a low score is not evidence of a distant shape.** `func_8004BB3C`
  scores 14/105 while being structurally 104 of 105 instructions identical to
  retail; one missing `addiu` shifted everything after it. Triaging stalls by
  score alone would have written it off as the unit's worst prospect when it
  is its best. Cross-check the real compiled length with
  `objdump -d build/src/<unit>.c.o` before believing a bad number.

  **Round 12 added a second instance of that, and this one has a one-command
  discriminator.** `func_8003D73C` scores 40/145 with *exactly correct total
  length* and zero inserted or deleted instructions — every non-matching word
  is the same instruction on a different register. The cause is that retail
  saturates the callee-saved file, so a body needing one more live cross-call
  value spills into `$fp` and renumbers everything downstream:

  ```sh
  grep -oE 'sw +\$s[0-9]' asm/nonmatchings/<unit>/<func>.s | sort -u | wc -l
  ```

  8 means no spare callee-saved register. Run this before staffing any
  register-flavoured stall — it is the difference between an unfixable
  register-identity stall and one with a specific permitted lever (reduce
  values live across calls).

  **It also caught a report mis-grouping two stalls.** `func_8003D73C`'s report
  described itself as the same class as its sibling `func_8003DAD4` (114/118);
  the census says 8 registers versus 6, so one is saturated and the other has
  two s-regs and `$fp` spare. The report had even recorded that the sibling's
  fixes did not transfer — which is the tell. **When two stalls in one unit are
  both described as "register" problems, measure before believing they are one
  class**, because the shared label is doing the grouping, not the evidence.

For uncarved ground, see Gate 2 in `docs/PARALLEL-RUNS.md`, which lists the
segments live and records the carve hazards found so far.

## Writing a class method

The game is plain C with a hand-rolled class framework — **proven, see
docs/research/class-framework.md**; do not reach for C++. What that means at
the keyboard:

- A method is an ordinary C function whose first parameter is the object:
  `void DreamSys_TimerTick(DreamSys *self, s32 delta)`. `$a0` is `this`.
- **Every object's method table pointer lives at offset 0.** This shape:

  ```
  lw    $v0, 0x0($s1)      ; obj->methods
  lw    $v0, 0x80($v0)     ; the slot
  jalr  $v0
  ```

  is a method call whose target is **in the data, not the instruction stream**.
  You cannot read it off the disassembly. Resolve it:

  ```sh
  .venv/bin/python3 tools/classtable.py DREAMSYS_METHODS
  .venv/bin/python3 tools/classtable.py DREAMSYS_METHODS --vs 0x800878D4
  ```

  Do not count slots by hand — an off-by-one silently names the wrong function,
  and the resulting C looks entirely reasonable.
- **`--vs` is the one to reach for first on an unfamiliar class.** A derived
  table is a copy of its base's with slots replaced, so the diff tells you what
  the subclass actually does. DreamSys inherits 48 slots, adds 90, overrides 8.
- Allocation sites look like: allocator call with a literal size, null check,
  then the constructor fetched from slot `+0x008` and called indirectly. That
  is a `New_X` function, and it is ordinary C.

## Permuter

**Set up and proven, round 8 (2026-09-02).** One command provisions a run:

```sh
tools/setup-permuter.sh <func> <seed.c>      # -> permuter-work/<func>
PATH=$PWD/permuter-work/bin:$PATH \
  .venv/bin/python3 tools/decomp-permuter/permuter.py --debug permuter-work/<func>
PATH=$PWD/permuter-work/bin:$PATH \
  .venv/bin/python3 tools/decomp-permuter/permuter.py -j 6 \
    --stop-on-zero --best-only permuter-work/<func>
```

`<seed.c>` is a few lines: the project `#include`s plus ONE function
definition, normally the near-miss body copied out of its match report. The
script generates `compile.sh` from the Makefile's own C rule, assembles retail's
bytes into `target.o`, reduces the seed to `base.c`, and proves the scaffold
compiles before handing it back. It also documents the four setup traps that
each look like a broken toolchain — read its header comment rather than
rediscovering them.

**ALWAYS PASS `--stack-diffs`, to `--debug` AND to the real search.** Without
it `permuter.py` does not measure the stack frame at all, and therefore
**falsely scores ZERO on any pure frame-size residue**. Round 18 hit exactly
this: `func_80065AE0` reported a zero that was really 28/40 with an 8-byte
frame overshoot, invisible until the flag was passed. The trap is sharpened by
the display: **without the flag the output still prints `Stack Differences: 0`**
— that line is not a measurement saying the frames agree, it is a field that
was never filled in, and it reads exactly like a clean result.

Two consequences, both paid for in round 18:

- Re-check any zero or near-zero measured without the flag before it goes into
  a report as a result. A false zero written down as a real one is the kind of
  finding that misleads every later round.
- Where a number is load-bearing, re-measure it with the flag and SAY you did.
  `func_80063144`'s two-divergence finding was re-run this way and came back an
  identical 465 with `Stack Differences: 0` genuinely measured, which is what
  makes it trustworthy rather than merely plausible.

**A permuter score drop is a LEAD, never a RESULT, until the real oracle
confirms it — and round 18 went 0-for-3 on trusting one.** Every permuter-local
improvement found on a register-shaped residue that round was FALSE against
`build-and-verify.sh` plus `funcdiff.py`:

- a cached-OT-pointer candidate scoring 240: real result 17/54, with funcdiff's
  drift warning firing (296814 bytes differing outside the range);
- an algebraically-identical loop-end rewrite scoring 60 (reproducibly, found
  twice): real result 52/70, **one word WORSE** than the 53/70 it started from,
  an in-range regression the permuter's own diff could not see;
- the `func_80065AE0` false zero above.

The permuter's scorer and the project's oracle do not measure the same thing.
Verify every candidate through the full chain before believing it, including —
especially — the ones that look best.

**Always run `--debug` first and check the base score against what the match
report claims.** A scaffold that scores something other than the reported
residue is scoring a different function than you think, and the whole search is
then wasted. `--debug` prints a penalty list: one insertion + one deletion
(score 200) is the signature of a genuine one-instruction residue.

**Sanity-check the numbers.** `permuter.py`'s score is not funcdiff's. Zero
means "identical to `target.o`"; it is not a word count, and it is not the
oracle. `./build-and-verify.sh` is.

**A permuter zero is a LEAD, not an answer.** Translate it to idiomatic C and
re-verify with `build-and-verify.sh` plus `funcdiff.py`. This is not a
formality — it decided both of the round-8 matches:

- `new_class_6d3c8` (24/24): the permuter's zero WAS idiomatic and went in
  essentially as found (`return` inside the `if`, null path falls off the end).
- `strcat` (42/42): the permuter's zero was a **dead store** on the null path.
  Committing it would have put provably dead code in `src/`. It was still a
  correct lead — it said retail's source *uses* `dest` there — and the
  idiomatic way to use it, `return dest;` instead of `return NULL;`, scored
  identically.

If only undefined-behaviour or duplicate-arm forms reach zero and no idiomatic
translation scores the same, mark the class permuter-exhausted in the report and
move on.

**What it is good for, from two data points.** Both round-8 targets were
one-instruction residues that three rounds of manual reshaping, `__asm__("")`
barriers and detailed `reorg.c` root-cause reasoning had failed to close, and
both fell in under 400 iterations (47 and 320) — well under a minute each. Both
turned out to be a mismatch in how many times the source MENTIONS a value, not
a scheduling choice. That is the class to reach for the permuter on. It is not
a substitute for getting the control-flow shape right: if the branch targets
differ, fix the source first.
