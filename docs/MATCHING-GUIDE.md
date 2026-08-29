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
- **A `lui`+`lw` pair where retail has a single `lw ..($gp)`.** This is the
  gp-relative blocker, and **no source-level reshaping will move it** — the
  addressing mode comes from the toolchain's `-G` value. Check for it before
  spending attempts: `grep -l 'gp_rel' asm/nonmatchings/<unit>/*.s`. A hit
  means STOP and file the report; see `docs/research/gp-relative-blocker.md`.
  Nine functions across two units burned attempts on this in one round.

## Unit state

Keep this current: the head reads it in Gate 1, and a number in `progress.py`
cannot express "large body, deep reconstruction, low cold-runner yield".

State after round 2026-08-29-a (5 runners, all killed mid-round by an
account-wide session limit; their work was salvaged by the head per
docs/PARALLEL-RUNS.md 4c).

| unit | queued | fresh | state |
| --- | --- | --- | --- |
| `DreamSys` | 105 | 104 | **16 matched.** Round 2026-08-29-a took the accessor range `0x80059310`–`0x800595A8`: 13 matched, and `func_8005942C` preserved as a mid-attempt snapshot (19/56 with heavy outside drift — *not* a near-miss; needs the globals near `0x80087EE8` named first). `include/DreamSys.h` now carries real field offsets and callback slots. The remaining 104 are untouched; still too big for one runner, so keep assigning named ranges. |
| `code_55dd4` | 34 | 0 | **DELIBERATELY UNWORKED** — banked for scheduling, not difficulty. First pick next round. Carved 2026-08-28; its tail was data (see the yaml at `0x57028`). |
| `Entity` | 25 | 0 | **DELIBERATELY UNWORKED** — banked for scheduling. Carved 2026-08-28, the first 25 of a 142-function block split at `func_8005DE18`; remainder is `Entity_b`, still `asm`. |
| `code_171e0` | 22 | 14 | **5 matched, 8 stalled.** All 8 stalls are the gp-relative blocker (`docs/research/gp-relative-blocker.md`) — a real toolchain escalation, verified by the head, not a runner misclassification. Do not staff those 8 again until the operator rules on `-G`. The other 14 are ordinary fresh ground. |
| `code_4cd08` | 14 | 14 | **3 matched, 0 stalls.** The cleanest unit of the round — the runner matched everything it attempted. lsddecomp called this "DreamAux". Owns the `0x206C` rodata slot (jump tables). |
| `code_1677c` | 11 | 10 | **4 matched, 1 stalled.** The stall is `new_class_6d3c8`, a single delay-slot residue in the `New_X` allocator shape that ~60 classes share — the best first permuter target in the project. |
| `class_16334` | 4 | 3 | **6 matched, 1 stalled.** The stall (`func_80025C30`) is the gp-relative blocker again, found independently of `code_171e0`'s. |
| `StageGrid` | 2 | 0 | **DELIBERATELY UNWORKED** — a 2-function queue is too thin to be worth a runner's provisioning cost. 3 matched. Remainder is `GetStageChunkFromMood` / `GetMoodFromStageChunk`; both need the `STAGE_CHUNK_MOODS` / `STAGE_GRID_DIMENSIONS` data slots understood. Head or warm-session work. |

## Uncarved ground

1100 functions still sit inside monolithic `asm` segments. Largest first:

| segment | functions | note |
| --- | --- | --- |
| `class_39e08` | 415 | Biggest single block; the class framework plus a large hierarchy. **Contains 13 PSX BIOS call stubs** (`jr $t2` with the vector in `$t2` and the call number in `$t1` — `0xB0`/`0x33` is BIOS `malloc`). Those are NOT expressible in C and will need an `hasm` segment or a literal-`.word` disposition; decide that at carve time, not when a runner hits one. Also holds 19 `jr $reg` dispatchers. |
| `code_179d8` | 274 | Owns the `0xFD8` rodata slot — 179 text pointers, so it will need attaching when carved. |
| `code_2c054` | 181 | |
| `Entity_b` | 117 | The remainder of Entity after the 2026-08-28 slice. Carve the next ~25 the same way. |
| `code_8220` | 56 | Owns the `0xA8C` rodata slot. |
| `code_d294` | 55 | |

The `psyq_*` segments (724 functions) are Sony SDK code. They are excluded from
the game-code denominator and should be left until the game's own code is done —
matching them proves nothing about this game.

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
