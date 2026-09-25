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

Plan revision: 20 (2026-09-25, after rounds 82 and 83: the class id tree is
corrected by the ctor chain, track 4 runs classes strictly in sequence rather
than one per round, a behaviour name for a class is allowed with evidence,
`--warnings` at every merge).
Changing the plan is a premium head task (§2); record the change in
`docs/PROGRESS.md` and bump this line.

## 1. What done means

The retail image is already reproduced byte-for-byte, and the Psy-Q SDK is
linked from Sony's objects. Neither "match every last function" nor "decompile
the SDK" is a goal. Done is all five of:

1. **Every matched game function, struct field, vtable slot and global has a
   name that says what the code does**, with the evidence for each name in
   its match report. Placeholder names remain only where the report says why.
2. **Every SDK function game code calls has Sony's name**, so a call site
   reads `CdRead(...)`, not `func_8002A3C0(...)`. The rest of the SDK stays as
   disassembly and is not counted.
3. **Every stall is characterised, not fought forever**: matching effort on
   the stalled corpus is bounded by a measured stop rule, and each stall
   carries either a readable `NON_MATCHING` body or a written reason it has
   none.
4. **Types are unified**: one definition per class in a shared header, and
   unit-local struct views merged into it.
5. **The docs fit their budgets** and a newcomer can read README, build, and
   find their way from a function to its class and its callers.

`plan.py` prints each of these as a track status.

## 2. Models: who runs on what

| role | default | switch, and to what |
| --- | --- | --- |
| **head** | Opus | **premium** only when the round itself will WRITE a new procedure or tool or change a RULE in a doc, or must adjudicate a HARD RULE tension or a toolchain lead. Escalated gaps are NOT a reason for the next round's head to be premium: the operator hands them to a premium session BETWEEN rounds, which revises this plan and the tools, and the next round runs on Opus again. An Opus head executes what is written and escalates every gap in its report instead of writing procedure (round 50 onward). MAINTAINING a doc within its existing rules (distilling entries to the archive to meet a budget, fixing a stale figure) is not a plan change: the head does it, or spawns a Sonnet for it, when `plan.py` warns. |
| **matching runner** (tracks 1, revisit) | decided by calibration: `plan.py` says which | round A Sonnet, round B Opus, on comparably ranked stalls; thereafter whichever produced more matches per runner-session. The head may override with `plan.py set-model`, with a reason in PROGRESS.md. |
| **naming runner** (track 3) | Opus | **Sonnet** once the head has reviewed two Opus-named units and found no wrong tier-A name. Back to Opus if a Sonnet unit fails review. |
| **mechanical runner** (track 2 identification, track 1b promotion, report hygiene) | Sonnet | never higher |
| **types runner** (track 4) | Opus | a premium head did the first classes (round 80: BasicClass, Pad, Class6D430) and does 4b's first global |

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
that takes turns across every open track (fresh matches, naming, a stall
runner, the SDK batch, revisits, promotions, types, close-out), and the head
fills its runner slots from the top, one unit per runner. A round with two
slots therefore always carries one naming pass and one stall runner; a
priority-sorted list starved every track but naming for two rounds. A job a
renaming job above it would rewrite (call-graph contention) is listed as
DEFERRED instead, with the reason; staff it once that job merges. Every
track has a stop or park rule so that "hard" becomes "parked with a written
reason" rather than another round.

### Tracks 1, 1b and 2

Tracks 1 and 2 reopened in revision 18: the "every game function matches"
track 1 had closed on was measured over segment names, and ten `psyq_*`
segments held game code, now `code_<fileoff>` units. Their `jal`s reached
Sony functions still named `func_`, so track 2 listed them at once; round 81
named them and track 2 is done again (the §4.4 prompt's "§3 track 2" is the
archive's section). Round 82 matched the last of that ground with no stall,
so tracks 1 and 1b are done again; `plan.py` reopens them by itself if a
carve or a reclassification turns up game code.

Fresh ground is never parked by the stall stop rule. `plan.py` lists one job
per unit, up to ten functions cheapest first; staff it with the matching
prompt (PARALLEL-RUNS §5) with K set to the job's list. A unit with fresh
ground waits for track 1 before its naming pass and is left out of track 3's
80% gate. Track 1b reopens by itself if stalls appear. The three tracks'
rules, stop rules and runner prompts are
`docs/archive/FINISHING-PLAN-tracks-1-2-2026-09-25.md`, verbatim. Two of them
are restated where they also apply: a function `progress.py` counts as
library is never renamed or retyped as game code (track 3), and a preserved
near-miss body lives only in an `#ifdef NON_MATCHING` block (CLAUDE.md). A
third is sharpened (revision 19; round 79 read it backwards): POSITION
evidence is the placed objects on either SIDE of the function; the candidate's
own object need not be placed, since an unplaced build is why it is a
candidate at all.

### Track 3: readability, one unit at a time

**Goal.** A reader can tell what each function, field, slot and global in the
unit does without opening the disassembly.

**Eligibility.** Every unit is eligible now; remaining `INCLUDE_ASM`s are
documented stalls. **Order:** `plan.py`'s centrality ranking (units whose
definitions other units reference most), because a name in a base class
propagates everywhere. **One pass per unit, no second pass** (revision 19):
the `func_`, `unk` and `slotNN` a marked unit still holds are named class by
class in track 4 (step 6 names slots and their occupants), and what track 4
leaves stays tier C with what is known written down.

**The pass, per unit.**
1. Read every function in the unit, its callers (`grep -rn` over `src/`),
   its class table (`tools/classtable.py`), and the unit's existing header
   comment and reports.
2. Name functions with `tools/rename.py` (one command: symbols file, sources,
   report file, extract, verify). Never by hand.
3. Name struct fields and vtable slots. (Functions and globals are NOT
   subject to this rule: `rename.py` renames a symbol tree-wide whoever calls
   it, because a symbol name is unique. Only FIELDS and SLOTS share names
   across structs.) Ownership is decided per FIELD by the COMPILER, never by
   grep: `slotNN` and `unkNN` names recur across unrelated structs, so a
   textual search over-counts (round 57: seven textual hits, one real
   accessor). Rename the field in the struct DEFINITION only and rebuild; the
   error list is the exact accessor set AMONG CODE THE DEFAULT BUILD
   COMPILES. `#ifdef NON_MATCHING` bodies are outside it by construction, so
   `tools/check-nonmatching.sh` is the second half of the list and is required
   after every field or slot rename (round 67 shipped a regression there).
   - every error, in both, is in your unit: fix them, both oracles, done;
   - any error is in another unit: revert the definition and do NOT rename. Put the proposed name, its
     tier and its evidence under `## Proposed field names` in the report of
     the function that established it, and post it to the broadcast. The HEAD
     applies it at merge time by TYPE SCOPE, never by whole-tree replace:
     rename the field in the struct DEFINITION only, rebuild, and the
     compiler lists every accessor of that struct as an error; fix exactly
     those, rebuild, oracle. Same-named fields in other structs are untouched
     because their definitions did not change. (Revision 2 claimed a
     whole-tree replace was safe because a mis-hit fails to compile; round 52
     measured `unk10` in 347 places across 50 files, where replacing
     definitions and uses together compiles clean and mislabels ~49 structs.)
   Never move an offset or a size in a shared header (CLAUDE.md, the
   shared-struct hazard). A return-type or parameter-type CORRECTION to a slot
   or prototype is allowed when the oracle stays green and the report lists
   every caller you checked (CLAUDE.md: a tail-call wrapper's byte match says
   nothing about its return type, so the callers are the evidence).
4. Name globals with `tools/rename.py`. Replace magic constants with named
   constants or enums where the meaning is established.
5. Write the unit's header comment: what the unit IS (which class or
   subsystem), in five to fifteen lines. Delete stale blocker banners.
6. Do the same to any `NON_MATCHING` bodies in the unit.
7. In each function's report add `## Naming`: the name, its tier, and the
   evidence (call sites, data touched, strings, table slot).
8. `./build-and-verify.sh` green after every step. A rename changes zero
   bytes; red means the rename is wrong.

**Naming rules.**
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
  `func_800xxxxx` otherwise;
  `plan.py` counts both as unnamed. A wrong tier-A name is worse than a
  placeholder. A tier-B name is expected to be sharpened later; renames are
  cheap now.
- **Conventions, from the code as it stands:** methods `Class__Method`
  (`DreamSys__AdvanceDay`), where `Class` is the struct's type name;
  free functions `VerbNoun` (`CalcDreamColor`); constructors `New_Class` and
  `Class__Class`; types `PascalCase`; fields `camelCase`; globals `gName`;
  unit-static data `sName`; constants `UPPER_SNAKE`; vtable slots named like
  the method they dispatch to. Do not invent a new style.
- **Every inherited name (FirecatFG's, `CREDITS.md`) is a tier-B hypothesis.**
  Confirm it with evidence or rename it; either way, record it.
- **Do not rename a Sony symbol, and give no game name to anything Sony
  owns**: a function `progress.py` counts as library, a variable
  `psyq-objects.ld` pins or an address inside one (`rename.py` refuses both),
  or a field of a struct only Sony functions read (round 75: 24 libsnd
  variables). All are track 2's.

**Head at merge.** Apply the runner's proposed cross-unit field names one at
a time by type scope (definition first, compiler lists the accessors, fix
those, oracle), then `make extract` (the symbols file changed) and the
oracle.

**Head review before `mark-unit`.** Sample five names per unit against their
evidence. A name the head has to change because it claims more than its
evidence shows, at ANY tier (round 77's "UnlessOverridden", "Wobble"), or a
class prefix `classtable.py` contradicts (round 73), sends the unit back.
Convention fixes (`g`/`s`, spelling) do not, and neither does a Sony
function no tool flagged: that gap is the tool's. Two clean Opus units in a row and
the naming runner becomes Sonnet (`plan.py set-model --role naming_runner
--model sonnet`); one Sonnet unit sent back and it goes back to Opus. Then:

```sh
python3 tools/plan.py mark-unit --unit <unit>
```

**Runner** prompt in §4.2. **Park rule:** a unit whose class cannot be
identified after a full pass is marked with what was learned and a
`NAMING PARKED: <reason>` line in its header comment; the head still marks it
so the plan moves on, and track 4 picks it up with the class.

### Track 4: types and data

**Opens** when 80% of units have passed track 3 (`plan.py` says).

**Goal.** 4a: one definition per class, in a header named for it, with every
unit-local view of the class merged into it. 4b: one type per global. Then
state values as enums, once their meanings are named.

**Measure, never grep.** `python3 tools/plan.py classes` prints every class in
tree order with its state (UNIFIED, PARKED, ready, waiting, `no C`), how many
of its own methods are C, and where its views live. `tools/typeviews.py`
compiles every unit through the pinned cpp and cc1 with `-gstabs` and reads
the layouts cc1 itself computed: `--census` (per class: own methods, object
views, table views, with files), `--merge V...` (the union of named layouts,
`CONFLICT` where two disagree in size, signedness or a slot's return type),
`--tree`, `--globals`, `--warnings` (every unit's `-Wall` output against
`config/typeviews-warnings.txt`: a slot's PARAMETER types are not in stabs,
and a disagreeing call site is a new warning), `--upcast`.

**The class model.** `include/BasicClass.h`'s banner is the worked example;
`include/Class6D430.h` is the first class with subclasses. One header per
class at `include/<Class>.h` (top level: the Makefile's header list and
`rename.py` read `include/*.h` only) holding the object struct, the method
table struct, the table's extern and getter, and the class's own method
prototypes. A class WITH subclasses also defines `<CLASS>_FIELDS(Methods)` and
`<CLASS>_SLOTS(Self, CtorParams)`; each class expands its parent's macro
first, so accessors stay flat at any depth and nothing is re-pathed (GCC
2.6.3 has no anonymous struct members, and embedding the base would turn
Entity's accessors into `this->base.base.base.x`). The table word +0x000 is a
nibble-path class id (the parent of 0x1F234 is 0xF234, 0x234, 0x34, 0x4, 0x0),
a first guess at the tree: a ctor calls its parent's ctor first, and
`typeviews.py` moves a class whose ctor makes the SAME first call as its id
parent's up beside it (round 83: Tod and ModelData, id children of
TimBlockSrc, chain to the active data source as TimBlockSrc does, so they are
its siblings; `plan.py classes` marks such a class "parent by CTOR CHAIN").
Classes are unified root first: `plan.py`
offers a class only when its ancestors are unified, parked, `no C` or `no C
yet`. A `no C` class (no own method C, no view anywhere) is not a job; its
subclasses expand the nearest unified ancestor's macros and list its slots
flat. `no C yet` is the same while its methods are carved INCLUDE_ASM game
code (revision 18: every round-80 `no C` class was one); it becomes an
ordinary class when track 1 matches one.

**Procedure, one class per commit.**
1. `typeviews.py --census`: the class's own methods and every object and
   table view. Grep the table symbol, its getter and its method names too: a
   unit's local `extern` of any of them is a view.
2. Names: `<Class>` from the existing object view (drop an `Obj`/unit
   suffix) or the table's stem, `<Class>Methods`, `include/<Class>.h`. A
   name for what the class's own methods DO is allowed when its header
   banner states that evidence (Actor: translation setters, local-axis
   moves, link search; round 82); never one for what it is guessed to be.
3. Union: `--merge` the object views, then the table views. Every CONFLICT
   is settled by the accessors' bytes; a slot's return type is its
   occupant's unless a caller's bytes need otherwise (a void call cannot
   cross-jump with a value-returning one: `include/Entity.h` slotC4).
4. Boundary: the class's own fields run from its parent's size to its own,
   the size from an allocation (`New_<Class>`) or, with none, from any
   subclass's first own field. A field a subclass view names inside that
   range is this class's. Pad what is unknown; never move an offset.
5. **Check who calls a function before you trust the fields it touches.**
   Layout is measured; meaning is not. Class6D430's `unk40..unk74` were
   method-table SLOTS: the function copying them is passed tables (round 80).
6. Slots: expand the parent's SLOTS with this class's type and ctor
   parameter list, then its own slots named for their occupants
   (`classtable.py <table> --vs <parent>`). An inherited slot keeps the
   parent's name at every accessor (Pad's `onButtonEvent` was BasicClass's
   `notifyParents`). An override is named for its slot with `rename.py`
   (`Class__Finalize`, `Class__Release`) unless its report shows the body
   does more than the slot name says.
7. Write the header; delete every other view and local `extern` of the
   table, getter and methods; include the class header where needed. Fix
   what the compiler lists: accessor renames, base-table calls upcast
   (`typeviews.py --upcast <getter> <Base> <files>`; a pointer cast emits no
   code), slot renames.
8. After every step: `./build-and-verify.sh` byte-identical,
   `typeviews.py --warnings` 0 new (a GONE warning is progress: rewrite the
   baseline with `--baseline` in that commit and say so),
   `tools/check-nonmatching.sh` green.
9. A renamed function's report gets a dated `Track 4` paragraph with the
   evidence. `rename.py` rewrites the old name in prose too, including
   sentences it turns into nonsense: read the diff.

**Rules.** A type change to a field or slot is allowed when the oracle stays
green and the commit names the accessors that settle it. A subclass's own
views are the subclass's job: leave them compiling and report any
contradiction with the parent. Once a class is unified, CLAUDE.md's
independent-local-views convention ends for it: no unit declares its own
view again, and `plan.py classes` lists any that appears as a STRAY VIEW.

**Staffing.** Opus runners, classes strictly in SEQUENCE: a class merge
touches every unit that sees the class, so the next class is staffed only
after the previous one is merged and marked, as many per round as that
allows (revision 20; round 82 ran three this way). `plan.py` lists only the
top ready class (most classes below it first). Prompt §4.6. Head
review before `python3 tools/plan.py mark-class --table <sym> --class
<Class>`: `plan.py classes` shows no stray view, the three oracles are green,
and three sampled slot or field names agree with their occupants or
accessors. **Park rule:** two views that disagree on a field's type at the
same offset, with both readings confirmed by their accessors, stay split
with a comment naming both (`mark-class --park "<reason>"`).

**Track 4b: one type per global.** `typeviews.py --globals` lists every
global declared `extern` with more than one type. The largest case is one
0x34-byte record table in the code_179d8 units, declared through thirteen
field-address symbols (`D_8008D988`...) in five record types. The procedure
(one struct type declared once; the field symbols removed from the symbols
file so accesses read `table[i].field`) has not been tried: its first
instance is a premium head's, who writes the recipe here and then runs
`plan.py set-track --track 4b --status open`, which turns `plan.py`'s
premium job into runner jobs.

### Track 5: close-out

Opens when tracks 3 and 4 are done. Items, ticked with `plan.py check --item`:

| item | done when |
| --- | --- |
| `readme` | a reader-facing README.md: what the game's code is, how it is organised (classes, subsystems, units), how to build, where the SDK comes from |
| `credits` | CREDITS.md names every inherited name, tool and reference |
| `asm-sites` | every live `__asm__` justified at the site or retired; the deferred questions from 2026-09-12 (`TransformAndCullPoly` as C or `INCLUDE_ASM`; the bare barriers) answered |
| `docs-budget` | every doc within its `plan.py` budget |
| `nonmatching-clean` | `tools/check-nonmatching.sh` green; every stall has a `NON_MATCHING` body or a written reason |

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
> a HARD RULE tension, a plan change). Never act on those yourself.

### 4.2 Naming runner prompt (track 3; head fills in `<>`)

> You are a naming runner for the LSD: Dream Emulator (PSX) decomp, round
> `<N>`. Work ONLY in worktree `<path>` on branch `runner/<name>`, on
> `src/<unit>.c`, the header(s) it owns `<list>`, and the match reports of its
> functions. Read CLAUDE.md, `docs/FINISHING-PLAN.md` §3 track 3 and
> `docs/PARALLEL-RUNS.md` §2 and §5 first.
>
> Do the pass in §3 track 3 in order, for every function in the unit including
> `NON_MATCHING` bodies. Use `python3 tools/rename.py OLD NEW` for every
> function and global rename; never rename by hand. Fields and slots follow
> step 3's ownership rule: rename what only your unit accesses; PROPOSE the
> rest in the report under `## Proposed field names` and on the broadcast, and
> leave the header alone. Read the broadcast before each function. After every
> rename and every header edit:
>
> ```sh
> ./build-and-verify.sh > /tmp/<name>_b.log 2>&1; echo "build exit=$?"; \
> grep -nE 'error:|parse error|undefined reference|\*\*\* \[[^]]*\.o\]' /tmp/<name>_b.log | head -8
> tools/check-nonmatching.sh   # after a FIELD or SLOT rename: the accessors the default build cannot see
> ```
>
> Exit 0, no hits, and check-nonmatching green, or the rename is wrong; revert it.
>
> Naming rules: name what the code does, not what you guess it is for; tiers
> A/B/C recorded per name in the report's `## Naming` section with the
> evidence; conventions as §3 track 3 lists them; inherited names are
> hypotheses to confirm or replace; Sony symbols are never renamed; a wrong
> tier-A name is worse than `func_`. Before calling a body unnameable, grep
> `src/` for its placeholder-named callees: one matched and named elsewhere
> often names the caller (round 66). When unsure, keep `func_` and write down
> what you know.
>
> Commit per logical step: one commit per `rename.py` run or per batch of
> renames in this unit, one commit per header edit, one for the unit header
> comment. `git status --porcelain` empty when you report. Never push, never
> edit shared docs or the splat yaml, never edit the symbols file except
> through `rename.py`.
>
> Final summary: a table of every function in the unit: old name, new name,
> tier, one-line evidence; the header comment you wrote; every header edit
> you made and every field name you PROPOSED for the head to apply; anything
> you could not name and why.

### 4.3, 4.4 (tracks 1b and 2)

Archived with their tracks: `docs/archive/FINISHING-PLAN-tracks-1-2-2026-09-25.md`.

### 4.5 Extern review prompt (track 3; Opus)

> You are a review runner for the LSD: Dream Emulator decomp, round `<N>`,
> worktree `<path>`, branch `runner/<name>`. Read CLAUDE.md and
> `docs/FINISHING-PLAN.md` §3 track 3. Run `python3 tools/externcheck.py`. For
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

### 4.6 Types runner prompt (track 4; Opus; head fills in `<>`)

> You are a types runner for the LSD: Dream Emulator (PSX) decomp, round
> `<N>`. Work ONLY in worktree `<path>` on branch `runner/<name>`. Your job is
> ONE class: `<table symbol>`, as `plan.py`'s job line names it. Read CLAUDE.md,
> `docs/FINISHING-PLAN.md` §3 track 4, `include/BasicClass.h`'s banner and
> `include/Class6D430.h` (the worked example of a class with subclasses), then
> do track 4's procedure for your class, in order. The edit is non-local: you
> may edit every file `python3 tools/typeviews.py --census` lists for your
> class, the units whose accessors the compiler then flags, and their match
> reports; you may not unify, rename or retype any other class. After every
> edit:
>
> ```sh
> ./build-and-verify.sh > /tmp/<name>_b.log 2>&1; echo "build exit=$?"; \
> grep -nE 'error:|parse error|undefined reference|has no member|conflicting|\*\*\* \[[^]]*\.o\]' /tmp/<name>_b.log | head -8
> python3 tools/typeviews.py --warnings | tail -3
> tools/check-nonmatching.sh
> ```
>
> `OK: build matches retail`, 0 new warnings and nonmatching green, or undo
> the last edit. Functions and globals are renamed only with
> `python3 tools/rename.py OLD NEW`; types, fields and slots by hand, the
> compiler listing the accessors. Before you name or retype a field, read the
> CALLERS of the functions that touch it (step 5). Commit per step (renames,
> the new header, each view replaced), with the message in a file
> (`git commit -F`): backticks in a double-quoted `-m` run as commands.
> `git status --porcelain` empty when you report; never push; never edit
> shared docs, the ledger, or the symbols file except through `rename.py`.
>
> Final summary: the header you wrote; every slot and field you named or
> renamed with the occupant or accessor that shows it; every view and local
> `extern` you deleted; every function you renamed and why; every `--merge`
> CONFLICT and how the bytes settled it; anything parked or unsettled; any
> contradiction you saw in a subclass's views.

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
