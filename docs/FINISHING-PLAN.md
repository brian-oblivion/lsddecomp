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

Plan revision: 7 (2026-09-19, after round 58: the make guard never fired in
a worktree; Gate 3 check 3 had no real-build measurement; externcheck's
guidance was wrong for the dead-argument idiom; the model table's
"escalated" clause was read as a hand-off).
Changing the plan is a Fable head task; record the change in
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
| **head** | Opus | **Fable** only when the round itself will WRITE a new procedure, tool or doc, or must adjudicate a HARD RULE tension or a toolchain lead. Escalated gaps are NOT a reason for the next round's head to be Fable: the operator hands them to a Fable session BETWEEN rounds, which revises this plan and the tools, and the next round runs on Opus again. An Opus head executes what is written and escalates every gap in its report instead of writing procedure (round 50 onward). |
| **matching runner** (tracks 1, revisit) | decided by calibration: `plan.py` says which | round A Sonnet, round B Opus, on comparably ranked stalls; thereafter whichever produced more matches per runner-session. The head may override with `plan.py set-model`, with a reason in PROGRESS.md. |
| **naming runner** (track 3) | Opus | **Sonnet** once the head has reviewed two Opus-named units and found no wrong tier-A name. Back to Opus if a Sonnet unit fails review. |
| **mechanical runner** (track 2 identification, track 1b promotion, report hygiene) | Sonnet | never higher |
| **types runner** (track 4) | Opus | head (Fable) does the first class itself |

Fable never runs as a runner. The head spawns runners with the Agent tool's
`model` parameter. **The head states its own model in its first message; if
this table says the round needs Fable and it is running as Opus, it says so
and stops before spawning anything.** The operator's runner cap in the pasted
prompt is binding; with none stated, use three.

## 3. Tracks

Tracks are not gates on each other. `plan.py` prints one ready-jobs list
that takes turns across every open track (fresh matches, naming, a stall
runner, the SDK batch, revisits, promotions, types, close-out), and the head
fills its runner slots from the top, one unit per runner. A round with two
slots therefore always carries one naming pass and one stall runner; a
priority-sorted list starved every track but naming for two rounds. Every
track has a stop or park rule so that "hard" becomes "parked with a written
reason" rather than another round.

### Track 1: stall matching, calibrated, with a stop rule

**Goal.** Take the matches that are still cheap; measure the rate; stop when
it is not worth the tokens.

**Order.** `fresh` functions first (however large). Then stalls by ATTEMPT
COST as `plan.py` reads it off each report: no spent-levers verdict first,
never permuter-searched next, length-exact next, smallest last. Each job line
carries the tags (`unspent,never-searched,len-exact`). `nearmiss.py` ranks by
size alone and is the wrong list to assign from; a head that finds itself
skipping the top job for a documented reason reports it, per §6, rather than
re-ranking by hand.

**Runner budget** (in the runner prompt, PARALLEL-RUNS.md §5): at most three
functions per session; stop a function after 30 consecutive builds without a
better funcdiff score or when its one bounded search ends; never re-attempt a
function whose report marks its levers spent unless the brief names a changed
state.

**Calibration and stop rule.** Measured in stall ATTEMPTS per model, not in
rounds: Sonnet runners take stall jobs until Sonnet has six attempts from the
ranked band, then Opus runners until Opus has six, then whichever produced
more matches per attempt keeps the track. `plan.py` says which model is next
and groups the ranked stalls into one-unit runner jobs of three, so a single
runner slot per round accumulates toward it. Fresh giants and revisits are
jobs too, but a round that worked only those is recorded with
`--not-calibration` and does not count (round 50 measured two runners on a
324w and a 954w fresh body and was re-flagged this way). After each round
the head records:

```sh
python3 tools/plan.py record-round --track 1 --round <N> --model <sonnet|opus> \
    --runners <R> --attempts <functions attempted> --matches <byte-exact matches> --note "..." \
    [--not-calibration]
```

When both models have their six attempts and the twelve together produced
fewer than 3 matches, `plan.py` parks the track by itself and says so. The
head does not argue with it in the same round. Parked means: no runner is
staffed onto a stall for matching. It does not delete anything.

**Spent markers are retired by whoever spends them.** A report carrying
`REOPENED -- ASSIGNABLE` or `DERIVATION ONLY -- ASSIGNABLE` counts as fresh
until that line is gone. The runner who attempts such a function replaces the
marker line with the outcome (a stall title with its three figures, or
MATCHED), in the same commit as the attempt. The head checks it at merge:
every attempted function's report must no longer carry a marker. Rounds 49
to 52 re-ranked the queue on markers nobody had retired.

**Revisit rule.** A score taken before a unit's types and names existed is
evidence about what was known then, not about the function (southpark round
126: 4 of 4 warm bodies matched once their unit's types were derived). So
after a unit passes track 3, each of its stalls becomes a REVISIT job exactly
once. A second trigger, added after rounds 55 and 57 both moved a stall by
re-reading an old title rather than by using new names: a stall whose report
mentions no round within the last ten is eligible too. `plan.py` lists both
kinds (a stall with no REVISITED line whose unit has passed, or whose title
is stale). A revisit is one bounded attempt with the budget above; its report
line says `REVISITED, round N: <outcome>; names/types <used | not relevant>`,
so it is not listed again and the naming hypothesis keeps being measured.

**Head at merge.** Verify per PARALLEL-RUNS.md §3.9, record the round, and
correct any report whose cause the round falsified.

### Track 1b: NON_MATCHING bodies

**Opens** when track 1 is parked or done.

**Goal.** Every stall with a hand-derived preserved body has that body in
`src/`, readable and compiled by `tools/check-nonmatching.sh`, while the
verified build keeps the `INCLUDE_ASM`. This is what sm64, oot, mm and most
matching projects do; the default build never sees the body.

**Shape**, and it is the only accepted shape:

```c
#ifdef NON_MATCHING
/* NON_MATCHING: 252/258 words, length exact. Residue: register identity in
 * the second loop (docs/match-reports/func_80066340.md). Hand-derived. */
void func_80066340(Foo *this, s32 arg1)
{
    ...
}
#else
INCLUDE_ASM("asm/nonmatchings/code_55dd4", func_80066340);
#endif
```

splat still emits the `.s` because the `INCLUDE_ASM(..., name)` text is in
the file; `progress.py` strips the `#ifdef` half so the function counts as
queued, not matched.

**Rules.**
- Hand-derived only. A permuter candidate is promoted only after a human-style
  review that its semantics are what the disassembly does; the report says
  which it was. A body that scored well by exploiting the scorer (UB, dead
  branches) is never promoted.
- The comment names the score, the residue class and the report.
- `./build-and-verify.sh` green (nothing changed) AND `tools/check-nonmatching.sh`
  green (the body compiles and references only linked symbols).
- The report gets a line `NON_MATCHING body promoted, round N`.
- Track 3 naming applies to these bodies exactly as to matched ones.

**Runner** (Sonnet), prompt in §4.3. **Park rule:** a stall whose preserved
body does not compile after one session of repair gets a `NO NON_MATCHING
BODY: <reason>` line in its report title region and is done.

### Track 2: the SDK call surface

**Goal.** Every function game code calls whose address lies in a Psy-Q
segment carries Sony's name. `plan.py --json` lists the unnamed ones under
`tracks.2.unnamed_list`. Nothing else in the SDK is a goal.

**The tool:** `.venv/bin/python3 tools/sdkname.py <func>...` (or `--all`)
scores each unnamed SDK function against every function in every object on
every disc in `sdk/` by relocation-masked comparison and a shape ratio, and
prints ranked candidates with disc and module, plus the placed objects on
either side of the function as position evidence. `--selfcheck 10` recovers
placed functions exactly (9 of 9 at revision 2); rerun it whenever the corpus
changes. Read its three flags: `EXACT` is the same build; `TINY` means a body
of six words or fewer matched a stub shape that many functions share, which is
not identification without position evidence; `AMBIGUOUS` means several names
match exactly and position decides. A top candidate with `masked` under about
0.6 and no `EXACT` is a function from a library build the discs do not carry,
and only position plus header prototype can name it.

**Evidence, strongest first**, and a name needs two independent kinds or one
fingerprint above the threshold the tool's self-check established:
1. fingerprint against a library object (disc and module recorded);
2. position: SDK archives link modules in a fixed order, so a function
   between two placed objects of one library is that library's module between
   them, and the `.LIB` listing says which functions the module exports;
3. the Psy-Q header prototype agreeing with every call site's argument
   shape (the byte oracle cannot see a wrong signature, so this is the check
   that catches one);
4. strings, BIOS call numbers, or hardware register addresses in the body.

**Recording.** The name goes in the symbols file via `tools/rename.py` with a
trailing comment: `// identified: fingerprint 0.96 vs libsnd/ss_xxx.o (3.3), header LIBSND.H`.
Declare it where the game calls it as a local `extern` copied from the Psy-Q
header's prototype, citing the header in a comment. Game units do NOT include
`psyq/*.H` and shared project headers do NOT carry Sony prototypes: that is
the `conflicting types` collision CLAUDE.md warns about, and
`include/code_2cc8c.h`'s "NO LONGER DECLARED HERE" notes are the precedent.
A call site whose argument shape disagrees with the prototype is a finding,
not a nuisance. Byte-exact after every rename. A function with no evidence
keeps `func_` and gets a `// <library>, unidentified:` comment on the line
above its symbols-file entry; `plan.py` reads that comment and stops offering
the function.

**Runner** (Sonnet), prompt in §4.4. **Done** when `plan.py` reports zero
unnamed. **Park rule:** a function whose best candidate is below the bar and
whose position is ambiguous keeps `func_` with a `// <library>, unidentified:`
comment naming the best candidate and score; it is done for this track.

### Track 3: readability, one unit at a time

**Goal.** A reader can tell what each function, field, slot and global in the
unit does without opening the disassembly.

**Eligibility.** Every unit is eligible now; remaining `INCLUDE_ASM`s are
documented stalls. **Order:** `plan.py`'s centrality ranking (units whose
definitions other units reference most), because a name in a base class
propagates everywhere.

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
   error list is the exact accessor set.
   - every error is in your unit: fix them, oracle, done;
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
  method whose CLASS is known is `Class__func_xxxxx` (the existing
  `BasicClass__func_17eb0` convention), and bare `func_800xxxxx` otherwise;
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
- **Do not rename a Sony symbol.** Those names are Sony's.

**Head at merge.** Apply the runner's proposed cross-unit field names one at
a time by type scope (definition first, compiler lists the accessors, fix
those, oracle), then `make extract` (the symbols file changed) and the
oracle.

**Head review before `mark-unit`.** Sample five names per unit against their
evidence. A tier-A name without evidence, or a name that asserts purpose
from a single call, sends the unit back. Two clean Opus units in a row and
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

**Goal.** One definition per class in `include/`, all unit-local struct views
merged into it; every global named; state values as enums.

**Procedure, one class at a time.** `tools/classtable.py --scan` lists the
method tables. For each class: collect every local view (`grep -n 'typedef
struct' src/*.c`), union the fields, check every offset against every
accessor in every unit, write the shared definition next to the existing
header's related declarations, delete the local views, build. This edit is
non-local by construction: the oracle is the only check, so one class per
commit and the whole oracle after each.

**Staffing.** The head does the first class itself (Fable, new procedure)
and writes the recipe here. Then Opus runners, one class each, but a class
merge touches several units, so classes are merged SEQUENTIALLY, never two
runners at once. **Park rule:** two views that disagree on a field's type at
the same offset, with both readings confirmed by their accessors, stay split
with a comment naming both; do not force a union.

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
> not, stop and tell me. Otherwise fill at most `<K, or 3 if I did not say>`
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
> ```
>
> Exit 0 and no hits, or the rename is wrong; revert it.
>
> Naming rules: name what the code does, not what you guess it is for; tiers
> A/B/C recorded per name in the report's `## Naming` section with the
> evidence; conventions as §3 track 3 lists them; inherited names are
> hypotheses to confirm or replace; Sony symbols are never renamed; a wrong
> tier-A name is worse than `func_`. When unsure, keep `func_` and write down
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

### 4.3 NON_MATCHING promotion prompt (track 1b; Sonnet)

> You are a mechanical runner for the LSD: Dream Emulator decomp, round `<N>`,
> worktree `<path>`, branch `runner/<name>`, unit `src/<unit>.c` ONLY. Read
> CLAUDE.md and `docs/FINISHING-PLAN.md` §3 track 1b. For each of `<functions>`:
> take the preserved `#if 0` body from `docs/match-reports/<func>.md`, confirm
> from the report that it is hand-derived (a permuter candidate needs the
> review the track describes; say so if you cannot do it), place it in the
> unit in ROM order in the exact `#ifdef NON_MATCHING ... #else INCLUDE_ASM
> #endif` shape with the required comment, then run `./build-and-verify.sh`
> (must stay green: you changed no bytes) and `tools/check-nonmatching.sh`
> (must be green). Fix declarations the body needs inside the `#ifdef` block
> or in the unit; never in a shared header. Add `NON_MATCHING body promoted,
> round <N>` to the report. One commit per function. A body you cannot make
> compile in one session gets `NO NON_MATCHING BODY: <reason>` in the report's
> title region instead. Report per function: promoted / not, and why.

### 4.4 SDK identification prompt (track 2; Sonnet, once `tools/sdkname.py` exists)

> You are a mechanical runner for the LSD: Dream Emulator decomp, round `<N>`,
> worktree `<path>`, branch `runner/<name>`. Read CLAUDE.md and
> `docs/FINISHING-PLAN.md` §3 track 2. For each function in `<list from
> plan.py>`: run `python3 tools/sdkname.py <func>`, gather the evidence kinds
> the track lists, and name it ONLY if the evidence rule is met, with
> `python3 tools/rename.py <func> <SonyName>` and the identification comment
> in the symbols file. Declare it at the call site as a local `extern` copied
> from the Psy-Q header prototype (never include `psyq/*.H` in a game unit,
> never put a Sony prototype in a shared header); a call site that disagrees
> with the prototype is a FINDING to report, not to paper over. `./build-and-verify.sh` green after each. A function below the
> evidence bar keeps `func_` and gets a `// <library>, unidentified: <best
> candidate and score>` comment. One commit per function. Report a table:
> function, name or unidentified, evidence, disc and module.

## 5. Doc hygiene

The southpark sister project's plan, prompts and scripts grew past the point
where an operator could paste one thing and walk away. The rules that keep
this repo from following:

- **One append-only narrative: `docs/PROGRESS.md`.** Every other doc states
  current rules. A rule carries its reason in one sentence and a pointer to
  the PROGRESS round or archive section that holds the story.
- **No counts in docs.** Measure with `progress.py` and `plan.py`.
- **Line budgets**, enforced by `plan.py` as warnings and by track 5 as a
  checklist item. Over budget means distil, and move the history to
  `docs/archive/`.
- **A new idiom is at most 12 lines in DECOMPILATION_LEARNINGS.md**, with
  its discriminator and its evidence in one clause each.
- **One head prompt.** New kinds of work get a track section and a runner
  prompt HERE, not a new doc and a new paste.
- **The ledger holds decisions, never counts.** If you find yourself writing
  a number into `plan-state.json`, it belongs in a tool.

## 6. When to reconsider the plan

Reconsider, as a Fable head task recorded in PROGRESS.md with the revision
line above bumped, when any of these happens:

- a stop rule fires and the operator wants the track reopened anyway;
- a track's yield is off by more than a factor of two from what its section
  assumes (say so with the measurement);
- a whole new class of work appears that no track covers;
- `plan.py`'s ordering keeps putting a job at the top that the head keeps
  skipping for a good reason. The fix is in the tool, not in ignoring it.

Do not reconsider because a round went badly. One round is a measurement.
