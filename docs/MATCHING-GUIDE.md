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

## Unit state

Keep this current: the head reads it in Gate 1, and a number in `progress.py`
cannot express "large body, deep reconstruction, low cold-runner yield".

State after round 2026-09-01 (3 runners, 6 passes; alpha took three passes on
one unit under protocol 3c, bravo and charlie each exhausted theirs).

**98 -> 154 matched.** All 92 functions assignable at round start were
pre-screened for both toolchain blockers and all 92 were clean — the first round
with an entirely negative pre-screen. The 66 stall classifications inherited
from round 4 were mechanically re-audited against their own disassembly and all
66 held, so the `fresh` column below is trustworthy as a ceiling.

**Three units are now DRY and four more are entirely blocked. Only `DreamSys`
has fresh ground, and its 27 remaining are all 35+ instructions.** One unit
cannot support three runners — **the next round should be a carve, not runners.**

| unit | queued | fresh | state |
| --- | --- | --- | --- |
| `DreamSys` | 55 | 27 | **66 matched — the round's engine**, 41 of them this round across three passes by one runner. `include/DreamSys.h` is now substantial: `sizeof` corrected to 0x928 (recovered from the allocator's own `ori $a0, $zero, 0x928`), the mood-graph and flashback struct tails mapped, ~16 vtable slots resolved. The cheap seam is EXHAUSTED — every one of the 27 remaining is 35+ instructions and several are 60-87, so this is large-body reconstruction work, not tail-pass work. Size it accordingly: fewer functions per runner, not more. 28 of its queue are stalled/blocked and stubbed. |
| `Entity` | 10 | 0 | **15 matched. DRY.** All 10 remaining are `addiu_at`-blocked and stubbed. Do not staff until that blocker is resolved. Pairs with an `Entity_b` carve, which is where its future is. |
| `code_55dd4` | 6 | 0 | **30 matched. DRY** — fully worked across four passes over two rounds. Its 6 stalls are all structural (none toolchain-blocked), so all 6 are in principle reachable: `func_80066340` (252/258) and `func_80065A5C` (30/33) are the best of them. Slot `+0x134` returns `u8 *`, NOT `void` — corrected this round; the earlier `void` came from a call site that discarded the result. |
| `code_4cd08` | 10 | 0 | **7 matched, 10 blocked.** Every remaining function is toolchain-blocked and stubbed. Do not staff until a blocker is resolved. |
| `code_171e0` | 15 | 0 | **12 matched, 15 blocked/stalled.** 14 gp-relative blocked, plus `strcat` at 41/42 (one redundant `move`, permuter target). No workable ground. |
| `code_1677c` | 2 | 0 | **13 matched.** Both remainders are stalls: `new_class_6d3c8` (23/24) and `func_80026698` (53/57), both permuter targets with bodies preserved. Both re-audited this round and both classifications hold — `new_class_6d3c8` in particular has had 20+ attempts and its branch targets verified; do not re-staff it, permute it. |
| `class_16334` | 2 | 0 | **8 matched.** Fully decompiled except its two gp-relative-blocked functions. **The "8" is a MATCHED count and this unit has ZERO fresh ground** — it has now been mistaken for available work in two consecutive round briefs. |
| `StageGrid` | 2 | 0 | **3 matched.** Its two remainders are `addiu_at`-blocked. Understanding the `STAGE_CHUNK_MOODS`/`STAGE_GRID_DIMENSIONS` data would not help; both index those tables through the fully-resolved `$at` form. |

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
