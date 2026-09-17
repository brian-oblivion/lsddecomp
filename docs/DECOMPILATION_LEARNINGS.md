# Decompilation learnings — idiom sheet

Idioms, build hygiene and verdict classes, compressed to the discriminator.
**Full history is `docs/archive/DECOMPILATION_LEARNINGS-full-2026-09-16.md`**,
referenced here as `(a §"heading")` and never repeated.

**Adding to this file.** One idiom = one entry, max 12 lines: a bold rule, its
discriminator (when it applies and when it does NOT), the measured evidence in
one clause, and a pointer to the archive section or `docs/PROGRESS.md` round
holding the story. No new narrative here. Nothing is added during a parallel
round: runners write `### Proposed learning` in their report, the head promotes
after merging.

## 1. Toolchain facts (proven)

- **GCC 2.6.3 Psy-Q + maspsx + binutils 2.43.1 reproduces retail byte-for-byte.**
  `-mips1 -mcpu=3000 -O2 -G0 -funsigned-char -fno-builtin -mno-abicalls`; maspsx
  `--aspsx-version=2.34 --dont-force-G0 --expand-div` plus the flags below.
  PINNED: a suspected problem is an escalation with a reproducer. (a §"Toolchain
  facts")
- **`cpp` and `cc1` take different flags; C89 only.** `-fno-builtin` is cc1-only
  and `cpp` exits 33 on it. A `//` comment is a parse error reported far from the
  comment. `-no-pad-sections` is required or gas pads `.text`. splat owns
  `include/*.inc`; hand edits die on extract. (a §"Toolchain facts")
- **`include/psyq/INLINE.H` and the LIBGPU `set*` macros are INERT — use
  `include/gte.h`.** Eight vendored headers are CRLF and `cpp` splices `\` only
  before LF, so 1134 macros expand to nothing: valid C, no warnings, no
  instructions. Hence **a negative about a MACRO counts only once you have proved
  it EXPANDED** — read the preprocessed output. (a §"Toolchain facts")
- **The game is plain C with a hand-rolled class framework — never `cc1plus`.**
  Constructors go through the table (impossible in C++), entries are 4 bytes
  against g++'s 8, null slots exist, vptr at offset 0, zero 8-byte-stride vtables
  against 128 flat tables. Resolve slots with `classtable.py`; a patchy `--vs`
  diff means the wrong ancestor, and the longer identical high-slot run is the
  real parent. (a §"The class framework (SETTLED")

### The three RESOLVED blockers

Each is one maspsx flag setting one behaviour, proven inert by a byte-exact
rebuild. Do not screen for them, do not stall on them, do not trust a verdict
predating the fix.

| construct | round | flag | doc |
| --- | --- | --- | --- |
| `addiu_at` (indexed global load) | 21 | `--addiu-at` | `research/addiu-at-blocker.md` |
| `gp_rel` (small-data global) | 42 | `--gp-symbols=config/gp-symbols.txt` | `research/gp-relative-blocker.md` |
| `nop_mflo_mfhi` | 42 | `--no-nop-mflo-mfhi` | `addiu-at-blocker.md`, RESOLVED addendum |

(a §"BLOCKED: no C function can reach a small-data global", §"BLOCKED: no C
function can load through a runtime-indexed global", §"BLOCKED: the
`nop_mflo_mfhi` screen runs FORWARD")

- **A screen is a claim WITH A DIRECTION, and a blocker's DEATH is scoped like
  its life.** The `mflo` hazard is `mflo`/`mfhi` followed WITHIN TWO instructions
  by `mult`/`div`; the reverse is not, and a retail `nop` is not an exemption —
  broken in both directions by four heads. Round 49 likewise killed "pre-round-42
  `mflo` permuter negatives are void": nearest `mult`/`div` is 8-107 instructions
  away across all seven candidates. (a §"BLOCKED: the `nop_mflo_mfhi` screen runs
  FORWARD", §"The `--no-nop-mflo-mfhi` re-search")
- **A reproduced mechanism is not a blocker until its CORPUS FREQUENCY is
  measured.** The `addiu`-vs-`ori` immediate reproduces perfectly and occurs ONCE
  against 1089 `ori` — that one is libcard's. Related: positive constants are
  always `ori`, negative always `addiu`, and `objdump` prints both `li`, so
  text-diffing tools are blind (signature: "one word short, no visible diff").
  (a §"A reproduced toolchain mechanism", §"`asm-differ` and the permuter compare
  TEXT")

## 2. Build hygiene

- **Restore `INCLUDE_ASM` BEFORE `make extract`, every time**, or splat deletes
  the stub you meant to restore and the failure reads like a path typo (three
  instances). **`make clean` deletes `asm/`**; `make extract` restores it. (a
  §"`make extract` while a function is LIVE C", §"`make extract` is
  match-status-aware", §"An operational trap: `make extract`", §"An operational
  hazard: `make clean`")
- **`make extract` after merging any match, then re-run funcdiff.** A stale `.s`
  gives a bogus WINDOW, not just a bogus score — `67808/67808 words match` for an
  82-word function, plausible end offset, only the start wrong. It does not fire
  reliably. (a §"A STALE `.s` makes `funcdiff` report a bogus WINDOW")
- **Read every score with the unit's other siblings reverted to `INCLUDE_ASM`**
  (`grep -c '^INCLUDE_ASM' src/<unit>.c`). One contiguous section means one short
  function shifts every later `.bss` symbol image-wide, and a sibling then reads
  146/196 against its true 171/196. Documented drift, not a new way a score lies.
  (a §"A length-short function shifts `.bss`", §"Leaving another function's stall
  body live")
- **Drift's best disguise is a PLAUSIBLE WRONG CONSTANT in the function you are
  editing** — two table bases exactly 0x10 low looked like a symbol bug; the
  function was 4 words short. A uniform offset across unrelated symbols is drift;
  check `build/lsdde.map` first. Conversely **a low score is not evidence of a
  distant shape**: 14/105 while 104 of 105 instructions matched, so cross-check
  length with `objdump -d`. (a §"Address drift's most convincing disguise",
  §"`funcdiff.py`'s byte range is not a stalled function's true length")
- **Never size anything by counting a `.s`'s instruction-comment lines** — a
  jump-table-owning function has rodata in its own `.s` with the same comment
  shape, which gave a 65533/65533 window, an `uncarved.py` over-count and a
  178-vs-147 size claim. Use the declared `nonmatching <name>, 0xNNN` size or
  `glabel`/`endlabel`. (a §"Tooling defect found in round 27", §"14 stalled
  functions were Sony library code")
- **A FLAG change rebuilds NOTHING, so flag experiments start falsely green** —
  `rm -rf build` first; a `-G8` trial reported green and was 19148 bytes off from
  scratch. A HEADER edit correctly rebuilds everything. Run a flags-from-`$(...)`
  pipeline under `bash -c`: zsh does not word-split, and a comparison where BOTH
  sides fail prints `IDENTICAL`. (a §"Address drift's most convincing disguise",
  §"Two method errors by the head")
- **Build every inherited body ONCE before trusting any figure in its report.**
  It fails three ways and only the first is loud: a stale symbol, a missing
  declaration, and a preamble that now CONFLICTS with the unit's declarations —
  the last two are fatal at `cc1` exit 33 with no `error:`/`parse error`, caught
  only by CLAUDE.md's `*** [….o]` pattern. Splicing from another checkout also
  drops `#include`s, and that parse error reads as a MATCH (fourteen fictional
  full matches from one stale build). (a §"An inherited body fails THREE ways",
  §"Address drift's most convincing disguise")
- **One preserved body in six carries a false "clean / drift-free" claim, and
  some carry a score that was NEVER MEASURABLE or a wrong READING.** Require
  funcdiff's outside-range count to be ZERO **and** the objdump word count to
  equal the `.s`'s declared size; output containing `WARNING` has no score in it.
  A body calling a nonexistent symbol never linked; a self-consistent prose
  reading of a guard survives indefinitely, so building re-tests the reading too.
  (a §"A preserved body's \"clean / drift-free\" claim", §"A preserved body can
  carry a score that was NEVER MEASURABLE", §"A preserved body can carry a wrong
  READING")
- **A report's CODE BLOCK and its STRUCTURAL claims are claims to verify, and it
  can claim a body that does not exist.** The listed body and what is banked in
  `src/` drift apart silently; a body can reproduce the recorded LENGTH while the
  mechanism credited for it is false; and which body supersedes which is stated
  in PROSE, so build no lexical rule — a screen that resolves that ambiguity
  resolves it wrongly and silently. (a §"A report's own CODE BLOCK is a claim to
  verify", §"\"Rebuild the inherited body\"", §"A report can claim a preserved
  body that does not exist", §"A report's MANDATED preservation form")
- **An SDK-object round breaks preserved bodies and unblocks reports; neither
  announces itself.** Renamed placeholders stop linking (`tools/stalesyms.py`);
  a stall filed on "unknown callee" may now be workable. A stale name means the
  figure is unverified, not wrong, so do not bulk-fix. And a screen that keeps
  flagging its own repairs never converges — `stalesyms.py` re-flagged three
  just-corrected bodies, so repaired reports read as untouched. (a §"A preserved
  body's symbols go stale", §"An SDK-object round can UNBLOCK a report", §"A
  preserved body's `jal` targets can go STALE", §"A screen that keeps flagging
  its own repairs")
- **A prototype for a function ANOTHER unit defines belongs in your `.c`, and a
  bulk rename breaks that silently** (`tools/headercontention.py`). **Retyping a
  global bare -> array silently invalidates a SIBLING's preserved body via
  pointer decay**: `D_X > 0` becomes "is this address nonzero", compiles clean,
  is not in the build, and survives "rebuilt verbatim". `grep -ln 'D_XXXXXXXX'
  docs/match-reports/*.md`; address-of contexts are safe, value contexts are not.
  (a §"Renaming a Sony function into a SHARED header", §"A plain-global-to-ARRAY
  retype")
- **Verify struct offsets with a host `-m32` `offsetof` build**, and compile
  immediately after drafting any struct over two fields — a native build widens
  pointer fields while the layout is wrong, and a missing `u8 padNN[...]`
  presents only as a whole-image SHA1 failure in a function that never touched
  the declaration. `sizeof` is not how this game allocates (`New_X` passes a
  literal byte count), so appending trailing fields is safe; `DreamSys` excepted.
  (a §"Confirmed on this game", §"Round 15")
- **A rodata slot holding JUMP TABLES must stay attached to its unit** — its
  words point at `.L8005…` labels local to each function's `.s`. Data is
  decompiled one slot at a time. Separately, a clean from-scratch build emits 111
  warnings including 9 implicit declarations, all hidden by the incremental
  build. Two fixed tooling defects worth the shape of: `setup-permuter.sh` ran
  `PERM_*` seeds through `cc1`; `block-raw-make.py` read `2>&1`'s stray `"2"` as
  a target. (a §"Address drift's most convincing disguise", §"An operational
  hazard: `make clean`", §"Tooling defects found in round 19")
- **A `D_XXXXXXXX` in rodata holding a STRING is a symbol to REFERENCE, never a
  string to retype.** splat has already emitted those bytes; a C literal emits a
  second copy, and because `section_order` puts `.rodata` first the whole image
  shifts and the first differing byte lands thousands of bytes AHEAD of the code
  you edited. Grep `asm/data/*.rodata.s` for the symbol first;
  `extern const char D_XXXXXXXX[];` is the only correct spelling. The mechanism
  is a fact about this build system and stands on the round-20 `func_8003FC70`
  whole-image green, the one SDK-exit precedent that survives. (a §"A rodata
  `D_XXXXXXXX` holding a STRING is a symbol to REFERENCE", §"The SDK-exit census")
- **Preserve a stalled body in `#if 0 ... #endif`, never in a `/* */` block
  comment**, inlined in the match report with every declaration it needs,
  positioned where it would compile. Inside a block comment a line starting with
  `*` is ambiguous between a continuation marker and a dereference. Which of
  several preserved bodies is authoritative is stated in PROSE; no lexical rule
  recovers it, so do not build a screen for it. (a §"A report's MANDATED
  preservation form can point at the WRONG body")

## 3. Source-shape idioms

### 3a. Control flow and block order

- **GCC 2.6.3 gives the FALLTHROUGH to whichever candidate is LAST in source
  order.** Two shapes: an arm that must `j` over a join must be written NOT-LAST
  (explicit `goto` over the block that falls through), and a duplicated
  assignment retail keeps in two copies needs the OTHER copy last. Closed
  `func_8005CBC8` 100/100 after ~77k permuter iterations on the wrong axis, and
  `func_8005DBF0` 74/74 after eleven variants. (a §"An arm that must JUMP")
- **Not strictly dominant — diagnose per instance.** Round 25 closed three
  functions with FOUR different placement fixes and regressed one already right.
  The content is the QUESTION: which block does retail place where, and which
  candidate did my source make last? Tell: a lone unconditional `j` to a nearby
  join whose delay slot carries real work. Build no screen — the one built
  flagged 51 functions and its single tested hit was false. (a §"THE LEVER IS NOT
  STRICTLY DOMINANT", §"The mechanical screen for this has NO measured
  precision")
- **"A barrier had no effect" is positive evidence FOR block order**, as is a
  flat permuter plateau (it cannot restructure LAYOUT). But "a barrier does not
  transfer" has three causes — block order, intra-block scheduling, and DCE
  (barrier-proof; cite `func_80029C40`) — and an `mflo`/`mfhi` is a NEGATIVE
  indicator. Once block order is RIGHT, "split it into two C variables" is
  actively harmful (twice-reproduced regressions). (a §"\"A barrier had no
  effect\"", §"Why the permuter cannot find this", §"\"A barrier does not
  transfer\"", §"One measured counter-indication", §"The NAIVE form of the fix")
- **A shared `return` block lands where the FIRST `return` sits in source order,
  and two early exits need an explicit `goto` to a hand-placed shared label** —
  even when both return the SAME value, or GCC tail-duplicates the second and it
  presents as SIZE drift. Phrase the LAST guard positively. Diagnostic: inverted
  entry-branch polarity plus a stray early `j`/`move v0,zero` means wrong PLACE,
  not a backwards condition (5/56 -> 15/56). (a §"A shared `return` block's
  PLACEMENT", §"Four narrower confirmations from round 23", §"Source-shape levers
  found this round")
- **Two-return guard: SUCCESS return inside, FAILURE return trailing** — the
  reverse costs two words because cross-jump will not merge those epilogues
  (twice, unrelated blocks). It is also the `New_X` null check verbatim. And
  **default-then-override beats two `return`s when the function CHOOSES BETWEEN
  TWO RESULTS** (three closes plus a first-try transfer); a pure accessor has no
  choice to express. (a §"A two-return guard", §"Round 16",
  §"Default-then-override vs two `return` statements")
- **`if`/`else` codegen is MECHANICAL: the test is always NOT(what you wrote),
  the `if`-body falls through, the `else`-body is the branch target** — so
  retail's placement gives the source polarity arithmetically (3x), polarity
  recurs at several nesting levels INDEPENDENTLY, and the tell is that only the
  branch MNEMONIC and two immediates differ. Whichever arm is textually LONGER
  needs to be the fall-through, decided by if/else-if versus nested-`if` (5x in
  one unit). (a §"Round 15", §"A fresh local that only carries one branch's
  result", §"Three more levers from the cold-ground runner")
- **Write a small early exit as an inverted guard — a DEFAULT, not a rule**
  (3x); round 21 found the counter-shape where a small bounds-check-then-body
  wants the plain form, so check which arm falls through. For a >2-arm dispatch,
  transcribe retail's CFG literally with `goto`/labels; and `break` + a post-loop
  `if` cannot express "jump PAST a fall-through tail", which needs a real `goto`.
  (a §"Confirmed on this game", §"Round 15", §"Round 45 addendum")
- **GCC 2.6.3's loop optimisations are SYNTAX-GATED, not CFG-gated.** LICM hoists
  a loop-carried literal comparison into a spare callee-saved register, one word
  SHORT; `label: …; if (cond) goto label;` defeats it, and register pressure is
  not the knob. Signature: extra `li $sN,<const>` outside, missing `move` inside.
  Does not generalise across loops even within one function. (a §"Source-shape
  levers found this round")
- **A redundant guard is NOT dead code — 2.6.3 compiles it literally.** GCC does
  not dedupe an explicit `if` against a loop's implicit entry test (where retail
  has ONE check, `guard + do-while` says so), and a provably-dead `x != 5 && x !=
  8 && x == 0xA` chain is byte-exact while its "obvious" sparse-`switch` rewrite
  measured 109627 bytes off. (a §"Round 16", §"Round 15", §"Round 12")
- **Independent global stores are freely reordered, so retail's store order is
  not evidence of source order — and a reorder can close a LENGTH gap** (87/88 ->
  88/88 after a function had been one word short throughout its history). Two
  branches with the same result want a combined `&&`, not nested `if`s, which
  duplicate the shared formula behind a jump. (a §"Independent global stores are
  freely reordered", §"Round 27: two branches with the same result")

### 3b. Switch and jump tables

- **For a DENSE switch, arm bodies are emitted in SOURCE order and the table
  order is readable off the binary** — sort arm labels by ADDRESS, map back
  through the jump table, write the cases in that order; the arm falling through
  into the shared tail is LAST, a free anchor. **But measure per function**: if
  label order coincides with ascending value, reordering buys nothing (82/82
  ascending versus 69/69 only at `25, 23, 5, 4, 18, 19`). (a §"A `switch`'s CASE
  ORDER is recoverable from the binary", §"The block-order rule extends to
  `switch` CASE order")
- **A dense switch must reproduce retail's JUMP-TABLE WIDTH.** GCC sizes the
  table from the span of case values it sees, so a wider retail table means a
  missing case label — usually an empty arm; `case 47:` widened 33 entries to 48
  and closed 79/79. A first diff at or just after the rodata table base is a
  WIDTH problem. The pivot tree depends on the exact value SET including empty
  cases, so a GAP is evidence of one. (a §"A dense `switch` must reproduce
  retail's JUMP-TABLE WIDTH", §"Confirmed on this game")
- **For a SPARSE switch GCC normalises comparison order to ascending value, so
  source order is NOT recoverable from it.** The tell that you have a `switch` at
  all is a range-split `slti`/`sltiu` mid-chain. SCOPE: needs at least THREE
  explicit case values; at two-plus-default, `switch` and `if`-chain are
  byte-identical. (a §"A `switch`'s CASE ORDER is recoverable from the binary")
- **A genuine `switch` and a logically identical if/else chain are not
  interchangeable, and neither is "the" answer** — a real `switch` moved
  `func_80032708` 24 -> 65/164 and closed `func_8004A070` 48/48 (direct
  `beq`-to-case with no skip-branches is a switch signature), while elsewhere a
  sparse switch beat a chain that would not converge. But `if`/`else-if` and a
  `return`-terminated sequential-`if` compile IDENTICALLY. (a §"Round 16",
  §"Round 12", §"Round 15", §"Source-shape levers found this round")
- **Never size a per-function residue with the whole-image byte count when the
  function contains a `switch`** — compiling it replaces the `.s`'s jump table
  with GCC's own and shifts the entire image. Use asm-differ markers or
  funcdiff's in-range score. (a §"Round 13 (2026-09-03)")

### 3c. Struct layout, types and widths

- **A local's DECLARED WIDTH is a codegen decision and `s16` is the expensive
  default.** An `s16`/`u16` local compared or indexed is re-sign-extended at each
  use — `sll 0x10`/`sra 0x10` PAIRS, never scheduling, never movable by a
  barrier. Declare the LOCAL `s32`, keep the FIELD `s16`. Trap: two locals of the
  same declared type can compile differently. (a §"A local's DECLARED WIDTH is a
  codegen decision")
- **Declare a narrow value as wide as the register it lives in.**
  `-funsigned-char` makes a `u8` local a real narrowing re-materialised at each
  use: a byte reused across comparisons wants `u32` (kills a spurious `andi`) or
  `s32` (gets `slt`, not `sltu`); plus `andi 0xff` -> `blez`, +44 raw words. **It
  can TRADE one defect for another** (15/24 either way), so read the DIFF: a
  changed SET of differing words means the lever worked and exposed a second
  defect. (a §"Round 27: a byte value reused across comparisons", §"The lever the
  diagnostic step actually surfaced", §"Round 27: an UNMOVED score")
- **A struct of all `s8`/`s16` has alignment 2, and alignment is a TWO-WAY lever
  read off retail's instruction WIDTH.** Alignment 2 makes a whole-struct
  assignment compile to `lwl`/`lwr` + `swl`/`swr` and one stray `s32` breaks it
  (4x). Inversely, if retail's tail is `lb`/`sb` where yours merges into a
  halfword, declare all-`s8` for alignment 1. (a §"Confirmed on this game",
  §"Round 13, second batch", §"Source-shape levers found this round")
- **Whole-struct assignment instead of a field-copy run is the highest-yield
  source lever found (7 closes, 3 unit families) — but only where the address
  RESISTS constant folding.** It helps for a second pointer, an array index or an
  out-parameter; it does nothing for a compile-time `self + literal` or a
  naturally-aligned run of same-type `s32`s. Arrays need a wrapper struct to test
  in C89. (a §"Aggregate assignment vs scalar field-copy")
- **A struct-layout claim must be settled CORPUS-WIDE: a shared base with a small
  compile-time fold is conclusive, separate `lui`/`addiu` pairs prove nothing.**
  `addiu $t2, $a3, 0x2` is only emittable if the compiler knows they are ONE
  object. Read the STRIDE too: two globals a few bytes apart with a common
  non-power-of-two stride are one array, split by USE (a tested member
  re-materialises its base; one whose address is passed becomes an induction
  variable). (a §"A struct-layout claim must be settled CORPUS-WIDE", §"Two
  globals a few bytes apart with a COMMON stride")
- **Settle a data-modelling hypothesis by reading the DATA SECTION, not by
  compiling variants** — two greps settled what eleven builds had not. Commit a
  corrected model even when byte-identical and not the cause; the negative is
  only trustworthy measured against it. (a §"A data-modelling hypothesis is
  settled by reading the DATA SECTION")
- **A symbol accessed at TWO WIDTHS needs a pointer CAST, not a scalar
  truncation** (`*(u8 *)&sym`), and the width can differ per USE. A field can be
  read SIGNED at its writer and UNSIGNED at another reader and both are right —
  type it where written, cast at the reader that disagrees. `bool` is `typedef
  int bool`, a full word. (a §"A symbol accessed at TWO WIDTHS", §"Round 13,
  final batch", §"Confirmed on this game")
- **A mask on the PRODUCT means a HALFWORD array, not a struct array.** Early
  `sll 3`, mid-block `andi 0xffff`, LATE base load, `sll 1` is `u16 woff =
  (u16)i * 8;` indexing an `(s16 *)` — eager in a LOOP, deferred outside one. Do
  NOT hoist the base pointer (three attempts made it worse). General form: when a
  residue is a MASK, ask WHICH VALUE is truncated. (a §"The \"split scaled
  index\"")

### 3d. Locals, naming and register identity

- **A named C variable gets ONE storage location for its whole scope, so any new
  name is a new allocno.** Treat ANY change to the set of named locals as
  potentially renumbering every callee-saved register, and re-measure LENGTH
  (confirmed at both ends of the size range). It is also why retail's transient
  rematerialization shape has no C spelling — bracketing lands one word out on
  each side, a characterised STALL and not grounds for raw asm. (a §"One named C
  variable gets ONE storage location", §"A fresh local that only carries one
  branch's result")
- **The SCOPE of a named local is the lever, not the name.** Hoisting to function
  entry adds pressure everywhere it is not needed; declaring the local INSIDE the
  block where the value must survive one call closed `func_800357B0` 179/179
  after rounds 35, 39 and 46 all failed by hoisting to the top. Three distinct
  forms exist, so "tried a named local" is under-specified. (a §"The SCOPE of a
  named local is the lever")
- **"Name it, THEN barrier it" is two-part and neither half works alone** — a
  named local plus a bare `__asm__("")` immediately after the declaration took
  `func_8003D73C` to exact length after five hand attempts and 27k iterations
  across rounds 12-46, while named-temp-only reproduces the old baseline exactly.
  The identical lever on a textually identical sibling line regressed it 114/118
  -> 14/118. (a §"\"Name it, THEN barrier it\"")
- **What a value is NAMED and how many times it is LOADED are two knobs, and the
  helpful direction is function-specific.** REMOVING a dead reload (`x->y->z =
  0`) was worth 12 and 10 words; ADDING a cached local pointer was the only thing
  that stopped strength reduction elsewhere; dropping a name to recompute inline
  is a lever where retail recomputes per path (8 words). Discriminator: if the
  variant changes LENGTH it is a true register-identity wall. (a §"Eliminating a
  DEAD RELOAD", §"What a value is NAMED", §"Dropping a name and recomputing
  inline")
- **Reuse a provably dead PARAMETER instead of a fresh local**, when the local
  only carries one branch's result to one later use — a parameter live on entry
  and never read again adds no allocno (62/62, 58/58); mutating a parameter in
  place is the same move. (a §"A fresh local that only carries one branch's
  result")
- **Match the NUMBER of live pointer variables retail has, not just the
  arithmetic.** An accumulate-in-place loop wants `cur = p; p++; *cur = …;` — not
  `p[i]`, not `*p++`; the longhand two-pointer form produces two destination
  registers where `*dst++ = x` collapses to one. SCOPE: both must be MUTATED, or
  copy propagation collapses the alias and the residue really is register
  identity. (a §"A \"the compiler will not spend a second register\" residue",
  §"Source-shape levers found this round")
- **Hoist BOTH values before EITHER is consumed.** If retail's two loads or
  multiplies are ADJACENT with consumers later, the source hoisted both (five
  closes on functions filed as register identity). It addresses load SCHEDULING,
  not allocation, so it can be required AND insufficient, and a forced hoist is
  not neutral. Preconditions that fail while looking like the shape: a
  branch-gated second load, loads far apart with the first consumed between,
  induction variables already live, delay-slot FILL choices. (a §"Hoist BOTH
  values", §"The hoist-both lever is LOCATED", §"A function can REQUIRE the
  hoist-both lever")
- **A residue surviving two levers INDEPENDENTLY has not been shown to survive
  their COMBINATION — if they target independent decisions** (68/70 -> 69/70; but
  on `CalcDreamColor` both acted on one fused address expression and all three
  variants were byte-identical). Conversely **levers do not commute**: if a
  residue MOVES rather than SHRINKS, revert before the next, and a
  "source-unreachable scheduling residue" verdict needs an isolation reproducer
  of the NAIVE body. (a §"And the corollary that generalises", §"The combination
  corollary", §"Levers do not commute")
- **A same-size pointer cast in a FUNCTION-SCOPE local can cost a callee-saved
  register — the cost is LIFETIME, not the name.** Casting inline at each use
  site took a function from wildly wrong to 106/110; a case-local temp was
  byte-identical; retyping the parameter was inert. Separately, splitting a
  combined declaration (`T x; x = expr;`) is a real lever for a value crossing a
  CALL boundary (19/19) and inert on parameter colour swaps (four negatives).
  (a §"A same-size pointer cast in a FUNCTION-SCOPE local", §"Source-shape levers
  found this round")
- **A local reused across two MUTUALLY EXCLUSIVE branches still perturbs
  allocation in the branch you did not touch** — symptom is an unexplained extra
  `move $vN,$v0` around a call return elsewhere. Two identical global reads with
  non-overlapping live ranges can want DIFFERENT registers, so one reused local
  manufactures a residue. A uniform register-slot shift across the function
  signals ONE EXTRA PERSISTENT LOCAL. (a §"Source-shape levers found this round",
  §"Round 45 addendum")
- **Cache a re-read struct field only across a CALL-FREE span; reload after any
  intervening call** — GCC has no guarantee a `jalr` did not write through the
  object, and the tell is the wrong NUMBER of callee-saved registers. Converse: a
  value you WROTE and reuse needs a named local, and an intervening whole-struct
  assignment defeats CSE. Whether to cache `self->methods` tends to be constant
  PER CLASS, determined once from a matched sibling but kept conditional. (a
  §"Round 11", §"Round 12", §"Round 15")
- **Two long-standing near-misses closed by DELETING a named value:** removing a
  decrementing pointer local that lives across loop iterations and indexing
  directly (177/177), and writing a division in place into its dying dividend.
  Generalising: when the result went to the wrong register and one operand is
  dead after, assign the result back into that operand. Narrower: splitting a
  table-address computation into two live locals helps ONLY from an unnamed
  global folded into one line (one win, two regressions). (a §"Two levers that
  closed long-standing near-misses by DELETING a named value", §"The
  two-independently-live-locals lever")
- **Keep a pointer computation that is one retail expression as ONE C statement**
  (47/117 -> 95/117); conversely **split a combined expression so the statement
  count matches retail's instruction count**. And **compute a value LAZILY**,
  where retail's control flow first needs it — an eager computation lengthens the
  live range, the same allocno-ranking mechanism as the `s16`-width entries. (a
  §"Confirmed on this game", §"Four smaller levers, measured in round 38",
  §"Four narrower confirmations from round 23")
- **Preheader emission follows the order values are NAMED**, so a "wrong
  preheader order" residue can be a naming problem no barrier reaches. Sibling
  VLA DECLARATION ORDER shifts priority for unrelated locals (42/56 -> 49/56),
  and stack-local declaration order fixes `$sp` offsets. A multi-value
  permutation can have ONE trigger (reading a reused value through a FRESH
  local); two INDEPENDENT accumulator pairs are fixed by INTERLEAVING the writes;
  and a redundant `local2 = local1;` can be load-bearing with no UB (3x). (a
  §"Four smaller levers, measured in round 38", §"Sibling VLA declaration ORDER",
  §"Confirmed on this game", §"A block of stores far ahead of a branch",
  §"Round 15")
- **`do { } while (0)` is a REGISTER-PRESSURE lever, not a scheduling lever.** It
  perturbs global state via pressure: inert across four delay-slot-fill sites,
  regressive on an unrelated matched sequence, a 4-word length closer elsewhere,
  61/70 -> 62/70 around a single early return, catastrophic (66/69 -> 1/69)
  applied to all four returns. Small straight-line bodies only; per-return, never
  whole-function. (a §"`do{...}while(0)` is a REGISTER-PRESSURE lever", §"`do { }
  while (0)` wrapping is a SMALL-BODY lever", §"Two levers from the re-send")
- **Levers measured INERT — do not re-derive.** C89 `register` (the legal form)
  is a no-op for allocation; a clobber-bearing barrier is no better than an empty
  one; a dummy unused SCALAR cannot nudge frame allocation; a same-valued alias
  is collapsed by copy propagation and an algebraic identity rewrite is
  transparent to value numbering; narrowing a cast-local's lexical SCOPE does not
  help while its LIVE RANGE crosses a call. (a §"Levers measured INERT on this
  pipeline")

### 3e. Frames and stack

- **An unused stack frame is reserved by an unused local ARRAY, never a scalar.**
  `s32 unused[2]` and `s16 unused[4]` green at 8 bytes; one scalar, two scalars,
  a 4-byte array and no local all RED; the `if (0)` guard is INERT. Read `addiu
  $sp, $sp, -N` and declare exactly N bytes. **SCOPE: only when your build emits
  NO `addiu $sp` at all** — otherwise the array STACKS on top (0x20 -> 0x40) and
  the score worsens, because that residue is PLACEMENT. (a §"An unused stack
  frame is reserved by an unused local ARRAY", §"SCOPE — measured")
- **The frame-padding idiom recovers frame ALIGNMENT, not word COUNT** — measured
  4 of 4: frame exact, word count unmoved. Use it FIRST as a cheap diagnostic
  realignment, then look for a separate companion fix; the advertised 18/376 ->
  48/376 came from a paired tail-duplication fix. An allocated-but-unused frame
  on a leaf function is not a residue at all. (a §"The frame-padding idiom
  recovers frame ALIGNMENT", §"An allocated-but-unused stack frame")
- **An oversized outgoing-arg area is evidence of DEAD CODE in the source** — GCC
  sizes it from every call expression before DCE. Chain: outgoing = lowest
  saved-register offset; above 0x10 is non-minimal; if nothing touches it, it is
  argument reservation; `outgoing / 4` is the widest call expression the compiler
  SAW. But it screens FRAME SIZE ONLY, so where length is already exact this
  lever has nothing to fix. (a §"An oversized outgoing-arg frame", §"ROUND 20:
  the signature is a screen for FRAME SIZE ONLY")
- **A wholly-unused STACK parameter is diagnosed from a FIXED OFFSET, not a
  register gap** — compare `frame_size + 0x10` against the first stack argument's
  actual `$sp` offset. Symptom: a handful of low-nibble-only diffs on branch
  targets, which reads like instruction selection. The dead-LOCAL trick does not
  substitute; the fix is an extra ARGUMENT. (a §"A wholly-unused STACK
  parameter")
- **Two or more `subu $sp, $sp, <reg>` with `move $fp, $sp` is VLAs, not
  pressure — and the rounding immediate carries the array BOUND** (`0xf` against
  `width` means `char buf[width + 1]`, `0xe` means `char buf[width]`; wrong is a
  LENGTH mismatch). An empty frame SMALLER than 0x10 is a dead local ARRAY write,
  not a dead call. On a never-built derivation, get the FRAME right first. (a
  §"Two VLAs, an `$fp` frame", §"Four narrower confirmations from round 22",
  §"Two diagnostic findings worth as much as the levers")

### 3f. Calls, arguments and return types

- **A value in an ARGUMENT register live at the next call IS an argument, even
  when its only visible use is a branch condition.** Trace forward to the next
  `jal`/`jalr`; if it is `$a0`-`$a3` and nothing overwrites it, the parameter
  list is wrong — it converted two "unreachable register-identity" stalls.
  Negative half: a `$v0` residue or a register dead at the next call is NOT this,
  and a register surviving an intervening call is not reliably an argument. (a
  §"Confirmed on this game")
- **A delay-slot residue writing `$a0`-`$a3` is a CALL ARGUMENT until proven
  otherwise.** Walk forward along the branch-TAKEN path to the first
  `jal`/`jalr`; a single-path liveness check is not one. The cause is usually a
  vtable slot typed with too few parameters, so `grep -rn 'slotNN' src/` across
  units first. (a §"A delay-slot residue writing `$a0`-`$a3`")
- **A bare `nop` in a call's delay slot establishes arity ONLY when the argument
  is not already in the right register** — those two cases are BYTE-IDENTICAL.
  **MIPS o32 fills argument registers strictly left to right**, so untouched
  `$a1` with `$a2`/`$a3` set PROVES a forwarded parameter; the converse fails,
  and a missing argument can be invisible from the callee, so type a slot from
  its CALL SITES. An unused parameter appears in the caller as an uninitialised
  local. (a §"3. A bare `nop` in a call's delay slot", §"Round 11", §"Confirmed
  on this game")
- **A discarded return value is never evidence of `void`**, and an empty-bodied
  occupant is never evidence the slot takes no arguments; bytes constrain the
  return type only where the caller USES the value. Prefer the slot's OCCUPANT
  (`classtable.py`) — a slot typed from an unattempted function's disassembly is
  a hypothesis with nothing behind it. (a §"Confirmed on this game",
  §"Cross-jump shape THREE")
- **A declared RETURN TYPE can block a cross-jump merge that should happen** —
  GCC will not merge a `void` call against a value-returning call whose result is
  discarded, because the RTL differs though the instructions are identical. When
  N identical calls merge into GROUPS, the grouping PARTITIONS BY DECLARED RETURN
  TYPE, and a 2+2 split is a TYPE mismatch no barrier touches. (a §"Cross-jump
  shape THREE")
- **Retyping a shared vtable slot is NOT local — prefer a LOCAL function-pointer
  variable.** Assign both differently-typed slots into one `void (*fn)(...)` and
  call `fn`: that forces the merge without the retype that silently broke a
  matched function in another unit. Two symmetric slots needed OPPOSITE answers,
  and a broken retype is a RED BUILD. Casting the slot down at ONE call site
  handles an arity mismatch. (a §"Confirmed on this game", §"Round 12", §"Round
  14")
- **A multi-exit function that stores a literal into a field and returns that
  same literal right after a `jalr` cannot be typed `s32` — try `void`** (18
  isolated reproducers: `$v0` consistency across the merge evicts the store's
  operand to `$v1`). The bytes pin the return type in neither direction, and
  retyping a matched `void`-shaped function by ADDING `return`s can grow it. An
  already-matched signature can be too NARROW on both sides. (a §"Round 15",
  §"Round 14", §"Source-shape levers found this round")
- **A struct RETURNED BY VALUE reads as a call with its arguments shifted** — GCC
  returns every struct through a hidden pointer as the invisible FIRST argument.
  Recognise `addiu $aN, $sp, <local offset>` in the `jalr`'s delay slot with the
  object one register later, and size the destination from the frame gap.
  Declaring the slot `void (*)(Dest *, Obj *)` matches the bytes and is WRONG. (a
  §"A struct RETURNED BY VALUE")
- **A vtable call needs no forward `extern`; a direct `jal` does** — type it from
  the registers before the `jal`, since only the call site's bytes depend on it.
  A slot resolving to a named C function must STILL be called through the struct
  field, and a slot genuinely called at several arities can be K&R-declared as a
  last resort. (a §"Confirmed on this game", §"Round 15", §"Round 14")
- **Type the CALLER's parameters so a callee prototype's implicit conversions
  emit the exact per-call narrowing seen in the disassembly** — those
  instructions belong to the call. Read the CALLEE's body for the width it uses:
  an over-narrow `s8` parameter costs a sign-extend at every call site, a `u8`
  lvalue passed to `s32` costs a redundant `andi 0xff`, and an unsigned narrow
  STACK argument loads as one `lhu` where a signed one is `lw`+`sll`+`sra`. (a
  §"A symbol accessed at TWO WIDTHS", §"Confirmed on this game")
- **A call result both stored into a field and reused later needs a named
  local**; and a call argument that is the same literal on every branch is not
  evidence it is passed as a literal — where each branch schedules its own `ori`,
  assign a per-branch local. (a §"Round 15", §"Source-shape levers found this
  round")

### 3g. Delay slots, arithmetic, one-instruction residues

- **A delay-slot instruction executes on the TAKEN path too, and a delay-slot
  store is UNCONDITIONAL.** Evaluate the value at the TARGET: `li $v0, 0x1` in a
  slot with `sw $v0, 0x24(s1)` at the target stores 1, not the 3 the comparison
  used, and reading it wrong manufactured two compiler mysteries. Modelling such
  a store as conditional compiles cleanly; hoisting one out of an `if` closed 8
  words. (a §"How to read a one-instruction residue", §"Read the delay slot
  before modelling the branch", §"Round 12")
- **A "dead-looking" filler may be a WRITE scoped wrong in your C — the
  discriminator is whether it has a MEMORY EFFECT.** A store can hide an
  unconditional write and is worth re-scoping (82/82); a register-to-register
  move cannot, so there the scheduling explanation stands. (a §"A delay-slot
  filler that \"looks dead\"")
- **GCC hoists an EXISTING instruction into a load-delay slot; it never invents
  one**, so a filler whose result looks dead is a lead about the source —
  reconstruct an expression making that value live at the branch, not the NUMBER.
  A `move $sN,$v0` in a `jal`'s OWN delay slot reads the value `$v0` held BEFORE
  that call; misreading it makes the C describe the wrong LIFETIME. (a §"Round
  13, second batch", §"Source-shape levers found this round")
- **When your diff is ONE redundant or ONE missing `move`, count how many times
  your SOURCE mentions the value** — one too many adds a `move`, one too few
  removes it, and the surplus lands in a free delay slot, which is why "different
  filler" and "one value too many" are indistinguishable. And first of all: **do
  the branch TARGETS agree?** A differing target is a differing CFG and always
  comes from the source (reading past this cost 25 words). (a §"How to read a
  one-instruction residue")
- **Do not transcribe a LOWERING back into C.** `xori`+`sltiu 1` is `x == K`;
  `sltu $r,$zero,$r` is `x != 0`; a same-amount `sll`/`sra` pair is a narrowing
  signed cast; a `mult`/`mfhi`/sign-fix chain is `/` or `%`. A bare `andi` with
  no sign-fix is `& (N-1)`. `~x + 1` is not `-x` — read retail's CHOICE OF
  INSTRUCTIONS as evidence about the source EXPRESSION, since a two-instruction
  encoding of a one-instruction operation is usually the source spelling it out.
  (a §"How to read a one-instruction residue", §"Confirmed on this game", §"Round
  13 (2026-09-03)")
- **Write the division; do not hand-derive the magic multiply** — `x / 127` and
  `x / 63` compile exactly to retail's sequences through the pinned pipeline. For
  an unknown divisor probe `cc1` or brute-force it: one magic constant encodes
  different divisors at different post-`mfhi` shifts, and the `sll`/`subu` chain
  disambiguates. Hand-decoding mis-guessed at least three times. (a §"Write the
  division", §"Round 12", §"Round 13, second batch")
- **An unsigned range check needs an EXPLICIT cast to get `sltiu`** —
  `(u32)(x - LO) < N`; without it, `slti`: same word count, wrong opcode. A loop
  bound's `sltiu` means an unsigned counter. But the FOLD's desirability is read
  off the disassembly per function — two units, opposite answers, one round. (a
  §"Round 13 (2026-09-03)", §"Round 13, final batch")
- **`&arr[i + j]` and `arr + i + j` are one instruction apart** — the subscript
  form sums then scales once, the pointer form scales each addend, so a
  one-word-short accessor with an `addu` on the wrong side of an `sll` is a
  SPELLING question (19/20 -> 20/20). (a §"`&arr[i + j]` and `arr + i + j`")
- **cc1 canonicalizes a commutative op's register operand order independently of
  source order — SETTLED on six instances in three units.** The decisive test is
  per-function: reverse the operands in C and see whether the output changes. It
  holds for SYMMETRIC addends, NOT where one addend is a base pointer and the
  expression can be RE-ASSOCIATED (grouping is not order: 51/51), and a local's
  declared WIDTH can also flip `rs` (47/47). (a
  §"Commutative-operand-order canonicalization")
- **A value only ONE branch consumes may still need computing UNCONDITIONALLY**,
  right after it is derived; and a value assigned on BOTH outcomes belongs ABOVE
  the `if`, where no rephrasing of the condition reaches it (88/88 first build
  after two arm-internal phrasings failed). (a §"Source-shape levers found this
  round")
- **Small expression shapes, each backed by a byte-exact match.** `cond ? A : B`
  and `!cond ? B : A` are not byte-equivalent — flip the ternary first.
  `d = -50; if (cond) d = 50;` beats a ternary for a two-constant selection.
  `(x & 1) ^ 1` and `!(x & 1)` schedule oppositely. Indexing a ternary with a
  nonzero constant DISTRIBUTES it into both branches. Comparison operand ORDER
  decides which load comes first. An expression retail evaluates TWICE must be
  WRITTEN twice. `return self->field = N;` reproduces the single-`ori` setter.
  (a §"Round 12", §"Confirmed on this game", §"Other confirmed shapes", §"Four
  narrower confirmations from round 22")
- **Loop shapes.** `while (*p) { p++; }` and `while (*p++) { } p--;` differ — a
  guard branch jumping TO a decrement means post-increment (25 words). A `for`
  header's multi-variable increment clause has a load-bearing ORDER. Let GCC
  hoist its own invariants (23 words). Reuse ONE counter for a guard and its loop
  test. A loop-carried multiplicand must be RECOMPUTED, not `+=`. A `(u8)`/`(u16)`
  mask on an induction variable used as an ARRAY INDEX controls strength
  reduction, and its width must match the surrounding comparison's. (a
  §"Confirmed on this game", §"Round 16", §"Four narrower confirmations from
  round 23", §"Two idioms that generalise further")
- **N independently-incrementing walkers over one array need N differently BASED
  view types** — strength reduction yields only to a changed BASE, never to
  regrouped fields. Keep the second view type OUT of the element struct; seed
  each walker with ONE cast outside the loop. Relatedly **cache the SCALAR a
  pointer is built from, not the pointer**: retail recomputes the address at each
  use from a cached byte OFFSET (one close, two partials). (a §"Round 13
  (2026-09-03)", §"Confirmed on this game", §"Cache the SCALAR a pointer is built
  from")
- **Length-gap levers, both directions.** 1 word SHORT with only a missing `move`
  before the epilogue: collapse two return points into one join. 1 word LONG with
  an extra `move $an,$vN` before a load plus a stray `nop`: stop giving a call's
  return value its own named local. Ranking corollary: a one-word gap SHIFTS
  everything after it, so a low raw match there is mostly RIPPLE (23/121 ->
  117/121 purely by closing the length). (a §"Length-gap levers")
- **Defeat constant canonicalization by routing the constant through an
  assignment** (`0xC0` became `-0x40`); GCC re-associates constant multiplies
  across a whole expression tree regardless of parenthesization, and only a
  STATEMENT BOUNDARY stops it. Defeat the store-flag collapse of `if (cond)
  return 1; return 0;` with `flag = 1; return flag;`. (a §"Defeat 2.6.3's
  constant canonicalization", §"Source-shape levers found this round", §"A third
  escape from the `sltiu` boolean-materialization fold")
- **Reproduce retail's bug; do not work around it.** `func_80027C80` stores
  through a never-assigned `$s2` — a real uninitialised-pointer bug in the
  shipped game. A local declared with no initialiser, stored through once at
  retail's own point, matched first build. Discriminator against the permuter's
  UB problem: whether the uninitialised value affects CONTROL FLOW. (a §"Round 45
  addendum")
- **Psy-Q OT splice shapes.** A 24-bit BITFIELD write (`unsigned addr : 24`,
  `P_TAG`) is how the OT splice is spelled; hand-written masks give the identical
  VALUE with different allocation and read as an unreachable register-identity
  residue. Declare a minimal local view of the tag. A macro's argument is
  evaluated once per expansion: `addPrim(ot, p)` loads `ot` TWICE, so caching it
  is two instructions short. (a §"Round 13, second batch")

### 3h. volatile and memory

- **`volatile` is the NARROW instrument for the instruction-ORDER class, and it
  is not the banned construct** — it names no register, exactly like the
  sanctioned bare `__asm__("")`. Where three barrier placements across two rounds
  all regressed, declaring three hardware-shadow fields `volatile` matched 14/14
  with no body change. (a §"`volatile` is a legitimate, and much NARROWER, tool")
- **The discriminator is EVIDENCE that the location is memory-mapped I/O, not
  whether `volatile` helps.** `volatile` because the hardware says so describes
  the game; `volatile` because the diff says so is a lever. One runner declared a
  documented `I_STAT`/`I_MASK` pair volatile (two closes) and DECLINED on an
  ordinary global, measuring that it changed nothing. Only two units touch
  hardware addresses. (a §"Round 16")
- **NARROW a `volatile` to the exact access that needs it** — applying it to a
  whole struct is what makes it look useless; qualifying only the WORD-sized
  field un-fused a div/mod pair while preserving retail's `lh` (193/213 ->
  211/213). When a qualifier lever "works but with a side effect", check whether
  the side effect is intrinsic to the LEVER or an artifact of WHERE it was
  applied. (a §"NARROW a `volatile` to the exact access that needs it")
- **`volatile` has TWO independent effects** — on a global's DECLARATION it
  controls elision and reordering of accesses; through a local `volatile T *` it
  also controls whether the ADDRESS COMPUTATION is folded into the memory
  instruction, so "I tried `volatile`" is at most one of two measurements. A
  volatile pointer CAST defeats the address fold and ADDS instructions. (a
  §"`volatile` has TWO independent effects", §"A symbol accessed at TWO WIDTHS")
- **`volatile` is the WRONG tool for an ADDRESS CSE, and it is NON-MONOTONIC** —
  it acts on the VALUE CSE, regresses a LICM residue (48/376 -> 6/376), and a
  correct 4-variable set beat every 2-of-4 subset while two more regressed.
  **The address CSE yields to the asm-label alias**: `extern T D_8008A8F8_b
  __asm__("D_8008A8F8");` leaves nothing to fold and closed `func_8003E968`
  41/41; six byte-verified uses already exist in `src/code_179d8_m.c`, and it
  renames a linker symbol, so it is not HARD RULE 6's banned construct. Cheaper
  still, **try REORDERING first**. (a §"Source-shape levers found this round",
  §"Negatives worth not re-deriving", §"Negatives worth their own entries", §"A
  repeated-global-address CSE is defeatable from C89")
- **GCC `-O2` merges per-branch constant stores to one global into a SINGLE
  shared store** (`if (x > 0) { g = 1; } else { g = 2; }`), where retail sometimes
  keeps one per arm. Levers: a bare `__asm__("")`, and a local `volatile T *`
  forcing unfolded addressing — which BACKFIRES inside a loop over a
  loop-invariant target. A repeated `x & IMM` is CSE'd on the VALUE, not the
  immediate. (a §"Round 16")
- **A store-then-reread of a narrow SIGNED field can compile as an unsigned
  reload plus a manual sign-extend rather than retail's `lb`** — distinct from
  the `-funsigned-char` byte-copy rule, which is about load WIDTH. A store to a
  global immediately followed by a read gets VALUE-FORWARDED, which a barrier
  cannot stop because it cannot move data — only `volatile` does. A short
  zero-fill of 2+ ADJACENT `.sbss`/`.sdata` words is a pointer-decrement
  `do`/`while` loop; and GCC does not eliminate a dead store into an
  ADDRESS-TAKEN local. (a §"Confirmed on this game", §"Four narrower
  confirmations from round 23", §"Other confirmed shapes", §"Round 11")

### 3i. GTE and inline asm

- **A COP2/GTE instruction's C form is the Psy-Q `gte_*` macro, and
  `include/gte.h` holds the GNU-syntax versions this pipeline can assemble.**
  Write `gte_stsxy3_g3(prim)`, never the `swc2` lines. The construct inside each
  is `__asm__ volatile("swc2 $12, 0x8(%0)" : : "r"(ptr) : "memory")`, which
  passes HARD RULE 6's test: `"r"` leaves the GPR to the allocator and
  `$12`-`$14` are COP2 data registers with no GPR identity to pin. (a §"Toolchain
  facts")
- **A whole-function `__asm__` is for instructions with NO C form, not
  constructs that are hard to type — and look in `gte.h` FIRST.** `func_800195EC`
  was carried as a 58-word `__asm__` because `rtpt`/`nclip`/`avsz3`/`cfc2` "have
  no C form"; every one is a macro, and the body is ordinary branching C over
  seven of them, byte-exact first build. If you cannot name an instruction with
  neither a C spelling nor a macro, it is not the exception. (a §"Round 13,
  second batch", §"Toolchain facts")
- **Name the right clobbers, and bracket real branches.** A GPR clobber naming
  `$2`-`$5` on a COP2 block forces spurious evictions that cascade through the
  whole function and present as a register-identity residue elsewhere. A block
  containing a real branch mnemonic needs an explicit tab-delimited
  `".set\tnoreorder\n\t"` … `".set\treorder\n\t"` bracket. (a §"Round 13
  (2026-09-03)", §"Toolchain facts")
- **A BARE `__asm__("")` CAN change register allocation, which HARD RULE 6's own
  test calls banned.** On game-neutral code, removing it swaps which register
  holds each of two values, and `__asm__("" ::: "memory")` was byte-identical —
  the memory clobber is not the risky half. What survives is DIRECTEDNESS: a
  barrier only perturbs. Discipline: say what a barrier DID, and stop if you are
  adding barriers one at a time until a register lands where you want it. (a §"A
  BARE `__asm__(\"\")` CAN CHANGE REGISTER ALLOCATION")
- **The permitted barrier is inert against every pass that is not the
  scheduler** — measured on cross-jump/tail-merge, loop-preheader emission order,
  a GCSE / value-availability hoist at ANY placement, and value forwarding. Ask
  which pass produced the residue first; `volatile` regresses the GCSE case by
  forcing a spill. (a §"The permitted `__asm__(\"\")` barrier is inert", §"A GCSE
  / value-availability hoist is immune")
- **Three conditions must hold for a barrier to be worth trying.** (1) A
  SUBSTITUTION residue, not an ABSENT instruction (`built=00000000` is an
  automatic no; four placements moved it zero times). (2) A genuine ORDERING
  decision, not a register CHOICE or cross-block CFG decision, where it is
  presumptively harmful. (3) BETWEEN straight-line statements in one basic block.
  Round 20: two wins, four regressions. Test its effect on WORD COUNT. (a §"The
  `__asm__(\"\")` barrier in round 20")
- **Barrier POSITION is a separate axis from its presence, and its scope is
  two-sided.** Placing one BEFORE the store that consumes a value forces eager
  materialisation where every prior round tried only after; blast radius scales
  with function size, so reach for it late and re-test removing it after any
  other fix. It DOES fix "same instructions, different ORDER"; it does NOT stop a
  backward hoist across itself or cross-jump merging. (a §"Source-shape levers
  found this round", §"Confirmed on this game", §"New residue classes opened this
  round", §"Round 11", §"Round 12")

### 3j. Permuter practice and false leads

- **Validate the scaffold in three checks BEFORE searching, and record which you
  ran.** (1) Does it compile and score. (2) Its insertion/deletion penalties — a
  near-0/0 scaffold suits a source-mutation search (zero at iteration 2642), a
  23/44 one wanders (73273 iterations of noise). (3) **Does its base score AGREE
  with the same body's score in the real build** — the isolated compile can
  allocate differently for identical source. A negative is not evidence unless
  check 3 passed. (a §"The permuter finding", §"The scaffold's
  insertion/deletion count is a COST predictor", §"A recorded permuter search is
  only evidence of COST")
- **Check 3's discriminator is AGREEMENT, not zero-ness — zero is the good sign
  in one case and the bad sign in another.** A 6/6 scaffold matching a 6/6 real
  build AGREES and paid (19/52 -> 22/52); a 0/0 scaffold is NECESSARY, not
  SUFFICIENT; and a PERFECT scaffold score on a non-matching function means the
  cause is OUTSIDE the function, so stop searching inside it. Check 3 licenses
  the SEARCH, not its output. (a §"A perfect permuter scaffold score can be the
  bad news", §"Two levers from the re-send", §"Check 3's AGREEMENT verdict")
- **`--stack-diffs` is MANDATORY on a frame or offset residue or you get a FALSE
  ZERO** — the scorer normalizes stack-offset differences away, and a false zero
  is expensive because a zero is treated as a lead to translate. (a
  §"`--stack-diffs` is MANDATORY")
- **A permuter number is in PERMUTER units, and LOWER is not a synonym for
  CLOSER.** `"Reorderings: 2"` is a bucket label; `210` is a weighted penalty
  published as "1 word remaining" for a 22/25 body; `Stack Differences: 0`
  without the flag was never filled in. Both directions measured: 2735-from-3735
  rebuilt as 14/164 with drift, while sub-base candidates translated to +13 and
  +24 real words — and the size of a drop is close to an inverse signal, so a
  local-best deserves MORE skepticism than a zero. **Translate AND measure.** (a
  §"A permuter number is in PERMUTER units", §"A permuter number is in permuter
  units — and LOWER", §"Two more independent demonstrations", §"A permuter
  improvement is a LEAD unconditionally")
- **Verification must match whether the candidate changes BEHAVIOUR.** A
  behaviour-changing candidate must be traced by hand — hoisting a call across an
  un-unrolled loop's back-edge changes only one branch immediate (176/177, and
  wrong). A behaviour-preserving one must be real-build verified: it can grow the
  frame or regress with drift while scoring better. (a §"The permuter can produce
  a SEMANTICALLY WRONG candidate", §"Two DISTINCT permuter false-lead patterns",
  §"A permuter candidate's verification must match")
- **The UB screen must be a FORWARD TRACE, not "reads before first assignment" —
  the scorer never executes anything.** Four of six hand-translated local-bests
  had correctness bugs and not all were uninitialised reads: one left a variable
  unset on a rare path, one read a reassigned variable under its OLD MEANING.
  Round 49 adds staleness across a LOOP BACK-EDGE and use-before-init from a
  reordered pair. **Trace forward on every reachable path, back-edges included.**
  (a §"The permuter UB screen was too narrow", §"The forward-trace UB screen")
- **A permuter improvement can be oracle-confirmed and still be UNSOUND C**, and
  **a zero is validated in ISOLATION and cannot see cross-TU damage.** A 208/223
  candidate hoisted a string-literal address into a CALLER-saved register outside
  a call-making loop; a genuine zero elsewhere required a shared global
  `volatile` that corrupted an already-matched sibling. Any candidate that
  retypes or moves an externally-linked symbol earns a whole-image run. (a §"A
  permuter improvement can be oracle-confirmed and still be UNSOUND C", §"A
  permuter zero is validated in ISOLATION")
- **Seed MINIMALLY, and re-seed from your closest MANUAL near-miss.** A
  whole-unit seed pulls in siblings' `INCLUDE_ASM` bodies and inflates the base;
  one function zeroed in 23 iterations once seeded from a body whose only defect
  was a register difference. `--debug`'s base-score COMPOSITION predicts
  viability better than its magnitude, and a base of 5 that will not move is a
  stronger negative than a base of 460 that halves. (a §"Seed the permuter
  MINIMALLY", §"Round 10 ran the permuter in anger")
- **A permuter negative is ONE SAMPLE of a stochastic process, in both
  directions.** A zero arrived at iteration 149 on the same scaffold where a
  33,480-iteration search had plateaued; elsewhere 163,644 fresh iterations
  reproduced an identical floor. Rank on whether a search EVER beat base —
  192,610 iterations never below base is an empty space — and track a manual
  `PERM_GENERAL` enumeration separately from an open random search. (a §"A
  permuter negative is ONE SAMPLE", §"\"Never re-searched\" and \"never went
  below base\"")
- **A sub-base candidate is worth translating even when the search never
  zeroed**, and the screening question is "reduced to STATEMENTS, what did it
  change?" — a 15-against-75 candidate's diff was almost all reformatting and
  collapsed to one statement pair worth twelve words. Not every part of a
  multi-change candidate is load-bearing, and never conclude from a FILTERED diff
  that a change is absent: compile both and compare the objects. (a §"A sub-base
  permuter candidate is worth translating", §"Two method errors by the head")
- **The scaffold's `base.c` is NOT the project's C** — a flattened TU with its
  own inlined typedefs; pasting it into the real unit fails, and the dangerous
  version is when it compiles while binding a DIFFERENT type of the same name. A
  stale flag list can manufacture a false NEGATIVE but never a false match;
  `setup-permuter.sh` now reads `MASPSX_FLAGS` from the Makefile. (a §"A permuter
  scaffold's `base.c` is NOT the project's C", §"A permuter negative is only as
  good as the flags")
- **A permuter result — zero or not — is a lead about WHICH AXIS moved.**
  `strcat`'s zero needed translation; `strstr`'s went in verbatim on an axis
  seven hand attempts never touched; `strcpy` never zeroed and its AXIS closed
  the function by hand in two iterations. A UB-tainted zero may carry an
  unrelated idiomatic fix — decompose it, adopt the ordinary C, reject the UB.
  (a §"The permuter on libc functions", §"Round 15")
- **A search's TAIL is not protected by committing before you wait.** A runner
  committed, wrote its report and ended its turn; the search then produced a
  better candidate into an untracked `permuter-work/` that teardown destroys,
  worth +5 words when recovered. After a timeout, read EVERY
  `permuter-work/<fn>/output-*/score.txt`. (a §"A search's TAIL is not
  protected")
- **If you adopt an inert form, the COMMENT at the site is part of the change** —
  two dead lines with no explanation are what a later reader deletes, and the
  image then goes red with nothing in the diff explaining why. The duplicate-arm
  trick is worth 12 words in one function, neutral in three and catastrophic
  (202/223 -> 17/223) with no pre-existing pressure — a diagnostic, and ask what
  a construct compiling to nothing is standing in for. (a §"A permuter form
  adopted on the real oracle", §"The duplicate-arm register trick")

## 4. Verdict classes and how far to trust them

- **"Register identity" is the LEAST reliable verdict class in this corpus.** It
  is self-sealing and 26% of everything queued. Contaminants found, all ordinary
  C: a masked byte parameter mistyped `s32`; a missing field-offset term; a
  statement swap between independent stores; a stale helper signature; a
  discarded return on a wrongly-`void` function; a cached local masquerading as
  SATURATION; a field-copy pair wanting whole-struct assignment; GCC's own
  scheduling misread as source order. Round 19 closed 17 in one round. (a §"A
  \"register-identity\" verdict is the least reliable class", §"Round 19
  confirmed the class")
- **It is a HYPOTHESIS about a mechanism; "the registers differ" does not
  establish it.** Differing registers fits both GCC wanting a different
  ALLOCATION (terminal) and GCC scheduling freely with allocation falling out
  (source-fixable) — ask what forced retail's order that does not force yours; a
  sibling filed terminal for three rounds closed 14/14 on a type change. Screen
  with `--debug`: one census member had `Register Differences: 0`, and a
  differing register COUNT is not identity at all. (a §"A register-identity
  verdict is a HYPOTHESIS", §"Round 27: the register COUNT and the
  addressing-mode IMMEDIATE")
- **"Pure register rotation" — zero reorderings, insertions and deletions, only
  register differences — IS a stall under project rules**, and `--debug`
  establishes it early. **"Tail merge" is an UMBRELLA, not a class**: merge COUNT
  versus merge DEPTH, neither lever transferring and both regressing when
  swapped, with count-vs-depth readable off the disassembly. (a §"New residue
  classes opened this round", §"\"Tail merge\" is an UMBRELLA")
- **Callee-saved demand is a ONE-DIRECTIONAL screen: use 7+ to DEPRIORITISE,
  never to decline.** splat writes `$fp` where objdump writes `s8`, so a screen
  written for one undercounts by exactly one; a LOW count predicts nothing. The
  threshold was corrected TWICE (5 -> 7, then the 7+ band matched byte-exact at
  8), because a screen built only from the failures it predicted needs a
  deliberate attempt on its wrong side. If you stop here, name a residue. (a
  §"Round 13: retail's callee-saved-register demand", §"Round 14 CORRECTION: the
  register threshold is 7, not 5")
- **A saturated file (8 of `$s0`-`$s7`) is its own class with a permitted lever:
  reduce the values live across each call.** It looks catastrophic (40/145) with
  exact length, zero ins/del and purely `r`-tagged renames, and is NOT the
  scheduling class — a sibling saving 6 registers had a delay-slot residue. Two
  residues live above the threshold and share no fix: a full PERMUTATION at zero
  drift, and a missing-register FRAME-SIZE gap. (a §"New residue classes opened
  this round", §"Round 13: retail's callee-saved-register demand")
- **A blocker screen cannot see OWNERSHIP — measured twice.** Fourteen stalled
  functions (1669 words, ~4100 lines of derivation) lay fully inside placed Sony
  objects while passing every blocker screen, so they read as the cleanest ground
  in the queue while being unmatchable by construction. Run
  `python3 tools/sdkstalls.py`. A screen measures the obstruction it was built
  for. (a §"14 stalled functions were Sony library code", §"A blocker screen
  cannot see OWNERSHIP", §"SDK code hides inside game segments")
- **"N words short" and "N/M words match" are DIFFERENT measurements that read
  identically.** A title must carry LENGTH, RAW WORD-MATCH and WHERE THE FIRST
  REAL DIFF IS — a body can be exactly the right length and match almost nothing
  (144/145 compiled, 50/145 raw), and once it drifts the in-range score is
  meaningless. Figures at different LENGTHS are not comparable, and a raw match
  may legitimately DROP as a function gets closer (278/282 -> 282/282 with raw
  132 -> 98). (a §"\"N words short\" and \"N/M words match\"", §"Two figures
  measured at different LENGTHS")
- **A near-miss word count describes WORD COUNT, not the number of
  divergences.** A 216/217 "one-word residue" scored 465 on `--debug` because two
  equal divergences hid behind a coincidence; a caller's score never validates a
  callee; "one instruction short" can be FOUR stacked residues; and an
  instruction that MOVED is ONE residue, not two. (a §"A near-miss word count
  describes WORD COUNT", §"Confirmed on this game", §"Count an instruction that
  MOVED")
- **A lever's NEGATIVE is scoped to the (function, lever, STATE) triple, and so
  is a POSITIVE.** A guard polarity inert in round 19 closed three words in round
  33; an arm swap that regressed one round was the biggest fix in another; a fix
  rejected in rounds 19 and 20 closed the function in round 49. **If you have
  changed anything else since a lever was rejected, the rejection has expired.**
  Write the surrounding state into the inert-lever line. (a §"A lever's NEGATIVE
  is scoped to the state it was tested under", §"A lever's NEGATIVE result is
  scoped", §"A recorded negative is scoped to the STATE", §"A lever's polarity
  verdict is scoped")
- **A long attempt list is not a broad one — list the AXES you varied.** Seven
  reshapes all varying expression form and none the control flow; twenty-five
  variations all keeping the same cached locals. If one axis has all the entries,
  that is the tell, and "N hand rephrasings failed" bounds the rephrasings TRIED,
  not the residue's reachability. (a §"Method lessons about ATTEMPT BREADTH",
  §"Late addendum — a twelfth lever")
- **A lever found on one function is evidence about THAT function until a second
  confirms it, and broadcasting it needs a request for the NEGATIVE.** Round 27's
  three levers each closed a real function and each made a different one worse;
  round 48 repeated the shape. Similarly **a class several reports agree on may
  be one error copied**: re-derive from asm-differ/objdump before acting on a
  report's DESCRIPTION, since four runners in four units each found a false
  mechanism claim in one round, one with retail and built swapped. (a §"Round
  27's HEADLINE", §"The meta-lesson, which is the same one twice", §"The
  frame-padding idiom recovers frame ALIGNMENT", §"An inherited report's PROSE
  can be wrong")
- **A residue next to a just-fixed defect may be the same root cause wearing a
  different face — test the transfer, do not assume it either way**, and **two
  entangled residues can each be WORSE alone and correct together**, so a change
  with independent evidence is worth keeping while you keep looking. (a §"A
  residue next to a just-fixed defect", §"Confirmed on this game", §"Round 14")
- **"Unscoreable" describes a SESSION's state, not a function's — compile a
  salvaged body as-is FIRST** (two matched 56/56 and 119/223 with no reshaping;
  say "no stop rule was applied" in the verdict line). Equally, **"this was
  exhausted before I arrived, and here is the evidence" is a result** and a
  zero-match pass is not a wasted pass. Budget a session by ATTEMPT HISTORY, not
  by score. (a §"\"Unscoreable\" describes a SESSION's state", §"Distinguishing
  \"spent\" from \"hard\"", §"Two levers from the re-send")
- **A screen over PROSE reports is a hint to verify, never a fact to brief.**
  Round 37 caught it counting the WORD "permuter"; round 41 found it counting a
  sibling's cited iteration count as this function's (6 hand-verified) and a
  scaffold CHECK as a run; round 49 found `15862+` scoring as never-searched.
  Worst: a scaffold rejected at the sanity gate is a STRONG negative leaving no
  iteration count, so a numeric screen ranks the least-searchable functions to
  the TOP. (a §"Gate 1b's sixth screen is neither SOUND nor COMPLETE", §"\"Was it
  searched\" is not a text-mining question")
- **Order a fresh unit by SIBLING GROUP, not size, and count trivial functions
  separately** — a fresh unit can be a third `jr $ra; nop` one-liners splat
  generated itself. When a function resembles a matched sibling, copy the
  sibling's exact local-versus-repeated-field idiom first; and an idiom recorded
  in one report is a candidate for every SIBLING in its unit (two round-33 levers
  were worth 5 and 10 words on untried functions). (a §"A same-size sibling
  family is a single unit of work", §"Round 13, final batch", §"Confirmed on this
  game", §"Two idioms that generalise further")
- **"The cheapest phrasing still OVERSHOOTS" is a distinct outcome from "no lever
  found"**, and a reshape fixing one named sub-residue can be a NET regression —
  trading 1-word-short for 1-word-long is not a wash. Bracketing the target
  between two adjacent expressible forms is how to argue a shape is unreachable.
  (a §"Two diagnostic findings worth as much as the levers", §"A reshape that
  fixes one named sub-residue", §"One named C variable gets ONE storage
  location")
- **An unpromoted learning does not exist** — an idiom left in one report cost a
  stall four addresses away. A claim of "unconditional" compiler behaviour tested
  along ONE axis is not a claim about the shape space: two were measured and
  rejected. Record the direction you did NOT take and why — a preserved body's
  "untried direction" note has outperformed a fresh derivation — and when two
  runners report the same anomaly, sweep the corpus before writing either up. (a
  §"How to read a one-instruction residue", §"Proposed learnings that did NOT
  earn promotion", §"Round 13 (2026-09-03)")
- **Open questions, carried unchanged.** The class-table header word at `+0x000`
  is a class id at least 20 bits wide, not 12 — find the WRITES. Four
  dead-looking data slots are probably method tables seen from the data side; the
  `classtable.py --scan` cross-reference is cheap and unrun. The `psyq_*`
  boundary at `0x39c80` rests on one name. Untested: member loads hoisted into a
  local at a loop's top, and a struct pointer to a global block. Comma
  expressions were TESTED for the two-exit case (byte-identical); single-exit is
  open. K&R definitions for a dead-but-real parameter are PROPOSED, NOT
  CONFIRMED. (a §"Open questions", §"Still unconfirmed here", §"Round 45
  addendum")

## 5. Withdrawn or SDK-voided — do not re-add

An entry here rests on Sony's linked SDK objects (built by ASPSX, not our pinned
pipeline, so no source shape of ours reached those bytes) or was retracted on
measurement. **Exception: a mechanism confirmed by a standalone reproducer
through the pinned pipeline, or one where our whole-image oracle went green on a
C body BEFORE the reclassification — those stand and are kept above.** Census
method and the surviving SDK-exit precedent (`func_8003FC70`, the
duplicated-rodata-string signature) are in a §"The SDK-exit census, re-run over
this document".

- §"A `for`-loop keeps a status value register-resident where `goto`/labels folds
  it away" — INSTANCE-LEVEL VERDICT WITHDRAWN (round 45); both functions are
  `libcd/sys.o`. Only "try the other loop spelling" survives, untested.
- §"A third escape from the `sltiu` boolean-materialization fold" — INSTANCE
  WITHDRAWN (round 47). The SHAPE survives as an untested hypothesis; round 17's
  two escapes stand.
- §"Round 27's HEADLINE" — **address-taken parameter** is 0-for-4 on game code
  (its positive is libcard, never matched). **Invert the guard** and **inline
  every call site** STAND: our oracle went green on C bodies for both. The
  section's conclusion — a lever needs a stated discriminator or it is a coin
  flip — stands.
- §"Round 27: two source levers for a residue that looks like register identity"
  — section 2's HEADLINE (guard polarity flips which arm falls through) CONFIRMED
  by reproducer, kept in 3a; its COROLLARY (test arm polarity first on a
  1-word-short function) WITHDRAWN, the flip is length-neutral. Section 3's
  headline is FALSE as stated; its caveat is the whole rule, kept in 3f.
- §"Round 27: the DCE-eliminated always-true check gets a structural hypothesis"
  — VOID as a game-code class (round 43); both instances are `libcd/iso9660.o`.
  The METHOD survives (three spellings sharing one local are one attempt; a
  global-sourced condition survives DCE, by reproducer); no instance verdict
  does.
- §"New residue classes opened this round" — the round-16 **retry-loop driver
  cluster** and the **commutative-operand SLOT order in `addu`** (16 instances
  inside ONE Sony function) are CLASSES WITHDRAWN (round 47). Residue 1's
  from-scratch reproducer survives; the prologue register-mapping puzzle and the
  `rs`/`rt` slot swap do not, and an `rs`/`rt` swap in game code is a NEW finding
  needing its own reproducer.
- §"Round 16" **framed wrappers** — the READING HEURISTIC survives; the CODEGEN
  claim *"GCC 2.6.3 here always pays for the frame"* was promoted on four Sony
  functions and has NO pinned-pipeline evidence. Re-earning it costs one
  reproducer nobody has run.
- §"Round 16" **`volatile` cast as a codegen lever**, worked example — WITHDRAWN
  (round 44); `func_8002C048` is `libc2/strcmp.o`. The PRINCIPLE (prefer removing
  the thing being CSE'd) and round 23's MMIO carve-out survive, in 3h. The same
  round's **`func_800323A8`** worked example for the loop-guard and
  loop-carried-multiplicand rules is likewise Sony's since round 33: both claims
  stand on their standalone reproducers, but do not read the function as an
  example.
- §"Two DISTINCT permuter false-lead patterns" and §"A residue next to a
  just-fixed defect" — their libcd instances are illustrations, not measurements.
  Pattern 2's MIPS-encoding half and echo's Entity counterweight are game code.
- §"NEW STALL CLASS: retail recomputes an address our GCC CSEs away" — its
  original worked example was withdrawn as Sony's `strcmp`, but `func_8003E968`
  is game code and the class is now CLOSED by the asm-label alias in 3h.
- **Not withdrawn, listed to stop them being re-proposed as a FIFTH way a score
  lies:** the forgotten-`padNN` struct insertion (round 13), drift misattributed
  to the function under the cursor (round 20), the jump-table funcdiff window
  (round 27, FIXED), and a length-short function shifting `.bss` (round 36). In
  every one the oracle went RED and was right; filing them as oracle defects
  would teach the next runner to distrust it exactly where it works.
