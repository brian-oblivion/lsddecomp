# Decompilation learnings — idiom sheet

Idioms, build hygiene and verdict classes, compressed to the discriminator. **Full
history is `docs/archive/DECOMPILATION_LEARNINGS-full-2026-09-16.md`**, referenced here
as `(a §"heading")` and never repeated.

**Adding to this file.** One idiom = one entry, max 12 lines: a bold rule, its
discriminator (when it applies and when it does NOT), the measured evidence in one
clause, and a pointer to the archive section or `docs/PROGRESS.md` round holding the
story. No new narrative here; nothing is added during a parallel round: runners write
`### Proposed learning` in their report, the head promotes after merging.

## 1. Toolchain facts (proven)

- **GCC 2.6.3 Psy-Q + maspsx + binutils 2.43.1 reproduces retail byte-for-byte.** `-mips1
  -mcpu=3000 -O2 -G0 -funsigned-char -fno-builtin -mno-abicalls`; maspsx
  `--aspsx-version=2.34 --dont-force-G0 --expand-div` plus the flags below. PINNED: a
  suspected problem is an escalation with a reproducer. (a §"Toolchain facts")
- **`cpp` and `cc1` take different flags; C89 only.** `-fno-builtin` is cc1-only and `cpp`
  exits 33 on it. A `//` comment is a parse error reported far from the comment.
  `-no-pad-sections` is required or gas pads `.text`. splat owns `include/*.inc`; hand
  edits die on extract. (a §"Toolchain facts")
- **`include/psyq/INLINE.H` and the LIBGPU `set*` macros are INERT — use
  `include/gte.h`.** Eight vendored headers are CRLF and `cpp` splices `\` only before LF,
  so 1134 macros expand to nothing: valid C, no warnings, no instructions. Hence **a
  negative about a MACRO counts only once you have proved it EXPANDED**. (a §"Toolchain
  facts")
- **The game is plain C with a hand-rolled class framework — never `cc1plus`.**
  Constructors go through the table, entries are 4 bytes against g++'s 8, zero 8-byte
  vtables against 128 flat tables. Resolve slots with `classtable.py`; a patchy `--vs`
  diff means the wrong ancestor, and the longer identical high-slot run is the real
  parent. (a §"The class framework (SETTLED")

### The three RESOLVED blockers

Each is one maspsx flag setting one behaviour, proven inert by a byte-exact rebuild. Do
not screen for them, do not stall on them, do not trust a verdict predating the fix.

| construct | round | flag | doc |
| --- | --- | --- | --- |
| `addiu_at` (indexed global load) | 21 | `--addiu-at` | `research/addiu-at-blocker.md` |
| `gp_rel` (small-data global) | 42 | `--gp-symbols=config/gp-symbols.txt` | `research/gp-relative-blocker.md` |
| `nop_mflo_mfhi` | 42 | `--no-nop-mflo-mfhi` | `addiu-at-blocker.md`, RESOLVED addendum |

(a §"BLOCKED: no C function can reach a small-data global", §"BLOCKED: no C
function can load through a runtime-indexed global", §"BLOCKED: the
`nop_mflo_mfhi` screen runs FORWARD")

- **A screen is a claim WITH A DIRECTION, and a blocker's DEATH is scoped like its life.**
  The `mflo` hazard is `mflo`/`mfhi` followed WITHIN TWO instructions by `mult`/`div`; the
  reverse is not, and a retail `nop` is no exemption (broken both ways by four heads).
  Round 49 likewise killed "pre-round-42 `mflo` negatives are void": nearest `mult`/`div`
  is 8-107 instructions away across all seven candidates. (a §"BLOCKED: the
  `nop_mflo_mfhi` screen runs FORWARD", §"The `--no-nop-mflo-mfhi` re-search")
- **A reproduced mechanism is not a blocker until its CORPUS FREQUENCY is measured.** The
  `addiu`-vs-`ori` immediate reproduces perfectly and occurs ONCE against 1089 `ori`.
  Positive constants are always `ori`, negative always `addiu`, and `objdump` prints both
  `li`, so text-diffing tools are blind (signature: "one word short, no visible diff").
  (a §"A reproduced toolchain mechanism", §"`asm-differ` and the permuter compare TEXT")

## 2. Build hygiene

- **Restore `INCLUDE_ASM` BEFORE `make extract`, every time**, or splat deletes the stub
  you meant to restore and the failure reads like a path typo (3x). **`make clean` deletes
  `asm/`**; `make extract` restores it. (a §"`make extract` while a function is LIVE C")
- **`make extract` after merging any match, then re-run funcdiff** — a stale `.s` gives a
  bogus WINDOW, not just a bogus score (`67808/67808 words match` for an 82-word
  function), and it does not fire reliably. (a §"A STALE `.s` makes `funcdiff` report a
  bogus WINDOW")
- **Read every score with the unit's other siblings reverted to `INCLUDE_ASM`** (`grep -c
  '^INCLUDE_ASM' src/<unit>.c`). One short function shifts every later `.bss` symbol
  image-wide, and a sibling then reads 146/196 against its true 171/196. (a §"A
  length-short function shifts `.bss`", §"Leaving another function's stall body live")
- **Drift's best disguise is a PLAUSIBLE WRONG CONSTANT in the function you are editing**
  — two table bases exactly 0x10 low looked like a symbol bug while the function was 4
  words short, so check `build/lsdde.map` first. Conversely **a low score is not evidence
  of a distant shape** (14/105 with 104 of 105 instructions matching): cross-check length
  with `objdump -d`. (a §"Address drift's most convincing disguise", §"`funcdiff.py`'s
  byte range is not a stalled function's true length")
- **Never size anything by counting a `.s`'s instruction-comment lines** — a
  jump-table-owning function has rodata in its own `.s` with the same comment shape (a
  65533/65533 window, and an `uncarved.py` over-count). Use the declared `nonmatching
  <name>, 0xNNN` size or `glabel`/`endlabel`. (a §"Tooling defect found in round 27")
- **A FLAG change rebuilds NOTHING, so flag experiments start falsely green** — `rm -rf
  build` first; a `-G8` trial reported green and was 19148 bytes off from scratch. Run a
  flags-from-`$(...)` pipeline under `bash -c`: zsh does not word-split, and a comparison
  where BOTH sides fail prints `IDENTICAL`. (a §"Two method errors by the head")
- **Build every inherited body ONCE before trusting any figure in its report.** It fails
  three ways and only the first is loud: a stale symbol, a missing declaration, and a
  preamble that CONFLICTS with the unit's declarations, the last two fatal at `cc1` exit
  33 with no `error:`/`parse error`. One stale build produced fourteen fictional matches.
  (a §"An inherited body fails THREE ways", §"Address drift's most convincing disguise")
- **One preserved body in six carries a false "clean / drift-free" claim; others carry a
  score that was NEVER MEASURABLE, a wrong READING, or a false mechanism behind a correct
  LENGTH.** Require funcdiff's outside-range count to be ZERO **and** the objdump word
  count to equal the `.s`'s declared size; output containing `WARNING` has no score in it.
  (a §"A preserved body's \"clean / drift-free\" claim", §"A report can claim a preserved
  body that does not exist")
- **A prototype for a function ANOTHER unit defines belongs in your `.c`, and a bulk
  rename breaks that silently** (`tools/headercontention.py`). **Retyping a global bare ->
  array silently invalidates a SIBLING's preserved body via pointer decay**: `D_X > 0`
  becomes "is this address nonzero", compiles clean, and survives "rebuilt verbatim"
  (`grep -ln 'D_XXXXXXXX' docs/match-reports/*.md`; address-of safe, value contexts not).
  (a §"Renaming a Sony function into a SHARED header", §"A plain-global-to-ARRAY retype")
- **Verify struct offsets with a host `-m32` `offsetof` build**, and compile immediately
  after drafting any struct over two fields: a missing `u8 padNN[...]` presents only as a
  whole-image SHA1 failure in a function that never touched the declaration. `sizeof` is
  not how this game allocates (`New_X` passes a literal byte count), so appending trailing
  fields is safe, `DreamSys` excepted. A rodata slot holding JUMP TABLES must stay
  attached to its unit. (a §"Confirmed on this game")
- **A `D_XXXXXXXX` in rodata holding a STRING is a symbol to REFERENCE, never a string to
  retype.** splat has already emitted those bytes; a C literal emits a second copy, and
  because `section_order` puts `.rodata` first the whole image shifts and the first
  differing byte lands thousands of bytes AHEAD of the code you edited. Grep
  `asm/data/*.rodata.s` for the symbol first; `extern const char D_XXXXXXXX[];` is the
  only correct spelling. The mechanism is a fact about this build system and stands on the
  round-20 `func_8003FC70` whole-image green, the one SDK-exit precedent that survives.
  (a §"A rodata `D_XXXXXXXX` holding a STRING is a symbol to REFERENCE")
- **Preserve a stalled body in `#if 0 ... #endif`, never in a `/* */` block comment**,
  inlined in the match report with every declaration it needs, positioned where it would
  compile: inside a block comment a line starting with `*` is ambiguous between a
  continuation marker and a dereference. Which of several preserved bodies is
  authoritative is stated in PROSE, so do not build a screen for it. (a §"A report's
  MANDATED preservation form can point at the WRONG body")
- **To learn a shape from a MATCHED sibling, diff its compiled OBJECT against your
  target's retail `.s` -- never its C against your C.** The C comparison shows what the two
  sources say; the object comparison shows what the compiler DID, which is the thing that
  has to agree. Round 62 closed `func_80031890` (73/73, ins 0/del 0) after four rounds
  because the matched `func_80031280`'s compiled tail IS retail's tail instruction for
  instruction; round 32 had compared the same pair in C, found a real `u32`/`u16` mask
  asymmetry, and missed that the sibling caches nothing -- the four cached locals were two
  documented residue clusters at once. Discriminator: use it whenever a sibling of the same
  family is already byte-exact, and read the object, not the source. (a round 62)

## 3. Source-shape idioms

### 3a. Control flow and block order

- **GCC 2.6.3 gives the FALLTHROUGH to whichever candidate is LAST in source order.** An
  arm that must `j` over a join must be written NOT-LAST (explicit `goto` over the block
  that falls through); a duplicated assignment retail keeps in two copies needs the OTHER
  copy last. Closed `func_8005CBC8` 100/100. (a §"An arm that must JUMP")
- **A guard's source POSITION decides where its block lands; its POLARITY does not.** To
  put `return 0` at the BOTTOM as retail has it, the body goes inside `if (cond) { ... }`
  with the return as the function's LAST statement; every early-return and `goto` spelling
  inlines that block at the top instead. (a round 60, `func_8002C278`, 11/76 -> 24/76)
- **Statement ORDER drives load-delay scheduling, and the effect is INVERTED in the
  output.** Writing the `unk10` store before `unk14`, against the store order that appears
  in the compiled code, is what makes cc1 hoist the `lh` into the delay slot. Treat order
  as an independent lever and try it both ways. (a round 60, `func_8002C278`)
- **Not strictly dominant — diagnose per instance.** Round 25 closed three functions with
  FOUR different placement fixes and regressed one already right. Tell: a lone
  unconditional `j` to a nearby join whose delay slot carries real work. Build no screen.
  (a §"THE LEVER IS NOT STRICTLY DOMINANT", §"The mechanical screen for this has NO
  measured precision")
- **"A barrier had no effect" is positive evidence FOR block order**, as is a flat
  permuter plateau. But "a barrier does not transfer" has three causes — block order,
  intra-block scheduling, and DCE (barrier-proof; cite `func_80029C40`) — and an
  `mflo`/`mfhi` is a NEGATIVE indicator. Once block order is RIGHT, "split it into two C
  variables" is actively harmful. (a §"\"A barrier had no effect\"")
- **A shared `return` block lands where the FIRST `return` sits, and two early exits need
  an explicit `goto` to a hand-placed label** — even when both return the SAME value.
  Diagnostic: inverted entry-branch polarity plus a stray early `j`/`move v0,zero` means
  wrong PLACE, not a backwards condition (5/56 -> 15/56). (a §"A shared `return` block's
  PLACEMENT", §"Source-shape levers found this round")
- **Two-return guard: SUCCESS return inside, FAILURE return trailing** — the reverse costs
  two words because cross-jump will not merge those epilogues, and it is the `New_X` null
  check verbatim. **Default-then-override beats two `return`s when the function CHOOSES
  BETWEEN TWO RESULTS** (three closes plus a first-try transfer); a pure accessor has no
  choice to express. (a §"A two-return guard", §"Default-then-override vs two `return`
  statements")
- **`if`/`else` codegen is MECHANICAL: the test is always NOT(what you wrote), the
  `if`-body falls through** — so retail's placement gives the source polarity
  arithmetically (3x), polarity recurs at several nesting levels INDEPENDENTLY, and the
  tell is that only the branch MNEMONIC and two immediates differ. Whichever arm is
  textually LONGER needs to be the fall-through. (a §"A fresh local that only carries one
  branch's result", §"Three more levers from the cold-ground runner")
- **Write a small early exit as an inverted guard — a DEFAULT, not a rule** (3x); round 21
  found the counter-shape, so check which arm falls through. For a >2-arm dispatch
  transcribe retail's CFG literally with `goto`/labels; `break` plus a post-loop `if`
  cannot express "jump PAST a fall-through tail". (a §"Round 15", §"Confirmed on this
  game")
- **GCC 2.6.3's loop optimisations are SYNTAX-GATED, not CFG-gated.** LICM hoists a
  loop-carried literal comparison into a spare callee-saved register, one word SHORT;
  `label: …; if (cond) goto label;` defeats it, and register pressure is not the knob.
  Signature: extra `li $sN,<const>` outside, missing `move` inside. Does not generalise
  across loops even within one function, and does not reach a repeated LOCAL
  STACK-ADDRESS CSE across non-adjacent call sites (round 54, `func_80028920`, two CFG
  shapes, neither); the address-CSE alias fix in 3h is global-only. (a round 54)
- **A redundant guard is NOT dead code — 2.6.3 compiles it literally.** GCC does not
  dedupe an explicit `if` against a loop's implicit entry test (where retail has ONE
  check, `guard + do-while` says so), and a provably-dead `x != 5 && x != 8 && x == 0xA`
  chain is byte-exact while its `switch` rewrite measured 109627 bytes off. (a §"Round
  16", §"Round 15", §"Round 12")
- **Independent global stores are freely reordered, so retail's store order is not
  evidence of source order — and a reorder can close a LENGTH gap** (87/88 -> 88/88 after
  a function had been one word short throughout). Two branches with the same result want a
  combined `&&`, not nested `if`s. (a §"Independent global stores are freely reordered",
  §"Round 27: two branches with the same result")
- **`do { ... } while (0)` is a REAL RTL construct to 2.6.3, not a no-op brace block.**
  The loop pass runs over it, so it can change code a plain `{ }` in the identical place
  does not, and that bare-brace control IS the discriminator. Its effect is not fixed:
  scheduling on `func_8004CAF0`, global register allocation on `func_80033C90`. Saturates
  at one placement. (a round 58, bravo + charlie)
- **Two levers that each MEASURE AS A REGRESSION alone can be byte-exact together.**
  `func_8004CFB8`: halves scored 17/28 and 10/28 singly, 28/28 jointly. Try the product
  before discarding either reading. (a round 58, bravo)
- **2.6.3's `jump_optimize` cross-jumps AFTER register allocation, so identical
  allocations collapse two identical blocks into one and different colours keep them
  apart.** Two tells, same mechanism: a body SHORTER than retail by about one repeated
  block has been tail-merged where retail's CSE made the copies differ (fix at ONE merge
  input; backwards costs 184 -> 203 words), and being N words SHORT where retail has a
  bare `j` into a shared tail you reach by falling through is a COLOUR difference, fixed
  upstream of the colours. Discriminator against the fallthrough rule above: there retail
  jumps and yours falls through; here both sides jump correctly and only duplication
  differs. (a round 58, a round 61)
- **A one-instruction `else` arm leaves NO BLOCK**: reorg steals it into the branch's own
  delay slot and the branch targets the outer join, so "retail assigns this in a delay
  slot, mine assigns it plainly" is evidence about if/else shape, not about the scheduler.
  Tell: a conditional branch targeting the OUTER join with a real assignment in its delay
  slot, the bare unconditional `j` of the fallthrough rule absent. 77/217 -> 208/217. (a
  docs/match-reports/func_80063144.md, round 59)

### 3b. Switch and jump tables

- **For a DENSE switch, arm bodies are emitted in SOURCE order and the table order is
  readable off the binary** — sort arm labels by ADDRESS, map back through the jump table,
  write the cases in that order; the arm falling through into the shared tail is LAST.
  **But measure per function**: if label order coincides with ascending value, reordering
  buys nothing (82/82 ascending versus 69/69 only at `25, 23, 5, 4, 18, 19`). (a §"A
  `switch`'s CASE ORDER is recoverable from the binary", §"The block-order rule extends to
  `switch` CASE order")
- **A dense switch must reproduce retail's JUMP-TABLE WIDTH.** A wider retail table means
  a missing case label, usually an empty arm; `case 47:` widened 33 entries to 48 and
  closed 79/79. A first diff at or just after the rodata table base is a WIDTH problem,
  and the pivot tree depends on the exact value SET, so a GAP is evidence of an empty
  case. (a §"A dense `switch` must reproduce retail's JUMP-TABLE WIDTH", §"Confirmed on
  this game")
- **For a SPARSE switch GCC normalises comparison order to ascending value, so source
  order is NOT recoverable from it.** The tell that it is a `switch` at all is a
  range-split `slti`/`sltiu` mid-chain. SCOPE: needs at least THREE explicit case values;
  at two-plus-default, `switch` and `if`-chain are byte-identical. (a §"A `switch`'s CASE
  ORDER is recoverable from the binary")
- **A genuine `switch` and a logically identical if/else chain are not interchangeable,
  and neither is "the" answer** — a real `switch` moved `func_80032708` 24 -> 65/164 and
  closed `func_8004A070` 48/48, while elsewhere a sparse switch beat a chain that would
  not converge. But `if`/`else-if` and a `return`-terminated sequential-`if` compile
  IDENTICALLY. Never size a residue with the whole-image byte count when a `switch` is
  present — compiling it relocates rodata. (a §"Round 16", §"Round 12")

### 3c. Struct layout, types and widths

- **A type-scoped field rename is enumerated by the COMPILER, and the compiler cannot see
  a `#if 0` preserved body.** Rename in the DEFINITION only and fix exactly the accessors
  the build reports (a whole-tree replace of `slotA4` mislabels five other classes), then
  sweep the preserved bodies the compiler skipped: `tools/stalesyms.py` scans match
  REPORTS, not `src/`. Discriminator: the build is green AND `awk '/^#if 0/{i=1}
  /^#endif/{i=0} i && /->oldName/' src/*.c` still prints (four such accessors left behind
  in round 54). (a round 54)
- **A local's DECLARED WIDTH is a codegen decision and `s16` is the expensive default.**
  An `s16`/`u16` local compared or indexed is re-sign-extended at each use — `sll
  0x10`/`sra 0x10` PAIRS, never scheduling, never movable by a barrier. Declare the LOCAL
  `s32`, keep the FIELD `s16`. Trap: two locals of the same declared type can compile
  differently. (a §"A local's DECLARED WIDTH")
- **Declare a narrow value as wide as the register it lives in.** A byte reused across
  comparisons wants `u32` (kills a spurious `andi`) or `s32` (gets `slt`, not `sltu`);
  `andi 0xff` -> `blez` was +44 raw words. **It can TRADE one defect for another** (15/24
  either way), so read the DIFF: a changed SET of differing words means the lever worked
  and exposed a second defect. (a §"The lever the diagnostic step actually surfaced")
- **A struct of all `s8`/`s16` has alignment 2, and alignment is a TWO-WAY lever read off
  retail's instruction WIDTH.** Alignment 2 makes a whole-struct assignment compile to
  `lwl`/`lwr` + `swl`/`swr` and one stray `s32` breaks it (4x); inversely, if retail's
  tail is `lb`/`sb` where yours merges into a halfword, declare all-`s8` for alignment 1.
  (a §"Source-shape levers found this round", §"Confirmed on this game")
- **Whole-struct assignment instead of a field-copy run is the highest-yield source lever
  found (7 closes, 3 unit families) — but only where the address RESISTS constant
  folding.** It helps for a second pointer, an array index or an out-parameter; it does
  nothing for a compile-time `self + literal` or a naturally-aligned run of same-type
  `s32`s. Arrays need a wrapper struct to test in C89. (a §"Aggregate assignment vs scalar
  field-copy")
- **A struct-layout claim must be settled CORPUS-WIDE: a shared base with a small
  compile-time fold is conclusive, separate `lui`/`addiu` pairs prove nothing.** `addiu
  $t2, $a3, 0x2` is only emittable if the compiler knows they are ONE object. Read the
  STRIDE too (two globals a few bytes apart with a common non-power-of-two stride are one
  array, split by USE), and settle a data-modelling hypothesis by reading the DATA
  SECTION. (a §"A struct-layout claim must be settled CORPUS-WIDE")
- **A mask on the PRODUCT means a HALFWORD array, not a struct array.** Early `sll 3`,
  mid-block `andi 0xffff`, LATE base load, `sll 1` is `u16 woff = (u16)i * 8;` indexing an
  `(s16 *)` — eager in a LOOP, deferred outside one. Do NOT hoist the base pointer.
  General form: when a residue is a MASK, ask WHICH VALUE is truncated. (a §"The \"split
  scaled index\"")
- **`(cond ? p : NULL)[i]` and `((T *)(cond ? p : NULL))->field` are NOT the same
  construct.** An index on a conditional is a tree `PLUS_EXPR` that `fold()` distributes
  into BOTH arms, combining the offset with the true arm's `addiu` and turning a NULL
  false arm into the literal `i * sizeof(T)`; a field reference is a `COMPONENT_REF`
  applied at expand time as the MEM displacement, leaving the arms as written.
  Discriminator: retail shows a CONSTANT base `addiu` plus a per-element displacement on
  the far side of a ternary. (a docs/match-reports/func_8001E7BC.md, round 57)

### 3d. Locals, naming and register identity

- **A named C variable gets ONE storage location for its whole scope, so any new name is a
  new allocno** — treat ANY change to the set of named locals as potentially renumbering
  every callee-saved register, and re-measure LENGTH. Retail's transient rematerialization
  shape still has no C spelling, but its trigger is MEASURED and narrower than naming: two
  pinned-pipeline reproducers show 2.6.3 rematerializes a constant when a delay-slot
  filler materializes it early and an intervening CALL invalidates the caller-saved
  register holding it (no-call control: one materialization). Discriminator: N copies of
  one literal at a merge point can be TWO stacked sub-mechanisms answering to no one
  lever. (a §"One named C variable gets ONE storage location", round 55)
- **The SCOPE of a named local is the lever, not the name.** Declaring it INSIDE the block
  where the value must survive one call closed `func_800357B0` 179/179 after three rounds
  failed by hoisting to function entry. (a §"The SCOPE of a named local is the lever")
- **"Name it, THEN barrier it" is two-part and neither half works alone** — a named local
  plus a bare `__asm__("")` immediately after the declaration took `func_8003D73C` to
  exact length, while named-temp-only reproduces the old baseline exactly. The identical
  lever on a sibling's textually identical line regressed it 114/118 -> 14/118. (a
  §"\"Name it, THEN barrier it\"")
- **What a value is NAMED and how many times it is LOADED are two knobs, and the helpful
  direction is function-specific.** REMOVING a dead reload was worth 12 and 10 words;
  ADDING a cached local pointer was the only thing that stopped strength reduction
  elsewhere. Discriminator: a variant that changes LENGTH means a true register-identity
  wall. (a §"Eliminating a DEAD RELOAD", §"What a value is NAMED")
- **Reuse a provably dead PARAMETER instead of a fresh local** when the local only carries
  one branch's result to one later use (62/62, 58/58); mutating a parameter in place is
  the same move. And **match the NUMBER of live pointer variables retail has**: an
  accumulate-in-place loop wants `cur = p; p++; *cur = …;`, not `p[i]` and not `*p++`, but
  both must be MUTATED or copy propagation collapses the alias. (a §"A fresh local that
  only carries one branch's result")
- **Hoist BOTH values before EITHER is consumed.** If retail's two loads or multiplies are
  ADJACENT with consumers later, the source hoisted both (five closes on functions filed
  as register identity). It addresses load SCHEDULING, so it can be required AND
  insufficient. Preconditions that fail while looking like the shape: a branch-gated
  second load, loads far apart with the first consumed between, induction variables
  already live, delay-slot FILL choices. (a §"Hoist BOTH values")
- **A residue surviving two levers INDEPENDENTLY has not been shown to survive their
  COMBINATION — if they target independent decisions** (68/70 -> 69/70; but on
  `CalcDreamColor` both acted on one fused address expression and all three variants were
  byte-identical). Conversely **levers do not commute**: if a residue MOVES rather than
  SHRINKS, revert before the next. (a §"The combination corollary", §"Levers do not
  commute")
- **A same-size pointer cast in a FUNCTION-SCOPE local can cost a callee-saved register —
  the cost is LIFETIME, not the name** (inline casts took a function to 106/110; a
  case-local temp was byte-identical). Splitting a combined declaration (`T x; x = expr;`)
  is a real lever for a value crossing a CALL boundary (19/19) and inert on parameter
  colour swaps. A uniform register-slot shift signals ONE EXTRA PERSISTENT LOCAL. (a §"A
  same-size pointer cast in a FUNCTION-SCOPE local")
- **Cache a re-read struct field only across a CALL-FREE span; reload after any
  intervening call** — the tell is the wrong NUMBER of callee-saved registers. Converse: a
  value you WROTE and reuse needs a named local. Whether to cache `self->methods` tends to
  be constant PER CLASS, determined from a matched sibling but kept conditional. (a
  §"Round 11", §"Round 12")
- **Two long-standing near-misses closed by DELETING a named value:** removing a
  decrementing pointer local that lives across loop iterations (177/177), and writing a
  division in place into its dying dividend — generally, when the result went to the wrong
  register and one operand is dead after, assign the result back into that operand. Keep a
  pointer computation that is one retail expression as ONE C statement (47/117 -> 95/117),
  and compute a value LAZILY where control flow first needs it. (a §"Two levers that
  closed long-standing near-misses by DELETING a named value")
- **`do { } while (0)` is a REGISTER-PRESSURE lever, not a scheduling lever** — inert
  across four delay-slot-fill sites, regressive on an unrelated matched sequence, a 4-word
  length closer elsewhere, 61/70 -> 62/70 around one early return, catastrophic (66/69 ->
  1/69) applied to all four. Small straight-line bodies only; per-return, never
  whole-function. (a §"`do{...}while(0)` is a REGISTER-PRESSURE lever", §"`do { } while
  (0)` wrapping is a SMALL-BODY lever")
- **The "mention a value twice" lever needs a value with a genuine SECOND, INDEPENDENT USE
  POINT, not merely a second textual mention.** That reconciles round 19's close
  (duplicating `hSpan` into `hSpan2` split a lifetime, closing a register) with the INERT
  entry below: a same-valued alias on a continuously-live value is copy-propagated away,
  measured byte-identical at 55/97 on `func_8004CAF0`'s `self`. Ask which of the two the
  candidate value is before spending an attempt. (round 53, bravo)
- **Swapping an intermediate local for a direct field write is a WHOLE-FUNCTION
  experiment, not a local one.** Removing `func_8004CFB8`'s cosmetic `val` changed cc1's
  block layout enough to regress the function's already-solved FIRST half, 25/28 to 15/28.
  Build and score the whole function after every such swap. (round 53, bravo)
- **Levers measured INERT — do not re-derive.** C89 `register` (the legal form) is a no-op
  for allocation; a clobber-bearing barrier is no better than an empty one; a dummy unused
  SCALAR cannot nudge frame allocation; a same-valued alias is collapsed by copy
  propagation and an algebraic identity rewrite is transparent to value numbering;
  narrowing a cast-local's lexical SCOPE does not help while its LIVE RANGE crosses a
  call. (a §"Levers measured INERT on this pipeline")
- **Three scalar assignments and ONE struct assignment can emit byte-identical
  instructions and still allocate a different register to the POINTER they go through.**
  `emit_block_move` expands a small struct copy as a single unit, so the base pointer's
  pseudo has a different reference count and live shape than when three statements each
  mention it. A register-identity residue on a pointer used by a run of same-shaped field
  copies is worth one struct-assignment attempt. (a
  docs/match-reports/func_8001E7BC.md, round 57)
- **When a residue is a missing register-to-register COPY, try DELETING the named local
  and inlining the expression** — the inverse of the "name the subexpression" lever.
  2.6.3's signed `x / 2**k` (k > 1) opens with `t = x`, and whether that copy survives is
  coalescing driven by `x`'s live range. Discriminator: a one-word copy residue where your
  build reuses a register destructively (`addiu v1,v1,3`) and retail uses a second
  (`addiu a2,v1,3`) is a LIVE-RANGE question. Not the "redundant move" permuter class. (a
  docs/match-reports/func_80062C58.md, round 59)
- **An explicit alias can force the parameter copy cc1 would otherwise coalesce away, and
  "the dead copy gets eliminated" is a per-BODY negative.** `Obj278 *p; p = self;` used for
  even ONE access made cc1 emit retail's `move`, and six of six joint-best permuter
  candidates converged on it. Using the alias EVERYWHERE collapses back to two pointers
  and cc1 coalesces again, so the lever is partial use. (a round 60, `func_8002C278`,
  -> 54/76)

### 3e. Frames and stack

- **An unused stack frame is reserved by an unused local ARRAY, never a scalar.** `s32
  unused[2]` and `s16 unused[4]` green at 8 bytes; one scalar, two scalars, a 4-byte array
  and no local all RED; the `if (0)` guard is INERT. **SCOPE: only when your build emits
  NO `addiu $sp` at all** — otherwise the array STACKS on top (0x20 -> 0x40) and the score
  worsens, because that residue is PLACEMENT. (a §"An unused stack frame is reserved by an
  unused local ARRAY", §"SCOPE — measured")
- **The frame-padding idiom recovers frame ALIGNMENT, not word COUNT** — measured 4 of 4:
  frame exact, word count unmoved. Use it FIRST as a cheap diagnostic realignment, then
  look for a separate companion fix. An allocated-but-unused frame on a leaf function is
  not a residue at all. (a §"The frame-padding idiom recovers frame ALIGNMENT", §"An
  allocated-but-unused stack frame")

- **The frame size bounds how many spilled locals a body can have, so it screens whole
  source shapes before you build one.** Retail's `func_80030980` frame is `-0x38` = 0x18
  outgoing args + 0x20 saved registers = ZERO spill bytes, so no body that forces seven
  scalars to memory can be its shape -- round 50's seven-`volatile` sweep was structurally
  excluded by a number already printed at the top of the `.s`. Read the frame first and
  subtract args and saves; what remains is the spill budget your C must fit. (a round 62)

### 3f. Calls, arguments and return types

- **A value in an ARGUMENT register live at the next call IS an argument, even when its
  only visible use is a branch condition** — it converted two "unreachable
  register-identity" stalls. Negative half: a `$v0` residue or a register dead at the next
  call is NOT this, and a register surviving an intervening call is not reliably an
  argument. (a §"Confirmed on this game")
- **A delay-slot residue writing `$a0`-`$a3` is a CALL ARGUMENT until proven otherwise.**
  Walk forward along the branch-TAKEN path to the first `jal`/`jalr`; a single-path
  liveness check is not one. The cause is usually a vtable slot typed with too few
  parameters, so `grep -rn 'slotNN' src/` across units first. (a §"A delay-slot residue
  writing `$a0`-`$a3`")
- **A bare `nop` in a call's delay slot establishes arity ONLY when the argument is not
  already in the right register** — those two cases are BYTE-IDENTICAL. **MIPS o32 fills
  argument registers strictly left to right**, so untouched `$a1` with `$a2`/`$a3` set
  PROVES a forwarded parameter; the converse fails, so type a slot from its CALL SITES.
  (a §"3. A bare `nop` in a call's delay slot", §"Round 11")
- **A discarded return value is never evidence of `void`**, and an empty-bodied occupant
  is never evidence the slot takes no arguments. Bytes constrain the return type only
  where the caller USES it; prefer the slot's OCCUPANT. (a §"Confirmed on this game")
- **A declared RETURN TYPE can block a cross-jump merge that should happen** — GCC will
  not merge a `void` call against a value-returning call whose result is discarded. When N
  identical calls merge into GROUPS, the grouping PARTITIONS BY DECLARED RETURN TYPE, and
  a 2+2 split is a TYPE mismatch no barrier touches. (a §"Cross-jump shape THREE")
- **Retyping a shared vtable slot is NOT local — prefer a LOCAL function-pointer
  variable.** Assign both differently-typed slots into one `void (*fn)(...)` and call
  `fn`: that forces the merge without the retype that silently broke a matched function in
  another unit. Two symmetric slots needed OPPOSITE answers, and a broken retype is a RED
  BUILD. (a §"Confirmed on this game", §"Round 12")
- **A multi-exit function that stores a literal into a field and returns that same literal
  right after a `jalr` cannot be typed `s32` — try `void`** (18 isolated reproducers). The
  bytes pin the return type in neither direction, and an already-matched signature can be
  too NARROW on both sides. (a §"Round 15")
- **Type the CALLER's parameters so a callee prototype's implicit conversions emit the
  exact per-call narrowing seen in the disassembly.** Read the CALLEE's body for the width
  it uses: an over-narrow `s8` parameter costs a sign-extend at every call site, a `u8`
  lvalue passed to `s32` costs a redundant `andi 0xff`, and an unsigned narrow STACK
  argument loads as one `lhu` where a signed one is `lw`+`sll`+`sra`. (a §"A symbol
  accessed at TWO WIDTHS")
- **A wrong extern arity and the deliberate dead-argument idiom are told apart at the CALL
  SITE, never in the callee** — the callee says "ignores `$a1`" either way. Does retail
  emit an instruction for the extra argument (`move a1,zero`, `li a0,0xff`)? Then the
  declaration is byte-load-bearing: keep the arity, annotate `/* arity-ok: */`. Delay slot
  a callee-save spill or a bare `nop` with the register already loaded? Then a
  disagreement with the definition is false. 15 of 17 were the idiom (round 59; 11 of 18
  in round 58). **The fix that touches no call site is an unspecified list `()`, not
  `(void)`.** (a docs/match-reports/GetClass6B5CCMethods.md, round 59)

### 3g. Delay slots, arithmetic, one-instruction residues

- **A delay-slot instruction executes on the TAKEN path too, and a delay-slot store is
  UNCONDITIONAL.** Evaluate the value at the TARGET: `li $v0, 0x1` in a slot with `sw
  $v0, 0x24(s1)` stores 1, not the 3 the comparison used, and modelling it as conditional
  compiles cleanly. (a §"Read the delay slot before modelling the branch")
- **A "dead-looking" filler may be a WRITE scoped wrong in your C — the discriminator is
  whether it has a MEMORY EFFECT.** A store can hide an unconditional write and is worth
  re-scoping (82/82); a register-to-register move cannot, so there the scheduling
  explanation stands. (a §"A delay-slot filler that \"looks dead\"")
- **GCC hoists an EXISTING instruction into a load-delay slot; it never invents one**, so
  a filler whose result looks dead is a lead about the source. A `move $sN,$v0` in a
  `jal`'s OWN delay slot reads the value `$v0` held BEFORE that call. (a §"Round 13,
  second batch", §"Source-shape levers found this round")
- **When your diff is ONE redundant or ONE missing `move`, count how many times your
  SOURCE mentions the value** — one too many adds a `move`, one too few removes it, and
  the surplus lands in a free delay slot, which is why "different filler" and "one value
  too many" are indistinguishable. First of all: **do the branch TARGETS agree?** A
  differing target is a differing CFG and always comes from the source. (a §"How to read a
  one-instruction residue")
- **Do not transcribe a LOWERING back into C.** `xori`+`sltiu 1` is `x == K`; `sltu
  $r,$zero,$r` is `x != 0`; a same-amount `sll`/`sra` pair is a narrowing signed cast; a
  `mult`/`mfhi`/sign-fix chain is `/` or `%`; a bare `andi` with no sign-fix is `& (N-1)`;
  `~x + 1` is not `-x`. Read retail's CHOICE OF INSTRUCTIONS as evidence about the source
  EXPRESSION. (a §"How to read a one-instruction residue", §"Confirmed on this game")
- **Write the division; do not hand-derive the magic multiply** — `x / 127` and `x / 63`
  compile exactly to retail's sequences. For an unknown divisor probe `cc1` or brute-force
  it: one magic constant encodes different divisors at different post-`mfhi` shifts, and
  the `sll`/`subu` chain disambiguates. (a §"Write the division", §"Round 12")
- **An unsigned range check needs an EXPLICIT cast to get `sltiu`** — `(u32)(x - LO) < N`;
  without it, `slti`: same word count, wrong opcode. A loop bound's `sltiu` means an
  unsigned counter. But the FOLD's desirability is read off the disassembly per function
  (two units, opposite answers, one round). (a §"Round 13 (2026-09-03)", §"Round 13, final
  batch")
- **cc1 canonicalizes a commutative op's register operand order independently of source
  order — SETTLED on six instances in three units.** The decisive test is per-function:
  reverse the operands in C and see whether the output changes. It holds for SYMMETRIC
  addends, NOT where one addend is a base pointer and the expression can be RE-ASSOCIATED
  (51/51), and a local's declared WIDTH can also flip `rs`. (a
  §"Commutative-operand-order canonicalization")
- **Small expression shapes, each backed by a byte-exact match.** `cond ? A : B` and
  `!cond ? B : A` are not byte-equivalent. `d = -50; if (cond) d = 50;` beats a ternary.
  `(x & 1) ^ 1` and `!(x & 1)` schedule oppositely. Indexing a ternary with a nonzero
  constant DISTRIBUTES it into both branches. Comparison operand ORDER decides which load
  comes first. An expression retail evaluates TWICE must be WRITTEN twice. `return
  self->field = N;` is the single-`ori` setter. (a §"Confirmed on this game", §"Round 12")
- **Loop shapes.** `while (*p) { p++; }` and `while (*p++) { } p--;` differ — a guard
  branch jumping TO a decrement means post-increment (25 words). A `for` header's
  multi-variable increment clause has a load-bearing ORDER. Let GCC hoist its own
  invariants (23 words). Reuse ONE counter for a guard and its loop test. A loop-carried
  multiplicand must be RECOMPUTED, not `+=`. A `(u8)`/`(u16)` mask on an induction
  variable used as an ARRAY INDEX controls strength reduction, and its width must match
  the comparison's. (a §"Four narrower confirmations from round 23")
- **Length-gap levers, both directions.** 1 word SHORT with only a missing `move` before
  the epilogue: collapse two return points into one join. 1 word LONG with an extra `move
  $an,$vN` before a load: stop giving a call's return value its own named local. A
  one-word gap SHIFTS everything after it, so a low raw match there is mostly RIPPLE
  (23/121 -> 117/121). (a §"Length-gap levers")
- **Defeat constant canonicalization by routing the constant through an assignment**
  (`0xC0` became `-0x40`); GCC re-associates constant multiplies across a whole expression
  tree regardless of parenthesization, and only a STATEMENT BOUNDARY stops it. Defeat the
  store-flag collapse of `if (cond) return 1; return 0;` with `flag = 1; return flag;`.
  (a §"Defeat 2.6.3's constant canonicalization")

### 3h. volatile and memory

- **`volatile` is the NARROW instrument for the instruction-ORDER class, and it is not the
  banned construct** — it names no register, exactly like the sanctioned bare
  `__asm__("")`. Where three barrier placements across two rounds all regressed, declaring
  three hardware-shadow fields `volatile` matched 14/14 with no body change. (a
  §"`volatile` is a legitimate, and much NARROWER, tool")
- **The discriminator is EVIDENCE that the location is memory-mapped I/O, not whether
  `volatile` helps.** One runner declared a documented `I_STAT`/`I_MASK` pair volatile
  (two closes) and DECLINED on an ordinary global, measuring that it changed nothing. Only
  two units touch hardware addresses. (a §"Round 16")
- **NARROW a `volatile` to the exact access that needs it** — qualifying only the
  WORD-sized field un-fused a div/mod pair while preserving retail's `lh` (193/213 ->
  211/213). When a qualifier lever "works but with a side effect", check whether the side
  effect is intrinsic to the LEVER or an artifact of WHERE it was applied. (a §"NARROW a
  `volatile` to the exact access that needs it")
- **`volatile` has TWO independent effects** — on a global's DECLARATION it controls
  elision and reordering of accesses; through a local `volatile T *` it also controls
  whether the ADDRESS COMPUTATION is folded into the memory instruction, so "I tried it"
  is at most one of two measurements. (a §"`volatile` has TWO independent effects")
- **A sibling's `volatile` set is a HYPOTHESIS to sweep outward from, never a set to
  copy**, and it is not confined to arithmetic locals: `func_80030980` closed to
  length-exact (324/324, zero drift, from 292/324) on six divisor-chain locals PLUS a pure
  address-arithmetic local. Add ONE variable at a time and switch the acceptance test to
  whole-image byte count once close, because **exact length is a qualitatively different
  state from "closest so far"**. Two systematic sweeps (15 builds) found nothing better
  either side, so do not re-budget single-variable perturbation without a new hypothesis.
  (round 50, alpha; `docs/match-reports/func_80030980.md`)
- **`volatile` is the WRONG tool for an ADDRESS CSE, and it is NON-MONOTONIC** — it acts
  on the VALUE CSE and regresses a LICM residue (48/376 -> 6/376). **The address CSE
  yields to the asm-label alias**: `extern T D_8008A8F8_b __asm__("D_8008A8F8");` leaves
  nothing to fold and closed `func_8003E968` 41/41; six byte-verified uses exist, and it
  renames a linker symbol, so it is not HARD RULE 6's banned construct. Cheaper still,
  **try REORDERING first**. (a §"Negatives worth not re-deriving", §"A
  repeated-global-address CSE is defeatable from C89")

### 3i. GTE and inline asm

- **A COP2/GTE instruction's C form is the Psy-Q `gte_*` macro, and `include/gte.h` holds
  the GNU-syntax versions this pipeline can assemble.** Write `gte_stsxy3_g3(prim)`, never
  the `swc2` lines. Each macro's `__asm__ volatile("swc2 $12, 0x8(%0)" : : "r"(ptr) :
  "memory")` passes HARD RULE 6's test: `"r"` leaves the GPR to the allocator and
  `$12`-`$14` are COP2 data registers with no GPR identity to pin. (a §"Toolchain facts")
- **A whole-function `__asm__` is for instructions with NO C form, not constructs that are
  hard to type — and look in `gte.h` FIRST.** `TransformAndCullPoly` was carried as a
  58-word `__asm__` because `rtpt`/`nclip`/`avsz3`/`cfc2` "have no C form"; every one is a
  macro and the body is ordinary branching C, byte-exact first build. (a §"Round 13,
  second batch", §"Toolchain facts")
- **Name the right clobbers, and bracket real branches.** A GPR clobber naming `$2`-`$5`
  on a COP2 block forces spurious evictions that present as a register-identity residue
  elsewhere. A block containing a real branch mnemonic needs an explicit tab-delimited
  `".set\tnoreorder\n\t"` … `".set\treorder\n\t"` bracket. (a §"Round 13 (2026-09-03)",
  §"Toolchain facts")
- **A BARE `__asm__("")` CAN change register allocation, which HARD RULE 6's own test
  calls banned.** On game-neutral code, removing it swaps which register holds each of two
  values, and `__asm__("" ::: "memory")` was byte-identical, so the memory clobber is not
  the risky half. What survives is DIRECTEDNESS: a barrier only perturbs. Say what a
  barrier DID, and stop if you are adding them one at a time until a register lands where
  you want it. (a §"A BARE `__asm__(\"\")` CAN CHANGE REGISTER ALLOCATION")
- **The permitted barrier is inert against every pass that is not the scheduler** —
  measured on cross-jump/tail-merge, loop-preheader emission order, a GCSE hoist at ANY
  placement, and value forwarding. Ask which pass produced the residue first. (a §"The
  permitted `__asm__(\"\")` barrier is inert", §"A GCSE / value-availability hoist is
  immune")
- **Three conditions must hold for a barrier to be worth trying.** (1) A SUBSTITUTION
  residue, not an ABSENT instruction (`built=00000000` is an automatic no). (2) A genuine
  ORDERING decision, not a register CHOICE or cross-block CFG decision. (3) BETWEEN
  straight-line statements in one basic block. Round 20: two wins, four regressions. Test
  its effect on WORD COUNT. (a §"The `__asm__(\"\")` barrier in round 20")

### 3j. Permuter practice and false leads

- **Validate the scaffold in three checks BEFORE searching, and record which you ran.**
  (1) Does it compile and score. (2) Its insertion/deletion penalties — a near-0/0
  scaffold suits a source-mutation search, a 23/44 one wanders. (3) **Does its base score
  AGREE with the same body's score in the real build** — the isolated compile can allocate
  differently for identical source. A negative is not evidence unless check 3 passed. (a
  §"The permuter finding", §"The scaffold's insertion/deletion count is a COST predictor",
  §"A recorded permuter search is only evidence of COST")
- **Check 3's discriminator is AGREEMENT, not zero-ness.** A 6/6 scaffold matching a 6/6
  real build AGREES and paid (19/52 -> 22/52); a 0/0 scaffold is NECESSARY, not
  SUFFICIENT; a PERFECT scaffold score on a non-matching function means the cause is
  OUTSIDE the function. Check 3 licenses the SEARCH, not its output. (a §"A perfect
  permuter scaffold score can be the bad news", §"Check 3's AGREEMENT verdict")
- **`--stack-diffs` is MANDATORY on a frame or offset residue or you get a FALSE ZERO** —
  the scorer normalizes stack-offset differences away, and a false zero is expensive
  because a zero is treated as a lead to translate. (a §"`--stack-diffs` is MANDATORY")
- **A permuter number is in PERMUTER units, and LOWER is not a synonym for CLOSER.**
  `"Reorderings: 2"` is a bucket label; `210` is a weighted penalty published as "1 word
  remaining" for a 22/25 body. Both directions measured: 2735-from-3735 rebuilt as 14/164
  with drift, while sub-base candidates translated to +13 and +24 real words, and a
  local-best deserves MORE skepticism than a zero. **Translate AND measure.** (a §"A
  permuter number is in PERMUTER units", §"A permuter improvement is a LEAD
  unconditionally")
- **Verification must match whether the candidate changes BEHAVIOUR.** A
  behaviour-changing candidate must be traced by hand (hoisting a call across an
  un-unrolled loop's back-edge changed only one branch immediate: 176/177, and wrong). A
  behaviour-preserving one must be real-build verified. (a §"The permuter can produce a
  SEMANTICALLY WRONG candidate", §"A permuter candidate's verification must match")
- **The UB screen must be a FORWARD TRACE, not "reads before first assignment" — the
  scorer never executes anything.** Four of six local-bests had correctness bugs and not
  all were uninitialised reads: one left a variable unset on a rare path, one read a
  reassigned variable under its OLD MEANING. Round 49 adds staleness across a LOOP
  BACK-EDGE and use-before-init from a reordered pair. (a §"The permuter UB screen was too
  narrow", §"The forward-trace UB screen")
- **A permuter improvement can be oracle-confirmed and still be UNSOUND C**, and **a zero
  is validated in ISOLATION and cannot see cross-TU damage.** A 208/223 candidate hoisted
  a string-literal address into a CALLER-saved register outside a call-making loop; a
  genuine zero elsewhere required a shared global `volatile` that corrupted a matched
  sibling. (a §"A permuter improvement can be oracle-confirmed and still be UNSOUND C",
  §"A permuter zero is validated in ISOLATION")
- **A permuter negative is ONE SAMPLE of a stochastic process, in both directions.** A
  zero arrived at iteration 149 where a 33,480-iteration search had plateaued; elsewhere
  163,644 fresh iterations reproduced an identical floor. Rank on whether a search EVER
  beat base, and track a manual `PERM_GENERAL` enumeration separately from an open random
  search. (a §"A permuter negative is ONE SAMPLE")
- **A permuter run that plateaus with NO MOVEMENT AT ALL points AWAY from the residue you
  measured**: it mutates expressions, operand order and temporaries within the statements
  given, and never moves a statement into an else arm absent from its base
  (`func_80063144`, 30485 iterations plateaued, real defect two statement placements,
  closed in 2 builds). **Corollary: on a length-defective function
  `insertions/deletions` is the signal and the word count misleads** — a correct fix ran
  27/27 -> 11/11 -> 7/7 -> 0/0 while the word score went 61 -> 56 -> 91 -> 213, and round
  45 rejected a correct lever on that 5-word drop. (a round 59)
- **An unexplained RELOAD is evidence of a BLKmode assignment upstream, not of register
  pressure.** gcc 2.6.3's `cse.c` answers a BLKmode (struct or array) `set` with
  `invalidate_memory()`, so a whole-struct assignment is a CSE memory barrier and a
  field-by-field one is transparent. Discriminator, one build: retail re-reads a global it
  already read, or a stack slot it just wrote, with NO call in between; write the
  adjacent-globals copy as `pos = *(PairXY *) &D_8008AB68;`. Took `func_80054850` from 6
  words short to 1 and closed its sibling `func_800549A8` 93/93 first attempt.
  **Corollary: a lever found on one function is worth one build on every recorded sibling
  before anything else is tried.** (a round 61)

## 4. Verdict classes and how far to trust them

- **A stall report's MEASUREMENT and its residue CLASS decay at different rates, and the
  class is what goes stale unnoticed.** Both round-59 revisits found the figures correct
  while the class ("delay-slot scheduling" both times) had never been re-questioned and
  was wrong both times; each closed in single-digit builds after 14 and 46 rounds. A
  revisit's first act is to re-derive the CLASS from the disassembly. (a round 59)
- **A register-identity verdict is a claim about the RESIDUE, not about the function, and
  it DECAYS as the rest of the function changes.** All three of round 44's residues on
  `func_8001E7BC` were filed as one class; two were ordinary source-shape differences
  elsewhere in the body. When a stall carries SEVERAL same-class residues, fix the
  STRUCTURE first and re-measure; a figure taken before the last structural change is
  evidence about that older body. (a docs/match-reports/func_8001E7BC.md, round 57)
- **"Register identity" is the LEAST reliable verdict class in this corpus.** It is
  self-sealing and 26% of everything queued. Contaminants found, all ordinary C: a masked
  byte parameter mistyped `s32`; a missing field-offset term; a statement swap between
  independent stores; a stale helper signature; a discarded return on a wrongly-`void`
  function; a cached local masquerading as SATURATION; a field-copy pair wanting
  whole-struct assignment. Round 19 closed 17 in one round. (a §"A \"register-identity\"
  verdict is the least reliable class", §"Round 19 confirmed the class")
- **It is a HYPOTHESIS about a mechanism; "the registers differ" does not establish it.**
  Differing registers fits both GCC wanting a different ALLOCATION (terminal) and GCC
  scheduling freely with allocation falling out (source-fixable); a sibling filed terminal
  for three rounds closed 14/14 on a type change. Screen with `--debug`: one census member
  had `Register Differences: 0`, and a differing register COUNT is not identity at all.
  (a §"A register-identity verdict is a HYPOTHESIS", §"Round 27: the register COUNT and
  the addressing-mode IMMEDIATE")
- **`funcdiff`'s `insertions N / deletions M` line is the mechanical discriminator: 0/0
  means genuine register identity, anything else means separable defects are hiding inside
  the class.** `func_8002C278` carried "pervasive register-allocation residue" for four
  rounds at 7/7 and held four separable defects, three of them plain C. At 0/0 every
  instruction is retail's in retail's order and only the register pairing differs, which
  IS the banned-fix category. Rebuild the preserved body and read the line. (a round 60)
- **"Pure register rotation" — zero reorderings, insertions and deletions, only register
  differences — IS a stall under project rules**, and `--debug` establishes it early.
  **"Tail merge" is an UMBRELLA, not a class**: merge COUNT versus merge DEPTH, neither
  lever transferring. **A saturated file (8 of `$s0`-`$s7`) is its own class** with a
  permitted lever — reduce the values live across each call — and looks catastrophic
  (40/145) at exact length; above the threshold a full PERMUTATION and a missing-register
  FRAME-SIZE gap share no fix. (a §"New residue classes opened this round")
- **Callee-saved demand is a ONE-DIRECTIONAL screen: use 7+ to DEPRIORITISE, never to
  decline.** splat writes `$fp` where objdump writes `s8`, so a screen written for one
  undercounts by exactly one; a LOW count predicts nothing. The threshold was corrected
  TWICE (5 -> 7, then the 7+ band matched byte-exact at 8), because a screen built only
  from the failures it predicted needs a deliberate attempt on its wrong side. (a §"Round
  13: retail's callee-saved-register demand", §"Round 14 CORRECTION")
- **A blocker screen cannot see OWNERSHIP — measured twice.** Fourteen stalled functions
  (1669 words, ~4100 lines of derivation) lay fully inside placed Sony objects while
  passing every blocker screen, reading as the cleanest ground in the queue while
  unmatchable by construction. Run `python3 tools/sdkstalls.py`. (a §"14 stalled functions
  were Sony library code", §"A blocker screen cannot see OWNERSHIP")
- **"N words short" and "N/M words match" are DIFFERENT measurements that read
  identically, and a word count is not a count of DIVERGENCES.** A title must carry
  LENGTH, RAW WORD-MATCH and WHERE THE FIRST REAL DIFF IS: a body can be the right length
  and match almost nothing (144/145 compiled, 50/145 raw), a 216/217 "one-word residue"
  scored 465 on `--debug`, "one instruction short" can be FOUR stacked residues, and an
  instruction that MOVED is ONE residue. Figures at different LENGTHS are not comparable,
  and a raw match may legitimately DROP as a function gets closer. (a §"Two figures
  measured at different LENGTHS")
- **A lever's NEGATIVE is scoped to the (function, lever, STATE) triple, and so is a
  POSITIVE.** A guard polarity inert in round 19 closed three words in round 33; a fix
  rejected in rounds 19 and 20 closed the function in round 49. **If you have changed
  anything else since a lever was rejected, the rejection has expired.** Equally, a lever
  found on one function is evidence about THAT function until a second confirms it, and a
  class several reports agree on may be one error copied, so re-derive from
  asm-differ/objdump before acting on any report's DESCRIPTION. (a §"A lever's NEGATIVE is
  scoped to the state it was tested under")
- **Budget by ATTEMPT HISTORY, not by score, and treat a screen over PROSE reports as a
  hint to verify.** "Unscoreable" describes a SESSION, not a function (two salvaged bodies
  matched 56/56 and 119/223 unreshaped), and a permuter-history screen has counted the
  WORD "permuter" and a sibling's iteration count as never-searched, ranking
  scaffold-rejected functions to the TOP. Rank a fresh unit by SIBLING GROUP, count
  trivial `jr $ra; nop` functions separately, and remember an unpromoted learning does not
  exist. (a §"\"Unscoreable\" describes a SESSION's state", §"Gate 1b's sixth screen is
  neither SOUND nor COMPLETE")

## 5. Withdrawn or SDK-voided — do not re-add

Each rests on Sony's linked SDK objects (ASPSX-built, so no source shape of ours reached
those bytes) or was retracted on measurement. **Exception: a mechanism confirmed by a
standalone reproducer, or one where our oracle went green on a C body BEFORE the
reclassification — those stand and are kept above.** Method and the one surviving
precedent (`func_8003FC70`) are in a §"The SDK-exit census, re-run over this document".

- §"A `for`-loop keeps a status value register-resident where `goto`/labels folds it away"
  — instance verdict WITHDRAWN (round 45), both functions are `libcd/sys.o`.
- §"A third escape from the `sltiu` boolean-materialization fold" — instance WITHDRAWN
  (round 47); the shape survives only as an untested hypothesis.
- §"Round 27's HEADLINE", **address-taken parameter** row — 0-for-4 on game code. The
  other two rows (invert the guard, inline every call site) STAND.
- §"Round 27: two source levers for a residue that looks like register identity" — the
  section-2 COROLLARY (test arm polarity on a 1-word-short function) WITHDRAWN,
  length-neutral; the section-3 headline is FALSE without its `$a0` caveat.
- §"Round 27: the DCE-eliminated always-true check gets a structural hypothesis" — VOID as
  a game-code class; both instances are `libcd/iso9660.o`. The METHOD survives.
- §"New residue classes opened this round" — the **retry-loop driver cluster** and the
  **commutative-operand SLOT order in `addu`** are CLASSES WITHDRAWN (round 47); an
  `rs`/`rt` swap in game code is a NEW finding needing its own reproducer.
- §"Round 16" **framed wrappers** — the reading heuristic survives; the codegen claim
  *"GCC 2.6.3 here always pays for the frame"* has NO pinned-pipeline evidence.
- §"Round 16" **`volatile` cast as a codegen lever** — worked example WITHDRAWN (round
  44), `func_8002C048` is `libc2/strcmp.o`; the principle and the MMIO carve-out stand.
  Its **`func_800323A8`** examples are Sony's too, though both claims keep their
  reproducers.
- §"Two DISTINCT permuter false-lead patterns" and §"A residue next to a just-fixed
  defect" — their libcd instances are illustrations, not measurements.
- §"NEW STALL CLASS: retail recomputes an address our GCC CSEs away" — original example
  was Sony's `strcmp`; `func_8003E968` is game code and the class is CLOSED (3h).
- **Not withdrawn, listed so they are not re-proposed as a FIFTH way a score lies:** the
  forgotten-`padNN` struct insertion (round 13), drift misattributed to the function under
  the cursor (round 20), the jump-table funcdiff window (round 27, FIXED), and a
  length-short function shifting `.bss` (round 36). In every one the oracle went RED and
  was right.

## 6. In the archive only, deliberately not carried

Single-instance or niche idioms that did not earn a line here. They are still
correct; read them in the archive if a residue matches the heading: §"A block
of stores far ahead of a branch may be entirely UNCONDITIONAL", §"A wholly-unused
STACK parameter", §"An oversized outgoing-arg frame is EVIDENCE OF DEAD CODE",
§"Two VLAs, an `$fp` frame, and a rounding immediate", §"A struct RETURNED BY
VALUE reads as a call with its arguments shifted", §"`&arr[i + j]` and
`arr + i + j` are one instruction apart", the cache-the-scalar-not-the-pointer
entry, and four permuter-practice notes superseded by the three-check scaffold
protocol in 3j (libc data points, scheduling-residue reproducer, seed minimally,
search-tail inert forms).
