# Progress log

One entry per session or round. The head writes these during consolidation.
Counts come from `python3 tools/progress.py`; dated entries are allowed to go
stale, prose elsewhere is not.

---

## 2026-09-09 — round 26: 8 matches, three carves, and two runner "toolchain leads" that were not

**1036 -> 1044 matched in `main` (76.40% -> 76.99% of game code); 135472 code
bytes matched (58.26% of game bytes). The project crossed 50% of ALL functions
(1044/2080).** Build green in `main` after all **thirteen** merges, **ZERO merge
conflicts**. `headercontention.py` reported NO CONTENTION before provisioning:
all five assigned units include no project header at all.

Five runners on cheap models, re-assigned as their queues emptied — thirteen
merges from five worktrees (alpha 3, bravo 2, charlie 2, delta 3, echo 3). The head worked adjudication and carving rather
than a queue of its own.

### The match count needs its composition, or it looks inflated

`comm -23` over the `INCLUDE_ASM` lists before and after shows only **4**
functions leaving the queue. The count is 8, and both are right:

- **4 from the pre-existing queue** — `func_80031280` (charlie, 135/135),
  `func_80053984` (delta, 82/82), `func_80052F10` (delta, 137/137),
  `func_800522DC` (delta, 69/69).
- **4 from ground carved DURING the round** — echo's clean 4/4 sweep of
  `code_179d8_o` (`new_class_6d4e8` 20/20, `func_80027228` 19/19,
  `func_80027274` 21/21, `func_800272C8` 2/2), which never appeared in the
  round-start queue at all.

Queue arithmetic: 229 start + 8 carved-in - 8 matched = 229.

### Three carves, and the one deliberately NOT taken

| unit | from | content |
| --- | --- | --- |
| `code_179d8_n` | `code_179d8_mid` | 3 clean bodies (161/180/282w), zero stub debt |
| `code_179d8_o` | `code_179d8` front window `[0..3]` | 4 clean (20/19/21/2w) — emptied by echo within the round |
| `code_179d8_p` | `code_179d8_mid_d` | 1 clean 131w body, frameless |

All three were cut to avoid a rodata attach and to carry **zero
blocked-function stub debt**. The fourth — `code_179d8` `[11..32]`, 4 clean
against 18 gp_rel-blocked — was worked out and **left in the yaml, not taken**:
head attention rather than ground was the binding constraint by then, and it
costs 18 mandatory stub reports (without them `progress.py` counts blocked
functions as FRESH and staffs the next runner into a wall).

### Both "toolchain leads" this round were refuted by reproducing them

Neither reached the operator, which is the point of the rule.

- **charlie's guard branch-polarity.** Proposed as a size- or
  context-sensitive REORG heuristic "not reachable from source shape", with a
  corpus census recommended. Size does not move it (1..40 statements, 17..134
  words, zero flips) and it IS source-reachable (a loop or a plain `if`/`else`
  in the fallthrough flips `beq`-FAR to `bne`-NEAR). But the probe rule does
  **not** reproduce charlie's function, whose fallthrough already has control
  flow — so the toolchain is innocent, the trigger is contextual, and no
  census is warranted.
- **alpha's div/mod fusion.** An `__asm__ __volatile__("" ::: "memory")`
  barrier reproduces retail's un-fused shape; alpha correctly flagged it and
  did not adopt it. **Refused** — not the construct HARD RULE 6 authorizes, and
  it fits neither side of that rule's register-identity-versus-order test, so
  authorizing it would be new policy rather than applying existing policy.
  Decisively, the premise was false: seven probes found that narrowing
  `volatile` to the WORD-sized field alone un-fuses the division AND preserves
  retail's `lh`. Alpha applied it and went **193/213 -> 211/213 in one change**.
  Left as an operator-facing policy question with a recommendation to decline;
  it blocks nothing.

### Findings promoted to DECOMPILATION_LEARNINGS

- **The "split scaled index" residue, diagnosed and closed.** Three functions
  across two rounds had it filed as a GCC scheduling mystery. It is the wrong
  source idiom: retail masks the **product** `idx*8`, not the index, which
  means a halfword array indexed by a truncated product, not a struct array
  indexed by a cast index. bravo confirmed it (`func_8002EDD4`: 1 word short ->
  **exact 270-word length**, 267/270) and **refined it** — the spelling depends
  on the call site, eager `u16` in a loop versus `s16` with the mask deferred
  outside one.
- **The block-order rule extends to `switch` CASE order**, in its SOURCE
  DECLARATION form. alpha found it (~43 words across two functions); its own
  summary and report disagreed on which rule it was and its data could not
  discriminate, so it was settled with a reproducer. delta then produced BOTH
  halves back to back — `func_80053984` where the lever was not needed and
  `func_800522DC` where it was essential — which turns it into a measurable
  discriminator rather than a thing to try.
- **NARROW a `volatile` to the exact access that needs it**, confirmed twice
  independently (alpha's div/mod fusion; delta's `*(void * volatile *)` forcing
  a cross-branch reload at 137/137).
- **A two-return guard** wants the success return inside and the failure return
  trailing; the reverse costs two words (echo).
- **The allocno-renumbering property is not size-dependent** — charlie
  confirmed it in a 121-word function.
- Two build-hygiene traps: **`make extract` while a function is live C
  destroys its own `.s` stub**, and **a stale `.s` makes funcdiff report a
  bogus WINDOW while saying "match"**.

### Process, including the head's own errors

- **`PARALLEL-RUNS` §2c gained a real gap.** charlie killed its permuter's root
  PID and reported it "confirmed gone" — true for that PID. `permuter.py -j 6`
  runs a forkserver, and six workers survived **reparented to init**, holding
  six cores, `cwd`-ed into the worktree about to be deleted. The head's
  pre-teardown sweep is load-bearing, not ceremonial.
- **`MATCHING-GUIDE` gained a third stale-report shape: a banked DERIVATION.**
  echo turned round 25's `DERIVATION ONLY` report into C and found two errors
  in it — a field name absent from the struct, and a table advance that is a
  fixed `+0x200` rather than `+ numVags*2`, provable from delay-slot semantics.
  A derivation read in source order gets delay-slot placement wrong, and a
  delay slot is where an increment hides.
- **Head error 1:** an ad-hoc word census counted attached jump-table `.word`
  lines, over-reporting two functions by 40%. `nearmiss.py` reads the size
  header and was right throughout.
- **Head error 2:** the `timeout` bound was in the spawn prompt and dropped
  from re-assignments; charlie then parked a turn on an unbounded
  `--stop-on-zero` search. §2c says put the bound in the ASSIGNMENT. Put it in
  re-assignments too.
- **Head error 3:** trusted a funcdiff "match" whose window was visibly wrong
  (67808 words). Verified the bytes directly, then found the stale-`.s` cause.

### Where it leaves the queue

**fresh 7, and thin** — 81w/147w/175w/376w/387w plus a reopened. The cheap seam
is gone again: `code_179d8_o` was the last of it and it is closed. Next round
should CARVE before staffing more than two runners; `code_179d8` `[11..32]` is
worked out in the yaml with its 18-stub cost stated. `class_3bb8c_h` remains
the trap — 13 of 17 are BIOS trampolines and it screens cleanest while being
least matchable.

---

## 2026-09-08 — round 25: 7 matches, and a new source-shape lever that closed four of them

**1029 -> 1036 matched in `main` (75.88% -> 76.40% of game code); 133532 code
bytes matched (57.43% of game bytes). Build green in `main` after all seven
merges, ZERO merge conflicts.** `headercontention.py` reported **NO
CONTENTION** before provisioning: the five assigned units shared no project
header at all (four include only `common.h`; `code_55dd4.h` and `DreamSys.h`
have one other reader each, neither staffed).

Gates 0-2 passed clean. Gate 0's re-extract cleared 17 stale
`asm/nonmatchings` files. Gate 1 measured **39 blocker-clean fresh/reopened**
functions, so no carve and no whole-round permuter. Five runners on cheap
models; the head worked its own queue in parallel.

### The round's result is a LEVER, not a count

**`func_8005CBC8` (`code_4cd08`) closed at 100/100** after three rounds pinned
at 99/100, thirteen hand attempts and ~77k permuter iterations. The residue
was **basic-block ORDER**, and every one of those attempts had been searching
the expression axis.

**GCC 2.6.3 gives the if/else fallthrough to whichever candidate is LAST in
source order.** So if retail's *jumping* block is the one your source puts
last, no expression reshape and no `__asm__("")` will ever reach it — the fix
is textual placement, an explicit `goto` over the block you want to fall
through. Full entry in `DECOMPILATION_LEARNINGS.md` ("An arm that must JUMP
has to be written NOT-LAST").

It was broadcast mid-round to all five runners, twice, and that is where the
round's yield came from — **four of the seven matches are attributable to
it**, across three different units:

| function | unit | who | note |
| --- | --- | --- | --- |
| `func_8005CBC8` 100/100 | `code_4cd08` | head | the lever's discovery |
| `func_8005DBF0` 74/74 | `Entity` | head | 2nd shape: duplicated assignment |
| `func_80036230` 115/115 | `code_179d8_f` | delta | 3rd variant: which side owns the `if` body |
| `func_80035E80` 47/47 | `code_179d8_k` | alpha | duplicated-write shape, found independently |

The other three matches are `func_80033260` (144/144, bravo),
`func_80058C58` (79/79, echo) and `GenerateInitialSpawn` (107/107, echo).

**The lever also produced four large NON-closing gains, which are not matches
and are counted as stalls:** `func_8002AEE0` **61 -> 153/174** (charlie, found
INDEPENDENTLY before the broadcast arrived — the strongest single piece of
evidence for the mechanism), `func_80059814` **14/53 -> 49/53 and exact
length**, `func_800598E8`'s **whole CFG now matching retail**, and
`func_80034F90` down to a single delay-slot residue.

*(This entry's first draft said "11 matches ... five of the eleven". Both
figures were wrong: `progress.py` moved 1029 -> 1036, and the fifth
"lever match" was `func_8002AEE0`, a 92-word GAIN on a function that is still
a stall. Corrected before commit by counting the merge commits, which is what
this document's own protocol says to do.)*

### THE LEVER IS NOT STRICTLY DOMINANT — three runners corrected the broadcast

This is the more valuable half, because the head's broadcast described one
shape and one fix, and **runners who tested it rather than adopting it found
its edges**:

- **echo made `func_80059BE0` WORSE** by applying it to an entry guard that
  already had its default living in the guard's own delay slot — a different,
  already-correct idiom. Check which shape the disassembly ALREADY shows.
- **charlie produced a twice-reproduced counter-example to the naive fix.**
  "Retail keeps two materializations, so give the duplicate its own C
  variable" regressed on `func_8002B4D4` in TWO separate rounds (to ~17/91,
  and 60 -> 50/91), because that function's block order was already correct.
  A second named variable changes allocation for the WHOLE function. **That
  lever helps only while block order is wrong; after that it is harmful.**
- **Four different placement fixes closed five functions**, so the
  transferable content is the QUESTION (which block does retail place where,
  and which candidate did my source make last), not any one fix. Delta said
  it directly: per-instance discovery, not pattern-matching from a prior fix.

### charlie decomposed the round's assigned investigation

"A bare `__asm__("")` fixes the redundant-raw-copy elision in an isolated
reduction and transfers to none of the real functions" is **not one
mechanism**. In `code_179d8_g` it is three, and the block-order hypothesis
explains exactly one: (1) block-order/shared-join fallthrough (confirmed,
`func_8002AEE0`); (2) pure intra-block scheduling with **no second block at
all** — confirmed via the permuter's `--debug` breakdown showing zero
insertions/deletions/register-differences — where a barrier is the right
instrument and no working position exists; (3) dead-code elimination removing
the redundant check before scheduling ever runs. Diagnose from the `.s`'s
block/label structure before reaching for a barrier.

### Other findings worth carrying

- **A dense `switch` must reproduce retail's jump-table WIDTH, not just its
  arms** (echo, `func_80058C58` 79/79). Thirteen real arms give a 33-entry
  table against retail's 48; a no-op high `case 47:` widened it and closed the
  function. Diagnostic: check `funcdiff`'s first-diff offset against the
  rodata table base before suspecting the body.
- **An `mflo`/`mfhi` residue is a COUNTER-indication for the block-order
  lever** (head, `func_8004CFB8`, variant 7 came out 5 words too long). cc1
  expands a `mult`/`mflo` pair together during RTL expansion, before any
  layout decision, so placement cannot move them apart.
- **`progress.py` gained `DERIVATION ONLY -- ASSIGNABLE`.** Bravo did the
  right thing on the 274-word `func_80032D34` — derived the algorithm,
  declined to burn attempts, wrote it up, and stated in its own prose
  "deliberately NOT filed as a STALL" — and the tool counted it as a stall
  anyway, deleting blocker-clean ground from every future round. Same failure
  `REOPENED -- ASSIGNABLE` was added for in round 22, from a direction that
  marker cannot express: there is no stall verdict to invalidate because there
  was never an attempt. Scoped explicitly to NEVER cover a body that was built
  and scored.
- **alpha found a 3-instance shared residue class** in `code_179d8_k`:
  `func_800349B0`, `func_80034AEC` and `func_80034C28` are each ONE WORD SHORT
  because retail computes a state-block pointer into one register and
  explicitly rescues it into another before repurposing the first as a loop
  counter, where this build's allocator picks the final register directly. 3
  of 6, not all 6 — the other two residues are unrelated.
- **Four more Sony SDK functions identified inside game-code segments**
  (`func_80033260` = `SsUtGetVagAtr`, `func_80032D34` = `SsVabOpen`-family,
  `func_8002B640` = `CdSearchFile`, `func_8002B94C` = `CD_newmedia`). This is
  NOT new — `SsUtGetVabHdr` in `code_179d8_i` set the precedent in an earlier
  round — but the count is now 5+, which bears on the game-code denominator.
  Bravo correctly did NOT rename `func_80033260`, because five other units
  carry a literal `jal func_80033260`; that constraint did not apply to
  `SsUtGetVabHdr`.

### Process notes

- **Delta stopped to wait on a bounded search three times**, the third after a
  numbered work order. The previous head warned "the work order is a repair,
  not an inoculation" and that is confirmed. What ended it was reassigning
  delta to cold ground in an unowned unit, where it immediately matched
  `func_80036230`. **Consider re-assignment, not another correction, as the
  second response to a waiting runner.**
- **The head asserted a process state it had not measured.** I told delta its
  `timeout 900` search "has reached its bound"; delta checked `ps`, found ~11
  minutes elapsed, blocked on the process directly rather than trusting the
  premise, and was right. Runners verifying the head is working as intended.
- **Alpha reached ~50 minutes with three reports untracked and zero commits** —
  round 18's failure mode. A direct commit-now order fixed it immediately and
  alpha then committed steadily. Poll `git status --porcelain` per worktree
  early, not at teardown.
- **The head declined to ship a mechanical screen for the new lever.** A grep
  for the tell (bare `j` to a nearby join, non-`nop` delay slot) flags **51**
  of the queue's functions; one was tested and the hit sat in that function's
  already-byte-exact half. Precision 1-tried/0-genuine is not enough to rank a
  queue with, and this project has paid four times for screens whose scope was
  reasoned instead of measured. Not in `tools/`, deliberately.

### Next round

**Carve first — `fresh` closed at 23 and, more importantly, the CHEAP SEAM IS
GONE.** The round started with five blocker-clean functions under 90 words;
the smallest now is 81w and most of the queue is 130-390w. A count of 23 reads
healthier than the ground actually is.

**And the uncarved reserve is nearly exhausted of clean ground. Measured this
round, per-function, over every uncarved game segment:**

```
workable clean (>4w):   19
trivial clean (<=4w):    3   (splat stubs)
gp_rel blocked:         55
nop_mflo_mfhi:           1
BIOS trampolines:       13   (all in class_3bb8c_h -- the round-17 trap)
```

So **`gp_rel` now blocks 137 functions: 82 in the live queue plus 55
uncarved.** That is the dominant obstruction in the project and it is the open
operator escalation (`docs/research/gp-relative-blocker.md`; the `-G`
experiment was authorised and REJECTED). `nop_mflo_mfhi` is 13 total (12
queued + 1 uncarved), and its remedy — a maspsx flag decoupling it from the
aspsx version, exactly what round 21 did for `addiu_at` — **remains untested
and is not a head decision.**

Carve candidates, derived this round rather than transcribed:

1. **`code_179d8` front window `[0..19]`: 7 of 20 clean** (of 43 functions and
   9 clean total). The cheap ones are `func_80027228` (19w),
   `new_class_6d4e8` (20w), `func_80027274` (21w), `func_800282AC` (32w),
   `func_80027FFC` (53w) — plus three trivial stubs. This is the only
   remaining cheap seam in the executable.
2. **`code_179d8_mid`: 3 of 3 clean** but big (161/180/282w). One unit.
3. **`class_3bb8c_n`: 3 of 23 clean** (`func_80054B1C` 13w, `func_80055874`
   31w, `func_80055258` 110w) — a poor carve, listed so nobody re-measures it.
4. **Do NOT carve `class_3bb8c_h`** — 13 of 17 are BIOS trampolines, the
   round-17 trap, and it screens as the cleanest ground left while being the
   least matchable.

Then runners. Best-posed open targets, all with fresh evidence:
`func_80059814` (49/53, one register-identity residue), `func_800598E8` (CFG
exact, 2 words), alpha's 3-instance register-rescue cluster in
`code_179d8_k`, and `func_80036528` (227/240, frame-exact, delta's two new
codegen levers under it).

---

## 2026-09-08 — round 24: a carve off two dead blocker verdicts, and a resolved blocker's third harvest

**1010 -> 1029 matched (74.48% -> 75.88% of game code). Build green in main
after every merge, ZERO merge conflicts** — `headercontention.py` reported NO
CONTENTION before provisioning and again before both re-sends.

Gate 0 green. §4a confirmed a single head. Gate 1's true fresh queue stood at
**24**, every function 43-274 words with the cheap seam of every staffed unit
exhausted, so Gate 2 fired.

### The carve: two segments freed by a blocker that died three rounds ago

Both had been left uncarved on censuses that measured `addiu_at`, RESOLVED in
round 21. Re-censused per function with the four canonical screens, and the
census reproduced round 23's table exactly:

| new unit | funcs | CLEAN | note |
| --- | --- | --- | --- |
| `code_179d8_k` | 18 | **18** | was "17 of its 18 addiu-$at blocked" |
| `code_179d8_l` | 12 | 9 | all 3 surviving nop_mflo_mfhi landed here |
| `code_179d8_m` | 12 | **12** | no blocked function at all |

`fresh` 24 -> 61; uncarved game code 133 -> 91. Two free matches (splat emitted
the empty C bodies for `func_8002E2F8`/`func_8002E300` itself).

`code_179d8_k` owns **THREE** jump tables, not the two its old comment claimed
(`func_80034690` -> `jtbl_80010CF0`; `func_800357B0` -> `jtbl_80010ED8` AND
`jtbl_80010F38`). The 0x14F0 rodata slot holds exactly those three and is
referenced from nowhere else, checked per symbol, so it attached WHOLE — no
split dance. The `mid_c` halves own no table and no rodata word in the image
points into their range.

**The carve hit both documented routine failures in one step**, worth recording
because the fix order matters: splitting `mid_c` in two left
`src/code_179d8_l.c` holding the 12 functions now owned by `_m`, whose `.s`
files no longer existed under `code_179d8_l/` — splat does not rewrite an
existing `.c`. Deleting the generated `.c` and re-extracting fixed it. The same
step left the two stale monolithic `asm/*.s` behind (Gate 0's own scenario).

### Stale blocker DIRECTIVES are systemic, and they are now swept mechanically

Round 23 found four unit headers listing functions as `addiu_at`-blocked after
round 21 resolved it. Round 24 found **six more**, which makes it a standing
class rather than an anecdote: `code_179d8_f` (2 free functions),
`code_179d8_g` (3), `class_3bb8c_l` (2), `code_179d8_d` (2, one of which the
head then matched), `class_3bb8c_t` (1, the function the head then attempted),
`code_179d8_b` (3, two already matched). Every one named functions the
blocker's death had already freed, and every one was phrased as a DIRECTIVE
("do NOT spend attempts on these").

The sweep: cross-check every `func_XXXXXXXX` named on a `src/*.c` comment line
framed as blocked against `nearmiss.py`'s ASM-derived verdict and against the
set of functions already defined as C.

**It must be read, not acted on — and it under-reports in one direction while
over-reporting in the other.** Line-scoped, it returned 9 hits of which 8 were
the corrected comments themselves (they contain "BLOCKED" while saying "NOT
BLOCKED"). Widened to a 3-line window it returned 16, of which 14 were. And it
MISSED two genuine ones whose blocker framing sat on a different line from the
function name — including `class_3bb8c_t`'s, which named the very function the
head went on to attempt.

The same disease was in the splat yaml, where it hid four more clean functions:
`code_179d8_mid` is **3 of 3 CLEAN** against "all three are addiu-$at blocked,
so there is nothing to staff here", and `code_179d8_mid_d` is clean too. Both
are carve candidates now.

### The resolved blocker's third harvest, and why it kept hiding

Round 22 built the `REOPENED -- ASSIGNABLE` marker and swept for reports citing
`addiu_at`. Round 23 found that sweep had missed `func_8005CBC8` because its
report "does not look like a stub". **Round 24 found four more, one step
further along the same axis**, via Gate 1b's contradiction sweep:

| function | unit | result |
| --- | --- | --- |
| `Entity__GetEventVideo` | Entity | **MATCHED 9/9**, first attempt |
| `Entity__GetUnlockEffect` | Entity | **MATCHED 14/14**, first attempt |
| `Entity__GetLinkStage` | Entity | **MATCHED 15/15**, first attempt |
| `func_80059814` | DreamSys | 8/53 -> **14/53**, residue 1 retired |
| `func_8005C508` | code_4cd08 | **MATCHED 56/56** (delta) — no source change at all |

All five matched or improved with C their own reports had already derived.
`Entity__GetEventVideo` is **9 words** — the smallest function in the queue.

**They hid because their reports were TOO well argued.** All four Entity/
DreamSys ones open with a **HEAD ADJUDICATION** box certifying that the head
reproduced the diagnosis independently from scratch and wrote it up
project-wide with a 502-of-502 corpus census. Nothing reads less like stale
ground, and every word of it was true. The adjudication verified the
MECHANISM, which never changed; what expired was the premise that the
mechanism was unfixable — a premise the reports state explicitly ("maspsx
exposes no `--addiu-at` flag"), and which round 21 falsified by adding exactly
that flag.

**So: a blocker's death invalidates the strongest reports as thoroughly as the
weakest, and without touching a word of what they say.** And the detector
under-reports: the sweep found `Entity__GetEventVideo` but MISSED both
siblings, whose titles do not match its grep. What found them was noticing the
shared preamble. **The FAMILY is the unit of staleness, not the report** —
blocked reports are written in families, one runner, one sitting, one cause.

The other direction was handled too: `func_800598E8` defers its whole analysis
to `func_80059814`'s, so its cause is equally dead — but it was deliberately
NOT marked `REOPENED`, because the sibling had just shown the blocker was
worth 6 words of ~45. A report can have a dead CAUSE and a live CONCLUSION,
and an honest annotation says which half it corrects.

### The head was wrong about `nop_mflo_mfhi`, and the pipeline caught it

CLAUDE.md defined the blocker as an `mflo`/`mfhi` followed within two
instructions by a `mult`/`div`, *"with no `nop` between them in retail's own
bytes"*. Reading that literally, the head "refined" the canonical grep with a
nop test and concluded two flagged functions were assignable — one of them
`func_8005D864`, which round 23 had just carefully re-adjudicated ONTO this
blocker.

One reproducer settled it: `int q = a / b; return q * c;` compiles to
`mflo` / `nop` / `nop` / `mult`. **The pipeline inserts TWO nops**, so retail's
single nop still leaves the sequence one word short. The refinement was
discarded, all 12 flagged functions are genuinely blocked, and round 23's
adjudication stands. Gate 1's wording is corrected; `DECOMPILATION_LEARNINGS`
had it right all along and the two documents had silently disagreed.

**This screen has now been broken in both directions available to it** —
inverted window (rounds 15, 16) and false qualifier (round 24) — by four
different heads, every one of whom had read the warning not to re-implement
it. Treat "improve the mflo screen" as a smell, not a task. And prose in a doc
is not a specification of a toolchain behaviour; the reproducer is.

### Toolchain: the permuter had DIVERGED from the pinned build

Runner delta found two bugs in `tools/setup-permuter.sh` and correctly fixed
them only in its own worktree, since `tools/` is shared. Both are fidelity
bugs — the script's job is to replicate the pinned pipeline:

1. The generated `compile.sh` omitted **`--addiu-at`**, which the Makefile has
   passed since round 21. The permuter was scoring candidates against a
   pipeline that FOLDS where retail unfolds.
2. `sed -e '1,4d'` deleted the `.s` prelude by POSITION. For an ordinary
   function lines 1-4 are exactly the two `.set`s, a blank, and `nonmatching`.
   But a function owning embedded rodata has `.section .rodata` on line 4, so
   the delete silently assembled its jump table into `.text`.

Verified rather than reasoned, on `func_8005CBC8`: old `.text` = 480 bytes with
no `.rodata` section; fixed, `.text` = 400 bytes = exactly 100 words = retail's
own length.

**Blast radius on existing verdicts is effectively ZERO, and that is measured.**
22 live functions carry a *permuter-exhausted* verdict — the verdict that
removes a function permanently. Of those, exactly one (`func_8005CBC8`)
contains the `addiu_at` construct, and exactly one owns embedded rodata: the
same one, whose verdict is from this round with delta's fixed copy. So no
existing verdict needs re-running. The Makefile was not touched.

### A figure carried forward without re-derivation, for the third round running

Delta rebuilt round 23's inherited "99/100" for `func_8005CBC8`, as its
assignment required, and found the preserved body measured **108/100 — eight
words LONG**. Round 23 asserted the number without deriving it from a build.
The conclusion survived: retail's jump table points indices 6 AND 7 at the
same handler, so the source needs one merged `case 6: case 7:` arm with the
index arithmetic derived rather than two arms with hardcoded constants. With
that fixed it is a genuine, rebuilt 99/100.

**This is the first time the unverified figure was the HEAD's own**, propagated
through a PROGRESS entry into the next round's assignment as fact. The standing
rule — rebuild any figure before putting it in an assignment — had been read as
a rule about reading reports. It is also a rule about writing summaries.

### Results

| runner | unit | matched | words | stalls | note |
| --- | --- | --- | --- | --- | --- |
| alpha | `code_179d8_k` | 3 | 93 | 5 | all five at EXACT compiled length |
| bravo | `code_179d8_m` | 6 | 426 | 0 | 6 for 6, first pass; re-sent |
| charlie | `code_179d8_l` | 2 | 111 | 3 | + a 3-for-3 unsolved residue class |
| delta | `code_4cd08` | 1 | 56 | 1 | + the two permuter-script bugs |
| echo | `code_55dd4` | 0 | 0 | 5 | all five re-verified, figures rebuilt |
| **head** | 6 units | **5** | **132** | 2 | incl. one overturned runner stall |

### The head's highest-yield intervention was overturning a SETTLED-class stall

Alpha filed `func_80034D90` (49/51, exact length) under the project's
**SETTLED** commutative-operand-order canonicalization class. The screen
matched exactly, the citation was accurate, and the learnings doc does say not
to spend attempts reordering commutative operands. Alpha even tested a reshape
(`u8 *ptr = rec->unk12 + (u8*)rec;`), watched it regress to 34/51, and stopped
— correct by the guidance as written.

**It closed at 51/51 by REGROUPING, not reordering:**

```c
((u8 *)rec)[rec->unk12 + 0x2C] = ...    /* rec + (off + 0x2C)  -> 49/51 */
*((u8 *)rec + rec->unk12 + 0x2C) = ...  /* (rec + off) + 0x2C  -> 51/51 */
```

That is round 23's own `&arr[i + j]` versus `arr + i + j` lever reaching the
very `addu` the class calls unreachable. The class needed a SCOPE BOUNDARY,
not another attempt:

- **Symmetric addends: the class holds.** Round 20's decisive datum stands —
  both C operand ORDERS gave the same wrong output.
- **One addend is a base pointer and the expression can be re-associated: it
  does not hold.** Grouping decides which value becomes `rs`, and grouping is
  not order — which is exactly why alpha's reversed-order reshape regressed.

**The boundary then predicted the sibling, which is what makes it a boundary
and not an anecdote.** `func_80035E80` — same unit, same round, same runner,
same class citation — has `addu a1,v0,v1` over two COMPUTED values with
nothing to re-associate, and stays a stall. Charlie independently found a
SECOND source-reachable axis (narrowing an addend to `u8` flips which operand
becomes `rs`, closing `func_8002DF7C` at 47/47); it is genuinely distinct,
because alpha's addend was already `u8`.

Cheap tell now written down: **when a function disagrees with its own
already-matched siblings' idiom, try the siblings' idiom before accepting a
class verdict.** Three of `code_179d8_k`'s matches use `(u8 *)rec +
rec->unk12` base-first; the stall used the subscript form.

### Two levers BOUNDED by measured negatives

- **The bare `__asm__("")` barrier.** Alpha's four scheduling-class reports
  mention it zero times, and one of them explicitly says "no source-level
  construct is known... not attempted". There is one, and the project permits
  it for order-only residues; echo closed a two-word deferral with it this
  same round. Tried by the head on `func_80034138`: **byte-identical output,
  still 66/69**, load pair still swapped, delay slot still a `nop`. So the
  lever closes a case where a value's computation is DEFERRED past where
  retail computes it; it does NOT reorder an adjacent load pair and does NOT
  install a dead computation into a delay slot. Worth bounding precisely,
  because echo's success could otherwise read as general.
- **Round 21's pointer-elimination lever** (echo): regressed
  `func_80065E1C` catastrophically to 2/68 with 60KB of drift. It requires the
  loop-carried pointer have no role beyond pairing with its scalar
  counterpart, and fails when the same buffer is re-read later through a
  different index expression.

### Runner-process findings

**`echo` returned zero matches and it was not a wasted pass.** Five reports
re-verified from scratch, every figure rebuilt and every one agreeing with the
prior number, all five titles converted to the three-figure format, two new
axes tried, and the first permuter run ever on `func_800662BC`. Its
drift-rate negative is worth keeping: round 19 measured roughly one inherited
body in six carrying a false "clean/drift-free" claim, and echo found **zero
drift in five**. One head adjudication on top: `func_80065A5C` (30/33, pure
3-word prologue store permutation, 13 exhausted hand attempts, loop body
byte-identical throughout) is a textbook permuter target that has never had a
permuter run — next round runs it there first.

**`charlie` flagged a standing unsolved class, 3-for-3 in one unit:**
"redundant-raw-copy elision" — cc1 proves a preserve-the-raw-parameter
register copy is bit-identical to a value computed elsewhere and elides it,
where retail keeps a genuinely separate register. A scheduling barrier fixes
it in small isolated reductions and did NOT transfer to any of the three real
functions. That is a dedicated-investigation candidate, not more per-function
attempts.

**2c recurred, and a work order fixed it only once.** Round 18 concluded that
an explicit numbered work order fixes a runner stopping to wait on a bounded
search. Round 24: two runners ended turns that way with zero commits, an order
fixed both, and delta then did it a SECOND time on its re-send despite having
received one. So the work order is a repair, not an inoculation — budget for
re-sending it, and check `git status --porcelain` per runner rather than
trusting that a runner which committed once keeps committing.

### Second passes: 2 of 5 runners re-sent, and they returned corrections rather than matches

Round 19 measured re-sends as the head's highest-yield structural move (9 of
17 matches). Round 24 re-sent the two runners whose first pass finished, into
NO-CONTENTION ground, and got **zero further matches** — but three durable
corrections and one salvage. That is worth recording as the other face of the
same lever: a re-send buys whatever the ground has left, and bravo's and
delta's remaining ground was 138-387 words apiece, not a cheap seam.

- **bravo** (`code_179d8_m`, own unit): two stalls at 138w and 228w, plus
  **a correction to one of its own first-pass reports** — `func_8002F3E8`'s
  second parameter is `s16`, not `s32`, because the MIPS ABI widening
  instructions that appeared to prove it wider do not distinguish the two.
  It re-verified `func_8002F610` still matches (60/60) afterwards. Also
  identified both of `func_8002EA44`'s magic divisors as **16129 = 127²** by
  batching thousands of candidate divisors through the pinned toolchain in
  ONE pass — a technique worth reusing whenever a magic constant needs
  naming.
- **delta** (`class_3bb8c`, unowned): one stall at correct length 63/63,
  58/63, with real header work behind it — a `void` -> `s32` return-type
  correction, an additive `Obj866E8` pad split, and two proven-size tables.
  Its permuter run captured the exit code this time (39,940 iterations,
  **124**, self-fired), and its one useful lead was a `do {} while (0);`
  scheduling barrier — ordinary C, not asm — which closed one register
  residue outright.

**A shared-header retype needed the round-7 check, and passed it.** Delta
retyped `func_8004BA40` from `void` to `s32` in a header ELEVEN units include.
The head checked rather than merged: the only caller `func_8004B700` is still
`INCLUDE_ASM` so it contributes retail's own bytes, no other unit declares or
defines the symbol, and the two new data symbols exist nowhere else. Safe. But
it leaves a **forward hazard now written into the report**: `func_8004B700` is
a live 125/140 stall in the same unit and it DISCARDS the return value, which
is exactly round 7's condition — a discarded call's declared return type
controls GCC's tail-merge grouping. Whoever attempts it needs that fact, or it
surfaces as an unexplained few-word residue.

### A struct-layout correction WITHDRAWN, on a discriminator this round had just retired

Bravo's `func_8002EA44` report filed an "important correction" that
`D_8008D7F0`/`D_8008D7F2` are two independent 0x10-stride arrays rather than
two fields of one record — contradicting `code_179d8_j.c`'s `Rec16D7F0` model
and round 23's adjudication of the same two symbols.

Its observation was accurate and its inference was not. The stated
discriminator — "a separate `%hi`/`%lo` pair per symbol rather than a shared
base with a `+2` fold" — is precisely the one established as NON-CONCLUSIVE
earlier the same round by `func_8002BC40`. Re-measured by the head: both
symbols get a full `lui`/`addiu` pair, each indexed by the same `a0 * 16`
offset. That is the non-conclusive case, so it cannot overturn anything, and
round 23's evidence (an `addiu $t2, $a3, 0x2` folding the second base off the
first's materialised address, which cc1 can only emit for one object) is
conclusive. **Conclusive evidence in one function beats non-conclusive
evidence in another about the same symbols.** The one-record model stands.

Two things this says about process, not about the symbols:

- Bravo merged main before this pass and therefore HAD the non-conclusiveness
  rule. So this is a runner applying a superseded discriminator, not one
  lacking a rule — which means promoting a rule does not retire its
  predecessor in anyone's head. Say in a re-send which rule a finding
  SUPERSEDES, not only what the new rule is.
- **Struct-layout claims are corpus-wide, so this is the class that most needs
  the head to check rather than merge.** It cost one grep and one look at the
  `.s`.

### Next round

**Runners, then a carve — not a carve first.** `fresh` closed at **37**, above
the ~20 a five-runner round needs, and it is much better ground than round
24 inherited: `code_179d8_k` alone has 15 fresh blocker-clean functions
including several under 90 words, `code_179d8_m` has 4 (138-387w) and
`code_179d8_l` has 7.

Priorities, cheapest and best-posed first:

1. **`func_80065A5C` (`code_55dd4`) on the permuter, before any hand
   reshape.** 30/33, exact length, loop body byte-identical, thirteen hand
   attempts all pinned at exactly 30/33, and it has NEVER had a permuter run.
   Do not add a second `__asm__("")` — echo measured that at 7/33.
2. **`func_8005CBC8` (`code_4cd08`)** stays the closest open function in the
   corpus at a now-REBUILT 99/100, with `setup-permuter.sh` fixed under it.
3. **`func_80058C58` (`DreamSys`, 79w) is pre-analysed and nearly free.** A
   flat 13-arm jump-table switch of mostly single stores; its rodata slot is
   already attached (`- [0x1F88, .rodata, DreamSys]`); the arm-to-case mapping
   is decoded and the case order is confirmed by round 23's fall-through
   lever. Its one cost is an ADDITIVE extension of `DreamSysBaseMethods`
   (which stops at slot 0xE0) to slot0x184/slot0x188 — a shared-header struct
   edit, so run the struct-edit check. Deliberately left with NO report file
   so it stays in `fresh`; the analysis is in this entry.
4. **`code_179d8_mid` (3 of 3 clean, 161/180/282w) and `code_179d8_mid_d`
   (1 clean, 131w)** are the carve candidates when one is next needed — four
   clean functions that were invisible behind stale yaml comments until this
   round.
5. **`charlie`'s "redundant-raw-copy elision" is the best-posed open
   investigation**: 3-for-3 in one unit, a barrier fixes it in isolated
   reductions and transfers to none of the three real functions. A dedicated
   look beats more per-function attempts.

Do NOT carve `class_3bb8c_h` (13 of 17 BIOS trampolines, the round-17 trap)
and leave `code_179d8` / `class_3bb8c_n` (gp_rel-dense) alone.

**Operator escalations, unchanged and both still open:** `gp_rel` (the larger)
and `nop_mflo_mfhi`. The latter's remedy — a maspsx flag decoupling it from
the aspsx version, exactly what round 21 did for `addiu_at` — remains
untested and is not a head decision.

---

## 2026-09-07 — round 23: 20 matches, three REOPENED functions closed, and a 99/100 near-miss recovered from a dead blocker

**990 -> 1010 matched (73.01% -> 74.48% of game code). Build green in main
after all six merges, ZERO merge conflicts.** `headercontention.py` reported
NO CONTENTION before provisioning: the five assigned units shared no project
header (three `code_179d8_*` slices include only `common.h`, plus one
`Entity_*` and one `class_3bb8c_*`).

Gates 0 and 1 passed clean — `fresh` stood at 50 with 47 blocker-clean
report-less functions, so no carve and no permuter round. Five runners on
cheap models; the head worked its own queue in parallel.

### Results

| runner | unit | matched | words | stalled |
| --- | --- | --- | --- | --- |
| alpha | `code_179d8_j` | 0 | 0 | 4 |
| bravo | `code_179d8_c` | 2 | 28 | 2 |
| charlie | `Entity` | 5 | 260 | 0 |
| delta | `code_179d8_b` | 2 | 28 | 1 |
| echo | `class_3bb8c_g` | 3 | 326 | 0 |
| **head** | 6 units | **8** | **417** | 1 |

Charlie and echo went 5-for-5 and 3-for-3. Alpha returned a zero-match pass
with four characterised residues on the hardest remaining ground in the
corpus — see below; that is a result, not a miss. (Its fifth assigned
function, `func_80031280` 135w, was explicitly held back by the head so alpha
could finish its bookkeeping; it is untouched and has no report.)

### The head's most valuable single intervention was a MEASUREMENT correction

Alpha filed `func_80031A44` as *"the function is 51/88, matches retail's exact
instruction count is off by exactly one word, and the residue is precisely one
missing `nop`"* — internally contradictory, since 51/88 means 37 words differ.
Flagged to alpha (which held the unit and the build) rather than fixed by the
head. Alpha re-measured, and the corrected title reads:

> length 1 word SHORT at 87/88; 51/88 raw word-match, but that 37-word gap is
> almost entirely the SHIFT from the one missing word, not independent residue

**That is a one-word near-miss and a prime permuter target, and "51/88" hid it
completely.** Alpha corrected all four of its titles the same way, and the
range they now distinguish makes the case for the format: `func_80031890`
(length EXACT 73/73, 51/73, first diff at word 37 — genuine scattered
residue), `func_80031A44` (1 short, gap is ripple), `func_80030404` (5 short,
21/96, first diff at word 0 — residue independent of the length gap). Three
very different states that all render as "≈50%" under one figure.

**A stall title must now carry three figures — length, raw word-match, and
where the first real diff is** (from `asm-differ`, never inferred). Added to
the runner prompt and to DECOMPILATION_LEARNINGS' round-20 entry, which had
the principle but not the format requirement.

### A cross-report contradiction, adjudicated

Alpha flagged that `func_80031CF0`'s report models `D_8008D7F0`/`D_8008D7F2`
as two independent 16-byte-stride arrays, contradicting what
`func_80030404`'s disassembly shows. Verified against the `.s` and **alpha is
right**: `addiu $t2, $a3, 0x2` folds the second base off the first symbol's
materialised address, which cc1 can only do if the two are ONE object. The
report's discriminator ("each field gets its own `lui`/`addiu`") measures
whether GCC CHOSE to share a base register in that one function — a register-
pressure question — not whether the symbols ARE one object, and in
`func_80031CF0` both models emit identical bytes. Model corrected; the stall
verdict stands. Left standing it would have made `func_80030404` unmatchable
by construction.

### The REOPENED mechanism paid, and round 22's sweep was incomplete

Round 22 added the `REOPENED -- ASSIGNABLE` marker so a resolved blocker
returns its ground. **Measured this round: 3 of the 5 marked functions
matched** — `func_8003C48C` (36w), `func_8003C63C` (100w) and `func_80049EB4`
(107w), all previously filed `addiu_at`-blocked and never attempted. A fourth,
`func_8004109C`, went from "NOT ATTEMPTED, predicted register saturation" to a
fully-derived 42/56. The fifth (954w) was left as too large.

**But the sweep missed the most valuable function in the queue, and the reason
generalises.** `func_8005CBC8` (`code_4cd08`) is a long, twice-corrected report
whose verdict read *"cannot be matched as C under the pinned toolchain
regardless of source shape"* — a considered plateau, not a stub. Its blocker
was `addiu_at`. Re-measured with the fix live: **the switch dispatch now
reproduces exactly and the residue is ONE word — 99/100.** It is now the
closest open near-miss in the corpus and a prime permuter target.

> **CORRECTED, round 24: the "99/100" in the paragraph above was never
> rebuilt from a build, and the body preserved alongside it measured
> 108/100 — EIGHT WORDS LONG.** Round 24's runner delta rebuilt it, as its
> assignment required, and found the discrepancy. Root cause: retail's jump
> table points indices 6 AND 7 at the *same* handler label, so the source
> needs `case 6: case 7:` as one merged arm with the index arithmetic
> derived, not two separate arms with hardcoded constants. With that fixed
> it IS a genuine, rebuilt 99/100 — the conclusion survived, the number was
> unverified, and the preserved body would have cost the next reader eight
> words of confusion.
>
> This is the THIRD consecutive round in which a figure was carried forward
> without being re-derived, and the first in which the unverified figure was
> the HEAD's own, propagated through this PROGRESS entry into round 24's
> assignment as fact. The standing rule — rank from title lines, but rebuild
> any figure before putting it in an assignment — is now also a rule about
> what a head writes down.

A blocker's death invalidates every report that RELIED on it, not just the
stubs that look like citations. There is a mechanical detector, now in Gate 1:
**`nearmiss.py` screens from the ASM, so a function it lists as blocker-CLEAN
whose report calls it blocked is a contradiction — and the report is the wrong
half.**

### Stale blocker claims in `src/*.c` COMMENTS — a class no tool can see

The same sweep found false blockers in unit header comments, which are
invisible to `progress.py` and read by every runner assigned to the unit.
Worse than a stale count, they are DIRECTIVES:

- `class_3bb8c_m` listed `func_800545FC` as `addiu-$at` blocked and said *"All
  four have stub reports; do not attempt them."* **Matched this round, 25/25,
  first attempt.** Three of that comment's four lines were still correct, which
  is exactly why nobody re-read it.
- `code_179d8_i` claimed 9 of 18 blocked with *"do not spend attempts on
  them"*. All nine had in fact been matched in rounds 21-22; the comment was
  stale but its ground already recovered.
- `code_179d8_c` and `code_2cc8c` both carried stale "all addiu_at" claims.
  Bravo spotted its own unit's and correctly did NOT edit it (parallel-mode
  rules); the head fixed it.
- **`code_179d8_j` still claims 13 `addiu_at`-blocked functions.** Fixed at
  consolidation; the functions it names are alpha's queue and are not blocked.

All corrected comments now name WHICH screen was run and when, so the next
reader can age the verdict instead of re-deriving it.

### One blocker attribution was wrong in the other direction

`func_8005D864` (`Entity`) is filed as blocked with the cause attributed
**entirely** to `addiu_at`. On the text it reads as newly assignable. It is
not: re-screened, it hits `nop_mflo_mfhi` (`mflo` / `nop` / `div`), still open.
Verdict right, cause wrong — and a mechanical reopen would have staffed a
runner into a real wall. Charlie, which matched all five of its assigned
functions, signed off calling it "the still-`addiu_at`-blocked function",
inheriting the error rather than making it. Report corrected.

### The round's biggest source-shape result: a switch's CASE ORDER is readable off the binary

Four independent instances, three of them by different runners, worth 6+ words
each. For a JUMP-TABLE switch GCC 2.6.3 lays the arm blocks out in **source**
order, so: sort the arm labels by address, map them through the jump table, and
**the arm that falls through into the shared tail is the LAST case in source.**

| function | unit | cost |
| --- | --- | --- |
| `func_8003C48C` | `code_2cc8c` | 6 words |
| `func_8004FBE4` | `class_3bb8c_g` | closed it |
| `func_80032588` | `code_179d8_c` | took a 9/53-shaped structural mismatch to 48/53 |
| `func_80049EB4` | `class_39e08` | confirmed off the labels |

A fifth instance is the strongest evidence: `func_8003FB1C` (`code_2cc8c_e`,
50w) had its case order **predicted before a line was written** and matched on
attempt one. Used predictively the lever is free; used diagnostically it costs
an attempt.

Echo measured the INVERSE for a sparse non-table switch (GCC normalises the
compare order to ascending value regardless of source), and bravo then **scoped
the related switch-vs-`if` tell**: the balanced-tree `slti` signal needs at
least THREE explicit case values — with two plus a default, `switch` and
`if`-chain are byte-identical. The head had proposed that lever without the
threshold; bravo's negative is the correction.

### Other findings promoted to DECOMPILATION_LEARNINGS

- **Cross-jump shape THREE: a declared RETURN TYPE can block a merge that
  should happen.** Four identical-shaped calls merged 2+2 instead of 4, and the
  grouping partitioned them exactly by `void`-vs-`s32`. **When N calls should
  merge into one site and instead merge into groups, the grouping partitions
  them by declared return type** — a type bug, not scheduling. And a slot
  annotated `OBSERVED: <fn> (STALL, not attempted)` is a *hypothesis*: prefer
  evidence from the slot's occupant, which may already be matched as C.
- **Two VLAs, an `$fp` frame, and a rounding immediate that carries the array
  bound.** `addiu <t>, <n>, 0xf` means `buf[n + 1]`; `0xe` means `buf[n]`. That
  hex digit is the only place a `+ 1` on a VLA bound is visible.
- **A struct returned BY VALUE reads as a call with its arguments shifted** —
  hidden destination pointer in `$a0`, object in `$a1`. Size the destination
  from the gap between the argument spill area and the first saved register.
- **A local's declared WIDTH is a codegen decision; `s16` is the expensive
  default** (re-sign-extension at each use, inside loops). Widen the LOCAL, never
  the field. Two locals of the same declared type can come out differently.
- **`&arr[i + j]` and `arr + i + j` are one instruction apart.**
- **An allocated-but-unused stack frame is not a residue** — it reproduces
  automatically on a leaf with enough live locals. Do not hunt for it.
- **A wholly-unused STACK parameter, diagnosed from a fixed-offset
  disagreement** against `frame_size + 0x10` (alpha). Its symptom is a handful
  of low-nibble-only diffs on branch targets — easy to misread as instruction
  selection — and the dead-local frame trick does NOT substitute for it.
- **`volatile` for a genuine hardware register is correct source, not a lever.**
  Bravo's IRQ-mask pair needed it (14/14 each) and bravo explicitly declined it
  for a global with no MMIO evidence, measuring that negative too.
- **A function's parameter LIST is not recoverable from its own body.**
  `func_8005DF9C` has an unused second parameter that leaves no trace; the
  natural one-argument signature is a `conflicting types` error against the
  canonical declaration. Both signature mistakes hit this round produce **zero**
  hits on `error:`/`parse error` and were caught only by the round-21
  `*** [….o]` alternative — a compile-time argument for keeping it.

### Negatives worth having

- `func_8003ECD0` (71/73): two further groupings tried, both 68/73, same
  regression point as attempts 2-5. **Nine attempts across two rounds now
  converge**; the next lever is the permuter, not another hand-written grouping.
- `func_80032588`: the nested-switch and typed-temp levers the head proposed
  are both clean negatives (bravo). The tail-merge axis is narrowed, not closed.
- `func_8004109C`: five reshapes, all 42/56 or worse. The two values whose live
  ranges compete cannot both be short-lived, so the register identity is
  structural rather than a spelling accident.

### Next round

**Carve, then runners.** `fresh` closed the round at **24** (down from 50) and
what remains is large — 135 to 274 words, with the cheap seam of every staffed
unit exhausted. That is below (5 runners x ~4 functions), so Gate 2 fires. The carve targets are measured, not guessed — per-function
blocker census over the uncarved monoliths:

| segment | funcs | CLEAN | gp_rel | nop_mflo_mfhi | trampolines |
| --- | --- | --- | --- | --- | --- |
| `code_179d8_tail` | 18 | **18** | 0 | 0 | 0 |
| `code_179d8_mid_c` | 24 | **21** | 0 | 3 | 0 |
| `code_179d8` | 43 | 9 | 34 | 0 | 0 |
| `class_3bb8c_n` | 23 | 3 | 20 | 0 | 0 |
| `class_3bb8c_h` | 17 | — | 0 | 0 | **13** |

`code_179d8_tail` is 18-of-18 clean — the best carve left in the executable —
and `code_179d8_mid_c` is 21-of-24. Together they refill `fresh` to ~67.
`code_179d8` and `class_3bb8c_n` are gp_rel-dense and should be left.
`class_3bb8c_h` remains the round-17 trap: 13 of its 17 are BIOS trampolines,
so it screens clean on every blocker while being the least matchable segment
in the file. Do not carve it.

Also worth one runner: **the permuter on `func_8005CBC8` (99/100)**, which is
the single closest function in the corpus and now has a one-word residue with a
precisely described cause.

---

## 2026-09-06 — round 22: 19 matches, two runner stalls adjudicated into matches, and a third honesty mechanism

**971 -> 990 matched (71.61% -> 73.01% of game code). Build green in main
after all four merges, zero merge conflicts — `headercontention.py` reported
NO CONTENTION before provisioning, and the four assigned units shared no
project header.**

Gates 0 and 1 both passed clean, so no carve and no permuter round: `fresh`
stood at 62 and the head's screen of all 69 report-less queued functions
(both live blockers plus the BIOS-trampoline fourth screen) came back **69 of
69 CLEAN**. Four runners on cheap models, six functions each.

### Results

| runner | unit | matched | words | stalled |
| --- | --- | --- | --- | --- |
| alpha | `code_179d8_j` | 3 | 70 | 3 |
| bravo | `code_179d8_i` | 6 | 355 | 0 |
| charlie | `DreamSys` | 3 | 143 | 3 |
| delta | `class_3bb8c_s` | 5 | 176 | 1 |
| **head** | (adjudication) | **2** | **148** | — |

Bravo went 6 for 6, several on the first attempt, and confirmed
`SsUtGetVabHdr` against `include/psyq/LIBSND.H:229` — a real SDK function, not
an inherited guess.

### The adjudications: two stalls became matches

Both were classification errors, not measurement errors — every score and
every permuter figure the runners reported was accurate.

- **`func_80056BBC` (delta, 86/87 -> 87/87).** Filed as the documented
  "redundant move" class: a `li $a1, 1` in a branch delay slot with no reader.
  It has a reader — the branch-**taken** path runs seven instructions to a
  `jalr` without writing `$a1`, so it is that call's second argument. The
  unit's local vtable view typed `slot64` with one parameter, so the `1` had
  no C to come from. Six matched call sites in four other units all pass a
  second argument.
- **`func_80031BA4` (alpha, 57/61 -> 61/61).** Filed as a source-unreachable
  scheduling residue. The naive body reproduced the "unreachable" ordering on
  the first try in isolation; the real gap was a missing empty 8-byte frame.
  Alpha's own three levers on the naive body give 61/61 — what broke it was
  four more lines of scaffolding stacked on top, which moved the residue alpha
  then reported.

Both are written up in full in their match reports, and both generalise; see
DECOMPILATION_LEARNINGS' round-22 entries. The transferable half is that a
**wrong CAUSE** on a near-miss is what closes a function to future rounds —
neither of these would have been re-opened by anyone reading the reports as
filed.

One adjudication came back **negative and is recorded as such**:
`DreamSys__InstanceEffectsOnJournal`'s cast/typing axis is now closed (three
forms tested, all inert or worse), with the one-argument-vs-two-argument call
shape identified as the remaining lead.

### Tooling: the `REOPENED -- ASSIGNABLE` marker

`progress.py` counts any queued function with a match-report file as a
documented stall. When a blocker is RESOLVED, every report written while it
was live keeps its function out of `fresh` permanently — nobody re-measures a
function everybody believes is blocked. Round 21 resolved `addiu_at` and
annotated five live reports with "BLOCKER RESOLVED — THIS FUNCTION IS NOW
ASSIGNABLE" in prose no tool could read.

Added an exact-phrase marker, the counterpart to `DELIBERATELY UNWORKED`, and
applied it to those five (`fresh` 62 -> 67 before the round's own work). Gate 1
in PARALLEL-RUNS now requires marking a blocker's reports as part of resolving
it. `gp_rel` alone has 82 that will need it.

### Next round

Runners again — `fresh` is 50 with no carve needed. The five reopened
functions are the sharpest ground: they are cold, blocker-clean, and were
triaged as hard only under a blocker that no longer exists.

---

## 2026-09-06 — round 21: 22 matches off fresh carve, an oracle defect fixed, and a three-round-old figure corrected

**949 -> 971 matched (69.99% -> 71.61% of game code; 50.04% -> 51.54% of game
bytes). Build green in main after all four merges, zero merge conflicts —
`headercontention.py` reported NO CONTENTION before provisioning.**

The round started with **`fresh` at 0**: all 220 queued functions across 55
carved units were documented stalls. So Gate 2 fired and the head carved
before staffing anyone.

### Carve (head, 3 units, each verified separately, all zero-byte-change)

| unit | funcs | clean | source |
| --- | --- | --- | --- |
| `code_179d8_i` | 18 | 9 | front slice of old `code_179d8_tail` |
| `code_179d8_j` | 26 | 12 | **middle** slice of `code_179d8_mid_c` |
| `class_3bb8c_s` | 10 | 5 | whole segment — banked `DELIBERATELY UNWORKED` |

Cut by blocker density rather than by "next": `code_179d8_mid_c`'s head is 5
clean of 24 and stays asm, so `code_179d8_j` comes out of the middle leaving a
one-function remainder (`code_179d8_mid_d`). Neither cut needed a rodata
attach — `code_179d8_mid_c` has zero `jtbl_` and zero `.word .L` across the
whole monolith, and `code_179d8_i`'s boundary keeps both of the old tail's
switch tables in the remainder. `class_3bb8c_h` was ruled out on the
trampoline screen: 15 of 17 "clean" but 13 BIOS trampolines, confirming the
round-17 note. 28 stub reports written for the blocked functions.

`fresh` 0 -> 20, `banked` 0 -> 5, uncarved 187 -> 133.

### Matches

| runner | unit | result |
| --- | --- | --- |
| alpha | `code_179d8_i` | **9 of 9**, five on the first attempt (incl. a 157-word dispatcher) |
| bravo | `code_179d8_j` | **10 of 11**, one stall (`func_80031D6C`, register identity) |
| delta | `code_2cc8c_e` | **2 matches**: `func_8003F848` 173->177/177, `func_80040154` 101->103/103 |
| charlie | `code_8220_c` | **0 matches** — confirmatory negative, 8 functions foreclosed |

Both freshly carved units were worked out in one round: `code_179d8_i` has no
workable ground left and `code_179d8_j` has none either, which is why `fresh`
is back to 0 (see "next round" below).

### The round's most valuable result was not a match

**The documented oracle grep missed 6 of 7 fatal compile errors, and had done
for 20 rounds.** Found by the head while running an ordinary experiment, not
while auditing: a duplicate `typedef` produced `build exit=2`, **zero grep
hits, and a plausible funcdiff score** — the exact signature the loop teaches
you to read as "fresh build, does not match yet". Only `funcdiff.py`'s mtime
staleness guard caught it, and that guard is documented as a BACKSTOP.

GCC 2.6.3 predates the `error:` prefix convention. Measured through the pinned
pipeline, all seven fatal (`cc1` exits 33): `parse error` is caught;
`conflicting types`, `redefinition`, `undeclared`, `too many arguments`,
`incompatible types in return` and `duplicate member` are all **missed**.
`parse error` covers syntax and `undefined reference` covers the linker — every
SEMANTIC error in between was invisible.

Fixed by matching make's failure on a COMPILE TARGET rather than message text:
`\*\*\* \[[^]]*\.o\]` fires on `[build/src/<unit>.c.o] Error 33` and not on
`[Makefile:74: check] Error 1`, preserving round 18's result while closing the
gap it opened. Updated in CLAUDE.md (with the measured table),
`docs/MATCHING-GUIDE.md` and the runner prompt, and broadcast mid-round to all
three live runners. **All three re-verified their banked scores under the
corrected grep; no score moved, and none had hit a masked error.** That
negative is worth recording — the defect was real but did not corrupt this
round's results.

### `func_800400B0`: a wrong figure that survived three rounds

Delta found `code_2cc8c_e`'s `func_800400B0` had carried "29/41, same total
instruction count, purely reordered" through rounds 18, 19 and 20. It is a
genuine one-word length regression — 40 compiled words against retail's 41,
with `func_80040154` linking at `0x80040150` instead of `0x80040154`.

**The head reproduced it, and the drift guard fires loudly every time:**
`WARNING: the build differs OUTSIDE this range too (224685 bytes)`, printed in
the same output as the score, three rounds running. **This is therefore NOT a
new way a score lies and was deliberately not filed as one** — CLAUDE.md is
explicit that recording a fired-guard case as an oracle defect teaches the next
runner to distrust the oracle exactly where it worked. It is round 20's
drift-attribution lesson in a worse form: there the drift was misattributed,
here it was not read at all. Written up under the round-19 preserved-body entry
as a sixth instance.

### charlie's negative, and the head's follow-up

charlie re-confirmed the `code_8220_c` OT-splice family (8 siblings, 46/54 to
104/112) as a genuine wall, and did one thing better than previous rounds: it
checked the "same cause, same position" claim by grepping all eight siblings'
own disassembly side by side rather than by pairwise analogy across reports.
The head verified this independently — `lui $a2, 0xFF000000>>16` sits at
instruction #13 in all eight.

charlie ruled out the `__asm__("")` barrier lever by ANALOGY to round 20's
`func_8003DAD4` regression. The reasoning was sound, but the project's rule is
that a lever's scope is measured, so the head ran it: four placements on
`func_8001A064`, three inert at 104/112, one regressing to 18/112. Same answer,
now evidence — and it produced a **precondition** that generalises past the
family: a barrier reorders instructions GCC already scheduled and cannot
CREATE one, so an ABSENT-instruction residue (`retail=<insn> built=00000000`)
is not a barrier candidate at all. Recorded in DECOMPILATION_LEARNINGS.

The head also spotted a one-past-end-pointer lead in this family and did NOT
re-send charlie on it — it turned out to be already on record as
`align_up_4(last_offset + width)` with a recorded negative permuter search.

### Learnings promoted

Two existing entries extended (the `__asm__("")` barrier precondition; the
preserved-body drift entry's sixth instance) and three new ones: the
delete-a-named-value lever pair from delta's two matches, the same-size
sibling family as one unit of work, and a cluster of narrowing/branch-polarity
bullets from alpha and bravo — including the stack-argument `lhu` vs
`lw`+`sll`+`sra` rule, which had never been recorded for the stack case.

### POST-ROUND: the `addiu_at` blocker was RESOLVED (operator-authorised)

After the round closed, the operator authorised the bounded experiment the
`addiu-at-blocker.md` escalation had been waiting on since 2026-08-30. It
worked, and it changes the next round's triage completely.

**The fix is option 1 from that doc's own list**: a `--addiu-at` flag on maspsx
that sets `addiu_at` ALONE, decoupled from the sub-2.30 version bump that drags
three nop-insertion rules with it. `tools/patches/maspsx-addiu-at.patch`,
committed here and applied by `tools/setup.sh`; `MASPSX_FLAGS` passes it.

Evidence, in order taken: the census reproduces at 971 matched (502 unfolded,
0 folded, **all 502 still unmatched — the pin had never been exercised against
this construct in 21 rounds**); `rm -rf build` + full rebuild stays byte-exact,
so the flag is provably inert on every matched function; the pinned pipeline
now emits retail's unfolded four-instruction form; and a real blocked function
(`func_800305F4`) went from unmatchable to 6/21 at exact length with the
previously-impossible `addiu $at,$at,%lo(...)` matching word for word. No C was
kept from that test.

**Effect on triage: `fresh` 0 -> 62, blocker-clean 85 -> 161, blocked 167 -> 91.**
76 functions unblocked. 66 pure blocker stubs deleted so those functions return
to `fresh`; one report rewritten to cite only its remaining `gp_rel` hit; five
substantive reports KEPT with a stale-verdict banner because their structural
analysis is still good — `func_80018464`, `func_8003C48C`, `func_8003C63C`,
`func_8004109C`, `func_80049EB4`. `func_8003C63C` is the function CLAUDE.md
cites as the case where seven attempts went into the wrong half of a two-word
residue; that half is now matchable.

`tools/nearmiss.py` no longer counts `addiu_at` as a blocker but still reports
it, tagged `addiu_at(RESOLVED-not-a-blocker)`, so a stale verdict is
distinguishable from a real block. CLAUDE.md's screen is down to two greps.

**Still open, and still the operator's:** `gp_rel` (82 functions, `-G` tried
and rejected) and `nop_mflo_mfhi` (9 functions). The latter is now the obvious
next candidate for exactly the same treatment — its own census already argues
it is the right mechanism at the wrong granularity, which is what was true of
`addiu_at` before this. **Untested. Not a head decision.**

**One pre-existing hazard this sharpened rather than caused:** `tools/setup.sh`
clones maspsx `--depth 1` from upstream master with **no pinned commit**. If
upstream moves the patched lines the build silently becomes something else —
except that `setup.sh` now fails loudly pointing at the research doc. Pinning
the clone, or upstreaming the flag to mkst/maspsx, are both operator calls.

### Next round

**SUPERSEDED by the POST-ROUND section above — `fresh` is 62, not 0.** Left
here rather than rewritten, because the reasoning below about WHICH ground is
left is still correct and only the staffing conclusion changed.

As the round closed `fresh` was **0**, and the plan was: `class_3bb8c_s` is
carved and banked with 5 workable functions, so one runner can start without a
carve; anything beyond that needs Gate 2 first. **The `addiu_at` resolution
removed that constraint entirely** — 62 fresh functions across many units is
enough to staff a full round with no carve at all, and Gate 2 should NOT fire
next round. Remaining uncarved game code is 133 functions and the good
windows are thinning — `code_179d8_mid_c`'s head (5 clean of 24),
`code_179d8_tail`'s remainder (1 clean of 18, both switch tables), `code_179d8`
proper (9 clean of 42, gp_rel-dense) and `class_3bb8c_n` (2 clean of 22).
`class_3bb8c_h` is 13 BIOS trampolines and needs an `hasm` disposition decision
rather than a runner.

The blocker-clean near-miss corpus (`tools/nearmiss.py`) remains the
better-posed queue at 81 entries, and round 21 is evidence it pays: delta
closed two long-standing ones. **But rank it with fresh scepticism about title
figures — round 21 makes three consecutive rounds in which a title-line number
was wrong, by a third distinct mechanism each time.**

---

## 2026-09-05 — round 20: 2 matches, six near-misses moved, and four inherited reports found wrong

**947 -> 949 matched (69.84% -> 69.99% of game code, 50.04% of game bytes).
Build green in main after every one of the round's 11 merges. Zero merge
conflicts — the five units were staffed with disjoint header sets and
`headercontention.py` reported NO CONTENTION before provisioning.**

**This round bought progress on RESIDUES rather than on the match count, and
that is the honest headline.** Two functions closed. Six moved substantially,
four of them by getting a compiled LENGTH exact — which converts a score that
`funcdiff.py` cannot be trusted to report into one that it can:

| function | unit | before -> after | note |
| --- | --- | --- | --- |
| `func_8003FC70` | code_2cc8c_e | **MATCH 35/35** | head; stall standing since round 14 |
| `func_8001A4C0` | code_8220_c | **MATCH 35/35** | echo; raw asm reworked into ordinary C |
| `func_8003F848` | code_2cc8c_e | 0 -> 173/177 | delta; never attempted before |
| `func_8004EF6C` | class_3bb8c_f | 0 -> 188/240 | charlie; never attempted, length off by 1 |
| `func_8002AA6C` | code_179d8_g | 119 -> 202/223 | bravo; **length now exact** |
| `func_8004BE54` | class_3bb8c | 0 -> 130/150 | charlie; **length exact**, 4 structs derived |
| `func_8001E110` | code_d294_b | 16 -> 95/118 | alpha; false diagnosis corrected |
| `func_8001D714` | code_d294_b | 130 -> 141/143 | alpha; permuter-found cross-jump lever |
| `func_80028DF0` | code_179d8_b | 10 -> 45/82 | bravo; **length now exact** |
| `func_80028F38` | code_179d8_b | 11 -> 46/79 | bravo; **length now exact**, fix transferred |
| `func_8002B4D4` | code_179d8_g | 34 -> 60/91 | bravo; **length now exact** |
| `func_80032BB8` | code_179d8_c | 7 -> 12/14 | echo; overturned "not fixable by reshaping" |
| `func_80059BE0` | DreamSys | drift 2 -> 1 word | echo; tail-merge defect fixed outright |

### The round's most transferable finding: inherited report PROSE is unreliable

**Four runners, in four unrelated units, each independently found a false
mechanism claim in a report they inherited** — alpha (`func_80065AE0`, a
padding local credited with a fix it never made), bravo (a prose/`objdump`
mismatch that was hiding a real fix), charlie (`func_8004C470`, retail and
built **swapped**), echo (`func_80061778`, a flat merge that is really a nested
two-level cross-jump). Alpha's `func_8001E110` is the costliest instance: a
"16/118, no drift" claim carried across rounds 13 and 19 while the contradicting
drift warning sat in the same command's own output. Correcting it reached
95/118.

`funcdiff.py` scores bytes and has no opinion about narrative. A wrong score is
corrected the next time anyone measures, because measuring is the job; **a wrong
narrative is nobody's job to re-measure**, so it survives and the next runner
builds on it.

### Classes settled, opened, and declined

- **Commutative-operand-order canonicalization: SETTLED**, 6 instances across 3
  unrelated units. cc1 fixes a commutative op's register operand order
  regardless of C source order — charlie's decisive datum is that *both* C
  orders produce the identical wrong output. Promoted on the strength of a
  discriminating test, not a tally.
- **"Tail merge" is an UMBRELLA, not a class.** echo separated merge-COUNT from
  merge-DEPTH and showed neither lever transfers.
- **Dead-call frame sizing is a screen for frame SIZE ONLY.** alpha falsified
  the round-19 census's two strongest rows with an isolated `cc1` reproducer: a
  dead 6-arg call and an unused local produce byte-identical output.
- **Delay-slot-fill choice** recorded as an OPEN observation, not a class —
  nobody has found its discriminating test.
- **A fifth "way a score lies" was proposed and DECLINED**, per the round-13
  precedent: alpha's own report confirms the drift warning fired, so the oracle
  worked. Recorded instead as an attribution hazard on way #3.

### Head errors this round, both recorded in the learnings

- **I gave charlie a figure I had not rebuilt.** `func_8004F8A4`'s title said
  "1 word / 4 bytes short" — a LENGTH statement — and I staffed it as the
  closest function in the queue. Its match score was 33/77. This is a *new*
  sub-case: rounds 18 and 19 misread figures out of report BODIES, mine came
  from the TITLE, which the guidance calls safe. Titles should give both
  quantities.
- **My COP2-clobber hypothesis for `code_8220_c` was wrong.** echo falsified it
  cleanly (zero GTE/COP2 mnemonics in all ten functions) and I re-verified with
  a sanity check that the grep fires 8x on the unit's known GTE bodies.

### Process

**Re-sends dominated again, as in round 19: 11 merges from 5 runners.** echo
ran six passes across five units. Every runner's first pass was merged before
its second was assigned, so nothing was ever at risk in a worktree.

`tools/nearmiss.py` was added — Gate 1b's queue as a tool, because heads have
re-implemented the `nop_mflo_mfhi` screen backwards twice and misread report
figures three times. It shells out to the canonical greps and prints verdict
text verbatim rather than extracting figures. Built the census by hand first;
both agreed exactly.

Rodata slot `0x1908` was split at `0x1994`, dissolving round 14's "alignment
constraint" lead — there is no alignment constraint; `D_80011194` is a string
literal and the C was emitting a second copy of it.

**Next round: runners again.** `fresh` is 0 and the 187 uncarved functions are
only ~26% blocker-clean (best segment 33%; `class_3bb8c_h` is 13/17 BIOS
trampolines), so carving still loses to the near-miss queue — 81 blocker-clean
functions, now much better characterised than at round 20's start. Best
first targets: `func_8001D714` (141/143), `func_8003D73C` (144/145 compiled),
`func_8004EF6C` (188/240, one word of length), `func_8003F848` (173/177),
`func_8002AA6C` (202/223). Run `python3 tools/nearmiss.py` rather than trusting
this list.

---

## 2026-09-05 — round 19: 17 matches, the last fresh function, and a frame size that reads dead code

**930 -> 947 matched (68.58% -> 69.84% of game code). Build green in main after
every one of the round's 10 merges, and every claimed match re-verified
INDIVIDUALLY after its merge.** Gate 1 was dry (`fresh` = 1 of 239), Gate 2's
carve was **rejected on measurement**, so this was a second Gate 1b round
against the register-shaped near-miss corpus. `fresh` is now **0** and
`stalled` equals `queued` exactly (222 = 222): every queued function in the
project has a report.

| runner | units | passes | matched | end state |
| --- | --- | --- | --- | --- |
| alpha | `code_2cc8c_e/_f/_b/_c`, `code_2cc8c` | 2 | 8 | reported cleanly |
| bravo | `class_3bb8c_b/_j/_l`, `class_3bb8c` | 1 | 1 | reported cleanly |
| charlie | `code_179d8_b/_c/_d/_f/_g` | 2 | 0 | reported cleanly |
| delta | `Entity_e/_g`, `DreamSys`, `class_3bb8c_p`, then `code_8220_c`, then `class_3bb8c_t` | 4 | 5 | reported cleanly |
| echo | `code_d294_b/_c`, `code_55dd4`, `class_3ac78`, `class_3bb8c_e` | 2 | 4 | reported cleanly |

All five reported cleanly for the second round running. **Cross-runner header
contention was zero across all 10 merges** — units were grouped so each project
header had exactly one owner, and `headercontention.py` was run on the plan
before provisioning.

### Gate 2 was rejected on measurement, and here is the measurement

Four-screen census over all **187** uncarved game functions: **46 clean.**
`code_179d8_mid_c` (51 funcs, 17 clean) and `code_179d8_tail` (36, 10) are
`addiu_at`-saturated; `code_179d8` (43, 9) is `gp_rel`-saturated;
`class_3bb8c_h` (17, 2) is 11 BIOS trampolines, confirming round 17. The
best-density segment, `class_3bb8c_s` at 5 of 10, is too small to staff. No
20-function window approaches what round 16 carved.

Method note, since Gate 2 warns every written census goes stale by being acted
on: this one split each monolithic `asm/*.s` at `^glabel` into per-function
files and ran the **documented shell greps verbatim** against each, rather than
re-expressing the `nop_mflo_mfhi` window in Python. That reimplementation has
now been inverted three times by three heads; shelling out costs nothing and
cannot invert.

### The round's finding: an oversized outgoing-arg frame reads DEAD CODE

**GCC 2.6.3 sizes the outgoing-argument area from every call expression's
argument count during RTL expansion — BEFORE dead-code elimination.** A call
inside `if (0)` emits zero instructions and still enlarges the frame.

Echo closed `func_8001EE98` (**31/31**) on this. Retail reserved `0x18` (six
words) against an o32 minimum of `0x10`, with one surviving `jal` to a callee
using only `a0`/`a1`/`a2`, and nothing ever written or read in those 24 bytes.
The reproducing source is an unprototyped `func_80015618()` called with three
live arguments plus a **dead six-argument call** to it inside `if (0)`.

**How it was reached is the process point.** Echo filed this as an unexplained
gap after building a reproducer proving a 3-argument call cannot reach 24
bytes. That reproducer was sound and its conclusion correct — but it tested the
hypothesis already known to be false, so it could not discriminate. The head
re-read the frame, confirmed the 24 bytes are untouched, observed that 24 is
*six words*, and handed back the six-argument hypothesis. That is the "verify
the reasoning, not just the score" job paying off; the runner had done all the
hard measurement and was one framing away.

A **census of the signature is now in DECOMPILATION_LEARNINGS**: 14 live
functions reserve more than `0x10` of outgoing-arg space that nothing ever
touches, 8 of them blocker-clean. **Two — `func_80065A5C` and `func_800662BC`,
both `code_55dd4`, both small and clean — have ZERO `jal` instructions**, so no
live call can justify any outgoing area at all. Those are the strongest leads
the project currently has.

It is a screening signal, not a verdict: echo checked `func_80065AE0`, which
has the same numeric signature, and found a parameter-copy timing residue
instead.

### The aggregate-assignment lever, and how it got bounded

Alpha closed **five** functions with one idiom — whole-struct/aggregate
assignment instead of scalar field-by-field copy, not interchangeable to GCC
2.6.3 even for a 3-byte struct. It was broadcast mid-round **with its boundary
stated**, because all five closures were siblings in one unit family and round
18's head had over-promoted a lever on a single success.

Every runner was asked for the **negative**, and all four answered:

- **delta** hit the same idiom independently in `DreamSys` (a 3-word
  out-parameter copy, 9 instructions vs retail's 6) — a second family, which is
  what promoted it from lead to learning, and an out-parameter variant that
  widens it past the "block copy in a loop" framing.
- **echo** found a real candidate in `code_55dd4`, tested it via a wrapper
  struct (arrays are not assignable in C89), and got **identical score, no
  movement**.
- **charlie** reported the shape absent from its units, with the two candidates
  named.
- **delta again**, on `code_8220_c`, produced not just a negative but a
  **mechanism**: the filler's operand is a compile-time-constant `self +
  literal`, which folds to the same instruction however it is spelled, leaving
  nothing for the delay-slot filler pass to hoist — and retail's own
  disassembly shows the tail copies manually unrolled, so making the offset
  runtime-valued is not available.

That yields a **predictive rule** rather than a list of exceptions: the lever
helps where the address is a genuinely runtime-only value that resists constant
folding, and does nothing where the compiler folds it anyway or where a
naturally-aligned same-type run already compiles optimally. Asking for the
negative is what produced this; the tally alone would have said "5 for 7".

### A permuter number is in PERMUTER units — three misreadings, one root cause

Three different permuter outputs were read as word counts, across two rounds:

| read as a word count | actually |
| --- | --- |
| `"Reorderings: 2"` | a bucket label for the scorer's internal edit-distance op |
| base score `210` | a weighted penalty (2 reg-diffs x5 + 1 ins x100 + 1 del x100) |
| `Stack Differences: 0` without `--stack-diffs` | a field never filled in |

**`func_8004042C` is the expensive one.** Its report title said "closed to ONE
isolated word". Alpha rebuilt it: **22/25 — three words.** The missing `move
$t0,$a1` is one missing *instruction*, but its absence forces two downstream
loads to read the wrong register. A single-instruction cause is not a
single-word residue. That figure had propagated from the report title into
round 18's PROGRESS, into round 19's staffing, and back to alpha as an
instruction to close "the one remaining word" — **three consumers, none of whom
could have caught it without rebuilding.** Alpha corrected the title and every
downstream claim, and reconfirmed permuter-exhausted over two searches.

### Preserved-body drift: five wrong out of ~thirty, found by three runners

Delta found two reports claiming "clean / drift-free" bodies that compile 2 and
1 words longer than retail. Broadcast mid-round; alpha independently found a
third (`func_80040C00`, claimed clean, actually 4/52 with 191204 bytes of
drift) and closed the function only because it re-derived from `objdump`.
Charlie and echo then verified ~20 more bodies as sound.

So the hazard is real at roughly **one in six** and now bounded rather than
alarming. The standing check is in DECOMPILATION_LEARNINGS in delta's own
wording.

### "Unscoreable" describes a SESSION's state, not a function's

Two 4c-salvaged bodies, both from runners killed mid-function:

- `func_8005942C` — filed at 19/56 with **140723 bytes of drift**, labelled
  "far from correct, not a near-miss". Delta **matched it 56/56**; the control
  flow was already right and it had three separable expression-shape bugs.
- `func_8002AA6C` — **never scored at all**, because no author survived to run
  the oracle. Charlie compiled it unchanged and got **119/223, one instruction
  short.**

Before reshaping a salvaged body, compile it as-is and take a number. A
salvage's paperwork looks exactly like a well-worked stall's, so it inherits
authority it never earned.

### Charlie: zero matches, and the discipline of the round

Charlie found a **genuine permuter zero** on `func_8002B3F4` and **rejected
it** — closing it required a `volatile` global that shifted a neighbouring
symbol's linked address and corrupted an already-matched sibling. A permuter
scores the target in isolation and has no view of the link, so a change that is
locally perfect and globally destructive scores as a win.

Charlie later distinguished the two cases that look identical in source:
`volatile` on a *global's declaration* (moves symbols, dangerous) versus on a
*local pointer dereferencing it* (safe, and the lever that took `func_8002B4D4`
17/91 -> 34/91). Its second pass, moved deliberately off the register-shaped
set to a different residue class, produced `func_8002A75C` **58/196 -> 171/196**
on two structural fixes.

### Re-sending runners was the highest-yield structural move

Nine of the round's 17 matches came from second, third and fourth passes by
agents already holding the context. **Delta alone ran four passes**: 5 matches,
three overturned verdicts, one mechanically-explained negative on the hardest
cluster in the project, and it closed the last fresh function. A zero-match
first pass is not a wasted pass — charlie's produced the round's best negatives.

Two of round 18's 2c problems stayed fixed (every search bounded, every one
terminated) and the third recurred once: alpha ended a turn on a status line
with uncommitted work and two concurrent searches, one of which had drifted into
the **main checkout** via the cwd reset. An explicit numbered work order with
hand work in it fixed it immediately, exactly as round 18 recorded. Main's tree
was clean throughout; the stray search damaged nothing but its results were
unattributable.

### Head errors this round — three, all the same root cause

1. **Ranked the queue by parsing residues out of report BODIES.** Round 18 had
   already written down that only the title/verdict line is safe. Charlie caught
   `func_80032BB8`'s "0/14", which I had pulled from a sentence comparing a
   **rejected** variant; the real residue is 7/14. Round 18's phrasing ("treat
   figures near correction/superseded wording as retracted") is necessary but
   not sufficient — this figure sat in an ordinary attempt narrative with no
   warning keyword near it.
2. **Told delta `func_80058228` was the fresh function.** It was backwards:
   that one had a report (a 52/56 stall) and `func_80058404` had none. My own
   near-miss data showed it and I named them the wrong way round. Cost nothing
   — delta matched both — but delta had to spend a paragraph correcting the
   assignment.
3. **Repeated the "one word remaining" figure** for `func_8004042C` into
   alpha's assignment without rebuilding it.

All three are the same error: **trusting a derived summary of my own census
instead of the census.** The mitigation is mechanical, not attitudinal — build
the Gate 1b ranking from title lines only, and rebuild any inherited figure
before putting it in an assignment.

### Two hook defects, both measured, neither worked around

Round 18 escalated "`make extract` is BLOCKED in the main checkout though the
hook lists that target as allowed, and it accepted the identical command in all
five worktrees; three invocation forms were tried and none worked."

**It is redirection, and the hook is not worktree-sensitive.**
`block-raw-make.py` tokenises with `shlex(punctuation_chars=True)` and splits at
`SEPARATORS`, which contains `|` and `;` but **no redirection operator** — so in
a redirected invocation the `>` and the log path stay inside make's span and are
judged as *targets*. Measured: bare and piped forms are ALLOWED; `> log` and
`2>&1 | tail` are BLOCKED. Round 18 simply happened to use redirected forms in
main and bare ones in the worktrees.

**Second defect, found by hitting it: the hook blocks PROSE about itself.**
Writing this section via a `cat <<'EOF'` heredoc was blocked, because a heredoc
body is command position to the tokeniser. The hook's docstring claims to have
fixed exactly this papercut class for `grep make Makefile` and `echo "run
make"`. This very likely bit round 18's head while writing its PROGRESS entry.

Both have clean workarounds (pipe instead of redirect; write files with the
Write tool). **Fixing a guardrail is the operator's call and neither was
touched.**

### Other tooling defects (measured, not acted on)

- **`tools/setup-permuter.sh` validates seeds through the real `cc1`, which
  cannot parse `PERM_GENERAL`/`PERM_VAR`** — so the guided-PERM approach several
  match reports recommend as a next step is not usable through this harness as
  written. Alpha hit it; the recommendation is stale wherever it appears.
- **Permuter scaffolds disagree with the real in-context build, three times, all
  inside `class_3bb8c.h`.** Possibly systemic to that class's register pressure.
  Check the scaffold's base score against `funcdiff`'s residue before spending a
  search.
- **`make clean` + re-extract desync** (charlie): extracting `asm/` while a
  `src/` file still holds a non-`INCLUDE_ASM` body leaves that function's `.s`
  missing and hard-fails the next revert. Restore the `INCLUDE_ASM` first.
- **Two runners did not capture permuter exit codes to a dedicated file**, which
  is the round-18 remedy for the lost-`$?` race. Alpha self-reported it.

### The rodata-migration blocker has exactly ONE instance

`func_8003FC70`'s report documents a fifth blocker the four-grep screen cannot
see. Censused: 16 generated `.s` files carry a migrated `.section .rodata`, all
live — and **15 are already caught by the `addiu_at` screen**, because the
migrated symbol is a `jtbl_*` and a jump table is that blocker by construction.
Only `func_8003FC70` is clean-screened and blocked by migration alone.

**So it does not earn a fifth standing screen** — the same measured-scope
argument `addiu-at-blocker.md` makes for itself. A way out exists (split slot
`0x1908` at `0x1994`, leaving `D_80011194` standalone; a plain symbol resolves
from a standalone slot, per the settled `0xA8C` precedent). It was deferred: a
`config/` change inside a live runner's unit family is how a head breaks its own
merge.

### Round 19 follow-up: two of the three tooling defects are FIXED

Fixed with the operator's authorisation after the round closed. Both were
workflow tooling, not the pinned toolchain -- no compiler, flag, `maspsx` or
verification file was touched, and `check.sha1`/`build.sha1` are untouched.

**1. `block-raw-make.py` no longer blocks what it advertises as allowed.**
Two defects, one cause -- the tokeniser judged non-target words as make
targets:

- **Redirection.** `SEPARATORS` had no redirection operator, so in
  `make extract > log` the `>` and the log path stayed inside make's span and
  were judged as *targets*. Adding them to `SEPARATORS` would NOT have worked:
  `shlex(punctuation_chars=True)` splits a leading file descriptor into its own
  token *before* the operator, so `2>&1` arrives as `("2", ">&", "1")` and the
  stray `"2"` would still have read as a target. The fix is a
  `strip_redirections()` pass that drops the operator, its target word, and a
  preceding bare-digit fd.
- **Heredoc bodies.** A heredoc body was tokenised as though it were a command
  line, so writing documentation *about* the hook was blocked by the hook --
  the exact papercut class its own docstring claims to have fixed for
  `grep make Makefile`. `strip_heredoc_bodies()` removes them.

  **The heredoc fix does NOT become a bypass**, which is the part worth
  checking: a heredoc fed to a *shell* really is a script, so the body is
  recursed into rather than discarded when a shell name appears in the
  introducing line. `bash <<'EOF' / make build / EOF` is still BLOCKED.

  Verified against 28 cases -- 15 that must be allowed and 13 that must still
  be blocked, including `make`, `make build`, `make > log`, `make 2>&1`,
  `make extract && make`, `sh -c 'make build'`, `time make`, and the two shell
  heredocs. The allow-rule is unchanged (`targets and all(t in
  ALLOWED_TARGETS)`), so a bare `make` with no targets still fails it.

**2. `tools/setup-permuter.sh` no longer dies opaquely on a PERM_* seed.**
Its scaffold validation compiles the seed with the real `cc1`, which has never
heard of `PERM_GENERAL`/`PERM_VAR` -- those are read by the permuter's own
pycparser front end, which substitutes concrete variants before any compile.
The result was `parse error before 'int'` under `set -e`, aborting the script
with no hint, which is why several reports' recommended "hinted PERM_GENERAL
search" could not be followed. Now detected: the scaffold build is skipped, the
reason is printed, and the permuter's own `--debug` is named as the correct
validator. Recorded as trap 5 in the script's own header. Both branches tested.

Two smaller fixes in the same file: the printed advice now says
`--debug --stack-diffs` in both places (without it a pure frame-size residue
falsely scores ZERO -- round 18), and the closing line no longer claims "base
compiles" in the branch where it deliberately did not.

**3. NOT fixed, and it is not a harness bug: permuter scaffolds disagreeing
with the real in-context build.** Three instances, all inside
`class_3bb8c.h`. The mechanism is real and not a defect to patch out -- an
isolated single-function compile genuinely does not reproduce the surrounding
register pressure, so it schedules differently (`func_8004BB3C`: the scaffold
hoists a pointer computation the real build defers past a `blez` guard). No
scaffold generator can fix that; it is what "compile one function alone" means.

What the script now does instead is make the DETECTION cheap: it prints the
`--debug --stack-diffs` invocation and says to check the base score against the
match report's residue before searching. **If the scaffold's base score
disagrees with `funcdiff`'s residue, the scaffold is scoring a different
problem and its results will not transfer.** That check is the remedy; the
divergence itself stays open, and whether it is systemic to that class's
register pressure is still unmeasured.

### Next move

**Runners again, and the queue is better posed than it was at the start of this
round.** Carving is still not the move — `fresh` is 0 but the 187 uncarved
functions are 75% blocked and the best window is 5 of 10.

Named entry points, in order:

1. **`func_80065A5C` and `func_800662BC`** (`code_55dd4`, both clean, both
   small) — zero `jal`, reserving `0x18` of outgoing-arg space that no live call
   can justify. The dead-call mechanism's best targets.
2. The rest of the 8 blocker-clean rows in the outgoing-arg census.
3. **`func_8002AA6C`** at 119/223 one instruction short, and **`func_8002A75C`**
   at 171/196 — both charlie's, both freshly narrowed with the residue named.
4. **`func_8003D73C`** at 144/145 with the register-saturation class already
   dissolved.
5. `func_8003FC70` via the rodata re-segmentation, as a head task at Gate 2.

The register-shaped corpus is not exhausted — 17 matches came out of it this
round and the contaminant list grew from three mechanisms to seven.

---

## 2026-09-05 — round 18: a permuter round, 9 matches, and a stall class that is 26% of the queue

**921 -> 930 matched (67.92% -> 68.58% of game code). Build green in main after
every one of the round's ~20 merges, and every claimed match re-verified
INDIVIDUALLY after its merge, not just on the whole-image SHA1.** Gate 1 was
dry (`fresh` = 1 of 248 queued), so this was a Gate 3 permuter round against
the Gate 1b near-miss corpus rather than new ground.

| runner | units | matched | end state |
| --- | --- | --- | --- |
| alpha | `code_8220_c` | 0 | reported cleanly |
| bravo | `code_2cc8c_e/_f/_c`, `code_2cc8c` | 2 | reported cleanly |
| charlie | `code_55dd4`, `code_d294_c` | 1 | reported cleanly |
| delta | `Entity_e/_g/_d`, `DreamSys`, `code_179d8_g` | 2 | reported cleanly |
| echo | `class_3bb8c_b/_f/_j`, `code_179d8_h` | 4 | reported cleanly |

All five reported cleanly — the first round since 13 with no runner lost to
infrastructure. Cross-runner header contention was **zero by construction**:
units were grouped by header family so each project header had exactly one
owner, and none of the ~20 merges conflicted.

Matches: `strstr` 30/30, `func_80028C54` 27/27, `strcpy` 17/17, `func_8004EEA0`
51/51 (echo); `DreamSys__LogMood` 14/14, `func_80064FBC` 70/70 (delta);
`func_8001EDAC` 22/22 (charlie); `func_800402F0` 66/66, `func_80040D74` 40/40
(bravo).

### The round's finding: "register-identity" is the least reliable verdict in the corpus

Two functions filed as unfixable register-identity stalls were overturned, and
a census then showed **62 of 240 queued functions — 26% — carry a
register-shaped verdict.**

- **`func_8004EEA0`** (matched 51/51): the cause was a masked byte parameter
  **mistyped `s32`**, cascading into what merely looked like a register
  permutation across the whole body.
- **`func_8004042C`** (25 words, now 1): filed as a whole-function register-bank
  swap, its report explicitly recording that the permuter was never tried
  *because of* that filing. A whole-struct assignment fixed a real structural
  defect and flipped `self`'s allocation to match retail.
- **`func_8003FCFC`** (third instance, partial): its "register bank differs"
  verdict was a **discarded return value** — retail copies `dst` into `$v0` for
  a `void`-declared function. `--debug` 290 -> 135.

**The epistemics trap is the transferable part.** `func_8004EEA0`'s wrong
verdict had been *corroborated by a sibling* (`func_8004C93C`, seven variants,
zero movement). Two functions agreeing did not validate the class, because both
shared the same unexamined assumption. Echo wrote the correction into
`func_8004C93C`'s own report rather than letting the match imply a verdict it
had not earned.

**The head over-promoted the fix mid-round and the runners corrected it.** The
type/declaration axis was broadcast as "the reusable lever" on the strength of
one success; the round's full evidence is that it closed exactly that one
function and was a **clean negative everywhere else** — alpha's five variants,
charlie's two exhaustive searches, echo's direct test on `func_8004C93C`
(1/109, markedly worse), delta's checks. Recorded in DECOMPILATION_LEARNINGS as
one cheap axis to try early, not the answer. Likewise the 62-function census is
a SCREENING list, not a verdict: `func_80066340` sits on it and has
`Register Differences: 0`.

Axes that did close register-shaped residues, all ordinary C: retype a
parameter to its real width; whole-struct assignment instead of a field-copy
pair; **split one combined expression into two statements** (`func_8001EDAC`,
after seven failed manual attempts); `return dst;` on a wrongly-`void`
function. Confirmed INERT across 3 functions and 9 attempts: declaration and
introduction ORDER.

### Two tooling defects, both measured

**1. `permuter.py --debug` without `--stack-diffs` falsely scores ZERO on a
pure frame-size residue.** Charlie's `func_80065AE0` "zero" was really 28/40
with an 8-byte frame overshoot. The trap is sharpened by the display: without
the flag the output still prints `Stack Differences: 0`, which is a field that
was never filled in and reads exactly like a clean result. Broadcast mid-round;
delta re-measured `func_80063144`'s headline 465 with the flag and got an
identical 465 with `Stack Differences: 0` genuinely measured, so that finding
holds under the stricter test. Now in MATCHING-GUIDE.

**2. The documented oracle chain could not distinguish a compile failure from an
ordinary SHA1 mismatch.** `build-and-verify.sh` exits 1 only for a bad retail
dump; everything else propagates make's exit code, **2**. Measured in an idle
worktree:

| case | `build exit=` | old grep | new grep |
| --- | --- | --- | --- |
| genuine compile error | 2 | 2 hits | 1 hit |
| clean compile, SHA1 mismatch | 2 | **1 hit** | 0 hits |

The old pattern's `Error [0-9]` matched make's own summary line
(`make: *** [Makefile:74: check] Error 1`), which make prints when the SHA1
check fails — i.e. it fired on a perfectly good build, in the single most
common situation there is. `Error [0-9]` removed from the chain in CLAUDE.md,
MATCHING-GUIDE and the runner prompt; the compiler-only patterns separate the
cases cleanly.

### A permuter score drop is a LEAD, never a RESULT — 0 for 3 this round

Every permuter-local improvement found on a register-shaped residue was FALSE
against the real oracle: a cached-OT-pointer candidate scoring 240 (real 17/54,
funcdiff's drift warning firing); an algebraically-identical loop-end rewrite
scoring 60, found reproducibly twice (real 52/70 — one word WORSE, an in-range
regression the permuter's own diff could not see); and the `func_80065AE0`
false zero. Alpha oracle-verified and reverted both of its own leads rather
than banking them, which is what makes the negative trustworthy.

### Collision rule 2c has two halves and round 17 fixed only one

Every runner got `timeout` in its assignment and **every search terminated on
its own** — the termination half stayed solved. But four of five runners still
ended turns waiting on those bounded timers, about a dozen times between them,
and two reached ~20 minutes with **zero commits** — one holding an entire
seven-function characterisation that existed only in its context window. A
prohibition ("do not end your turn to wait") produced one more stop each; an
explicit numbered work order with hand work in it fixed it immediately and in
every case. Written up in PARALLEL-RUNS 2c, along with the one-search-at-a-time
rule (three runners ran 2-3 concurrent searches, peaking at load 62 on 32
cores, one running two searches on the SAME function) and the contention
caveat on negatives.

### Head errors worth recording

- **A pre-merge check that defeated itself.** The head's `git status`
  invocation printed a literal `(empty=clean)` regardless of the actual output,
  so a real modification in main was in the output and merged past. It was
  benign — a runner had written a title change to a main path instead of its
  worktree, with the authoritative version safe on its branch — but the check
  existed precisely to catch that. Print raw status with an end-marker.
- **Gate 1b scores must not be parsed from report PROSE.** The head's queue
  ranking pulled `func_8004042C`'s "25/25" out of a *correction preamble*
  describing the number being retracted, and conflated `func_8001EE98`'s 19/31
  with a 29/29 belonging to `func_8001E58C`, an already-matched **caller**. A
  `jal` encodes only a symbol address, so a caller's score can never validate a
  callee. Parse only a report's title/verdict line, and treat any figure near
  "correction"/"earlier version"/"superseded" as retracted.
- **A false contention alarm.** `grep -l 'code_8220.h' src/*.c` matched five
  units and briefly looked like cross-runner contention; all five were prose
  mentions inside comments. `headercontention.py` parses `#include` properly
  and was right. Wrong instrument, not a tool bug.

### Open, for the operator

- **`make extract` is BLOCKED in the main checkout** by
  `.claude/hooks/block-raw-make.py`, though the hook's own message lists that
  target as allowed, and it accepted the identical command in all five
  worktrees. Three invocation forms were tried and none worked; the hook was
  not worked around. Impact was nil this round (the stale files are exactly the
  functions matched, `progress.py` states outright that counts are unaffected,
  build green) but it will bite whenever main genuinely needs a re-extract.

  **RESOLVED IN ROUND 19 — it is REDIRECTION, and the hook is not
  worktree-sensitive.** `SEPARATORS` in `block-raw-make.py` omits redirection
  operators, so `>` and the log path stay inside make's span and are judged as
  *targets*. Bare and piped forms are allowed; `> log` and `2>&1 | tail` are
  blocked. Round 18 happened to use redirected forms in main and bare ones in
  the worktrees, which made it look per-checkout. Round 19 re-extracted main
  successfully with the piped form. Still a real defect (and the hook also
  blocks writing prose about itself via a heredoc), still the operator's call
  to fix — see round 19's entry.
- **The `permuter exit=$?` line is lost intermittently** — three runners
  independently reported the trailing echo never landing, from a race between
  the outer tool timeout and the inner `timeout` plus multiprocessing shutdown.
  That defeats the 124-vs-137 mechanism exactly when it is needed. Redirect the
  exit code to its own file rather than appending to the search log.
- **An agent thread's cwd resets between Bash calls**, so a runner command
  without an explicit `cd` lands in the MAIN checkout. Delta hit it twice,
  caught both itself. This is the mechanism behind rule 5 violations that look
  like a second head.
- `origin/main` was pushed again by something that is not this head (round 17
  flagged the same). No foreign commits, branches or worktrees exist — every
  commit is this session's. **Not pushed by this head; the operator's call.**

**Next move.** Runners again, on the register-shaped corpus. This round showed
the class is recoverable by ordinary C at a meaningful rate, the queue is 62
functions with reports already written, and the cheapest entry points are
named: `func_8003FCFC` (290 -> 135, strongest re-attempt), `func_8004042C` (one
word), `func_8004C93C` (never searched; its `--debug` shows 11 insertions/11
deletions, not the clean register-only signature its classification implies),
and `func_80063144`'s open divergence #2. Carving is NOT the next move —
`fresh` is 1 and 187 uncarved functions remain, but the near-miss corpus is
better posed than cold ground.

---

## 2026-09-05 — round 17: 5 runners, 115 matches, and three "unconditional" claims that were not

**788 -> 921 matched (58.11% -> 67.92% of game code; matched bytes 42.70% ->
47.92%). Build green in main after every one of the twelve merges, and every
claimed match re-verified individually AFTER its merge.** 115 of the 133 new
matches came from runner work; the other 18 are `jr $ra; nop` bodies splat
generated itself in the eight fresh carves.

**The round ended on an infrastructure failure, not a stop rule** — four of
five runners were killed mid-work by the weekly API limit. Their committed
work was merged and two in-flight bodies were salvaged under §4c. **Five
worktrees are still standing, deliberately; see "Teardown deferred" below.**

| runner | units | p1 | p2 | p3 | stalls | end state |
| --- | --- | --- | --- | --- | --- | --- |
| alpha | `class_3bb8c_p`, then `class_3bb8c_t` | 17 | 10 | — | 1 + 1 salvaged | died to API limit |
| bravo | `class_3bb8c_o`, `class_3bb8c_r`, then a permuter pass | 18 | 20 | 2 | 0 | died to API limit |
| charlie | `code_179d8_f`, then a permuter pass | 15 | 1 | 4 | 1 | died to API limit |
| delta | `code_179d8_g` | 9 | 0 | — | 4 + 1 salvaged | died to API limit |
| echo | `code_179d8_e`, then `code_179d8_h` | 12 | 8 | — | 6 | reported cleanly |

Bravo was the round's strongest at 40 matches across three assignments with
zero stalls, and never once touched a shared header.

### Gate 1 was dry, so the round was mostly carving

`fresh` was **0** at the start — all 222 queued functions already had reports.
Eight units were carved in three waves (five up front, three mid-round to
re-staff runners whose units were exhausted): `code_179d8_e/_f/_g/_h`,
`class_3bb8c_o/_p/_r/_t`. Every slice was chosen by MEASURED blocker density
per 20-function window, not by "next", and every one verified green with zero
bytes changed. None contained a `jtbl_` reference, so none needed a rodata
attach — the `code_179d8_tail` cut was deliberately placed BEHIND both of that
segment's jump-table owners to leave that debt in the remainder.

Gate 0 earned its place again: a stale `asm/class_3bb8c_j.s` monolith plus 37
stale nonmatchings made `progress.py` report **293** uncarved game functions
where the truth after re-extracting was **346**.

**Header contention was zero by construction.** Carving across two blocks put
three runners on units with no project header at all;
`headercontention.py` on the five-unit plan printed "NO CONTENTION". Of the
twelve merges, **none conflicted**. Alpha did edit `include/DreamSys.h` (shared
with `DreamSys.c`'s 89 matched functions) and every one of its six edits was
verified size-preserving BY ARITHMETIC before merging, not by trusting the
build: pad 12 -> 4+4+4, 4 -> 2+2, 8 -> 4+4, vtable 24 -> 4+20, 32 -> 8x4, 4 -> 4,
12 -> 4+8.

### The round's real finding: three "unconditional" compiler claims, all wrong

Three separate runners filed a stall as an unconditional compiler behaviour,
two of them proposing a new toolchain blocker. **All three were tested along
exactly one axis, and all three broke on the second axis the head tried.**

1. **echo, `func_8002CA3C`** — "GCC unconditionally folds `x - (x>>k)*N` into
   `x & (N-1)` once it can prove `x >= 0`", backed by seven probes. All seven
   varied `/` vs `>>`, guard vs no guard, barrier vs none, while holding the
   intermediate's TYPE fixed. Narrowing the multiplicand
   (`lo = index - (short)hi * 16`) blocks the fold and reproduces retail's
   whole sequence bar one operand. Moved from "5/55, possible blocker" to
   "shape reproduced, one operand off".
2. **echo, `func_80028C54`** — the branchless `sltiu` fold, flagged as possibly
   the same class as the above. It is not, and it is not unconditional: it is
   boolean materialization, and it breaks on either a side effect in an arm or
   return values that are not a bare 0/1 pair. Retail has a branch there, so
   retail's source is doing something else on one path.
3. **charlie, `func_80035F3C`** — "CSEs two `x & IMM` sharing the same-valued
   IMMEDIATE". Measured: the merge is keyed on the VALUE. Two `x & 0xFF` on the
   same `x` give one `andi`; on different operands they give two. Ordinary CSE.
   The fix was real; the mechanism was not.

**The generalisation, now in DECOMPILATION_LEARNINGS: retail came out of THIS
compiler, so "no C reaches these bytes" is a claim about the entire shape
space, not about the axis you happened to vary.** It earns the same standard
CLAUDE.md already sets for the no-C-form exception. This matters because a
blocker filed on one axis becomes a stub, and a stub removes the function from
`fresh` permanently — round 16's lesson, arriving from a new direction.

Four proposed learnings were checked against reproducers rather than
transcribed. Two were promoted (delta's per-branch-constant-store merge, which
reproduces cleanly; charlie's CSE finding with its mechanism corrected), and
two were recorded as NOT promoted (the fold above, and alpha's claim that a
`for`-header increment differs from the same statement in the body — three
spellings emit byte-identical code).

### Gate 3 works, and the near-miss corpus is a better queue than cold ground

With the good windows carved out, both idle runners went onto targeted
permuter passes over documented near-misses instead of a thin carve. **Six
near-misses closed** — `func_8003CCDC` 27/27, `func_8002C048` 25/25,
`func_8003FDB0` 31/31, `func_800404D0` 31/31, `func_8004D47C` 33/33,
`func_8002C0AC` 32/32 — across five units nobody was holding.

That queue is now documented as **Gate 1b**: 72 non-blocker scored near-misses,
only 4 already permuter-exhausted. It must be screened from the ASM, not the
report prose — the head's first list included `func_8003C63C` (15/16) because
its report says "toolchain blocked" without naming the class, and it is
addiu-$at.

### Three new rules in PARALLEL-RUNS, each paid for this round

- **The blocker screen is BLIND to BIOS trampolines, and fails flatteringly.**
  `class_3bb8c_h` screens 15 of 17 clean — the best figure of any remaining
  segment — and is 13 BIOS trampolines plus two addiu-$at functions. **Two
  workable functions out of seventeen.** The trampoline grep is the fourth
  screen when RANKING segments, not just a carve-time chore.
- **Collision rule 2c: the permuter WAIT-LOOP.** A runner ended three
  consecutive turns on "waiting for the permuter", indistinguishable from one
  still working. `--stop-on-zero` means a search that never reaches zero never
  terminates. Put the bound in the ASSIGNMENT (`timeout`, and its 124-vs-137
  exit codes) — a prohibition is weaker than a bound, and "do not start
  anything new" was sent and a fourth search started anyway. Two of its runs
  were still alive, 34 and 20 minutes in, holding twelve cores, when it
  reported them killed; the head verified by `/proc/<pid>/cwd` and killed by
  PID.
- **Gate 1b**, above.

### Teardown deferred — five worktrees are still standing

`alpha`, `bravo`, `charlie`, `delta`, `echo` at
`../lsddecomp2-wt-<name>`. All five branches are fully merged
(`main..runner/<name>` is empty for every one) and both remaining dirty files
have been salvaged into reports, so nothing is at risk. They were left up for
one reason:

**`origin/main` was pushed to `adb9e92` at 2026-09-04 19:05 by something that
was not this head.** No foreign commit, worktree or branch exists — every
commit at and below that point is this session's own work, and the runner
branches are exactly where they were left. So this is most likely the operator
or an automation pushing this session's commits, not a second head. But it is
unexplained, and the two irreversible actions available here are `git push` and
`git worktree remove --force`. **Neither was taken.** `main` is 17 commits
ahead of `origin/main` and NOT pushed; the operator's call.

### §4c salvage

- `func_80058228` (`class_3bb8c_t`) — **52/56**, scored by the documented
  method (copy into main, full oracle, funcdiff, restore, re-verify green).
  Length matches so the in-range read is trustworthy. Residue is two SWAPPED
  store offsets on a colour struct: a field-order question, not codegen.
- `func_8002AA6C` (`code_179d8_g`) — **deliberately unscored.** funcdiff
  refused the number (296081 bytes differ outside the range), which is
  CLAUDE.md's third way a score lies caught by its own guard. Recording an
  invented figure would wrongly settle whether the body is worth resuming.

Both are labelled MID-ATTEMPT SNAPSHOT: no author applied a stop rule to
either. Worth recording against the runners' own last words — alpha's was
"Matched immediately. Let's write the report and commit", which referred to a
function it had ALREADY committed; the uncommitted body is a different,
non-matching one. **A dying runner's last line is evidence about what it was
thinking, never about what is in the tree.**

### Next move

**Permuter round, not runners and not a carve.** The near-miss corpus (72
scored, 4 exhausted) is now larger and better-posed than the cold ground left:
~45 truly workable uncarved game functions, scattered across segments that are
20-33% clean, with `class_3bb8c_h` a trampoline trap. Six near-misses fell to
the permuter this round in a few hours of two runners' time.

Two things to settle when convenient, neither urgent: the `class_3bb8c_h`
trampoline block needs an `hasm`-vs-`.word` disposition decided, and the
724-function Psy-Q library is untouched and excluded from the game
denominator.

---

## 2026-09-04 — round 16: 5 runners, 51 matches, and two corrections to inherited screens

**732 -> 788 matched (53.98% -> 58.11% of game code; matched bytes 40.20% ->
42.70%). Build green in main after every one of the ten merges, and every
claimed match re-verified individually AFTER its merge.** 51 of the 56 new
matches came from runner work; the other 5 are empty `jr $ra; nop` bodies
splat generated itself in a fresh carve, and are counted here only because
`progress.py` counts them.

**The fresh queue ended at 0 — every one of the 222 queued functions now has
a report file.**

| runner | unit(s) | p1 | p2 | p3 | stalls | end state |
| --- | --- | --- | --- | --- | --- | --- |
| alpha | `class_3bb8c_i`, then `code_2cc8c_f` + `class_3ac78` | 6 | 2 | — | 0 | all exhausted |
| bravo | `class_3bb8c_k`, then `code_179d8_d` | 6 | 9 | 1 | 3 | both exhausted |
| charlie | `code_179d8_b` | 10 | 4 | — | 3 | exhausted |
| delta | `code_179d8_c` | 8 | 3 | — | 5 | exhausted |
| echo | `class_3bb8c_l` | 2 | — | — | 1 | exhausted |

Every unit staffed this round is now fully worked. Four of five runners were
re-sent at least once (§3c); bravo went three passes across two units and was
the round's most productive at 16.

### Gate 2: carve across blocks, and the contention result that justifies it

Gate 1 found only 17 fresh functions, 15 of them in one class block. Rather
than staff five contending runners as round 15 did, the head carved two units
out of `code_179d8` (`_b` = window [60..79], `_c` = [200..219]) chosen by
measured blocker density rather than by "next", and later a third (`_d` =
[100..119]) mid-round to re-staff bravo. Both `_b` and `_c` owned exactly one
jump table in the shared `0xFD8` rodata slot, so it was split at two ownership
boundaries rather than attached whole; `_d` owned none.

**Result: 0 of 10 merges conflicted on a header, with three runners editing
`class_3bb8c.h`** — against 6 of 7 conflicting in round 15. PARALLEL-RUNS'
contention table now carries the mechanism: it is not the number of runners
sharing a header, it is what they put in it. Every edit was an additive pad
split with its total preserved (the head verified each by arithmetic, not by
trusting the build); no cross-unit prototype went into the shared header, and
one runner matched `class_39e08.h`'s pre-existing canonical declaration for
`func_80052B70` instead of writing its own — the exact failure that killed
round 15. Two runners independently reached class `D_80087034` and did not
collide, one keeping its view local by choice.

The two conflicts that DID happen were both the head's own doing, and are a
new class: `modify/delete` on a match-report file, because the head deleted a
stub report while a live runner was turning that same file into a real one.

### Correction 1: the head inverted the third blocker screen — again

The head re-implemented the `nop_mflo_mfhi` grep in Python for a carve-time
window census and wrote it **backwards** (looking for `mult`/`div` *before*
the `mflo`/`mfhi`), which is precisely the error DECOMPILATION_LEARNINGS
records round 15's head making — committed by a head that had read that entry.
The Gate 1 screen over the assigned queue used the correct shell form, so no
runner was handed a blocked function, but the carve census was wrong and the
head wrote **two stub reports for functions that were never blocked.**

Measured cost over the 239 functions of the old monolith still visible as
`.s`: 4 correct forward hits versus 14 inverted, of which 4 are pure false
blockers. `func_800292F4` — one of the two the head stubbed — was **matched at
65/65** by runner charlie within the hour of the correction. The other,
`func_8002C278`, was attempted and stalled at 11/76 on register identity, so
it too belonged in a runner's queue rather than in a stub.

**Why this is the round's most important entry: a false blocker is strictly
worse than a missed one.** A missed blocker costs a runner some attempts and
still produces a report. A false blocker becomes a stub, `progress.py` counts
a stub as a documented stall, and the function leaves `fresh` **permanently** —
nobody re-measures what everyone believes is blocked. PARALLEL-RUNS now says
run the shell form and do not re-express it.

Both stubs were deleted (returning the functions to `fresh`), and the header
comments in the two affected units were fixed **by the runners holding those
files**, not by the head, per collision rule 6's one-channel rule.

### Correction 2: the 7+ callee-saved-register band is no longer 0-matched

`func_8004A534` matched **163/163 with 8 distinct callee-saved registers**
(`$s0`-`$s7`). The head sent alpha into it *expecting a stall* on the strength
of the documented 0-matched/4-stalled figure, told it a measured stall report
was the deliverable, and then verified both the register census (8) and the
match. Corroborated the same round by delta, which attempted all three of its
large bodies and reported that **register count was not the blocker in any of
them** — two needed 0 and 1 s-registers anyway — and by charlie, which said
the same of its three stalls when asked directly.

This is the **second** correction to this threshold (round 14 moved it 5 -> 7),
and round 14's own lesson explains why: a threshold needs samples on both
sides before it is a threshold. The 7+ band had four stalls and no
attempted-and-matched samples, so it could not distinguish "7 is fatal" from
"nobody has tried". One deliberate attempt settled it. Read the count as a
**correlate** of "large function needing deep struct reconstruction", which is
what actually costs — `func_8004A534`'s load-bearing insight was a struct-shape
one. Still a real deprioritisation signal at 1-of-5; never grounds to skip.

### The method lesson: a long attempt list is not a broad one

Two stalls were argued with impressive attempt counts that all varied ONE
axis. `func_80032BB8`: seven reshapes, every one varying the *expression* form
(array index vs pointer arithmetic, temp vs no temp, declaration split,
parameter type), none changing the control flow. `func_8002C048`: twenty-five
variations, all keeping the cached `c1`/`c2` locals, none testing whether
those locals should exist — which is exactly what the already-documented
no-cache idiom points at. Both runners were sent back on the untested axis,
and the results went in opposite directions:

- `func_8002C048` **improved as a stall**: the no-cache idiom produced a body
  with **no `volatile` anywhere** and removed the spurious `andi`, landing at
  23/25 versus 24/25. A *lower* score and a better artifact — the residue is
  now a genuine instruction-order swap rather than an artifact of a codegen
  lever, and a preserved body needing `volatile` would have put source in the
  tree that misrepresents the game. Bravo also bounded it: blanket-removing
  *all* cached locals regressed to 13/25, because one cache is a genuine
  cross-iteration carry. The idiom is a scalpel, not a blanket rule.
- The head's own guard-polarity hypothesis was **wrong**, and delta proved it
  by request: flipping both stalls to positive-first regressed both. That
  checked negative is what scopes the block-order lever to raw values reaching
  `beqz`/`bnez` directly and stops it being over-generalised to guard-clause
  range checks. Asking for the negative is cheap and it is what turns a lever
  into a scoped lever.

### New residue classes

- **Commutative-operand SLOT order in `addu`, not reachable from C.** In
  `func_800323A8` all sixteen field accesses differ only in which operand slot
  (`rs` vs `rt`) a commutative add uses — same registers, same values, so not
  the register-identity class. Rewriting all 15 source sites with the operands
  swapped produced a **byte-identical build**: cc1 canonicalises
  pointer+integer addition regardless of source order. Not a toolchain
  escalation; the 15-site swap *is* the isolation experiment.
- **The retry-loop driver cluster** — `func_80028DF0`, `func_80028F38`,
  `func_80029074`, three instances of one shape, fully reverse-engineered,
  stalled on a status-value fold and a prologue register-mapping puzzle after
  ~90 manual attempts and ~70k permuter iterations (best 55/85 with an exact
  length match). Charlie built a pinned-pipeline reproducer of the
  status-value shape and it did **not** reproduce retail — the more useful
  result, ruling out a toolchain defect and saying something contextual is
  still missing from the C.

### A score that lied, caught by splicing rather than by reading

Echo filed `func_80053ACC` as "STALLED at 28/71". `funcdiff` had printed that
number **together with its own drift warning** (159121 bytes differing outside
the range, because the body is one word shorter than retail) — way-a-score-lies
#3 firing exactly as designed. Echo's analysis body had hand-corrected to
~70/71, so its reasoning was sound; the **title** carried the untrustworthy
number, and a title is what the next round triages from.

The head spliced the preserved body back into `main`, built, and read it with
`asm-differ` instead: the residue is two differing instructions and one missing
word, and they are **one** decision — retail copies the masked switch value
into `$a0` in the `bnez` delay slot and then compares against that copy, where
the build sinks `li v0,0x2` into the slot and keeps comparing `$v1`. Retail's
`move a0,v1` is even dead on the not-taken path. That reclassifies it from
"close to unmatchable" to a **permuter candidate**. Report corrected.

**Never headline an in-range score that came with a drift warning.**

### Two patterns that did NOT earn promotion

Both runners were asked for a negative check and both answered honestly that
they had none — the only reason these are recorded as open rather than
promoted. The "untouched register means a vtable slot's arity is short"
pattern has two positive sightings and zero negative checks; the "`$a1`
`lui`-set before its only read means scratch" pattern has one sighting,
re-checked in a second pass with no second instance. A near-cousin found the
same round sharpens the first: an untouched-looking `$a0` that was a fresh
`move a0, zero` for a literal argument. So the real tell is unexplained
register *activity* near a call, not "forwarded argument". Both need a sweep.

### Bookkeeping

- **Self-reported counts were wrong twice, in both directions**, both caught
  by counting commits rather than reading summaries (§3). Delta reported "14
  total for this unit" against an actual 11; charlie's first-pass summary
  listed functions it had not reached. All the *work* was sound in both cases
  — every claimed match verified byte-exact. Nothing self-reported is
  load-bearing; the branch is.
- `progress.py`'s stale-asm warning fires benignly in this workflow: a matched
  function's `.s` lingers until the next re-extract, and splat emits both an
  empty `void f(void){}` body *and* a `.s` for trivial `jr $ra; nop`
  functions, so those five in `code_179d8_d` are stale by construction and
  re-extracting cannot clear them. The warning's suggested fix does not apply
  to that case.
- The head blocked its own merge once (`git merge` exit 2, refusing to start)
  by having uncommitted consolidation work touching the same report files.
  Check `git status --porcelain` in `main` before each merge, not only in the
  worktrees.

### Next round

**Runners on a fresh carve, plus a permuter pass.** The fresh queue is 0, so
Gate 2 fires immediately: 346 uncarved game functions remain, in `code_179d8`
(head), `code_179d8_mid`, `code_179d8_mid_b`, `code_179d8_tail`,
`class_3bb8c_n` (113) and `class_3bb8c_h` (the BIOS trampoline block, which
needs an `hasm`/`.word` disposition decided at carve time, not by a runner).
Carve across two different blocks to keep header contention at zero, and take
the census with the shell screen.

There is also now a real permuter queue of honest one- and two-word
near-misses: `func_80053ACC` (2 instructions + 1 word), `func_8002C0AC`
(30/32), `func_8002C048` (23/25), `func_80029074` (55/85, exact length).

---

## 2026-09-04 — round 15: 5 runners, 67 matches, 2 stalls, and one rule found four times

**665 -> 732 matched (49.04% -> 53.98% of game code; matched bytes 36.94% ->
40.20%). Build green in main after every one of the seven merges, and every
claimed match re-verified individually AFTER its merge.** All 67 came from
runners; two functions stalled, both with reports.

| runner | unit | pass 1 | pass 2 | stalls | end state |
| --- | --- | --- | --- | --- | --- |
| alpha | `class_3bb8c_i` | 12 | — | 0 | 6 fresh left |
| bravo | `class_3bb8c_j` | 12 | 4 | 2 | **fully worked** |
| charlie | `class_3bb8c_k` | 12 | — | 0 | 6 fresh left |
| delta | `class_3bb8c_l` | 12 | — | 0 | 3 fresh left |
| echo | `class_3bb8c_m` | 12 | 3 | 0 | **fully worked** |

Every runner matched everything it attempted on the first pass — 60 for 60.
Two of the three early finishers were sent back into their own units (§3c) and
produced 7 more matches plus both stalls. Alpha, charlie and delta were not
re-sent because the head's merge queue had become the bottleneck, which is the
honest reason and a sizing lesson: **head attention, not runner capacity, is
what capped this round.**

**Gate 0 fired again, and it is now the second round running where a `pull`
was the cause.** `progress.py` named five stale monoliths; cleaning them and
re-extracting moved uncarved game code from **276 to 486**. The build was
green throughout — this round's symptom was purely the silent under-report,
not round 13's red build, which means the loud half of the failure is not
reliable and the warning at the top of the output is the only tell.

**Gate 2: carved four units** (`class_3bb8c_j`/`_k`/`_l`/`_m`, 80 functions)
out of the 193-function remainder, one segment at a time, verified after each.
The `0x1EF4` rodata slot had **three** owners and split twice; it came up green
first try because the ownership greps were run before the attach rather than
after a red link. Head filed stub reports for the 11 screened-blocked
functions, which is what makes `fresh` honest — 86, not 97.

### The round's main finding: shared-header prototypes, hit four times

All five runners worked adjacent slices of ONE class block, so all five edited
`include/class_3bb8c.h` and **six of seven merges conflicted.** Every conflict
was complementary (runners naming different slots of the same structs from
different call sites) and cheap to union. The expensive part was four
collisions git cannot see, all the same shape — a cross-unit prototype in a
unit-local type placed in the shared header:

1. `func_80051A4C` — alpha's header prototype vs bravo's *definition*. Hard
   compile error.
2. `func_80053EB4` — delta declared it "still INCLUDE_ASM" while echo matched
   it the same round.
3. `func_8004A4B8` — **the one that matters.** delta's header prototype
   collided with the project's pre-existing canonical declaration in
   `class_39e08.h`. Invisible when delta's own work verified, because
   `class_3bb8c_l` does not include that header; it surfaced **two merges
   later** in `class_3bb8c_k`, the first unit to include both. So the runner
   who writes this sees green and the unit that breaks is one that never
   touched it.
4. `func_80040FC0` — alpha's header prototype vs bravo's local one, where
   *nobody* has the definition (still `INCLUDE_ASM` in `code_2cc8c_f`), so
   both are call-site typings and both units are byte-exact with their own.

Rule now in PARALLEL-RUNS §1, with the head's post-merge grep. After the
fourth, the head swept all **41** prototypes in that header against every
including unit and every other project header rather than waiting for a fifth.

**Flagged for the operator, not acted on:** that sweep shows
`include/class_3bb8c.h` has accumulated prototypes that shadow canonical
declarations in other headers with different types. Four remain as
warning-level pointer-vs-pointer disagreements — the same 4 warnings present
*before* this round, and they are the sanctioned
multiple-independent-local-views convention rather than new damage. Left
alone deliberately: byte-exact and green, and retyping to tidy them would
reach hundreds of matched functions for no byte gain. It is real debt and it
is only the accident of which unit includes which header that keeps most of it
warnings instead of errors.

### Two runner claims corrected from the binary

- **`D_80086ED0`'s instance size.** bravo reported 0x54, alpha 0x4C.
  `func_80050BA8` does `li a0,0x4c` and ctors through `func_80051A4C()`, which
  returns `&D_80086ED0` — alpha is right. charlie's match of `func_80052B60`
  then showed bravo's 0x54 object belongs to `D_80086F88`, a class bravo had
  not yet identified. So the type named `Class86ED0` in `class_3bb8c_j.c` is
  **misnamed**. No bytes affected; bravo was messaged rather than its live file
  edited (§6).
- **`ObjM` == `Obj87034_3bb8c_l`.** Proven by a cross-unit call, not a
  resemblance: `func_80053EB4` is defined in `_m` taking `ObjM *` and called
  from `_l` passing `Obj87034_3bb8c_l *`. Deliberately **not** unified —
  merging two field maps is a struct edit reaching 24 matched functions, which
  is the round-13 hazard and not something to do inside a merge resolution.
  Recorded as a HEAD NOTE for a dedicated change.

### Head corrections of its own work

- **The `nop_mflo_mfhi` screen.** Auditing the stall corpus for round 13's
  wrong-CAUSE trap, the head re-implemented Gate 1's third grep in Python and
  **inverted it** — `grep -A2 mflo` reads the two lines *after*, not before.
  That produced four false blockers, one on a 252/258 near-miss. Both
  directions were then reproduced in isolation: `mult -> mfhi` is CLEAN (the
  divide-by-constant idiom comes out exactly as retail has it), `mflo/mfhi ->
  mult/div` is BLOCKED (two nops inserted). Re-run correctly over all 285
  queued functions: **7 hits, all 7 already reported as blocked.** The corpus
  had no mis-filed cause, and 7-of-285 keeps that grep with the head.
- **`func_80053F84`'s "redundant" guard.** Its
  `x != 5 && x != 8 && x == 0xA` has two provably-dead conjuncts. The head
  hypothesised a sparse `switch` and **measured it: 109627 bytes off.** Echo's
  form is correct; the note now in that report exists so nobody "simplifies"
  it.
- **The remaining-work grep was unanchored** and counted `INCLUDE_ASM` inside
  a runner's own comment, reading bravo's 8 remaining as 9 — impugning an
  accurate summary, the inverse of what the check is for. Now `^INCLUDE_ASM`.

### Other structural findings

- **Three of five units spanned two or more vtables**, and all three runners
  filed it as an *anomaly* because the carve comment implied one class per
  unit. A slice cut at ROM-address boundaries has no reason to align with
  class boundaries. Gate 2 corrected.
- **Runner commit granularity slipped once, disclosed.** Alpha could not
  produce per-function source commits (its 12 were derived together and a
  replay was blocked by a permission classifier), so it landed one combined
  source commit plus 12 report commits and said so plainly. Echo's first pass
  assembled its commits index-only via `git apply --cached`; the branch tip is
  what was verified, and its intermediate commits are not independently
  buildable. Both were acceptable and both were disclosed, which is the part
  that mattered.

### Next round — carve FIRST, into `code_179d8`, and spread

Round 15's own lesson was "spread runners across unrelated blocks". That is
now a Gate 1 command (`tools/headercontention.py`, added after the round) and
the groundwork below is measured so the next head does not re-derive it.

**Spreading is NOT available from existing fresh ground — measured.** The 17
fresh functions live in `class_3bb8c_i` (6), `class_3bb8c_k` (6),
`class_3bb8c_l` (3), all of which include `class_3bb8c.h` and therefore all
contend; the only header-independent units left have 1 fresh each
(`class_3ac78`, `code_2cc8c_f`). `headercontention.py` on those three returns a
mutually-independent subset of **1 of 3**. So a spread round requires a carve
into a different block, and **Gate 2 should fire before provisioning, not
after**.

**Carve `code_179d8`.** It is the only large uncarved block that is not
`class_3bb8c`, it has no existing project header, so a unit carved from it
contends with nothing. Two cautions, both measured:

- **Do not carve its first 20 functions** — that window is 6/20 clean.
  Blockers cluster: `[100..119]` is **20/20 clean**, `[60..79]` 16/20,
  `[40..59]` and `[80..99]` 15/20. The splat yaml's "43% blocked" note on this
  segment is true in aggregate and a bad carve guide; see Gate 2's window
  table.
- **Its rodata slot `0xFD8` will need the attach/split dance**, and this one is
  known to genuinely hold text pointers — 179 of its 483 words are vram
  addresses inside `code_179d8`'s own text, the rest ASCII. Because those
  pointers span the whole segment, expect to split the slot per carved slice
  rather than attach it whole. Gate 2 documents the three-step version.

`class_3bb8c_n` (113 functions, the round-15 remainder) also has clean
windows — `[60..79]` 19/20, `[80..99]` 18/20, and its first 20 are 1/20, so
the same do-not-carve-the-front caution applies. But it stays inside the
contended block, so prefer it as the *second* carve, or for a deliberately
concentrated smaller round.

**Sizing.** Three contending runners plus re-sends likely beats five
contending runners without them: round 15 left alpha, charlie and delta
un-resent with fresh ground still in their units purely because the head's
merge queue saturated. If the round does end up concentrated, make it smaller.

`class_3bb8c_h`'s 13 `jr $t2` BIOS trampolines remain the one carve needing an
operator decision (`hasm` segment vs literal `.word`), unchanged.

---

## 2026-09-03 — round 13: 5 runners across 11 unit-passes, 110 matches, and four triage corrections

**454 -> 568 matched (33.48% -> 41.89% of game code; matched bytes 26.30% ->
32.55%). Build green in main after every one of the eleven merges, and every
claimed match re-verified individually AFTER its merge.** 110 of the +114 came
from runners and the head; the other 4 are `jr $ra; nop` bodies splat
generated itself in a newly carved unit.

Five runners, but **eleven unit-passes** — every runner that finished was sent
back into new ground rather than idled, three of them twice. That was the
round's biggest structural lever and it is worth repeating.

**Gate 0 did not exist and the round found out the hard way.** `main` did not
build. The round-12 pull had brought in four carves and nobody re-extracted
`asm/`, so `build-and-verify.sh` died on `can't open
asm/nonmatchings/<unit>/....s` — unattributable to any commit. Worse,
`progress.py` silently UNDER-REPORTED: it printed a stale-asm warning above
the table and then a table computed as if the un-extracted units did not
exist. **581 uncarved game functions before re-extracting, 742 after.** Every
triage decision reads off that table, and the warning that would have caught
it prints where a `tail` pipe drops it. Now Gate 0 in PARALLEL-RUNS.md.

**Gates.** 4a clean throughout, re-checked before each teardown: no other
head, no foreign commits, no pushes to `origin/main` that were not ours. Gate 1
found 49 true fresh functions across 8 units, all clear of the two blockers
CLAUDE.md names — which turned out to be the wrong screen (below). Gate 2
fired four times DURING the round, not just at the start, because runners kept
emptying their units: `code_8220_c` (16), `code_d294_c` (15),
`Entity_f`/`Entity_g` (37, split), `code_2cc8c_d` (33). Every carve verified
green on its own before the next, and all four changed zero bytes.

### There is a THIRD blocker screen and the head was not running it

Runner bravo stalled `func_8001CEB4` on `nop_mflo_mfhi` — the pinned pipeline
inserting `nop`s between an `mfhi` and a following `mult` that retail does not
have. The screen for it lives inside `docs/research/addiu-at-blocker.md`, not
in CLAUDE.md's two-grep list, so Gate 1 had not applied it and a blocked
function was assigned. Re-measured as that document asks:

| bucket | round 11 | round 13 |
| --- | --- | --- |
| matched C | 0 | **0** |
| queued `INCLUDE_ASM` | 2 | **4** |
| uncarved | 7 | 7 |

Zero of 568 matched functions contain the construct, which is what a real
blocker looks like. **The per-runner standing screen stays rejected** at 4 of
156 — that was measured and declined twice already. But round 11's cost model
was wrong in one direction: it assumed the 30-attempt cap bounds walking into
this cheaply, and bravo instead spent a FULL DERIVATION (field reads, dispatch
structure, a `/360` division verified against the pinned `cc1`) before the
construct surfaced. A blocker that only appears once you understand the
function is not bounded by an attempt counter. **So the fix was granularity,
not scope** — the same correction that document already makes about the flag
itself — and the screen now runs once per round in the head's Gate 1. It
immediately caught two more functions pre-assignment in later carves.

### Four triage corrections, which is where the head's time went

- **`func_8005CBC8`: near-miss -> BLOCKED.** Filed as "1 word short,
  instruction-selection preference" after seven attempts. It is two words
  short and one of them is `addiu $at,$at,%lo(jtbl_*)` — the known
  `addiu-at` blocker, whose hit sat unexamined at line 66 of the function's
  own `.s`. Unmatchable as C. The other half of its residue WAS reachable and
  is now fixed in the preserved body: retail's `nor`+`addiu` is `~sel + 1`,
  not `-sel`. **The blocker screen now gates a stall's CAUSE, not just
  assignment** — a wrong score self-corrects when someone re-measures, a
  wrong cause is what the next round acts on.
- **`func_8004BB3C`: 14/105 -> 90/105 at correct length**, by finishing the
  lever its own report left unpulled and had recorded a WRONG reason for
  (struct-size corruption, which dissolves once the sub-type is never
  embedded). Retail's two array walkers have different BASES, so N
  independently-incrementing walkers need N differently-based stride-sized
  view types; no field regrouping splits them.
- **An 8-function stall class overturned.** Charlie derived
  "register-identity, not reachable from C" from 6 attempts on one root and
  applied it to 7 siblings. The head reached instruction-exact (zero
  inserted, zero deleted) on the root: the OT splice is a **24-bit BITFIELD**
  write (Psy-Q `P_TAG`'s `addr:24`), not hand-written masking — same value,
  different register allocation — and the OT expression must be
  **re-evaluated, not cached**, because `addPrim(ot, p)` expands its argument
  twice. Handed back; charlie applied it to all 8 and every one is now
  instruction-exact. The transferable part is why it read as exhausted: all
  six attempts varied HOW the masking was expressed while holding fixed the
  assumption that masking was in the source at all. **Six attempts along one
  axis reads exactly like a search of the whole space.**
- **A whole-function `__asm__` reworked into six lines of C.**
  `func_8001A3EC` was matched by transcribing 53 words of assembly, on the
  reasoning that a straight-line frameless body justified it, citing a real
  GTE function as precedent. It is reachable with an idiom already documented
  and already confirmed three times (all-`s16` struct -> alignment 2 ->
  `lwl`/`lwr`). CLAUDE.md's inline-asm exception is now scoped to "no C form
  EXISTS", and writing one requires naming the instruction that has no C
  spelling — otherwise `INCLUDE_ASM` already does the same job more honestly.

### One runner classification REJECTED

Delta proposed its forgotten-padding bug as a fifth "way a score lies". It is
not one: the oracle went RED and was correct to. Filing it as a fifth would
teach the next runner that the oracle cannot be trusted in exactly the case
where it can. Delta's own write-up reaches the same conclusion in its last
sentence. Both halves of what it DID find were kept — the shared-struct
hazard is broader than the "slot retype" wording, and its `cmp -l` ->
`lsdde.map` localization recipe, after fixing an off-by-one (`cmp -l` is
1-based, verified on a probe). Delta then used the corrected recipe to
self-catch two MORE instances of the same class, so it is now four across two
rounds and recorded as standing.

### HARD RULE 6 was wrong about this repository

It banned extended-asm operand constraints "categorically, not judgement
calls". `src/code_8220_b.c` has carried eight byte-verified `"r"`-constraint
blocks since before the rule was written, for COP2 `swc2` stores that have no
C spelling at all. A runner obeying the literal rule would either refuse a
legitimate match or believe it had violated a hard rule by achieving one —
charlie hit exactly that and flagged it. Now scoped to register identity, with
the GTE exception and its two measured traps (a wrong GPR clobber on an
`lwc2`/`swc2` block cascades through the whole function's allocation; an
unbracketed branch mnemonic loses its delay slot).

### TOOLCHAIN LEAD, escalated and NOT acted on

`docs/research/maspsx-noreorder-lead.md`. maspsx inserts a defensive `nop`
after real branch mnemonics inside `__asm__` blocks, displacing the intended
delay-slot instruction — a control-flow semantic change. It tracks reorder
state in a flat `is_reorder` flag matched against tab-delimited `.set` lines,
and its own `.ent` handling EMITS `.set noreorder` without routing it back
through `process_line`, so the flag never updates. Because the flag has no
scope, an unclosed block-local `noreorder` also eats the `nop` off cc1's own
`j $31` epilogue and damages the FOLLOWING function. Found by charlie,
**reproduced in isolation by the head before escalating**, per the rule that a
lead reaches the operator only with a reproducer. Not blocking: the
source-level bracket works and three functions are byte-exact with it. The
obvious version bump is explicitly not the remedy.

### A screen validated, and a head error inside it

Echo used the round-12 register-saturation screen PREDICTIVELY and called both
its functions' difficulty class before writing any C. Measured against the
round's own outcomes over 27 functions: matched (22) averaged **1.86** distinct
callee-saved registers with **none at 5+**; stalled (5) averaged 5.00, max 9.
Recorded as strictly **one-directional** — high predicts a register stall, low
predicts nothing, since three of the five stalls sat at 2-3 and failed for
unrelated reasons. It is the first screen that supplies what `fresh` cannot
see: low cold-runner yield. The head independently counted one function at 8
registers and echo said 9; **echo was right** — `$30` is `$fp` and `$s8` and
the same register, splat writes `$fp`, `objdump` writes `s8`, and the head's
regex missed one. Documented next to the screen.

### Runners

alpha 11/12 `code_2cc8c_c` then 18/20 `Entity_g`; bravo 10/11 `Entity_e`, 2/3
`code_d294`, 8/13 `code_d294_c`; charlie 8/8 `code_8220_b`, 4/14
`code_8220_c`, then 0 matches but 8 functions unblocked on the family
re-attempt; delta 4/8 `code_d294_b` then 26/27 `code_2cc8c_d`; echo 2/3
`Entity_d`, 0/2 `class_3bb8c_b`, then **17/17 on `Entity_f`** with no stalls.

**`Entity_f` is the round's cleanest result and it was not luck.** All 37
functions of that segment were measured before carving — zero hits on all
three blocker screens and zero demanding 5+ registers — and echo was told
explicitly that a stall there would therefore mean a genuine source-shape
problem. There were none.

**Echo's zero-match pass was not a wasted pass** and is the case for §3c: it
established that the two register-saturation residues above the threshold need
opposite fixes (a full permutation at zero drift versus a missing-register
frame-size gap), that three instances sit in one header family, and that
levers proven elsewhere in the round do not transfer in. That negative is what
stops the next round spending a budget there.

### Header collisions: three, and one git did not flag

Two runners on adjacent slices of one carve is the documented hazard and it
fired three times. `Entity.h` between bravo and echo: complementary, unioned,
both runners' explicit existing-declaration notes made it one pass.
`code_d294.h` between delta and bravo: a REAL collision — the same field given
two different type NAMES, resolved on the sibling struct's naming and on which
name matched code already referenced. Then the one worth remembering:
`func_8001ECFC` was DECLARED typed by delta and MATCHED with raw `s16 *` by
bravo, and **git auto-merged both without a conflict marker**; only the build
caught it, on `conflicting types`. Resolved toward the typed signature after
testing that it still matches 44/44 — better than either runner's own version.
A shared-header collision git does not mark is more dangerous than one it does.

### Operational

Runner alpha found `git checkout --`, `git show HEAD:file > file` and
`git stash` all blocked by the safety classifier — correctly, since each would
have reverted 11 matched functions — leaving no sanctioned way to stage. It
worked around with `git hash-object -w` plus `git update-index --cacheinfo`,
which never touches the working tree and is strictly safer than what was
blocked. Worth knowing for the next runner that hits it.

Alpha also sat on 11 matched functions and 12 reports with **zero commits**
for hours before being told to commit. It came out clean, but that is the
failure mode that loses a pass silently, and §3b's "check `git status
--porcelain` the moment a runner reports" should be "check it while they
work". Echo was caught in the same state later and flagged early.

**Teardown complete: all five worktrees removed, all five runner branches
deleted, `main` green, nothing deferred.**

### Next round

**Carve first — `fresh` is 1.** The single remaining fresh function is
`class_3ac78`'s `func_8004A534`, which the register screen puts at 8 and which
should be handed out expecting a documented stall, not a match. Uncarved
ground is 641 functions in three segments: `class_3bb8c_d` (305),
`code_179d8` (274) and `code_2cc8c_e` (60). Two things are already measured
about `code_2cc8c_e` and recorded in the splat yaml so nobody leads with them:
`func_8003F2AC` is `nop_mflo_mfhi`-blocked AND wants 6 callee-saved registers,
and `func_8003FB1C` is `addiu_at`-blocked and owns a real jump table whose
rodata slot will need attaching at carve time. `code_179d8`'s `0xFD8` rodata
slot genuinely holds text pointers and will need attaching too.

The `func_800197C4` family is the best-value stall work available: 8 functions,
all instruction-exact, one shared residue, and a named lead — `self`'s real
type, visible from `func_80018464` in `code_8220_b` (the deprioritized
958-instruction body) rather than from any family member.

---

## 2026-09-03 — round 12: 5 runners, 35 matches, and the SDK's macro layer found inert

**415 -> 454 matched (30.60% -> 33.48% of game code; matched bytes 22.04% ->
23.47%). Build green in main after every one of the five merges.** Five
runners, each on its own unit, one pass each. 35 matches came from the runners;
the other 4 of the +39 are `jr $ra; nop` bodies splat generated itself in the
newly carved units.

**Gates.** 4a clean: no other head, no foreign commits, no pre-existing
worktrees, `origin/main` reflog re-checked before teardown. Gate 1 found **14**
true fresh functions — all clear of both blockers, but spread 5/3/3/2/1 across
five units, which is four units too thin to staff. So Gate 2 fired and the head
carved four 20-function slices before provisioning anything: `code_2cc8c_c`,
`Entity_e`, `code_8220_b`, `code_d294_b`. Blocker-density census at carve time
per round 10's lesson: `code_2cc8c_c` 0/20, `Entity_e` 1/20, `code_d294_b`
1/20, `code_8220_b` 2/20. `fresh` went 14 -> 90. Each carve verified green on
its own before the next was attempted.

**The worktree permission grant PARALLEL-RUNS.md described as being in place
did not exist.** No `.claude/settings.local.json`, and no
`additionalDirectories` key at project or user level either — so for eleven
rounds the doc had asserted, as settled fact, a setup that had never existed in
this clone, and every prior round was approving runner commands interactively.
The same paragraph also claimed the file was gitignored; it was not, so the
first head to actually write it would have committed one operator's absolute
paths into every clone. Escalated to the operator, granted, and the
`.gitignore` entry added. **A doc claim about MACHINE STATE decays differently
from one about the binary**: a wrong fact about the executable gets caught the
next time someone measures it, because measuring is the job; a wrong fact about
a settings file is nobody's job to re-measure and survives on age alone.

**TOOLCHAIN LEAD, escalated and NOT acted on — the Psy-Q inline macro layer is
silently inert.** Eight headers under `include/psyq/` have CRLF line endings,
and 2.6.3's `cpp` splices `\` only when `LF` follows immediately. **1134
multi-line macros** — 1043 in `INLINE.H`, 87 in `LIBGPU.H`, 4 in `LIBGS.H` —
expand to `{\ ;`, an empty block plus a null statement. That is valid C: it
compiles clean, warns nothing, and emits nothing. So `gte_stsxy3(...)` silently
does NOTHING. Blast radius today is zero (no `gte_*` call sites in `src/`), so
it is a landmine rather than an active bug. Census and reproducer in
`docs/research/psyq-header-crlf-blocker.md`. Not fixed: un-breaking 1134 macros
in pinned vendored headers is the operator's call under rule 5.

It also produced a near-miss worth recording. The first test of whether the SDK
could express retail's GTE sequence showed the macro emitting nothing at all —
which reads exactly like a clean negative and was actually the CRLF bug. **A
negative result about a macro is only evidence once you have proved the macro
expanded.** Check the preprocessed output, not just the objdump.

**Runners.** charlie 8/8 on `code_8220_b`, alpha 8/8 on `code_2cc8c_c`, bravo
8/8 on `Entity_e`, delta 8/8 on `code_d294_b`, echo 3/5 on `code_2cc8c_b` with
two stalls. Every claimed match was re-verified individually in main after
merging, and every count was taken from the commits rather than the summaries
(§3) — all five reconciled exactly this round, including reports-per-function
and remaining `INCLUDE_ASM`.

charlie matched six GTE store leaves whose bodies are necessarily inline asm —
there is no C that emits `swc2`. Checked against HARD RULE 6 and it passes:
`"r"(ptr)` leaves the GPR to the allocator, `$12`/`$13`/`$14` are COP2 *data*
registers with no GPR identity to pin, and the project already ships Sony
headers built on exactly that construct. Two overstatements in its report were
corrected: it asserted "the only way" without ever testing the SDK path, and
its clobber list is thinner than Sony's own (fine for a standalone leaf, wrong
as a template).

**Head triage found two real defects in otherwise sound work, both of the same
species — a label doing the work of evidence.**

1. **A misattributed method-table slot, propagated one round.**
   `Obj86B60Methods::slot118` was recorded in `func_8003C944.md` as holding
   `func_8003DFA0`, which actually sits at `+0x120`; `+0x118` is
   `func_8003DE30`. Alpha caught it, the head re-verified against the table
   bytes. It survived because `func_8003C944` only dispatches through the slot
   and discards the result — its match was byte-exact with the wrong name
   written down, and stayed byte-exact after the fix. **An entry naming a
   slot's OCCUPANT is a claim about DATA; the compiled offset that matching
   verifies is a different claim, and only the second one ever gets checked.**

2. **Two different stall classes filed as one.** Echo reported
   `func_8003D73C` (40/145) as the same register-identity class as
   `func_8003DAD4` (114/118). The retail-side census says otherwise: DAD4 saves
   6 callee-saved registers (`$s0..$s5`) with two s-regs and `$fp` spare, and
   its residue is delay-slot placement plus one temp choice. D73C saves **8**
   (`$s0..$s7`) — the entire callee-saved file, zero headroom — so its body
   needs a 9th live cross-call value, spills into `$fp`, and renumbers every
   register downstream. Exact length, zero inserted/deleted instructions,
   40/145. Echo had even recorded the evidence against its own framing (DAD4's
   fixes did not transfer) without connecting it.

   **New stall class, and unlike register-identity it has a lever.**
   `grep -oE 'sw +\$s[0-9]' <func>.s | sort -u | wc -l` — 8 means saturated.
   The fix is to reduce values live across calls, and echo had already filed
   that exact lever as an unrelated observation (re-dereference rather than
   cache in a local).

**Learnings promoted:** 15 entries — 3 toolchain facts and 12 source-shape
idioms, plus the new stall class. Highlights: crossjump is sensitive to a
vtable slot's declared return type and a LOCAL function pointer is the safe
lever (bravo avoided the round-7 shared-retype hazard correctly); a sparse
`switch` can beat an `if`/`else` chain **without** contradicting the
dense-switch blocker, because the discriminator is DENSITY (verified zero jump
tables in the instance); a statement in a branch's delay slot is unconditional;
a `"memory"` clobber anchors memory only and a register-only `i = 0` floats
across it; and re-dereference-don't-cache, with its shape-specific counter-case
where retail DOES cache an array-indexed loop bound — two superficially
identical questions with opposite answers, the same trap shape as round 7's
`slotC4`/`slotCC`.

**Housekeeping.** The carve left four orphaned monolithic `asm/*.s` behind on
rename; `progress.py` warned, they were deleted and re-extracted, and the build
re-verified. This is the same warning that concealed round 11's wrong headline
number — clearing it is not cosmetic.

**Left deliberately unworked:** `func_80018464` (954 words, `code_8220_b`) is
by far the largest function in any carved unit. Not blocked — real work, just
not a fit for one runner's attempt budget. Left as `INCLUDE_ASM` with **no**
report so it stays FRESH for a round that can staff it deliberately.

**Next round: runners.** `fresh` stands at 49 across seven units with no carve
needed — `code_2cc8c_c` 12, `Entity_e` 11, `code_8220_b` 9, `code_d294_b` 8,
`Entity_d` 3, `code_d294` 3, `class_3bb8c_b` 2, `class_3ac78` 1. Four units
carry 8+ each, which staffs four runners without touching the yaml.

---

## 2026-09-03 — round 11: 4 runners, 62 matches, and a headline number that was wrong

**353 -> 415 matched (26.03% -> 30.60% of game code; past thirty percent).
Build green in main after every one of the eight merges.** Four runners, each
given a freshly carved unit and then sent back into the SAME unit for a second
pass. `code_8220` became the project's fifth fully-worked unit (16 matched, 4
correctly-blocked, 0 fresh).

**A correction before any gate: the project's stated percentage was wrong.**
`progress.py` was warning about two stale `asm/*.s` files left behind by an
earlier rename. Because they were not declared in the splat config the tool
IGNORED them — and ignoring them removed real functions from the denominator.
Uncarved game code read 464 when the truth was 902, and game-code completion
read **38.13% when it was actually 26.03%**. Deleting the two files and
re-extracting fixed it. No previously claimed match is affected; every one of
them still verifies. What was wrong was only the headline, and it was wrong in
the flattering direction for an unknown number of rounds. **A `progress.py`
warning is not cosmetic — clear it before reading the numbers under it.**

**Gates.** 4a clean: no other head, no foreign commits, no pre-existing
worktrees. Gate 1 found **3** true fresh functions, far under (4 runners x 8),
so Gate 2 fired and the head carved four 20-function slices before provisioning
anything: `code_2cc8c_b`, `Entity_d`, `code_d294`, `code_8220`. Blocker-density
census at carve time, per round 10's lesson — three of the four came out at
zero blocked functions, `code_8220` at 4 of 20. `fresh` went 3 -> 80. Each
carve was verified green on its own before the next was attempted.

**Runners.** All four went 8-for-8 on the first pass, so all four were sent
back into their own units (§3c) rather than idled. charlie took **16** on
`code_d294` and reconstructed a whole class from nothing — no header existed
for that block when it started. delta took **15** and closed out `code_8220`
entirely, reconstructing `BasicClass`, the ROOT of the game's class framework,
from its callers before matching the primitives that confirmed it. bravo took
**16** on `Entity_d`, alpha **15** on `code_2cc8c_b`. Zero stalls across all
62 matches — the four carves were unusually clean ground, and the second
passes hit the larger bodies without a single unrecoverable residue.

**The round's best finding is a fingerprint, not a match.** bravo found that
one function being a single word short makes EVERY later function in ROM order
score near-zero simultaneously, and that the tell is `funcdiff`'s "differs
outside range" showing the SAME six-figure value across all of them. CLAUDE.md
already named address drift as one of the four ways a score lies; it had no
stated signature. bravo then hit it a second time and applied the rule
deliberately instead of debugging downstream symptoms.

**Two runners independently hit GCC 2.6.3's cross-jump/tail-merge pass** in
different units — alpha establishing that a bare `__asm__("")` cannot suppress
it (cross-jump is block-level, not local scheduling), delta finding it folds
two textually identical calls into one site. Two independent instances in one
round makes it a rule. alpha then produced the discriminator that makes the
pair actionable: same instructions in a different ORDER means a barrier helps;
a missing or duplicated instruction from cross-jumping means it will not.

**An open residue class moved.** bravo reached the "identical assignment
reaching different merge points" class (open since `func_8005DBF0`, 72/74)
with a barrier — but the function only matched once a statement-order fix
removed the root cause, after which the barrier was unnecessary and was
removed. So the merge was a SYMPTOM of wrong statement order. The retry lever
for `func_8005DBF0` is the statement order, not the barrier.

**A generalization the head invited and a runner correctly refused.** Asked
whether a unit-wide field-read order held, alpha found it held for five
functions and was violated outright by a sixth, which matched immediately in
its own literal order. Reported as a negative rather than as the confirmation
the question was fishing for. The order is per-function.

**Three inherited "facts" corrected, all by measurement:**

- PARALLEL-RUNS Gate 2 listed `0xA8C` as a text-pointer rodata slot needing
  attachment, "confirmed against the data". It holds zero pointers — 12 ASCII
  words, one printf format string. `code_8220` carved and linked green with it
  left standalone. (`0xFD8` was re-surveyed and does stand: 179 real pointers.)
- DECOMPILATION_LEARNINGS called `New_X` allocator wrappers the highest-value
  permuter target on the reading that the shape was unmatched. There are two
  variants: the CHECKED one already matched in `Entity.c` and matched again
  cold this round (`func_8001CA94`, first attempt), and the RETURN-REGARDLESS
  one that actually stalls. The permuter target is the second, smaller
  population — count it before spending a round on it.
- A match report proposed promoting a THIRD standing blocker screen
  (`nop_mflo_mfhi`). Measured: 2 of 181 queued functions, against `gp_rel`'s
  125. Not falsified — zero matched C functions contain the pattern — but far
  too narrow for a standing grep. Recorded in the blocker doc with the number
  to re-measure as carving proceeds.

**A head-protocol lesson, from the head's own near-miss.** alpha reported
"8/8 attempted" and listed 4 functions remaining; the branch held 7 commits, 7
reports and 5 remaining `INCLUDE_ASM`. The work was sound and all 7 verified,
but the count was wrong in both directions and the omitted function
(`func_8003DAD4`) would have silently dropped out of the next round's queue.
Count from the commits, never from the summary table. Added to §3.

**Also corrected:** charlie's 16 reports were labelled "round 10" and "round 2"
(its own second pass), both ambiguous in the durable record. Relabelled to
round 11 with dates.

**Deliberately left, and why.** charlie stopped with 3 functions untouched
(`func_8001CD60`, `func_8001CEB4`, `func_8001D008`) rather than rush
unexplored fixed-point magic-constant division; alpha left the 5 largest in
`code_2cc8c_b` (80-147 words); bravo left 3 (156-224 words). None were
attempted, so none carries a report and all correctly read as `fresh`. That is
honest bookkeeping, not an omission.

**Next round.** Runners again, but Gate 2 will fire first: 14 fresh across
three units is under (4 runners x 8), and 11 of those 14 are the large bodies
three runners deliberately declined. Uncarved blocks available, derived at
close of round:

```
305 class_3bb8c_d    274 code_179d8    113 code_2cc8c_c
 57 Entity_e          36 code_8220_b    35 code_d294_b
```

`code_179d8` remains the one to avoid — 43% blocked. `code_8220_b` and
`code_d294_b` are the immediate continuations of the two blocks whose class
designs were reconstructed this round, so they are the cheapest ground
available despite being the smallest: the type vocabulary already exists.

A permuter round is NOT indicated. This round produced **zero stalls**, so
there is no near-miss residue to permute — the permuter's input is exactly
what a clean round fails to generate.

## 2026-09-02 — round 10: 5 runners, 30 matches, and the head breaking its own rule

**320 -> 350 matched (23.60% -> 25.81% of game code; past a quarter). Build
green in main at every step.** Five runners, the maximum the permission grant
allows. `Entity_c` became the project's fourth fully-matched unit at 20/20.

**Gates.** Gate 4a clean — no other head, no stale worktrees. Gate 1 found 22
fresh, all clear of both blockers. Gate 2 carved `code_2cc8c` (20 functions),
chosen over the larger `code_179d8` on a **blocker-density census** rather than
size: 3% of `code_2cc8c`'s functions hit the open blockers against 43% of
`code_179d8`'s. That census is worth re-running at every carve; it inverted the
obvious pick. The carve needed its rodata jump-table slot **split** rather than
attached, because slot `0x1890` holds three tables and one belongs to a
function out in the asm remainder — attaching it whole would have broken that
table in the other direction.

**Runners.** echo took **16 of 20** on `code_2cc8c`, ground carved hours
earlier, against a target of 8 — and established the unit's whole type
vocabulary, which is what makes the block's remaining 133 functions cheaper.
alpha went 6 for 6 on `Entity_c` then closed a sixth-round stall to finish the
unit. bravo took 3 of 4 in `DreamSys` and closed `DreamSys__TimerTick` (62/62)
with a single `__asm__("")` its own report had recorded as untried. delta took
3 in `class_3bb8c_c`, then moved to `class_3ac78` and spent its second pass on
struct corrections. charlie matched nothing in `class_3bb8c` but moved
`func_8004B700` 52/140 -> 125/140 and characterised all four of its stalls.

**Redeploying finishers into their own units paid for itself.** Every one of
the four early finishers was sent back rather than idled or replaced, and that
produced two matches (`func_8005F544`, `DreamSys__TimerTick`), three
exhausted-class verdicts, and the `code_2cc8c.h` evidence-boundary work. The
two matches were both closed by levers the functions' OWN prior reports had
named and left untried.

### The head's own error, which cost the round more than any stall

Screening the carve, I found two functions whose only `addiu $at, $at, %lo`
hit was a `%lo(jtbl_*)` jump table. I reasoned that a table loading a CODE
address must be a different construct from an indexed DATA global, "corrected"
CLAUDE.md's screening grep to exclude `jtbl`, audited the corpus, recovered
three functions that had been banked as blocked, and **broadcast it to five
live runners — one of which had already been handed two of them as work.**

Then I measured it. cc1 emits the same generic pseudo-op for both
(`lw $2,$L13($2)` against `lbu $2,D_x($4)`), and the folding happens in maspsx
*below* cc1, which cannot tell them apart. Dense switches are inside the
blocker. All five runners got retractions; the grep is reverted with a note at
the point of temptation; the blocker doc carries the reproducer.

**The generalisation, now in CLAUDE.md:** the project rule "never escalate a
toolchain lead you have not reproduced in isolation" runs in **both
directions**, and de-escalating is the worse one. Widening a blocker's scope
costs attempts; narrowing it sends runners at functions that cannot match and
leaves behind reports indistinguishable from ordinary stalls. The reproducer
took under a second and was available from the first minute.

It also caused the round's only merge conflict, because I both messaged echo
AND edited echo's unit file on main. New PARALLEL-RUNS rule 6: **a correction
goes down one channel.** Default to the message — echo's version of the
paragraph was better and is what I kept.

### Three workflow bugs the round exposed

- **A conflicted merge is a FOURTH way a score lies.** `git merge` returned 1
  with conflict markers in a `src/` file, and `build-and-verify.sh` run
  immediately after still printed `build exit=0` and `OK`. Both halves of the
  usual discipline pass. Now documented next to the other three, with the
  `MERGE_HEAD` check.
- **A worktree isolates files, not the process table.** alpha finished with the
  permuter, tidied up with `pkill -9 -f "decomp-permuter"`, and killed bravo's
  in-progress searches in another worktree. Same shape as round 8's shared
  `/tmp/b.log` crossover. It fails SILENTLY — bravo's summary would have read
  "the permuter found nothing", indistinguishable from the truth, and a
  fabricated permuter-exhausted verdict is exactly what stops future rounds
  trying. Detected only because alpha self-reported. Now collision rule 2b,
  with the scoped-kill form.
- **`permuter-seeds/` was not gitignored**, so any runner using the permuter
  was structurally unable to report the clean `git status --porcelain` the
  protocol demands. That check is one of two honesty mechanisms the round
  depends on; it must not cry wolf on the runners doing the hardest work.

**And a good discriminator came out of it:** asked to prove its searches were
not killed, echo answered from exit codes rather than inference — GNU `timeout`
returns **124** when it killed the child itself, versus **137** for an external
SIGKILL. Wrap long searches in `timeout` so "did someone shoot my search?" has
a recorded answer.

### The permuter, run in anger for the first time

Five searches, ~140,000 iterations: two matches, three exhausted verdicts. What
separated them was **seeding, not closeness**. `func_8005F544` went to zero in
23 iterations once re-seeded from a manual near-miss whose only remaining
defect was a register difference. `func_8003CB68` — a single-register residue —
found no improvement over its seed at all in 13.5k iterations, which retires an
earlier round's claim that the permuter closes those fast.

**A base score of 5 that will not move is a stronger negative than a base of
460 that halves.** `--debug`'s base-score COMPOSITION (register differences
versus insertions and deletions) predicts viability where its magnitude
misleads. Three of the five searches converged on forms that are not real C —
stale-register reuse, a `volatile unsigned int` return in place of an enum —
and the runners rejected all three. bravo marked `CalcDreamColor` exhausted
*for its region* but explicitly not overall, because the same search surfaced a
clean candidate at 135 it did not have budget to apply. That granularity is the
right habit.

### Head verification, and where it did not pay

The protocol calls stall adjudication the highest-yield head activity. This
round it was mixed, and both halves are worth recording. It caught real
problems: delta's "3-way rotation" is a 2-way exchange (its proposed learning
would have misfired as a screen), delta credited its own type split to round 9,
and charlie labelled two STALLED functions "MATCHED" in header comments the
whole region's declarations rest on. All three would have hardened into
precedent.

But two head attempts on `func_8004D47C` failed. Retail reads `arg2` four
times against two each for the others and `$s0` goes to the most-referenced
pseudo, so equalising reference counts looked decisive; a block-scope
declaration will not compile (C89 keeps the outer prototype's arity) and an
empty-parens file-scope declaration regressed a neighbouring match 20/20 ->
19/20. Filed as **unresolved rather than refuted** — the counts were never
actually equalised.

### Fresh queue is nearly dry — next round must carve

**3 fresh functions left**, all large: `func_8004A534` (163 insn, deliberately
left unattempted and unreported by delta so it stays counted as fresh),
`func_8004C6A8` (165), `func_8004C93C` (109). 101 of the 104 queued functions
now carry stall reports.

Carve candidates, measured this round with the RAW (correct) grep —
`class_3bb8c_d` 305 functions at 17% blocked, `code_179d8` 274 at 43%,
`code_2cc8c_b` 133 at 1%, `Entity_d` 77 at 1%, `code_8220` 56 at 14%,
`code_d294` 55 at 4%. **`code_2cc8c_b` and `Entity_d` are the standouts**, and
`code_2cc8c_b` has echo's fresh header to inherit. Re-derive rather than trust
these numbers.

## 2026-09-02 — round 9: 2 runners, 15 matches, and a lesson about consolidation

**299 -> 320 matched (22.05% -> 23.60% of game code). Build green in main
throughout.** Two runners by operator choice, on the only two units that had
full queues.

**Gates.** Gate 1 screened both queues and excluded two functions with
existing stub reports. Gate 2 carved `class_3bb8c_c` (20 functions) before
provisioning, and in doing so **corrected a prediction round 8 had written
down**: the yaml comment and round 8's own PROGRESS entry both said the next
carve was where the BIOS-trampoline and jump-table dispositions would have to
be settled. Measured, 80 functions precede the segment's first `jlabel` and 96
precede its first `jr $t2`, so roughly four more slices fit first. Acting on
the old note would have meant expensive work four carves early. The yaml now
carries the measurement.

**Runners: 14 matches, 3 stalls.** bravo went **9 for 9** on `class_3bb8c_c`,
brand-new ground carved hours earlier, no stalls. alpha took 5 of 8 on
`class_3bb8c_b`, whose cheap seam went last round — including `func_8004CE24`
at 97 words, which needed four independent fixes composed. bravo also found
**two previously unknown sibling vtable classes** by tracing a getter's
`lui`/`addiu` back into `asm/data/*.s`, which `classtable.py` had no knowledge
of; that technique is now a promoted learning.

**The shared-header collision was predicted, announced, and cheap.** Round 8
discovered that collision rule 1 partitions `src/` but not headers, and paid
for it at merge time. This round the head spotted at ASSIGNMENT time that both
units describe the same `Obj866E8` class, told both runners (additive edits,
declarations beside related ones, state any change to an existing
declaration), and predicted the specific conflict point — bravo's unit reads
`0x2F4($a0)`, past the struct's then-current `+0x1E4` tail. It landed exactly
there: **one** conflict region, alpha's retype of `unk1E4` against bravo's
appended tail, complementary, resolved in one pass. The rule worked.

**The round's most useful finding is about the head's own job.** alpha stalled
`func_8004D1D0` one word short on the redundant-`move` class and, following
MATCHING-GUIDE's "best-posed permuter target" advice, correctly declined to
burn attempts. The lever that closes it had been derived **last round** by
another runner on `func_8004C0AC`, four addresses away in the same class
block — mention the source expression twice, bound first, loop variable
second — and **round 8's consolidation failed to promote it** out of that one
match report. The head applied it this round: **29/29 first try.**

So: an unpromoted learning does not exist. The runner's process was correct
throughout and the cost landed a round later as a stall someone had already
solved. That entry is now in DECOMPILATION_LEARNINGS with its own history
attached, alongside the direction distinction it needs to be usable — round
8's lever REMOVES a redundant `move` your source restates, this one ADDS the
`move` retail genuinely has, and both are the same rule about mention count.

**Head stall triage, 3 for 3 audited, 1 reclaimed:**

| function | runner verdict | head verdict |
| --- | --- | --- |
| `func_8004D1D0` | stall, permuter target | **MATCHED 29/29** with last round's unpromoted lever |
| `func_8004CFB8` | stall, cc1 RTL choice | stall CONFIRMED, **reclassified**: source-shape + permuter, NOT a toolchain blocker |
| `func_8004CAF0` | stall, frame off by 2 registers | CONFIRMED, and the untried next move named |

The reclassification matters for routing. `func_8004CFB8`'s reproducer work
was exemplary and rules the downstream tools out, but "cc1 expands this
statement shape differently" is the ordinary condition of an unsolved
function, not a pending operator decision — filing it beside the two
escalated blockers would have implied one. The head also tested one further
shape there (retail has ONE logical multiply, speculatively hoisted into the
delay slot) and it scored **17/28 against the runner's 25/28**, closing off
the most plausible remaining non-permuter reading.

`func_8004CAF0`'s 8/92 is a cascade, not a distance: a frame-size difference
moves every stack offset in the function. Its fix inverts the usual instinct —
give each intermediate its own named local so the frame GROWS to retail's
`-0x38`, rather than reusing one temp and letting GCC coalesce.

**Also settled this round:** `func_8004D3DC` prompted a check of
a helper-arity question. The callee `func_8001E57C` reads
neither `$a0` nor `$a1` — it takes **no arguments** and returns
`&D_8006B5CC`. So `src/class_3ac78.c`'s 2-argument declaration and
`class_3bb8c.h`'s 1-argument declaration are both wrong about the function and
both right about their own call site, and both units are byte-exact. Retail's
source called one zero-argument getter with different argument counts from
different files, which is what C89 does with no prototype in scope. Both sites
are now annotated do-not-reconcile, because collapsing either to `(void)`
changes the caller's argument setup and breaks the match.

**Toolchain: nothing new.** `nop_mflo_mfhi` still stands at two instances from
round 8; no round-9 function reached either open blocker except the one stubbed
at carve time.

**Next move: runners, and carve first.** Both units worked this round are now
down to 2 and 4 fresh, so the fresh queue is thin per unit again even though
the total is respectable. `Entity_d` (77 uncarved) is the natural next carve
and there is no shortage — 922 game functions remain uncarved. The permuter
now has three named candidates whose reports say what to try
(`func_8004CFB8`, best isolated at 3 words in 28; `func_8004CD38`;
`func_8004C470`), and `func_8004CAF0` is explicitly NOT one of them — it wants
a runner with the variable-lifetime hypothesis stated up front.

---

## 2026-09-02 — round 8: 4 runners, 33 matches, and the permuter's first run

**266 -> 299 matched (19.62% -> 22.05% of game code). Build green in main
throughout. `code_1677c` is now the project's third fully-matched unit.**

**Gates.** Gate 1 found 44 nominal `fresh`, but screening every candidate
against the two open blockers cut DreamSys's 12 to what it really had and
turned up two functions in `class_3bb8c` that `progress.py` was counting as
fresh while they sat squarely on the `addiu_at` blocker — stub reports filed
before provisioning, which is the only thing that stops the next round staffing
someone onto them. Gate 2 carved `class_3bb8c_b`, 20 functions off the front of
the old remainder, all 20 clear of both blockers; the boundary checks and the
BIOS-stub/jump-table hazards still ahead of it are recorded in the splat yaml.
Gate 3 did not fire: 62 screened-clean fresh functions is not a dry queue.

**Runners: 30 matches, 5 stalls, across four units.** alpha 9/10
(`class_3bb8c_b`, brand-new ground), bravo 8/9 (`Entity_c`), charlie 7/9
(`class_3bb8c`), delta 6/8 (`DreamSys`). Every runner filed a report for every
function it touched, matched ones included, and every tree was clean at report
time — though two runners had to be asked, one of them twice, before they
stopped batching commits. Cold-runner yield stays high on freshly carved
ground, which is the third round in a row saying carve-and-staff beats
polishing residue.

**The head's own result is the round's headline: `tools/decomp-permuter` had
never been run on this project, and it closed a three-round stall within a
minute of working.** MATCHING-GUIDE had already nominated the target
(`new_class_6d3c8`); the permuter reached score 0 at iteration 47 after three
rounds, 20+ recorded attempts and a root-cause hypothesis argued against GCC's
`reorg.c` had all failed. Two more standing stalls fell the same day. The
setup is now `tools/setup-permuter.sh`, committed rather than left in the
gitignored `permuter-work/` where I first built it, with the four
looks-like-a-broken-toolchain traps fixed and explained in its header.

**Three retired stalls, and none of them was what its report said.**

| function | had stood as | actually was |
| --- | --- | --- |
| `new_class_6d3c8` (24/24) | delay-slot filler choice, 3 rounds | one surplus `return` on the null path |
| `strcat` (42/42) | ditto + a head adjudication | `return dest;` not `return NULL;` on a guard |
| `func_80026698` (57/57) | switch-lowering internals, 2 rounds | the stored value misread as 3; it is 1 |

The generalisation, now the first subsection of DECOMPILATION_LEARNINGS'
source-shape idioms: **when a diff is one redundant or one missing `move`,
count how many times your source mentions the value.** `new_class_6d3c8`
mentioned it once too many, `strcat` once too few, and in both the surplus or
missing copy landed in a delay slot — which is precisely why both read as
scheduler whims. A surplus value has to go somewhere and a free delay slot is
where the scheduler puts it, so "different filler" and "one value too many"
are indistinguishable in a diff. Prefer the surplus-value reading; it has a
fix.

`func_80026698` came from neither reshaping nor the permuter (which improved it
215 -> 130 and stalled). It came from noticing that `li $v0, 0x1` sat in the
delay slot of the case-3 branch, so `$v0` holds 1 — not the 3 it held for the
comparison — by the time the target's store runs. One misread value had
manufactured two separate compiler mysteries across two rounds.

**Permuter limits, measured rather than assumed.** Two searches found nothing:
`func_8005DBF0` (700 -> 660) and, indirectly, `func_80026698`. Both negatives
are informative and both are recorded in their reports. The permuter searches
source SHAPES; `func_8005DBF0`'s residue is GCC's cross-jumping pass collapsing
two textually identical assignments, which is not a shape, and
`func_80026698`'s was a semantic misreading. That report now redirects the next
attempt at a TYPE change instead, on the strength of round 7's `slotC4`
finding — the one precedent this project has for suppressing a tail-merge.

**Two workflow bugs, both real, both fixed at the source.** The runner prompt
hardcoded `/tmp/b.log` for every runner; `/tmp` is not per-worktree, and two
runners crossed over on it, one acting on the other's build result — a false
oracle in the log a runner greps to decide whether its score means anything.
And collision rule 1 partitioned `src/` but not headers, so assigning two
adjacent slices of one carve handed two runners one shared header and produced
the round's only conflict. Both are now documented with their reasons.

**Resolving that conflict is worth reading as a template.** Neither runner was
wrong. They had derived the same objects from opposite ends: both
independently found the same struct field from different call sites, each had
fields the other lacked, both kept the struct size right. The only true
collision was two different type NAMES for one field, which I unified — the
multiple-independent-local-views convention is for views in different unit
headers, not inside one. Every match from both runners was then re-verified
individually, because a shared-declaration merge that compiles is not
necessarily one that preserves codegen.

**Toolchain: `nop_mflo_mfhi` now has a SECOND independent instance.** delta hit
it on `DreamSys__GetPreviousDayMood`, whose logic is confirmed correct
instruction-for-instruction via asm-differ, after `IsDaySpecial` in round 7.
Two instances in two functions is the point at which a corpus census is worth
doing. Escalated, not acted on.

**Also reconciled:** `class_3ac78.h` typed `D_800866E8` slot `+0x124` as
`void *(*)(Class866E8 *, void *)`. alpha matched the occupant byte-exactly as
`s32 func_8004C5D0(Obj866E8 *self, s32 key)` and correctly left the other
unit's header alone; the head retyped it. Safe here only because the slot has
no C call site yet — its caller is still `INCLUDE_ASM` — and the whole-image
SHA1 was re-verified after.

**Next move: runners again, on a carve.** The fresh queue is 26 and thin per
unit; `class_3bb8c_b` retains the most at 10, and it is the same freshly
carved ground that produced this round's best cold yield. Carve one or two
more slices first — `class_3bb8c_c` is next in line and is where the
BIOS-trampoline and jump-table dispositions finally have to be made, which the
yaml now says explicitly. A permuter round is no longer speculative: it has a
working harness and three named candidates whose reports say what to try
(`func_8004CD38`, better posed than the rest; `func_8005F544`;
`func_8004C470`). But it is still worth less than cold ground while carving is
this productive.

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

**The overlap lasted the whole round, not the twenty minutes it first
appeared to.** Round 6's head went quiet in `git log` after 13:59, and this
head recorded it as finished on that basis. It was not: `main` was pushed to
`origin` twice more during round 7 — at 15:02 and again later — by something
other than this head, each time carrying this head's own commits. Harmless in
effect (the pushed content was already merged and verified here, and
`origin/main` never diverged from local), but "no new commits in `git log`"
is NOT evidence a concurrent session has ended. **A quiet log means a quiet
log.** Check `git reflog show origin/main` too, which is what finally showed
these.

**Standing operator escalation: rule 5 (main is single-occupancy) has no
mechanical enforcement.** Nothing stopped either head writing to main, or
pushing it. The hooks guard `check.sha1`/`build.sha1` and `asm/`, not the
branch and not the remote.

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
