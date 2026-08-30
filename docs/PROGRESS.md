# Progress log

One entry per session or round. The head writes these during consolidation.
Counts come from `python3 tools/progress.py`; dated entries are allowed to go
stale, prose elsewhere is not.

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
