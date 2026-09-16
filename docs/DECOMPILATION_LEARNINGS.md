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
  function scores a mismatch with the stores simply absent. Census,
  reproducer and disposition: `docs/research/psyq-header-crlf-blocker.md`.
  **Escalated, not fixed** — un-breaking 1134 macros in pinned vendored
  headers is an operator call. **The `gte_*` call sites that DO exist in
  `src/` (2026-09-14) come from `include/gte.h`, not from `INLINE.H`** — a
  project-owned, LF-terminated reimplementation of the macros the game
  used, in the GNU flavour (base pointer as an `"r"` operand) rather than
  `INLINE.H`'s ASPSX flavour (`move $12,%0` plus Sony macro-call words that
  gas emits as literal bytes). Include `gte.h`, never `INLINE.H`.
- **A COP2/GTE instruction's C form is the Psy-Q `gte_*` macro, and
  `include/gte.h` holds the GNU-syntax versions that this pipeline can
  assemble (round 12, restated 2026-09-14).** There is no C expression that
  emits `swc2`/`lwc2`/`cfc2` or a GTE cofun; the game's source called Sony's
  macros, and retail's bytes are those macros' expansions with GCC's own
  allocation around them. The construct inside each macro is
  `__asm__ volatile("swc2 $12, 0x8(%0)" : : "r"(ptr) : "memory")`, which
  passes CLAUDE.md HARD RULE 6's test: `"r"` leaves the GPR to the
  allocator, and `$12`/`$13`/`$14` are COP2 *data* registers named in the
  instruction text with no GPR identity to pin. Write `gte_stsxy3_g3(prim)`,
  not the `swc2` lines, and add a missing macro to `gte.h` following the
  SDK's name for it (`INLINE.H` for the names, `GTENOM.H` for the COP2
  register assignments) rather than open-coding it at the call site.

  Clobbers: name what the block actually writes, and nothing else. The
  store leaves need only `"memory"`; `gte_stflg` uses GPRs `$12`/`$13` as
  scratch and names exactly those. Round 12 recommended carrying the SDK's
  `"$12","$13","$14","$15"` list on every block "in case it goes `static
  inline`"; that list is a GPR clobber that does not describe what a
  COP2-only block does, and the later "wrong clobber cascades" entry is what
  it costs when GCC believes it.

  A store leaf is a one-line macro call; the larger lesson is
  `func_800195EC`, which was carried from round 13 as a 58-word
  whole-function `__asm__` with a hand-managed `noreorder` bracket because
  `rtpt`/`nclip`/`avsz3`/`cfc2` "have no C form". They have macros. Written
  as branching C over `gte_rtpt`/`gte_stflg`/`gte_nclip`/`gte_stopz`/
  `gte_stdp`/`gte_avsz3`/`gte_stotz`/`gte_stsxy3` it matched on the first
  build. The tell was in the disassembly the whole time: retail's flag test
  (`cfc2 $12,$31; addi $13,$zero,4; sll $13,$13,16; and; sw`) is the
  `gte_stflg` macro body verbatim, and the `addiu $2, $5, 0x5c` in front of
  it is GCC materialising `&ctx->flag` for the macro's `%0`.
  (`func_800196D4`, `func_800196E8`, `func_800196FC`, `func_80019710`,
  `func_80019724`, `func_8001974C`, `func_800195EC`)
- **A negative result about a MACRO is only evidence once you have proved the
  macro EXPANDED (round 12).** Testing whether the SDK could express retail's
  GTE sequence produced an objdump with the macro emitting nothing at all,
  which reads exactly like a clean negative and was actually the CRLF bug
  above. Check the preprocessed output, not just the objdump:
  `cpp ... | sed -n '/yourfunc/,/^}/p'`.

### BLOCKED: no C function can reach a small-data global (2026-08-29)

> **RESOLVED, round 42 (2026-09-15).** maspsx `--gp-symbols=config/gp-symbols.txt`
> (`tools/patches/maspsx-lsd-flags.patch`) emits retail's `%gp_rel($gp)` form
> for exactly the symbols defined in the sdata/sbss segments; `-G` is
> unchanged at every stage. Byte-exact whole image, three blocked functions
> matched on the first build. **The `grep -l gp_rel` routing rule below is
> RETIRED** — a `%gp_rel` reference is an ordinary global access. The
> diagnosis below ("needs a non-zero `-G`") was wrong; see the RESOLVED
> section of `docs/research/gp-relative-blocker.md` for what it actually was.

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

> **RESOLVED, round 42 (2026-09-15).** maspsx `--no-nop-mflo-mfhi`
> (`tools/patches/maspsx-lsd-flags.patch`) leaves cc1's `#nop` hints commented,
> as retail does at every site. Byte-exact whole image; `IsDaySpecial` 52/52 on
> the first build. The construct no longer blocks; the FORWARD/backward lesson
> below stands, because `nearmiss.py` still reports the (resolved) screen.

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

### SDK code hides inside game segments — check `psyq_sdk.py coverage` before matching anything library-shaped (2026-09-11)

The Psy-Q libraries are now LINKED from Sony's own objects (splat `o`
segments; `docs/research/psyq-sdk-objects.md`). Placing every SDK object
against retail found 45 of them inside segments the config calls game code:
`libc2` string functions in `code_171e0`/`code_179d8_*`, nineteen `libsnd`
driver internals across `code_179d8_*`, `libgs`/`libgte` helpers in
`code_2cc8c_e`, and the thirteen 16-byte BIOS trampolines in `class_3bb8c_h*`
that the yaml comments debate turning into `hasm`. `func_8003FC70`, closed as
a 35/35 game match in round 20, is `libgs/gs_108.o`.

Matching such a function is not wrong — the bytes match — but it is work on
code that will be replaced by an `o` segment, and its report then documents a
function that no longer exists in the game denominator. So: before working a
function whose body looks like a string routine, a sound-driver step, a GTE
helper or a BIOS stub, run

```sh
.venv/bin/python3 tools/psyq_sdk.py coverage    # "Placed objects that fall inside GAME-code segments"
```

and if it is listed, convert the segment instead of matching the function.

### `make extract` while a function is LIVE C silently destroys its own `.s` stub (round 26)

**splat only regenerates `asm/nonmatchings/<unit>/<func>.s` for a function
that is currently `INCLUDE_ASM`'d in `src/`.** So running `make extract`
mid-experiment — while your function is live C rather than wrapped — drops
that function's stub, and with it your ability to put the function back.

Runner alpha hit this on `func_80035B2C`, caught it on the very next build
failure, and recovered by restoring `INCLUDE_ASM` and re-running
`make extract`. The recovery is easy; noticing is the problem, because the
failure arrives as a missing-include error that reads like a path typo rather
than as "you deleted the thing you were going to restore".

**Rule: restore `INCLUDE_ASM` BEFORE `make extract`, every time.** `extract`
is one of the four make targets the project allows you to run directly, which
makes it easy to reach for without thinking about what it regenerates from.

### A STALE `.s` makes `funcdiff` report a bogus WINDOW, not just a bogus score (round 26)

`progress.py` warns about stale `asm/` files and the warning reads like
bookkeeping. It is not. `funcdiff.py` resolves a function's byte range from
three sources in descending precision, the FIRST being the `.s` file's own
`glabel`/`endlabel` block. After merging a runner's match, that `.s` is stale
— the function is C now — and funcdiff can hand back a range that is not the
function at all.

Measured at round 26's merge of `func_80053984`, immediately after the merge
and before re-extracting:

```
func_80053984: 67808/67808 words match (file 0x1F4C-0x442CC)
```

67808 words is obviously not an 82-word function, but note the shape of the
lie: it says **match**, with a plausible-looking file range whose END
(`0x442CC` = vram `0x80053ACC`) is genuinely the next function. Only the
START is wrong. A reader skimming for the word "match" gets a green light.

After `make extract`:

```
func_80053984: 82/82 words match (file 0x44184-0x442CC)
```

which agrees exactly with a direct byte comparison over the true range
(`0x44184..0x442CC`, 328 bytes, identical). Note also that the sibling
`func_80052F10`, merged in the same commit with an equally stale `.s`,
reported correctly — so this does not fire reliably enough to be noticed by
habit.

**Rule: `make extract` after merging any branch that matched a function, then
re-run funcdiff.** The whole-image SHA1 is unaffected by this and remains the
authority — `build-and-verify.sh` being green already proves every byte,
including the function whose window funcdiff mis-reported. When the two
disagree, the SHA1 wins and funcdiff's range is what needs re-deriving.


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

### The "split scaled index": a mask on the PRODUCT means a HALFWORD array, not a struct array (round 26)

**Symptom.** Retail computes a partial product early — `sll $a0, $v1, 3` for
what is really a 0x10-stride record index — keeps it live across a long run of
unrelated stores, masks it mid-block with `andi $a0, $a0, 0xffff`, and only
then doubles it and adds a base pointer that is loaded LATE:

```
sll   a0, v1, 3          # idx*8, FIRST instruction of the loop body
... twenty unrelated 0x34-stride stores ...
andi  a0, a0, 0xffff     # the mask -- on the PRODUCT, not the index
lui/lw  v0, D_8006DAD4   # base pointer, loaded LAST
sll   a0, a0, 1          # *2
addu  a0, a0, v0
sh    ..., 6(a0) / 4(a0) / 8(a0) / 0(a0) / 2(a0) / 0xA(a0)
```

Round 26 met this in three functions of `code_179d8_m` and it had been filed
across two rounds as a GCC scheduling mystery, with a recommendation to hunt
for an unidentified statement using the un-doubled product. **It is neither.
It is the wrong source idiom, and the tell is exactly which value gets
truncated.**

**A `(u16)` cast on the INDEX into a 0x10-stride struct array can never
produce this** — the mask then applies to `idx` and the scale is a single
`sll 4`. What produces it is a **`u16` holding `idx * 8`, used to index a
HALFWORD pointer**: the `*2` is just `s16` scaling, and
`(u16)(idx*8) * 2 == idx*16`, so the addressing is identical while the
instruction sequence is not.

```c
u16 woff = (u16)i * 8;                    /* the truncated PRODUCT */
...
((s16 *)D_8006DAD4)[woff + 3] = 0x200;    /* +0x6 */
((s16 *)D_8006DAD4)[woff + 2] = 0x1000;   /* +0x4 */
((s16 *)D_8006DAD4)[woff + 0] = 0;
```

Side by side through the pinned pipeline, the struct-cast form emits one
`sll 4`; the halfword form emits the early `sll 3`, the mid-block `andi`, the
late base load and the `+N` displacements — instruction for instruction.

**Do NOT hoist the base pointer.** It stays late on its own. Three separate
attempts to front-load it (a `Rec *rec = &...[i]` at block top, an explicit
scalar `idx8`, a raw pointer-arithmetic rewrite) all made the match WORSE,
which is what made the residue look unreachable.

**REFINEMENT — the correct spelling depends on whether the call site is a
LOOP.** Confirmed by applying it to two functions in the same unit:

- **Loop call site** (`func_8002EDD4`): `u16 woff = (u16)i * 8;` computed
  eagerly as the loop body's first statement. Correct — the function went
  from 1 word short to **exact 270-word length, 267/270**, with the remaining
  3 words a single register-identity swap.
- **NON-loop call site** (`func_8002E4D8`): the eager `u16` spelling triggers
  a spurious zero-extension pair, because the parameter needs its own
  sign-extension for an unrelated 0x34-stride chain elsewhere in the function
  and the two extensions are not shared. There, keep `woff` typed **`s16`**
  and defer the `(u16)` mask to the point of use.

So the idiom is "truncated product indexing a halfword pointer" in both cases;
what changes is whether the truncation is eager or deferred, and a loop is what
makes eager correct.

**The general lesson, which is the part worth carrying: when a residue is a
MASK, look at WHICH VALUE is being truncated.** A mask on the index and a mask
on the product are different source constructs that compute the same address,
and reading one as the other sends you hunting for a missing statement that
does not exist.

### NARROW a `volatile` to the exact access that needs it — two independent confirmations (round 26)

**`volatile` is a scalpel, and applying it to a whole struct or every
participating field is what makes it look like it does not work.** Two
functions in different units confirmed this independently in one round.

**Case 1 — defeating GCC's div/mod fusion (`func_80035B2C`, `code_179d8_k`).**
GCC 2.6.3 fuses `q = A/B` and `r = A%B` on provably identical operands into a
single `div`, taking `mflo` and `mfhi`. Retail instead recomputes the whole
dividend and issues **two** `divu`. Probed through the pinned pipeline, none
of these break the fusion: a shared named temp, an intervening store, an
intervening call, or two independent pointer copies. Declaring the
participating fields `volatile` DOES break it — but applied to the sub-word
field it turns retail's plain `lh` into `lhu` + widen, which is why it was
first written off as unusable.

Applying `volatile` to the **word-sized field alone** un-fuses the division
*and* preserves the `lh`, because a `lw` has no load width to get wrong:

```c
volatile s32 *pbpm = &rec->unk8C;   /* only the lw-sized field */
q = (rec->unk4A * *pbpm * 10) / d;  /* rec->unk4A stays an ordinary read */
r = (rec->unk4A * *pbpm * 10) % d;
```

That single change took the function from 193/213 to **211/213**.

**Case 2 — forcing a cross-branch reload (`func_80052F10`, `class_3bb8c_l`).**
Retail keeps a reload that ordinary CSE elides. `*(void * volatile *)
&self->unk38` forces it, and the function matched at 137/137. This is the
already-documented fold-defeating idiom used in the OPPOSITE direction —
forcing a reload rather than permitting a fold.

**The transferable rule: when a qualifier-based lever "works but with a side
effect", check whether the side effect is intrinsic to the LEVER or an
artifact of WHERE it was applied.** Narrow the application before discarding
it. Rejecting a lever because one application of it had a side effect is not
the same as the lever failing — and the difference here was 18 words.

**Corollary, and this is what keeps it honest: this is not a licence for an
`__asm__` barrier.** The same round, a runner found that
`__asm__ __volatile__("" ::: "memory")` also reproduces retail's un-fused
shape, and correctly did not adopt it. It was refused: it is not the bare
`__asm__("")` HARD RULE 6 authorizes, it fits neither side of that rule's
register-identity-versus-order test (so authorizing it would be new policy,
not applying existing policy), and "barrier away any fusion retail did not do"
would become the standard answer to a whole class of residues. The plain-C
lever above is what closed 18 of the 20 words, which is the argument.

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

### The block-order rule extends to `switch` CASE order — SOURCE order, not value order (round 26)

**GCC 2.6.3 builds a `switch`'s compare tree from the SORTED case values, but
lays out the case BODIES in SOURCE DECLARATION ORDER.** So a sparse or dense
`switch` carries exactly the same block-order penalty as a misordered
if/else chain, and it is easy to miss because the compare instructions —
which most readers check first — can look identical between your build and
retail. It is the downstream branch OFFSETS that diverge, so it presents as
a mid-function word-match cliff rather than as a wrong instruction.

Runner alpha found this in round 26 and it was worth ~43 words across two
functions in `code_179d8_k`: `func_8003424C` went 92 -> 122/172 on it, and
`func_80034690` closed 13 of an 18-word gap.

**Read the rule carefully: SOURCE order, not ascending case-VALUE order.**
Alpha's summary said value order and its report said source order; both fit
alpha's data, because both its functions happened to be written in ascending
order and so could not discriminate. The head ran the discriminating case
through the pinned pipeline — cases written 30, 10, 20:

```c
switch (k) {
case 30: s30(); return;
case 10: s10(); return;
case 20: s20(); return;
}
```

Bodies come out `s30` @0x38, `s10` @0x48, `s20` @0x58 — **source order**. The
compare tree is meanwhile a binary search over sorted values (`beq 0x14` /
`slti 21` / `beq 0xa` / `bne 0x1e`), independent of how the cases are written.

**And it does NOT follow that every jump table needs reordering. Round 26
produced BOTH cases, from one runner, back to back — which is what turns this
from a lever into a checkable rule:**

| function | retail's case-body order | lever |
| --- | --- | --- |
| `func_80053984` (`class_3bb8c_l`) | coincides with ascending case value | **not needed** — matched 82/82 with a plainly ascending `switch`, first attempt |
| `func_800522DC` (`class_3bb8c_k`) | `25, 23, 5, 4, 18, 19` — neither ascending nor descending | **essential** — matched 69/69 first attempt once the cases were written in that exact order |

**So the discriminator is concrete and must be MEASURED per function, never
assumed in either direction: compare the jump table's own label order against
sorted case-value order.** If they coincide, an ordinary ascending `switch` is
correct and reordering buys nothing. If they do not, the file order of your
`case` labels must reproduce retail's table order literally.

Reading it the wrong way is expensive in both directions — assuming the lever
is always needed makes you reorder a `switch` that was already right, and
assuming it is never needed leaves a whole-function block-order residue that
no expression reshaping will touch.

### A two-return guard: write the SUCCESS return inside, the FAILURE return trailing (round 26)

Same family, different shape, and the existing block-order entry does not
cover it. `new_class_6d4e8` (`code_179d8_o`) compiled to the right LENGTH but
the wrong delay-slot value — `move v0,s0` where retail has `move v0,zero`.
The fix was purely which of the two logical returns is written first:

```c
/* matches: success return INSIDE the guard, failure return TRAILING */
if (self != NULL) {
    /* ... */
    return self;
}
return NULL;
```

The reverse arrangement cost **two extra words**, because GCC's cross-jump
pass would not merge that arrangement's epilogues. So on a guard with two
returns the choice is not stylistic: one order lets the epilogues merge and
the other does not.

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
3. **Dead-code elimination.** ~~`func_8002B94C`'s redundant-looking check is
   removed by the optimizer BEFORE scheduling runs, which no barrier
   placement can rescue.~~ **WITHDRAWN (round 43): the cited instance is
   Sony's `CD_newmedia`** (`lib/libcd/iso9660.o`, reclassified round 34), so
   no source shape of ours ever produced those bytes and this category has no
   surviving worked example. The general point that DCE runs BEFORE scheduling
   and is therefore barrier-proof is still true of GCC 2.6.3 -- and
   `func_80029C40` measured an instance of it independently, on game code, in
   round 35 (an `andi` mask after an already-zero-extending `lbu`). **Cite
   that one.**

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

> **INSTANCE WITHDRAWN (round 47, head). `func_80028C54` IS SONY'S** --
> converted to a linked SDK object in round 34, so it was never game code and
> the "matched 27/27" below was never a match of ours. Round 38's rule: an SDK
> exit voids a precedent retroactively and totally, because no source shape of
> ours ever reached those bytes. **What survives is the SHAPE as a hypothesis
> to test**, since the two escapes it extends are round 17's and are
> independent of this instance; what does NOT survive is any claim that this
> form was confirmed against retail by our pipeline. Nobody has demonstrated
> the third escape on a game function. Treat it as untested.

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

### The SDK-exit census, re-run over this document (round 45) -- and the one case where an SDK exit does NOT void the precedent

Round 43 ran Gate 1b's seventh screen over the shared docs for the first time
and withdrew one named learning. Round 45 re-ran it and found **four more
affected entries in this file, two of them named levers in the repertoire**.
They are corrected in place below; this entry records the METHOD and the one
refinement, because the class regenerates every time a batch of functions is
reclassified.

**The census, 2026-09-15.** 1089 functions have a match report and are no
longer a live `INCLUDE_ASM`. Splitting them by round 38's three exits:

```sh
grep -hoP '^INCLUDE_ASM\("[^"]*", \K[^,)]*' src/*.c | sort -u > /tmp/live.txt
ls docs/match-reports/ | sed 's/\.md$//' | sort -u > /tmp/reported.txt
comm -13 /tmp/live.txt /tmp/reported.txt > /tmp/closed.txt
while read f; do grep -qE "(^|[ *])${f}\(" src/*.c && continue
    grep -q "\b$f\b" build/lsdde.map && echo "RENAMED $f" || echo "SDK $f"
done < /tmp/closed.txt
```

**Do not stop at that split — resolve each SDK hit to its OWNING OBJECT by
ADDRESS**, which the name-based check cannot do and which is what makes a
correction citable:

```sh
# .text <addr> <size> build/lib/<lib>/<module>.o  -> bisect func_XXXXXXXX's vram into it
```

85 SDK exits, every one resolved: **libsnd 40, libcd 29, libgs 9, libc2 5,
libgte 2.** That concentration is itself worth knowing — `libsnd` and `libcd`
between them own 69 of 85, so a learning drawn from the sound driver or the CD
code is the one most likely to be citing Sony.

**THE REFINEMENT, and it is the reason to resolve by address rather than
trusting the rule: round 38's "no source shape ever reached those bytes" is
true for 84 of the 85, and measurably FALSE for one.** `func_8003FC70`
(`libgs/gs_108.o`) was matched **byte-exact as C** in round 20 and only
converted to a linked object in round 34. So for that one function the
whole-image oracle really did go green on real bytes, and the learning it
carries — the duplicated-rodata-string signature, in CLAUDE.md — stands on
that evidence rather than in spite of it.

So the disposition is three-way, not two-way, and the third case is rare
enough to be worth naming rather than screening for:

| what the SDK exit's report says | what a citation of it is worth |
| --- | --- |
| never matched (84 of 85) | **voided**, per round 38 — and doubly so for any "retail does X, our compiler does Y" claim, since those bytes came from Sony's ASPSX build and not our pinned pipeline |
| matched byte-exact, then converted (1 of 85) | **stands** — the oracle went green with real bytes; provenance is irrelevant to a mechanism the build system exhibits on its own |
| a mechanism confirmed by a standalone reproducer through the pinned pipeline | **stands regardless of the instance** — the reproducer does not care who wrote retail (round 43) |

**What round 45's pass actually cost the repertoire.** Two of the three
round-27 levers turned out to have zero game-code positives, and the round-20
`for`-loop idiom turned out to have zero game-code instances at all. None of
those was a weak entry: each was written up carefully, each carried measured
score deltas, and one was promoted specifically BECAUSE it transferred cleanly
to a sibling — a sibling in the same Sony object. **A clean transfer between
two functions is not evidence they are game code, and "it transferred" is
exactly the observation that makes a reader stop checking.**

### A `for`-loop keeps a status value register-resident where `goto`/labels folds it away (round 20) -- INSTANCE-LEVEL VERDICT WITHDRAWN (round 45)

> **ROUND 45 CORRECTION -- READ BEFORE APPLYING THIS.** Both functions this
> idiom was found and "confirmed" on are **Sony's**: `func_80028DF0` and
> `func_80028F38` are `lib/libcd/sys.o`, reclassified in round 34. So is
> `func_80029074`, cited two entries below. **This idiom has ZERO game-code
> instances**, and the "clean transfer" that promoted it from a one-function
> accident to a named idiom was a transfer between two modules of one Sony
> object.
>
> Two independent reasons the evidence does not carry, per round 43's rule:
> no source shape ever reached those bytes, so the derivation was never
> validated against anything; and the claim is of the form *"retail keeps the
> value register-resident and our compiler does not"*, which here compares
> GCC 2.6.3 against **Sony's own ASPSX build** rather than against our pinned
> pipeline. A negative rules out nothing and a positive proves nothing.
>
> **What survives:** "try the other loop spelling before concluding a residue
> is allocation" is a cheap thing to try and costs one iteration, so it stays
> in the repertoire below -- as an UNTESTED suggestion, not a confirmed idiom.
> **What is withdrawn:** the claim that GCC 2.6.3 behaves this way, the two
> score deltas, and the "confirmed by clean transfer" promotion. The first
> game-code instance is still unmeasured. This is the same class round 43
> withdrew the "DCE-eliminated always-true check" for, and it was found the
> same way -- by re-running the SDK-exit census over this document.

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
`func_80029074` (**Sony's `lib/libcd/sys.o`, reclassified round 34** -- the
caution stands, since it is a claim about the PERMUTER's scorer and not about
retail's provenance, but a search against bytes no source shape can reach was
guaranteed to produce exactly this, so treat the instance as an illustration
rather than as a measurement): a candidate scored 835 against a base of 1170 — a large
apparent gain — and verified *worse* against the real build, at 2/85 words with
the length grown from 85 to 89. bravo caught two more of these in the same
pass. The permuter scores in isolation and cannot see what the linked image
does; a score improvement is a hypothesis, and the real oracle is the only
test.

**2. A candidate that is SEMANTICALLY WRONG yet scores almost perfectly.**
delta, on `func_8003F848` (**Sony's `lib/libgs/gs_133.o`** -- but the MECHANISM
here is a fact about MIPS encoding, not about whose assembler wrote retail, and
it holds independently): 176/177 words, and incorrect C — it hoisted a call
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
entire fix transferred with no tuning at all -- though see the round-45
correction above: `func_80028DF0` and `func_80028F38` are two modules of one
Sony object (`lib/libcd/sys.o`), so this is a transfer WITHIN Sony's code and
is not evidence about game code. The counterweight below, echo's Entity pair,
IS game code and is the half of this entry that carries weight.

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
allocation-neutral.

**SCOPE, sharpened round 26: this is not a big-function effect.** The entries
above were all found in large bodies, which left open the reading that adding
or removing a named local only perturbs the ranking once a function is under
real register pressure. Runner charlie hit the identical renumbering in
`func_800569A8` (`class_3bb8c_s`) — **121 words**, far smaller than the
functions where it was first characterised — having already hit it in
`code_179d8_j`'s much larger bodies in the same round. Two confirmations at
opposite ends of the size range make it a general property of GCC 2.6.3's
allocator, not a symptom of pressure. So treat ANY change to the set of named
locals as potentially renumbering every callee-saved register in the function,
and re-measure total length after one — including in a function small enough
that it feels safe. Contrast the existing entries on the opposite direction
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


### Tooling defect found in round 27 (FIXED): funcdiff's window for a jump-table-owning function

`funcdiff.py`'s most precise range source read `asm/nonmatchings/**/<name>.s`
and took `min`/`max` over **every** instruction-comment offset in the file. A
function that OWNS a jump table has its rodata emitted into its own `.s` as a
leading `.section .rodata` block, and those data words carry the same
`/* fileofs vram word */` comment shape as instructions. The rodata slot sits
at a far LOWER file offset than the text, so the window spanned the gap:

```
func_800513D0: 65533/65533 words match (file 0x1E28-0x41E1C)   <- the bug
func_800513D0: 147/147 words match     (file 0x41BD0-0x41E1C)  <- fixed
```

**Two things made this worse than a wrong denominator, and the second is why
it is recorded here rather than shrugged off.**

- **It disabled the drift guard.** With a ~262KB window almost nothing is
  "outside this range", so the out-of-range byte count that
  CLAUDE.md's third way-a-score-lies depends on can no longer fire.
- **For a near-miss it prints a precise-looking ratio in which 3 words off and
  30 words off are indistinguishable.** A runner iterating on such a function
  reads a nonsense denominator with no indication anything is wrong.

The fix is to delimit by `glabel`/`endlabel`, which the *second* range source
in the same function was already doing correctly — so the bug was an
inconsistency between two sources in one function, not a missing idea.

**It is NOT a fifth way a score lies, and the reasoning matters.** CLAUDE.md's
four are live hazards in the oracle chain. This was a tool bug with a fix, and
it is fixed; adding it to that list would imply the reader must still guard
against it by hand. It is filed the same way round 19's tooling defects were.

**How it surfaced is the transferable part: nobody was misled, because the
runner did not use the number.** Delta sized `func_800513D0` from its
`nonmatching func_800513D0, 0x24C` header — the authoritative declared size —
rather than from a comment-line count or a funcdiff denominator, because the
assignment told it to. The head then noticed the absurd `65533/65533` while
re-verifying the merge. **The same root cause (rodata inlined in a function's
own `.s`, indistinguishable from text by comment shape) had already produced
two other bugs the same round** — an over-count in `tools/uncarved.py`
(`main`'s `func_80011994` reported as 49w against a real 2w) and a 178-vs-147
size discrepancy in an assignment. So: **when a `.s` can contain data, never
size or bound anything by counting its instruction-comment lines. Use the
declared `nonmatching <name>, 0xNNN` size, or delimit by
`glabel`/`endlabel`.**

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
  constructs that are hard to type.** See CLAUDE.md HARD RULE 6. An awkward
  unaligned struct copy does not qualify, and one was reworked into six lines
  of C this round after being matched as a transcription. If you cannot name
  the instruction that has no C spelling, it is not the exception. (Round 13
  named GTE `rtpt`/`nclip`/`cfc2` and COP2 `swc2`/`lwc2` as qualifying
  instructions; 2026-09-14 narrowed that further — each of those has a
  `gte_*` macro in `include/gte.h`, the macro is its C form, and the one
  function that had relied on them was rewritten as C. See the store-leaf
  entry near the top of this file.)

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
  check, `guard + do-while` is what expresses that (`func_800323A8` -- **a
  SONY object since round 33, so do not go read it as a worked example; the
  claim stands anyway because it was verified with a standalone toolchain
  reproducer through the pinned pipeline, which does not care who wrote
  retail's bytes**).
- **A loop-carried multiplicand must be recomputed from the loop counter, not
  accumulated with `+=`.** cc1 strength-reduces a constant multiply over an
  induction variable, so a `+=`-accumulated value produces the wrong shape;
  recomputing it by multiplication each iteration reproduces retail.
  (`func_800323A8` -- **a Sony object since round 33; the claim stands on its
  standalone reproducer, not on that instance**.)
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

  > **THE PROMOTING NEGATIVE CHECK IS ENTIRELY SONY'S CODE (round 47, head).**
  > All four functions it names were converted to linked SDK objects in round
  > 34. Apply round 43's split rather than throwing the entry away, because
  > the two halves land differently:
  > - **The READING HEURISTIC survives** — "a framed, `$ra`-saving,
  >   one-call-and-return body in retail is still `return callee(fwd-args);`"
  >   is a claim about how to read retail's bytes, and it does not care which
  >   assembler produced them.
  > - **The CODEGEN claim does not** — *"GCC 2.6.3 here always pays for the
  >   frame"* is a claim about OUR compiler, and every instance offered in
  >   support of it came out of Sony's ASPSX build. That is not a negative
  >   check of our pipeline; it is a survey of somebody else's. **The entry is
  >   promoted on evidence it does not have.** Re-earning it costs one
  >   standalone reproducer through the pinned pipeline, which nobody has run.
  >
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

  > **ROUND 44: THIS BULLET'S WORKED EXAMPLE IS SONY'S CODE.** `func_8002C048`
  > is `strcmp` (`lib/libc2/strcmp.o`), reclassified round 34. Retail's bytes
  > there came out of Sony's ASPSX build, not our pinned GCC 2.6.3 + maspsx, so
  > the specific mechanism claimed below -- that the lever "worked" and that the
  > defensive `andi` re-mask is its symptom -- is an INSTANCE-LEVEL verdict
  > derived from a comparison that was never our compiler against our compiler.
  > It is WITHDRAWN as evidence. Two halves survive untouched, and they are the
  > halves worth having: the PRINCIPLE *"prefer removing the thing being CSE'd
  > rather than reaching for `volatile`"*, which is a source-hygiene argument
  > that does not depend on any instance; and the round-23 carve-out below,
  > whose functions (`func_80032BF0`, `func_80032C28`) are genuine game code.
  >
  > **And note the direction, because round 44 found the opposite residue.**
  > This bullet is about our compiler CSE-ing away a reload retail KEEPS. Echo's
  > `func_8003E968` is the mirror: retail recomputes a `lui`/`addiu` address
  > pair TWICE, independently, back to back, and our GCC CSEs it to once --
  > coming out 2 words SHORT. `volatile` is not the tool for that direction and
  > neither is removing a cache; what is needed is to stop the two reads being
  > recognisable as the same object. That is an OPEN question on game code, and
  > it is the first instance of this class with a game-code stall attached.

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

### Round 27's HEADLINE: three levers were found, and ALL THREE have a measured counter-example

> **ROUND 44 CORRECTION -- TWO OF THESE THREE LEVERS HAVE NO GAME-CODE EVIDENCE
> AT ALL, AND THE TABLE BELOW DOES NOT SHOW IT.** Crossing this section against
> the SDK-exit census (Gate 1b's seventh screen, run over the shared docs rather
> than only over reports):
>
> - **address-taken parameter** -- its sole positive, `func_80050B28`, is Psy-Q
>   **libcard** and is marked `NOT GAME CODE` (round 39). Against game code the
>   table's own right-hand column records it **0 for 4**. Net game-code evidence:
>   zero successes, four failures.
> - **invert the guard** -- its sole positive, `func_80050AA4`, was **converted to
>   a linked Sony object in round 33**. Its only game-code test is the failure
>   already in the table (`func_8004EF6C`, 188/240 -> 6/240). Net game-code
>   evidence: zero successes, one catastrophic failure.
> - **inline every call site** -- **this row STANDS.** `func_800513D0` is genuine
>   game C, matched byte-exact at 147/147, and is still defined in
>   `src/class_3bb8c_i.c`.
>
> **Why an SDK positive is worth less than nothing here, rather than merely
> unproven.** Both levers are claims of the form *"retail does X and our compiler
> does Y"*. Sony's objects were built by **ASPSX, not by our pinned GCC 2.6.3 +
> maspsx pipeline**, so a lever that "worked" on those bytes was reconciling our
> compiler against a different assembler's output. It cannot be evidence about
> what our compiler needs, in either direction -- which is exactly why the two
> levers transfer so badly to the game functions people then tried them on.
>
> **Disposition:** the two rows are WITHDRAWN as levers. They are kept below
> because the section's CONCLUSION -- that a lever needs a stated discriminator
> or it is a coin flip -- is not only still right, it is now much better
> supported than when it was written. Do not spend attempts reproducing the two
> withdrawn positives. The address-taken discriminator (diff your prologue's
> saved-register list against retail's) is a mechanical test that survives
> independently of the instance it was found on -- use the test, not the claim.

**Read this before applying any lever from round 27.**

> **ROUND 45 CORRECTION: TWO OF THE THREE POSITIVES IN THE TABLE BELOW WERE
> NEVER GAME CODE, SO TWO OF THE THREE LEVERS HAVE ZERO GAME-CODE POSITIVES.**
>
> - **Address-taken parameter.** Its only positive, `func_80050B28`, is Psy-Q
>   **libcard** -- marked `NOT GAME CODE` in its own report since round 39,
>   proved by segment topology and the `addiu`/`ori` assembler fingerprint.
>   Its four negatives are all genuine game code. So the row reads **0 for 4**,
>   not 1 for 5.
> - **Invert the guard.** Its only positive, `func_80050AA4`, is
>   `lib/libc2/todigit.o`, reclassified round 34. Its negative,
>   `func_8004EF6C`, is game code. So this row reads **0 for 1**.
> - **Inline every call site** is INTACT: `func_800513D0` and `func_8004F8A4`
>   are both game code, and the row stands exactly as written.
>
> Why an SDK positive is worth less than nothing here rather than merely
> unproven: those bytes came out of **Sony's ASPSX build**, not our pinned
> pipeline, so "reshaping the C made our output match retail" was never a
> statement about GCC 2.6.3 -- and no source shape could ever have reached
> them anyway. A lever whose every positive is Sony's has not been shown to
> work on this project at all.
>
> **What this does NOT do is retire the levers.** The discriminators stated
> below the table are mechanical tests (diff your prologue's saved-register
> list against retail's; check the arms are actually swapped before inverting
> polarity) and a mechanical test survives the instance it was found on --
> that is this document's own rule, stated two paragraphs above the table.
> Use the tests. Do not cite the positives.
>
> And note what the correction does to the round-27 lesson itself. The lesson
> was *"each of these closed a real function and then made a different one
> worse, so none is a rule"*. The truer version is sharper: **two of them never
> closed a game function in the first place**, and their apparent successes
> were measured against an assembler we do not run.

The round produced three
source levers, each of which closed or advanced a real function, and each of
which was then measured to make a *different* function WORSE. None is a rule;
all three are hypotheses to test per function. This is the same shape as round
7's two superficially symmetric vtable slots that needed opposite answers, and
it is the most useful thing the round established.

| lever | where it WORKED | where it made things WORSE |
| --- | --- | --- |
| **address-taken parameter** forces retail's home-slot spill instead of `$s0` promotion | `func_80050B28`: 2-words-long-with-drift -> exact length | **0 for 4** on every other candidate (delta on `func_8004B700`, `func_8004BB3C`; alpha on `func_800351D0`; bravo on `func_8002E4D8`) |
| **invert the guard** so the expensive arm falls through | `func_80050AA4`: 11/25 and one word short -> **25/25 byte-exact** | `func_8004EF6C`: **188/240 -> 6/240**, catastrophic. The guard was ALREADY correctly polarized; `slt`/shift replaced retail's `bne` |
| **inline every call site** rather than factoring a shared tail | `func_800513D0`: 9-words-short-with-drift -> **147/147 byte-exact** | `func_8004F8A4`: **4 words too long**. Same runner, same header family, same round |

**The two rows that matter most are the second and third, because in each the
same person applied their own successful lever to a neighbouring function and
it went backwards.** Delta found the inline-call lever on `func_800513D0` and
had it fail on `func_8004F8A4`. The head found guard inversion on
`func_80050AA4` and had delta measure it destroying `func_8004EF6C`. Proximity,
family, size and residue *description* all failed to predict transfer.

**So each lever needs its DISCRIMINATOR stated, or it is a coin flip.** What is
known:

- **Address-taken parameter** — has a real discriminator, and it is one
  command: diff your compiled prologue's saved-register list against retail's.
  Retail saves FEWER than you -> applies. SAME set -> genuine register
  identity, stop. NONE on either side -> cannot apply, look elsewhere. Given
  it went 0 for 4, its realistic value is **confirming** identity verdicts,
  not producing matches.
- **Invert the guard** — discriminator NOT established. What is known is the
  failure mode: it destroyed a function whose guard was already correct, by
  changing the *comparison* (`== -1` -> `< 0`) rather than only the arm order.
  So **check first that the arms are actually swapped relative to retail**, and
  invert the polarity WITHOUT changing the comparison operator. Do not reach
  for it on a residue already attributed to register allocation or delay-slot
  placement.
- **Inline every call site** — discriminator NOT established, and this one has
  the tightest counter-example of the three. Both functions are class-framework
  dispatchers in the `class_3bb8c` family. The only visible difference is that
  `func_800513D0`'s duplicated blocks are byte-identical *case bodies* GCC
  cross-jump-merged, whereas `func_8004F8A4`'s shared tail is genuinely shared
  in retail too. **Check whether retail duplicates the block before you
  duplicate it** — read the `.s` for two copies, do not infer from being short.

**The generalisable rule, and it is a process rule:** a lever discovered on one
function is evidence about *that* function until a second function confirms it.
Broadcasting one mid-round is still right — it is cheap and it found all three
counter-examples above within hours — but **broadcast it as a hypothesis with a
request for the negative**, which is what produced this table. A lever
broadcast as a rule would have had four runners applying three coin flips.

**And record the negatives in the reports, not just the positives.** Every row
in the right-hand column above exists because a runner was told that a
mechanised negative was a wanted result. `func_8004EF6C`'s report now says
"arm polarity regressed this 188/240 -> 6/240, the guard was already correct,
the residue is register-allocation-driven delay-slot placement" — which is
worth more to the next round than another attempt would have been, because it
removes an axis instead of adding an attempt.

### Round 27: two source levers for a residue that looks like register identity

Both were found by the head on 12-to-25-word functions in `class_3bb8c_u` /
`class_3bb8c_v`, each backed by a byte-exact match or a measured length fix,
and both were broadcast mid-round and answered with negatives by two runners.
They are grouped because they share a failure mode: **each produces a diff that
reads as register identity or as an unreachable encoding choice, and each is in
fact reachable from the source.**

#### 1. An incoming PARAMETER that survives a call has two retail shapes

GCC 2.6.3 promotes such a parameter to a **callee-saved register**, costing a
save/restore pair and 2 words of length:

```
addiu sp,sp,-0x18 / sw s0,0x10(sp) / sw ra,0x14(sp)
jal callee1 + move s0,a0         <- parked in $s0
move a0,s0 / ... / jal callee2
lw ra,0x14(sp) / lw s0,0x10(sp) / addiu sp,sp,0x18
```

Retail sometimes instead spills it to **its own incoming home slot** and
reloads it, using no callee-saved register at all:

```
addiu sp,sp,-0x20 / sw ra,0x1c(sp)
jal callee1 + sw a0,0x20(sp)     <- spill to the HOME SLOT, in the delay slot
lw a0,0x20(sp) / ... / jal callee2
lw ra,0x1c(sp) / addiu sp,sp,0x20
```

**Taking the parameter's address forces memory residency and switches GCC to
the home-slot form**, at retail's exact length:

```c
s32 f(s32 chan) {
    s32 *p = &chan;
    callee1();
    return callee2(*p, 0x3F, 0);
}
```

Measured on `func_80050B28` (12 words): default form 2 words LONG with drift;
address-taken form **exact length, correct instruction sequence, 5/12 words**.
A bare `__asm__("")` was **INERT** at both positions tried.

**THE SIGNATURE, and it is narrow:** apply it when the build is **LONGER** than
retail and the excess is a `sw`/`lw` pair on a callee-saved register **retail
does not save at all**, around an incoming parameter. Two words per promoted
parameter.

**THE DISCRIMINATOR against genuine register identity — this is the part that
matters, and it is one command.** Diff your compiled prologue's saved-register
list against retail's:

- retail saves **FEWER** callee-saved registers than your body -> structural,
  this lever applies, keep going.
- retail saves the **SAME** set and merely uses different registers for the
  same values -> genuine register identity. Stop; it is banned to fix.
- retail saves **NONE and neither do you** -> the lever cannot apply at all
  and the residue is somewhere else entirely. Runner bravo added this third
  outcome, which the two-way test above does not name: on `func_8002E4D8` it
  grepped for `sw $s0`-`$s7`/`$fp` in both retail and its own standalone-built
  body and found **zero in either**, so there was no promotion to undo. Check
  for the instructions; do not infer them from a frame-size difference.

**FOUR APPLICATIONS ACROSS THREE RUNNERS THIS ROUND, ALL NEGATIVE. Record that
plainly: the lever is real -- it took `func_80050B28` from 2-words-long-with-
drift to exact length -- and it transferred to NOTHING else in the round.** Its
scope is narrower than the "compiled LONG" signature suggests.

- delta, `func_8004B700` (125/140) and `func_8004BB3C` (90/105): retail saves
  the identical set at identical offsets. **Both verdicts upgraded from
  plausible to checked.**
- alpha, `func_800351D0` (4 words long): same 10 callee-saved registers as
  retail; residue is really a `$s5` rematerialization.
- bravo, `func_8002E4D8` (6 words long): zero callee-saved saves on either
  side; residue is a redundant sign-extension (`u16`-vs-`s16` accumulator
  compare), unrelated to register pressure.

So the lever's realistic value in a round is **confirming register-identity
verdicts rather than overturning them** — which is worth having, because a
confirmed verdict stops the next round re-litigating it, but it is not a source
of matches. Do not staff a round on the expectation that it will be.

**The sibling failure to avoid: a residue class is a property of the FUNCTION'S
OWN SHAPE, not of its neighbourhood.** `func_80050B28`'s report predicted its
8-word sibling `func_80050A84` would show the same residue, on the grounds of a
shared call chain, unit and size. It has no parameter, so the mechanism cannot
arise, and its delay-slot `nop` says so one grep away. Sharing a chain, a unit
or a carve predicts nothing about sharing a residue.

> **ROUND 46 CORRECTION: SECTIONS 2 AND 3 BELOW REST ENTIRELY ON SDK-EXIT
> INSTANCES, AND ROUND 45'S SWEEP DID NOT REACH THEM.** Round 45 corrected the
> lever TABLE above and stopped at the table. Everything from here to the end
> of section 3 is measured on `func_80050AA4` (`lib/libc2/todigit.o`,
> reclassified round 34) and `func_80050A84` (`atol`, same library) -- both
> Sony's, neither ever game code. That is the round-43 finding arriving one
> subsection later than anyone swept: **the family is the unit of staleness,
> and a correction box is not a fence.**
>
> Rather than annotate and move on, the head put both claims through the
> pinned pipeline on **game-neutral code that mentions retail nowhere**. That
> is the test round 43 prescribes -- a mechanism verified by reproducer
> survives an SDK exit, because the reproducer does not care who wrote
> retail's bytes; an instance-level verdict does not. Results, one row per
> claim:
>
> | claim | verdict |
> | --- | --- |
> | guard polarity flips WHICH ARM FALLS THROUGH (section 2's headline) | **CONFIRMED** |
> | the flip also costs/saves a word, so a 1-word-short two-armed function should be tested for arm polarity FIRST (section 2's corollary) | **WITHDRAWN** |
> | a bare `nop` in a call's delay slot means the callee took no argument (section 3's headline) | **FALSE AS STATED** |
> | ...unless the argument was already live in `$a0` (section 3's caveat) | **CONFIRMED, and it is the whole rule** |
>
> **Section 2's headline holds.** Two forms of the same two-armed function,
> differing only in guard polarity, put opposite arms in the fallthrough
> position: the constant-return arm falls through when the guard tests the
> expensive condition, and the call path falls through when the guard is
> inverted to an early return of the constant. Use it.
>
> **Section 2's COROLLARY does not, and it is the half most likely to be acted
> on.** Both forms came out at **15 words**. The layout flip is length-neutral
> here, so "the missing word may be a delay-slot `nop` only the correct layout
> leaves empty" does not follow from the flip -- it was a fact about that one
> libc2 function's surroundings. Inverting a guard to chase a one-word length
> gap is not supported; invert it when your ARMS are demonstrably swapped,
> which is what the section title already says.
>
> **Section 3's headline is false without its caveat, and the caveat is not a
> footnote to it -- it is the rule.** Three calls through the pinned pipeline:
> an argument the callee does not already hold puts the setup in the delay
> slot (`move a0,a1`); an argument **already live in `$a0`** emits a bare
> `nop`; a genuinely argument-less call emits a bare `nop`. The last two are
> **byte-identical**, so a bare `nop` does not distinguish them and cannot on
> its own establish arity. Always check whether `$a0` already holds the value
> first. Section 3 states this as a caveat "hit immediately afterwards"; it is
> the discriminator, and without it the lever mis-types functions in exactly
> the way its own last sentence warns.
>
> Reproducers are three and six lines; re-run them rather than trusting this
> box. **And note the recipe in CLAUDE.md's "Escalate, do not experiment" does
> not work as written in this environment's shell:** it word-splits
> `$(sed -n 's/^MASPSX_FLAGS...' Makefile)` into maspsx's argv, which `sh` and
> `bash` do and **zsh does not** -- under zsh the whole flag list arrives as
> one argument and maspsx dies parsing `--aspsx-version=2.34 --dont-force-G0
> ...` as a version number. The failure is loud, so it costs a minute rather
> than a wrong result; run the recipe under `bash -c`, and keep reading the
> flags from the Makefile rather than retyping them (round 43).


#### 2. When your arms come out swapped, invert the GUARD, not the structure

This sharpens the existing "a two-armed `if`/`else`'s LAYOUT and its
VALUE-PER-ARM are independently wrong-able" entry with the **source change that
actually flips the layout**, plus two that do not.

GCC 2.6.3 chooses which arm falls through partly on arm cost: it will inline a
short constant-return arm as the fallthrough and branch to a call-bearing arm.
Retail frequently does the reverse. Writing the **cheap arm as an early-return
guard** and the **expensive arm as the fallthrough** flips it:

```c
    /* 11/25, and ONE WORD SHORT -- arms inverted vs retail */
    if (flags & 3) { return (call(c) & 0xFF) - 0x57; }
    return 0x98967F;

    /* 25/25 byte-exact -- sentinel first, call path falls through */
    if (!(flags & 3)) { return 0x98967F; }
    return (call(c) & 0xFF) - 0x57;
```

Measured on `func_80050AA4`. **Two things one instinctively reaches for are
INERT** — rewriting it as an explicit `else if`/`else` chain, and hoisting the
arm's expression into a named local. Both left the layout exactly as it was.
Only the guard's polarity moved it.

**Corollary worth its own line: this presented as "1 word SHORT", and the
missing word was a `nop` in a delay slot that the wrong layout let GCC fill.**
So a one-word-short residue on a two-armed function is worth testing for arm
polarity **before** anything else — the missing word may be a delay-slot `nop`
only the correct layout leaves empty.

#### 3. A bare `nop` in a call's delay slot is evidence about the callee's ARITY

GCC fills a call's delay slot with argument setup whenever there is any to do,
so a bare `nop` there means there was none. That distinguishes
`return callee();` from `return callee(arg);` by inspection — and those two
compile to *different bytes*, unlike the return-type question in the same
position, which compiles to identical bytes and cannot be read off at all
(CLAUDE.md's existing wrapper rule).

**Caveat, hit immediately afterwards in the next function along:** it only
proves that when the argument is **not already in the right register**. In
`func_80050AA4` the delay slot is also a `nop` and the call *does* pass its
argument — `$a0` already held it after an earlier `andi`, so GCC had nothing
to emit. Check whether `$a0` is already live with the value before concluding
the call is argument-less. Used correctly it settled `func_80050A84`'s
signature (`void`, byte-exact first attempt); used carelessly it would have
mis-typed `func_80050AA4`.

### Round 27: the register COUNT and the addressing-mode IMMEDIATE are ONE axis -- and a named local is NOT the lever for it

**This entry exists because the head proposed a mechanism, a runner tested it
exactly as specified, and it was wrong.** The correction is more useful than
the hypothesis was, and it bounds the SCOPE of the allocno entry above --
which the head had over-generalised.

`func_8002BCEC` (`code_179d8_d`, 175 words) came out 3 words short, with one
struct-field write whose `swl`/`swr` immediates were `7`/`4` where retail has
`3`/`0`. Runner charlie originally filed it as register identity. **The head
overturned that verdict, correctly**: charlie's own description had one form
using **8** callee-saved registers and the other **7**, and a differing
register COUNT is not identity -- identity is the same allocation with
different names. So CLAUDE.md's identity ban does not apply and the function
must not be marked "do not try".

**The head then proposed the wrong reason for it.** Reasoning from the allocno
entry above, it argued the 8-register form came from `UWord *sizeField = ...`
creating a new allocno, and that an inline cast expression with no named
pointer would be the untried third combination that got both right.

Charlie tried that exact form and then settled the question with six measured
variants:

| destination spelling | callee-saved regs | `swl`/`swr` immediates |
| --- | --- | --- |
| `*(UWord*)((u8*)base+off+4)` -- the head's suggested inline cast, **no named pointer anywhere** | 8 (extra) | `3`/`0` **(right)** |
| `UWord *sizeField = ...; *sizeField = ...` (new named local) | 8 (extra) | `3`/`0` (right) |
| `slot->size` -- reusing an **already-live** pointer, **zero new names** | 7 (right) | `7`/`4` (wrong) |
| `*(UWord*)((u8*)slot+4)` -- same reused pointer, raw-cast spelling | 7 (right) | `7`/`4` (wrong) |
| `EntryB3F0 *base = D_8008B3F0;` then `*(UWord*)((u8*)base+off+4)` | 8 (extra) | `3`/`0` (right) |

**Row 1 and row 2 are the same outcome, so "no named pointer" was not a new
combination. Row 3 is the decisive one: the same address computed through an
ALREADY-LIVE pointer, introducing no name at all, still gets the WRONG
immediates.** So introducing a name is not what causes the wrong immediates
and removing one is not what fixes them.

**What the rows do show, and it is sharper than either hypothesis: the
register count and the immediate split are ONE axis, not two knobs.** Either
the address is materialised fresh into its own register (right immediates, one
extra register) or it is folded into the store's immediate (right count, wrong
immediates). Retail's shape -- an extra `addiu` into a **scratch** register,
costing no extra *persistent* register -- was reached by none of the six. That
is the third shape a future attempt needs, and it is a real open residue.

Two immediately reusable by-products:

- **Rows 3 and 4 are byte-identical output.** Struct-field access and raw
  pointer arithmetic compile the same once the base register is the same, so
  **re-spelling one as the other is never a lever** -- it retires a whole
  class of guesses that look like distinct attempts.
- **The allocno entry's scope is now bounded.** "A new name creates a new
  allocno competing for a callee-saved register" is real and has closed
  functions. It does **not** govern how GCC materialises an address for a
  store. Do not reach for it on an addressing-mode residue.

**The generalisable process point, since this is the second time in one round
a confident mechanism was wrong:** the head's verdict correction (not identity)
was right and its mechanism (allocno) was wrong, and those are separable
claims. A runner told "your class is wrong, and here is why" should test the
*why* and report back when it fails -- charlie did, with a table, and the
result is a bounded open residue instead of a false lead the next round would
have re-run. Ask for the mechanism to be tested, not accepted.

### Round 27: the DCE-eliminated always-true check gets a structural hypothesis (and two axes closed)

> **VOID AS A GAME-CODE CLASS -- round 43, head. Read this before the section
> below.** Both of the "two confirmed instances" are **Sony's**:
> `func_8002B94C` is `CD_newmedia` and `func_8002BCEC` is `CD_cachefile`, both
> linked from `lib/libcd/iso9660.o` and reclassified in round 34. The class
> therefore has **zero game-code instances**, and every structural hypothesis
> below is a hypothesis about what SOURCE EXPRESSION produced bytes that were
> never compiled from our source at all -- they came out of Sony's own ASPSX
> build. "Retail keeps a check our compiler removes" is not even the right
> comparison here: it compares our pinned pipeline against a different
> assembler's output, so a negative result below rules out nothing about GCC
> 2.6.3 and a positive one would prove nothing either.
>
> **What survives is the METHOD, not the finding.** Round 27's reasoning --
> that three spellings sharing one local are one attempt, not three; that a
> global-sourced condition survives DCE where a local-sourced one does not --
> was sound, and the global-vs-local observation was verified with a
> standalone reproducer through the pinned pipeline, which is what makes that
> half independent of who wrote the bytes. Keep the reproducer-backed claim;
> do not carry any instance-level verdict forward, and do not treat this
> section as precedent for a residue you meet in game code.
>
> This is round 38's SDK-exit rule applied to a SHARED DOC rather than to a
> match report, which is the gap round 43 found: the seventh screen in
> `docs/PARALLEL-RUNS.md` crosses closed functions against other REPORTS and
> never looked at this file -- the one file every runner is told to read.

The class -- retail keeps a check GCC's dead-code elimination removes -- now
has **two confirmed instances in sibling CD functions**: `func_8002B94C`
(`CD_newmedia`) and `func_8002BCEC` (`CD_cachefile`). Round 27 closed two of
its axes and produced the first structural lead.

`func_8002B94C`'s report already concluded the original source expression
"must not have been immediately foldable to a literal by this compiler". The
head pushed on that: charlie's three prior attempts (`while(1)`,
`if(ok==1)`, `while(ok==1)`) all used a **local** whose value GCC can trace,
so they were one attempt in three spellings, not three attempts.

Two new axes, both measured, both negative:

- **A global-sourced condition DOES survive DCE** -- confirmed, so the
  premise is right -- but produces the **wrong instruction shape**: a real
  `lw`+`slti`+`bnez` where retail has `ori`/`beqz`/`nop`.
- **A call-sourced condition is ruled out structurally, without a build.**
  Retail's window is exactly three instructions (`ori`/`beqz`/`nop`) and
  there is no room in it for a load or a call. That is the right way to kill
  an axis -- read the space available, rather than compiling a guess.

**The structural lead, which is the round's actual contribution here:** in
BOTH instances the dead check sits **immediately after a conditionally-executed
debug print** -- i.e. at a **branch-merge join**, not in straight-line code.
That makes a join-crossing DCE limitation a plausible trigger: GCC's
elimination may be weaker across a join than within a basic block, so retail's
compiler kept a check that any straight-line reproduction loses. Untested, and
worth checking on a **third** instance before believing it.

So the axes now stand: not a scheduling residue (barriers are inapplicable in
principle, since the question is whether a branch EXISTS); not reachable from a
traceable local; not from a global (survives, wrong shape); not from a call (no
room). What is untried is reproducing the **join** rather than the condition.

**THE CLASS HAS TWO INSTANCES, NOT MORE -- a corpus search for a third came
back EMPTY, and the head's first version of that search was WRONG in a way
worth recording.** Charlie asked for a third instance to test the join
hypothesis on. The head searched every `.s` for the signature -- an
`ori $R, $zero, K` immediately followed by a branch testing `$R` -- and got
four hits: the two known instances plus `func_80059BE0` (`DreamSys`) and
`func_8001E110` (`code_d294_b`). **Both extras are false positives, and both
for the same reason: the search paired FILE-ADJACENT instructions without
respecting delay slots and label boundaries.**

- `func_8001E110`: the `ori $v0, $zero, 0x1` sits in the **delay slot of a
  `j`** to the epilogue -- it is a RETURN VALUE, not a condition. The branch
  the search paired it with is at a label (`.L8001E178`) reached from an
  earlier branch entirely. The two never execute in sequence.
- `func_80059BE0`: the `ori $s1, $zero, 0x1` is the last instruction before a
  join label, and `$s1` is set to `sltiu $v0, 0x1` on the *other* incoming
  path. So `$s1` is a genuine **phi** -- 1 on one path, computed on the other
  -- and the `beqz $s1` after the join is NOT an always-known branch. It only
  looks like a constant feeding the next branch because of where the join
  falls.

**The lesson is the one this round had already written down twice, arriving on
its author: a constant written just before a join label is a PHI INPUT, not a
dead check, and an instruction in a delay slot does not precede the next line
in execution order.** Any search for "branch on a just-materialised constant"
must (a) exclude an `ori` sitting in a branch's delay slot and (b) confirm no
label falls between the constant and the branch. Reading a disassembly in
source order gets delay-slot placement wrong -- which is exactly what
`docs/MATCHING-GUIDE.md` says about derivations, and it applies to corpus
searches identically.

So the honest state: **two confirmed instances, both in sibling CD functions,
and no third exists in the corpus by this signature.** That makes the class
rare, keeps the join hypothesis untestable for now, and means the next
instance should be looked for by its STRUCTURE (a check surviving at a
branch-merge join) rather than by this instruction pattern.

### Round 27: an UNMOVED score can mean two symptoms of one defect traded places

**This is the round's subtlest finding and it belongs next to "the four ways a
score lies", because it is a fifth way a *change* lies rather than a way a
score lies.** Runner bravo, `code_2cc8c_f`.

`func_80040FC0` sat at 15/24 before and after a source change, which reads as
"the change did nothing". It did not. Toggling the reused byte's local width
between `u8` and `u32` **trades one word-cost for another**:

- as `u8`, a spurious `andi` mask appears (one word wrong);
- as `u32`, the mask goes away and a *different* word goes wrong — the same
  missing "redundant cursor cache" register its sibling `func_80041020` hit.

Net score identical, two different defects. The reading "inert lever, move on"
would have discarded a correct change and left the axis unexplored.

**Why it matters beyond this function: it is what proved the unit's three
stalls are ONE defect, not three.** `func_80041020`, `func_80040FC0` and the
cursor-register residue had been filed separately across rounds. Isolating the
trade showed the same underlying cause behind all of them, which is worth far
more than either individual score.

**The habit: toggle one variable at a time and read the DIFF, not the score.**
A word-count is a scalar summary of a vector; two changes of opposite sign
cancel in it. When a change you have good reason to believe in leaves the score
unmoved, run `tools/asm-differ/diff.py` and check whether the *set* of
differing words changed. If it did, the lever worked and uncovered a second
defect.

### Round 27: a byte value reused across comparisons needs a WIDER local than the byte

Two independent confirmations, both bravo, in two different units — so this is
a lever rather than an anecdote.

- First pass, `code_179d8_m`: `chan = call() & 0xFF` must stay `s32`, not `u8`,
  to get retail's `slt` rather than `sltu` on a later comparison.
- Second pass, `code_2cc8c_f`/`func_80041020`: a byte value reused across
  several comparisons needs a `u32` local, not `u8`, or GCC emits a spurious
  `andi` mask at each use.

**The rule: declare a narrow value in a local as wide as the register it lives
in, even when every value provably fits in a byte.** `-funsigned-char` means a
`u8` local is a real narrowing GCC re-materialises at each use; retail's
compiler was given a wider declared type. This is the same family as the
existing `s16`-width entries, arriving from the comparison side.

**And note it is NOT free** — see the entry above. On `func_80040FC0` widening
the type removed the `andi` and exposed a different missing word, so the score
did not move. Widen it *and* read the diff.

### Round 27: two branches with the same result need a combined `&&`, not nested `if`s

Also bravo, `func_80041020`, pipeline-isolated. Where two logically-distinct
false outcomes share one output formula, retail reaches that formula as a
**single fallthrough instruction**. Nested `if`s duplicate it behind a jump; a
combined `&&` condition shares it. This was one of the two changes that took the
function from the wrong length to exact length, so it is worth reaching for
whenever a body is LONG by roughly the size of a duplicated tail.

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

- > **CLASS WITHDRAWN (round 47, head). ALL THREE INSTANCES ARE SONY'S** --
  > `func_80028DF0`, `func_80028F38` and `func_80029074` were all converted to
  > linked SDK objects in round 34. This named class therefore has **zero
  > game-code instances**, which is exactly the failure round 43 found in the
  > "DCE-eliminated always-true check" learning. The ~90 manual attempts and
  > ~70k permuter iterations recorded below were spent on Sony's code.
  > **The keep/withdraw split (round 43):** residue 1's *reproducer* survives
  > — it was run through the pinned pipeline on a from-scratch shape and does
  > not care who wrote retail's bytes, and its finding ("the divergence is not
  > a toolchain defect") still stands. Residue 2's prologue register-mapping
  > puzzle does NOT: it compares our output against **Sony's ASPSX build**,
  > not against our compiler's target, so it rules out nothing about GCC
  > 2.6.3. Do not cite this cluster as precedent for a game function.
  >
  **NEW, round 16: the "retry-loop driver" cluster — three instances of ONE
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

- > **CLASS WITHDRAWN (round 47, head). ITS ONLY INSTANCE IS SONY'S** --
  > `func_800323A8` was converted to a linked SDK object in round 33. This is
  > round 43's SHARPER case, and the sharpness is the point: the claim is
  > *"retail encodes `addu $v0,$a0,$v0` where our build encodes
  > `addu $v0,$v0,$a0`"*, which compares our pinned pipeline against **a
  > different assembler's output**. A negative there rules out nothing about
  > GCC 2.6.3 and a positive would have proved nothing either. "Sixteen
  > confirmed instances" is sixteen instances inside one Sony function, not
  > sixteen functions. **No game function has ever exhibited this.** If you
  > meet an `rs`/`rt` slot swap in game code, it is a new finding and needs
  > its own standalone reproducer — not a citation of this entry.
  >
  **NEW, round 16: commutative-operand SLOT order in `addu`, not reachable
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

## Round 31 (2026-09-11) — five closes, and the mechanisms behind them

Five functions closed in `code_179d8_k` this round and four of the five came
from three distinct source-shape levers. All were found against the pinned
toolchain and verified by the whole-image SHA1, not by a per-function score.

### Cache the SCALAR a pointer is built from, not the pointer — closed THREE

A "one word short" residue in the register-rescue class is often a
POINTER-CACHING artifact. The C holds a persistent pointer across several
uses; retail recomputes the address expression at each use site from a cached
byte OFFSET. Rewriting it that way — keep the raw offset in a local,
recompute `base[offset]`-style at each of the use sites — closed
`func_80034AEC`, and the same edit was a component of `func_800349B0` and
`func_80034C28`.

This is the highest-yield single lever found in several rounds, because the
"one word short" shape is common and its raw word-match is usually wrecked by
shift ripple, which makes it LOOK far away when it is one edit away. When you
see one word short plus a low raw match, try this before anything expensive.

`func_800349B0` needed one further separable fix: shrink the scratch struct's
TRAILING padding to correct the frame size while keeping the field's own
offset unchanged. Frame size and field offset are independent knobs; changing
the field offset would have been the non-local struct edit that breaks
already-matched siblings.

### Defeat 2.6.3's constant canonicalization by routing the constant through an assignment

GCC 2.6.3 strength-reduces some immediates — a `0xC0` became `-0x40`.
Assigning the constant to a separate local first, then using that local,
suppresses the canonicalization and emits retail's form. Closed the last word
of `func_80034C28`.

### A delay-slot filler that "looks dead" may be a WRITE scoped wrong in your C

MIPS delay slots always execute. So a store sitting in retail's delay slot
runs on BOTH branch outcomes, and if your C nests that assignment inside one
arm of the conditional, you get a scheduling-shaped diff that is really a
SCOPE bug. `func_80034F90` was misdiagnosed in round 25 as a scheduling
residue on exactly this; hoisting `rec->unk16 = a2` out of the arm and up to
the top of the whole `case` closed it at 82/82.

**The discriminator is whether the delay-slot instruction has a MEMORY
EFFECT.** A store or a `sb`/`sh`/`sw` can hide an unconditional write and is
worth re-scoping. A bare register-to-register move cannot hide anything, so
for that the scheduling explanation stands — which is what re-confirmed
`func_80034138`'s verdict in the same unit and the same sitting. Two
superficially identical "dead filler" residues, opposite answers, and the
memory effect is what separates them.

### Independent global stores are freely reordered — a reorder can close a LENGTH gap

`func_80031A44` (`code_179d8_j`) had been one word short through its entire
recorded history. Swapping two adjacent independent global stores — writing
`D_8008EA26 = idx;` BEFORE `D_8008EA22 = 0x21;`, which is the opposite of the
order retail's own instructions suggest — closed the length gap outright:
87/88 -> **88/88 exact**, raw 41/88 -> 84/88.

The general point: GCC 2.6.3 reorders stores to independent globals as it
likes, so retail's instruction order is NOT evidence of the source order, and
"my C already matches retail's apparent order" is not a reason to leave the
pair alone. It is a cheap axis and it can move LENGTH, not just placement.

## Permuter practice (round 31)

### `--stack-diffs` is MANDATORY on a frame or offset residue, or you get a FALSE ZERO

Without it the permuter's scorer normalizes stack-offset differences away, so
a candidate that differs from retail only in where things sit in the frame
scores ZERO while being wrong. Round 31's echo hit exactly this on
`func_80050B28`, caught it, and re-ran correctly (~80k iterations, best score
8, gap isolated to one 4-byte slot: `ra` at `0x18` against retail's `0x1c`).

A false zero is the expensive kind of wrong: a zero is treated as a LEAD to
translate into C, and translating a false one burns an attempt budget proving
that the oracle disagrees with a search that was never measuring the right
thing. Pass `--stack-diffs` whenever the residue touches the frame at all.

### A permuter candidate's verification must match whether it changes BEHAVIOUR

- Behaviour-CHANGING candidate (control flow, trip count): trace it by hand.
  Inspection of the score cannot tell you it is invalid.
- Behaviour-PRESERVING candidate: real-build-verify it. Inspection alone can
  miss a length regression that `--stack-diffs` does not surface.

Round 31 added a **third** confirmed instance of "permuter score improves, the
real oracle regresses" (alpha, `func_800662BC`: a behaviour-preserving
candidate that verified at 19/33 WITH drift against a 17/33 baseline). Treat a
permuter improvement as a hypothesis until the whole-image build agrees.

Round 31's echo also found a best-scoring candidate on `func_80050948` that
was semantically INVALID, and flagged it in the report so a later round does
not mistake the score for progress. Do that — an unflagged high-scoring
invalid candidate reads exactly like banked progress.

### A minimal reproducer for a SCHEDULING residue must reproduce the value's ARRIVAL

Extracting a scheduling residue into a small probe only works if the probe
reproduces HOW the value arrives — e.g. as a register-to-register move from a
call return — not merely its type and its uses. Get that wrong and the probe
silently drops the very instruction whose position is in question, and then
agrees with you for the wrong reason (alpha, `func_80066340`).

## A preserved body's `jal` targets can go STALE across an SDK-object round

Two runners hit this independently in round 31, which makes it a standing
hazard rather than an anecdote. A body preserved in an older report calls a
function by a `func_ADDRESS` placeholder name; the Psy-Q SDK-linking rounds
then gave that address its real Sony symbol, and the placeholder no longer
resolves. Splicing the body back in now fails to LINK, and an
`undefined reference` on a preserved body reads as though the body itself is
broken.

Measured instances: `func_80012C20` is now `printf` (echo, `func_8002BCEC`);
`func_80039228`/`func_80039104`/`func_800375E8` are now
`_spu_setInTransfer`/`SpuInitMalloc`/`SpuSetNoiseVoice` (charlie,
`func_8002EDD4` and `func_8002F700`).

**Before trusting an `undefined reference` from a spliced preserved body,
grep the symbol.** If a real name now exists for that address, the body is
fine and only its call spelling is stale. This will keep recurring for as
long as SDK-object rounds keep naming addresses.

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

## Round 32 (2026-09-12) — `volatile` as a scheduling instrument, and a register-identity verdict that was not one

### `volatile` is a legitimate, and much NARROWER, tool for the instruction-ORDER residue class

`func_80032C60` (`code_179d8_c`) sat for three rounds as a one-instruction
delay-slot residue: GCC filled an unconditional `j`'s delay slot with a
`sh zero,0(v1)` store where retail leaves a genuine `nop`. Every register
already matched. The project's sanctioned instrument for exactly this class is
a bare `__asm__("")`, and **three placements were tried across two rounds and
all three regressed** — the report concluded "barrier placement is now
exhausted for this specific residue", which was true and was read as the
stronger claim that the RESIDUE was exhausted.

It closes with no change to the function body at all. `RCntEntry` shadows the
PSX root-counter registers at `0x1F801100`/`0x1F801110`/`0x1F801120`; declaring
its three hardware fields `volatile` forces retail's ordering and the function
matches 14/14. The kept body is character-for-character round 16's "best C
reached".

**Why the narrower tool wins.** A bare `__asm__("")` is a blunt scheduling
fence: it constrains *everything* crossing that one program point, which here
meant the early `li $v0,1` and the CSE of the two return constants as well as
the store — which is why every placement fixed the store and broke something
else. `volatile` constrains only the ACCESS. Measured, all on a green
whole-image SHA1:

| form | result |
| --- | --- |
| plain `u16` + any of three barrier placements | all REGRESS past the 1-instruction residue |
| plain `u16` + single-exit join variable | WORSE: 1 word short, adds trailing `move v0,a1` |
| `*(volatile u16 *)&p->field = 0;` at the use site | **MATCH** |
| `volatile` on the struct field(s) | **MATCH** — kept |

It also **subsumes** the `__asm__("")` that `SetRCnt` carried for an analogous
hoist: barrier removed, `SetRCnt` still 40/40. Leaving both would tell the next
reader the barrier is doing work the type is doing.

**It is not the banned construct.** HARD RULE 6's test is whether removing it
changes WHICH REGISTER holds a value. A `volatile` qualifier names no register;
it changes what may move across what, exactly like the sanctioned bare
`__asm__("")`.

**Scope, measured rather than assumed:** only two units in the corpus touch
hardware addresses at all (`code_179d8_c`, `code_1677c`) and the second has an
empty queue. So this is a real mechanism with a narrow footprint — a lever to
reach for when the displaced instruction is a memory access the hardware
observes, **not** a sweep to run. Do not scatter `volatile` over ordinary
globals to nudge a schedule; use it where it is TRUE and the honest typing
happens also to be the matching one.

### A register-identity verdict is a HYPOTHESIS about a mechanism, and "the registers differ" does not establish it

The same typing closed the sibling `func_80032BB8` first attempt, 14/14 — and
that one had been filed for three rounds as **"register identity, not fixable
by reshaping"**. Under HARD RULE 6 that is a *terminal* verdict: the only
constructs that fix register identity are banned, so a function filed that way
correctly stops attracting attempts. It was ordinary matchable C.

The observation was accurate; the mechanism inferred from it was wrong.
Registers differing is compatible with at least two causes:

- GCC genuinely wanting a different ALLOCATION — terminal under HARD RULE 6;
- GCC being free to SCHEDULE in an order retail was not, with the allocation
  falling out of the order it picked — ordinary, and fixable from the source.

**The discriminator is whether anything in the C constrains the ORDER.** Before
filing register-identity, ask what forced the order in retail's build that does
not force it in yours. If the answer is "an access the hardware makes
observable", the fix is a type and the verdict is wrong.

This is CLAUDE.md's wrong-CAUSE hazard in its expensive direction. A wrong
SCORE is corrected the next time anyone measures, because measuring is the job.
A wrong CAUSE is what the next round acts on — and when the wrong cause is one
the project's own rules say to stop at, it does not merely mislead, it *closes
the file*.

### 14 stalled functions were Sony library code, and every blocker screen passed them

`psyq_sdk.py coverage` has always printed a section headed "Placed objects that
fall inside GAME-code segments (SDK code miscounted as game)". Nobody crossed
it against the STALL QUEUE. Fourteen live `INCLUDE_ASM` functions — 1669 words,
~4100 lines of accumulated derivation — lie FULLY inside objects already placed
and verified against retail: `libcd/sys.o` (3), `libcd/iso9660.o` (3),
`libsnd/vs_vh.o`, `libsnd/sstable.o`, `libsnd/vm_vsu.o`, `libsnd/adsr.o`,
`libgs/gs_131.o` (2), `libgte/fgo_00.o`, `libc2/atoi.o`.

**They pass every blocker screen** — no `gp_rel`, no `mflo`/`mfhi` hazard, not
a trampoline — so they read as the cleanest ground in the queue while being
unmatchable by construction. That is the Gate 2 trampoline lesson arriving in
Gate 1b: *a screen measures the obstruction it was built for and says nothing
about the ones it was not.*

CLAUDE.md already carried the rule and already named the command. The gap was
never the rule — it was that checking meant reading a 60-line object list
against a 200-line queue by hand, per round, and spotting an overlap in hex.
`python3 tools/sdkstalls.py` does it in one command, and `nearmiss.py` now
excludes them from "ASSIGN FROM HERE" so the screen cannot be skipped.

**One bug worth recording, because its wrong answer was plausible.** Taking a
function's extent as min/max over its whole `.s` picks up trailing data and
jump-table sections thousands of words away, and reported **143** overlaps
against a true **14**. Anchor on the function's own `glabel`/`endlabel`. A
census that over-reports by 10x is easy to catch; one that over-reports by 15%
would not have been.

### `asm-differ` and the permuter compare TEXT, so two different words can look identical (round 32)

`objdump` renders both `0x2405003f` (`addiu a1,$zero,0x3f`) and `0x3405003f`
(`ori a1,$zero,0x3f`) as the same string: **`li a1,0x3f`**. Anything that
diffs disassembly TEXT is therefore blind to the difference — `asm-differ`
and the permuter's own scorer both are. Only `funcdiff.py`'s raw word compare
sees it.

Two consequences, and the second is the sharp one:

- A function can read as "one word short with no visible diff anywhere". That
  is the signature. When the text diff looks clean but the count is wrong,
  **compare the words, not the rendering.**
- **A permuter cannot optimise toward a difference it cannot see.** A search
  on such a function is not merely unlucky, it is scoring a target that
  excludes the actual residue, so "the permuter found nothing" carries none of
  its usual weight there. Round 32's `func_80050B28` had this sitting unseen
  in every round's analysis since round 27, including a permuter run.

The specific encoding rule behind it, measured over the retail image rather
than reasoned: through the pinned pipeline a **positive** constant is ALWAYS
`ori` and a **negative** constant is ALWAYS `addiu` (`ori` zero-extends, so a
negative cannot be one). Of the 252 `addiu`-form `li`s inside functions we
match byte-exact, 248 are negative and the other 4 are data misread as code.
Retail chose per-context and sometimes used `addiu` for a positive value; that
choice is unreachable. **Scope: exactly 1 of 220 live queued functions
contains one, and it is one instruction** — a real gap, and a tiny one. Do not
generalise it into a blocker class.

### A report's own CODE BLOCK is a claim to verify, not a transcription to trust (round 32)

The project already knows not to parse a FIGURE out of a report body (rounds
18, 19, 20). Round 32 extends it to the source: `func_80059BE0`'s report listed
a "best-reached C" containing a `bit` local, and what was actually banked in
`src/DreamSys.c` had that inlined away — the two had silently diverged.

The runner did the right thing and rebuilt BOTH forms before changing anything:
the report's listed version scores **14/79**, the banked version was far
better. So the prose was wrong and the code was right, but nothing local said
which — and had it been the other way round, an attempt would have started from
a body several words worse than the one already on disk, with its score
attributed to the new idea being tried.

**The check is one build and it is the same one the preserved-body drift rule
already asks for:** splice in what the report says, build, and compare against
what `src/` holds before you treat either as the baseline. A report and its
unit drift apart the moment anyone edits one without the other, and git marks
neither as wrong.

This is the same shape as the round-19 finding that roughly one preserved body
in six carries a false "clean / drift-free" claim. Both say: **the report is
evidence about a build that happened once, and the oracle is the build you run
now.**

### A permuter scaffold can silently score a DIFFERENT residue than the real build — sanity-check it before searching (round 32, second instance)

`tools/setup-permuter.sh` builds an ISOLATED compile of one function. That
compile can schedule differently from the same function inside its real
translation unit, and when it does, the search optimises toward a residue the
oracle does not have. A zero there would not transfer; a negative there proves
nothing.

Round 32's `func_8004C1C0` caught it the right way: `--debug` on the scaffold
reported **9 insertions and 9 deletions**, while the real in-context build has
**zero** of either (its 72/106 is register identity plus one reread choice, no
missing or extra instructions). The scaffold was discarded unused rather than
searched. This is the second recorded instance — `func_8004BB3C` hit the
identical trap in round 17.

**So the check is mandatory, not optional, and it is one command:** run
`--debug` on the scaffold and compare its insertion/deletion/reordering profile
against the residue `funcdiff.py` and `asm-differ` report for the REAL build,
*before* spending a search. If the two disagree about the SHAPE of the residue,
the scaffold is measuring a different function and the only correct move is to
delete it.

Note how this compounds with the two other permuter cautions this file
carries. A search can fail to find a fix that exists (round 32's
`func_8001E6F8`, closed by a two-line transposition after 34,825 iterations);
a flat score can simply mean the sampled space missed it (round 32's
`func_8002EDD4`, closed by a second independent run at iteration 66,199); and
now the scaffold itself can be scoring the wrong target. **A permuter negative
is evidence about one search against one scaffold, and nothing more.**

## Round 33 (2026-09-12) — `volatile`'s two effects, lever scoping, and a screen that cannot see ownership

### `volatile` has TWO independent effects and conflating them wastes the lever

Round 32 established `volatile` as the narrow instrument for the
instruction-ORDER class. Round 33's charlie (`code_179d8_g`) reports a
refinement that matters when the first spelling does nothing:

- Qualifying a **global's declaration** controls whether repeat reads/writes
  of it can be optimised away or reordered.
- Routing an access through a local **`volatile T *` pointer** ALSO controls
  whether the ADDRESS COMPUTATION gets folded into the memory instruction.

Those produce different residues. "I tried `volatile` and it did nothing" is
therefore not one measurement but at most one of two, and the second spelling
is the one that moves an unfolded-store residue. **Try both before concluding
the lever is dead.** Reported by charlie against `D_8006D8F8`/`D_8006D8F4` in
`code_179d8_g`; not independently re-derived by the head, so treat the
mechanism as reported-and-plausible rather than settled, and say which you
observed when you use it.

### A lever's NEGATIVE result is scoped to the state it was tested under

Round 19 recorded, correctly, that flipping a particular guard's polarity in
`func_8002B4D4` did nothing. Round 33 flipped the same guard and it closed
three words — because an unrelated earlier fix had landed in between and moved
the residue.

**So a recorded negative is a fact about a (function, lever, STATE) triple,
not about the function.** Re-try the cheap levers after any change that moves
the diff, and do not read an old negative in a report as settled. This is the
same shape as the permuter lesson one section up — a negative is evidence
about one search against one scaffold — arriving in the manual levers, and it
is the reason a long report full of ruled-out axes is not the same thing as an
exhausted function.

The corollary is a ranking rule, not a matching one: **budget a session by
ATTEMPT HISTORY, not by score.** Round 33's alpha was staffed onto `code_55dd4`
because its titles showed 252/258 and 33/40, the best-looking odds on the
board, and produced five confirmatory negatives — every one of those functions
already had five or six rounds behind it and two had 190,000+ permuter
iterations against confirmed scaffolds. A title line carries length,
word-match and first diff; it says nothing about what has been spent. See
Gate 1b in docs/PARALLEL-RUNS.md.

### Two negatives worth having

- **cc1 2.6.3's argument-evaluation order for a call is NOT steered by which
  local is assigned first in source.** Swapping two independent scalar loads
  feeding the same call's arguments is inert. Reported twice in
  `code_179d8_g` (`func_8002A75C`, `func_8002AEE0`). Treat it as a
  low-probability lever rather than a default first move on a
  redundant-load residue.
- **An early `return` inside one arm of a diagnostic block does not reproduce
  a retail join of the shape `move v0,zero` / `bnez v0,<target>`**, even
  though it is logically equivalent and shorter. Retail's source had the
  redundant-looking flag-then-branch, so write that.

### A blocker screen cannot see OWNERSHIP, and this is now measured twice

Round 32 found 14 stalled functions lying inside placed Sony objects. Round 33
converted five runs of them and hit the same shape from the other direction:
`code_179d8_i`'s carve-time header had certified `func_80032D34` as "ordinary
large fresh ground, not blocked", verified against both live screens with zero
hits. **The verification was correct and the conclusion was still wrong** — the
function is `SsVabOpenHeadWithMode` (`libsnd/vs_vh.o`), and a screen that
measures toolchain obstructions says nothing about who wrote the code. It read
as the cleanest and largest ground in the unit while being unmatchable by
construction, and round 26 spent a 232-line derivation on it.

The same unit had carried, since carve time, a comment saying a nearby
function "IS the SDK utility, not a coincidentally-named local". The finding
was sitting in prose no tool could read for several rounds. `sdkstalls.py`
exists so this is a command rather than a noticing; `nearmiss.py` runs it.

### A preserved body can carry a score that was NEVER MEASURABLE

Round 19 established that roughly one inherited body in six carries a false
"clean / drift-free" claim — the report's listed C having diverged from what
was banked in `src/`. Round 33 found a strictly worse case in
`func_8002CF18` (`code_179d8_l`): the preserved body called `func_800375E8`,
**a symbol that does not exist under that name** (it is `SpuSetNoiseVoice`).
That body could never have linked, so nobody ever built it, so its recorded
score was not a measurement of anything.

**This is a different failure from drift and needs a different check.** Drift
is a mismatch between two things that both exist, and you catch it by
comparing the report against `src/`. A never-linked body is self-consistent
everywhere it is written down; the only thing that exposes it is putting it
through the compiler. So: **before you trust an inherited score, build the
inherited body once.** It costs one iteration of the loop and it is the only
way to tell "this was measured and I can reproduce it" from "this was never
measured at all".

Once corrected — the symbol plus two unsigned-compare type residues — the
function measures 163/167, and that figure is now a real one.

### Two idioms that generalise further than the report that found them

- **The duplicate-in-both-arms idiom extends from a shared STATEMENT to a
  shared POINTER/ADDRESS computation.** Hoisting `e = &D_8008E978[idxStruct];`
  above an `if` is what a human writes; retail's compiler saw it written
  inside each arm. Moving it into both arms closed 5 words on
  `func_8002E138` (108/112 -> 113/112).
- **The narrow-cast-defeats-strength-reduction idiom is UNIT-wide, not local
  to the function that discovered it** — and **the cast width must match the
  surrounding comparison's width** (`(u16)i`, not `(u8)i`). `func_8002CF18`'s
  report had named the fix; nobody had tried it on `func_8002DDBC` in the
  same unit, where it was worth 10 words (98/112 -> 108/112).

The transferable half is the second clause of each: **an idiom recorded in one
function's report is a candidate for every sibling in its unit**, and a report
is not a place a lever goes to be finished with.

### A GCSE / value-availability hoist is immune to `__asm__("")` at ANY placement

Round 33 confirmed independently in two `code_179d8_l` functions
(`func_8002CF18`, `func_8002E308`) that GCC's hoist of a side-effect-free
redundant expression cannot be fenced by a bare barrier at any placement
tried, and that `volatile` regresses it by forcing a stack spill. Note
`func_8002E308`'s original report blamed copy-elision; the mechanism is the
hoist.

**This is a MECHANISM claim, not a "we have not found the right spot yet".**
It matters because the barrier is the project's reflex for anything that looks
like ordering, and this residue looks exactly like ordering while being a
value-availability decision made earlier. Recognising the class saves the
placements — and per round 33's scoping rule, it is still a claim about this
mechanism under this pipeline, so re-test it if something else moves the
residue first.

### Renaming a Sony function into a SHARED header is a latent `conflicting types`

Converting `libgs/gs_131` renamed `func_8003F2AC` to `GsSetRefView2` — and one
of the hits was `include/code_2cc8c.h`, which six units include. The build
stayed green. But that header will one day sit next to `include/psyq/LIBGS.H`'s
own prototype for the same name, and the failure would then appear in whichever
unit includes both first — a unit that never touched the declaration.

A prototype for a function ANOTHER translation unit defines — a linked Sony
object emphatically included — belongs in the `.c` that calls it. The SDK
guide already says this; what round 33 adds is that **a bulk rename is how the
rule gets broken silently**, because `sed` does not know which of its hits is
a shared header. Read every hunk a rename produces. In the same round a rename
also rewrote two HISTORICAL comments that were quoting an old symbol name on
purpose.


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

## A preserved body's symbols go stale across an SDK-object round (round 35)

A match report's preserved body is meant to be spliceable: CLAUDE.md requires it
inlined "with every declaration it needs, positioned where it would compile".
An SDK-object round breaks that wholesale, by renaming placeholder
`func_XXXXXXXX` symbols to their real Sony names (`func_80025900` -> `VSync`,
`func_80013348` -> `strlen`). The body still reads correctly and is internally
consistent; it simply no longer links.

**Three independent instances across three rounds** make this a class rather
than an anecdote: round 33 found `func_800375E8` (really `SpuSetNoiseVoice`) by
hand; round 35's echo found four renames in one unit; round 35's delta found
`func_8004109C`'s recorded 42/56 had **never been measured at all** -- funcdiff's
staleness guard fired, and the real figure was 42/56 only after the names were
fixed, then 49/56 with a new lever.

```sh
python3 tools/stalesyms.py      # preserved bodies referencing renamed symbols
```

Two calibrations, both measured rather than assumed:

- **A stale name does not invalidate the RESIDUE.** Echo corrected all three of
  its bodies and reproduced the recorded scores exactly. A hit means "this
  figure is unverified until someone compiles it", not "this figure is wrong".
- **A hit does not mean nobody noticed, and this is the trap.** Both echo's and
  delta's reports document the rename in PROSE while leaving the BODY on the
  old names -- the runner fixed the names in its working tree to measure, and
  the durable artifact kept the un-linkable version. Such a report reads as
  though it were already handled.

Do not bulk-fix the flagged bodies. Each correction needs a rebuild to confirm,
and renaming them unverified would manufacture exactly the unverified-figure
problem the tool exists to catch.

## An SDK-object round can UNBLOCK a report as silently as it breaks a body (round 35)

The section above records SDK renames breaking preserved bodies. The same
rounds move the other way too, and that direction has no detector at all.

`func_800357B0` was stalled on "no independent corroboration" for nine
cross-unit calls and an unresolved argument type. Round 34's SDK conversion
retired that blocker **without touching the function**: every one of those calls
became a real `libsnd` symbol (`SsUtGetVagAtr`, `SsUtReverbOn`,
`_SsUtBuildADSR`, ...). Rewritten from the disassembly against the real
signatures it reaches 163/179, length exact, zero drift.

Neither direction announces itself in the report that is now wrong, and neither
is visible to any blocker screen. So after an SDK-object round, a stall whose
verdict rests on "unknown callee" or "uncorroborated signature" is worth
re-reading even though nothing in its own unit changed. This is the same
staleness shape as `PARALLEL-RUNS.md`'s "a blocker's death invalidates every
report that relied on it" -- the blocker here is just an SDK symbol rather than
a toolchain construct.

## Levers measured INERT on this pipeline (round 35)

Eighteen documented negatives came out of round 35's five units. These six
generalise past the function that produced them, and belong in the "already
tried, do not re-derive" bucket rather than the "untried" one:

- **Plain C89 `register`** (the LEGAL form -- no `asm("$N")`, so not the banned
  lever) is a **no-op for allocation**. GCC 2.6.3 at `-O2` already treats
  optimizable locals as implicit register candidates.
- **A clobber-bearing `__asm__` barrier is no better than an empty one** at
  suppressing `fill_eager_delay_slots`. GCC discards both before the delay-slot
  scheduler runs.
- **`__asm__("")`'s effect is NOT consistent across siblings in one residue
  class** -- sharply regressive on one function, completely inert on its
  neighbour in the same unit and the same class. Re-measure per function; do
  not infer it from a sibling.
- **A dummy unused local cannot nudge frame allocation** -- dead-code-eliminated
  before register allocation sees it.
- **A same-valued alias variable is collapsed by copy-propagation** before
  value-numbering runs, so it is not an anti-CSE lever. Likewise an **algebraic
  identity rewrite** (`-(b-a)` for `a-b`) is transparent to GCC 2.6.3's value
  numbering.
- **Narrowing a cast-local's lexical SCOPE does not help when its LIVE RANGE
  still crosses a call.** Scope and live range are different axes and only the
  second one drives the register cost.

Note this does NOT make any of them permanently dead: a lever's negative is
scoped to the state it was tested under, and round 33 closed three words with a
lever a round-19 report had called inert, once an unrelated fix had moved the
residue.

## Sibling VLA declaration ORDER is a real register-allocation lever (round 35)

The one POSITIVE lever of round 35. The relative declaration order of two
sibling VLAs shifts `global_alloc`'s register priority for **unrelated,
non-VLA** locals -- even though it changes no local's source-level reference
count. Declaring `padded` before `text` put two of four permuted registers onto
retail's choice and moved `func_8004109C` from 42/56 to 49/56.

Worth trying on any VLA-bearing function whose residue is register identity,
BEFORE accepting that verdict.

## Seed the permuter MINIMALLY (round 35)

A permuter scaffold seeded from a whole unit file pulls in sibling functions'
live `INCLUDE_ASM` bodies and inflates the base score by orders of magnitude,
which makes the search meaningless and the rejection uninterpretable.

A nonzero-stack-difference rejection is worth re-testing once with a minimal
seed before trusting it -- but when a minimal and a larger seed agree, the
rejection is real and the residue is genuinely not a permuter target.


## An inherited body fails THREE ways, not one, and two are textually silent (round 36)

Round 35 found that a preserved body's symbols go stale across an SDK-object
round. Round 36 corrected fifteen such bodies and rebuilt every one, and
"correct the names and rebuild" turned out to be necessary and not sufficient.
Three distinct things can be wrong with a body you inherit:

| failure | compile message | fix |
| --- | --- | --- |
| stale name | `undefined reference to 'func_XXXXXXXX'` | rename to the current symbol |
| missing declaration | `` `D_8008EA22' undeclared `` | carry the declaration in |
| preamble duplicates the unit's | `conflicting types for 'D_8008D99C'` | RECONCILE — drop or retype the preamble |

The third is the new one and it is not a runner's sloppiness: the body travels
with its own `extern` preamble precisely because CLAUDE.md requires it to
("with every declaration it needs, positioned where it would compile"). What
changed is the UNIT. A later carve, or a later runner recovering declarations
that an earlier carve dropped, adds declarations for the same symbols at
different types — and the preamble that made the body self-sufficient becomes a
duplicate-at-conflicting-type against the file it belongs to.
`func_80031A44`'s round-31 salvage body is the worked example.

**Only the first row produces a hit on `error:` or `parse error`.** The other
two are fatal (`cc1` exits 33) and textually silent, caught solely by the
`*** [….o]` pattern — CLAUDE.md's round-21 table arriving in practice three
times in a single round. If you trimmed that pattern you would read all three
as "clean build, does not match yet".

## A report's MANDATED preservation form can point at the WRONG body (round 36)

CLAUDE.md mandates `#if 0 ... #endif` for a preserved body, so it is natural —
for a reader or a tool — to treat the `#if 0` block as the authoritative one.
`func_80031A44.md` is the counterexample: its `#if 0` block holds a SUPERSEDED
87/88 attempt, and the authoritative 84/88 body (the figure in its own title)
sits in a fenced block six sections lower, under a heading reading "THIS
SUPERSEDES THE TITLE FIGURES ABOVE".

Positional rules fail the same way. In round 36 one runner put its corrected
snapshot immediately after the report title and another appended it at the end,
both reasonably; here the live body is neither first nor last.

**Which body supersedes which is stated in PROSE, and no lexical rule follows
prose.** So do not build one. `stalesyms.py` had such a rule for part of one
session — clear a report once the new name appears in any preserved region —
and it produced a FALSE CLEARANCE on the first report it met, hiding a stale
live body. It now reports every stale block with its line number and how many
blocks the report has, and leaves the choice to the reader.

The general form, which this project keeps rediscovering: **a screen that
resolves an ambiguity on the reader's behalf will resolve it wrongly somewhere,
and silently.** Surfacing the ambiguity costs a line of output.

## A screen that keeps flagging its own repairs never converges (round 36)

`stalesyms.py` re-flagged all three bodies one runner had just corrected, the
same afternoon, by three self-referential routes:

- a prose sentence writing `` `#if 0` `` in backticks — which is how a report
  explains that it fixed a stale body — opened a bogus region that ran to the
  next real `#endif` and swallowed the explanation;
- the corrected body's own `/* CdControl/... (was func_80028DF0/...) */` note
  was scanned as code, though a name in a comment cannot fail to link and the
  project requires the note to be there;
- an old "here is what I tried" fenced block was read as a resume-from body.

The count therefore could not fall no matter how much work was done. A repaired
report reads as untouched, and the next round re-staffs it. **This is the same
expensive direction as a false blocker — it deletes finished work from view —
but it arrives through DOCUMENTATION rather than through a grep, so it is
invisible to the usual check of re-running the screen.** The tell is a screen
whose number does not move in a round that demonstrably fixed instances of what
it measures.

## A length-short function shifts `.bss` for the WHOLE IMAGE (round 36)

The project links one contiguous `.main` section, so a function compiled one
word short does not merely shift later text in its own unit — it shifts every
later `.bss` symbol in the executable. Reading a sibling's score with such a
function simultaneously live gives numbers tens of words off:
`func_8002A75C` reads **146/196** against its true **171/196** with
`func_8002B3F4` (one word short) also spliced in.

**This is NOT a new way a score lies.** It is documented way 3, address drift,
and funcdiff's guard fires correctly and loudly — 294582 bytes differing out of
range, "this per-function read is NOT trustworthy". What the mechanism explains
is why that out-of-range count is IMAGE-sized rather than unit-sized, which is
otherwise startling enough to be mistaken for a broken tree.

The practice: **read every score with the unit's other siblings reverted to
`INCLUDE_ASM`.** One in-progress edit at a time.

## A permuter number is in permuter units — and LOWER is not a synonym for CLOSER (round 36)

The existing entry on this warns against reading a permuter penalty as a retail
word count. Round 36 adds the other direction: a permuter search on
`func_8002B4D4` found a candidate scoring 620 -> 240, a large apparent
improvement, which rebuilt as an actual REGRESSION — 90 instructions against
retail's 91.

So a permuter result is a LEAD in both directions. Verify every candidate
against the real oracle before recording it, including — especially — the ones
whose metric moved the way you hoped.


## A "the compiler will not spend a second register" residue is a claim about the SOURCE (round 37)

**Three rounds filed `func_80040FC0` and `func_80041020` as one "redundant
cursor cache" toolchain class**, on the shared evidence that retail keeps a
separate destination register and *"every C form tried collapses it into
one"*. Every one of those forms had a SINGLE destination pointer in it.

Retail's source has **two**, and writing two produces two:

```c
d = dst;      /* move a2,a0  -- the pre-increment value */
dst++;        /* addiu a0,a0,1 */
*d = lead;    /* sb v1,0(a2)  -- stores through the COPY, not the parameter */
```

That is the **longhand of `*dst++ = x`, and the two are not equivalent in
codegen**: the post-increment spelling collapses to one register and stores
through the parameter. With the longhand, GCC 2.6.3 reproduced retail's
sequence exactly on both functions — correct length, correct CFG, every
opcode and immediate in retail's own slot (`func_80040FC0` 15/24 -> 22/24
with a lone two-word transposition left; `func_80041020` 19/31 -> 20/31 with
nothing left but a 3-way register renaming).

**SCOPE, measured rather than assumed — both variables must be MUTATED.**
`func_8004042C` looks identical in shape (retail copies both parameters into
fresh registers at entry) and its round-14 **attempt 2** had already tried
explicit local copies, recording *"no change at all"*. The discriminator:

- two pointers **incremented every iteration** = two live induction
  variables = two registers. The idiom works.
- a copy that is **never modified** is a pure alias, and copy propagation
  collapses it however it is spelled. The idiom cannot work, and that
  residue really is register identity.

So when screening for more instances, look for **a loop in which two
pointers advance**, not for a report that says retail spends a register this
build will not. A candidate pool built on the latter phrasing is mostly
false — two of the first entries opened from one died on inspection.

**And the class had TWO members, not the three its cross-references claimed.**
`func_800407F8` was carried as an "analogous stall" from round 18 onward; it
was MATCHED 11/11 in round 19 by an unrelated lever. **A class assembled by
CROSS-REFERENCE keeps counting a member after that member is closed**,
because the closing round updates the function's own report and not the
reports pointing at it.

## A permuter improvement is a LEAD unconditionally, and the size of the drop means nothing (round 37)

MATCHING-GUIDE scoped round 18's "0-for-3 trusting a permuter improvement"
to register-shaped residues. **Round 37 reproduced it across three functions
and three different residue classes in one runner session** (alpha): every
best candidate either gained nothing, relocated the residue, or regressed
when spliced into the real build. Treat the rule as unconditional.

**And the magnitude of a score drop is not a reliability signal — it is
closer to the opposite.** The round's largest relative drop (`func_8002E308`,
6315 -> 2175) was the most dramatically wrong once verified. There is no
cheap proxy; splice it in and run the oracle.

Three corollaries, all measured the same round:

- **Not every part of a multi-change candidate is load-bearing.** On
  `func_8002CF18` the candidate bundled four changes and only two did
  anything; the pointer-indirection and declaration-reorder parts were
  INERT, proven by direct testing. Isolate the minimal fix before writing it
  up, or the report teaches three cargo-cult changes alongside the real one.
- **A local-best deserves MORE skepticism than a zero, not less** (charlie).
  One candidate eliminated an address-drift trap entirely (134342 -> 1 byte)
  and improved funcdiff's raw count, yet was a net REGRESSION once counted
  through `asm-differ`'s realignment.
- **A base score far above 200 does not mean the scaffold is broken**
  (delta). "One insertion + one deletion = 200" is the signature of ONE
  class, not a validity test. Scaffolds scoring 770 / 760 / 1279 were each
  confirmed faithful by direct objdump of target vs base; distrusting them
  would have thrown away three valid searches.

**`do { ... } while (0);` around a statement group is a real C-level length
lever** — fully idiomatic, not an asm hack. It closed a 4-word length gap on
`func_8002CF18` where barriers, aliasing and algebraic rewrites had all
failed, and did nothing for `func_8002E138` in the same round.

## The permitted `__asm__("")` barrier is inert against every pass that is not the scheduler (round 37)

Two independent measurements, different passes, same conclusion:

- it does **not** affect GCC's cross-jump / tail-merge RTL pass, which
  decides over whole basic blocks rather than instruction order (delta,
  confirmed in an isolated `cpp|cc1|maspsx|as` reproducer);
- it does **not** affect loop-preheader EMISSION order — neither position,
  before or after the statement, moved a two-word transposition on
  `func_80040FC0` (head).

The permitted barrier moves SCHEDULING. Before reaching for it, ask which
pass produced the residue; if the answer is not the scheduler, it will do
nothing and the negative is predictable rather than informative.

## "Rebuild the inherited body" must extend to the report's STRUCTURAL claims (round 37)

Round 33 established that a preserved body can carry a score that was never
measurable (it called a symbol that did not exist, so it never linked).
Round 37 found the next form along, and it is textually silent in a way the
first was not.

`func_8004E6B8`'s body **links, compiles, and reproduces the recorded LENGTH
exactly** (1 word short, 48/49). Its recorded *internal structure* is false:
the report credits an attempt-4 goto/if-split rewrite with eliminating a
cross-jump merge of two call sites, and `objdump` of the rebuilt body shows
ONE `jal` — still merged. Confirmed in an isolated reproducer with zero
project headers, and **not** toolchain drift: `addiu_at` is a maspsx flag
and maspsx runs AFTER cc1, so it cannot touch cc1's cross-jump decision.

So a length figure can be correct while the mechanism it is attributed to is
wrong, and only reading the disassembly against the report's prose catches
it. The same round found two more of the round-33 kind by the same rebuild
discipline (`func_8002FAC4`'s body did not link — `func_80032148` is now
`SpuVmVSetUp` and `D_8008EA0D` was never a real linker symbol) and one plain
arithmetic slip that had stood since round 26 (`func_8002EA44` is 222 words
built, 6 short, not the recorded 223/5-short — every round since ranked it
on a figure off by one).

## Two figures measured at different LENGTHS are not comparable (round 37)

`func_80029F10` went 278/282 -> **282/282, length exact**, and its raw
word-match FELL, 132/282 -> 98/282. Adopting it was still right, and the
reasoning generalises: the old 132 was measured with a 4-word length gap and
was inflated by shift ripple, while the 98 is measured at exact length and
counts true independent residues. Deletion-only lines fell 6 -> 4, so no new
structural gaps. Exact length also ends the drift that made every prior
reading of that function untrustworthy.

**A raw word-match may therefore legitimately DROP as a function gets
closer**, and comparing it against a figure recorded at a different length is
meaningless. This is the round-23 "three figures in the title" rule showing
its teeth: length and word-match are different measurements, and only the
first is comparable across states.

Note the lever that got there was `volatile` on a global array's extern
declaration (plus its aliasing locals) — a distinct form from the
single-pointer-cast already documented, and principled here rather than a
hack, since that array is mutated by a callback.

## An unused stack frame is reserved by an unused local ARRAY, never by a scalar (round 38)

**Closed `func_80031CF0` (31/31).** Retail opens `addiu $sp, $sp, -0x8`, closes
`addiu $sp, $sp, 0x8`, and **never stores to the frame**. Nothing in the
function's visible behaviour needs one.

The reflex is to treat that as a toolchain residue, or to reach for a
dead-code hack to force the allocation. Neither is right. Six variants,
measured one per build against the whole-image oracle:

| variant | bytes | whole-image build |
| --- | --- | --- |
| `s32 unused[2];` | 8 | **green, 31/31** |
| `s16 unused[4];` | 8 | **green, 31/31** |
| `s32 dead[2];` + `if (0) { dead[0] = 1; }` | 8 | green, 31/31 |
| `s32 unused[1];` | 4 | RED |
| `s32 unused;` (scalar) | 4 | RED |
| `s32 d0; s32 d1;` (two scalars) | 8 | RED |
| no local at all | 0 | RED |

**The discriminator is ARRAY-vs-SCALAR, and then SIZE — not usedness.** An
unused local *array* reserves stack frame space equal to its own size under
GCC 2.6.3 at `-O2`. An unused *scalar* does not, however many you declare:
it is register-allocated and then eliminated. The two-scalar row is the
control — same 8 bytes as the array form, still red.

**The `if (0)` guard is inert.** It does not rescue a scalar and an array does
not need it. It appeared in this function's first matching body and was
removed with no change to the bytes. Left standing it would have become the
project's precedent for the next unused frame, which is why it is worth
recording that it was measured rather than merely disliked.

**So an unused-frame residue is a SIZE MEASUREMENT, not a wall.** Read the
`addiu $sp, $sp, -N` off the prologue and declare an unused local array of
exactly N bytes. Getting N wrong fails the whole-image build outright rather
than scoring low, so the oracle answers immediately and the search space is
one number.

**This QUALIFIES a claim already written down here unconditionally.** Round
35's update in `docs/match-reports/func_8002DDBC.md` reports that *"GCC 2.6.3
dead-code-eliminates an unused local"* — reached from a scalar experiment and
stated without the qualifier. That is true of scalars and **false of arrays**,
and the difference is the whole lever. A negative measured on one storage
class is not a negative for the other.

### SCOPE — measured, and it is narrower than the paragraph above implies

This entry originally closed by naming `func_8001A268` (`code_8220_c`, filed as
*"unused-frame placement residue, 53/70 words"*) as the obvious next
application. **That was wrong, it was written without checking, and it was
caught by measuring rather than by reasoning — so the correction is kept here
rather than quietly deleted.**

The lever answers *"retail reserves a frame and my build reserves NONE."*
`func_8001A268` is the other situation entirely: **the build already reserves
`0x20`**, with no local asking for it, and retail reserves the same `0x20`. The
residue is purely WHERE the adjustment sits — retail schedules
`addiu $sp,$sp,-0x20` into the delay slot of the loop-skipping branch at word
17, the build emits it as an ordinary prologue at word 0. Measured:

| body | frame emitted | score |
| --- | --- | --- |
| preserved body as filed | `-0x20` at word 0 | 53/70 |
| same body + `s32 unused[8];` | **`-0x40`** at word 0 | **52/70** |

The declared array did not *relocate* the reservation, it **stacked on top of
one that was already there** — 0x20 became 0x40 — and the score got worse.

**So the discriminator for reaching for this lever is "does my build emit NO
`addiu $sp` at all", not "does retail have an unused frame".** Check the built
object before you reach for it:

```sh
tools/binutils/bin/mipsel-linux-gnu-objdump -d build/src/<unit>.c.o \
  | awk '/<func_XXXXXXXX>:/,/^$/' | grep 'addiu.*sp,sp'
```

No line means the lever applies and the array size is the number to solve for.
A line already matching retail's size means the residue is PLACEMENT, this
lever is inert, and adding a local actively regresses it.

That GCC 2.6.3 reserves a baseline frame for some frameless leaf functions and
not others is unexplained and is not needed to use the rule — the objdump check
settles it per function in one second, which is the same discipline CLAUDE.md
states for blockers: **a lever's SCOPE is measured, not reasoned.**

## Hoist BOTH values before EITHER is consumed — the lever that four "register identity" verdicts were hiding (round 38)

Round 38 closed five functions across two units on one shape, after three
prior rounds had filed them as register-identity or scheduling residues that
source reordering could not reach.

**The shape:** retail computes the value for field 2 **before consuming** the
value for field 1. Every prior attempt wrote the natural C — finish field 1
(load, transform, store), then start field 2 — which gives GCC 2.6.3 no reason
to keep both live at once, so its allocator never reaches retail's assignment.
Hoisting both into explicit temporaries *before either is used* reproduces
retail's scheduling and every register role at once.

| function | what retail does first | result |
| --- | --- | --- |
| `func_80031D6C` | loads **both** `s16` fields before dividing **either** | 35/35 |
| `func_80031DF8` | computes `p1*129` **and** `p2*129` back-to-back before any table indexing | 39/39 |
| `func_80041020` | copies the second byte into its own value before the first's arithmetic | 31/31 |
| `func_8004109C` | (same family; 4-value variant) | 56/56 |

`func_80031D6C` had been filed since round 21 as *pure register identity* after
four exhaustive declaration- and statement-reorder attempts. The lever was
never tried because reordering *statements* and hoisting *values* look like the
same move and are not: reordering leaves each value consumed where it was
produced; hoisting separates production from consumption.

**The diagnostic is in the disassembly and takes one read:** if retail's two
loads (or two multiplies) are ADJACENT and their consumers come later, the
source hoisted both. If load/use alternates, it did not.

### And the corollary that generalises past this shape

**A residue that survives each of two levers independently has NOT been shown
to survive their combination, and that combinatorial gap is where these hide.**
`func_8004C470` had twice been filed with its `addu` operand order *"confirmed
(twice, both operand-textual-orders tried) immune to source reordering"*. That
is true: flipping operand order alone is inert, reconfirmed at 68/70. Hoisting
the field into a local alone is also inert. **Both together reach 69/70.**

So when a report says a residue is immune, read what was actually tried. Two
inert levers are weak evidence about their conjunction, and the conjunction is
cheap to test.

## The hoist-both lever is LOCATED: it addresses load SCHEDULING, not register allocation (round 39, five independent measurements)

Round 38 named "hoist BOTH values before EITHER is consumed" from five
functions it closed, and it read as a general answer to register-identity
residues. Round 39 put it in front of five units chosen for variety and it
came back negative in four of them, with a mechanism each time. Taken
together those negatives locate the lever rather than weakening it.

| where | precondition | result |
| --- | --- | --- |
| head, `func_8003DAD4` | holds; hoist already applied | **required and insufficient** -- removing it costs 90 words and 2 instructions, and 10 further variants are inert |
| bravo, `code_8220_c` | fails -- a single already-atomic macro expansion, not two producible values | forcing it **regressed** 46/54 -> 44/54 |
| alpha, `code_179d8_k` | fails in all 8, six distinct ways | inert |
| charlie, `class_3bb8c` | fails 3 ways; holds in 1 | in the one case it holds, applied since round 13 and insufficient alone |
| echo, `DreamSys` | fails in all 5 | inert |

**The three ways charlie found the precondition failing are worth memorising,
because each looks like the shape from a distance:** the second value's load
is **branch-gated** rather than adjacent (`func_8004C470`); the two loads are
**34 bytes apart with the first fully consumed between them**
(`func_8004C1C0`); the "two values" are a pair of **induction variables
already live throughout**, so there is no staggered consumption to create
(`func_8004BB3C`). Echo adds a fourth: four of its five residues are
**delay-slot FILL CHOICES between two already-independent instructions** --
there is no dependency chain for a hoist to shorten, so the lever has nothing
to attach to.

**So: check the precondition against the `.s` before reshaping. A forced hoist
is not a neutral experiment** -- bravo's cost two words and echo's cost a
callee-saved register. And per `func_8003DAD4`, "the hoist is already in this
body" is *not* evidence the lever was tried and failed; it may be evidence the
lever already paid and the remainder is a different class.

## The combination corollary, refined twice in one round -- it has a PRECONDITION too

Round 38's corollary ("a residue that survives two levers independently has
not been shown to survive their combination") is real and paid again in round
39, but it was being read as "always try the conjunction". Two runners bounded
it from opposite sides.

**Charlie, positively: the combination helps a COMPOUND ASSIGNMENT's implicit
re-dereference, because there is something for the hoist to eliminate.**
`func_8004BE54`'s two `(*slot)->unk10 |= flagBit` sites closed (130/150 ->
132/150) by hoisting the reloaded field into a local before the `|=`, combined
with an operand order that was inert alone. **The analogous `addu` site in the
same function did not respond** -- it is read once and never re-dereferenced,
so a plain single-use commutative operand has nothing for the hoist to remove.

**Echo, negatively and more sharply: combining two levers is only a NEW
EXPERIMENT if they target INDEPENDENT compiler decisions, not the same one
twice.** On `CalcDreamColor`, hoist alone, operand-order reversal alone, and
both together produced **byte-identical** output, because both levers act on
the same single fused address expression. That is the precondition charlie's
case satisfied and this one does not.

**And a head-side trap the same round: a conjunction's score says nothing
until each component has been measured alone.** On `func_8003ECD0` the
attempt-6 x attempt-7 conjunction scored 16/73 against a 71/73 baseline, which
looks like "conjunctions can combine destructively". It is not: attempt 6's
own reuse half scores 16/73 by itself, so the conjunction is fully
attributable to one component and the barrier contributes nothing. This is the
project's attribution discipline (CLAUDE.md's wrong-CAUSE hazard) applied to a
lever instead of a blocker.

## Dropping a name and recomputing inline -- a lever, and the same fact as a constraint (round 39)

**As a lever (alpha, `func_800357B0`, 163/179 -> 171/179):** where retail
recomputes a value fresh on each converging path rather than keeping it live,
a named local that hoists it **over-commits a register-allocation decision
retail never made**. Dropping the name and recomputing the expression inline
at its one real use closed 8 words.

**As a constraint (head, `func_8003ECD0`):** the same fact read backwards.
Retail recomputes `4 << self->unk3C` at its second site; naming it once and
reusing it at both sites scores **16/73 against a 71/73 baseline, with
whole-image drift**. Naming it but letting the second site recompute is inert
at 71/73.

So before hoisting a re-derivable expression into a local, check whether
retail recomputes it. If it does, the name is the bug.

**Echo confirms the pointer case, third instance this round: function-scope
hoisting of a re-derivable POINTER or ADDRESS reliably costs a callee-saved
register** (`func_8005A9CC`, 18/88 with 135807 bytes of drift).

## A reshape that fixes one named sub-residue can still be a NET regression (round 39)

`func_800598E8` is 1 word SHORT. Echo duplicated its tail call at only the
decay branch's exit -- narrower than round 32's already-rejected full
duplication -- and it **does** fix the `$a0`-vs-`$s0` register-identity half.
But GCC does not cross-jump-merge the duplicated call site back down, so the
function grows to 78 words: **1 word LONG instead of 1 word short.** It trades
one real residue for another rather than closing either.

Record such a result as a trade, not as a wash and not as an improvement --
the word count is identical in magnitude and the function is no closer.

## The duplicate-arm register trick: real, oracle-confirmable, and strictly context-dependent (round 39)

Delta closed `func_8002AEE0` (174/174) partly with
`if (alwaysTrue) { X; } else { X; }` -- identical arms, which GCC cross-jumps
back into one instruction, so the construct emits nothing and only perturbs
register allocation. It is worth **12 words** there (162/174 without it).

**It does not generalise, and delta scoped it rather than promoting it:**
neutral on `func_8002B198`'s three permuter candidates and on
`func_8002B4D4`, and it **regressed `func_8002AA6C` from 202/223 to 17/223**
when applied at the very top of a function, where there is no pre-existing
register pressure for it to redirect.

**Treat it as a diagnostic, not a fix.** `docs/PARALLEL-RUNS.md` Gate 3 names
duplicate-arm forms alongside UB as the signature of an exhausted class, and
the head's eleven attempts to replace this one with idiomatic C all failed.
The useful question about a construct that compiles to nothing is **"what is
it standing in for?"** -- and a tautological null check is what a MIS-MODELLED
global looks like. Both operands there are spelled address-of; if either is
really a pointer global in retail's source, the check is genuine and the
duplication disappears. See `docs/match-reports/func_8002AEE0.md`.

## A reproduced toolchain mechanism is not a blocker until its CORPUS FREQUENCY is measured (round 39)

`func_80050B28`'s report established, correctly and in isolation through the
pinned pipeline, that retail's word 5 is `addiu $a1,$zero,0x3F` where our
build emits `ori`; that `objdump` and `asm-differ` print BOTH as `li a1,0x3f`
and cannot see the difference; and that the choice is made entirely by
`maspsx.expand_load_immediate()`, which picks `ori` for every
`0 < K < 0x10000` under the pinned `--aspsx-version=2.34`. Every word of that
reproduces. The conclusion drawn from it — that the function was toolchain-
blocked — did not survive one more command:

```sh
grep -rcE '\b(addiu|ori) +\$[a-z0-9]+, \$zero, 0x' asm/
```

**One `addiu` in the entire executable's disassembly, against 1089 `ori`** —
and the one is that function's word 5. A rule wrong for 1 instruction in 1090
is not wrong; the instruction came from a different assembler. Sony's own 178
objects in `lib/` carry both forms (226 `addiu`, 386 `ori`) because the game
mixed library builds, and **`libapi` and `libcard` are 100% `addiu`**. The
function tiles libcard's own object run and calls only libcard functions: it
is libcard code whose object is not on the user's discs.

**The lesson is about which claim the scope rule attaches to.** CLAUDE.md
already says a blocker's SCOPE is measured and not reasoned, and that rule was
applied — to the MECHANISM. Reproducing a mechanism in isolation tells you the
tool behaves that way; it tells you nothing about whether retail ever depended
on the other behaviour. Those are two measurements and only one of them was
taken.

**Corollary for reading reports: a report can have a right CAUSE and a wrong
OWNERSHIP, and that combination is harder to spot than either alone.** It
produces a document that is internally consistent, carefully argued, and
recommending permuter budget on code no C was ever compiled to. Nothing in it
reads as stale. See `docs/PARALLEL-RUNS.md`, the eighth Gate 1b screen, for
the two mechanical detectors.

## A function can REQUIRE the hoist-both lever and still not match (round 39)

Round 38 named "hoist BOTH values before EITHER is consumed" from five
functions it CLOSED. `func_8003DAD4` is the first measured case where the
lever is **load-bearing and insufficient**: the preserved body already hoists
both fields, and dropping the second temp alone collapses it from 114/118 to
24/118 and makes the function two words shorter. Ten further source-shape
variants — six declaration/statement-order permutations, two barrier
positions, a no-base-local spelling, a temps-at-top spelling — all land on the
identical 114/118 or worse.

**So the lever reproduces retail's load/store SCHEDULING; it does not by
itself settle which REGISTER each hoisted value lands in.** Two consequences:

- *"The hoist is already in this body"* is **not** evidence the lever was
  tried and failed. It may be evidence the lever already paid, and that what
  remains is a different class.
- The precondition is real and worth checking before reshaping: runner bravo
  measured a clean negative the same round on `code_8220_c`, where the residue
  is a single already-atomic macro expansion rather than two independently
  producible values. Forcing a hoisted form there **regressed** 46/54 → 44/54.
  A forced hoist is not a neutral experiment.

## Count an instruction that MOVED as ONE residue, not two (round 39)

`func_8003DAD4` reports four differing words and is two instructions from
matching. Two of the four are a single `addu $s1,$zero,$zero` that swapped
delay slots with a `nop` — retail fills the loop guard's `blez` slot with the
counter zero-init and leaves the early-return branch a `nop`; our build does
the opposite. A raw word-match charges the move at both ends.

This matters because the next round ranks from title figures. "114/118" reads
as four independent residues and prices the function accordingly; "two
instructions, one of them a delay-slot swap" is a different and much better
target. Say which when it applies.

## Four smaller levers, measured in round 38

1. **A "wrong preheader order" residue can be a source NAMING-ORDER problem,
   not a scheduling one.** `func_80040FC0`'s last residue was two transposed
   preheader words; `__asm__("")` could not reach it (consistent with the
   barrier being inert outside the scheduler). Naming the comparison constant
   in its own local, *assigned before* the pointer copy, reproduced retail's
   emission order. Preheader emission follows the order values are named.

2. **A register-identity residue that permutes several unrelated-looking
   values can have ONE shared trigger.** `func_80041020`'s 3-way and
   `func_8004109C`'s 4-value permutations each collapsed to a single change:
   reading a reused value through a **fresh local** rather than through the
   variable that already held it.

3. **Splitting a combined expression into two statements, so the statement
   count matches retail's instruction count, is a distinct lever** and
   combines with declaration-order tricks rather than substituting for them
   (`fill = width - strlen(...)` split in two, in `func_8004109C`).

4. **A preserved body's forward declaration of a SIBLING in its own unit goes
   stale the moment that sibling is matched.** `func_8004109C`'s body declared
   `func_80041020` with a guessed signature; matching that sibling earlier in
   the same round made the declaration a `conflicting types` error. This is
   the round-33 stale-symbol class arriving from inside a single unit — check
   for it before reading a rebuild failure as evidence about the body.

## Eliminating a DEAD RELOAD is a register-allocation lever, and it surfaced twice in one round (round 40)

The construct, in full:

```c
/* before */                      /* after */
p = x->y;                         x->y->z = 0;
p->z = 0;
```

where `p` **already holds `x->y`** from earlier in the same block, so the
reload is semantically dead. It is not a readability change: on
`func_8004B700` it took the function from **125/140 to 137/140** -- twelve
words, none of them at the reload's own site. Removing the redundant load
re-shaped allocation across the whole enclosing loop.

**Why it is worth writing down rather than filing as one function's trivia:
the identical mutation was the best saved candidate on `func_8004BE54` in the
same round**, from a search that had no knowledge of the other. Two different
functions, two independent searches, one construct. That is the signature of a
real lever rather than a local accident.

It is the mirror image of the round-39 *"dropping a name and recomputing
inline"* entry, and the pair together are the general statement: **what a
value is NAMED and how many times it is LOADED are separate axes, and GCC
2.6.3's allocator is sensitive to both.** A near-miss whose residue is
register identity should be read for dead reloads before it is read for
anything else -- they are cheap to spot (a local assigned a field chain, then
that chain re-read) and cheap to test.

**Both instances are now TESTED and both paid** (round 40, continuation):
`func_8004B700` 125/140 -> 137/140 and `func_8004BE54` 132/150 -> 142/150.
This paragraph read *"`func_8004BE54`'s instance is still UNTESTED"* until
round 41 -- written while the search was still running, and left standing
after bravo tested it within the hour. A lever's status line is exactly the
sentence a later round reads to decide whether to pull it, so a stale
UNTESTED here is a lever nobody pulls twice.

Still not a universal, and the scope is the useful half: on `func_8004B700`
the *other* two shapes tried at the residue site both went BACKWARDS (107/140
dropping the local entirely, 136/140 adding a value temp). The lever is
"remove a load that is already dead", not "rewrite pointer chains".

## A sub-base permuter candidate is worth translating even when the search never reached ZERO (round 40)

Gate 3 is written around zeros, and that phrasing is load-bearing enough that
it reads as "no zero, nothing to translate". Measured otherwise:
`func_8004B700`'s search ran 37155 iterations, **never reached zero**, and its
best candidate scored **15 against a base of 75** -- and translating that
candidate was worth twelve words.

**And the screening question is not "how good is the score" but "reduced to
STATEMENTS, what did it actually change?"** The raw diff of that candidate is
enormous and almost entirely the permuter's own reformatting: brace style,
added parentheses, `asm` for `__asm__`, whitespace collapse. Anyone reading it
raw would reasonably conclude there was nothing there. Reduce both sides
first, then read:

```sh
awk '/<func>\(/,0' <file> | tr -d '\n' \
  | sed 's/[[:space:]]\+/ /g; s/{/;/g; s/}/;/g' | tr ';' '\n' \
  | sed 's/^ *//; s/ *$//; /^$/d' > /tmp/<tag>.stmt
# ...for base.c and for output-N-1/source.c, then diff -u the two.
```

On `func_8004B700` that collapsed the entire diff to **one statement pair**.
The cost of finding out is one command.

## A permuter scaffold's `base.c` is NOT the project's C (round 40)

Round 33 established that a preserved body calling a symbol that no longer
exists could never have linked, so its recorded figure measured nothing. **The
same trap sits one step earlier and is easier to fall into**, because the
scaffold body looks exactly like project source and is right there next to a
score.

`tools/setup-permuter.sh` produces a FLATTENED translation unit: the
function plus its own inlined preamble of typedefs. Pasting that body into the
real unit fails -- measured this round on `func_8004BE54` with
``LinkResource' undeclared`` -- because the function's local types live in its
match report's `#if 0` block, not in the unit. The failure here was loud and
cost a minute. **The dangerous version is when it is quiet**: a scaffold body
that happens to compile in the unit while binding a DIFFERENT type of the same
name would produce a real-looking figure for the wrong code.

So: bring the declarations across with the body, build, *then* believe the
number. Same discipline as round 33, one layer down.

## A recorded permuter search is only evidence of COST if its SCAFFOLD was validated (round 40)

Gate 1b's sixth screen ranks on whether a function has ever been
permuter-searched, on the reasoning that an unsearched function has its
cheapest lever untried. Round 40 found the screen's blind spot, twice,
independently, in one runner session.

Two functions listed as "searched" in that round's own assignment table had
their searches run against scaffolds their reports **explicitly documented as
scoring a different residue than the real build** -- `func_8004BB3C` (round
17: 2 insertions / 2 deletions in isolation against 0/0 in context) and
`func_8004C1C0` (round 32: 9 and 9 against 0 and 0). The runner rebuilt both
scaffolds from scratch and reproduced both mismatch figures exactly, four and
fifteen rounds later respectively.

**A search against a scaffold that does not reproduce the residue is
permuter-INCONCLUSIVE, not permuter-exhausted**, and the distinction is
invisible to a grep for iteration counts. So run `--debug --stack-diffs` and
compare the base score's composition against the real in-context residue
BEFORE searching -- which the setup script already advises -- and record the
comparison, because that record is the only thing that tells a later round
whether the iteration count it is ranking on meant anything.

The general shape is this project's most-repeated one, arriving at the screen
that was built to price cost: **a screen measures the obstruction it was built
for.** The sixth screen measures whether a search RAN. It says nothing about
whether the search could have SUCCEEDED.

## A data-modelling hypothesis is settled by reading the DATA SECTION, not by compiling variants (round 40)

`func_8002AEE0` carried a tautological `if (p6A0 || pF8)` with identical arms,
kept because it is worth twelve words, and round 39 flagged the obvious
hypothesis: a tautological null check is what a MIS-MODELLED global looks
like, so is either operand really a pointer global?

Both halves were settled with **two greps and no builds**:

- `D_8006D6A0` is a fixed **8-element table of rodata string addresses**
  (`asm/data/5DDFC.data.s`) -- an array, so the decay is tautologically
  non-null.
- `D_8006D8F8` is **one zero word** that a sibling function stores
  `VSync(-1)`'s return into -- an `s32` timestamp. A pointer global holds an
  address; this holds a frame count.

**A different mis-modelling was real, and the tell was not the tautology at
all.** `pF8[-1]`, four lines below and used four times, is an index nobody
writes by hand: negative-indexing off a named global only means anything if
the neighbouring word is part of the same object. The ten consecutive words
are one array, which the unit's own zeroing walk had implied for rounds.
Declared and indexed as one: **byte-identical, whole image green.**

And the result that matters: **it does not dissolve the construct.** Under the
corrected model the tautology is still worth the same twelve words, and every
residual diff is a pure register swap. The twelve words are register
ALLOCATION, not data modelling.

Two transferable points:

- **Read the data before compiling variants.** Round 39 spent eleven builds on
  source-shape permutations of this construct; the ownership question took two
  greps and was conclusive in a way an attempt count never is.
- **Commit a corrected model even when it turns out not to be the cause.** It
  was byte-identical, so it costs nothing, the next reader does not
  re-discover it -- and the negative result is only trustworthy *because* it
  was measured against the corrected model.

## Gate 1b's sixth screen is neither SOUND nor COMPLETE, and it fails in three separate directions (round 41)

The sixth screen asks *"has this function ever been permuter-searched"* and
keys on **evidence that a search RAN** -- iteration counts, `rc=`, `base
score`, `permuter-exhausted`, `--stop-on-zero`. Round 37 already corrected it
once, from keying on the WORD "permuter" (which matched reports merely
*recommending* a search) to keying on run evidence. Measured again in round 41
over the 101-function blocker-clean queue, it is still wrong three ways, and
they do not all point the same direction:

| direction | mechanism | measured instances |
| --- | --- | --- |
| **counts a sibling's run as this function's** | an honest cross-reference QUOTES the evidence | **6** (`code_8220_c` RCpoly siblings) |
| **counts a scaffold CHECK as a run** | `base score` is in the evidence set, but building and checking a scaffold is not searching with it | **4** say *"no search was run"* / *"REJECTED, not searched"* in plain text |
| **misses a real run** | the iteration pattern `[0-9]{3,}[, ]*(iteration\|iters)` cannot span an adjective | `func_800400B0`'s *"~94,000 unguided iterations"* scores **0** hits |

**The cross-reference direction is the one worth internalising, because
nobody did anything wrong to produce it.** Round 40's alpha searched the
RCpoly family's root case (`func_800197C4`) deeply, confirmed each sibling's
scaffold scored identically, and wrote *"base confirmed, cross-reference
only"* as its own section heading. That is exactly the honest disposition.
But a faithful citation of a sibling's search reproduces the sibling's
iteration count verbatim, so the screen reads six unsearched functions as
spent. **Honesty in a report is indistinguishable from provenance to a grep.**

**And the scaffold-check direction matters because it inverts the ninth
screen.** Round 40 established that a search against an unvalidated scaffold
is permuter-INCONCLUSIVE. The correct response -- build the scaffold, check
it with `--debug --stack-diffs`, find it unrepresentative, and decline to
spend search budget -- leaves `base score` in the report and nothing else.
So **doing the right thing makes the function read as searched**, and four
reports that state outright that no search was run are counted as spent
ground.

The general shape is this document's most-repeated one, arriving on a screen
built to measure COST rather than an obstruction: **a screen measures the
thing it was built for, and a report is prose about a search, not the search.**
Two practical consequences:

- **Do not rank on the sixth screen alone.** When it says SEARCHED, check
  whether the figure is this function's own before treating the ground as
  spent -- a figure that appears next to another `func_XXXXXXXX` on the same
  line is the tell, and it is one grep.
- **When you cite a sibling's search, say so in the VERDICT REGION**, not
  only in the section body. The six RCpoly reports now carry a `SEARCH
  PROVENANCE CORRECTED` note in their first lines for exactly this reason.

**What is NOT claimed here.** A tightened screen counting only iteration/`rc`
figures returns 42 functions with no run evidence against round 37's 26, and
that 16-function gap is **not** a finding -- hand-checking all sixteen shows
it mixes real searches whose iteration counts are phrased outside the pattern
with scaffold checks where no search ran. The two screens bracket the truth
and neither is exact. The **6** are hand-verified and unambiguous; they are
the only figure here worth carrying forward.

## A plain-global-to-ARRAY retype silently invalidates a SIBLING's preserved body, via pointer decay, and it still compiles clean (round 41)

The round's most dangerous finding, because every existing guard passed it.

Round 40 retyped a global in `code_179d8_g` from a bare `D_8006D8DC` to
`s32 D_8006D8DC[10]`. `func_8002B4D4`'s preserved `#if 0` body, written
earlier, still spelled it bare -- and a bare array name **decays to a
pointer**, so the body's `D_8006D8DC > 0` quietly stopped meaning *"is the
retry counter positive"* and started meaning *"is this address nonzero"*
(always true). Corrected to `D_8006D8DC[0]`, the function moved **60/91 ->
84/91**, and a fresh search against the corrected seed resurrected a
candidate round 36 had rejected as unsafe -- it was only unsafe against the
broken base.

**Why this is a new class and not an instance of the struct-edit hazard.**
The struct-edit rule (round 13) is about an edit that breaks an
ALREADY-MATCHED function, and it is caught loudly: the whole-image SHA1 goes
red. This breaks a PRESERVED BODY, which is not in the build at all, so:

- nothing fails, ever -- there is no oracle for text inside `#if 0`;
- it **compiles clean** when resumed, because the decayed form is valid C;
- it **survives "rebuilt verbatim"**. Rounds 39 and 40 both re-derived this
  body and both reported reproducing the figure. They did -- the figure was
  reproducible and wrong, because seed and build were consistently broken
  together.

That last point is what makes it worse than round 33's never-linked body. A
never-linked body fails loudly the first time anyone compiles it. This one
links, runs the search, and yields a *measured, reproducible, wrong* number.

**So when you retype a global (bare -> array, or any change that alters what
a bare mention MEANS), grep every preserved body in the unit for the bare
symbol**, exactly as the struct-edit rule says to grep every caller:

```sh
grep -ln 'D_XXXXXXXX' docs/match-reports/*.md     # then read each in context
```

Address-of contexts are safe (`&D_X` and `D_X` are numerically identical for
an array); VALUE contexts are not. Charlie checked its unit's other four
reports and found one more bare use, in an address-of context, correctly
left alone.

## A permuter improvement can be oracle-confirmed and still be UNSOUND C (round 41)

`func_8002AA6C`'s best candidate scored **208/223** against a base of 202 --
a real improvement by the project's own oracle, in range, no drift. It was
**rejected anyway**, and correctly: `objdump` showed it hoists a string-
literal address into a **caller-saved** register outside a loop that makes
calls, so the value is clobbered on every iteration after the first. The
concern was verified rather than asserted -- moving the reassignment back
inside the loop lost the entire gain, which is what a genuine
loop-invariance bug looks like.

The runner took the next-best candidate (215/223) instead, which reuses two
already-dead locals as sinks and is sound.

**The general point: byte-match in range is not a soundness proof.** The
oracle compares OUR bytes against RETAIL's over one function; it cannot tell
you that the C you wrote has the same meaning as the C Sony wrote, only that
this compiler turned yours into those bytes. A permuter candidate is machine-
generated C nobody has read, so read it -- especially any value hoisted out
of a loop, and especially across a call boundary.

## Two more independent demonstrations that a permuter score is not a word count (round 41)

DECOMPILATION_LEARNINGS already carries *"a permuter number is in PERMUTER
units, not retail words"*. Round 41 produced the two starkest instances yet,
in opposite directions, which together fix the rule's shape:

- **delta, `func_80032708`:** best candidate scored **2735 against a base of
  3735** -- a large, early, never-beaten improvement. Rebuilt through the
  real pipeline: **14/164 with drift**, against the 65/164 it started from.
- **charlie, `func_8002B198`:** a second 900s search produced **four**
  candidates that improved the permuter's metric. Rebuilt, **every one was a
  regression** (27-39/91, several with genuine length drift).
- **charlie, `func_8002AA6C` and `func_8002B4D4`, the other direction:**
  sub-base candidates that never reached zero translated to **+13** and
  **+24** real words.

So neither "sub-base means nothing" nor "sub-base means progress" is right.
**Translate AND measure** -- the score tells you where to look, never what
you found.

## What a value is NAMED and how many times it is LOADED: the axis has TWO directions, and neither is the lever (round 41)

Round 40 found that REMOVING a dead reload was worth 12 and 10 words.
Round 41 tested both directions in one round:

- **alpha** screened `class_3bb8c`'s two register-identity stalls for dead
  reloads and found the PRECONDITION absent -- neither function ever caches
  the value it repeatedly dereferences, so there is nothing to remove. Five
  cache/inline variants tried anyway all produced **wrong-LENGTH** compiles.
- **echo** closed `func_800585B4` (56/56) by doing the OPPOSITE: caching an
  array base into a **local pointer** and controlling its reassignment, which
  was the only thing that stopped cc1 2.6.3's loop optimizer strength-reducing
  the access into a hoisted induction variable. No rewrite of the *access
  expression* could -- round 24 had already exhausted three of those.

Together with round 39's *"dropping a name and recomputing inline"*, the
statement is now: **naming and load-count are two knobs, 2.6.3's allocator is
sensitive to both, and the direction that helps is function-specific.** The
useful discriminator, from alpha: **if a cache/inline variant changes the
function's LENGTH, you are looking at a true register-identity wall and the
axis has nothing to offer; if it only moves registers, keep pulling.**

## The scaffold's insertion/deletion count is a COST predictor, not just a validity check (round 41)

Round 40 introduced `--debug --stack-diffs` validation as a CORRECTNESS test
(*is this search capable of succeeding*). Bravo ran the two halves of a
controlled comparison in one session:

| function | scaffold | outcome |
| --- | --- | --- |
| `func_8001E110` | **0 insertions / 0 deletions**, base 105 | zero at **iteration 2642**, MATCHED |
| `func_8001DA28` | **23 insertions / 44 deletions**, base 7735 | **73273 iterations**, rc=124, every candidate noise or UB |

A near-0/0 scaffold means the residue is a small register/scheduling
perturbation the permuter is well-suited to; a large insertion/deletion count
means the divergence is structural and a bounded search will wander. Bravo
predicted the second negative BEFORE running it, from the scaffold alone, and
ran it anyway to record the measurement -- which is the right call for a
first-ever search and would be the wrong call for a repeat.

**Compose this with Gate 1b's sixth screen:** prefer a never-searched function
whose scaffold is near 0/0. That ranks *which* unsearched function to search
first, which the sixth screen alone cannot do.

**And a corollary for the dead-reload lever specifically:** a 0/0 scaffold
means there is no instruction to REMOVE, so the lever has nothing to act on.
Bravo reached that conclusion mechanically on `func_8001E4A4`; alpha reached
the same conclusion by hand on two other functions. Same answer, two routes.

## Default-then-override vs two `return` statements — and the bound that keeps it honest (round 43)

Alpha closed three functions with one reshape, and the mechanism is about
retail's REGISTER PLAN rather than its semantics. Two `return` statements
produce the right VALUES and the wrong plan:

```c
/* 1/8 words. Right values, wrong registers: GCC puts the loaded global in
 * $v0 and the constant in $v1 where retail has them swapped, and retail
 * computes the "0" result unconditionally in a branch DELAY SLOT
 * (`addu $a0,$zero,$zero`) -- which two return paths cannot express. */
s32 func_8005BF48(void) {
    if (D_8008ACC4 == 0xC) return 0;
    return (s32)&D_8008ABF0;
}

/* 8/8 byte-exact. One result variable, initialised to the default, then
 * conditionally overridden. */
s32 func_8005BF48(void) {
    s32 result;
    result = 0;
    if (D_8008ACC4 != 0xC) result = (s32)&D_8008ABF0;
    return result;
}
```

Confirmed three times independently in one unit (`func_8005BF48`,
`Test4InstantTeleporters`, `func_8005BD3C`), and then `func_8005C02C` matched
**first try** by reusing `func_8005BD3C`'s shape — which is the transfer test,
and it passed.

**THE BOUND, AND IT ARRIVED IN THE SAME ROUND FROM A DIFFERENT RUNNER.** It
would be easy to write this up as "reopened `gp_rel` functions need a source
reshape". That is false. bravo's `code_171e0` family — seven functions,
11w–15w — compiled byte-exact from ordinary C on the FIRST build every time,
and its own commit for `func_80026CAC` says "no source-shape derivation
needed". Round 43's overall hit rate (47 of 49 attempted) is driven by exactly
that: most reopened functions just compile.

**The discriminator is whether the function CHOOSES BETWEEN TWO RESULTS**, not
whether it was ever blocked. A pure accessor or a straight-line dispatch has no
choice to express and no register plan to get wrong. Filing the lever without
this bound would send the next runner hunting a shape residue in functions that
do not have one — the same error shape as a false blocker, pointed at source
form instead of the toolchain.

Related, same round, same character: `if`/`else` BLOCK ORDER and PHYSICAL block
layout had to mirror retail's (`Test4InstantTeleporters`, `func_8005BE90` —
five attempts, switch and `||`-chain both rejected before a `goto` chain in
retail's exact interleaved order landed). And bravo's `func_80027024` came out
one word long from an early-return guard, because GCC placed the larger arm
first and needed a trailing jump; flipping the test to match the `beq` polarity
directly fixed it.

## A permuter negative is only as good as the flags its scaffold was built with (round 43)

`tools/setup-permuter.sh` hardcodes `MASPSX_FLAGS` independently of the
Makefile's. When round 42 added `--gp-symbols` and `--no-nop-mflo-mfhi` to the
build, the permuter's generated `compile.sh` did not follow, so **every search
since on a function touching `gp_rel` or `mflo`/`mfhi` has been scored against
a baseline that cannot reach zero** — at least one word differs no matter what
C the permuter emits.

Two searches in round 43, one round apart in nothing but attention:

| function | iterations | verdict |
| --- | --- | --- |
| `func_8004DCD0` (delta) | 26500 | **valid** — delta spotted the gap and hand-patched its own gitignored `compile.sh` |
| `func_80026CFC` (bravo) | 150582 | **artifact** — reads `D_8008A84C`, which IS in `config/gp-symbols.txt` |

**The asymmetry is what makes this survivable: a stale flag list can
manufacture a false NEGATIVE but never a false MATCH**, because a match is
confirmed by the whole-image SHA1 and a scorer never is. So no matched function
is in doubt; only search negatives are. That is also why the failure is
expensive in the usual direction — a false negative removes a function from
future rounds, and nobody re-searches ground believed exhausted.

**The generalisable rule: a scaffold is part of the measurement, and a
measurement's tooling can go stale without anything failing loudly.** Delta's
own tell is the one to copy — *if a permuter base score looks absurd
immediately after a clean manual build scored well, suspect the generated
`compile.sh` before suspecting the candidate.*

Delta correctly did NOT patch the shared tool: a flag change is an operator
escalation (CLAUDE.md, "Escalate, do not experiment"), even when the change
would merely bring a helper back into agreement with the Makefile.

## Round 44 (2026-09-15) — five runners, 16 matches, and the reopened vein measured against the near-miss corpus

**The round's structural result, stated once: the round-42 blocker resolutions
reopened COLD ground and did NOT unstick WORKED ground. Those are two different
queues and they should be staffed differently.** Both halves were measured this
round rather than argued:

- Every one of the round's 16 matches came from the REOPENED set, and almost all
  landed on the FIRST build.
- alpha rebuilt all seven `code_8220_c` near-misses in isolation and every score
  was **identical to the historical record** — no drift, no movement. Its residue
  is register-identity plus a computed-but-unused address, which has nothing to
  do with `gp_rel`/`nop_mflo_mfhi` and gained nothing from their resolution.

So a reopened stub is worth staffing ahead of a near-miss with a better-looking
number, and a near-miss that predates the fixes should not be expected to move
just because the blockers died.

### Length-gap levers: a matched pair covering both directions

Found by bravo on two functions in one unit, and they are opposites — do not mix
them up. Both present as "one word off with an otherwise clean CFG".

| symptom | cause | fix |
| --- | --- | --- |
| **1 word SHORT**, only diff a missing `move` before the epilogue | register allocation reacting to EARLY-RETURN PHRASING | collapse two return points into a single join point |
| **1 word LONG**, an extra `move $an,$vN` landing BEFORE a load instead of filling its delay slot, with a stray `nop` later | register PROMOTION from giving a call's return value its own named local | store the result straight to its eventual home and re-derive by cast at each use, so local CSE reuses the raw `$v0` |

**The corollary is what pays, and it is a ranking rule: a one-word LENGTH gap
SHIFTS everything after it, so a low raw word-match on a 1-word-short function is
mostly RIPPLE, not independent residue.** charlie's `func_800569A8` went
**23/121 → 117/121** purely by closing the length. Do not rank 1-word-short
functions by raw word-match; it measures the gap, not the work.

### Read the delay slot before modelling the branch (alpha, correctness)

A store that appears textually INSIDE a `beqz` block may be that branch's own
delay-slot filler and therefore execute UNCONDITIONALLY; only instructions
genuinely after the delay slot are gated. Modelling it as conditional C produces
wrong code that COMPILES CLEANLY, so nothing catches it but the byte count.
Hoisting one such store out of an `if` closed alpha's last 8 words in a single
attempt.

### Write the division; do not hand-derive the magic multiply (delta)

**Verified through the PINNED PIPELINE**, which is why this survives regardless of
whose code the instance was: plain `u32 x / 127` and `x / 63` compile EXACTLY to
retail's magic-multiply + correction sequences. There is no need to reconstruct
the `(hi + (x - hi) >> 1) >> N` form by hand — write the division.

### Other confirmed shapes

- **A short zero-fill of 2+ ADJACENT `.sbss`/`.sdata` words is a pointer-decrement
  `do`/`while` loop**, not per-symbol scalar stores (those emit `gp_rel` stores
  with the wrong shape AND length). `m2ctx.py --run` recovers the loop directly.
  Disproportionately relevant while the queue is `gp_rel` functions, which touch
  small data by construction. (bravo)
- **A conditional expression retail evaluates TWICE inside one statement must be
  WRITTEN twice**; caching it in a variable makes GCC compute it once and the
  function comes up short. (echo)
- **Indexing a ternary with a nonzero constant DISTRIBUTES the index into both
  branches**, corrupting the NULL/zero fallback arm. (echo)
- **`(x & 1) ^ 1` and `!(x & 1)` are semantically identical and schedule
  oppositely**; only the explicit mask-then-flip matches retail's `andi`/`xori`
  order. (bravo)
- **`tools/classtable.py` names the occupant of a slot your unit does not own** —
  enough to type a return/arg shape without guessing, when a struct pad covers an
  offset a new call reaches through. (bravo)
- **"Two pointers in the asm" does NOT imply "two pointers in C".** A single
  struct pointer often already matches; forcing two cost delta 7 words. (delta)

### Negatives worth their own entries

- **A memory barrier does not affect pure ADDRESS CSE.** (echo, `func_8003E968`)
- **`volatile` as an anti-optimization lever is NON-MONOTONIC.** delta's correct
  4-variable set beat every 2-of-4 subset, and two MORE variables regressed
  sharply. Do not add it incrementally and assume improvement.
- **The narrow-cast/loop-strength-reduction idiom does not transfer on shape
  alone** — it regressed `func_8002D8E0` 295 → 320. Discriminator: check whether
  the narrowing serves the MULTIPLY or only the COMPARISON. (delta)
- **Array vs pointer-arithmetic vs explicit-shift spellings of `&table[idx]` are
  byte-identical**, confirmed across two rounds. Not a lever. (charlie)

### A lever's polarity verdict is scoped to the state it was tested in (charlie)

The same if/else arm-order swap that REGRESSED a function in an earlier round was
the single biggest fix here, once two other levers had landed first. An
"X made it worse" note is a finding about a STATE, not a fact about a branch.
This is the round-33 principle arriving on a new axis, and it argues for
re-testing a ruled-out axis after any unrelated fix moves the residue.

### NEW STALL CLASS: retail recomputes an address our GCC CSEs away

`func_8003E968` (echo, 39/41, 2 words SHORT). Every field, global and constant is
correct. Retail computes the SAME `lui`/`addiu %hi/%lo` pair TWICE, independently
and back to back; our GCC 2.6.3 CSEs it to once. Tried and inert or worse: memory
barrier, `volatile`, per-field copies, a permuter-suggested chained assignment.

**Note the DIRECTION, because the `volatile` bullet above covers only the other
one.** That bullet is about our compiler CSE-ing away a reload retail KEEPS; this
is the mirror. Its worked example (`func_8002C048`) was withdrawn this round as
Sony's `strcmp`, so **`func_8003E968` is the first GAME-CODE instance of this
class in either direction**, and the open question is what source shape stops two
reads being recognisable as the same object.

### A preserved body can carry a wrong READING, not merely a stale figure (charlie)

Round 26 recorded `self->unk24` as an entry guard "bounded `< 0x1F5`". Retail's
`sltiu` + `bnez`-to-EXIT means the block runs only when `unk24 >= 0x1F5` — the
other side of the threshold. Flipping it closed a word outright with no other
effect. This extends round 33's "build the inherited body before trusting its
score": **building also re-tests the READING.** A polarity error is invisible in
prose because the prose is self-consistent, and it survives indefinitely.

## Round 45 (2026-09-15) — six runners, 39 matches, and a permuter scaffold that was measuring the wrong program

### The permuter finding, which is the round's most load-bearing result

**Round 41 established `--debug --stack-diffs` as a COST test — 0 insertions /
0 deletions predicts a cheap search. Round 45 ran two bounded searches and
broke that claim in both directions at once.** Charlie, on `code_179d8_l`:

| function | stack-diffs | iterations | result |
| --- | --- | --- | --- |
| `func_8002D8E0` | 49 ins / 51 del | 51435, rc=124 | negative — **and the base score was an artifact** |
| `func_8002CD08` | **0 / 0** | ~63000, rc=124 | negative — **never beat its own seed once** |

The second row is the direct refutation: a 0/0 scaffold predicted a cheap
search and bought nothing at all. **A 0/0 scaffold is NECESSARY, not
SUFFICIENT.** Where the residue is pure register identity, no source-mutation
search can reach it however clean the scaffold is, and the scaffold check
cannot see that — it measures frame-layout fidelity, not whether the residue
lies inside the search space.

**The first row is worse and is a new check, not a refinement of an old one.
The permuter's isolated single-function scaffold can produce MATERIALLY
DIFFERENT REGISTER ALLOCATION than the real translation unit, for the
IDENTICAL source.** So its base score is a measurement of a different program,
every candidate is scored against that different program, and a search that
"found nothing" never tested what you thought it tested. Charlie caught it by
rebuilding the same body in-tree and diffing against retail directly — the two
disagreed.

So the scaffold now needs a third check, and it is the cheapest of the three:

1. *(round 40)* does the scaffold compile and score at all — a CORRECTNESS test;
2. *(round 41)* what are its insertion/deletion penalties — a COST test, now
   known to be necessary and not sufficient;
3. **(round 45) does the scaffold's BASE SCORE agree with the same body's score
   in the real build?** If not, stop: the search would be scoring a program you
   are not building. One rebuild answers it.

**And note what this does to a recorded negative.** "Permuter tried, negative"
is not evidence about the function unless check 3 passed — it may be evidence
that the search was invalid. Round 37 already found the queue's permuter
history over-counted searches by keying on the WORD; this is the same error one
level deeper, where a search genuinely ran and still measured nothing. When you
record a negative, record which of the three checks you ran.

### Source-shape levers found this round

All were found on GAME CODE and verified against the whole-image SHA1 — stated
explicitly because this round also withdrew three older levers whose positives
turned out to be Sony's (see the SDK-exit census entry above).

- **Alignment is a TWO-WAY lever and the direction is read off retail's
  instruction WIDTH** (bravo, `func_8004D6AC` 22/22). The documented idiom is
  all-`s8`/`s16` -> alignment 2 -> whole-struct assignment compiles to
  `lwl`/`lwr` + `swl`/`swr`. The inverse is now confirmed too: if retail's tail
  is byte-by-byte (`lb`/`sb` pairs) where yours merges into a halfword, declare
  the struct **all-`s8` so its alignment is 1** — alignment 2 is enough for GCC
  2.6.3 to trust an `lh`/`sh` halfword move for a 2-byte remainder that happens
  to land 2-aligned, silently merging two retail instructions into one and
  shifting the whole image. Neither direction is the default; look at the width.
- **An accumulate-in-place pointer loop wants an explicit scratch-then-advance**
  (delta, `func_8001CEB4` 85/85): `cur = p; p++; *cur = ...;`, NOT `p[i] = ...`
  and NOT `*p++ = ...`. Array indexing cost one extra word (a spurious setup
  move) and desynced the allocation from there on.
- **`switch` and a logically-equivalent if/else-if chain are not interchangeable**
  (bravo, `func_8004A070` 48/48). A 2-3-way dispatch compiling to retail's
  direct-`beq`-to-case shape, with no skip-branches, is a `switch` signature.
- **GCC 2.6.3 re-associates constant multiplies across a whole expression tree
  regardless of source parenthesization** (charlie, needed twice). Only a
  STATEMENT BOUNDARY — a separate intermediate variable — stops it. Parentheses
  are not a barrier here and writing more of them does nothing.
- **A constant set in a branch's DELAY SLOT applies on BOTH paths** (echo).
  Reading it as a conditional value invents a branch retail does not have.
- **An early exit that jumps to a `return CONST` block already sitting at the
  function's tail must be written as `if (cond == 0) { body; return X; }
  return Y;`** (echo) — not as a duplicated early return, which emits a second
  copy of the tail.
- **Two textually identical global reads with non-overlapping live ranges can
  legitimately want DIFFERENT registers** (echo). Forcing them into one reused
  C local manufactures a register-identity residue that was not there.

### Two negatives worth as much as the levers

- **A frame-size lever that fixes one function can REGRESS a structurally
  near-identical sibling** (charlie): `func_8002D8E0`'s `volatile` intermediates
  helped there and hurt `func_8002D1B4`. Same shape as round 27's three levers
  and round 7's two symmetric vtable slots — **measure the transfer, never
  assume it**, in either direction.
- **A "free" value can be scheduled into an UNRELATED SIBLING BRANCH's delay
  slot, arbitrarily far from its use** (delta, `func_80062C58`). Plain local
  statement reordering does not reproduce that, so a residue of this shape is
  not a reordering problem and should not be attacked as one.

### An operational trap: `make extract` while C is spliced in

Foxtrot ran `make extract` with its C live in the unit, and **splat then skipped
generating the nonmatching `.s` files for exactly those functions** — the files
it needed to keep working. Recovery is mechanical (restore the `INCLUDE_ASM`
stubs, re-extract) but the failure is quiet and reads like the disassembly
having gone missing. **Re-extract only from a unit whose functions are all
`INCLUDE_ASM`.** This is the same family as the four ways a score lies: a tool
answering truthfully about a tree that is not in the state you assumed.

Foxtrot also spent significant time on an apparent "gp_rel address shift" that
was ordinary address drift from two functions compiling to the wrong length —
which is the attribution hazard round 20 documented, arriving in a fresh carve.
Check the length before reaching for a linker explanation.

### Round 45 addendum — alpha and echo's late findings

**Reproduce retail's bug; do not work around it** (echo, `func_80027C80`
48/48). The function stores through `$s2` and `$s2` is never assigned on any
path — a genuine uninitialised-local-pointer bug in the shipped game. A local
pointer declared with no initialiser and stored through exactly once, at
retail's own point, matched byte-exact on the first build. GCC put it in `$s2`
because `$s0`/`$s1` were already claimed by the two parameters, so the
uninitialised local landed on the next free callee-saved register — retail's
own allocation, for retail's own reason. **No `volatile`, no fake initialiser,
no inline asm.**

Note the distinction from the permuter's UB problem, because they look alike
and are not: a permuter candidate that BRANCHES on an uninitialised read is
exploiting the scorer and is disqualified (round 41). Reproducing an
uninitialised STORE that retail demonstrably performs is just matching the code
that shipped. The discriminator is whether the uninitialised value affects
control flow in your C, not whether it is uninitialised.

**`break` + a post-loop `if` cannot express "jump PAST a fall-through tail"**
(echo, `func_80027FFC` 53/53, third attempt). A bounded retry loop whose
exhaustion path falls into a `printf` and whose success path must skip that
`printf` needs an explicit `goto`: GCC 2.6.3 does not fold the post-loop check
away and compiles a real extra `bne`. This is the same family as the
already-documented `do { } while (0)` and nested-guard entries — the loop-exit
SHAPE is a source-level choice the compiler does not normalise.

**Mutate the parameter in place when retail keeps the walking pointer in one
register** (alpha's lever, applied by echo to close the above). Introducing a
separate loop-cursor local costs a register and desyncs the allocation. This is
the same finding as delta's scratch-then-advance entry approached from the
other side, and the two together say: **match the NUMBER of live pointer
variables retail has, not just the arithmetic.**

*This lever reached echo through `tools/broadcast.sh`, from a runner in a
different unit, and closed a function the same round it was posted. That is the
channel doing the job it was added for — and worth recording, because the
alternative history is that it sat in alpha's write-up until the next round.*

**A uniform register-slot shift across a whole function signals ONE EXTRA
PERSISTENT LOCAL** (alpha). Not a scheduling problem and not a per-site
allocation accident: if every register number is displaced by the same amount,
look for a local you introduced that retail does not have.

**Adding a local that only changes READ TIMING is safe; touching an
expression's OUTER OPERAND ORDER cascades hard** (alpha). Useful for choosing
which rephrasing to try first when a residue could be attacked either way.

**K&R (old-style) definitions as an escape hatch for a dead-but-real
parameter** (alpha, PROPOSED AND NOT CONFIRMED). Both functions alpha reached
it on remain STALLS (92/114 and 99/107), so this is a hypothesis with two
partial results behind it, not a lever. It is legal C89 and it is NOT a banned
construct — it pins no register and names no operand — but nothing has yet
closed on it. Recorded so the next attempt starts from it rather than
rediscovering it; do not cite it as established.

## A BARE `__asm__("")` CAN CHANGE REGISTER ALLOCATION, WHICH IS WHAT HARD RULE 6's OWN TEST CALLS BANNED (round 46)

CLAUDE.md HARD RULE 6 permits one construct by name and then supplies a
behavioural test that the named construct can fail:

> *"A bare `__asm__("")` scheduling barrier is allowed. The test: if removing
> it changes WHICH REGISTER holds a value, it is banned; if it only changes
> instruction ORDER, it is allowed."*

Measured through the pinned pipeline on game-neutral code that mentions retail
nowhere — three variants of one function, differing only in the barrier:

```c
extern int G;
int sink(int);
int noBarrier(int a, int b) { int t = G;                          sink(a); return t + G + b; }
int bareBar  (int a, int b) { int t = G; __asm__("");             sink(a); return t + G + b; }
int memBar   (int a, int b) { int t = G; __asm__("" ::: "memory"); sink(a); return t + G + b; }
```

| variant | `t` | `b` |
| --- | --- | --- |
| `noBarrier` | `$s1` | `$s0` |
| `bareBar` | **`$s0`** | **`$s1`** |
| `memBar` | `$s0` | `$s1` — **byte-identical to `bareBar`** |

**Removing the bare barrier swaps which physical register holds each value.**
By the rule's literal test that makes the permitted construct banned.

Two consequences, and the second is the useful one:

- **The memory clobber is not the risky half.** `__asm__("" ::: "memory")` and
  the bare form were byte-identical here. A runner avoiding the clobber on
  rule grounds is avoiding the wrong thing; the barrier itself is what moves
  allocation. (This came up adjudicating round 46's `func_8003E4B8`, where
  delta used the clobber form. That adjudication was upheld on other grounds
  — see below.)
- **The distinction that survives the measurement is DIRECTEDNESS, not the
  construct's name.** `register T v asm("$N")` and an extended-asm operand
  constraint are **directed**: you name the register you want and you get it.
  A barrier is **undirected**: you cannot choose what the allocator does, only
  perturb it, and you take whatever comes out. The rule's intent is plainly to
  ban pinning — and the literal test over-fires on undirected constructs,
  because *any* perturbation of allocation trips it.

**Working discipline until the wording is settled** (this is a HARD RULE, so
the rewrite is an operator decision, not a head one — escalated round 46):

- A barrier stays allowed, but **say in the report what it did**. If a barrier
  moved the register MAPPING rather than only instruction ORDER, state that
  explicitly rather than leaning on "bare barrier is allowed" as blanket cover.
- **Adding barriers one at a time until a register lands where you want it is
  DIRECTED by construction**, whatever the syntax looks like, and has left the
  allowed category. Stop and file the stall.

**Why this was not caught for forty-five rounds is itself the pattern.** The
rule pairs a *name* with a *test*, and every previous reader took the name as
settling the case for that construct and applied the test only to the two
banned forms. Nobody ran the test on the thing the rule permits. That is the
same shape as this project's screen failures — **a rule states the obstruction
it was written for, and says nothing about whether its own exemptions satisfy
it** — arriving on a HARD RULE instead of on a grep.

**And note what this does NOT do: it does not retire the barrier or reopen any
stall closed with one.** `func_8003E4B8`'s round-46 verdict stands on its own
evidence — a whole-function parameter-colour swap, correctly distinguished
from the prologue-callee-save-store-order class (same final mapping, different
store order) — and the function remains `INCLUDE_ASM`, so its barrier lives in
a preserved body and reaches no build.

## Round 46 (2026-09-15) — five runners plus three re-sends, 17 matches, and a permuter candidate that was lost by writing up too early

Staffing: five units-groups chosen so that **every header-contending pair sat
inside ONE runner's own assignment** (`headercontention.py` reported 8
contending pairs and all 8 were internal), giving zero cross-runner header
exposure. Three worktrees were re-staffed after their runners finished —
`SendMessage` does not exist in this environment, so a re-send is a fresh
agent into the freed worktree, which loses context but keeps a provisioned,
byte-verified tree. **Two of the round's 17 matches and its single best
recovery came out of those re-sends**, which is the third round running that
the §3c substitute has paid.

Matches: `class_3bb8c_n` 8 + 6 (cold carve, first ever worked), `code_179d8_r`
2 (**COMPLETE**), `code_8220` 1. Improved stalls: `func_80017B34`
92→**101**/114, `func_8001E7BC` 136→142/180, `func_8001DDF4` 0→29/199 (first
ever BUILD of a derivation-only report), `func_8003E4B8` 21→23/32.

### A search's TAIL is not protected by committing before you wait (round 46)

Round 18 established that a runner must not end its turn to wait on a bounded
search, because it ends up with zero commits. Round 46 found the variant that
survives that fix, and it cost real words.

`bravo`'s first sitting did everything the commit discipline asks: it ran the
first-ever permuter search on `func_80017B34`, **committed** the improvement it
had (92→96/114), wrote its report recording candidate `output-250-1` (score
250, base 385), reported clean — and then ended its turn to wait. The search
kept running and produced **`output-220-1`, score 220**, in no report and no
commit, in an untracked `permuter-work/` directory that teardown destroys.

Re-staffed to recover it, the candidate **translated to a real +5 words
(96→101/114, zero drift)** — worth more than everything that sitting did
record.

- **Committing before you wait protects YOUR work. It does not protect the
  SEARCH's.** These are different objects and only the first has ever been
  covered by the rule.
- **After a bounded search's timeout fires, read every
  `permuter-work/<fn>/output-*/score.txt`**, not only the candidate you
  happened to notice while it ran. Then translate and measure the best one.
- The permuter-score ordering agreed with the funcdiff ordering *here*
  (220 beat 250), but that is not a law — rounds 40 and 41 have it going both
  ways, so **translate AND measure** regardless.

### Source-shape levers found this round

Each closed or advanced a real function. None is a rule; all are hypotheses to
test per function, and several have measured negatives attached.

- **Same-length `beq`/`bne`-only residue: read what retail sets UNCONDITIONALLY
  in the branch's own delay slot.** If the same value is assigned on *both*
  branch outcomes, the assignment belongs ABOVE the `if`, not inside either
  arm — and no rephrasing of the condition (including its De Morgan mirror)
  can reach it. Closed `func_800286E4` 88/88 on the first build after round 45
  had tried both arm-internal phrasings.
- **Split a combined declaration: `T x = expr;` vs `T x; x = expr;`.** C89
  treats these identically; GCC 2.6.3's allocator does not. Closed
  `func_80028540` 19/19 (moved a value from a spurious extra saved register
  into `$s0`). **Scope, with FOUR independent negatives measured the same
  round** (`func_800357B0` regressed 171→161, `func_800344FC` inert,
  `func_8004BB3C` inert, `func_8004BA40` inert): it applies to **a value
  crossing a call boundary whose timing is already confirmed correct**, NOT to
  whole-function parameter register-colour swaps. This is an ordinary
  statement split — no asm, no constraint, nowhere near HARD RULE 6.
- **A call argument that is the same literal on every branch is not evidence it
  is passed as a literal at the call site.** Where each branch schedules its
  own `ori` independently, assign a per-branch local instead
  (`func_80055A24`).
- **`~x + 1` instead of `-x`** lets the delay-slot filler hoist a safe negate
  (`func_80055874`).
- **Defeat GCC's store-flag collapse of `if (cond) return 1; return 0;`** by
  writing `flag = 1; return flag;` (`func_80055874`, same function, second
  lever).
- **Hoist a global read across a call boundary into a local before a loop** —
  and its negative: inert when there is no call in between (`func_80054F30`).
  Note this is the same call-boundary scoping the declaration-split lever has.
- **Put the longer continuation in the `if` body and the trivial early return
  as the fall-through**, to avoid an extra jump (`func_8005556C`).
- **A ternary's default/override order is not guaranteed** — rewrite as
  explicit imperative assignment (`func_80054758`).
- **Reuse an already-dead local to hold a comparison's boolean** rather than
  declaring a fresh temporary. This was the load-bearing half of the permuter
  zero that closed `func_80017CFC` 107/107.
- **A barrier's POSITION is a separate axis from its presence.** Placing one
  *before* the store that consumes a value forces eager materialisation where
  every prior round had only ever tried *after* (`func_8003E4B8`, 21→23). Read
  this together with the barrier finding above: **state what a barrier did.**
- **Brute-force an unrecognised magic-number divisor through the pinned
  pipeline** rather than trying to recognise it by eye (`func_800549A8`, `/600`).
- **An already-matched callee's signature can be too NARROW**, and the caller
  is where that surfaces. A match does not certify a signature — the wrapper
  rule already says a byte match cannot see a return type, and this is the
  argument-side counterpart.

### Two diagnostic findings worth as much as the levers

- **Frame-size first, on a never-built derivation.** `func_8001DDF4` had a
  structure-only report and no score at all; the first thing that moved it was
  a missing `0x18`-byte stack buffer, not any expression-level work. When a
  body has never been compiled, get the frame right before reading any
  instruction diff — everything downstream is shifted until you do.
- **"The cheapest phrasing still OVERSHOOTS" is a distinct outcome from "no
  lever found".** `func_8001D714` is 2 words short at 141/143; two new
  phrasings came out at 147 and 146. That brackets the gap rather than failing
  to close it, and it tells the next attempt to look between 141 and 146
  instead of re-searching the same space.
- **Leaving another function's stall body live in your unit produces a
  spurious DRIFT warning on the function you are actually measuring.** Delta
  caught this on itself mid-round. It is round 20's attribution hazard
  reappearing from the runner's own edits rather than an inherited sibling's —
  `grep -c '^INCLUDE_ASM' src/<unit>.c` against your starting count is the
  check, and it costs nothing.

### Late addendum — a twelfth lever, arriving after consolidation

`bravo`'s original session re-notified **after** the round was merged, torn
down and pushed, and its summary named a lever the head's consolidation had
missed. It was in the merged report the whole time (`func_80017B34.md`,
"Lever 3"), so nothing was lost — but it was not promoted, and the report's
version is sharper than the summary's:

- **Hoist a "used on every path" field pair into locals PER `if`-statement —
  not across `if`s, and not merged into one shared hoist point.** GCC 2.6.3
  does **not** CSE a struct field read across the *condition* and the *body of
  the same arm*, so spelling `cursor->prev`/`cursor->next` out fresh in each
  arm reloads each operand a second time relative to retail, which schedules
  ONE combined load of both fields immediately before each `if`. A scoped
  block per `if` took `func_80017B34` from 37/114 with 5 words of drift to
  **92/114 with zero drift**.
- **Its discriminator, measured rather than reasoned:** fully hoisting both
  checks to ONE shared pair of locals, read once and reused by both `if`s, is
  a *different and wrong* shape — it also removes retail's second, genuinely
  repeated pair of loads and **undershoots to 105 words, 9 short**. Retail
  re-reads the fields fresh for the SECOND `if`; it just does not re-read
  within a single `if`'s own condition-versus-body. **Scope the hoist to
  exactly one `if` at a time.**

And the general point bravo drew from it, which pairs with this round's
search-tail finding:

- **"N hand rephrasings failed" bounds the rephrasings TRIED, not the
  residue's REACHABILITY.** A bounded permuter pass is still worth running
  after the hand axes are exhausted, provided the `--debug` cost check shows a
  real (non-zero) insertion/deletion count. Four hand rephrasings of the
  expression had all failed to move this residue; hoisting it unconditionally
  — a different axis, not a further rephrasing — is what moved it.

**Two process notes, because the re-notification itself is evidence.**

- **A subagent can re-notify long after its worktree is gone, and it will
  reconstruct the round from `git log` and narrate it as its own.** bravo's
  late report claimed its worktree was *"destroyed mid-round while my bounded
  permuter search was still running."* It was not: the search had already
  terminated on its own 1500s bound, the head's self-excluding sweep showed
  **zero** live permuter processes before anything was touched, and the
  worktree was clean with nothing uncommitted. The re-staffing then *recovered*
  the search output bravo had left uncollected, for +5 words. The account is
  plausible, self-consistent and wrong — which is exactly why
  PARALLEL-RUNS says to count from the branch rather than the summary.
- **A late summary is still worth reading for LEVERS even when its narrative
  is wrong.** This one was wrong about the teardown and right about the
  compiler, and the second half was worth promoting.

## Round 47 (2026-09-16) — the executable finishes carving, and a runner corrects the head

**Round 47's structural result is that Gate 2 is over**: `uncarved.py` reports
zero uncarved game functions. Everything from here is near-miss and stall work.
See `docs/PARALLEL-RUNS.md` Gate 2 for the two new carve hazards found taking
the last three segments.

### The round's most important finding, and it travelled UPWARD

The head's opening broadcast generalised round 46's **five-function**
scaffold-mismatch measurement into *"every recorded permuter negative in the
`class_3bb8c`/`Obj866E8` family is void"*. Runner charlie measured it instead
of obeying it and found **2 of 6, not 6 of 6** — four genuine negatives (183k,
184k, 37k, 34k iterations) that the blanket claim would have discarded.

The head checked charlie's reasoning before adopting it, because the obvious
objection is that charlie read HISTORICAL scaffold records while round 46's
finding came from FRESH ones. **The objection fails on the evidence:** the
confirmed mismatches were already dirty in their historical records too
(`func_8004CD38`: 4 insertions / 5 deletions; `func_8004C93C`: 11/11, noted at
the time as not the clean register-diffs-only signature). The record
discriminates, so the historical read is sound.

**The usable form is a SIGNATURE comparison, not a family label** — scaffold
`--debug --stack-diffs` showing pure register differences with zero
ins/del/reorder, against a real-build residue that is also pure register
identity at exact length, AGREE; nonzero ins/del against zero drift is a
MISMATCH. A sixth mismatch (`func_80054FD8`) then turned up in a *different
unit*, which is what proves the signature travels and the family label does
not. Full write-up in `PARALLEL-RUNS.md`'s Gate 3 box.

Two generalisations worth keeping separately from the permuter question:

- **"The right mechanism at the wrong granularity"** is this project's most
  repeated error shape, and this is its Gate-3 instance. The mechanism was
  real and measured; the granularity was invented.
- **A cheap-model runner disagreeing with the head, with numbers, is the
  protocol working.** The broadcast exists so corrections travel; nothing said
  they only travel downward. Post levers to it and read the replies as
  evidence rather than as compliance.

### Source-shape levers found this round

Each closed or advanced a real function. None is a rule; all are hypotheses to
test per function, and the measured negatives are part of the finding.

- **A `move $sN,$v0` sitting in a `jal`'s OWN delay slot reads the value `$v0`
  held BEFORE that call, never that call's return** — the delay slot executes
  before the jump is taken. Misreading it as "this call's return used twice"
  is unfixable by any amount of local-variable restructuring, because the C
  then describes the wrong value's **lifetime** rather than the wrong shape.
  Closed `func_800118DC` — the game's own `main()` — immediately once
  corrected (delta).
- **GCC 2.6.3's loop optimisations are SYNTAX-GATED, not CFG-gated** (alpha,
  confirmed with five isolated pinned-pipeline variants). Its LICM hoists a
  loop-carried literal comparison into a spare callee-saved register, coming
  out one word SHORTER than retail's recompute-every-iteration shape;
  rewriting only the outer loop as `label: ...; if (cond) goto label;` defeats
  the hoist. **Register pressure is not the knob** — the hoist happened with
  0, 2 and 3 real parameters live. Diagnostic signature: an extra
  `li $sN,<const>` outside the loop paired with a missing `move` inside it.
  Second confirmed instance of syntax-gating after round 45's
  break-past-fall-through finding.
- **…and its own limit, which is the more useful half: the answer does not
  generalise across loops even within ONE function** (alpha). Whether retail
  hoists a retry loop's constant is a per-loop fact you read off that loop's
  own `.s` — look for `li $sN,<const>` above the retry label. A sibling retry
  loop in the same unit matched as a plain `do`/`while`. A second axis showed
  up too: loop **size/complexity**, where an outer loop wrapping two nested
  retry constructs needed the `goto` form while a simpler sibling did not.
- **A local reused across two MUTUALLY EXCLUSIVE branches still perturbs
  register allocation in the branch you did not touch** (alpha, three isolated
  variants). Symptom: an unexplained extra `move $vN,$v0` around a call's
  return in one branch, with nothing wrong-looking in that branch's own code —
  the cause is a *different* sibling branch assigning into a same-named local.
  Give each branch's value its own name even though they are never live
  together.
- **Try REORDERING before `volatile` or a barrier** for round 44's "retail
  re-reads a global our GCC CSEs into an already-live register" class (alpha):
  moving the redundant read to immediately after the assignment that makes the
  two reads provably equal can defeat it, where round 44 measured
  `volatile`/barriers inert for its own instance. Cheaper and non-invasive.
- **`volatile` is the WRONG TOOL for an unwanted address-CSE** (bravo). It
  blocks reordering and elision of the *access*; it does not stop GCC 2.6.3
  caching a global's **address** across a call. A bare `__asm__("")` did not
  move it either (`func_80055258`).
- **Two early exits returning the SAME value still need an explicit
  `goto`-to-shared-label** (bravo). Without it GCC 2.6.3 can tail-duplicate
  the second one into its own inline stub — which presents as a SIZE drift,
  not a near-miss, so it is easy to misattribute.
- **A value only ONE branch consumes may still need computing
  UNCONDITIONALLY**, right after it is derived and before the `if` (charlie).
  Writing it only in the consuming arm drops a `move` retail always emits.
  Sibling of round 46's "value set on both outcomes belongs above the `if`",
  but note the asymmetry: here the *other* branch recomputes its own narrower
  view inline.
- **Whichever arm is textually LONGER needs to be the fall-through** (alpha,
  fifth confirmed instance in one unit). Read it off the `.s`; do not guess it
  from which is written as `if` and which as `else`. Related diagnostic: **a
  DUPLICATED call pair in the diff is worth checking as a branch-polarity bug
  before assuming two independent residues.**
- **An already-matched function's CALL SITE can under-declare what it silently
  forwards** (bravo) — an argument already resident in the right register
  needs no explicit move, so the call site compiles and matches while omitting
  a parameter. This is the argument-side counterpart to round 46's
  callee-side "a matched signature can be too narrow".

### A permuter form adopted on the real oracle, and the comment that keeps it alive

`func_800558F0` (77/77) closed on a permuter candidate that inserts a dead
`i++; i--;` pair to perturb the allocator back into retail's register colours.
It clears round 41's bar: `i` is the initialized loop counter, so this is an
**inert pair and not the uninitialized read round 41 says to reject**, and it
was kept because the whole-image SHA1 verifies — not because the permuter's
scorer liked it. It is also nowhere near HARD RULE 6, which bans forcing
register identity via asm or operand constraints, not ordinary C statements.

**Round 41's requirement to comment such a form AT THE SITE is not decoration,
and it was missed here and added at merge time.** Two obviously-dead lines with
no explanation are what a later reader deletes as cleanup — and the image then
goes red with nothing in the diff explaining why. If you adopt an inert form,
the comment is part of the change.

**Its scope, measured in the same unit and the same round:** the trick is NOT a
blanket answer to register-colour stalls. `func_80054FD8` has the same residue
shape and its scaffold DISAGREES with the real build (1 reordering, 6
insertions, 6 deletions against zero drift), so its search was correctly
declined. Check the signature first.

### Two levers from the re-send, both oracle-verified, neither a match

The re-sent runner closed nothing and still produced the round's two most
transferable findings. **A zero-match pass is not a wasted pass** — this is the
third round to measure that, and it is why re-sends keep being worth the slot.

- **`do { return; } while (0)` around a single-statement early return is NOT a
  no-op for GCC 2.6.3.** It changes basic-block shape enough to alter register
  allocation elsewhere in the function: `func_800344FC` went 61/70 → 62/70,
  narrowing a register-rotation residue from 9 words to 8. It looks like
  syntactic noise and was verified through the real oracle, not the permuter's
  scorer.
  **Its scope, measured the same round:** applying it wholesale to all four
  returns of `func_80034138` regressed that function catastrophically (66/69 →
  1/69, 210365 bytes of drift). So it is a per-return experiment, never a
  whole-function rewrite.
- **A scaffold whose insertion/deletion count is far from 0/0 is a reason to
  RUN check (c), not a reason to decline the search.** `func_8004B030`'s
  scaffold showed 6 insertions / 6 deletions; the real in-tree build showed the
  identical insertions and deletion at the identical score, so scaffold and
  real build AGREE and the harness is representative. Searching on that basis
  gave **19/52 → 22/52**. The discriminator is AGREEMENT between the two
  measurements; zero-ness is merely its commonest form. See `PARALLEL-RUNS.md`
  Gate 3, where the head's own table had to be corrected for exactly this.

### Distinguishing "spent" from "hard", and saying which

The re-send re-confirmed five `code_179d8_g` and `class_3ac78` near-misses as
**already spent** rather than re-litigating them — reading 600-to-1255-line
reports and reporting which routes were dead, including one
(`func_8002B3F4`) where the permuter HAD found a genuine zero in round 19 that
was rejected because it corrupts a sibling's `.bss` address. It also caught
itself re-deriving a negative round 39 had already recorded, with the same
260994-byte drift figure, and **said so instead of padding the report**.

That is the disposition round 33 asked for and it is worth naming as a skill:
**"this was exhausted before I arrived, and here is the evidence" is a
result.** It lets the next round rank on COST rather than re-deriving it, which
is the one thing a title line cannot carry.

### Negatives worth as much as the levers

- **Four `code_179d8_h`/`code_179d8_j` near-misses fail permuter check (b)
  hard** — insertion/deletion pairs of 4/3, 5/4, 5/5 and 17/22 (the last being
  ~40% of its own instruction count) — and all four searches were declined on
  that evidence (delta). These are control-flow-shape and
  register-allocation-strategy gaps, not expression-tree rewrites a
  source-mutation search is built to close. **The permuter's own `--debug`
  diff independently named the same residues five prior hand attempts had
  converged on**, which makes check (b) a diagnostic and not only a gate.
- **Round 46's twelfth lever (field-pair-per-`if` hoist) is inapplicable to
  all six `class_3bb8c` residues** (charlie) — none has that shape. Recorded so
  the next round does not re-litigate it.

### A report can claim a preserved body that does not exist

`func_8005511C.md` and `func_80054FD8.md` both described a body preserved in
`#if 0` that **was never actually inlined as compilable source**. This is the
failure CLAUDE.md warns about — a body that lives only as a description — and
it is invisible to every static reading, because the report is internally
coherent. **Only the instruction to BUILD every inherited body once catches
it** (round 33's rule, earning its keep again). Both fixed this round.

### An operational hazard: `make clean` deletes `asm/`

The head ran `make clean` in the main checkout to get a warning census off a
from-scratch build. `clean` is `rm -rf $(BUILD_DIR) asm $(GAME).ld`, so it
takes the generated disassembly with it and the next build dies on
`can't open asm/nonmatchings/<unit>/<fn>.s`. It reads exactly like somebody
broke the tree and is attributable to no commit — the same signature Gate 0
documents for a stale `asm/` after a pull. **`make extract` restores it**, and
`clean` is one of the four hook-permitted targets precisely because this is
recoverable, but do not run it in the shared checkout without expecting the
red build that follows.

What the census bought, which is worth knowing independently: a genuinely clean
build emits **111 warnings**, including 9 implicit function declarations and 6
`type mismatch with previous implicit declaration`. **The incremental build
hides every one of them**, so nobody iterating on one function ever sees them.
They are latent rather than urgent — the image is byte-exact — but an implicit
declaration means the compiler is not checking that call at all.
