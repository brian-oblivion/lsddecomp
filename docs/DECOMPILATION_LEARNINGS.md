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

### New residue classes opened this round (not yet closed)

- **"Identical assignment reaching different merge points."** GCC tail-merges
  two identical `doDetach = 1;` statements that retail keeps separate.
  Unreached by reshaping, by `__asm__("")` barriers, or by the argument-register
  test. (`func_8005DBF0`, 72/74 — the round's closest near-miss)
- **Magic-multiply constant load POSITION.** A GCC-synthesized multiplier
  constant whose load placement has no direct C-source counterpart; two
  symmetric 3-word clusters, unmoved by six reshapes and by a late barrier
  (which made it worse). Flagged as a candidate "not a source-shape question"
  case. (`func_80066340`, 252/258)
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
- **A patchy `--vs` diff may mean you picked the wrong ancestor, not that
  there is no inheritance.** If a derived table's *high* slots line up
  byte-for-byte with some other candidate table's high slots, re-run `--vs`
  against that one; the longer identical run is the real parent. This is how
  `code_1677c`'s class was found to descend from `D_8006E4F0`, an intermediate
  between `BasicClass` and itself, rather than directly from `BasicClass`.
  (`func_80026108`)
- **`New_X` allocator wrappers are a recurring shape with a known residue.**
  malloc → null check → constructor through the class's own slot `+0x008` →
  return the allocation regardless of the constructor's return. The first one
  attempted (`new_class_6d3c8`) stalled on a single delay-slot residue that no
  `if`/`goto`/temp-variable reshaping and no `__asm__("")` barrier closed.
  Since roughly 60 classes share this shape, this is the highest-value single
  target for the first permuter round: one source shape that closes it
  plausibly unblocks every `New_X` in the game. Do not re-derive the same 20+
  manual attempts per class in the meantime.

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
