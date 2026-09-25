# Decompilation learnings — idiom sheet

Idioms, build hygiene and verdict classes, compressed to the discriminator. **Full history
is `docs/archive/DECOMPILATION_LEARNINGS-full-2026-09-16.md`**, referenced here as `(a
§"heading")` and never repeated.

**Adding to this file.** One idiom = one entry, max 12 lines: a bold rule, its discriminator
(when it applies and when it does NOT), the measured evidence in one clause, and a pointer
to the archive section or `docs/PROGRESS.md` round holding the story. No new narrative here;
nothing is added during a parallel round: runners write `### Proposed learning` in their
report, the head promotes after merging.

## 1. Toolchain facts (proven)

### The four RESOLVED blockers

Each is one maspsx flag setting one behaviour, proven inert by a byte-exact rebuild. Do not
screen for them, do not stall on them, do not trust a verdict predating the fix.

Which four, which flag, which round and which research doc: the table in CLAUDE.md, "Open
toolchain blockers", which every runner and head reads first and which owns that list. Do not
keep a second copy here; a stale copy of it is worse than no copy.

(a §"BLOCKED: no C function can reach a small-data global", §"BLOCKED: no C function can
load through a runtime-indexed global", §"BLOCKED: the `nop_mflo_mfhi` screen runs FORWARD")

- **A screen is a claim WITH A DIRECTION, and a blocker's DEATH is scoped like its life.**
  The `mflo` hazard is `mflo`/`mfhi` followed WITHIN TWO instructions by `mult`/`div`; the
  reverse is not, and a retail `nop` is no exemption (broken both ways by four heads). Round
  49 likewise killed "pre-round-42 `mflo` negatives are void": nearest `mult`/`div` is 8-107
  instructions away across all seven candidates. (a §"BLOCKED: the `nop_mflo_mfhi` screen
  runs FORWARD", §"The `--no-nop-mflo-mfhi` re-search")
- **A reproduced mechanism is not a blocker until its CORPUS FREQUENCY is measured.** The
  `addiu`-vs-`ori` immediate reproduces perfectly and occurs ONCE against 1089 `ori`.
  Positive constants are always `ori`, negative always `addiu`, and `objdump` prints both
  `li`, so text-diffing tools are blind (signature: "one word short, no visible diff"). (a
  §"A reproduced toolchain mechanism", §"`asm-differ` and the permuter compare TEXT")

## 2. Build hygiene

- **Restore `INCLUDE_ASM` BEFORE `make extract`, every time**, or splat deletes the stub you meant
  to restore and the failure reads like a path typo (3x). **`make clean` deletes `asm/`**; `make
  extract` restores it. (a §"`make extract` while a function is LIVE C")
- **`make extract` after merging any match, then re-run funcdiff** — a stale `.s` gives a bogus
  WINDOW, not just a bogus score (`67808/67808 words match` for an 82-word function), and it does
  not fire reliably. (a §"A STALE `.s` makes `funcdiff` report a bogus WINDOW")
- **Read every score with the unit's other siblings reverted to `INCLUDE_ASM`** (`grep -c
  '^INCLUDE_ASM' src/<unit>.c`). One short function shifts every later `.bss` symbol image-wide, and
  a sibling then reads 146/196 against its true 171/196. (a §"A length-short function shifts
  `.bss`", §"Leaving another function's stall body live")
- **Drift's best disguise is a PLAUSIBLE WRONG CONSTANT in the function you are editing** — two
  table bases exactly 0x10 low looked like a symbol bug while the function was 4 words short, so
  check `build/lsdde.map` first. Conversely **a low score is not evidence of a distant shape**
  (14/105 with 104 of 105 instructions matching): cross-check length with `objdump -d`. (a §"Address
  drift's most convincing disguise", §"`funcdiff.py`'s byte range is not a stalled function's true
  length")
- **Never size anything by counting a `.s`'s instruction-comment lines** — a jump-table-owning
  function has rodata in its own `.s` with the same comment shape (a 65533/65533 window, and an
  `uncarved.py` over-count). Use the declared `nonmatching <name>, 0xNNN` size or
  `glabel`/`endlabel`. (a §"Tooling defect found in round 27")
- **A FLAG change rebuilds NOTHING, so flag experiments start falsely green** — `rm -rf build`
  first; a `-G8` trial reported green and was 19148 bytes off from scratch. Run flags-from-`$(...)`
  under `bash -c`: zsh does not word-split, and a comparison where BOTH sides fail prints
  `IDENTICAL`. (a §"Two method errors by the head")
- **Build every inherited body ONCE before trusting any figure in its report.** It fails three ways,
  only the first loud: a stale symbol, a missing declaration, a preamble that CONFLICTS with the
  unit's declarations — the last two fatal at `cc1` exit 33 with no `error:`/`parse error`. One
  stale build produced fourteen fictional matches. (a §"An inherited body fails THREE ways",
  §"Address drift's most convincing disguise")
- **One preserved body in six carries a false "clean / drift-free" claim; others carry a score that
  was NEVER MEASURABLE, a wrong READING, or a false mechanism behind a correct LENGTH.** Require
  funcdiff's outside-range count ZERO **and** objdump word count equal to the `.s`'s declared size;
  `WARNING` output has no score in it. (a §"A preserved body's \"clean / drift-free\" claim", §"A
  report can claim a preserved body that does not exist")
- **A prototype for a function ANOTHER unit defines belongs in your `.c`, and a bulk rename breaks
  that silently** (`tools/headercontention.py`). **Retyping a global bare -> array silently
  invalidates a SIBLING's preserved body via pointer decay**: `D_X > 0` becomes "is this address
  nonzero", compiles clean, and survives "rebuilt verbatim" (`grep -ln 'D_XXXXXXXX'
  docs/match-reports/*.md`; address-of safe, value contexts not). (a §"Renaming a Sony function into
  a SHARED header", §"A plain-global-to-ARRAY retype")
- **Verify struct offsets with a host `-m32` `offsetof` build**, and compile immediately after
  drafting any struct over two fields: a missing `u8 padNN[...]` presents only as a whole-image SHA1
  failure in a function that never touched the declaration. `sizeof` is not how this game allocates
  (`New_X` passes a literal byte count), so trailing fields are safe, `DreamSys` excepted. A rodata
  slot holding JUMP TABLES must stay attached to its unit. (a §"Confirmed on this game")
## 3. Source-shape idioms

### 3a. Control flow and block order

- **GCC 2.6.3 gives the FALLTHROUGH to whichever candidate is LAST in source order.** An arm that
  must `j` over a join must be written NOT-LAST (explicit `goto` over the block that falls through);
  a duplicated assignment retail keeps in two copies needs the OTHER copy last. Closed
  `CheckDreamAuxTriggerCondition` 100/100. (a §"An arm that must JUMP")
- **"A barrier had no effect" is positive evidence FOR block order**, as is a flat permuter plateau.
  But "a barrier does not transfer" has three causes — block order, intra-block scheduling, and DCE
  (barrier-proof; `CD_ready`) — and an `mflo`/`mfhi` is a NEGATIVE indicator. Once block order
  is RIGHT, splitting into two C variables is actively harmful. (a §"\"A barrier had no effect\"")
- **A shared `return` block lands where the FIRST `return` sits, and two early exits need an
  explicit `goto` to a hand-placed label** — even when both return the SAME value. Diagnostic:
  inverted entry-branch polarity plus a stray early `j`/`move v0,zero` means wrong PLACE, not a
  backwards condition (5/56 -> 15/56). (a §"A shared `return` block's PLACEMENT", §"Source-shape
  levers found this round")
- **Two-return guard: SUCCESS return inside, FAILURE return trailing** — the reverse costs two words
  because cross-jump will not merge those epilogues (the `New_X` null check verbatim).
  **Default-then-override beats two `return`s when the function CHOOSES BETWEEN TWO RESULTS** (three
  closes plus a first-try transfer); a pure accessor has no choice to express. (a §"A two-return
  guard", §"Default-then-override vs two `return` statements")
- **`if`/`else` codegen is MECHANICAL: the test is always NOT(what you wrote), the `if`-body falls
  through** — so retail's placement gives the source polarity arithmetically (3x), recurring at
  several nesting levels INDEPENDENTLY; the tell is only the branch MNEMONIC and two immediates
  differing, and whichever arm is textually LONGER needs to be the fall-through. **Write a small
  early exit as an inverted guard — a DEFAULT, not a rule** (3x; round 21 found the counter-shape),
  so check which arm falls through. For a >2-arm dispatch transcribe retail's CFG literally with
  `goto`/labels; `break` plus a post-loop `if` cannot express "jump PAST a fall-through tail". (a
  §"A fresh local that only carries one branch's result", §"Round 15", §"Confirmed on this game")
- **GCC 2.6.3's loop optimisations are SYNTAX-GATED, not CFG-gated.** LICM hoists a loop-carried
  literal comparison into a spare callee-saved register, one word SHORT; `label: …; if (cond) goto
  label;` defeats it, and register pressure is not the knob. Signature: extra `li $sN,<const>`
  outside, missing `move` inside. It also covers a loop-invariant STACK ADDRESS: retail recomputing
  `addiu $sN,$sp,K` inside a retry loop while the built C holds it in a saved register (one word
  long, saved registers rotated) is a goto loop, and so is a back edge that targets the argument
  setup rather than the `jal` (a `do/while` copies that setup into the delay slot). Decide per loop:
  `ReadCdFile` has both kinds. `OpenCdFile`/`ReadCdFile` closed this way (round 74) after seven
  rounds filed them as CSE plus register rotation; round 54's "does not reach" was wrong. (a round 54)
  The gate also covers STRENGTH REDUCTION: a function 28 words LONG with an eighth saved register
  and a bigger frame, every loop carrying a second induction variable, closed at 954/954 once all
  13 inner loops were `label: ...; if (--n != 0) goto label;` (`func_80018464`, round 76).
  The MIRROR: a constant retail hoists into `$sN` before a loop (`li s1,4`) needs a `do/while`, since
  loop.c cannot see a goto loop (`Class6B5CC__NotifyTaggedParents`, 48/54 -> 54/54 first build).
- **A redundant guard is NOT dead code — 2.6.3 compiles it literally.** GCC does not dedupe an
  explicit `if` against a loop's implicit entry test (where retail has ONE check, `guard + do-while`
  says so), and a provably-dead `x != 5 && x != 8 && x == 0xA` chain is byte-exact while its
  `switch` rewrite measured 109627 bytes off. (a §"Round 16", §"Round 15", §"Round 12")
- **Independent global stores are freely reordered, so retail's store order is not evidence of
  source order — and a reorder can close a LENGTH gap** (87/88 -> 88/88 after a function had been
  one word short throughout). Two branches with the same result want a combined `&&`, not nested
  `if`s. (a §"Independent global stores are freely reordered", §"Round 27: two branches with the
  same result")
- **`do { ... } while (0)` is a REAL RTL construct to 2.6.3, not a no-op brace block.** The loop
  pass runs over it, so it can change code a plain `{ }` in the identical place does not, and that
  bare-brace control IS the discriminator. Its effect is not fixed: scheduling on `Class866E8__SplitFootprintSlot`,
  global register allocation on `Snd_decrescendo`. (a round 58, bravo + charlie)
- **A one-instruction `else` arm leaves NO BLOCK**: reorg steals it into the branch's own delay slot
  and the branch targets the outer join, so "retail assigns this in a delay slot, mine assigns it
  plainly" is evidence about if/else shape, not about the scheduler. Tell: a conditional branch
  targeting the OUTER join with a real assignment in its delay slot, the bare unconditional `j` of
  the fallthrough rule absent. 77/217 -> 208/217. (a docs/match-reports/Entity__MoodCue81.md, round 59)
- **A bare `__asm__("")` at a basic-block JOIN blocks GCC's eager delay-slot fill and can COST
  instructions.** It is legal under HARD RULE 6 (it moves no value between registers) and still made
  `SpuVmAlloc` worse for four rounds. The discriminator is WHERE it sits: inside a block a barrier
  orders that block's statements, but at a join it denies the scheduler the instruction it would have
  hoisted into the branch's delay slot, costing a nop per join. A barrier that makes the body LONGER
  is this, not a block-order result. (a round 65)
- **The ORDER of comma expressions in a `for` increment clause is a scheduling lever**: swapping
  `i++, p++` to `p++, i++` fixed a loop tail's `addu`/`addiu` pairing, and the inner loop's swap
  closed `Class866E8__DispatchToRectCells` 117/117 after a round-19 permuter bound fired. Try it by
  hand at any loop-tail residue before a search. (round 71)
- **A loop whose ONLY call sits at the bottom, reached by an entry `j`, and is also the loop's first
  action is `for (f(); cond; f())` with the call written TWICE**: cross-jump merges the copies into
  retail's single bottom call. `while (f(), cond)` puts the exit test at the top and is four words
  short. The operand order inside a for-init (`i = 0, p = base`) is a scheduling lever like the
  increment's, deciding which setup lands around an intervening `jalr`. (round 82, code_322b4)
- **One store retail shows at a join may be the SAME store written in both arms**: GCC merges the
  identical stores into the join block, and the label that merge creates stops a following reload
  from hoisting above it. `Class866E8__SplitFootprintSlot` 97/97; the barriers and `do{}while(0)` it had carried were
  compensating for that missing label. (round 71)
- **A stall whose compiled LENGTH differs from retail is a control-flow defect until shown
  otherwise, whatever its title calls it.** `Class86F88__Class86F88`, filed "register rotation, 6/107" since
  round 9, compiled 2 long: an assignment retail runs unconditionally sat under a guard; a running
  max needed a ternary (a field load stored straight back to the same field = `x = (x < y) ? y :
  x`); two initialisations belonged before a call. (PROGRESS round 73)
- **An early `return <const>` near the top can decide delay-slot fillers at LATER branches.**
  Same instructions, equal length, fillers swapped: rewrite as `if (ok) { ...; return 1; } return
  0;`. Closed `DreamSys__TryInstantTeleportLink` 58/63. (PROGRESS round 73)
- **Guard + `do/while` + a leading `__asm__("")` + a `u8 unused[N]` frame filler is ONE wrong
  loop kind, and a plain `for` fixes all three symptoms** (prologue store order, the early
  `move $sN,$aN`, frame size): three `code_55dd4` stalls, one with 192k permuter iterations on the
  wrong shape. Where `i++` sits decides whether its `addiu` fills the `jalr` slot. The reverse also
  occurs (`UpdatePolyBBoxAndCull`: a value computed before the loop-skip test took the slot from the stack
  adjustment), and a retry loop testing its counter AFTER the call is a `while`, not a goto loop
  (`TaskObjF__CheckCardStatus`). Whatever precedes a branch in the source decides its delay slot. (round 75)
- **Two argument set-ups sharing one `jal`, reached by a `j` whose delay slot sets the differing
  argument, are TWO calls GCC cross-jumped**, not one call with an argument chosen in a variable
  (`Entity__MoodCue111`, seven words short: its four `li v0,-0x3C` were each block's first instruction
  copied into branch delay slots). A per-branch `return` blocks the merge; a `void` function lets
  three calls merge (`TaskObjF__func_8004F8A4`). Retail's `j` into a shared STORE is likewise two
  identical stores (`Class86B60__TickNameFieldCursor`). (round 75)
- **When retail keeps BOTH arms of an if/else and the build presets the constant before the branch,
  test the RESULT variable.** jump.c turns `if (x % 20) v = a; else v = 0;` into "set 0, then maybe
  overwrite"; `v = (x / 20) * 20; if (x != v) ...` blocks it and leaves the product in the result
  register (retail's `beq a1,v0`). Ternary, `switch`, both-arm stores and `if ((v = x % 20) != 0)` all
  measured negative (`StyleFillEffectKind2`, 49/79 -> 79/79, round 76).
  The MIRROR: when retail DOES preset (`li v0,1` ahead of every test), write the tests as one `&&`
  chain whose then-arm returns the other constant (`if (a && b && c) return 0; return 1;`); an `||`
  chain or separate `if`s never preset (`IsPointOutOfBounds`, 2/27 -> 27/27, round 77).
- **A one-word `j`-target diff onto a DIFFERENT copy of an identical call tail is a cross-jump
  choice, and the C's rejoin point decides it.** Retail's target is where its source's control
  flow goes next. `if (s == 1) { A(); continue; }` inside `for (;;)` with a
  trailing `if (s == 0) break;` jumps to the loop head's call; `do { ... } while (s != 0)` jumps to
  the tail before the test (`Class6E4F0__RunMainLoop`, 62/63 -> 63/63, round 81).

### 3b. Switch and jump tables

- **For a DENSE switch, arm bodies are emitted in SOURCE order and the table order is readable off
  the binary** — sort arm labels by ADDRESS, map back through the jump table, write the cases in
  that order; the arm falling through into the shared tail is LAST. Measure per function: if label
  order coincides with ascending value, reordering buys nothing (82/82 ascending versus 69/69 only
  at `25, 23, 5, 4, 18, 19`). (a §"A `switch`'s CASE ORDER is recoverable from the binary", §"The
  block-order rule extends to `switch` CASE order")
- **A dense switch must reproduce retail's JUMP-TABLE WIDTH.** A wider retail table means a missing
  case label, usually an empty arm; `case 47:` widened 33 entries to 48 and closed 79/79. A first
  diff at or just after the rodata table base is a WIDTH problem, and a GAP is evidence of an empty
  case. (a §"A dense `switch` must reproduce retail's JUMP-TABLE WIDTH", §"Confirmed on this game")
- **For a SPARSE switch GCC normalises comparison order to ascending value, so source order is NOT
  recoverable from it.** The tell it is a `switch` at all is a range-split `slti`/`sltiu` mid-chain.
  SCOPE: needs at least THREE explicit case values; at two-plus-default, `switch` and `if`-chain are
  byte-identical. (a §"A `switch`'s CASE ORDER is recoverable from the binary")
- **A genuine `switch` and a logically identical if/else chain are not interchangeable, and neither
  is "the" answer** — a real `switch` moved `_SsStart` 24 -> 65/164 and closed `func_8004A070`
  48/48, while elsewhere a sparse switch beat a chain that would not converge; `if`/`else-if` and a
  `return`-terminated sequential-`if` compile IDENTICALLY. Never size a residue with the whole-image
  byte count when a `switch` is present — compiling it relocates rodata. (a §"Round 16", §"Round
  12")
- **N identical two-instruction tails ending `j <exit>` with a store in the delay slot are usually
  ONE statement AFTER the block, not N inside it.** `dbr_schedule` fills a `j L` + `nop` by COPYING
  `L`'s first instruction and retargeting the jump past it, so a single post-`switch` statement is
  replicated into every `break` path and reads as a hand-written copy in each case. Length does NOT
  discriminate — the two shapes are often EQUAL length, ins 0 / del 0, skeleton diffs 0. What does:
  written as real per-case source, 2.6.3 cross-jumps the `default:` copy onto the first identical
  tail, moving ONLY the bounds-check branch target and the jump-table slot for the unhandled
  in-range value — presenting as a 3-byte whole-image diff at 150/151. (a round 67,
  `Class6D4E8__RunRequestQueue`)

### 3c. Struct layout, types and widths

- **A local's DECLARED WIDTH is a codegen decision and `s16` is the expensive default.** An
  `s16`/`u16` local compared or indexed is re-sign-extended at each use — `sll 0x10`/`sra 0x10`
  PAIRS, never scheduling, never movable by a barrier. Declare the LOCAL `s32`, keep the FIELD
  `s16`; two locals of the same declared type can still compile differently. (a §"A local's DECLARED
  WIDTH")
- **Declare a narrow value as wide as the register it lives in.** A byte reused across comparisons
  wants `u32` (kills a spurious `andi`) or `s32` (gets `slt`, not `sltu`); `andi 0xff` -> `blez` was
  +44 raw words. It can TRADE one defect for another (15/24 either way) — a changed SET of differing
  words means the lever worked and exposed a second defect. (a §"The lever the diagnostic step
  actually surfaced") The converse holds for a PARAMETER: an `andi 0xff` at the USE (a call's delay slot) with the prologue's `li` moved is a `u8` parameter (s32/u32: same length, 24/27); an `andi` mid-body before `srl`/`sll` is a `u32` parameter masked by an explicit `x &= 0xFF;` (`u8` drops the mask, `s32` gives `sra`). (round 82, code_322b4)
- **A struct of all `s8`/`s16` has alignment 2, and alignment is a TWO-WAY lever read off retail's
  instruction WIDTH.** Alignment 2 makes a whole-struct assignment compile to `lwl`/`lwr` +
  `swl`/`swr` and one stray `s32` breaks it (4x); inversely, if retail's tail is `lb`/`sb` where
  yours merges into a halfword, declare all-`s8` for alignment 1. (a §"Source-shape levers found
  this round", §"Confirmed on this game")
- **Whole-struct assignment instead of a field-copy run is the highest-yield source lever found (7
  closes, 3 unit families) — but only where the address RESISTS constant folding.** It helps a
  second pointer, an array index or an out-parameter; nothing for a compile-time `self + literal` or
  a naturally-aligned run of same-type `s32`s. Arrays need a wrapper struct to test in C89. (a
  §"Aggregate assignment vs scalar field-copy")
- **A struct-layout claim must be settled CORPUS-WIDE: a shared base with a small compile-time fold
  is conclusive, separate `lui`/`addiu` pairs prove nothing.** `addiu $t2, $a3, 0x2` is only
  emittable if the compiler knows they are ONE object. Read the STRIDE too (two globals a few bytes
  apart with a common non-power-of-two stride are one array, split by USE; two relocations off
  one strided index are two parallel arrays, round 79). (a §"A struct-layout
  claim must be settled CORPUS-WIDE")
- **A mask on the PRODUCT means a HALFWORD array, not a struct array.** Early `sll 3`, mid-block
  `andi 0xffff`, LATE base load, `sll 1` is `u16 woff = (u16)i * 8;` indexing an `(s16 *)` — eager
  in a LOOP, deferred outside one; do NOT hoist the base pointer. When a residue is a MASK, ask
  WHICH VALUE is truncated. **The SIGN of the truncation decides whether the scale shift FUSES:**
  `s16 woff = i * 8;` gives retail's split `sll 19`/`sra 15` (cc1 fuses the sign-extend's
  `sll 16`/`sra 16` with the scale), while the `(u16)` spelling above is a mask and stays
  `andi`+`sll 3`. Round 31 spent three attempts on the unsigned spelling alone.
  (a §"The \"split scaled index\"", round 66 bravo)
### 3d. Locals, naming and register identity

- **A named C variable gets ONE storage location for its whole scope, so any new name is a new
  allocno** — treat ANY change to the set of named locals as potentially renumbering every
  callee-saved register, and re-measure LENGTH. Retail's transient rematerialization shape has a
  measured trigger, and one instance was not rematerialization at all (round 75, below): 2.6.3 rematerializes a constant when a delay-slot filler
  materializes it early and an intervening CALL invalidates the caller-saved register holding it. N
  copies of one literal at a merge point can be TWO stacked sub-mechanisms answering to no one
  lever. (a §"One named C variable gets ONE storage location", round 55)
- **The SCOPE of a named local is the lever, not the name.** Declaring it INSIDE the block where the
  value must survive one call closed `Snd_setVabAttr` 179/179 after three rounds failed by hoisting
  to function entry. (a §"The SCOPE of a named local is the lever")
- **A block move is its own tell: N loads into fresh scratch registers, then N stores, then a
  RELOAD of a word just stored.** That is a whole-struct copy (`pos = *src;`), opaque to later
  passes, so the field is re-read from the stack. It closed `Obj86B60__RefreshSlotView`/`Obj86B60__CommitElementScroll` (8-byte
  pair; the "name it, then barrier it" pair this entry used to recommend was imitating it by
  hand), `Class86B60__TickNameFieldCursor` (3 signed bytes) and `DreamSys__TryStaircaseLink` (10 bytes). (round 75; again round 81,
  `FlatLightObj__SetColor`: 3 `s8` bytes; the per-field copy is 3 words longer)
- **Write an expression twice rather than naming it before a branch, and walk a parameter rather
  than a copy of it.** CSE supplies retail's `move sN,sM` for a second `&tab[idx]`, while a name
  taken before the branch swapped the first address computation (`DriftModelChildren`); an
  `arr = arg0` copy put arg0's entry `move` LAST, read for three rounds as a "whole-function
  argument rotation" whose registers were in fact identical (`StyleFillEffectKind0`). Both also
  apply in reverse: `(*p++ = f()) == NULL` as one expression dissolved a filed rule-6 swap. (round 75)
- **What a value is NAMED and how many times it is LOADED are two knobs, and the helpful direction
  is function-specific.** REMOVING a dead reload was worth 12 and 10 words; ADDING a cached local
  pointer was the only thing that stopped strength reduction elsewhere. A variant that changes
  LENGTH means a true register-identity wall. (a §"Eliminating a DEAD RELOAD", §"What a value is
  NAMED")
- **Reuse a provably dead PARAMETER instead of a fresh local** when it only carries one branch's
  result to one later use (62/62, 58/58); mutating a parameter in place is the same move. **Match
  the NUMBER of live pointer variables retail has**: an accumulate-in-place loop wants `cur = p;
  p++; *cur = …;`, not `p[i]` or `*p++`, but both must be MUTATED or copy propagation collapses the
  alias. (a §"A fresh local that only carries one branch's result")
- **Hoist BOTH values before EITHER is consumed.** If retail's two loads or multiplies are ADJACENT
  with consumers later, the source hoisted both (five closes filed as register identity). It
  addresses load SCHEDULING, so it can be required AND insufficient. Fails to apply: a branch-gated
  second load, loads far apart with the first consumed between, induction variables already live,
  delay-slot FILL choices. (a §"Hoist BOTH values")
- **A residue surviving two levers INDEPENDENTLY has not been shown to survive their COMBINATION —
  if they target independent decisions** (68/70 -> 69/70; but on `CalcDreamColor` both acted on one
  fused address expression and all three variants were byte-identical). **Levers do not commute**:
  if a residue MOVES rather than SHRINKS, revert before the next. (a §"The combination corollary",
  §"Levers do not commute")
- **And the MIRROR, which is the more surprising half: two levers each measured BYTE-INERT
  are not jointly inert.** On `StyleFillEffectKind2` a 495/1900 permuter candidate split into (a) a
  single-use store alias and (b) a named local for a global load passed as an argument: each
  alone byte-identical in-tree, the two together moved ins 7/7 -> 3/3. The same pseudo then
  carries two unrelated values in two DISJOINT live ranges, which splits its lifetime, and
  REUSE is a property of the PAIR — a single-use alias is copy-propagated away. Consequence:
  screening a candidate by halves, two inert halves do NOT license discarding it.
  (a round 64, charlie)
- **`do { } while (0)` is a REGISTER-PRESSURE lever, not a scheduling lever** — inert across four
  delay-slot-fill sites and regressive on an unrelated matched sequence elsewhere, but catastrophic
  (66/69 -> 1/69) applied whole-function. Small straight-line bodies only, per-return, never
  whole-function. (a §"`do{...}while(0)` is a REGISTER-PRESSURE lever", §"`do { } while (0)`
  wrapping is a SMALL-BODY lever")
  **It also restores LOOP-DEPTH weighting**: local-alloc weights a pseudo by the loop nesting it sits in,
  so after goto loops remove a level, a `v0`/`v1` swap repeating at every site of one statement closes
  by wrapping just that statement in `do { } while (0)` (`func_80018464`'s six CLUT updates, round 76).
- **Levers measured INERT — do not re-derive.** C89 `register` (the legal form) is a no-op for
  allocation; a clobber-bearing barrier is no better than an empty one; a dummy unused SCALAR cannot
  nudge frame allocation; a same-valued alias is collapsed by copy propagation and an algebraic
  identity rewrite is transparent to value numbering; narrowing a cast-local's lexical SCOPE does
  not help while its LIVE RANGE crosses a call. (a §"Levers measured INERT on this pipeline")
- **Three scalar assignments and ONE struct assignment can emit byte-identical instructions and
  still allocate a different register to the POINTER they go through.** `emit_block_move` expands a
  small struct copy as a single unit, so the base pointer's pseudo has a different reference count
  and live shape than when three statements each mention it. Worth one struct-assignment attempt on
  a register-identity residue on a pointer used by a run of same-shaped field copies. The mechanism
  (round 73): the inline copy (`movstrsi_internal`) CLOBBERS `$v0`/`$v1`/`$a0`/`$a1`, so a parameter
  live across a whole-struct assignment loses its incoming register; an unexplained entry `move
  $a3,$a0` next to batched `lw/lw/sw/sw` is the tell (`Class6E99C__PushPosition`). (a
  docs/match-reports/func_8001E7BC.md, round 57)
- **When a residue is a missing register-to-register COPY, try DELETING the named local and
  inlining the expression** — the inverse of the "name the subexpression" lever. 2.6.3's
  signed `x / 2**k` (k > 1) opens with `t = x`, and whether that copy survives is coalescing
  driven by `x`'s live range. Discriminator: a one-word copy residue where your build reuses
  a register destructively (`addiu v1,v1,3`) and retail uses a second (`addiu a2,v1,3`) is a
  LIVE-RANGE question, not the "redundant move" permuter class. **The live range must cross
  a CALL**: one `s32 r` reused for three `rand()` results cost one `move $a1,$v0` per call and
  deleting it closed 25/87 -> 87/87, while deleting a SINGLE-USE local measured exactly inert
  in the same unit. (a docs/match-reports/Entity__MoodCue78.md, round 59; a round 64, charlie)
- **Local COUNT is the lever on a register-identity residue in all THREE directions — delete, merge
  and add — not only delete.** Round 65 worked one unit's three stalls at once: deleting four cached
  tail temps made `vmNoiseOn2`'s whole 25-instruction tail byte-exact and fixed two registers four
  blocks UPSTREAM of the deletion; reusing an already-dead local in `SpuVmKeyOnNow` was worth 1700
  asm-differ points where a fresh one was worth 500; ADDING a hoisted base pointer moved
  `SpuVmAlloc`. The delete direction needs a CALL-crossing live range, so it does not apply to
  leaves. A permuter never merges or deletes locals, so local count is a PARAMETER of its search
  space: `Class866E8__ComputeRateEntry` (four locals into two) and `Class866E8__BuildRateEntries` (one deleted) closed after ~330k
  iterations missed both. (a round 65; round 63)

- **A register swap that REPEATS at every expansion of a `do { } while (0)` macro closes as a
  `static __inline__` function.** The inline's parameters get their own pseudos at each call, so
  the allocation order differs from textual expansion. `getintr`'s 8-byte copy, 10 sites
  with dst/counter swapped, went 253 -> 333/337, ins/del 0/0, on this alone. Discriminator: the
  SAME swap at every copy of one repeated block. (round 70, delta)

- **A base-plus-running-offset walk in retail is strength reduction of `&self->arr[i]`**: write
  the index and let GCC produce the walk and its delay-slot increment. Hand-rolling `(u8 *)self +
  off; off += 0x1C` reproduces the arithmetic but not the schedule (`Class866E8__ResetAllElements`,
  9/74 -> 74/74 on the first build). (round 71)
  Likewise a register stepping by a constant beside the counter is GCC's own `i * K`: write
  `table + i * 3`, not a hand-stepped counter (`StyleBuildDecorSet`, 17/86 -> 86/86, round 76).
- **A value in a callee-saved register with no call visibly crossing it was ASSIGNED before a
  call**: GCC sank the computation into a delay slot after the call, which reads as a pointless
  promotion. Move the assignment to right after its input is loaded (`Class866E8__ComputeFootprintFromRotation`, filed as a
  HARD RULE 6 stall in round 34, 85/165 -> 131/165 length-exact). (round 71)

- **A full parameter register swap at 0/0 can be global-alloc PRIORITY, and `cc1 -dl` measures
  it**: `floor_log2(refs) * refs / live_length` per pseudo, highest takes `$s0`. Two separate
  `methods->slotNN(self)` calls gave `self` one reference too many; retail picks `fn` per arm and
  makes ONE `fn(self)` call (its tell: a shared `jalr; move a0,sN` tail), so `arg1` outranks it
  (`IntermediateBase__SetState` 32/32, "unreachable" since round 13). (round 72)
- **Split a variable REUSED for two unrelated values** — the inverse of the delete-a-local lever.
  Discriminator: an equal-length rotation among saved registers, one local assigned in two
  independent halves. Closed `Class86F88__LoadResources` (three-way rotation, 75/95) on the
  first build and `Class866E8__LoadElementResources` 142/150. (PROGRESS round 73)
- **An address whose BASE is `$a0` while the object lives in `$s0` means a second C variable
  aliasing `this`.** Scheduling and delay-slot filling move instructions but never change which
  register an address uses. Closed `DreamSys__StepLookYaw` (1 short). (PROGRESS round 73)
- **The same instruction in BOTH arms' delay slots is ONE source statement after the join**, copied
  by reorg; writing it per arm adds references, raises that pseudo's global-alloc priority and
  rotates every saved register. Also: two loops may share ONE counter. Closed `ServiceSoundCueSet`
  110/132. (PROGRESS round 73)
- **`(u8)x` creates an 8-bit temporary that stays live in a saved register; `x & 0xFF` does not.**
  The tell is an entry `move aN,sM` nothing explains; a bound test may then need `(u32)`. Closed
  `SePitchBend` after four rounds. (PROGRESS round 73)
- **Commutative `+` operand order is fixed at RTL generation: a FIELD-LOAD operand goes second
  whatever the source order; a NAMED local keeps its source position** (`cc1 -dr`). So flipping
  textual order is inert, and a field load FIRST in retail means the source had it in a local;
  assign it inside the expression (`a < (w = r->f) + tol`) to keep load order. Closed
  `Class866E8__FindElementForPosition` 69/70 after ~183k permuter iterations. (PROGRESS round 73)
- **A `+4` walker set up from an ARGUMENT register after a loop's count check is GCC's loop
  optimiser, not a C pointer**: advance the parameter itself and take `&p->field` inside the body; a
  C-initialised second pointer always lands before the check. Closed `Class866E8__ApplyRateEntries`. (round 73)

### 3e. Frames and stack

- **An array or struct gets its stack slot when its declaration expands; an address-taken SCALAR only
  at its first `&`**, so a lone scalar always lands after every array and no declaration order moves it.
  A scalar/array slot swap that resists reordering is ONE struct (`{s32 count; Vec3S16 v[8];}` in
  `Class6B5CC__TryAttachNearby`, 140/143 -> 143/143 after seven order variants failed, round 76).

- **An unused stack frame is reserved by an unused local ARRAY, never a scalar.** `s32 unused[2]`
  and `s16 unused[4]` green at 8 bytes; one scalar, two scalars, a 4-byte array and no local all
  RED; the `if (0)` guard is INERT. **SCOPE: only when your build emits NO `addiu $sp` at all** —
  otherwise the array STACKS on top (0x20 -> 0x40) and the score worsens, because that residue is
  PLACEMENT. (a §"An unused stack frame is reserved by an unused local ARRAY", §"SCOPE — measured")
- **The frame-padding idiom recovers frame ALIGNMENT, not word COUNT** — measured 4 of 4: frame
  exact, word count unmoved. Use it FIRST as a cheap diagnostic realignment, then look for a
  separate companion fix. An allocated-but-unused frame on a leaf function is not a residue at all.
  (a §"The frame-padding idiom recovers frame ALIGNMENT", §"An allocated-but-unused stack frame")
- **The filler follows the block rule too.** Untouched frame bytes ABOVE an inner-block aggregate
  are an unused aggregate declared in that SAME inner block, after it: outer-block address-taken
  scalars get the lower slots, inner-block aggregates stack above them in declaration order
  (`Unk18Obj__DrawNode`, an unused `SVECTOR` beside `VECTOR pos`, frame 0x90 -> 0x98, round 81).

### 3f. Calls, arguments and return types

- **A value in an ARGUMENT register live at the next call IS an argument — including a delay-slot
  residue writing `$a0`-`$a3`, which is a CALL ARGUMENT until proven otherwise**, even when its only
  visible use is a branch condition. Walk forward along the branch-TAKEN path to the first
  `jal`/`jalr`; a single-path liveness check is not one. It converted two "unreachable
  register-identity" stalls. Negative half: a `$v0` residue, a register dead at the next call, or
  one merely surviving an intervening call is NOT this. The cause is usually a vtable slot typed
  with too few parameters — `grep -rn 'slotNN' src/` across units first. (a §"Confirmed on this
  game", §"A delay-slot residue writing `$a0`-`$a3`")
- **A slot `jalr` with `$a0` never set before it (it still holds `self` from entry) may be a
  ZERO-argument call in the source**: 2.6.3 emits `move $a0,$s0` for an explicit `self` even when
  `$a0` already holds it. Declare the slot UNPROTOTYPED (`T (*slot)();`) in the local view so
  callers passing `self` stay legal C. **Neither form is a safe default**: the same situation matched
  as `slot(self)` in `func_80045C94` (the zero-argument form moved a constant out of the delay slot
  and ran one word long) and as `slot()` in `func_80048BC0` and `TileMap__Load`. Try the other
  first when one misses. (round 82, code_39094 and code_33808)
- **The same forward trace applies to a DEAD PARAMETER's register, which 2.6.3 reuses as scratch.**
  A delay-slot `move $aN, $vM` is filler only once `$aN`'s next READ on every path is found; in
  `SeqPlay` a store two blocks on read it, so the source stored the wrong value (the unused
  parameter instead of a local). A permuter never swaps which variable a statement stores, so
  searching cannot find it. (round 69)
- **A bare `nop` in a call's delay slot establishes arity ONLY when the argument is not already in
  the right register** — those two cases are BYTE-IDENTICAL. **MIPS o32 fills argument registers
  strictly left to right**, so untouched `$a1` with `$a2`/`$a3` set PROVES a forwarded parameter;
  the converse fails, so type a slot from its CALL SITES. (a §"3. A bare `nop` in a call's delay
  slot", §"Round 11")
  A parameter `move`d into `$a2`/`$a3` at ENTRY is a forwarded argument of a later call until shown
  otherwise: `Class6D940__ResolveEntry`'s four-round "register identity" was `slot80` missing its fourth argument
  (54/76 -> 76/76, round 76).
- **A discarded return value is never evidence of `void`, and an empty-bodied occupant is never
  evidence the slot takes no arguments** — bytes constrain the return type only where the caller
  USES it; prefer the slot's OCCUPANT. **A declared RETURN TYPE can also block a cross-jump merge
  that should happen**: GCC will not merge a `void` call against a value-returning call whose result
  is discarded, so when N identical calls merge into GROUPS, the grouping PARTITIONS BY DECLARED
  RETURN TYPE — a 2+2 split is a TYPE mismatch no barrier touches. (a §"Confirmed on this game",
  §"Cross-jump shape THREE")
- **Before reshaping a suspected tail-merge stall, LIST the return types of every call in the chain
  — retail's merge set must be type-uniform.** A 30-second check, strictly cheaper than any reshape.
  Why it is decisive: `(set (reg v0) (call ...))` and a bare `(call ...)` are not `rtx_equal_p`, and
  the call insn is where a shared suffix must BEGIN, so a return-type mismatch forecloses the WHOLE
  merge rather than costing one instruction — length comes out long by roughly 3 words per un-merged
  arm. Confirmed in BOTH directions on two slots: `void`→`s32` stopped a merge and cost
  already-matched `Entity__MoodCue00` 4 words; `s32`→`void` restored one and closed `Entity__MoodCue115`'s
  two-round stall at 198/198 with the derived body UNCHANGED. Two chains that appear to miss in
  OPPOSITE directions are the same uniform deficit twice, not two bugs — that misreading is what
  cost the stall its second round. (a round 68)
- **Retyping a shared vtable slot is NOT local — prefer a LOCAL function-pointer variable.** Assign
  both differently-typed slots into one `void (*fn)(...)` and call `fn`: that forces the merge
  without the retype that silently broke a matched function in another unit. Two symmetric slots
  needed OPPOSITE answers, and a broken retype is a RED BUILD. (a §"Confirmed on this game", §"Round
  12")
- **A multi-exit function that stores a literal into a field and returns that same literal right
  after a `jalr` cannot be typed `s32` — try `void`** (18 isolated reproducers). The bytes pin the
  return type in neither direction, and an already-matched signature can be too NARROW on both
  sides. (a §"Round 15")
- **Type the CALLER's parameters so a callee prototype's implicit conversions emit the exact
  per-call narrowing seen in the disassembly.** Read the CALLEE's body for the width it uses: an
  over-narrow `s8` parameter costs a sign-extend at every call site, a `u8` lvalue passed to `s32`
  costs a redundant `andi 0xff`, and an unsigned narrow STACK argument loads as one `lhu` where a
  signed one is `lw`+`sll`+`sra`. (a §"A symbol accessed at TWO WIDTHS")
- **A caller's local prototype that NARROWS a parameter the callee reads as a full word looks like a
  tail-merge or register-identity stall.** Beyond the sign-extend pair, the narrowed argument is not
  CSE'd with the widened value later calls reuse, so the build comes out N words short with one
  fewer callee-saved register. Discriminator: the callee masks the parameter wider than its declared
  type (`0xFF00` on an `s16`). `_SsSetControlChange` went 195 -> 190 -> 200/200 on `s16`->`s32`; pair it
  with the return-type listing. (round 69)
- **The DEFINITION's own parameters narrow too: a parameter that hops `$a0` -> `$a3` -> `$s5`
  with every use re-sign-extending it is an `s16` parameter, not an allocation residue.** Declaring
  `(s16 a0, s16 a1)` instead of `(s32, s32)` plus casts reproduced retail's hop in `Snd_crescendo`,
  then a `u16` callee prototype (the callee reads it with `lhu`) and an `s16` field took a 13-short
  round-25 stall to 240/240. (round 70, bravo)
- **A wrong extern arity and the deliberate dead-argument idiom are told apart at the CALL SITE,
  never in the callee** — the callee says "ignores `$a1`" either way. Does retail emit an
  instruction for the extra argument (`move a1,zero`, `li a0,0xff`)? Then the declaration is
  byte-load-bearing: keep the arity, annotate `/* arity-ok: */`. Delay slot a callee-save spill or a
  bare `nop` with the register already loaded? Then a disagreement with the definition is false. 15
  of 17 were the idiom (round 59; 11 of 18 in round 58). **The fix that touches no call site is an
  unspecified list `()`, not `(void)`.** (a docs/match-reports/GetClass6B5CCMethods.md, round 59)
- **A literal argument "scheduled late" after a call that sets only `$a0` can be a
  FORWARDED-PARAMETER arity defect.** Check the previous call: if its callee reads `$aN` and the
  caller never writes it, declare and pass the caller's own parameter (a file-local fn-pointer view
  if the shared slot is under-declared); a `move $tN,$aK` whose `$tN` later takes a call result is
  the parameter reused as the result (`aK = f(..., aK)`). Closed `Class6E99C__StartFadeToIndex`/
  `StartFadeDefault` after 94k permuter iterations. (PROGRESS round 73)
- **Read every call's argument registers in retail before any other lever; round 75's commonest
  defect was the wrong NUMBER of arguments or a missing return, 17 functions.** A "filler"
  `addiu $v0,...` before `jr $ra` is the return value (eight RCpoly wrappers). An argument FORWARDED
  from the caller's own parameter emits no set-up, so a three-argument call reads as two
  (`BuildMemcardPath`, `slot0x38`); a stray early `$aN` write is the omitted argument (`slot4C`).
  A method typed WIDER than retail passes keeps values live: extra saved registers, a frame
  difference (`BuildLinkQueries`). A `jalr` with no `$a0` set before it takes no argument
  (`SetActiveDataSource`). Fix the slot or prototype once its callers are checked. (round 75)

### 3g. Delay slots, arithmetic, one-instruction residues

- **A call argument's address (`&self->f`) sitting in the delay slot of an EARLIER guard branch is
  a pointer local assigned before the test.** Inline at the call it is one word short
  (`func_8003B4A8`, round 81).
- **A callee-saved register holding a stack struct's address from the prologue on is an explicit
  pointer local** (`MATRIX *ls = &lsBuf;`): `Unk18Obj__DrawNode` 19 -> 150 of 449, round 81.

- **A delay-slot instruction executes on the TAKEN path too, and a delay-slot store is
  UNCONDITIONAL.** Evaluate the value at the TARGET: `li $v0, 0x1` in a slot with `sw $v0, 0x24(s1)`
  stores 1, not the 3 the comparison used, and modelling it as conditional compiles cleanly. (a
  §"Read the delay slot before modelling the branch")
- **A "dead-looking" filler may be a WRITE scoped wrong in your C — the discriminator is whether it
  has a MEMORY EFFECT.** A store can hide an unconditional write and is worth re-scoping (82/82); a
  register-to-register move cannot, so there the scheduling explanation stands. (a §"A delay-slot
  filler that \"looks dead\"")
- **GCC hoists an EXISTING instruction into a load-delay slot; it never invents one**, so a filler
  whose result looks dead is a lead about the source. A `move $sN,$v0` in a `jal`'s OWN delay slot
  reads the value `$v0` held BEFORE that call. (a §"Round 13, second batch", §"Source-shape levers
  found this round")
- **When your diff is ONE redundant or ONE missing `move`, count how many times your SOURCE mentions
  the value** — one too many adds a `move`, one too few removes it, and the surplus lands in a free
  delay slot, which is why "different filler" and "one value too many" are indistinguishable. First:
  do the branch TARGETS agree? A differing target is a differing CFG and always comes from the
  source. (a §"How to read a one-instruction residue")
- **Do not transcribe a LOWERING back into C.** `xori`+`sltiu 1` is `x == K`; `sltu $r,$zero,$r` is
  `x != 0`; a same-amount `sll`/`sra` pair is a narrowing signed cast; a `mult`/`mfhi`/sign-fix
  chain is `/` or `%`; a bare `andi` with no sign-fix is `& (N-1)`; `~x + 1` is not `-x`. Read
  retail's CHOICE OF INSTRUCTIONS as evidence about the source EXPRESSION. (a §"How to read a
  one-instruction residue", §"Confirmed on this game")
- **Write the division; do not hand-derive the magic multiply** — `x / 127` and `x / 63` compile
  exactly to retail's sequences. For an unknown divisor probe `cc1` or brute-force it: one magic
  constant encodes different divisors at different post-`mfhi` shifts, and the `sll`/`subu` chain
  disambiguates. (a §"Write the division", §"Round 12")
- **An unsigned range check needs an EXPLICIT cast to get `sltiu`** — `(u32)(x - LO) < N`; without
  it, `slti`: same word count, wrong opcode. A loop bound's `sltiu` means an unsigned counter. But
  the FOLD's desirability is read off the disassembly per function (two units, opposite answers, one
  round). (a §"Round 13 (2026-09-03)", §"Round 13, final batch")
- **cc1 canonicalizes a commutative op's register operand order independently of source order —
  SETTLED on six instances in three units.** The decisive test is per-function: reverse the operands
  in C and see whether the output changes. It holds for SYMMETRIC addends, NOT where one addend is a
  base pointer and the expression can be RE-ASSOCIATED (51/51), and a local's declared WIDTH can
  also flip `rs`. (a §"Commutative-operand-order canonicalization")
- **Small expression shapes, each backed by a byte-exact match.** `cond ? A : B` and `!cond ? B : A`
  are not byte-equivalent. `d = -50; if (cond) d = 50;` beats a ternary. `(x & 1) ^ 1` and `!(x &
  1)` schedule oppositely. Indexing a ternary with a nonzero constant DISTRIBUTES it into both
  branches. Comparison operand ORDER decides which load comes first. An expression retail evaluates
  TWICE must be WRITTEN twice. `return self->field = N;` is the single-`ori` setter. (a §"Confirmed
  on this game", §"Round 12")
- **Loop shapes.** `while (*p) { p++; }` and `while (*p++) { } p--;` differ — a guard branch jumping
  TO a decrement means post-increment (25 words). A `for` header's multi-variable increment clause
  has a load-bearing ORDER. Let GCC hoist its own invariants (23 words). Reuse ONE counter for a
  guard and its loop test. A loop-carried multiplicand must be RECOMPUTED, not `+=`. A
  `(u8)`/`(u16)` mask on an induction variable used as an ARRAY INDEX controls strength reduction,
  and its width must match the comparison's. (a §"Four narrower confirmations from round 23")
- **Length-gap levers, both directions.** 1 word SHORT with only a missing `move` before the
  epilogue: collapse two return points into one join. 1 word LONG with an extra `move $an,$vN`
  before a load: stop giving a call's return value its own named local. A one-word gap SHIFTS
  everything after it, so a low raw match there is mostly RIPPLE (23/121 -> 117/121). (a
  §"Length-gap levers")
- **Defeat constant canonicalization by routing the constant through an assignment** (`0xC0` became
  `-0x40`); GCC re-associates constant multiplies across a whole expression tree regardless of
  parenthesization, and only a STATEMENT BOUNDARY stops it. Defeat the store-flag collapse of `if
  (cond) return 1; return 0;` with `flag = 1; return flag;`. (a §"Defeat 2.6.3's constant
  canonicalization") The mechanism is the TREE folder, before RTL: it hoists a literal out of a sum
  (`s + (p + 0x14)` -> `(s + p) + 0x14`), so no spelling containing the literal reaches retail's
  grouping. A local `hdr = 0x14` survives it and cse turns it back into an immediate (`Unk18Obj__InitOt`,
  73/73 after 14 groupings and ~89k permuter iterations); splitting `(w + 20) - span` into two
  statements closed `Class866E8__SplitFootprintSlot`. (round 71) The same goes for a multiply and a byte constant: retail's `li; mult` right after storing that constant is the multiply BY THE STORED FIELD (a literal `* 15` strength-reduces to `sll`/`subu`, one word short; round 82, code_33808), and `li 0x80` feeding `s8` stores is `s32 grey = 0x80;` (the literal folds to `li -0x80`; `InitGsSprite`, round 82).
- **A flat table indexed `&T[r*C]` then `[c]` wants a named row-pointer local, `T (*tbl)[C] = ...;
  tbl[r][c]`**: the local puts the table-address load before the index arithmetic; the inline
  cast does not. Closed `CalcDreamColor` 28/35. (round 73)
- **A code label changes what later passes may delete or combine, and every `if (c) return X;`
  leaves one.** jump2 deletes a redundant `move a0,s0` only if its backward scan reaches the entry
  copy without crossing a label; turning early returns into a nested `if` chain let it, which stopped
  `addiu a0,s0,0x16c` being hoisted into a delay slot (`DreamSys__TryStaircaseLink`, read with `-da`).
  combine likewise folds `x - (x>>4)*16` to `andi` only when the multiply is the shifted value's
  FIRST use in the block; use `hi` for something else first and retail's `sll`/`subu` stays
  (`VabStreamObj__PlayTone`; a barrier between them does not stop the fold). (round 75)

### 3h. volatile and memory

- **cc1's scheduler lets a load pass a STRUCT-FIELD store but not a plain `*p` store.** When one
  store must sink below a later global load (often into a `jal` delay slot) and everything else
  matches, store through a pointer to a one-field struct: `q->p = x`, not `*q = x`. The same rule
  in the other direction: read a flags word through a struct field to let loads hoist above global
  stores. Discriminator: the built code has the store first and the load after, with identical words.
  Three closes in round 76 (`StyleFillEffectKind3` 81/81, `StyleFillEffectKind2`, `func_80018464`).

- **`volatile` is the NARROW instrument for the instruction-ORDER class, not the banned construct**
  — it names no register, exactly like the sanctioned bare `__asm__("")`. Where three barrier
  placements across two rounds all regressed, declaring three hardware-shadow fields `volatile`
  matched 14/14 with no body change. **The discriminator for using it is EVIDENCE the location is
  memory-mapped I/O, not whether `volatile` helps**: a documented `I_STAT`/`I_MASK` pair (two
  closes) versus DECLINED on an ordinary global that measured no change — only two units touch
  hardware addresses. (a §"`volatile` is a legitimate, and much NARROWER, tool", §"Round 16")
- **`volatile` on a POINTEE is a scheduling barrier, and cc1 2.6.3 orders volatile accesses only
  against OTHER volatile accesses.** With `D_8006DAD4` (the SPU voice registers, `0x1F801C00`)
  typed `u16 *`, cc1 hoisted an unrelated `volatile u16` global's store/reload pair across six
  stores through that pointer; typing it `volatile u16 *` pinned the pair back, worth 54/131 ->
  98/131 on its own. So when a near-miss looks like "cc1 hoisted something retail left in place",
  ask which memory operand is memory-mapped I/O BEFORE reaching for a barrier or filing a
  scheduling stall. Discriminator: dump cc1's own output and look for a `#.set volatile` marker
  that has migrated past non-volatile stores. (a round 66, bravo)
### 3i. GTE and inline asm

- **A BARE `__asm__("")` CAN change register allocation, which HARD RULE 6's own test calls
  banned.** On game-neutral code, removing it swaps which register holds each of two values, while
  `__asm__("" ::: "memory")` was byte-identical, so the memory clobber is not the risky half. What
  survives is DIRECTEDNESS: a barrier only perturbs — say what a barrier DID, and stop if you are
  adding them one at a time until a register lands where you want it. (a §"A BARE `__asm__(\"\")`
  CAN CHANGE REGISTER ALLOCATION")
- **The permitted `__asm__("")` barrier is inert against every pass that is not the scheduler** —
  measured on cross-jump/tail-merge, loop-preheader emission order, a GCSE hoist at ANY placement,
  and value forwarding; ask which pass produced the residue first. **Three conditions must hold for
  it to be worth trying**: (1) a SUBSTITUTION residue, not an ABSENT instruction (`built=00000000`
  is an automatic no); (2) a genuine ORDERING decision, not a register CHOICE or cross-block CFG
  decision; (3) BETWEEN straight-line statements in one basic block. Round 20: two wins, four
  regressions — test its effect on WORD COUNT. (a §"The permitted `__asm__(\"\")` barrier is inert",
  §"A GCSE / value-availability hoist is immune", §"The `__asm__(\"\")` barrier in round 20")

### 3j. Permuter practice and false leads

- Distilled to the archive (round 76): the scaffold three checks (PARALLEL-RUNS §3.5 is the live
  procedure), check 3's agreement discriminator, and oracle-confirmed-but-unsound candidates.

## 4. Verdict classes and how far to trust them

- **A stall report's MEASUREMENT and its residue CLASS decay at different rates, and the class is
  what goes stale unnoticed.** Both round-59 revisits found the figures correct while the class
  ("delay-slot scheduling" both times) had never been re-questioned and was wrong both times; each
  closed in single-digit builds after 14 and 46 rounds. A revisit's first act is to re-derive the
  CLASS from the disassembly. (a round 59)
- **A register-identity verdict is a claim about the RESIDUE, not about the function, and it DECAYS
  as the rest of the function changes.** All three of round 44's residues on `func_8001E7BC` were
  filed as one class; two were ordinary source-shape differences elsewhere in the body. When a stall
  carries SEVERAL same-class residues, fix the STRUCTURE first and re-measure. (a
  docs/match-reports/func_8001E7BC.md, round 57)
- **"Register identity" is the LEAST reliable verdict class in this corpus** — self-sealing and 26%
  of everything queued. Contaminants found, all ordinary C: a masked byte parameter mistyped `s32`,
  a missing field-offset term, a stale helper signature, a field-copy pair wanting whole-struct
  assignment. Round 19 closed 17 in one round. (a §"A \"register-identity\" verdict is the least
  reliable class", §"Round 19 confirmed the class") Round 75 revisited 31 stalls, most filed as
  register identity, scheduling or delay slots: 30 matched as ordinary source shape (arity, loop
  kind, struct copy, CSE), none by permuter, and the 31st was Sony code. (PROGRESS round 75)
- **It is a HYPOTHESIS about a mechanism; "the registers differ" does not establish it.** Differing
  registers fits both GCC wanting a different ALLOCATION (terminal) and GCC scheduling freely with
  allocation falling out (source-fixable); a sibling filed terminal for three rounds closed 14/14 on
  a type change. Screen with `--debug`: a differing register COUNT is not identity at all. (a §"A
  register-identity verdict is a HYPOTHESIS", §"Round 27: the register COUNT and the addressing-mode
  IMMEDIATE")
- **A lever's NEGATIVE is scoped to the (function, lever, STATE) triple, and so is a POSITIVE.** A
  guard polarity inert in round 19 closed three words in round 33; a fix rejected in rounds 19 and
  20 closed the function in round 49. **If you have changed anything else since a lever was
  rejected, the rejection has expired.** A class several reports agree on may be one error copied,
  so re-derive from asm-differ/objdump before acting on any report's DESCRIPTION. (a §"A lever's
  NEGATIVE is scoped to the state it was tested under")

## 5. Withdrawn or SDK-voided — do not re-add

Each rests on Sony's linked SDK objects (ASPSX-built, so no source shape of ours reached
those bytes) or was retracted on measurement. **Exception: a mechanism confirmed by a
standalone reproducer, or one where our oracle went green on a C body BEFORE the
reclassification — those stand and are kept above.** Method and the surviving precedent
(`func_8003FC70`): §"The SDK-exit census, re-run over this document" (full text of four condensed
entries below: archive, "Distilled out on 2026-09-22").

- §"A `for`-loop keeps a status value register-resident where `goto`/labels folds it away" —
  WITHDRAWN (round 45), both functions `libcd/sys.o`.
- §"A third escape from the `sltiu` boolean-materialization fold" — WITHDRAWN (round 47); an
  untested hypothesis only.
- §"Round 27's HEADLINE", **address-taken parameter** row — 0-for-4 on game code; other rows STAND.
- §"Round 27: two source levers for a residue that looks like register identity" — section-2
  COROLLARY WITHDRAWN; section-3 headline FALSE without its `$a0` caveat.
- §"Round 27: the DCE-eliminated always-true check gets a structural hypothesis" — VOID as a
  game-code class, both instances `libcd/iso9660.o`; the METHOD survives.
- §"New residue classes opened this round" — the retry-loop driver cluster and the
  commutative-operand SLOT order in `addu` are CLASSES WITHDRAWN (round 47).
- §"Round 16" **framed wrappers** — reading heuristic survives; codegen claim has NO
  pinned-pipeline evidence.
- §"Round 16" **`volatile` cast as a codegen lever** — worked example WITHDRAWN (round 44),
  `func_8002C048` is Sony's; principle and MMIO carve-out stand.
- §"Two DISTINCT permuter false-lead patterns" and §"A residue next to a just-fixed defect" —
  libcd instances only, illustrations not measurements.
- §"NEW STALL CLASS: retail recomputes an address our GCC CSEs away" — original example Sony's
  `strcmp`; `Unk18Obj__InitDefaults` is game code, CLOSED (3h).
- **Not a fifth way a score lies** (forgotten-`padNN` pad, drift misattributed to the cursor
  function, jump-table funcdiff window, `.bss`-shifting length-short function): CLAUDE.md covers
  all four by name; do not re-propose.

## 6. In the archive only, deliberately not carried

Single-instance or niche idioms that did not earn a line here. They are still correct; read
them in the archive if a residue matches the heading: §"A block of stores far ahead of a
branch may be entirely UNCONDITIONAL", §"A wholly-unused STACK parameter", §"An oversized
outgoing-arg frame is EVIDENCE OF DEAD CODE", §"Two VLAs, an `$fp` frame, and a rounding
immediate", §"A struct RETURNED BY VALUE reads as a call with its arguments shifted",
§"`&arr[i + j]` and `arr + i + j` are one instruction apart", the
cache-the-scalar-not-the-pointer entry, and four permuter-practice notes superseded by the
three-check scaffold protocol in 3j (libc data points, scheduling-residue reproducer, seed
minimally, search-tail inert forms). Distilled out round 67: §"The arm-order lever has a
cheap COUNTER-indication", §"An explicit alias can force the parameter copy cc1 would
otherwise coalesce away", §"Retail reuses the same counter pseudo-registers across sibling
loops and SWAPS their outer/inner roles". Distilled out round 68: §"Two independent
CFG/scheduling levers, both from `func_8002C278`" (now `Class6D940__ResolveEntry`), §"The `mention a value twice` lever needs a
genuine SECOND, INDEPENDENT USE POINT" (it reconciles the round-19 close with the INERT entry in
3d), §"HImode constant narrowing". Distilled out round 69: §"Two long-standing near-misses closed by
DELETING a named value" (the local-count entries in 3d carry the lever), §"When the residue
is a lone scheduling difference, sweep one statement's PLACEMENT", §"Inherited no-op
statements must be tested in BOTH directions". Distilled out rounds 71-72: §"A narrow signed field may need an `s32` LOCAL", §"A missing `andi 0xff`" (getintr). Distilled out round 73: §"An INCOMPLETE-ARRAY global declaration" (contextual, single
instance), §"A same-size pointer cast in a FUNCTION-SCOPE local", §"A permuter run that plateaus
with NO MOVEMENT AT ALL", §"The frame size bounds how many spilled locals"; the 3j local-count entry
folded into 3d's. Distilled out round 77: §"`volatile` is the WRONG tool for an ADDRESS CSE" (the asm-label alias lever). Distilled out round 81 (process, carried by FINISHING-PLAN track 3 step 3 and `check-nonmatching.sh`): §"A type-scoped field rename is enumerated by the COMPILER", §"A preserved `#if 0` body carries the declarations of the round that WROTE it".
