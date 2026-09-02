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
grep -nE 'Error [0-9]|error:|parse error|undefined reference' /tmp/b.log | head -8
.venv/bin/python3 tools/funcdiff.py <func>
```

`build exit=` is the only line that decides whether the score means anything.
See CLAUDE.md "The three ways a score lies".

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
  Nine functions across two units burned attempts on this in one round.

- **Branch TARGETS disagree, not just delay slots.** This one is a
  discriminator, not a residue, and it outranks everything else in this list.
  A differing delay slot is a scheduler choice; a differing branch *target* is a
  different control-flow graph, and a different CFG always comes from the
  source. **Line up the branch targets before classifying anything as a
  scheduling stall, a delay-slot choice or a toolchain lead.** In round
  2026-08-30-a this turned a confidently-filed "unreachable compiler-internal"
  stall at 16/42 into 41/42 with a one-line source change, and a second runner
  credited it with closing a function outright (10/69 -> 69/69).
- **One instruction short, everything after it shifted.** Suspect the
  `addiu_at` blocker before suspecting your C. Check with
  `grep -n 'addiu *\$at, *\$at, *%lo' asm/nonmatchings/<unit>/<func>.s`;
  a hit means STOP and file the stub (docs/research/addiu-at-blocker.md).
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
reconstruction, low cold-runner yield". Two things tell you that, and both are
live rather than transcribed:

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

Not yet set up on this project. When a near-miss is worth it,
`tools/decomp-permuter` is cloned and `tools/permuter_settings.toml` points at
the Psy-Q compiler.

**The first target is chosen: `new_class_6d3c8`.** It is the `New_X` allocator
shape (malloc → null check → constructor through slot `+0x008` → return the
allocation), it stalls on one delay-slot residue that manual reshaping and a
`__asm__("")` barrier both failed to close, and roughly 60 classes share the
shape. One source form that closes it plausibly unblocks every `New_X` in the
game, which is a far better return than permuting an isolated near-miss. A permuter zero is a LEAD: translate it to idiomatic C and
re-verify with funcdiff. If only undefined-behaviour or duplicate-arm forms
reach zero, mark the class permuter-exhausted in the report and move on.
