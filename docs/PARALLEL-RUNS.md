# Parallel runs: worktrees and the head-agent protocol

How several agents work this repo at once without corrupting each other, under
one head agent that provisions, triages, merges and consolidates.

This is the PROCEDURE. The history that produced each rule, with the round
numbers and the measurements, is `docs/archive/PARALLEL-RUNS-full-2026-09-16.md`
(referenced below as "archive") and `docs/PROGRESS.md`. A rule here states its
reason in one sentence and links the story; do not paste stories back in.
`tools/plan.py` warns when this file grows past its line budget.

WHAT to run a round on, in what order, and which model does what, is
`docs/FINISHING-PLAN.md`. The operator pastes the head prompt from THERE. This
file is what that prompt tells the head to follow for the mechanics of a round.

## 1. Worktrees

Two sessions in one checkout share `build/`, and concurrent builds interleave
objects and can report a FALSE MISMATCH on the only oracle the project has. So
every runner gets a git worktree: its own tree, build directory and branch.

```sh
tools/setup-worktree.sh alpha     # -> ../<checkout>-wt-alpha, branch runner/alpha
```

- **Five fixed names: alpha, bravo, charlie, delta, echo.** The operator's
  permission grant (`.claude/settings.local.json`, `additionalDirectories`,
  gitignored, absolute paths) covers exactly those, so a sixth name prompts on
  every command with no error saying why. Five runners is the cap. Check the
  grant exists rather than believing this paragraph:

  ```sh
  python3 -c "import json;print(json.load(open('.claude/settings.local.json')).get('permissions',{}).get('additionalDirectories'))"
  ```

  A missing grant is the operator's call to make, not the head's. Ask.
- The script symlinks the gitignored essentials (executable, venv, toolchain,
  `lib/`, the broadcast file), runs `make extract`, and PROVES the worktree
  byte-verifies before handing it over. A worktree that does not verify
  produces scores that mean nothing.
- `<checkout>` is the basename of your main checkout. Read the real path off
  the script's output or `git worktree list`; never hardcode it.
- **Teardown** needs `--force` because `asm/`, `build/` and `lsdde.ld` are
  untracked, so git's own "are you sure" never fires. The four preconditions in
  §3.7 are the only safety net.

  ```sh
  git worktree remove --force ../<checkout>-wt-<name> && git branch -d runner/<name>
  ```

## 2. Collision rules

1. **One unit per runner.** A runner touches `src/<unit>.c`, headers that
   unit includes, and `docs/match-reports/` files for functions in it. Nothing
   else.

   Headers are NOT partitioned by this rule. Price header contention at
   assignment time, with one command, and tell runners the mitigations:

   ```sh
   python3 tools/headercontention.py <unit> <unit> ...
   ```

   Contention is a function of what runners PUT in a shared header, not how
   many share it. Three runners edited one header in round 16 with zero
   conflicts because every edit was an additive pad split with the total
   preserved, and no cross-unit prototype went into the shared header. Round
   15 put five runners on one header without those rules and six of seven
   merges conflicted (archive §"Collision rules" 1).

   The three things to tell runners sharing a header: header edits strictly
   ADDITIVE; a prototype for a function ANOTHER unit defines, or an `extern`
   of a unit-local type, goes in your own `.c`, never in the shared header
   (it collides as `conflicting types` in whichever OTHER unit includes both
   headers, possibly two merges later); state explicitly in the summary any
   change to an EXISTING declaration.

   At merge time, expect two runners' views of one struct to be complementary
   and union them; unify a field that got two type names; then re-verify
   every match from both runners individually, not just the whole-image SHA1.

2. **A preserved body is INLINED in the match report as literal source, with
   every declaration it needs, positioned where it would compile.** Not a
   path, and never a path into a gitignored directory. Splice it back in and
   build once before restoring the `INCLUDE_ASM`: a body that never linked has
   a score that measured nothing (archive §Gate 1b, round 33).

3. **No shared-doc edits by runners.** Runners do not edit
   `DECOMPILATION_LEARNINGS.md`, `MATCHING-GUIDE.md`, `PROGRESS.md`,
   `FINISHING-PLAN.md`, the symbols file, or the splat yaml. A generalisable
   finding goes in the match report under `### Proposed learning`; the head
   consolidates. Exception: a NAMING runner edits the symbols file through
   `tools/rename.py` only, for symbols in its own unit (FINISHING-PLAN track 3).

4. **Commit per match on the runner branch; never push.** A branch strictly
   behind `main` may `git merge main --ff-only`. A diverged branch never
   merges itself; only the head adjudicates a conflict.

5. **The main checkout is single-occupancy while a round is live.** Only the
   head works there, and only for merges and consolidation. Measure a
   salvaged or experimental body in a worktree, never in `main`: a red build
   in the shared checkout is an attribution hazard for everyone reading it.

6. **A correction goes down ONE channel.** Fix a live runner's file by
   telling the runner (broadcast), OR by editing `main`, never both; both
   produce the same paragraph twice and a conflict on the round's largest
   branch. Default to the message. Edit `main` only for files no live runner
   owns.

7. **A worktree isolates FILES, not the process table, `/tmp`, or ports.**
   - Never `pkill -f <tool name>`; it kills every runner's search. Kill by a
     PID you captured, or scope to your own worktree path.
   - Log paths carry the runner name: `/tmp/<name>_b.log`, never `/tmp/b.log`.
   - Bound every long search with `timeout`, and write its exit status to its
     OWN file, because a multiprocessing search's shutdown noise races an
     `echo` on a shared descriptor (archive §2b, round 48):

     ```sh
     timeout 600 <search> > search.log 2>&1; rc=$?; printf '%s\n' "$rc" > rc.txt
     ```

     `124` means your bound fired; `137` means something outside killed it.
     Record the iteration count in every negative as the content-level
     fallback.
   - One search at a time per runner. Under saturation a second search halves
     both iteration rates and the payoff is binary.
   - A negative collected under load is "not closed in N iterations under
     load", never "permuter-exhausted".

## 3. The head protocol

The head runs in the main checkout. Which model it runs on is decided by the
model table in FINISHING-PLAN.md, not here.

### 3.1 Before anything: am I the only head?

Rounds 6 and 7 ran concurrently on recycled branch names and each head
recorded the other's actions as its runners misbehaving (archive §4a).

```sh
git log --oneline -5; git reflog -15; git reflog show origin/main; git worktree list
```

A commit you did not make, or a worktree you did not provision, means another
session is live. Do not delete or re-create any worktree or branch; escalate.

### 3.2 Gate 0: verify and re-extract

`asm/` is gitignored and a `git pull` does not re-extract it, so a fresh
carve or rename leaves `progress.py` silently UNDER-reporting and the build red
on a missing `.s` (archive §Gate 0). Every round, first:

```sh
python3 tools/progress.py            # read the TOP: stale-asm warnings print above the table
rm -f asm/<each stale monolith it named>.s
make extract && ./build-and-verify.sh; echo "build exit=$?"
```

Not green after this: stop and diagnose. Do not triage, spawn or carve. The
stale-monolith warning is per worktree, so a runner seeing it in its own tree
while `main` is clean is expected; tell it so.

### 3.3 Gate 1: what to assign

`python3 tools/plan.py` prints the ready-jobs list across all open tracks
with a model per job. For matching jobs it ranks from `tools/nearmiss.py`,
which runs every screen from the ASM and prints report titles verbatim.
Assign from `progress.py`'s `fresh` column and `nearmiss.py`'s `ASSIGN FROM
HERE` list, never from raw `queued`.

**The honesty markers `progress.py` and `nearmiss.py` key on.** Each exists
because a report file's mere existence turns a function from FRESH into
STALL, so any report that is not a stall verdict deletes ground silently:

| marker | where | effect | anchoring |
| --- | --- | --- | --- |
| `REOPENED -- ASSIGNABLE` | report, on a line of its own | stall verdict invalidated (its blocker died); counts as fresh | start of line; a title prefix breaks it |
| `DERIVATION ONLY -- ASSIGNABLE` | report, own line | partial derivation, no body ever built; counts as fresh | start of line |
| `NOT GAME CODE` | report title region (first 8 lines) | Sony's code with no object to prove it; excluded from assignment | title window |
| `DELIBERATELY UNWORKED` | unit header comment | carved but not offered; counts as banked | bare substring, whole file: never write the phrase where you do not mean it to fire |

A marker edit is not done until the COUNT moves. Check the column, not the
diff (archive §Gate 1, round 46).

**Screens `nearmiss.py` runs for you, and what each misses** (archive §Gate
1b, one round each):

1. The three toolchain constructs, all RESOLVED, reported with a
   `(RESOLVED-not-a-blocker)` tag and never counted. A report whose verdict
   predates the fix is wrong in its CAUSE, and a wrong cause is what the next
   round acts on.
2. Sony ownership by placed object (`tools/sdkstalls.py`). These pass every
   other screen and read as the cleanest ground in the queue.
3. Sony ownership with NO object on any disc: the `NOT GAME CODE` marker,
   a head decision backed by segment topology and the `addiu`/`ori` assembler
   fingerprint, never by smell.
4. Rank from TITLE lines only. A report body is full of figures describing
   variants that were thrown away, and three consecutive heads pulled a wrong
   one. A title is only as good as the last person who rebuilt it; rebuild
   any figure before it goes in an assignment.
5. Attempt history. A 252/258 got there by being worked; the figure that
   ranks it first is also evidence the cheap levers are spent. Prefer the
   smallest gap with the SHALLOWEST history and mix worked and unworked
   ground in one assignment.
6. Permuter history, keyed on evidence of a RUN (iteration counts, `rc=`,
   `base score`), never on the word "permuter". The word appears most where
   the lever was recommended and never pulled.
7. Stale cross-references. A closed function's citers are never updated. A
   citation of a function that left the queue into a Sony object is a
   precedent that NEVER EXISTED; one that left by matching is a better lead
   than claimed. `tools/stalesyms.py` catches renamed callees in preserved
   bodies; the cross-reference sweep is the shell in archive §Gate 1b round 38.

**Also at Gate 1:** price header contention (§2.1); read
`src/<unit>.c` header comments for stale directives ("do not attempt", "blocked
until X") and retire them; a directive that names its own expiry condition is
still obeyed for rounds after the condition is met.

### 3.4 Gate 2: carving

Closed. `python3 tools/uncarved.py` reports zero uncarved game functions since
round 47. Re-run the tool rather than believing this line. If it ever reports
ground again, the recipe and its five routine failures (orphaned rodata
jump-table slot, shared rodata slot, data tail, `dlabel` in a text segment,
reverting a carve leaves the `.c`) are archive §Gate 2, and the rule is: one
segment at a time, byte-neutral with `INCLUDE_ASM` in place, extract TWICE.

### 3.5 Gate 3: permuter

A scorer zero is a LEAD; `OK: build matches retail` is an ANSWER. Reject UB
candidates; adopt any form the real oracle verifies and comment it at the
site. Before spending a search, three checks, cheapest first (archive §Gate 3,
rounds 40 to 47):

1. Does the scaffold compile and score? (correctness)
2. `--debug --stack-diffs` insertion/deletion count (cost, necessary not
   sufficient)
3. Does the scaffold's signature AGREE with the same body rebuilt in the real
   tree? Three outcomes, one of which says search:

| scaffold | real build | verdict |
| --- | --- | --- |
| any signature | the same signature | AGREE, search is meaningful |
| dirtier than the real build | zero drift | MISMATCH, scaffold artifact, decline |
| perfect zero | does not match | whole-file effect, decline, stop looking inside the function |

Zero-ness is not the discriminator; agreement is. A recorded negative whose
search never passed check 3 is not evidence about the function, and a
negative is a verdict about the BODY it was measured on: after a rewrite that
moves the score, re-run checks 2 and 3 before citing the old negative
(round 57, bravo). Translate
every promising candidate AND measure it in-tree: permuter units are not
funcdiff words, in either direction.

### 3.6 Spawn, then triage as runners finish

- `tools/broadcast.sh clear` before spawning. The broadcast file is the only
  channel; `SendMessage` is not available in this environment and its absence
  is not an anomaly. Post every lever, negative and correction the moment you
  have it; read it at every triage, because runners post corrections upward
  and a cheap-model runner disagreeing with the head, with numbers, is the
  protocol working (round 47).
- Spawn one runner per worktree in the background with the prompt in §5 (or
  the track prompt in FINISHING-PLAN.md). Write down the agent-to-runner
  mapping.
- **Count matches from the branch, not the summary:**

  ```sh
  git log --oneline main..runner/<name>
  git show runner/<name>:src/<unit>.c | grep -c '^INCLUDE_ASM'    # anchored
  ```

- **A report file for every function touched**, matched included. A runner
  showing `stalled 0` after filing stalls wrote none; send it back.
- **A stall classification is a hypothesis.** Verify the reasoning, not the
  score: what is the class's discriminator and does this instance have it?
  This is the highest-yield thing the head does.
- Never run `build-and-verify.sh` inside a live runner's worktree. Inspect
  with `git -C <wt> log` and `git -C <wt> status --porcelain`.
- **Check `git status --porcelain` in every worktree the moment a runner
  reports.** Non-zero means send it back to commit. Do not commit on its
  behalf unless it is dead.
- Audit a runner's diff from the merge-base, not from `main`:
  `git diff --name-only $(git merge-base main runner/<name>)..runner/<name>`.

### 3.7 Liveness: a notification is not a death certificate

A runner notifies the head each time it stops with no live background children
of its own. It can resume afterwards. So "stopped, and no processes" is a
restatement of the event that summoned you, not evidence of death. Three
heads (rounds 31, 43, 49) recovered or killed under live runners on that test
(archive §2d, §"A runner that stops...").

- **Teardown is the only deadline.** Salvage uncommitted work only when you
  are about to `git worktree remove`, and not before.
- Never spawn a replacement into a worktree whose agent might resume. A
  re-send goes to a FRESH worktree name.
- Sample twice, minutes apart, before concluding anything: a live runner's
  tree keeps changing.
- Do the salvage (non-destructive) and DEFER the kill (irreversible) when
  both are triggered by the same inference.
- Sweep for orphaned search workers before teardown, matching on a
  structural argument and excluding your own process ancestry (shell in
  archive §2c round 27); `pgrep -f <pattern>` matches the shell running it.
- Salvage, when it is time: identify the in-flight function by diffing the
  `INCLUDE_ASM` list against `HEAD`; score it in a WORKTREE; preserve it in
  the report labelled a mid-attempt snapshot.

The durable fix is on the runner side and is in the prompt: COMMIT BEFORE YOU
WAIT. A runner that commits before backgrounding a search costs nothing when
it stalls.

### 3.8 Re-sends

Second, third and fourth passes produced 9 of round 19's 17 matches. Since
there is no wake channel, a re-send is a fresh agent in a fresh worktree name,
handed the round's findings IN THE PROMPT, sent to DIFFERENT ground than the
wall its predecessor hit. Ask for the disposition of every assigned function
including "did not reach"; an unlisted function drops out of the next queue.

### 3.9 Merge, consolidate, push

```sh
git status --porcelain                       # your own tree must be clean first
git merge --no-ff runner/<name>; echo "merge exit=$?"
git rev-parse -q --verify MERGE_HEAD >/dev/null && echo "MERGE IN PROGRESS"
./build-and-verify.sh; echo "build exit=$?"
```

A green build on top of `MERGE IN PROGRESS` means nothing (CLAUDE.md, fourth
way a score lies). Resolve first, verify after. Expect `modify/delete`
conflicts on report files the head stubbed and a runner then wrote; take the
runner's. Then, for every function the runner attempted, confirm its report
no longer carries an `-- ASSIGNABLE` marker (`grep -l -- '-- ASSIGNABLE'
docs/match-reports/<func>.md`); a spent marker left in place re-ranks the
next round's queue on ground that is no longer fresh.

**`make extract` again after merging any branch that changed the symbols
file** — i.e. every track 3 naming merge. `asm/` is untracked, so the runner's
`rename.py` re-extract happened in ITS worktree and `main`'s disassembly still
carries the old names; the link then fails on `undefined reference to
func_OLD` from whichever unit has an `INCLUDE_ASM` calling the renamed
function. Gate 0 covers this at the START of a round, not at merge (round 50,
merging `runner/charlie`: `src/code_55dd4.c` against `func_8001E770`).

**Do not pipe `build-and-verify.sh` into `tail`/`head` to read its tail.**
`$?` is then the PIPE's last command and is always 0, which is the "necessary
but not sufficient" hazard reinstalled by hand. Redirect to your own log and
echo `$?` before grepping it (round 50, the head did this and read `build
exit=0` off a failed link).

After all merges: grep the tree for every symbol a shared-header resolution
declares; promote `Proposed learning` entries into DECOMPILATION_LEARNINGS.md
(one idiom, at most 12 lines, story to PROGRESS.md); write the PROGRESS.md
entry; update `config/plan-state.json` through `tools/plan.py` as
FINISHING-PLAN.md says; fix in place any doc line the round invalidated.

Teardown goes through the guard, which refuses on an unmerged commit, an
uncommitted path, or a live search in the worktree (round 57 removed a
worktree with an unmerged commit by chaining the command onto a push):

```sh
tools/teardown-worktree.sh <name>            # refuses unless merged, clean, idle
python3 tools/externcheck.py                 # after any round that MATCHED: externs vs new signatures
git status --porcelain && ./build-and-verify.sh && git push origin main
```

`externcheck.py` exists because a wrong `extern` arity is invisible to the
byte oracle: the callee reads registers the caller already left loaded
(round 57, `func_8001E7BC` declared `(void)` and called with none while
taking three). A match is the first moment a real signature exists to check
against.

Escalate to the operator, never act yourself, on: any toolchain or flag
change; a branch that fails to verify after claimed matches; a runner that
edited protected files; two runners with contradictory learnings.

## 4. Sizing

A clean build is under a second, so CPU is not the constraint. HEAD ATTENTION
is: five runners means five stall classes to adjudicate, five merges and a
consolidation. Stay at three if the head also takes a substantial task.
Spread runners across units with disjoint header sets; a concentrated round
should be SMALLER than a spread one. Give the leaf-dense unit to the runner
with the largest queue.

## 5. Matching runner prompt (head fills in `<>`)

> You are a matching runner for the LSD: Dream Emulator (PSX) decomp. **This
> is round `<N>`**; use that number in every report, title and commit message.
> Work ONLY in the worktree `<path>` (cd there first) on branch
> `runner/<name>`. Read CLAUDE.md, docs/MATCHING-GUIDE.md and
> docs/DECOMPILATION_LEARNINGS.md, then work the following functions from
> `src/<unit>.c` ONLY, in this order: `<list, cheapest first, with each one's
> recorded title figure and what a good negative looks like>`.
>
> **Budget.** At most `<K, default 3>` functions this session. Stop a function
> when 30 consecutive builds have not improved its best funcdiff score, or when
> your one bounded permuter search on it ends; then file the report and move
> on. Do not re-attempt a function whose report says its cheap levers are
> spent unless the broadcast or your brief names a CHANGED state. If the
> report carries a `REOPENED -- ASSIGNABLE` or `DERIVATION ONLY --
> ASSIGNABLE` line, your attempt SPENDS it: replace that line with the
> outcome in the same commit, or the function stays counted as fresh forever.
>
> **Broadcast.** Nobody can message you and you cannot message anyone. Run
> `tools/broadcast.sh read` before each function and act on it. The moment you
> have a lever, a negative ("lever X does not apply here"), a toolchain smell or
> a question, post it: `tools/broadcast.sh post --from <name> "..."`. A finding
> banked for the write-up reaches nobody. A commit on your branch you did not
> make is the head, not corruption.
>
> **The oracle.** Chain the build and the score so you cannot read a number
> from a failed build:
>
> ```sh
> ./build-and-verify.sh > /tmp/<name>_b.log 2>&1; echo "build exit=$?"; \
> grep -nE 'error:|parse error|undefined reference|\*\*\* \[[^]]*\.o\]' /tmp/<name>_b.log | head -8; \
> .venv/bin/python3 tools/funcdiff.py <fn>
> ```
>
> Any grep hit means your C did not build and the score is stale. Exit 2 with
> no hit is a fresh build that does not match yet. The `*** [….o]` pattern is
> load-bearing: GCC 2.6.3 prints most fatal errors without the word `error`.
> funcdiff exits 2 when it cannot trust its own number; read its warnings.
> Your log path carries YOUR name; `/tmp` is shared.
>
> **Searches.** One at a time. `timeout` on every search, exit status to its
> own file (`rc.txt`), iteration count in every negative. Run Gate 3's three
> checks (PARALLEL-RUNS.md §3.5) before spending a search and record which you
> ran. A search is BACKGROUND work: while it runs, read the next function's
> asm and prepare its report. COMMIT BEFORE YOU WAIT. Nothing runs on your
> behalf and no notification is coming to you; poll your own search and finish.
>
> **Rules that miscompile the image if broken.** Every definition and
> `INCLUDE_ASM` in the unit stays in STRICT ROM-address order. Never
> `register T v asm("$N")` or an extended-asm operand constraint to fix a
> register (a bare `__asm__("")` barrier is allowed; the GTE exception is in
> CLAUDE.md HARD RULE 6). Any struct edit is potentially non-local: run the
> whole oracle after every one. A rodata `D_XXXXXXXX` holding a string is a
> symbol to reference, never a literal to retype. A tail-call wrapper's byte
> match says nothing about its return type; write `return callee(...)`, but
> retyping a shared vtable slot needs every caller checked first.
>
> **Stalls.** No score short of byte-exact leaves C in `src/`; restore the
> `INCLUDE_ASM`. Preserve the body in the report in `#if 0 ... #endif`, never a
> block comment, inlined with every declaration it needs, and build the
> inherited body once before trusting its recorded score. A stall report's
> TITLE carries three figures: length (exact, or N words short/long), raw
> word-match M/N, and where the first real diff is per
> `tools/asm-differ/diff.py`. "N short" and "M/N" are different measurements
> that read identically.
>
> **Reports and commits.** A `docs/match-reports/<func>.md` for EVERY function
> you touch, matched ones included. One commit per matched function; stalls may
> batch. `git status --porcelain` must be empty when you report. Never push.
> Never edit shared docs, the symbols file or the splat yaml; put a
> generalisable finding under `### Proposed learning` in the report.
>
> **Final summary:** per assigned function, one line: MATCHED / STALL with the
> three title figures / SKIPPED with why / NOT REACHED. Then levers found,
> negatives found, and anything that smelled like a toolchain problem (report
> it, never act on it).
