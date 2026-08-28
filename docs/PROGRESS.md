# Progress log

One entry per session or round. The head writes these during consolidation.
Counts come from `python3 tools/progress.py`; dated entries are allowed to go
stale, prose elsewhere is not.

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
