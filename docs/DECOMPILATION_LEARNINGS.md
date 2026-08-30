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

### Confirmed on this game (each backed by a byte-exact match)

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

### Still unconfirmed here

Candidates from other GCC 2.x projects — treat each as a thing to test:

- Member loads hoisted into a local at the top of a loop.
- A struct pointer to a global block, rather than several separate globals.
- Comma expressions and assignment-in-condition, which 2.x schedules
  differently from the separated form.

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

- **What is the class-table header word at `+0x000`?** Not a pointer, and it
  varies per class (`0x1F34`, `0x34`, `0x1130`, `0x00011144`, `0x0E03`…). Some
  values look like packed fields — the low byte is often `0x03`, `0x44`, `0x64`.
  A class id, a flags word, or an instance size. Answerable by cross-referencing
  the values against what the framework's own code at `class_16334` /
  `code_179d8` does with them; nobody has looked.
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
