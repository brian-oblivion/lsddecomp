# Matching guide

The per-function loop, in detail, plus the current per-unit state. Read
CLAUDE.md first for the hard rules and the toolchain facts.

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

## Unit state

Keep this current: the head reads it in Gate 1, and a number in `progress.py`
cannot express "large body, deep reconstruction, low cold-runner yield".

| unit | queued | state |
| --- | --- | --- |
| `StageGrid` | 2 | 3 matched. Remainder is `GetStageChunkFromMood` / `GetMoodFromStageChunk` — both need the `STAGE_CHUNK_MOODS` / `STAGE_GRID_DIMENSIONS` data slots understood, and m2c already produces a clean body for the second. Good warm start. |
| `DreamSys` | 118 | Untouched. The game's core state machine — dream timer, day advance, mood graph, flashbacks. Symbol names are descriptive (see `config/symbols.slps01556.lsdde.txt`) and `include/DreamSys.h` already carries a struct guess. The largest carved queue; split it across runners by address range. |

## Uncarved ground

1230 functions still sit inside monolithic `asm` segments. Largest first:

| segment | functions | note |
| --- | --- | --- |
| `class_39e08` | 415 | Biggest single block. `New_*`/`*__*` naming suggests C++-style dispatch — expect splat under-splits (see PARALLEL-RUNS Gate 2). |
| `code_179d8` | 274 | |
| `code_2c054` | 181 | |
| `Entity` | 142 | |
| `code_8220` | 56 | |
| `code_d294` | 55 | |
| `code_55dd4` | 36 | |
| `code_171e0` | 27 | |
| `code_4cd08` | 17 | "DreamAux" per lsddecomp's notes. Small — a good first carve. |
| `code_1677c` | 15 | Small; another good first carve. |
| `class_16334` | 10 | Smallest. |

The `psyq_*` segments (724 functions) are Sony SDK code. They are excluded from
the game-code denominator and should be left until the game's own code is done —
matching them proves nothing about this game.

## Permuter

Not yet set up on this project. When a near-miss is worth it,
`tools/decomp-permuter` is cloned and `tools/permuter_settings.toml` points at
the Psy-Q compiler. A permuter zero is a LEAD: translate it to idiomatic C and
re-verify with funcdiff. If only undefined-behaviour or duplicate-arm forms
reach zero, mark the class permuter-exhausted in the report and move on.
