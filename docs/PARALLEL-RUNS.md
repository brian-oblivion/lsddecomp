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
tools/setup-worktree.sh alpha     # -> ../<checkout>-wt-alpha, branch runner/alpha
tools/setup-worktree.sh bravo     # -> ../<checkout>-wt-bravo, branch runner/bravo
```

**Worktrees land OUTSIDE the project directory, so they are outside whatever
directory the session trusts.** `../<checkout>-wt-<name>` is a sibling of the
checkout, not a child, which is ordinary git practice but means an agent
working in one is operating outside the primary working directory. In
permissive/auto permission modes that shows up as runners prompting for
approval on ordinary commands — most visibly `git commit`, which they run
often — while the head in the main checkout is never asked. It is not a
misconfiguration and nothing is wrong with the worktree.

Decide before spawning: either grant the worktree paths (so the round runs
unattended), or expect to approve runner commands interactively. Granting is a
permission change, so it is the operator's call, not the head's — ask, do not
assume.

**On this machine the operator has granted it**, in
`.claude/settings.local.json`:

```json
{ "permissions": { "additionalDirectories": [
    "<abs path>/<checkout>-wt-alpha", "...-wt-bravo", "...-wt-charlie",
    "...-wt-delta", "...-wt-echo" ] } }
```

That file is the right home for it and the committed `.claude/settings.json` is
not: `additionalDirectories` takes ABSOLUTE paths, and the worktree path is
derived from the basename of whoever's checkout it is, so a committed entry
would be wrong in every clone but one. `settings.local.json` is gitignored, so
each operator grants their own.

**Two consequences for the head, and the first one is a real constraint:**

- **The grant is an allowlist of five FIXED names — alpha, bravo, charlie,
  delta, echo.** Provisioning a worktree under any other name puts it outside
  the grant and the round starts prompting again, with no error that says why.
  Stick to those five, which also caps a round at five runners. If you need a
  sixth, that is a settings change and therefore an operator decision.
- **It does not shrink §4a.** Fixed, recycled names are exactly what made two
  heads collide in rounds 6 and 7, and a standing grant on those names gives
  the next head one more reason to reuse them without looking. Run the §4a
  checks anyway.

**`<checkout>` is the basename of YOUR main checkout, not a fixed string.**
The script derives the destination from it, so a clone named `lsddecomp`
produces `../lsddecomp-wt-alpha`. This document previously hardcoded a
`lsddecomp2-` prefix from the checkout it was written in, which is wrong in any
clone named anything else — and it appeared in the `--force` teardown command
below, where a stale path is not a typo but a hazard. Read the real path off
the script's own output, or off `git worktree list`.

The script symlinks the gitignored essentials (the executable, the venv, the
toolchain), runs `make extract`, and **proves the worktree byte-verifies before
handing it over**. That last step is not ceremony: a runner in a worktree that
does not verify produces scores that mean nothing, and it has no way to notice.

Teardown, after the four preconditions in §4b:

```sh
git worktree remove --force ../<checkout>-wt-<name> && git branch -d runner/<name>
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

   **This rule does NOT partition headers, and the head must plan for that.**
   Round 8 assigned `class_3bb8c` and `class_3bb8c_b` — adjacent slices of one
   class block — to two runners, who then both edited `include/class_3bb8c.h`
   and produced the round's only merge conflict. Two consequences, both the
   head's job:

   - **At assignment time**, notice when two units share a header (adjacent
     slices of one carve almost always do) and tell both runners: header edits
     strictly ADDITIVE, place each new declaration next to related existing
     ones rather than in a block at the top or bottom, and *state explicitly in
     the final summary* any change to an EXISTING declaration. That last part
     is what makes the merge cheap; both round-8 runners complied and the
     conflict took one pass to resolve.
   - **At merge time**, expect the two runners' views to be COMPLEMENTARY
     rather than contradictory, and union them. In round 8 both had
     independently found the same struct field from different call sites, each
     had fields the other lacked, both had kept the struct size right, and the
     only true collision was that they had given one field two different type
     NAMES. Unify the name: the project's multiple-independent-local-views
     convention is for views in DIFFERENT unit headers, and two names for one
     field inside a single header is a trap for the next reader.
   - **Then re-verify EVERY match from BOTH runners individually**, not just
     the whole-image SHA1. A shared-declaration merge that compiles is not the
     same as one that preserves codegen.

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

2b. **A worktree isolates FILES, not the PROCESS TABLE — never `pkill -f` on
   a tool name.** Round 10's runner alpha finished with the permuter, cleaned
   up after itself with `pkill -9 -f "decomp-permuter"`, and killed runner
   **bravo's** in-progress search in a different worktree. The pattern matched
   because the tool path is identical in every worktree: nothing about
   `-f decomp-permuter` is scoped to the caller. Alpha self-reported it, which
   is the only reason it was ever detected — bravo's own summary would have
   read as "the permuter found nothing", and that is indistinguishable from
   the truth.

   **This is the same shape as the `/tmp/b.log` crossover in round 8**, and
   worth generalising once rather than rediscovering a third time: a git
   worktree gives each runner its own files, build directory and branch. It
   gives them a SHARED `/tmp`, a SHARED process table, and shared ports. Any
   runner action addressed by a global name rather than a path can reach into
   another runner, and it fails silently because the other runner has no way
   to distinguish sabotage from a normal negative result.

   Kill by PID you captured yourself, or scope the match to your own worktree
   path:

   ```sh
   pkill -9 -f "lsddecomp2-wt-<name>.*decomp-permuter"   # scoped
   pkill -9 -f decomp-permuter                            # hits EVERY runner
   ```

   The head's part: when a runner reports a permuter or long-running search
   that found nothing, and another runner ran one in the same window, **check
   whether the negative is real before writing it into a report.** A
   fabricated permuter-exhausted verdict is expensive — it is exactly the
   finding that stops future rounds from trying.

   **And there is a decisive way to check, better than comparing timestamps.**
   Round 10's echo, asked the same question, answered it from EXIT CODES
   rather than inference: GNU `timeout` returns **124** when it is the one
   that killed the child on schedule, whereas a child SIGKILLed from outside
   surfaces as **137**. So a search wrapped in `timeout N` that came back 124
   terminated on its own terms, and no `pkill` reached it. Wrap long searches
   in `timeout` for exactly this reason — it makes "did my own limit fire, or
   did someone shoot it?" a question with a recorded answer instead of a
   judgement call. A harness-level cap (e.g. a tool's own wall-clock limit)
   is a third case again, and its wording identifies it; say which of the
   three ended the run.

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

6. **A correction goes down ONE channel, not both.** When the head discovers
   mid-round that something in a live runner's unit is wrong, it can message
   the runner or it can fix the file on `main` — doing both produces the same
   paragraph written twice, differently, and a merge conflict in a `src/`
   file at the worst moment. Round 10 retracted a blocker misclassification
   by messaging runner echo AND editing echo's unit header comment on main;
   echo complied, wrote its own better version, and the merge conflicted. The
   conflict was cheap to resolve, but it arrived attached to the round's
   largest branch (16 matches) and it is what exposed the conflicted-merge
   oracle trap in CLAUDE.md's "four ways a score lies".

   **Default to the message.** The runner is holding the unit, has the
   context, and will write it better than the head will — echo's version was
   kept. Edit `main` only for files no live runner owns: shared docs, another
   unit's header, the splat config. If the head must fix a live unit's file
   anyway, say so in the message explicitly so the runner leaves it alone.

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
       printf '%s: ' "$n"; git -C ../<checkout>-wt-$n status --porcelain | wc -l
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

4a. **BEFORE ANYTHING ELSE, establish that you are the only head in this
   checkout.** Rounds 6 and 7 (2026-09-02) ran CONCURRENTLY: the operator
   started round 7's head while round 6's head was still live, and both worked
   the same `main`. Nothing was corrupted — every commit was real, no protected
   file was touched, and `main` byte-verified throughout — but each head
   misread the other's actions as its own runners misbehaving, and both wrote
   that misreading down as fact. Round 6's PROGRESS entry accused its runner
   alpha of a protocol violation for two commits round 7's head had made, and
   filed round 7's runner output as "the runners kept working after reporting".
   Round 7's head, symmetrically, found commits on `main` it had not made and
   had to rule out a rogue runner before it could rule in a second head.

   **The mechanism is RECYCLED NAMES.** Worktrees and branches are `runner/
   alpha`…`runner/echo` every round. A head that tears down and re-provisions
   those names hands the other head a branch with the same name, a different
   round's work on it, and no signal that it changed underneath. `git merge
   runner/charlie` then merges a stranger's commits, verifies green (they are
   real matches), and reads as completely normal.

   Cheap checks, in order:

   ```sh
   git log --oneline -5                     # commits you do not remember making
   git reflog -15                           # merges/commits you did not perform
   git reflog show origin/main              # pushes you did not make
   git worktree list                        # worktrees you did not provision
   ```

   **A quiet `git log` is not evidence the other session has ended.** Round 6's
   head stopped committing at 13:59 and round 7's head recorded it as
   finished on that basis — but `main` was pushed to `origin` twice more
   during round 7, carrying round 7's own commits. Only
   `git reflog show origin/main` showed it. Re-check before teardown, not
   just at the start.

   A commit in `git log` that is not in your own record is the tell, and
   authorship does NOT distinguish it — every agent commits as the operator.
   If you find one: **do not delete or re-create any worktree or branch**
   (that is what caused the collision), let live runners finish and commit,
   merge what is genuinely yours, and escalate to the operator. Recycling a
   name is only safe when you know no other head is holding it.

4b. **Teardown has four preconditions.** Check all four before
   `git worktree remove --force` — which, because generated files always make
   plain `remove` refuse, is the command you will actually type, with its
   safety net already disabled:
   - Every function the runner touched has a `docs/match-reports/` FILE.
   - Every runner has REPORTED, not merely gone quiet.
   - `git log --oneline main..runner/<name>` is EMPTY for every branch.
     (`git branch -d` refusing to delete is a backstop, not a check.)
   - The agent-ID-to-runner mapping is right for any wrap-up messages.

   **Check them immediately before the `--force`, not once at the start of
   consolidation.** Cheap, and sound regardless of why a branch moved. Round 6
   read a moving branch as "has REPORTED does not imply has stopped writing";
   the branches were in fact advancing because another head's runners were
   committing to the same recycled names (§4a). A runner writing on past its
   own summary has not actually been observed here. Re-check because the tree
   can move, not because you know which agent moves it.

   **And do not skip teardown.** Round 6 deferred it for good reasons and left
   five worktrees standing; round 7 inherited them, and one held THREE
   uncommitted byte-exact matches with their reports (`func_8005A7A0`,
   `func_8005B904`, `func_8005B990`) that no summary had ever mentioned. They
   survived only because nobody ran `--force` on that worktree. Deferring is
   the right call when agents may still be live — but it hands the next head a
   §4c salvage it has no way to anticipate, so say so explicitly in PROGRESS,
   naming the worktrees.

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
reconstruction, low cold-runner yield". Before believing a number, size the
queue (`wc -l asm/nonmatchings/<unit>/*.s | sort -rn`) and read the match
reports for the functions you intend to assign — see "Unit and segment state"
in MATCHING-GUIDE.

**Gate 2 — carve to refill.** If fresh-assignable functions are fewer than
roughly (runners × per-runner target), carve new units BEFORE provisioning.
Most of the game is still uncarved, so this gate fires early and often.

List the candidates live, largest first. **Do not work from a written list** —
carving renames things, and every list of these written down so far has ended
up naming segments that no longer exist:

```sh
for f in asm/*.s; do b=$(basename "$f" .s); case "$b" in psyq_*|header) continue;; esac
    printf '%6d %s\n' "$(grep -c '^glabel' "$f")" "$b"; done | sort -rn
```

The `psyq_*` segments are Sony SDK code: excluded from the game-code
denominator, and left for last. Matching them proves nothing about this game.

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

     **MEASURED ON THIS GAME, AND IT DOES NOT HAPPEN — do not go hunting for
     it.** The reasoning above predicted it would: the game dispatches ~1425
     methods through tables in the data (docs/research/class-framework.md), and
     none of those entry points has a `jal`. The prediction was wrong, because
     spimdisasm also scans DATA for pointers into `.text` and makes a symbol
     from each one. The check: of the **894** distinct function addresses
     referenced by class tables, **894 have a glabel** — the only three without
     one are already matched as C, so no `.s` is generated for them. A
     corpus-wide `addiu $sp, $sp, -` census over every uncarved segment
     (`class_39e08` included) returns **zero** functions with more than one
     prologue.

     Re-run both if you doubt it; they are seconds. But budget nothing for
     under-split hunting here, and treat a hit as genuinely surprising rather
     than as the expected case. **This paragraph is the correction of a
     confident, unverified claim written two commits earlier** — the sister
     project's under-split problem is real, it just does not transfer, and
     nothing but running the check would have told us.
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
   - *BIOS call stubs are not expressible in C.* A body of the shape `jr $t2`
     with the vector in `$t2` and the call number in `$t1` is a PSX BIOS
     trampoline (`0xB0`/`0x33` is BIOS `malloc`). There is no C that compiles
     to it, so it needs an `hasm` segment or a literal `.word` disposition —
     **decide which at carve time**, not when a runner hits one and burns an
     attempt budget discovering it. They cluster in the class-framework block:

     ```sh
     grep -c 'jr *\$t2' asm/<segment>.s
     ```

     At the time of writing every one of them (13) sits in a single segment,
     but that segment is a carve remainder and its name changes as carving
     proceeds — run the grep rather than trusting a name.
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
   bytes.** Red build → fix or revert; never force it. **Carve ONE segment at a
   time and verify each**, rather than a batch: the whole cycle is about two
   seconds here, and a batch that goes red tells you nothing about which
   segment did it.

   Three failures are routine, all seen in the first carve round, and all of
   them are a red LINK rather than anything subtler:

   - **`undefined reference to '.LXXXXXXXX'`** — the unit's switch jump tables
     live in a rodata slot that is still a standalone segment. Those words
     point at labels *local to the unit's function `.s` files*, which a
     separate rodata object cannot see. Attach the slot:
     `- [0xNNNN, .rodata, <unit>]`. `code_4cd08` needed `0x206C` — a slot the
     inherited yaml had labelled `# greyman`, so **do not trust the inherited
     rodata comments to say who owns a slot**.

     Two standalone slots are known to hold text pointers and will need
     attaching when their segments are carved: **`0xFD8` (`code_179d8`)** and
     **`0xA8C` (`code_8220`)**. Both were confirmed against the data, not read
     off the yaml comment.
   - **`undefined reference to 'D_XXXXXXXX'`** — the segment's tail is DATA,
     not code, and an `asm` segment was emitting it inline. Find where the text
     really ends and declare the rest: `code_55dd4`'s text stops at `0x57028`
     and the remainder is a few ints plus a character-classification table.
     Declare it **`data`, not `rodata`** — `section_order` puts `.rodata` first,
     so a `rodata` declaration relocates those bytes to the top of the image.
   - **Every function defined twice, after you REVERT a carve.** splat does not
     delete `src/<unit>.c` when a segment goes back to `asm`, so the stale unit
     keeps its `INCLUDE_ASM`s while the monolithic `.s` returns. **Reverting a
     carve means deleting the generated `.c` in the same step.**
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
> `./build-and-verify.sh > /tmp/<name>_b.log 2>&1; echo "build exit=$?"; grep -nE 'Error [0-9]|error:|parse error|undefined reference' /tmp/<name>_b.log | head -8; .venv/bin/python3 tools/funcdiff.py <fn>`
>
> **The log path MUST carry your runner name.** This prompt used to say
> `/tmp/b.log` for everyone, and in round 8 two runners writing that one
> file crossed over — one of them read another runner's build result and
> acted on it before noticing. `/tmp` is not per-worktree. That is a false
> oracle in the worst possible place: the log you grep to decide whether
> your score means anything. Substitute your own name into the path here
> and in every later invocation.
>
> **And by the same logic, never `pkill -f` a tool name.** Your worktree
> isolates files, not the process table: `pkill -9 -f decomp-permuter`
> matches every runner's permuter, because the tool path is identical in
> every worktree. Round 10's alpha did exactly this while tidying up and
> killed bravo's in-progress search. Kill by a PID you captured yourself,
> or scope the pattern to your own worktree path
> (`pkill -f "…-wt-<name>.*decomp-permuter"`). `/tmp`, the process table
> and ports are all shared; only files and git state are yours.
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
> **But if acting on that means retyping a SHARED VTABLE SLOT, check every
> other caller first.** Retyping a slot `void` -> `s32` is not local: it can
> change an ALREADY-MATCHED function's codegen elsewhere. Round 7 hit this
> exactly — with `EntityMethods::slotC4` typed `void`, GCC tail-merges two of
> `func_8005E160`'s identical discarded `slotC4` calls into one; retyped to
> `s32` it stops merging them, costing 4 words and shifting every later
> function in that unit. The runner caught it by recompiling the other
> caller's translation unit in isolation and diffing the `.s` BEFORE touching
> the real build, and kept the slot `void`. In the same round `slotCC` faced
> the identical question, was checked the same way, came back clean, and WAS
> retyped: **two superficially symmetric slots needed opposite answers**, so
> the check is per-slot and cannot be reasoned by analogy.
>
> The cheap version of the check: `grep -rn 'slotNN' src/` for every other
> caller, rebuild, and confirm the whole-image SHA1 is still green — a slot
> retype that breaks another function shows up as a red build, not as a
> diff in the function you are working on.
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
