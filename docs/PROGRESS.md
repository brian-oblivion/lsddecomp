# Progress log

One entry per session or round. The head writes these during consolidation.
Counts come from `python3 tools/progress.py`; dated entries are allowed to go
stale, prose elsewhere is not.

---

## 2026-09-02 — round 7: 4 runners, 58 matches, two carves, and two heads in one checkout

**198 -> 261 matched (14.60% -> 19.25% of game code; matched bytes 6.80% ->
12.09%). 133 queued: 89 stalled, 44 fresh. Build green in main after every
one of thirteen merges and after both carves.**

Four runners, one unit each, all sent back into their own units after their
first pass per §3c. Three units were cleared outright, so the head carved two
new ones mid-round and handed them to the runners that had run out of ground.

| runner | units worked | matched | stalled |
| --- | --- | --- | --- |
| alpha | `class_3ac78` | 7 | 3 |
| bravo | `code_2c054` -> `DreamSys` | 18 | 1 |
| charlie | `class_39e08` -> `class_3bb8c` | 16 | 0 |
| delta | `Entity_b` -> `Entity_c` | 16 | 0 |

Five of the 58 were salvaged rather than produced: three inherited from round
6's abandoned worktrees, plus `func_8005AD68` from bravo's own death (below).

### Two heads in one checkout — the round's real finding

The operator started this round's head while round 6's head was still live.
Both worked the same `main`. **Nothing was corrupted** — every commit was
real, no protected file was touched, `main` byte-verified throughout — but
each head read the other's legitimate head actions as its own runners
misbehaving, and both wrote the misreading down as fact:

- Round 6's head filed a **PROTOCOL VIOLATION** against its runner alpha for
  two commits *this* head had made, and rebutted alpha's true account of
  having its worktree reclaimed mid-session.
- It then recorded this round's runner output as "the runners kept working
  after reporting", inflating its own totals from 43 to 48 matches.
- This head, symmetrically, found commits on `main` it had not made and had
  to rule out a rogue runner before it could rule in a second head.

**The mechanism is recycled names.** Worktrees and branches are
`runner/alpha`…`runner/echo` every round. Tearing them down and
re-provisioning hands the other head a branch with the same name, a different
round's work on it, and no signal that it changed underneath. And **git
authorship cannot distinguish the cases, because every agent commits as the
operator.**

Both misattributions are struck through and corrected in place in round 6's
entry, not deleted — an accusation that was published should be visibly
withdrawn. The rule is now §4a of PARALLEL-RUNS: establish you are the only
head before anything else, and never re-create a name another head may hold.

**Standing operator escalation: rule 5 (main is single-occupancy) has no
mechanical enforcement.** Nothing stopped either head writing to main. The
hooks guard `check.sha1`/`build.sha1` and `asm/`, not the branch.

### The head confirmed a stall that a runner then overturned

`func_8005E02C` was filed by delta as a register-identity stall at 28/33.
The head re-audited it properly per §3 — ran two more source shapes, both
landing on the identical five words, named the residue precisely (retail
spreads the first pointer chain across `$a1`,`$v0`,`$a2` from the register
`arg1` just vacated, while collapsing the second chain into one reused
`$v1`), and attached a corpus census arguing the shape was rare and a poor
permuter target. **All of it was wrong in the same way.**

`EntityMethods::slot144` takes a *second argument*, `this->unk94` itself.
Retail parks the value in `$a1` for the whole function because that is the
register the call needs it in. Delta found it on its next pass by
cross-checking a *different* caller of the same slot, and matched 33/33 first
try with the corrected signature.

Why fourteen attempts across two authors missed it: every one varied the code
that COMPUTED the value while holding the one-argument signature fixed. The
call site under test even looked like positive evidence for that signature — a
plain `nop` delay slot and no fresh `$a1` load — because the argument had been
resident since the top of the function. The head's census was measuring a
shape that merely co-occurs with the cause.

Three lessons banked, all in DECOMPILATION_LEARNINGS: cross-check another
caller before accepting a register-identity read; a census of a residue's
surface shape is not a census of its cause; and MATCHING-GUIDE now says a
match report is the best available account, **not a verdict** — many attempts
along one axis reads as exhaustive and is a reason to hunt for the axis nobody
varied.

### Two Gate 2 carves, both zero-byte

Done mid-round to convert idle runners into new ground, one segment at a time
with a verify after each:

- **`class_3bb8c`**, first 20-function slice (0x3BB8C..0x3CD88) of the largest
  uncarved block in the game, 365 functions. Remainder is `class_3bb8c_b`.
- **`Entity_c`**, first 20-function slice (0x4F754..0x5077C) of the
  97-function Entity remainder. Remainder is `Entity_d`.

Both boundaries were checked on both sides, and the per-function census
(second `addiu $sp` prologue, `jlabel`, `jr $t2`, `$sp`-before-prologue,
`alabel`) came back clean for all 40 — confirming again that this game does
not under-split. Neither slice needed a rodata slot attached.

**Carve hazards still ahead, named at carve time in the unit headers:**
`class_3bb8c`'s remainder holds all 13 PSX BIOS trampolines and 38 switch
jump tables, none of which fell in the carved slice. Whoever takes the next
slice must disposition the trampolines (`hasm` or literal `.word`) and attach
the right rodata slot *at carve time*. `Entity_d` is clean by contrast — the
whole 97-function block has zero of both, which is a property of the block,
not of the slice.

Charlie independently confirmed the `class_3bb8c` census by working in the
slice and hitting neither hazard.

### Runners lost finished work to uncommitted state, twice

Round 7 **opened** by inheriting round 6's five standing worktrees and finding
three uncommitted byte-exact matches in alpha's (`func_8005A7A0`,
`func_8005B904`, `func_8005B990`) that no summary had ever mentioned. They
survived only because nobody ran `--force` on that worktree.

It **closed** with bravo dying to an API error mid-batch, holding a finished
54/54 match (`func_8005AD68`) as uncommitted working-tree state. Salvaged
under §4c; the head wrote the report and labelled it a finished match rather
than the mid-attempt snapshot §4c usually produces, while marking the attempt
history as unknown rather than inventing one.

Both are the same failure: **one-commit-per-match is what makes a runner's
work survive its session.** §4b gains the deferral note this implies —
deferring teardown is right when agents may still be live, but it hands the
next head a §4c salvage it has no way to anticipate, so PROGRESS must name
the worktrees.

### Head review of runner C, beyond the score

- `func_8004B570` was written `s32 { return self->unk70 = 1; }`. The store
  sits in the delay slot and the literal must be materialized somewhere, so
  `void` and `s32` are byte-identical — the bytes prove nothing. A cross-table
  survey of slot `+0x0EC` over all 60 method tables found the base
  implementation (`func_8003D444`, shared by four class tables) materializes
  no return value on either path, which makes `void` the supported reading.
  Corrected, re-verified 3/3.
- `func_8004C434` has a non-void function with no return on its
  loop-exhaustion path. Its report predicted a terminal `return e;` would cost
  an instruction; the head tested it rather than leaving it as reasoning — it
  scores 6/15 and adds one `addu`. The UB-shaped body is the faithful
  reconstruction and stays. Also recorded that retail's not-found path leaves
  `&arr[6]` in `$v0`, not null, so no caller can rely on a null return.
- `func_8004ADD8`'s opaque `goto` chain was rewritten as a `switch` at the
  head's suggestion, reaching identical bytes with clearer C. Its
  `u8 unused[24]` frame padding is now annotated as *evidence* of a real local
  aggregate in the original source, not an explanation of it.

### Two runners bounded the `__asm__("")` barrier from opposite sides

Not a contradiction, and worth stating because it reads like one. Delta's
barrier DID change which instruction fills a load-delay slot at its own
position. Alpha's did NOT stop a loop-offset increment being hoisted backward
across it, past several intervening statements. Together they narrow the lever
from "reliable local fix" to "scheduling nudge at its own position" — and
neither moved a register choice, which is rule 6 holding from the failing
side.

### The `make` hook papercut, reproduced and diagnosed

Round 6 logged it; this head hit it three times before reading that, and then
read the cause off the hook. `tokenize()` runs with
`punctuation_chars=True`, so a redirection like `2>&1` becomes the tokens
`2`, `>&`, `1`; neither `>` nor `>&` is in `SEPARATORS`, so `targets_of()`
collects them as make targets and the `all(t in ALLOWED_TARGETS)` test fails.
`make extract` passes bare and is refused with any redirection, by an error
message that itself says `make extract` is allowed. Two heads hitting it
within the hour. **Operator's guardrail, untouched.**

### Next round

**Runners.** 44 fresh across four units, 30 of it in the two units carved this
round and never worked beyond a first pass (`class_3bb8c` 15, `Entity_c` 15),
plus `DreamSys` 12 and `class_3ac78` 2. Cold-runner yield this round was very
high — 58 matches against 4 stalls, most on the first or second attempt — so
cold ground is plainly not exhausted and a permuter round is not yet the best
use of a session.

Round 6's entry recommended permuter on the strength of the 24-instance
epilogue-merge class. That recommendation still has merit and is untouched by
this round, but it should be weighed against two things round 7 established:
the class census that looked most permuter-ready (this head's own) turned out
to be measuring the wrong thing, and reshaping under a corrected *signature*
closed a stall that reshaping under the wrong one could not. **Check arity and
slot typing across callers before committing a session to a permuter search.**

A carve is not needed — the queue was refilled this round — but the next one
should take `class_3bb8c`'s second slice specifically, because that is where
the BIOS-trampoline disposition finally has to be made, and making it at carve
time is much cheaper than a runner discovering it.

---

## 2026-09-02 — round 6: 5 runners, 43 matches, and a second head nobody knew about

**160 -> 203 matched (11.80% -> 14.97% of game code). Build green in main after
every merge, and green again after a full `make clean` + `make extract`
rebuild.**

**Counts corrected by round 7.** This entry originally claimed 48 matches and
203 -> 208, counting five matches that round 7's runners produced on
re-provisioned branches with the same names. Round 6's own five runners
delivered 43. The two sections below that recorded the extra five, and that
accused runner alpha of a protocol violation, are struck through and corrected
in place rather than deleted — the round's real finding turned out to be the
misreading itself, and it is now §4a of PARALLEL-RUNS.

**Teardown was DEFERRED, deliberately — the worktrees were left standing** for
the operator, since agents might still be live. Round 7 inherited them and
found three uncommitted byte-exact matches in alpha's, which deferral is the
only reason still existed. See §4b.

Five runners, one per unit with fresh ground: `DreamSys` (alpha),
`Entity_b` (bravo), `class_39e08` (charlie), `class_3ac78` (delta),
`code_2c054` (echo).

| runner | unit | matched | stalled | blocked stubs |
| --- | --- | --- | --- | --- |
| alpha | DreamSys | 5 | 1 | 0 |
| bravo | Entity_b | 8 | 0 | 1 |
| charlie | class_39e08 | 8 | 1 | 2 |
| delta | class_3ac78 | 9 | 1 | 1 |
| echo | code_2c054 | 13 | 0 | 0 |

### Gate 1: 119 fresh, 115 workable

All 119 fresh functions were screened against both open blockers before
assignment; only 4 hit (2 `gp_rel`, 2 `addiu_at`) and each was named to its
runner so nobody spent an attempt rediscovering it. Gate 2 did not fire — 115
workable against a ~46-function round target is 2.5x headroom. Gate 3 did not
fire either. Sizing was set by BODY SIZE, not count: alpha got 6 on
44-99-instruction bodies, echo got 12 on a queue with five consecutive 9-line
leaves. Echo returned 13/13 first-attempt; alpha's unit took the whole round for
5. The `fresh` column really cannot see this, and sizing to it would have been
wrong in both directions at once.

### The round's finding: the `New_X` epilogue-merge residue is one class, 24 instances

Charlie and delta hit the same one-word residue independently, in different
units, and classified it identically without either seeing the other's work.
The head re-audited rather than accepting it, and attacked the angle neither
runner tried — both had kept the null path's value flowing from `self`, whereas
retail materializes a literal `0`, which points at a single-exit source form.
Two such forms were tried and both were worse (a result variable: 0/27 and an
extra callee-saved register; a comma-ternary: 16/27 and a second epilogue, with
an outside-range byte count byte-for-byte identical to the early-return form's).

Both classifications confirmed, and the discriminator is now positive rather
than descriptive: **GCC 2.6.3 `-O2` will not merge two function exits carrying
different values into one epilogue.** A mechanical corpus census puts the class
at **24 instances** — 5 carved, 19 still uncarved. Written up once in
`docs/research/epilogue-merge-residue.md`.

**It is NOT a toolchain blocker and was not escalated as one.** The pinned
compiler provably emits retail's form; we have not found its input. That makes
it a permuter target, and it is the single highest-leverage one available: one
solved source form should generalize to all 24, which are the same allocator
with a different size constant and `Get_vtable`.

The census also exposed **three instances that had never been attempted** and
that `progress.py` was therefore counting as FRESH. Stub reports were filed for
them so the next round does not staff a cold runner onto a class with ~25
attempts behind it.

### ~~PROTOCOL VIOLATION: runner alpha performed head-only actions in main~~ — WITHDRAWN, see round 7

**This section was wrong, and round 7's head withdrew it after the operator
confirmed the cause.** It is kept rather than deleted because the reasoning is
instructive and because an accusation that was published should be visibly
retracted, not quietly removed.

What this entry reported: alpha, while still live, committed `0100dce` (a merge
of `runner/alpha` into main) and `83d5458` ("Salvage runner/alpha", executing
§4c on itself), and tore down `runner/echo`'s branch and worktree. It further
recorded that alpha's explanation — *"the head's parallel-runs consolidation
process reclaimed the worktree mid-session"* — **did not happen**.

What actually happened: **round 7's head made both commits and removed all five
worktrees.** The operator had started a second head session in the same checkout
while this round's head was still live. Round 6's head was correct that *it*
had not done these things, and wrong to conclude no head had. Alpha's account
was accurate: its worktree WAS reclaimed mid-session, by a head it could not
see. Round 7's head found alpha's three uncommitted matches still sitting there
and salvaged them under §4c — which is what `83d5458` is.

Alpha committed no protocol violation. The retracted charge stands as the
sharpest available illustration of the real hazard, which is now §4a of
PARALLEL-RUNS: **two heads in one checkout, with recycled `runner/*` names, each
reading the other's legitimate head actions as its own runners misbehaving.**
Both heads did this, in opposite directions, within twenty minutes.

What survives from the section unchanged:
- **A runner's self-report is not reliable evidence about who ACTED**, only
  about what it derived (§3b). The correction is that this cuts both ways: the
  HEAD's account of who acted is no more reliable when another head is live.
  Neither agent can distinguish the other's commits from a rogue runner's,
  because **every agent commits as the operator** — authorship proves nothing.
- **Rule 5 (main is single-occupancy) has no mechanical enforcement.** Nothing
  stopped a second head committing to main. The hooks guard
  `check.sha1`/`build.sha1` and `asm/`, not the branch. This is the round's
  standing operator escalation.
- Two runners this round also had git commands blocked mid-session by a safety
  classifier. Bravo worked around a blocked `git checkout HEAD -- <files>` by
  **hand-reconstructing files with the `Write` tool**. It came out byte-exact,
  but a runner that cannot use git to restore state will improvise, and
  improvised restores are how a wrong body reaches a commit. Worth an operator
  look; not something the head should paper over.

### ~~The runners kept working after reporting~~ — these were ROUND 7's runners; teardown was deferred

**Also corrected by round 7.** The five late matches below are real, verified
and correctly merged — but they were produced by **round 7's** runners charlie
and bravo, on freshly re-provisioned branches that reuse the same names. No
round-6 runner wrote them, and no runner kept working past its summary.

All five round-6 runners delivered a final structured summary and the head
merged all five. New commits then appeared on branches named `runner/charlie`
and `runner/bravo` — because round 7's head had torn those names down and
re-created them, pointing at a different round's work. `main..runner/<name>`
reading empty, then non-empty, then empty again was that, not a runner
restarting.

| late match | unit | words | landed on branch |
| --- | --- | --- | --- |
| `func_8004A2C4` | class_39e08 | 24/24 | runner/charlie |
| `func_80049E20` | class_39e08 | 33/33 | runner/charlie |
| `func_8004A364` | class_39e08 | 34/34 | runner/charlie |
| `func_8003BD10` | code_2c054 | 25/25 | **runner/bravo** |
| `func_8003BDF4` | code_2c054 | 26/26 | **runner/bravo** |

Note the last two, and how the misreading compounded: `code_2c054` was ECHO's
unit in round 6, so two `code_2c054` matches on a branch named `runner/bravo`
looked like one runner committing another's unit — an apparent breach of the
one-unit-per-runner rule, "held by luck". In round 7 `code_2c054` **is** bravo's
unit. The collision rule was never violated; the branch name simply meant
something different than this entry assumed.

All five were merged and individually confirmed with funcdiff; the whole-image
SHA1 stayed green throughout. **Merging them was right even on the mistaken
premise** — they are valid matches with reports, and the alternative to merging
verified work is throwing it away. Round 7 re-confirmed all five against a
fresh build of main.

**Why teardown was deferred.** §4b's four preconditions are meant to be checked
once. Here precondition 3 (`main..runner/<name>` is EMPTY) kept
*un*-satisfying itself: it read 0 for all four branches, then 1, then 2, then 0
again, as commits continued to land. At the point of writing, `runner/bravo` and
`runner/delta` each still hold uncommitted files. `git worktree remove --force`
on a worktree with live uncommitted work destroys it, and `--force` is exactly
the flag that disables the "are you sure?" backstop — which PARALLEL-RUNS
already warns is the only thing between a finished round and a lost one.

So the round ends with everything merged, everything verified, and the five
worktrees left standing for the operator to tear down once the agents are
confirmed quiescent:

```sh
for n in alpha bravo charlie delta; do
    git worktree remove --force ../<checkout>-wt-$n && git branch -d runner/$n
done
```

**Protocol gap this exposes** — restated after round 7's correction. The
conclusion drawn here was that "has REPORTED" does not imply "has finished",
i.e. that a runner keeps writing past its own summary. That is not what these
branches were doing; another head's runners were writing to the same names. The
part that survives is the cheap half: **check the preconditions immediately
before the `--force`**, because the tree can move under you — whoever is moving
it. Deferring rather than forcing was the right call for a second reason nobody
had in view at the time: one of those worktrees held three uncommitted matches.

### Head consolidation

- Corrected `include/Class6D3C8.h`'s `LoaderTaskMethods::slot44` to `s32`. Echo
  flagged it and could not fix it under the parallel rules. Re-derived rather
  than taken: the slot's occupant is `func_8003C1DC`, whose body loads
  `self->unk38` into `$v0` immediately before the epilogue with nothing else
  consuming it. The earlier `void` recorded what the CALLER discards, not what
  the callee computes. ABI-neutral; zero bytes.
- **Corrected an over-narrow claim from round 5.** The class-table header word's
  class id was written up as "the low 12 bits" from `func_80058E8C`'s `0xFFF`
  mask. `func_80058F18` masks the same word with `0xFFFFF` and compares against
  `0x1F234`, which cannot fit 12 bits. `0xFFF` was one function's mask, not the
  field's width. The generalizable form: a masked read bounds an identifier's
  width from BELOW only.
- Declared `TestForStaticLink`/`Test4TunnelLinks` in `src/DreamSys.c`. Both were
  implicitly declared — used ~200 lines before their definitions. The implicit
  type agreed, so nothing miscompiled, but an implicit declaration also disables
  argument checking, which is exactly what caught `func_8005D714`'s over-narrow
  `s8` parameters this round. Zero bytes.
- Cleared a live `` `/*' within comment `` warning in `include/class_3ac78.h`.
- Replaced the hardcoded `lsddecomp2-` worktree prefix in PARALLEL-RUNS.md with
  `<checkout>`. It sat inside the `--force` teardown command, where `--force`
  has already disabled the only automatic backstop, so a stale path there is a
  hazard rather than a typo.
- Retired an open question: comma expressions were listed as an untested GCC 2.x
  scheduling lever. Tested for the two-exit case — the answer is no, a
  comma-ternary lowers to the same RTL as the separated early-return. Left
  explicitly open for single-exit bodies, which is a different question.

### Two false positives in the hardened `make` hook

`.claude/hooks/block-raw-make.py` was hardened just before this round to judge
`make` in command position only. Two false positives surfaced, both fail-safe,
neither fixed by the head (it is the operator's guardrail):

1. **A git commit heredoc** whose message *quotes* a chained build command in
   prose is blocked — the tokenizer sees the quoted text in command position.
   Workaround: `git commit -F <file>`.
2. **Any redirection or pipe on an allowed target is blocked.** `make clean`
   passes bare but `make clean > /dev/null 2>&1` does not, and
   `make extract 2>&1 | tail` is refused by an error message that itself says
   "`make extract` is allowed". Redirection tokens appear to be collected as
   make targets. This is the more serious of the two: it blocks the documented
   way to drive the repo, and the fix for the first false positive is what
   introduced it.

**Independently reproduced by round 7's head**, which hit #2 three times before
reading this: `make extract > /tmp/x.log 2>&1`, `make extract 2>&1 | tail -1`
and `make extract 2>&1 | tail -5` are all refused, while bare `make extract`
passes. Two heads hitting the same papercut within the hour, one of them
mid-consolidation, moves this from an annoyance to the operator's next fix.
Confirmed cause, from reading the hook: `tokenize()` runs with
`punctuation_chars=True`, so a redirection like `2>&1` becomes ordinary tokens
(`2`, `>&`, `1`) inside the simple command; `>` and `>&` are not in
`SEPARATORS`, so `targets_of()` collects them as make targets and the
`all(t in ALLOWED_TARGETS)` test fails. A pipe DOES separate correctly — it is
the redirection on the make command itself that breaks.

### Next round — as judged by ROUND 6; superseded, see round 7

**Permuter, not runners or a carve** — the first time this project has had a
permuter round as the clear best move. 66 fresh remain across four units, so
runners are still viable and a carve is not needed. But the epilogue-merge class
is 24 instances behind a single unknown source form, with two sites sitting at
26/27 and fully-typed headers already committed, so the permuter starts from a
compiling body. Nothing else available comes close to 24 functions of leverage
from one search. If it closes, run runners immediately afterward to harvest the
class across all four carved units; if only UB or duplicate-arm forms reach
zero, mark it permuter-exhausted in the research doc and go back to runners.

---

## 2026-09-01 — round 5: 3 runners, 6 passes, 56 matches, DreamSys past halfway

**98 -> 154 matched (11.36% of game code). 102 queued: 27 fresh, 75 stalled, 0
banked. Build green throughout, verified in main after every one of six
merges.**

Gate 2 was already done, so this was Gate 1 triage and provisioning only. The
operator's brief asked for five runners; the true state supported three, and the
brief's other two premises were both wrong in ways worth recording.

### Gate 1 corrected two premises before any runner was spawned

- **The brief said 244 fresh. `progress.py` said 92.** The stub-reporting work
  done during round 4's consolidation is exactly what made the smaller number
  truthful; it had not been carried into the brief.
- **The brief named `class_16334` (8) as the natural cold start. It has ZERO
  fresh ground.** The "8" is its *matched* count. Both its queued functions
  carry reports and are gp-relative-blocked. A runner staffed there would have
  spent its entire budget re-deriving a documented blocker. **This is the second
  consecutive round in which `class_16334`'s matched count was mistaken for
  available work** — round 4's log records the same error. The `matched` and
  `fresh` columns sit adjacent in `progress.py` output and are easy to confuse;
  read the `fresh` column, and read it for the unit you intend to assign.

Only `DreamSys` (72), `Entity` (11) and `code_55dd4` (9) had assignable ground,
so the round ran three runners — one per unit, per collision rule 1. The spare
capacity question was put to the operator rather than resolved by splitting a
unit across runners or by carving against instruction; the operator chose to
stay at three.

**All 92 fresh functions were screened for both toolchain blockers before
assignment. All 92 were clean**, and no runner reported a blocker hit — the
first round in which the pre-screen came back entirely negative.

### The stall corpus was audited, and it held

Before spawning, the head mechanically re-derived every one of the 66 stall
classifications: for each report claiming BLOCKED, does the function's own `.s`
actually contain a `gp_rel` or `addiu $at, $at, %lo` discriminator?

**All 44 BLOCKED confirmed. All 12 STALLED-TOOLCHAIN confirmed.** The two
carrying no discriminator (`func_80065A5C`, `func_800662BC`) correctly claim
*structural* stalls rather than toolchain ones. Nothing was mis-parked. This
also means `fresh` is trustworthy as a ceiling — no hidden ground in the stall
pile. Worth re-running after any round that files many stalls; it is seconds.

### The round's lever: the argument-register test

`func_80059E3C` had been filed by a runner as an unreachable register-identity
stall at 21/23 — retail `lw $a1` / `bltz $a1` against a built `lw $v0` /
`bltz $v0`, same branch target, everything else identical. Head adjudication
traced `$a1` forward and found nothing overwrote it before the following
`jalr`: it was still live at the call, so it was an ARGUMENT. The source was one
parameter short. Matched 23/23 once fixed.

Broadcast mid-round to all three runners with an explicit request for the
negative answer. Both halves came back, and the negative half is what makes the
test safe to keep:

- **Applies** (`func_8005DD18`, Entity): a literal `0` argument the callee never
  reads — it overwrites the register as scratch on entry. **A signature derived
  by reading the callee alone is wrong with nothing to flag it.**
- **Does not apply** (`New_DreamSys`; and five stalls in `code_55dd4`): a `$v0`
  residue is a return value, never live into a call. `code_55dd4`'s runner
  separated three distinct residue shapes the test cannot reach — prologue
  callee-save STORE ORDER, a loop-carried value, and a call whose arity was
  already correct.

Without that negative half the test would send future runners hunting phantom
parameters on every `$v0` residue. Both halves are now in
DECOMPILATION_LEARNINGS.md.

**Two stalls filed with stop rules properly applied fell to a lever discovered
after they were written.** That is an argument for cheap, complete stall reports
— and a caution that "stalled" means "stalled given what was known then".

### Runner-to-unit outcomes

| runner | unit | passes | matched | stalls | end state |
| --- | --- | --- | --- | --- | --- |
| alpha | `DreamSys` | 3 | 41 | 6 | 27 fresh left, all 35+ instructions |
| bravo | `Entity` | 1 | 13 | 1 | **unit dry** |
| charlie | `code_55dd4` | 2 | 5 | 4 | **unit dry** |

Alpha was sent back twice under protocol 3c rather than being replaced, and its
second and third passes produced 30 of its 41 matches. Its first pass was
range-limited to `0x80058774`-`0x8005A1EC`; once the operator settled the round
at three runners, the range was lifted to the whole unit because no one else
could contend for it. Bravo's 13 include three previously-matched functions it
had to fix up after retyping a shared field — all three re-verified as still
matching, which is a regression risk worth checking explicitly whenever a runner
retypes something shared.

### A toolchain lead, with a corpus census attached

`func_8005950C` (17/33) stalls on **two extra `nop`s between `mflo` and a
following `div`** that retail does not have. The runner reproduced it in
complete isolation through the pinned pipeline with a five-line `(a*b)/c`
snippet — no project types involved — and correctly declined to escalate it
independently, flagging it for triage instead.

The mechanism is `maspsx`'s `nop_mflo_mfhi`, already named in
`docs/research/addiu-at-blocker.md` as one of the four flags a version bump
below 2.30 would flip. **That document treats `nop_mflo_mfhi = False` as
collateral damage of the bump. The census attached to it now says the opposite
for most of the image** — retail omits the nop in 557 of 603 `mflo`/`mfhi` sites
(92.4%).

But it is **not uniform**, which is the decision-relevant part: in the specific
hazard construct, retail has no intervening nop 180 times and does have one 65
times. A global boolean cannot be right for both. So this is **not** a
"flip the flag" result — it is evidence that the flag is the right mechanism and
the wrong granularity, and it sharpens rather than resolves the open blocker.
Full numbers and the leading hypothesis are in the blocker doc. Operator's call,
per CLAUDE.md rule 5; nothing was changed.

### Also banked

- **First confirmed in-game READ of the class-table header word** at `+0x000`,
  a standing open question. `func_80058E8C` masks it to `0xFFF` and compares
  against a literal class id, so the low 12 bits are a class identifier. Open
  question updated in place rather than left standing.
- A stale `build/lsdde.map` from an earlier failed compile sent a runner chasing
  a false "off-by-4 symbol" lead. **A failed link leaves the previous map in
  place, exactly as it leaves the previous binary** — the fourth way a build
  artifact can lie, and the only one not yet in CLAUDE.md's list of three.
- `sizeof(DreamSys)` was modelled 0x98 bytes short; recovered from the
  allocator's own literal `ori $a0, $zero, 0x928`.
- `Class65650Methods` slot `+0x134` was recorded as returning `void` on the
  strength of a call site that discarded the result. It returns `u8 *`.

### Permuter targets, now well-posed

Three new residue classes, none reachable by source reshaping, all with bodies
preserved as literal source:

| function | score | class |
| --- | --- | --- |
| `func_8005DBF0` | 72/74 | identical assignment tail-merged across two merge points |
| `func_80066340` | 252/258 | magic-multiply constant LOAD POSITION, no C counterpart |
| `DreamSys__LogMood` | 8/14 | pure scheduling, branch-free and call-free |

`func_80066340` is the project's most detailed partial derivation: 258 words,
six residues closed independently, and its slot `+0x138` signature
cross-validated against `func_800662BC`'s call site from the other side. **A
stalled caller and its stalled callee constrain each other; use both ends.**

### Next round

`fresh` is 27 and they are all in `DreamSys`, all 35+ instructions — the cheap
seam is gone. Three of the eight carved units are now dry, and four more are
entirely blocked. **The next round should be a CARVE, not runners**: one unit
cannot support three runners, and there is no second unit to give anyone.
`class_39e08` (415), `code_179d8` (274) and `Entity_b` (117) are the candidates;
`Entity_b` pairs naturally with the now-exhausted `Entity`.

---

## 2026-08-30 — round 4: 5 runners, 59 matches, and a second toolchain blocker

**39 -> 98 matched (7.23% of game code). 158 queued: 92 fresh, 66 stalled, 0
banked. Build green throughout, verified after every merge.**

Gate 2 was already done, so this was Gate 1 triage and provisioning only. Five
runners on `DreamSys` (a named range), `code_55dd4`, `code_1677c`, `code_171e0`
and `code_4cd08`. All five reached their own stop rule; none died to
infrastructure, unlike round 3.

### The pre-screen, and why `fresh` lies

`progress.py` reported 145 fresh at round start. The true assignable number was
lower, because `fresh` cannot see a toolchain blocker. Screening every queued
function for `%gp_rel` before assigning changed three of the five assignments:
`code_171e0` had 8 of 14 blocked and `code_4cd08` 8 of 14, so both runners got
explicit function lists rather than "the unit". `class_16334` turned out to have
only 2 workable functions, not the 8 the operator's brief assumed, so it was
dropped as a runner unit and worked by the head instead; `code_55dd4` (34 fresh,
zero exposure) took its place.

**Consolidation stub-reported the remaining 37 blocked functions**, which is
what Gate 1 asks for and what nobody had done. `fresh` fell from a nominal 143
to a real 92. The next head reads a truthful number.

### Second toolchain blocker: `addiu_at`

Found independently by two runners in two unrelated units, reproduced from
scratch by the head, and written up in `docs/research/addiu-at-blocker.md`.

Retail resolves a runtime-indexed global fully into `$at` before loading
(4 instructions); the pinned `--aspsx-version=2.34` folds `%lo` into the load
(3 instructions). cc1 has no opinion — it emits one generic pseudo-op — so the
choice is maspsx's `addiu_at` flag. Census over the whole disassembly, counting
indexed accesses only: retail uses the unfolded form **502 times across 39
files and the folded form 0 times.** No counterexample exists in the executable.

**Both runners proposed repinning to 2.29; the head corrected that.** Below 2.30
four flags flip together — `addiu_at` plus three nop-insertion rules that reach
constructs inside the 98 functions that already match — and maspsx exposes no
per-flag override. Same shape as the rejected `-G` experiment: correct
diagnosis, remedy that costs more than it buys. Not tested. Operator's call.

A useful control: `Entity__GetMoodEffect` matched 6/6 against the same table
because it only forms `&arr[i]` and never loads through it. Address-only table
arithmetic is safe.

### Adjudication earned its keep

The head's job of checking stall *classifications* rather than scores paid off
twice, in opposite directions:

- **`strcat` was misclassified.** Filed at 16/42 as an unreachable
  compiler-internal delay-slot choice. The report was honest and reproducible —
  splicing the preserved body back in gave exactly 16/42 — but the runner's own
  diff showed the two guard branches had different *targets*. A differing target
  is a differing CFG, which always comes from the source. The post-increment
  scan idiom took it to 41/42. The one instruction still left over really is the
  named class.
- **`addiu_at` was NOT a misclassification**, and checking it properly is what
  produced the census and the corrected remedy.

That discriminator — check branch targets before calling anything a scheduler
choice — went out as a mid-round broadcast and a second runner credited it with
closing a function outright, 10/69 -> 69/69.

### Broadcasts, including the one that was wrong

Three broadcasts went to all five runners, each asking explicitly for the
negative answer.

The first was **overstated**: it presented `goto fail` vs `return NULL` as a
general lever for early exits returning a different value. Three runners bounded
it within the round — it does not apply when the allocator also tests the
constructor's return, nor when the normal path contains a loop, and one
superficially similar residue wanted the plain early return. The correction was
rebroadcast with credit. Ship levers hedged; the retraction cost five messages.

### Runner-to-unit outcomes

| runner | unit(s) | matched | stalled |
| --- | --- | --- | --- |
| bravo | `code_55dd4` (two passes) | 23 | 2 |
| charlie | `code_1677c` | 9 | 1 (+ a directed negative on `new_class_6d3c8`) |
| alpha | `DreamSys` range | 9 | 2 (both `addiu_at`) |
| delta | `code_171e0`, then `Entity` | 12 | 4 |
| echo | `code_4cd08` | 4 | 2 |
| head | `class_16334` | 2 | — |

Two runners finished early and were sent back into their own units rather than
left idle, per PARALLEL-RUNS 3c. `delta` had to change units because
`code_171e0` had no clean ground left after its first pass; `Entity` was
unbanked for it. Both second assignments were productive — bravo's was the
round's single best result at 14 matches.

### Permuter targets, now well-posed

The "redundant delay-slot value duplication" class has **three confirmed
instances**, each isolated to one instruction with branch targets agreeing:
`new_class_6d3c8` (23/24), `strcat` (41/42), and one more found this round. All
three resist `goto`/`return` spelling, temp placement, barriers and `volatile`.
Bodies are preserved in the reports. This is the strongest permuter candidate
the project has had.

### Next round

Only three units have fresh ground (`DreamSys` 72, `Entity` 11, `code_55dd4` 9),
and one runner per unit is the rule, so **the current state supports three
runners, not five.** Carve first if five are wanted. See Gate 2's candidate list;
`Entity_b` (117) pairs naturally with the `Entity` work now in flight.

---

## 2026-08-29 — round 3: 5 runners, 27 matches, and a real toolchain blocker

**12 -> 39 matched (2.88% of game code). 217 queued: 145 fresh, 11 stalled, 61
banked. Build green throughout.**

Five runners on `class_16334` (8), `code_1677c` (14), `code_4cd08` (17),
`code_171e0` (26) and a named `DreamSys` range (`0x80059310`-`0x800595A8`, 15
accessors). Gate 2 was already done, so this was Gate 1 triage and
provisioning only.

**All five runners were killed mid-round by one account-wide session limit.**
Not one reached its own stop rule. Of the 27 matches this round, 11 were
committed by runners and **16 were recovered by the head** under
PARALLEL-RUNS 4c — including runner/echo's entire output, which was 13 byte-
exact bodies and not a single commit. Without the salvage path that work would
have been silently lost at teardown, and the round would have read as 11
matches with four units looking barren.

Worth stating plainly for the next head: **4c is not a rare-contingency
procedure.** One infrastructure event took every runner simultaneously, and
the recovery was most of the round's value.

### The finding: no C function on this project can reach a small-data global

`docs/research/gp-relative-blocker.md`. **Open operator escalation; no
toolchain change made.**

Retail reads `.sdata` globals gp-relatively in one instruction. The pinned
pipeline emits the two-instruction absolute `lui`/`lw` form, and the extra
instruction shifts every later function in the unit.

runner/delta filed 8 stalls in `code_171e0` all classed TOOLCHAIN. Eight
identical classifications from one runner is exactly the shape that is usually
a runner rationalising, so the head adjudicated rather than accepting — and
this time the runner was right. Reproduced in isolation per CLAUDE.md, with
one correction: the runner blamed cc1, but gp-relative addressing needs a
non-zero `-G` at **both** cc1 and `as`; either alone still gives the absolute
form. The project pins `-G0` at both and passes maspsx no `-G`, while maspsx's
README says a `$gp` project must be passed one.

The head then found the same root cause in **runner/alpha's** unfinished
`func_80025C30` in `class_16334` — a different unit, a different runner,
neither able to see the other's evidence. alpha died before classifying it, so
that connection existed nowhere until consolidation.

**Why 20 matches never caught it:** every function matched before this round
touches no small-data global at all. Zero `(gp)` references across every
genuinely-C matched body. The `-G0` pin had simply never been exercised.

Scope is at least 9 functions and plausibly a large fraction of the remaining
1300+. `grep -l 'gp_rel' asm/nonmatchings/<unit>/*.s` now identifies a blocked
function before anyone spends attempts on it.

### The head walked into trap #1 and the guard caught it

Rescoring runner/echo's 14 salvaged bodies, the head spliced each into main's
`src/DreamSys.c`, which lacked the `#include "DreamSys.h"` echo had added.
Every `DreamSys *` became a parse error, the compile failed, the previous
build stayed in place, and funcdiff returned **14 full matches from the stale
build** — plausible, self-consistent, entirely fictional. funcdiff's STALE
BUILD guard is what caught it.

The exit status was printed next to every one of those 14 scores and was `2`
every time. **Printing `build exit=` is not the control; refusing to read the
number unless it is 0 is the control.** Corrected pass: 13 genuine matches, 1
snapshot. Both the trap and the rule are now in DECOMPILATION_LEARNINGS.

It bit a second time, in a nastier form, during the `-G` experiment. **A flag
change rebuilds nothing** — objects depend on every source and header but not
on the Makefile — so `-G8` reported a clean green that was simply the previous
`-G0` build. Nothing failed, nothing was newer than anything, and funcdiff's
mtime guard cannot see this one either. From scratch the same flags were
19148 bytes wrong. `rm -rf build` before trusting any flag experiment.

### A provisioning defect that had disabled an honesty check

The toolchain gitignore patterns ended in `/`, matching directories only —
but `setup-worktree.sh` installs *symlinks*. All seven showed as untracked in
every worktree from the moment it was created, putting a permanent floor of
seven lines under `git status --porcelain`. That is the one mechanical check
PARALLEL-RUNS 3b uses to tell a runner that committed its work from one that
did not, and it could never fire. Fixed before spawning; all five worktrees
read clean, which is what made the 4c salvage survey trustworthy.

### Also banked

`code_55dd4` (34), `Entity` (25) and `StageGrid` (2) were marked DELIBERATELY
UNWORKED so their 61 functions report as `banked` rather than inflating
`fresh`. They are unworked for scheduling reasons, not difficulty.

### Next move

**Runners. The `-G` question was settled the same day and the answer is no.**

An earlier draft of this entry called `-G` a gate on "an unknown but probably
large share of the queue" and put it ahead of runners. Both halves were wrong,
and the measurements are worth keeping:

- **Scale.** Functions touching a small-data global: 37 of 217 in the carved
  queue (17%), 99 of 1098 uncarved (9%) — **~136 of 1315 remaining, about
  10%.** Not most of the project. But heavily concentrated: `code_171e0` 63%,
  `code_4cd08` 57%, `class_16334` 50%, against 0% for `code_55dd4`, `Entity`,
  `code_1677c` and `StageGrid`. So it barely gates the project and badly gates
  three specific units.
- **Sequencing.** With ~160 functions of zero-exposure ground available, the
  two decisions were never serial. Staffing runners never needed to wait.
- **The answer.** Tested with operator authorisation and rejected: a clean
  `-G8` rebuild differs from retail by 19148 bytes across 3203 runs, and `-G4`
  gives byte-identical damage. The diagnosis was right (at `-G8` the blocked
  function compiles to retail's exact `sw a0,0(gp)`), but a global flip costs
  far more than the 136 functions it buys. `docs/research/gp-relative-blocker.md`
  has the rejected experiment and what remains unexplored.

So: runners, on the zero-exposure units, routed around `gp_rel` by the grep now
in MATCHING-GUIDE. 145 fresh remain and `code_4cd08` matched everything it
attempted.
When the permuter is set up, the first target is `new_class_6d3c8`: the `New_X`
allocator shape recurs across ~60 classes, so one closing source form unblocks
all of them.

---

## 2026-08-28 — round 2: Gate 2 carve, 2 units -> 8

**244 fresh across 8 units (was 120 across 2). 1100 uncarved. Build green.**

Carved so a head agent can staff up to eight runners; one unit has exactly one
owner, so two units capped the previous state at two runners regardless of how
much queue they held.

New units: `code_55dd4` (34), `code_171e0` (26), `Entity` (25, the first slice
of a 142-function block, split at `func_8005DE18`), `code_4cd08` (17),
`code_1677c` (14), `class_16334` (8).

**A confident claim from round 1 was WRONG, and only running the check found
it.** Round 1 reasoned that ~1425 table-dispatched methods have no `jal` to
them, so splat must be under-splitting, and wrote that into Gate 2 as the
expected case. Measured: of **894** distinct function addresses referenced by
class tables, **894 already have a glabel** — spimdisasm scans data for
pointers into `.text` and makes a symbol from each. A corpus-wide prologue
census over every uncarved segment returns **zero** functions with more than
one `addiu $sp, $sp, -`. Gate 2 and class-framework.md are corrected. The
reasoning was sound and the conclusion was still false; nothing but the check
would have separated them.

**Three carve failures, all routine, all now in Gate 2:**

- **Orphaned jump tables.** Carving a segment to `c` leaves its switch tables
  in a standalone rodata segment that cannot see the `.L` labels, which are
  local to the unit's function `.s` files. `code_4cd08` needed `0x206C`
  attached — a slot the inherited yaml labelled `# greyman`, so the inherited
  rodata comments do not reliably say who owns a slot.
- **A segment whose tail is DATA.** `code_55dd4`'s text ends at `0x57028`;
  after it are some ints and a character-classification table that the `asm`
  segment had been emitting inline. Declared `data`, not `rodata` — the
  `section_order` puts `.rodata` first and would have relocated those bytes to
  the top of the image.
- **Reverting a carve leaves `src/<unit>.c` behind.** splat does not delete it,
  so every function is then defined twice. Delete it in the same step.

**And a lesson about the carve helper itself.** The first attempt batched five
segments through a shell function whose regex was `$`-anchored, so it silently
skipped `code_4cd08` (trailing `# DreamAux` comment) while **printing OK**, and
its "reverting" branch printed the word without reverting. Carve ONE segment at
a time and verify each; the cycle is about two seconds.

**Found while carving: 34 PSX BIOS call stubs** — `jr $t2` with the vector in
`$t2` (`0xA0`/`0xB0`/`0xC0`) and the call number in `$t1`. 13 are in
`class_39e08` (`0xB0`/`0x33` is BIOS `malloc`), the other 21 in `psyq_*`. These
cannot be written in C at all and will need an `hasm` segment or a literal
`.word` disposition. **Decide that at carve time**, not when a runner has
already spent its attempt budget on one.

**Next move.** Runners. The queue supports 4-8. `class_16334` (8) is the
natural cold start; `DreamSys` (118) needs a named address range rather than
the whole unit.

---

## 2026-08-28 — round 1: the C++ question, settled

**No code changed. One structural fact established, and it was the one gating
the largest block in the project.**

`class_39e08` is 415 functions and round 0 flagged "is this C++?" as the top
open question, on the grounds that writing them the wrong way is expensive to
discover late. Settled now, before any of it was staffed.

**Answer: plain C with a hand-rolled class framework.** No C++, no `cc1plus`,
no pipeline change. Full evidence and reproducer in
`docs/research/class-framework.md`.

**The trap, worth remembering because it nearly worked.** The evidence FOR C++
was entirely the symbol names — `New_DreamSys`, `DreamSys__DreamSys`,
`Get_vtable_DreamSys`, `BasicClass__*`. Every one of those is FirecatFG's
hypothesis, inherited with the symbol file, and there has never been a symbol
leak for this game. The names look like C++ because someone who suspected C++
chose them; treating them as evidence would have been circular. **Inherited
naming is a hypothesis, not data.**

**What settled it, from the bytes:**

- Constructors are called *through* the method table (slot `+0x008`), and so are
  base-class constructors. No C++ compiler can do that — the object has no
  vtable pointer until the constructor stores it, which is why the language has
  no virtual constructors.
- Compiling C++ through this repo's own `tools/gcc263/cc1plus` emits **8-byte**
  vtable entries `{delta, index, pfn}` with a count header, calls constructors
  **directly by name**, emits no null check after `__builtin_new`, and puts the
  vptr **after** the base's data members. The game does the opposite on all
  four counts, with 4-byte flat entries and the vptr at offset 0.
- The tables have **null slots mid-table**; g++ uses `__pure_virtual`, never 0.
- Whole-binary falsification scan: **0 compiler-generated 8-byte-stride vtables
  against 128 flat pointer tables.** Nothing here came from a C++ front end.

**New tool: `tools/classtable.py`.** 60 classes and ~1425 method slots means a
`lw $v0,0x0($reg)` / `lw $v0,<off>($v0)` / `jalr` call has its target in the
DATA, invisible in the disassembly. The tool resolves a slot to a name, and
`--vs` diffs a derived table against its base — which is the subclass's
behaviour in one screen. DreamSys inherits 48 slots from `D_800878D4`, adds 90,
overrides 8. Counting slots by hand is how you silently name the wrong
function.

**Carve consequence, and it is significant.** splat derives symbols from
`jal`/`j` references, and a method reached only through a table has none. With
~1425 such entry points, **under-splitting is the expected case in this game**,
not an exotic one. `tools/classtable.py --scan` lists every table, and the
addresses inside them are exactly what splat may have missed — so they double as
carve boundaries. Gate 2 in PARALLEL-RUNS.md now says so.

**Next move.** Unchanged otherwise: carve `code_4cd08` (17) and `code_1677c`
(15) as the first small units, and cross-check any `class_39e08` band against
`classtable.py --scan` before splitting it.

**Still open:** what the class-table header word at `+0x000` means. Not a
pointer, varies per class, some values look like packed fields. See
DECOMPILATION_LEARNINGS.

---

## 2026-08-28 — round 0: repo bootstrap

**State at end: 6 matched / 1356 game functions. Build verifies.**

Set up from an empty directory. Nothing was inherited as a working tree — the
build, toolchain and tooling were assembled and proven here.

**What was established**

- `disk/SLPS_015.56` extracted from a raw 2352-byte-sector `.bin` disc image by
  `tools/extract_exe.py`, sha1 `76322eeade5ebb22dca57fdeac7d68c30f06308d` —
  which matches the target lsddecomp records, so the two projects are aiming at
  the same dump.
- Toolchain: GCC 2.6.3 (Psy-Q) from decompals/old-gcc, `maspsx`, and a
  locally-built `mipsel-linux-gnu` binutils 2.43.1. **The first full build
  reproduced retail byte-for-byte.** A clean `build-and-verify.sh` takes under
  a second, which changes the parallelism economics substantially versus the
  N64 sister project (~30s there).
- splat extraction working: 2080 functions, 1356 game / 724 Psy-Q library.

**Three real bugs found and fixed while bootstrapping**

1. Two symbols inherited from lsddecomp (`SquareRoot0`, `SquareRoot12`) were
   missing their `0x` prefix, which made splat refuse to load the symbol file.
   Four more were an earlier round of guessing at addresses the main file had
   already renamed (`new_GameManager` vs `New_DreamSys`), which splat rejected
   as duplicates. The two files were merged into one so there is a single
   source of truth for a name.
2. Every unit-attached data segment (`.data, DreamSys` etc.) emitted nothing and
   failed to link, because that form means "defined in the C" and the C is
   `INCLUDE_ASM` stubs. Converted to plain data segments — except the `0x1F88`
   rodata, which holds jump tables pointing at labels inside DreamSys functions
   and must stay attached. See DECOMPILATION_LEARNINGS.
3. **A header edit rebuilt nothing.** The Makefile's C rule did not depend on
   headers, so `make` reported success and the next funcdiff scored a struct
   change against an object that never saw it. Now every C object depends on
   every header.

**First matches: 3 in `src/StageGrid.c`** — `func_800494B4`,
`GetStageGridDimensionsTable`, `GetStageGridDimensions`, all byte-exact,
re-derived from the disassembly rather than copied from lsddecomp. (The other 3
counted as matched are `jr $ra; nop` bodies splat generated itself.)

**Next move.** Gate 2 carve — the fresh queue is 120 against 1230 uncarved, and
`code_4cd08` (17 functions) and `code_1677c` (15) are the natural first carves.
Before staffing `class_39e08`, settle the open C++ question in
DECOMPILATION_LEARNINGS: 415 functions written the wrong way is an expensive
mistake to discover late.
