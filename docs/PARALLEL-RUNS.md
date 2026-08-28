# Parallel matching runs — worktrees + head-agent orchestration

How to run several matching sessions at once without them corrupting each
other, supervised by one senior "head" agent that provisions cheap runners and
triages their results.

Most of the specific rules here are scar tissue from a sister project (South
Park N64) where the same workflow ran for ninety-odd rounds. The reasons are
kept next to the rules deliberately: a rule without its reason gets "helpfully"
undone by the next session. Where a rule has not yet been tested *on this
project*, it says so.

## Why worktrees are required

Two sessions in one checkout share `build/`. Concurrent `build-and-verify.sh`
runs interleave objects and can report a **false mismatch** — poisoning the only
oracle either session trusts, silently. A git worktree gives each session its
own working tree, build directory and branch, while sharing history so merges
are ordinary.

```sh
tools/setup-worktree.sh alpha     # -> ../lsddecomp2-wt-alpha, branch runner/alpha
tools/setup-worktree.sh bravo     # -> ../lsddecomp2-wt-bravo, branch runner/bravo
```

The script symlinks the gitignored essentials (the executable, the venv, the
toolchain), runs `make extract`, and **proves the worktree byte-verifies before
handing it over**. That last step is not ceremony: a runner in a worktree that
does not verify produces scores that mean nothing, and it has no way to notice.

Teardown, after the four preconditions in §4b:

```sh
git worktree remove --force ../lsddecomp2-wt-<name> && git branch -d runner/<name>
```

**`--force` is required here, not a shortcut.** `asm/`, `build/` and `lsdde.ld`
are generated and untracked, so plain `git worktree remove` always refuses.
That means the "are you sure?" backstop never fires for us and the four
preconditions below are the *only* thing standing between a finished round and
a lost one. Check them by hand, every time.

## Collision rules (what makes parallelism safe)

1. **One unit per runner.** A runner is assigned exactly one `src/<unit>.c` and
   touches only that file plus `docs/match-reports/` entries for functions in
   it. Nothing else.

2. **A preserved body must be INLINED in the match report, as literal source,
   with every declaration it needs, positioned where it would compile.** Both
   halves matter. A body inlined perfectly whose types and globals live only in
   prose does not compile as preserved, and reconstructing the declarations
   lands short of the reported score with no way to tell which guess is wrong.
   The cheap self-test before restoring the `INCLUDE_ASM`: splice the preserved
   text back in and build once.

   Not a path — and above all not a path into a gitignored directory like
   `permuter-work/`, which exists in exactly one checkout and travels to no
   worktree, machine or clone. The same applies to a probe harness: if a probe
   establishes something, inline the probe.

3. **No shared-doc edits in parallel mode.** Runners must NOT edit
   `DECOMPILATION_LEARNINGS.md`, `MATCHING-GUIDE.md`, `PROGRESS.md`,
   `config/symbols.slps01556.lsdde.txt` or the splat yaml. A generalizable
   discovery goes in the match report under a `### Proposed learning` heading;
   the head consolidates after merging.

4. **Commit per match on the worktree branch; never push.** The head merges.
   A runner whose branch is *strictly behind* main may bring itself current
   with `git merge main --ff-only` — that cannot conflict or create a merge
   commit, and it is the fix when a long round leaves a worktree's extraction
   stale. A runner must NOT perform a real merge once its branch has diverged;
   only the head can adjudicate a conflict between two units' claims.

5. **The main checkout is single-occupancy while a round is live.** Only the
   head works there. A concurrent session's uncommitted files get swept into
   the head's consolidation commit. If you must work during a round, use
   another worktree.

## The head-agent protocol

The head runs in the MAIN checkout on an expensive model. Its loop:

1. **Provision.** Run the decision gates below, then `tools/setup-worktree.sh
   <name>` per runner and decide unit assignments (`python3 tools/progress.py`).

2. **Spawn** one subagent per worktree, cheap model, in background, with the
   runner prompt at the end of this file. **Write down the agent-ID-to-runner
   mapping and name the unit in every message** — a misaddressed wrap-up
   instruction is otherwise undetectable by its recipient.

3. **Triage on completion.** Read each runner's structured summary.
   - Matched functions: `cd` into the worktree, spot-check `tools/funcdiff.py`
     per claimed match, review the C for idiomatic quality and the commits for
     hygiene. **Do NOT run `build-and-verify.sh` inside a live runner's
     worktree** — that is the exact interleaving hazard this document opens
     with, and it can hand the runner a false mismatch. Verification belongs in
     main after merging. To inspect without disturbing: `git -C <wt> log` and
     `git -C <wt> status --porcelain` touch nothing.
   - **A runner's stall CLASSIFICATION is a hypothesis, not a finding.** Verify
     the reasoning, not just the score. Ask what the class's actual
     discriminator is and whether this instance has it. **This is the
     highest-yield thing the head does** — in the sister project it repeatedly
     turned a "toolchain lead" into an ordinary match on the head's first
     attempt. Budget head time for it explicitly; it is worth more per token
     than one more runner.
   - **Check a report file exists for every function touched, matched ones
     included.** Mechanically: `python3 tools/progress.py` and read the unit's
     `stalled` column. A runner that filed several stalls and shows `stalled 0`
     wrote none. Send it back — it still has the attempt history and will do it
     faster and more accurately than you can reconstruct it.

3b. **Check `git status --porcelain` in every worktree the moment a runner
   REPORTS, not at teardown.** A runner's final message is evidence about what
   it DERIVED, never about what it COMMITTED, and those are different claims.
   In the sister project two of four runners once finished a full pass with
   everything untracked, on a shared misreading of the commit rule, and their
   summaries were entirely accurate.

   ```sh
   for n in alpha bravo charlie delta; do
       printf '%s: ' "$n"; git -C ../lsddecomp2-wt-$n status --porcelain | wc -l
   done
   ```

   Non-zero means send that runner back to commit BEFORE you merge or tear
   down. Do not commit on its behalf unless the runner is dead — it knows which
   report belongs to which function and you are guessing.

3c. **Send an early finisher back into its OWN unit** (message the same agent —
   same worktree, same branch, context intact) rather than letting it idle or
   spawning a cold replacement. This is the highest-yield *structural* move
   available: every derivation is already in that agent's context, and a
   zero-match first pass is not a wasted pass — its reports are what prove a
   stall class has several instances. Hand it the lever explicitly, name the
   exact remaining functions in a cheapest-first order, say which residues are
   expected to survive, and tell it to UPDATE existing reports rather than
   replace them. Merge the first pass anyway; merging twice is free.

4. **Merge** sequentially, in main: `git merge --no-ff runner/<name>` →
   `./build-and-verify.sh` → next. Disjoint units make conflicts rare.

4b. **Teardown has four preconditions.** Check all four before
   `git worktree remove --force` — which, because generated files always make
   plain `remove` refuse, is the command you will actually type, with its
   safety net already disabled:
   - Every function the runner touched has a `docs/match-reports/` FILE.
   - Every runner has REPORTED, not merely gone quiet.
   - `git log --oneline main..runner/<name>` is EMPTY for every branch.
     (`git branch -d` refusing to delete is a backstop, not a check.)
   - The agent-ID-to-runner mapping is right for any wrap-up messages.

4c. **When a runner dies to INFRASTRUCTURE rather than its own stop rule,
   salvage its uncommitted body before teardown.** The runner cannot file its
   own report; it is gone. So the head does it:
   1. `git -C <wt> status --porcelain` on EVERY worktree, not just quiet ones.
   2. Identify the in-flight function: diff the `INCLUDE_ASM(` list between the
      worktree file and its own `HEAD`. The missing entry is what was live.
   3. **Score it.** Copy the worktree's `.c` into main, `./build-and-verify.sh`,
      `tools/funcdiff.py <fn>`, then `git checkout --` the file. Safe once the
      runner is dead, and the number is what makes the body usable later — a
      body at 128/146 is worth resuming and one at 1/276 is not, and you cannot
      tell without measuring.
   4. Preserve each body as literal source in its report, with the score.
   5. **Label it a mid-attempt snapshot, not a considered plateau.** No author
      applied a stop rule to it, so it carries none of the "this is as far as
      reshaping got" implication a normal preserved body does.

   Related trap: **audit a runner's diff from the MERGE-BASE, not from `main`.**
   `git diff --name-only main..runner/<name>` lists files where *main* has moved
   ahead — the head's own commits, shown reversed — and reads exactly like a
   runner touching protected files. Use
   `git diff --name-only $(git merge-base main runner/<name>)..runner/<name>`.

5. **Consolidate.** Promote `Proposed learning` entries into
   DECOMPILATION_LEARNINGS.md, update MATCHING-GUIDE's unit-state list, write
   the PROGRESS.md entry. Sweep for staleness: any doc line the round
   invalidated gets fixed in place, never left contradicted.

6. **Report to the operator** only what matters: new stall classes, suspected
   toolchain deltas, hook violations or oracle anomalies, match count. Then
   recommend: another runner round, a carve, or permuter.

Escalate immediately if: a branch fails to verify after a runner claimed
matches; a runner edited protected files; two runners produced contradictory
learnings.

## Decision gates — what the head does ITSELF

Run these IN ORDER at the start of every round. Never spawn runners into a
state a gate should have fixed first. These are the head's jobs, not
escalations.

**Gate 1 — true queue triage (always).** Assign from `progress.py`'s **`fresh`**
column, never from raw `queued` — the latter includes documented stalls, and
staffing a runner onto one means paying again to re-derive what someone already
wrote down. Keep both honesty mechanisms fed:

- every stall or blocked function MUST have its own match-report file (a stub
  report citing an aggregate classification is fine);
- every unit carved but not offered to runners MUST say **`DELIBERATELY
  UNWORKED`** in its header comment. `progress.py` keys on that exact phrase to
  move the unit into the `banked` column instead of `fresh`. Without it a carve
  inflates `fresh` by its whole size and the next head mis-triages.

`fresh` is a ceiling, not a work order: it cannot see "large body, deep
reconstruction, low cold-runner yield". Read the unit-state list in
MATCHING-GUIDE before believing a number.

**Gate 2 — carve to refill.** If fresh-assignable functions are fewer than
roughly (runners × per-runner target), carve new units BEFORE provisioning.
There are **1230 uncarved functions** here, so this gate will fire early and
often. The candidates, largest first: `class_39e08` (415), `code_179d8` (274),
`code_2c054` (181), `Entity` (142), `psyq_*` (library — leave for last).

1. Pick the next contiguous run of uncarved functions, ~20 per unit.
2. **Check boundaries in the disassembly, on both sides.**
   - *Under-split.* A function allocates its frame exactly once, so more than
     one `addiu $sp, $sp, -N` inside a single function's `.s` means the symbol
     covers SEVERAL functions:

     ```sh
     grep -c 'addiu *\$sp, *\$sp, *-' asm/nonmatchings/<unit>/<func>.s
     ```

     Any count above 1 is conclusive. splat under-splits when a function is
     reached only through a function-pointer table, because it derives symbols
     from `jal`/`j` references and there are none — so expect this wherever
     table dispatch is common.

     **In this game that is the NORMAL case, not an exotic one.** It is plain C
     built on a hand-rolled class framework (proven —
     docs/research/class-framework.md), with **60 classes and ~1425 method slots
     dispatched through tables in the data**. Every one of those slots is an
     entry point splat has no `jal` to. A cheap corroborator: a
     prologue/epilogue frame-size mismatch. Another: an m2c seed for a
     400-instruction symbol that comes back with a 9-line body — m2c stops at
     the first `jr $ra`.

     `tools/classtable.py --scan` lists every table, and the addresses inside
     them are exactly the entry points splat may have missed — so they double
     as carve boundaries. Cross-check a candidate band against it before
     splitting.
   - *Exit side.* A "function" with no epilogue and no callers that falls
     through into the next one is a mis-carve — fold it, do not split it. Scan
     for non-`.L` labels (alt-entries) and `jr $reg` dispatchers.
   - *Entry side.* Flag any function whose FIRST instruction touches `$sp`
     before its own `addiu $sp, $sp, -N`. Reading the caller's frame before
     allocating your own is only possible for a pre-prologue fragment: it is
     picking up a stack argument on the caller's behalf and falling through into
     the real body. Also grep for splat's `alabel` directive, which no
     `^(\w+):` regex matches: `grep -rn '^\s*alabel' asm/nonmatchings/<unit>/`.
     That check can only fire AFTER extraction, so run it as a post-carve
     confirmation.
   - *A dead orphan is not automatically inert.* Grep the orphan's `.s` for
     label references (`grep -o '\.L[0-9A-F]*'`). Empty output means a
     fall-through orphan: `INCLUDE_ASM` forever, done. Non-empty means it
     branches into its successor, and that label is LOCAL to the successor's
     `.s` — so `INCLUDE_ASM` only assembles while the successor is ALSO
     `INCLUDE_ASM`, and the build dies the moment anyone matches the successor.
     Emit the orphan as its literal encoding instead,
     `__asm__(".word 0x........");`, in source position. Do this AT CARVE TIME,
     or it surfaces as a build break in a runner that has already spent its
     attempt budget.
3. Split the segment in the splat yaml, `make extract`, create the new
   `src/<unit>.c`.
4. **`./build-and-verify.sh` MUST stay green — a correct carve changes zero
   bytes.** Red build → revert the carve and escalate; never force it.
5. Commit the carve on main as its own commit, THEN provision worktrees, so
   runners inherit it and never touch the yaml.

**Gate 3 — permuter round instead.** If the fresh queue is dry and carving is
blocked, or the stall residue is worth more than cold ground (near-misses like
88/90), run a permuter round. A zero is a LEAD, not an answer: translate it to
idiomatic C and re-verify with funcdiff. If only UB or duplicate-arm forms reach
zero, mark the class permuter-exhausted in the report and move on.

**Never authorized — always an operator escalation, evidence attached:**
toolchain or flag changes of any kind; editing the protected verification files;
bypassing build-and-verify.

Failing a gate is allowed. If one cannot be satisfied confidently — ambiguous
carve boundary, contradictory unit state — report the specifics and stop, rather
than burning a runner round on a bad premise.

## Sizing guidance

This project is **much cheaper per build than the N64 sister project**: a clean
`build-and-verify.sh` takes well under a second against ~30s there. CPU
contention during builds is therefore not the binding constraint, and on a
32-core host 4–6 runners is comfortable.

What actually saturates is **HEAD ATTENTION**. Six runners means five or six
stall classes to adjudicate, six merges and a doc consolidation. Budget the
head's time as the scarce resource; if the head also intends to take a
substantial task of its own, stay at three.

Queues need not be equal — give the leaf-dense unit to the runner with the
largest queue. Carve BEFORE spawning so runners never touch the yaml.

**Mid-round broadcasts are worth their cost.** A lever found at hour one is
worth several times more applied across four units than banked for the
write-up. Push it to every live runner immediately, and **ask explicitly for the
negative answer** — "does this apply to your unit?" A runner reporting that it
does NOT apply is cheap, and it stops the next head re-litigating the question.

## Runner subagent prompt (head fills in `<>`)

> You are a matching runner for the LSD: Dream Emulator (PSX) decomp. Work ONLY
> in the worktree `<path>` (cd there first) on branch `runner/<name>`. Read
> CLAUDE.md, docs/MATCHING-GUIDE.md and docs/DECOMPILATION_LEARNINGS.md, then
> decompile up to `<N>` functions from the `INCLUDE_ASM` entries in
> `src/<unit>.c` ONLY.
>
> **The oracle.** `./build-and-verify.sh` plus `tools/funcdiff.py`. Chain them
> so you cannot read a score from a failed build:
> `./build-and-verify.sh > /tmp/b.log 2>&1; echo "build exit=$?"; grep -nE 'Error [0-9]|error:|parse error|undefined reference' /tmp/b.log | head -8; .venv/bin/python3 tools/funcdiff.py <fn>`
> — the ONLY line that decides whether the number is meaningful is `build
> exit=`. funcdiff also guards this itself and exits 2 when it cannot trust the
> number; read its warnings. A failed COMPILE and a failed LINK both leave the
> previous build in place, and if that build had the function as `INCLUDE_ASM`
> you get a FALSE FULL MATCH with no diagnostic. See CLAUDE.md, "The three ways
> a score lies".
>
> **Hard stop at 30 attempts per function.** Restore the `INCLUDE_ASM` for
> anything short of a byte-exact match — no score justifies leaving C in
> `src/`, it fails the whole-file SHA1 on merge — and INLINE the body you
> reached into the report as literal source, with its declarations.
>
> **A match report FILE for EVERY function you touch, matched ones included**
> (`docs/match-reports/<func>.md`). A file, not a `.c` comment and not your
> final message. `tools/progress.py` decides STALL vs untouched FRESH ground
> purely by whether that file exists, so a stall with no report gets someone
> staffed straight back onto it.
>
> **COMMIT EVERYTHING YOU PRODUCE, INCLUDING STALLS.** One commit per MATCHED
> function (do not batch matches); stalls may be batched into a single "stall
> reports" commit. Read that carefully — it is NOT "commit only on a match".
> An uncommitted file does not exist: `git status --porcelain` must be empty
> when you report. Never push.
>
> **Keep every function and INCLUDE_ASM in your unit in STRICT ROM-ADDRESS
> ORDER.** Writing a definition out of order miscompiles the whole image — the
> link succeeds, the SHA1 fails, and previously-matched functions appear to
> regress with diffs unrelated to your C.
>
> **Never use `register T v asm("$N")` or an extended-asm operand constraint to
> fix a register.** Both are banned project rules. A bare `__asm__("")`
> scheduling barrier is allowed. The test: if removing it changes WHICH REGISTER
> holds a value it is banned; if it only changes instruction ORDER it is
> allowed. A register-identity mismatch is a STALL — report it.
>
> **For a one-line wrapper that tail-calls a non-void function, the byte match
> tells you NOTHING about the return type** — a `void` wrapper around an `s32`
> tail call is byte-identical. Write `return callee(...);` unless you have
> positive evidence the function is void.
>
> PARALLEL MODE RULES: do not edit DECOMPILATION_LEARNINGS.md,
> MATCHING-GUIDE.md, PROGRESS.md, config/, or any file outside your unit. Put
> generalizable discoveries in the match report under `### Proposed learning`.
>
> Your final message must be a structured summary: functions matched (with word
> counts), functions stalled (with class + best score), proposed learnings (one
> line each), and anything anomalous.

## Head-agent session prompt (operator pastes this)

> Act as the head agent for parallel matching runs per docs/PARALLEL-RUNS.md.
> First run the decision gates: triage the TRUE fresh queue, carve new units
> yourself if the queue is thin, or run a permuter round instead if that is the
> better move — these are your jobs, not escalations. Then, if running runners:
> create `<K>` worktrees, assign units, spawn one cheap-model runner subagent
> per worktree in background, triage their summaries as they finish, verify and
> merge their branches into main, consolidate learnings, update docs, tear down
> the worktrees, and report: matches, new stall classes, toolchain leads (never
> act on these yourself), and whether the next round should be runners, a carve,
> or permuter.
