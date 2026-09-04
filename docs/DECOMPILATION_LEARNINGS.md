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

## Build hygiene (proven, the hard way)

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
  `func_800596E8`)
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
