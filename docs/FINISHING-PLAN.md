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

Plan revision: 44 (2026-09-28, round 105's premium head: track 11's
file renames are `tools/unitfile.py rename|header|check`; revision 43 the
declaration census, 42 phase 4, 41 phase 3).
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

**Phase 3 (tracks 10 to 13; operator, 2026-09-28):** the tree is ready to
publish: one declaration per name and one convention (10), snake_case file
names (11), every header documented as API with no process text left in
any comment (12), and a README and lint a stranger can use (13).

**Phase 4 (track 14; operator, 2026-09-28):** the same C builds for Linux,
so a PC port can be written on top of it. The port itself, with its platform
layer, renderer, audio and packaging, is a separate repository that pins this
one, as sm64's and oot's ports are. This repo only keeps its C portable.

`plan.py` prints each as a track status; `tools/readability.py` measures the
debt behind 6 to 8.

## 2. Models: who runs on what

| role | default | switch, and to what |
| --- | --- | --- |
| **head** | Opus | **premium** only when the round itself will WRITE a new procedure or tool or change a RULE in a doc, or must adjudicate a HARD RULE tension or a toolchain lead, or `plan.py` lists a setup item (a premium item in a track's checklist). Escalated gaps are NOT a reason for the next round's head to be premium: the operator hands them to a premium session BETWEEN rounds, which revises this plan and the tools, and the next round runs on Opus again. An Opus head executes what is written and escalates every gap in its report instead of writing procedure (round 50 onward). MAINTAINING a doc within its existing rules (distilling entries to the archive to meet a budget, fixing a stale figure) is not a plan change: the head does it, or spawns a Sonnet for it, when `plan.py` warns. |
| **item runner** (tracks 9 to 13) | Opus | Sonnet only where `plan.py` prints it for an item (a mechanical list); naming and documentation stay Opus, as polish did (Sonnet accepted 1 of 5, revision 33) |
| **mechanical runner** (track 6 table renames, reopened track 2 or 1b, report hygiene) | Sonnet | never higher |

Tracks 1, 3, 4 and 6 to 8 have their own rows in the archives (§3) if they reopen.

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
declared with two types); follow the archived section then, beside phase
2, which stays open once track 5's checklist is ticked. Two of their
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
  name, receiver `self`; free functions `VerbNoun`; constructors `New_Class`
  and `Class__Class`; method tables `g<Class>Methods`, getters
  `Get<Class>Methods`; types `PascalCase`; fields
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

`include/basic_class.h`'s banner is the worked example; `include/file_resource.h`
the first class with subclasses. One header per class (its own, named in
snake_case by track 11; `plan.class_header` finds it) holds the object struct, the method table struct, the table's extern and
getter, and the class's own method prototypes; a class with subclasses also
defines `<CLASS>_FIELDS(Methods)` and `<CLASS>_SLOTS(Self, CtorParams)`,
which each subclass expands first, so accessors stay flat at any depth.
Table word +0x000 is a nibble-path class id (the parent of 0x1F234 is 0xF234,
0x234, 0x34, 0x4, 0x0), a first guess at the tree that a ctor's first call
corrects (`plan.py classes`, "parent by CTOR CHAIN"). Once a class is unified
no unit declares its own view of it; `plan.py classes` lists a STRAY VIEW.

### Phase 2 and 3 rules (tracks 6 to 13)

- **Every commit changes zero bytes.** `./build-and-verify.sh` after every
  step, `python3 tools/typeviews.py --warnings` 0 new (a renamed warning is
  GONE plus NEW: rewrite the baseline with `--baseline` in that commit and say
  so), `tools/check-nonmatching.sh` green.
- **Renames go through a tool and replay.** Symbols: `tools/rename.py`.
  Types and class families: `tools/renametype.py`. Files: `tools/unitfile.py`.
  Each tool run is its own commit whose message's first line is the exact
  command. At merge, take `main`'s side of each hunk that conflicts only by
  a rename (modify/delete: keep the renamed file) and the owning runner's
  side of a hunk in its own unit that also changes content, then `python3
  tools/replay.py`, which re-applies both sides' commands to what the other
  wrote and three-way merges the ledger, then the oracle. That is why jobs defer only on
  their EDIT sets and a rename touching a hundred units does not serialise
  a round.
- **Comments explain the code** (operator, 2026-09-26). A unit's banner says
  what the file holds; a function comment says what a reader needs and the
  code does not show. Project history (round numbers, who found what, a
  derivation, retail addresses) belongs in the match report: move it there,
  do not delete it, under a heading naming history. The rename tools leave
  those sections, and a line already naming the new name, as written, and
  keep every other name current: never restore one by hand. A construct that exists only because it matches keeps
  one line, `/* MATCHING: <what would break> */`, so nobody tidies it away.
- **Style** (after track 7's setup): `clang-format -i` on every file you
  touched, before its commit.
- **Measure, then read.** `python3 tools/readability.py --unit <u> -v` lists
  every hit; the patterns are a floor, never the definition of readable. A
  pattern met by a meaningless rename (`var_s0` to `v0`) has met nothing.

### Tracks 6 to 9 (phase 2): done

Their procedures and runner prompts are archived verbatim in
`docs/archive/FINISHING-PLAN-tracks-6-9-2026-09-28.md`; the phase 2 rules
above stay live. `plan.py` reopens a track when the tree regresses; follow
the archived section then.

### Phase 3 (tracks 10 to 13): the tree is ready to publish

Operator, 2026-09-28, after a six-area review of the finished tree. Each
track is a checklist of items, like track 9: an item is one runner job
(prompt §4.7), its "done when" is printed by `plan.py`, and the head ticks
it with `plan.py check --item <item>` and the measurement. A track opens when
the one before it is done; setup items (premium) come first in their track.
The review's findings, by file and line and grouped by item, are
`docs/research/release-review-2026-09-28.md`: a runner starts from its
item's section there, and verifies each finding before acting on it.

**Track 10: one declaration, one convention.** A function or global is
declared once, in the header of the file that defines it (Sony's name from
Sony's header), with the definition's types; no unit re-declares it
(`tools/declcheck.py`; a view kept for its bytes carries a `MATCHING:` line). The
conventions of §3 hold everywhere: `Get<Class>Methods` (not `Get_vtable_`),
`self` (not `this`), guards `<NAME>_H`, `s` data never exported from a
header, no tab outside the `.inc` files. Names the review found misleading
are renamed. Sony code carried as
C takes Sony's names and headers, and its preserved bodies use the
`NON_MATCHING` form. Then the debt left after track 7 gets a second pass per
area: `unk`, `slot`, raw offsets, m2c locals and placeholders are named where
their accessors show what they are, and each area's duplicate types are
merged into one by hand (a merge, not a rename). A literal is named only when it means
more than its value (a Sony constant, a state, a bit, a size two places
share); a cue's frame numbers and timings stay literal, so `magic` is not a
target.

**Track 11: file names.** Every game file in `src/` and `include/` is
snake_case (`scene_node.c`, `scene_node.h`) and named for what it holds; a
file holding two classes is named for its subsystem, never a concatenation
(`TextEntryItemList`). Types keep PascalCase. `tools/unitfile.py rename <Unit>
<dir>/<name>` moves a unit and its same-stem header (by paths only when the
stem is a type), `unitfile.py header <Old> <name>` a header no unit owns, and
`unitfile.py check` lists what is left; each run is its own commit. The tools
list the rule docs' mentions instead of rewriting them: the head updates
those at merge.

**Track 12: documented API** (after every name has settled). Every game
header is API documentation in Doxygen form: a `/** @file */` saying what it
declares, a short `/** */` block on each class or struct (what it is, its
parent, where its methods live, its lifecycle), and on every prototype a
`/** @brief */` with `@param` per parameter and `@return` when not void;
fields and slots take trailing `/**< */`. A method table's slots point at
the method (`/**< @see Class__Method */`) instead of describing it twice.
Nothing in a header is process text (retail, registers, cc1, rounds, tools,
reports): what justifies a C spelling moves to the `.c` as one `MATCHING:`
line, and any derivation to the report. `.c` comments follow the same rule;
long banners split into the header's class doc and the functions' docs. The
setup item writes `tools/apidoc.py` (per header: undocumented prototypes,
missing `@param`, process-text hits) and a `Doxyfile`.

**Track 13: publish.** README (what the game's code is, the layout, how to
build and verify, how to change code and keep it matching); a lint that
needs no disc (`make format`, `apidoc.py`, `readability.py`) runnable as CI;
the licence and the fate of the process docs (`docs/`, CLAUDE.md, one-off
tools) are the operator's decisions, recorded in PROGRESS.md.

**Track 14: PC port groundwork** (phase 4). The premium `port-design` item
writes `docs/research/pc-port-design.md`. It weighs psyz (the Psy-Q
reimplementation sotn-decomp's in-repo PC build links) against a platform
layer of our own, covering the renderer, audio, disc access from the user's
image, pointer width (32-bit first or 64-bit clean), the build and licence
compatibility. It says what stays in this repo and what goes to the port
repo, and the operator approves it (`port-approach`). Then `make pc-check`
compiles every game `.c` with the host compiler under `-DPLATFORM_PC`,
listing the unresolved Sony symbols that are the port's surface, while the
matching build stays byte-identical. After that, no pointer is held in an
integer type, and layout assumptions carry static asserts.

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
>
> **Your shell's working directory resets between commands:** start EVERY
> command with `cd <path> &&`, and trust the oracle's `OK:` line only if it
> ends `(tree: <path>)` (round 97: a runner built the main checkout and
> reported four commits byte-exact that never compiled).

### 4.7 Item runner prompt (tracks 9 to 13; head fills in `<>`)

> You are an item runner for the LSD: Dream Emulator decomp, round `<N>`,
> worktree `<path>`, branch `runner/<name>`. Read CLAUDE.md and
> `docs/FINISHING-PLAN.md` §1 and §3 track `<T>`. Your item is `<item>`: make its
> "done when" true, editing only `<files>`. State nothing you did not measure:
> every figure comes from a command you ran (`progress.py`, `plan.py`,
> `readability.py`), and a README states no counts at all, only the command
> that prints them. Name what the code does, as §3's naming rules say.
> `./build-and-verify.sh` green after every edit to `src/` or `include/`.
> Commit per logical step; `git status --porcelain` empty when you report;
> never push; never edit other docs, the ledger or the symbols file except
> through the rename tools. Final summary: what you changed, what you
> measured, and anything you could not settle.
>
> **Your shell's working directory resets between commands:** start EVERY
> command with `cd <path> &&`, and trust the oracle's `OK:` line only if it
> ends `(tree: <path>)` (round 97: a runner built the main checkout and
> reported four commits byte-exact that never compiled).

### 4.8 to 4.10

Archived with tracks 6 to 8 (§3).

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
