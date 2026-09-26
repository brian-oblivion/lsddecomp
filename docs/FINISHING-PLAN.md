# Finishing plan

How this project gets from "most functions byte-match" to "readable source
that resembles what the developers wrote", in a form one head agent can run
round after round from a single pasted prompt, without the operator choosing
its work.

Three things, and only three:

| what | where | holds |
| --- | --- | --- |
| the plan | this file | definition of done, the tracks, the model table, the prompts, the rules |
| the measurement | `python3 tools/plan.py` | every count, each track's status, the ranked ready-jobs list |
| the ledger | `config/plan-state.json`, written only through `plan.py` | decisions the tree cannot express: calibration results, units passed, tracks parked |

**This file quotes no counts.** A number written here is right for one round
and wrong for every round after. Run the tool. The mechanics of a round
(worktrees, collision rules, merging, liveness) are `docs/PARALLEL-RUNS.md`;
the per-function matching loop is CLAUDE.md and `docs/MATCHING-GUIDE.md`.
This file does not repeat them.

Plan revision: 28 (2026-09-26, premium session after round 90: renames
replay through `tools/replay.py`; track 8 splits are content-only and park).
Changing the plan is a premium head task (§2); record the change in
`docs/PROGRESS.md` and bump this line.

## 1. What done means

The retail image is already reproduced byte-for-byte, and the Psy-Q SDK is
linked from Sony's objects. Neither "match every last function" nor "decompile
the SDK" is a goal.

**Phase 1 (tracks 1 to 5, done at round 89):** every game function is C;
every SDK callee has Sony's name; stalls are characterised; each class has one
definition in its own header; the docs fit their budgets.

**Phase 2 (tracks 6 to 9; operator, 2026-09-26):** a reader who opens any
file sees what a game's source looks like, not a decompiler's:

6. **Every type says what it is.** No class, struct or typedef is named for
   an address, a unit or an offset (`Class6B5CC`, `Vec3_d294`, `Sub14`);
   Sony's types appear under Sony's names from the SDK headers.
7. **Every function body reads without the disassembly**: named globals,
   locals and fields, no raw offset arithmetic, constants named where their
   meaning is known, comments that explain the code instead of recording
   the project's history.
8. **Every file is named for what it holds** and, where the binary can tell,
   is one original translation unit.
9. **The tree is laid out and formatted like a project**: subsystem
   directories, one code style, a README that maps it.

`plan.py` prints each as a track status; `tools/readability.py` measures the
debt behind 6 to 8.

## 2. Models: who runs on what

| role | default | switch, and to what |
| --- | --- | --- |
| **head** | Opus | **premium** only when the round itself will WRITE a new procedure or tool or change a RULE in a doc, or must adjudicate a HARD RULE tension or a toolchain lead, or `plan.py` lists a setup item (tracks 6 and 7). Escalated gaps are NOT a reason for the next round's head to be premium: the operator hands them to a premium session BETWEEN rounds, which revises this plan and the tools, and the next round runs on Opus again. An Opus head executes what is written and escalates every gap in its report instead of writing procedure (round 50 onward). MAINTAINING a doc within its existing rules (distilling entries to the archive to meet a budget, fixing a stale figure) is not a plan change: the head does it, or spawns a Sonnet for it, when `plan.py` warns. |
| **types runner** (track 6) | Opus | never lower: a class name propagates into every unit that sees it |
| **polish runner** (track 7) | Opus | **Sonnet** once the head has reviewed two Opus-polished units and sent none back (`plan.py set-model --role polish_runner --model sonnet`); back to Opus if a Sonnet unit is sent back |
| **files runner** (track 8) | Opus | never lower |
| **close-out runner** (track 9) | Opus | |
| **mechanical runner** (track 6 table renames, reopened track 2 or 1b, report hygiene) | Sonnet | never higher |

Tracks 1, 3 and 4 have their own rows in the archives (§3) if they reopen.

**Premium is a role, not a model name: head agent (Opus 5.5)** today, the
strongest model the operator has (it replaced Fable 5.1 in revision 14; the
next swap edits this line and `plan.py`'s `PREMIUM`). A session is premium
because the operator started it as one, with the plan-change mandate, not
because of its model string, so it stays distinct from an ordinary Opus head
even on the same model. A premium session never runs as a runner. The head
spawns runners with the Agent tool's `model` parameter. **The head states its
own model and role in its first message; if this table says the round needs a
premium head and the operator did not start it as one, it says so and stops
before spawning anything.** The operator's runner cap in the pasted prompt is
binding; with none stated, use five (PARALLEL-RUNS §4 still says stay at three
when the head takes a substantial task itself).

## 3. Tracks

Tracks are not gates on each other. `plan.py` prints one ready-jobs list
that takes turns across every open track, and the head fills its runner slots
from the top, one job per runner. A job whose edit set (`(units: ...)` on its
line) overlaps a job above it is DEFERRED. Every track has a stop or park rule
so that "hard" becomes "parked with a written reason" rather than another
round.

### Tracks 1 to 5 (phase 1): done

Their rules, stop rules and runner prompts are archived verbatim:
`docs/archive/FINISHING-PLAN-tracks-1-2-2026-09-25.md` (matching, stalls,
SDK names) and `docs/archive/FINISHING-PLAN-tracks-3-5-2026-09-26.md`
(naming pass, class unification, close-out). `plan.py` reopens a track by
itself when the tree regresses (fresh ground, a stray class view, a global
declared with two types); follow the archived section then. Two of their
rules are standing for every track: a function `progress.py` counts as
library is never renamed or retyped as game code, and a preserved near-miss
body lives only in an `#ifdef NON_MATCHING` block (CLAUDE.md).

### Naming rules (every track)

- **Name what the code does, never what you guess it is for.**
  `DreamSys__ResetLinkState` is evidence-based even when nobody knows why;
  `PlayNightmareSound` is a guess when all you see is a sound call.
- **Tiers, recorded in the report:** A, purpose known: evident from the body
  alone, or from two or more callers that agree, and a pure leaf whose
  mechanics ARE its purpose (a getter, a clamp, a list push) is tier A by
  definition; B, mechanics described but purpose in the game not established;
  C, placeholder kept, with what IS known written down. The tier-C form for a
  method whose CLASS is known is `Class__func_xxxxx` (e.g. `Foo__func_12345`;
  examples here are synthetic so `rename.py` can never rewrite them), and bare
  `func_800xxxxx` otherwise. A wrong tier-A name is worse than a
  placeholder. A tier-B name is expected to be sharpened later; renames are
  cheap.
- **Conventions:** methods `Class__Method`, where `Class` is the struct's type
  name; free functions `VerbNoun`; constructors `New_Class` and
  `Class__Class`; method tables `g<Class>Methods`; types `PascalCase`; fields
  `camelCase`; globals `gName`; unit-static data `sName`; constants and enum
  members `UPPER_SNAKE`; vtable slots named like the method they dispatch to;
  files named for the class or subsystem they hold. Do not invent a new style.
- **Every inherited name (FirecatFG's, `CREDITS.md`) is a tier-B hypothesis.**
- **Sony keeps Sony's names**: a function `progress.py` counts as library, a
  variable `psyq-objects.ld` pins or an address inside one, data only Sony
  functions read (`tools/sonydata.py`), and every Sony type (`VECTOR`,
  `GsCOORDINATE2`). `rename.py` refuses the first three.
- **Fields and slots are renamed in the struct DEFINITION only**, and the
  compiler's error list is the accessor set (a textual grep over-counts:
  `unk10` recurs across unrelated structs). `tools/check-nonmatching.sh` lists
  the accessors the default build cannot see. An accessor in a unit outside
  your job: revert, and propose the name in the report (`## Proposed field
  names`) and on the broadcast; the head applies it by type scope at merge.
  A field no code accesses is padding (`u8 padNN[...]`), not a name. Never
  move an offset or a size.

### The class model

`include/BasicClass.h`'s banner is the worked example; `include/Class6D430.h`
the first class with subclasses. One header per class, `include/<Class>.h`,
holds the object struct, the method table struct, the table's extern and
getter, and the class's own method prototypes; a class with subclasses also
defines `<CLASS>_FIELDS(Methods)` and `<CLASS>_SLOTS(Self, CtorParams)`,
which each subclass expands first, so accessors stay flat at any depth.
Table word +0x000 is a nibble-path class id (the parent of 0x1F234 is 0xF234,
0x234, 0x34, 0x4, 0x0), a first guess at the tree that a ctor's first call
corrects (`plan.py classes`, "parent by CTOR CHAIN"). Once a class is unified
no unit declares its own view of it; `plan.py classes` lists a STRAY VIEW.

### Phase 2 rules (tracks 6 to 9)

- **Every commit changes zero bytes.** `./build-and-verify.sh` after every
  step, `python3 tools/typeviews.py --warnings` 0 new (a renamed warning is
  GONE plus NEW: rewrite the baseline with `--baseline` in that commit and say
  so), `tools/check-nonmatching.sh` green.
- **Renames go through a tool and replay.** Symbols: `tools/rename.py`.
  Types and class families: `tools/renametype.py`. Files: `tools/unitfile.py`.
  Each tool run is its own commit whose message's first line is the exact
  command. At merge, take `main`'s side of each hunk that conflicts only by
  a rename (modify/delete: keep the renamed file), then `python3
  tools/replay.py`, which re-applies both sides' commands to what the other
  wrote and three-way merges the ledger, then the oracle. That is why jobs defer only on
  their EDIT sets and a rename touching a hundred units does not serialise
  a round.
- **Comments explain the code** (operator, 2026-09-26). A unit's banner says
  what the file holds; a function comment says what a reader needs and the
  code does not show. Project history (round numbers, who found what, a
  derivation, retail addresses) belongs in the match report: move it there,
  do not delete it. A construct that exists only because it matches keeps
  one line, `/* MATCHING: <what would break> */`, so nobody tidies it away.
- **Style** (after track 7's setup): `clang-format -i` on every file you
  touched, before its commit.
- **Measure, then read.** `python3 tools/readability.py --unit <u> -v` lists
  every hit; the patterns are a floor, never the definition of readable. A
  pattern met by a meaningless rename (`var_s0` to `v0`) has met nothing.

### Track 6: every type says what it is

**Sony's headers (setup `sdk-headers`, done round 91).** `include/psyq/` holds
the SDK headers in LF, lowercase, as Sony's own `#include`s spell them. A unit
takes Sony's types and prototypes from them: `#include "common.h"`, then
`<libgte.h>`, `<libgpu.h>`, `<libgs.h>` in that order, then whichever of
`<libetc.h>`, `<libcd.h>`, `<libsnd.h>`, `<libspu.h>`, `<libpress.h>` it
calls; project headers last. `include/types.h` defines `u_char` to `u_long`
under `<sys/types.h>`'s guards (`u_long` stays `unsigned int`, so a `u32 *`
passes without a cast). Never include `<inline.h>` (ASPSX flavour;
`include/gte.h` replaces it). A file that re-declares a Sony name its own way
(a local `GsIMAGE`, a local `PadInit` prototype) cannot sit beside these;
`python3 tools/sonyheaders.py` lists them, and `plan.py` attaches each to its
job: a header's to track 6, a unit's to its polish pass. The fix is always
Sony's declaration, never a rename of Sony's.

**Jobs**, from `plan.py`, root class first: one per class whose type or table
name is a placeholder, carrying the placeholder types its header defines, and
one per other file that defines placeholder types. The head adds a name the
patterns cannot see (an opaque one such as `ObjM`) with `plan.py flag-type`.

**The pass, per class** (prompt §4.8):
1. Read the header banner, every own method and its report, the subclasses
   and slots (`plan.py classes`, `tools/classtable.py <table> --vs <parent>`),
   and the callers (`grep -rn` over `src/`).
2. Name the class for what its methods make it (a node in a transform
   hierarchy that wraps a GsDOBJ2 and its GsCOORDINATE2, say), per the naming
   rules; the banner states the evidence. A class you cannot name after the
   pass stays, `plan.py park-type --name <Class> --reason "..."`.
3. `python3 tools/renametype.py OLD NEW --dry-run`, read every token it
   lists, then without `--dry-run`. The table is `g<Class>Methods`: a
   `D_`/`ALLCAPS_METHODS` table is renamed with `rename.py`.
4. Member types: one that IS a Sony struct (layout and use agree with the
   SDK header) is deleted and Sony's type used, its fields becoming Sony's
   (the compiler lists the accessors); the rest get names with
   `renametype.py`. The class's own `unk` fields, `slotNN`s and
   `Class__func_` methods are named where their accessors or occupants show
   what they do.
5. Rewrite the banner as documentation (what it is, parent, lifecycle, who
   creates it), history to the reports.

A non-class job does steps 3 to 5 for its types; a type another unit also
needs moves to the header that owns its subject (archived track 4b). **Head
review:** three names against their evidence; one that claims more than its
evidence shows sends the job back. **Park rule:** `park-type` with the reason,
listed by `plan.py`.

### Track 7: every body reads without the disassembly

**Style (setup `format`, done round 91).** `.clang-format` is the house style
(clang-format 22): 4-space indent, attached braces, `*` on the name, a blank
line between definitions, includes never sorted (their order is load-bearing),
comments never reflowed, strings never split. `make format` formats `src/` and
`include/` (never `include/psyq/`). A multi-line `#define` and a K&R
definition sit between `/* clang-format off */` and `/* clang-format on */`
(block form: cpp is C89); write a new one the same way. A tool that reads
`src/` must not depend on layout: the format commit left every tool's output
unchanged, measured, after fixing two that did.

**One pass per unit**, in `plan.py`'s order (most debt first), prompt §4.9:
1. Placeholder globals and functions: `rename.py`, the naming rules.
2. Unit-local structs: name accessed `unk` fields, pad the rest. A raw offset
   access (`*(s32 *)((u8 *)p + 0x18)`) becomes a field of the struct `p`
   really points to; another header's struct follows the field rule.
3. Locals and parameters: m2c names become roles (`i`, `count`, `node`).
   A local's name is not in the object, so this changes zero bytes; changing
   a declaration's TYPE or scope is not a rename, and the oracle decides.
4. Constants, where the meaning is established: Sony's name first (`ONE`,
   libgpu's primitive macros where they expand to the same code); then an
   `enum` for states, kinds, commands and `switch` cases, in the header that
   owns the value; a `#define` for sizes, limits and masks; `ARRAY_COUNT()`
   for element counts. Decimal for counts, sizes, timers and colours; hex for
   masks, flags and addresses. A literal stays when a name would only
   restate it. A helper macro (fixed point, `ARRAY_COUNT`) lives in
   `include/common.h`, added by the first runner who needs it.
5. Comments and banner, per the phase 2 rules.
6. `clang-format -i`, oracle, commit.

Evidence for a name goes in the function's report (`## Naming`); for a
constant, in the comment on its definition. **Head review before
`plan.py mark-unit --unit <u> --track 7`:** five sampled names or constants
against their evidence. There is no second pass; `readability.py` keeps
listing what is left.

### Track 8: every file is named for what it holds

**Regions**: a run of game units between two non-game objects
(`plan.py regions`). A region is ready once its units have had track 7 and
their names are settled (track 6), because a file is named for its content.

**Evidence** (`python3 tools/tuboundary.py`, `--unit <u>` gap by gap). The
rodata decides some edges: cc1 emits each function's strings and jump tables
in function order and the linker lays sections out in object order, so a
rodata crossing proves two units were one file (merge), and two jump tables
of different parity mod 8 prove a boundary between their functions (split;
none exists inside any unit, and a merge never crosses one).
The tool validates this against every edge between placed Sony objects and
says so first; if the validation fails, nothing it prints is evidence. Its
soft signal (single-user data) is a hint with a printed error rate. Where the
binary is silent, content decides: one class's methods, or one subsystem,
make one file; a twenty-function carve slice is never a boundary by itself;
never merge across a forced boundary.

**The pass, per region** (prompt §4.10): merge with
`python3 tools/unitfile.py merge A B` (adjacent units only; the compiler then
lists the duplicate declarations, keep the owning header's); rename with
`unitfile.py rename OLD NEW`, which also moves a same-stem header; name a file
for its class (`SceneNode.c`) or subsystem, and Sony code carried as C for
its Sony object (`libsnd_vmanager.c`). The banner says what the file holds and
what decided its edges. A files runner may edit its region's yaml lines,
through `unitfile.py` only (the one exception to PARALLEL-RUNS §2's yaml
rule). **Head at merge:** `make extract`, then delete `build/src` and rebuild,
because unit names changed. **Park rule:** a region the evidence and content
cannot settle, or that content alone would split (no tool splits a unit),
keeps its carve edges, content-named, with the reason in each banner.

### Track 9: close-out

Opens when tracks 6 to 8 are done. Items, ticked with `plan.py check --item`:
`layout` (subsystem directories under `src/`, chosen from what the files hold:
`unitfile.py rename <unit> <dir>/<unit>`; README's code map says what lives
where), `readme`, `comments` (readability history 0, headers too), `style`
(`make format` leaves no diff), `docs-budget`, `nonmatching-clean`. An item
already true when measured is ticked by the head with the measurement in
PROGRESS.md; the rest are Opus runner jobs, prompt §4.7, one item each.

## 4. Prompts

### 4.1 Head prompt (the operator pastes this, unchanged, every round)

> Act as the head agent for the LSD: Dream Emulator decomp finishing plan.
> Say which model you are running as. Read `docs/FINISHING-PLAN.md`, then
> `docs/PARALLEL-RUNS.md`, then CLAUDE.md. Run Gate 0 (PARALLEL-RUNS §3.2) and
> `python3 tools/plan.py`. If the top jobs need a model this table says you are
> not, stop and tell me. Otherwise fill at most `<K, or 5 if I did not say>`
> runner slots from the top of the ready-jobs list, one unit per runner,
> checking header contention, and spawn each with its track's prompt from
> FINISHING-PLAN §4 (matching: PARALLEL-RUNS §5) on the model the list names.
> Triage, merge and verify per PARALLEL-RUNS §3.6 to §3.9. At the end: record
> the round (`plan.py record-round`, `mark-unit`, `set-model`, `set-track` as
> applies), write the PROGRESS.md entry, update any doc line the round
> invalidated, commit, push, tear down. Report: what moved on each track, what
> `plan.py` says next, and anything that needs my decision (toolchain leads,
> a HARD RULE tension, a plan change). Never act on those yourself, except
> that a head I started as premium (§2) makes a plan change as a recorded
> revision; toolchain leads and HARD RULE tensions come to me either way.

### 4.2 to 4.4, 4.6

Archived with their tracks (§3).

### 4.5 Extern review prompt (track 3; Opus)

> You are a review runner for the LSD: Dream Emulator decomp, round `<N>`,
> worktree `<path>`, branch `runner/<name>`. Read CLAUDE.md and
> `docs/FINISHING-PLAN.md` §3's naming rules. Run `python3 tools/externcheck.py`. For
> each function it lists: read the CALLEE's disassembly (or its matched C) and
> decide which registers it actually reads. If an `extern` declares fewer or
> more parameters than the callee reads, fix that `extern` line only, never a
> call site's arguments. If the callee reads a register the caller leaves
> loaded from its own arguments (the forwarding idiom), the CALLER's
> reconstruction is the incomplete side: give it the forwarded parameter if
> that is byte-identical, otherwise annotate the extern line with
> `/* arity-ok: <why> */`. `./build-and-verify.sh` green after every edit;
> `python3 tools/externcheck.py` clean when you finish. One commit per
> function. This touches many units: it runs ALONE in its round or the head
> merges it last. Report a table: function, what was wrong, what you did.

### 4.7 Close-out runner prompt (track 9; Opus; head fills in `<>`)

> You are a close-out runner for the LSD: Dream Emulator decomp, round `<N>`,
> worktree `<path>`, branch `runner/<name>`. Read CLAUDE.md and
> `docs/FINISHING-PLAN.md` §1 and §3 track 9. Your item is `<item>`: make its
> "done when" true, editing only `<files>`. State nothing you did not measure:
> every figure comes from a command you ran (`progress.py`, `plan.py`,
> `readability.py`), and a README states no counts at all, only the command
> that prints them. Name what the code does, as §3's naming rules say.
> `./build-and-verify.sh` green after every edit to `src/` or `include/`.
> Commit per logical step; `git status --porcelain` empty when you report;
> never push; never edit other docs, the ledger or the symbols file except
> through the rename tools. Final summary: what you changed, what you
> measured, and anything you could not settle.

### 4.8 Types runner prompt (track 6; Opus; head fills in `<>`)

> You are a types runner for the LSD: Dream Emulator (PSX) decomp, round
> `<N>`. Work ONLY in worktree `<path>` on branch `runner/<name>`. Your job is
> `plan.py`'s line: `<job line>`. Read CLAUDE.md, then `docs/FINISHING-PLAN.md`
> §3 (naming rules, the class model, phase 2 rules, track 6), then do track
> 6's pass for your job. You edit the header(s) your job names, the class's
> own units' banners and the match reports; every other file changes only
> through `tools/rename.py`, `tools/renametype.py` (always `--dry-run` first)
> or the compiler's accessor list. Other runners this round: `<jobs>`; post on
> the broadcast before editing a header outside your job. After every edit:
>
> ```sh
> ./build-and-verify.sh > /tmp/<name>_b.log 2>&1; echo "build exit=$?"; \
> grep -nE 'error:|parse error|undefined reference|has no member|conflicting|\*\*\* \[[^]]*\.o\]' /tmp/<name>_b.log | head -8
> python3 tools/typeviews.py --warnings | tail -3
> tools/check-nonmatching.sh
> ```
>
> `OK: build matches retail`, 0 new warnings and nonmatching green, or undo
> the last edit. One commit per tool run, its message's first line the exact
> command (`git commit -F <file>`: backticks in a double-quoted `-m` run as
> commands); one commit per header edit. `git status --porcelain` empty when
> you report; never push; never edit shared docs or the ledger.
>
> Final summary: every name you gave (old, new, tier, one-line evidence);
> every Sony type you substituted and the accessors it renamed; the banner you
> wrote; every field name you PROPOSED instead of applying; anything you
> parked and why.

### 4.9 Polish runner prompt (track 7; head fills in `<>`)

> You are a polish runner for the LSD: Dream Emulator (PSX) decomp, round
> `<N>`. Work ONLY in worktree `<path>` on branch `runner/<name>`, on
> `src/<unit>.c`, the unit-local declarations in it, and the match reports of
> its functions. Read CLAUDE.md, then `docs/FINISHING-PLAN.md` §3 (naming
> rules, phase 2 rules, track 7). Run
> `python3 tools/readability.py --unit <unit> -v`, then do track 7's pass in
> order, for every function in the unit including `NON_MATCHING` bodies.
> Class and type names are track 6's jobs: leave them. Renames only through
> `tools/rename.py` / `tools/renametype.py`; fields and
> slots by the naming rules' field rule (propose what another unit accesses);
> constants into the header that owns them only if your unit is the only
> includer that changes, else propose. After every step, the three oracles
> (build with the grep, `typeviews.py --warnings`, `check-nonmatching.sh`),
> `clang-format -i` on the files you touched, then commit: one per tool run
> (the command as the message's first line), one per step otherwise. Read the
> broadcast before each function. `git status --porcelain` empty when you
> report; never push; never edit shared docs, the yaml or the ledger.
>
> Final summary: `readability.py --unit <unit>` before and after; a table of
> every name and constant you introduced (old, new, tier, evidence); the unit
> banner you wrote; what you moved from comments into which reports; every
> proposal for the head; what you left and why.

### 4.10 Files runner prompt (track 8; Opus; head fills in `<>`)

> You are a files runner for the LSD: Dream Emulator (PSX) decomp, round
> `<N>`, worktree `<path>`, branch `runner/<name>`. Your job is `plan.py`'s
> line: `<job line>`. Read CLAUDE.md, then `docs/FINISHING-PLAN.md` §3 (phase
> 2 rules, track 8). For each unit run `python3 tools/tuboundary.py --unit
> <unit>` and read its banner and functions; decide each region's files from
> the evidence first and content second, and write the decision down before
> you act. Then `python3 tools/unitfile.py merge|rename ... --dry-run`, and
> for real; resolve what the compiler lists after a merge by keeping the
> owning header's declaration. You edit only your units, their same-stem
> headers and their yaml lines through `unitfile.py`. The three oracles after
> every step; one commit per tool run, the command as its message's first
> line. `git status --porcelain` empty when you report; never push.
>
> Final summary: per region, the files before and after, the evidence that
> decided each edge (quote `tuboundary.py`), what content decided, and
> anything parked.

## 5. Doc hygiene

The southpark sister project's plan, prompts and scripts grew past the point
where an operator could paste one thing and walk away. The rules that keep
this repo from following:

- **One append-only narrative: `docs/PROGRESS.md`.** Every other doc states
  current rules. A rule carries its reason in one sentence and a pointer to
  the PROGRESS round or archive section that holds the story.
- **No counts in docs.** Measure with `progress.py` and `plan.py`.
- **Word budgets** (words, not lines: round 64 met a line budget by
  reflowing to a wider column with the text unchanged), enforced by `plan.py`
  as warnings and by track 5 as a checklist item. Over budget means distil,
  i.e. move whole entries to
  `docs/archive/`. Promoting a round's idioms is never suppressed to protect
  a budget; the head promotes, then distils (or spawns a Sonnet to) in the
  same consolidation when the warning shows. Distillation is maintenance,
  not a plan change.
- **A new idiom is at most 12 lines in DECOMPILATION_LEARNINGS.md**, with
  its discriminator and its evidence in one clause each.
- **One head prompt.** New kinds of work get a track section and a runner
  prompt HERE, not a new doc and a new paste.
- **The ledger holds decisions, never counts.** If you find yourself writing
  a number into `plan-state.json`, it belongs in a tool.

## 6. When to reconsider the plan

Reconsider, as a premium head task recorded in PROGRESS.md with the revision
line above bumped, when any of these happens:

- a stop rule fires and the operator wants the track reopened anyway;
- a track's yield is off by more than a factor of two from what its section
  assumes (say so with the measurement);
- a whole new class of work appears that no track covers;
- `plan.py`'s ordering keeps putting a job at the top that the head keeps
  skipping for a good reason. The fix is in the tool, not in ignoring it.

Do not reconsider because a round went badly. One round is a measurement.
