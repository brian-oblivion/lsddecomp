# LSD: Dream Emulator (PSX) — matching decompilation

Matching decompilation of *LSD: Dream Emulator* (PlayStation, 1998, Asmik Ace /
OutSide Directors Company, SLPS-01556). The goal is C source that compiles
byte-for-byte to the retail `SLPS_015.56` executable.

## HARD RULES — read before doing anything

1. **NEVER edit `check.sha1` or `build.sha1`.** They hold the ground-truth
   SHA1 `76322eeade5ebb22dca57fdeac7d68c30f06308d`, which is what the whole
   project verifies against. Editing one to make a build "pass" is not a
   match, it is a deleted oracle — and it is nearly invisible in review,
   because one hex string changes and everything goes green. A hook blocks it.
2. **All builds go through `./build-and-verify.sh`.** A bare `make` produces
   bytes and says nothing about whether they are the right ones; every score
   read after one is unanchored. A hook blocks in-repo `make` for anything but
   `extract`, `progress`, `format` and `clean`.
3. **NEVER commit the executable or a disc image.** `disk/SLPS_015.56` and any
   `.bin`/`.cue`/`.iso` are gitignored and stay that way. Bring-your-own-disc.
4. **NEVER edit anything under `asm/`.** It is generated from the executable by
   splat and rewritten on every `make extract` — `make extract` deletes
   `asm/nonmatchings/` outright. To rename a symbol, edit
   `config/symbols.slps01556.lsdde.txt`; to change segmentation, edit
   `config/splat.slps01556.lsdde.yaml`; then re-extract. A hook blocks it.
   (If a future segment holds assembly that was *never* compiled from C, it
   becomes an `hasm` segment: authored source, committed, editable. There are
   none yet, and adding one means updating the yaml, `.gitignore` and
   `.claude/hooks/block-asm-edits.sh` together.)
5. **The toolchain is PINNED.** Compiler, binutils, flags, `maspsx` version —
   none of it is a knob to turn while matching. A suspected toolchain problem
   is something to *report with a reproducer*, never something to experiment
   with mid-round. See "Escalate, do not experiment" below.
6. **Never fix a register mismatch with `register T v asm("$N")` or an
   extended-asm operand constraint.** Both are banned project rules, not
   judgement calls. A bare `__asm__("")` scheduling barrier is allowed. The
   test: if removing it changes WHICH REGISTER holds a value, it is banned; if
   it only changes instruction ORDER, it is allowed. A register-identity
   mismatch is a STALL — write the report.

## Where the project is

**Measure it, do not read it here.** Counts, percentages and per-unit queues
change every round, so this file deliberately quotes none of them:

```sh
python3 tools/progress.py
```

**No number about project state belongs in this file.** A count written here is
correct for one round and quietly wrong for every round after, and the reader
cannot tell which they are in. Worse than the numbers are the *conclusions*
drawn from them — "the next round must carve", "only one unit has fresh
ground", a list of the biggest uncarved blocks. Those read as standing
instructions, so a stale one sends a whole round to redo finished work or to
carve a segment that no longer exists under that name. If you catch yourself
adding a figure below, put it in `docs/PROGRESS.md` (a dated log, where being a
snapshot is the point) or derive it from a command.

How to read `progress.py` correctly:

- **The library split is derived, not hardcoded.** Any subsegment named
  `psyq_*` in the splat config counts as Psy-Q SDK everywhere, and is excluded
  from the game-code percentage.
- **Not every matched function was work.** Some bodies are just `jr $ra; nop`
  and splat generated them itself.
- **The `fresh` column cannot see toolchain blockers** — screen candidates
  yourself, below.
- **A unit has exactly one owner**, so the number of carved units is the
  ceiling on parallel runners. The `unit` table is therefore the staffing plan:
  if only one unit has `fresh` left, the next round has to carve before it can
  run more than one runner.

The build verifies, and a clean `./build-and-verify.sh` takes under a second —
which is what makes many parallel runners cheap here.

## Open toolchain blockers

Two are open. Both are escalated with reproducers and corpus censuses, both are
the operator's call, and neither is something to experiment with:

- `docs/research/gp-relative-blocker.md` — the `-G` experiment was run with
  authorisation and REJECTED.
- `docs/research/addiu-at-blocker.md` — not tested; the obvious version bump is
  not the remedy. A corpus census attached to it shows the `nop_mflo_mfhi` flag
  inside it is the right MECHANISM at the wrong GRANULARITY, which sharpens the
  blocker without resolving it.

Between them they account for a large fraction of everything queued, so screen
any candidate function before spending attempts on it:

```sh
grep -n 'gp_rel' asm/nonmatchings/<unit>/<func>.s
grep -n 'addiu *\$at, *\$at, *%lo' asm/nonmatchings/<unit>/<func>.s
```

A hit in either means the function is blocked. `progress.py`'s `fresh` column
cannot see this, which is why every blocked function carries a stub report.

**A `%lo(jtbl_*)` hit counts. A dense `switch` is blocked exactly like an
indexed global, and this is the one place the screen looks like it is
over-reporting when it is not.** The temptation is obvious: a jump-table
dispatch loads a CODE address and jumps (`lw $v0, 0x0($at)` then `jr $v0`),
an indexed global loads a DATA value, so they look like two constructs and
the `jtbl_*` one looks harmless. Round 10's head acted on that reading,
"corrected" this grep to `| grep -v '%lo(jtbl'`, told five live runners to
use the corrected version, and had to retract all of it.

They are ONE construct at the layer that decides. cc1 emits the same generic
pseudo-op for both and expresses no opinion on addressing:

```
lbu $2,D_80089EAC($4)     # indexed global
lw  $2,$L13($2)           # switch jump table
```

The folding happens in maspsx, BELOW cc1, which cannot tell them apart and
does not try. Through the pinned pipeline a dense 10-case switch comes out
FOLDED (`lui $at` / `addu $at,$at,$v0` / `lw $v0,%lo(...)($at)` — three
instructions) where retail has the UNFOLDED four. Reproducer, if you want to
see it yourself rather than trust this paragraph — it is under a second:

```c
extern int sink(int);
int probe(int sel) {
    switch (sel) {   /* ten dense cases, each `return sink(N);` */
    case 0: return sink(10);
    /* ... cases 1..9 ... */
    }
    return -1;
}
```

The lesson generalises past this grep: **a blocker's SCOPE is measured, not
reasoned.** The two constructs differ in every way that is visible in the
disassembly and in no way that matters to the tool doing the expanding.

## Carving new ground

Uncarved code sits in monolithic top-level `asm/*.s` segments. List them
biggest-first — the names change as carving proceeds, so derive them rather
than trusting any list:

```sh
for f in asm/*.s; do b=$(basename "$f" .s); case "$b" in psyq_*|header) continue;; esac
    printf '%6d %s\n' "$(grep -c '^glabel' "$f")" "$b"; done | sort -rn
```

Carving a unit out of one of those is Gate 2 in `docs/PARALLEL-RUNS.md`. Do NOT
budget time for under-split hunting — it was predicted, measured, and does not
happen (894 of 894 table-dispatched entry points already have symbols). Do
budget for the two carve failures that ARE routine: an orphaned rodata
jump-table slot, and a segment whose tail is data. Both are documented in
Gate 2.

## Key technical facts (derived from the binary)

- **Plain PS-X EXE, no overlays, no compression.** 505856 bytes = a 0x800-byte
  header plus one contiguous image loaded at vram `0x80010000`. Every byte of
  code in the game is in this one file, which makes it dramatically simpler
  than a cartridge project — there is no bank switching, no demand paging and
  no asset compression to reverse first.
- **`file offset = vram - 0x80010000 + 0x800`.** splat's instruction comments
  carry the file offset directly: `/* 39CD8 800494D8 E8FFBD27 */` is
  `FILEOFS VRAM WORD`.
- **Little-endian MIPS R3000, no FPU.** Software floating point (`-msoft-float`
  is cc1's default here). `$gp` is `0x8008A808`, and `%gp_rel` addressing shows
  up constantly — a `lw $v0, %gp_rel(sym)($gp)` is an ordinary global access,
  not something exotic.
- **The compiler is GCC 2.6.3, Psy-Q patched.** cc1's own banner says
  `GNU C 2.6.3 [AL 1.1, MM 40] Sony Playstation compiled by GNU C`. Flags:
  `-mips1 -mcpu=3000 -O2 -G0 -funsigned-char -fno-builtin -mno-abicalls`.
- **`maspsx` sits between cc1 and gas, and it is load-bearing.** Sony's ASPSX
  assembler expanded macros (`div`, `li`, …) differently from GNU as, and those
  expansions are part of the retail bytes. Without maspsx the executable does
  not even assemble.
- **It is C89, and 2.6.3's `cpp` is strict about it.** `//` comments are a
  parse error — every header in `include/` uses `/* */`. `cpp` also rejects
  `-fno-builtin`, which is a cc1-only flag; that asymmetry is why the Makefile
  has separate `CPP_FLAGS` and `CC_FLAGS`.
- **`char` is unsigned** (`-funsigned-char`). A signed byte load is `s8`.
- **It is plain C with a HAND-ROLLED class framework — not C++.** Proven, with
  a reproducer, in `docs/research/class-framework.md`: constructors are called
  *through* the method table (impossible for a C++ compiler), entries are 4
  bytes where GCC 2.6.3's own C++ emits 8, and a scan of the whole executable
  finds **zero** compiler-generated vtables against **128** flat pointer tables.
  Do not reach for `cc1plus`. Methods are ordinary C functions with an explicit
  `this` first parameter, the table pointer sits at object offset 0, and the
  tables are data. Resolve a slot with `tools/classtable.py`, never by counting.
  **The suggestive symbol names are FirecatFG's hypotheses, not evidence** —
  they look like C++ because someone who suspected C++ chose them.

## The decompilation loop

1. Pick an `INCLUDE_ASM(...)` in a `src/*.c` unit. (Or carve new ground first —
   see Gate 2 in docs/PARALLEL-RUNS.md.)
2. Read `asm/nonmatchings/<unit>/<func>.s`. Optionally seed with m2c:

   ```sh
   .venv/bin/python3 tools/m2ctx.py <unit> --sig 'void <func>(Foo *a, s32 b)' --run
   ```

   **Always pass `--sig`.** m2c types arguments from a declaration of the
   target, and `INCLUDE_ASM` leaves none, so without it every argument comes
   out `void *arg0` and every field `->unk8` no matter how good your context
   is. A `padNN[0x..]` access in the output means the struct you guessed is
   missing a field at that offset — a wrong guess is itself a measurement.
3. Write C in place of the `INCLUDE_ASM`. Keep declarations at block top (C89),
   real struct fields rather than pointer arithmetic, and **keep every function
   in the unit in strict ROM-address order** — writing one out of order
   miscompiles the whole image, the link succeeds, and previously-matched
   functions appear to regress with diffs that look unrelated to your change.
4. Verify. Chain the two so you cannot read a score from a failed build:

   ```sh
   ./build-and-verify.sh > /tmp/b.log 2>&1; echo "build exit=$?"; \
   grep -nE 'Error [0-9]|error:|parse error|undefined reference' /tmp/b.log | head -8; \
   .venv/bin/python3 tools/funcdiff.py <func>
   ```

   Working in parallel? Use `/tmp/<name>_b.log`, not the shared `/tmp/b.log`
   — see docs/PARALLEL-RUNS.md.

   The only line that decides whether the number is meaningful is `build
   exit=`. To *read* a diff rather than score it, use asm-differ:
   `.venv/bin/python3 tools/asm-differ/diff.py <func>`.
5. Iterate. On a stall, restore the `INCLUDE_ASM` and write the match report
   (see below). **No score short of byte-exact justifies leaving C in `src/`** —
   it fails the whole-file SHA1 the moment it is merged.
6. On a match: keep the C idiomatic, name things sensibly, add new struct
   knowledge to `include/`, write the report, commit.

## The three ways a score lies

`funcdiff.py` guards two of these mechanically and will exit 2 rather than
hand you a number it does not trust. Know all three anyway — the guards are a
backstop, not a substitute for reading `build exit=`.

1. **Stale build.** funcdiff reads the BUILT file. A failed compile *or a
   failed link* leaves the previous build in place, so the score comes from the
   last build that succeeded — and if that build had the function as
   `INCLUDE_ASM`, it reports a FULL MATCH with no diagnostic anywhere. The
   reason knowing this is not enough: **a stale number is often plausible and
   self-consistent**, equal to a real score you measured minutes earlier.
   funcdiff compares mtimes and warns.
2. **Still `INCLUDE_ASM`.** The build succeeds, the output is fresh, and the
   score is still a full match — because an `INCLUDE_ASM` function contributes
   retail's own assembled bytes, so this compares retail against retail.
   Nothing fails and there is nothing to grep for. It fires most often when
   spot-checking someone else's stall claim *after a merge*, because merging
   restores the `INCLUDE_ASM`. funcdiff warns.
3. **Address drift.** If your C is a different length, everything after it
   shifts and the per-function window no longer means what it says. funcdiff
   reports how many bytes differ OUTSIDE the range; a non-zero count there
   makes the in-range score untrustworthy. `build-and-verify.sh` is the oracle.

## Standing checks

```sh
python3 tools/progress.py          # counts, and warns about stale asm/ files
python3 tools/srcpath.py           # unit names unique, layout sane
./build-and-verify.sh              # THE oracle
```

## Match reports

`docs/match-reports/<func>.md`, **one file per function you touch — matched
ones included, not just stalls.** `tools/progress.py` decides whether a queued
function is a documented STALL or untouched FRESH ground purely by whether that
file exists. A stall with no report is counted as unworked, and the next round
staffs someone straight back onto it to re-derive what you already established.

A report is a file, not a `.c` comment and not a final chat message — those die
with the session. If you preserved a near-miss body, **inline it in the report
as literal source, with every declaration it needs**, positioned where it would
compile. A body that lives only as a path is a body that exists in exactly one
checkout and travels to no worktree, machine or clone.

Preserve a stalled body in `#if 0 ... #endif`, **never in a `/* */` block
comment**: inside one, a line whose first non-space character is `*` is
ambiguous between the comment's continuation marker and a C dereference, and
nothing local tells them apart.

## Escalate, do not experiment

Toolchain or flag changes of any kind are an operator escalation with evidence
attached, never something to try mid-round. And **never escalate a toolchain
lead you have not tried and failed to reproduce in isolation** — extract the
construct into a self-contained `.c` and run it through the pinned pipeline:

```sh
tools/gcc263/cpp -Iinclude -Iinclude/psyq -undef -lang-c -nostdinc -Dmips -D__GNUC__=2 /tmp/t.c \
  | tools/gcc263/cc1 -mips1 -mcpu=3000 -quiet -G0 -O2 \
  | .venv/bin/python3 tools/maspsx/maspsx.py --aspsx-version=2.34 --dont-force-G0 --expand-div \
  | tools/binutils/bin/mipsel-linux-gnu-as -march=r3000 -EL -no-pad-sections -G0 -o /tmp/t.o
tools/binutils/bin/mipsel-linux-gnu-objdump -d /tmp/t.o
```

Under a second per variant. **A minimal reproducer that FAILS to reproduce is
itself the result** — it proves the toolchain innocent and the trigger
contextual, which is usually the more useful finding.

Note the permuter mutates C source under the pinned toolchain; that is not a
toolchain change.

## Commands

```sh
./tools/setup.sh                   # one-command bootstrap for a fresh clone
./build-and-verify.sh              # THE canonical build + verify
make extract                       # regenerate asm/ from the executable
python3 tools/progress.py          # where the project is
python3 tools/funcdiff.py <func>   # per-function score
python3 tools/classtable.py --scan # the 60 class method tables
python3 tools/classtable.py <t> --vs <base>   # what a subclass overrides
tools/setup-worktree.sh <name>     # provision a parallel runner
```

## Layout

| path | what |
| --- | --- |
| `disk/SLPS_015.56` | the retail executable. Yours, never committed. |
| `config/` | splat segmentation + symbol names |
| `include/` | project headers and the Psy-Q SDK headers |
| `src/` | carved C units — where the work happens |
| `asm/` | generated disassembly. Never edit, never commit. |
| `tools/` | the toolchain and the workflow tools |
| `docs/` | the guides; `docs/match-reports/` is the durable record |

## Workflow references

- `docs/PARALLEL-RUNS.md` — running several matching sessions at once under a
  head agent. **Read this before spawning anything.**
- `docs/MATCHING-GUIDE.md` — the per-function loop in detail, and how to read
  a unit's real state instead of a transcribed one.
- `docs/DECOMPILATION_LEARNINGS.md` — source-shape idioms and open questions.
- `docs/PROGRESS.md` — the running session log.
- `CREDITS.md` — this project stands on FirecatFG's lsddecomp for its
  segmentation and symbol names. Read it.
