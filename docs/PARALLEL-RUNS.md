# Parallel matching runs — worktrees + head-agent orchestration

How to run several matching sessions at once without them corrupting each
other, supervised by one senior "head" agent that provisions cheap runners and
triages their results.

Most of the specific rules here are scar tissue from a sister project (South
Park N64) where the same workflow ran for ninety-odd rounds. The reasons are
kept next to the rules deliberately: a rule without its reason gets "helpfully"
undone by the next session. Where a rule has not yet been tested *on this
project*, it says so.

> **A different kind of round exists now:** converting Psy-Q library
> disassembly to Sony's linked objects. Same worktree machinery, different
> collision rules (the edited files are shared config, not per-unit C) and
> different prompts — see `docs/SDK-OBJECTS-RUNS.md`.

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

The grant lives in `.claude/settings.local.json`:

```json
{ "permissions": { "additionalDirectories": [
    "<abs path>/<checkout>-wt-alpha", "...-wt-bravo", "...-wt-charlie",
    "...-wt-delta", "...-wt-echo" ] } }
```

That file is the right home for it and the committed `.claude/settings.json` is
not: `additionalDirectories` takes ABSOLUTE paths, and the worktree path is
derived from the basename of whoever's checkout it is, so a committed entry
would be wrong in every clone but one.

**CHECK THAT THE FILE EXISTS; DO NOT INFER IT FROM THIS PARAGRAPH.** For eleven
rounds this section read "on this machine the operator has granted it" as
settled fact. Round 12 went to spawn runners and found no
`.claude/settings.local.json` at all, and no `additionalDirectories` key at
project or user level either — the grant had never existed in this clone, and
every prior round had simply been approving runner commands interactively
without the doc ever noticing. Worse, the same paragraph asserted
`settings.local.json` "is gitignored"; it was not, so the first head to
actually write the file would have committed one operator's absolute paths into
everyone's clone — the exact failure the paragraph above explains how to avoid.
Round 12 added the `.gitignore` entry.

Two lessons, and the second is the general one:

- The grant is a permission change, so it is the operator's call, not the
  head's. Verify, then ASK — do not write it silently. One command:

  ```sh
  python3 -c "import json;print(json.load(open('.claude/settings.local.json')).get('permissions',{}).get('additionalDirectories'))"
  ```

- **A doc claim about MACHINE STATE decays differently from a doc claim about
  the BINARY.** A wrong fact about the executable gets caught the next time
  someone re-measures it, because measuring is the job. A wrong fact about a
  gitignore entry or a settings file is nobody's job to re-measure, so it
  survives indefinitely and is believed precisely because it has been there a
  long time. Anything here describing a FILE THAT SHOULD EXIST needs the
  one-line check that proves it printed next to it.

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
toolchain, and `lib/` — the Sony library objects the link needs), runs `make extract`, and **proves the worktree byte-verifies before
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

   **This rule does NOT partition headers, and the head must plan for that —
   at ASSIGNMENT time, with one command, before provisioning anything:**

   ```sh
   python3 tools/headercontention.py                     # the whole map
   python3 tools/headercontention.py <unit> <unit> ...   # verdict on a plan
   ```

   Two runners whose units include the same project header will both edit it.
   That is not a hypothetical and it is not rare — it has fired in every round
   that concentrated runners, and the cost scales with **how many runners share
   a header**, not with how many runners there are:

   | round | staffing | outcome |
   | --- | --- | --- |
   | 8 | 2 runners, adjacent slices of one carve | the round's only conflict |
   | 13 | 3 shared-header collisions | one git auto-merged with **no conflict marker**; only the build caught it, on `conflicting types` |
   | 15 | 5 runners, all `class_3bb8c_*` | **6 of 7 merges conflicted**; 4 hard prototype collisions, one latent across two merges |
   | 16 | 5 runners: **3** on `class_3bb8c_*`, **2** on fresh `code_179d8` carves with no project header | **0 of 10 merges conflicted on a header.** Three runners edited `class_3bb8c.h` and every one auto-merged |

   **Round 16 is the measurement that makes this rule actionable rather than
   cautionary, and the mechanism is worth stating plainly: contention is not
   a function of HOW MANY runners share a header, it is a function of what
   they put IN it.** Three runners edited `class_3bb8c.h` in round 16 — the
   same header, one more runner than round 13's three collisions — and
   nothing conflicted, across ten merges. What changed was not the count:

   - Every edit was an **additive pad split whose total was preserved**
     (verify this by hand at merge time; it is arithmetic, and the head
     checked every one). No offset moved, so no already-matched function's
     codegen could shift.
   - **No cross-unit prototype went into the shared header.** One runner
     matched a function whose canonical declaration already lived in
     `class_39e08.h` and *matched that declaration* instead of writing its
     own — the exact failure that killed round 15.
   - **Two runners independently reached the same class (`D_80087034`) and
     did not collide**, because one deliberately kept its view local to its
     own `.c` while the other put its view in the shared header. That is the
     multiple-independent-local-views convention working as intended.

   So the mitigations in this rule are not damage control for a bad staffing
   choice — applied up front they reduce contention to zero at three runners
   on one header. Tell runners all three things explicitly at spawn time.

   **NEW conflict class, round 16, and it is the HEAD's own doing.** Both of
   that round's two conflicts were `CONFLICT (modify/delete)` on a
   `docs/match-reports/` file: the head had written a stub report for a
   function it believed blocked, discovered mid-round that the screen was
   wrong, **deleted the stub on `main`** — and the live runner meanwhile
   turned that same file into a real report. Resolution is trivial (`git add`
   the runner's version; it is strictly better than a deleted stub), but two
   things about it matter:

   - It is invisible in the contention tooling, because it is not a header
     and not a `src/` file. `headercontention.py` cannot predict it.
   - It arrives as `merge exit=1` with `MERGE IN PROGRESS`, which is the
     state where a green `build-and-verify.sh` means **nothing** (see
     CLAUDE.md's fourth way a score lies). Resolve first, verify after.

   If you delete a stub report for a function a live runner holds, expect
   this and prefer telling the runner over racing it.

   **And a plainer trap the same round: the head can block its own merge.**
   `git merge` returned `exit=2` and refused to start — `Your local changes
   to the following files would be overwritten by merge` — because the head
   had uncommitted consolidation work touching the same report files. That is
   not a conflict and leaves no `MERGE_HEAD`; the fix is to commit your own
   work first. Check `git status --porcelain` in `main` before each merge,
   not just in the worktrees.

   None of those cost a match and none were unresolvable. What they cost was
   **head attention** — the constraint this document names as binding. Round 15
   left three runners un-resent with fresh ground still in their units because
   the merge queue, not runner capacity, had become the bottleneck. Prefer
   units whose header sets are disjoint; `headercontention.py` with no
   arguments lists the units that share nothing with anyone, and those are free
   to staff alongside anything.

   **Contention is a property of the HEADER SET, not the unit-name prefix, and
   this is the part that bites.** `class_3bb8c_k` looks like an ordinary
   `class_3bb8c_*` slice, but it also includes `class_39e08.h` — and that is
   exactly the edge that broke round 15, where a prototype one runner put in
   `class_3bb8c.h` collided with the canonical declaration in `class_39e08.h`,
   in the single unit that sees both. Grouping by name prefix would not have
   shown it. Run the tool rather than reading the unit names.

   **Concentrating is still sometimes right** — it is often the only ground
   available, and a `fresh` queue that lives entirely in one block leaves no
   choice. Take the trade knowingly: the tool prints the mitigations to apply
   when you do, and they are the same ones the rest of this rule describes.
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

   **A cross-unit PROTOTYPE in a unit-local type must never go in a shared
   header — and this one can stay latent across several merges before it
   breaks a DIFFERENT runner's unit.** Round 15 hit it three times in one
   round, which is why it has its own rule now. The pattern: runner A writes
   `extern <A's local type> *func_XXXX(void);` into `include/<shared>.h`,
   because A calls that function. It is fine until some translation unit sees
   a second, incompatible declaration of the same name:

   - **against another runner's DEFINITION.** alpha declared
     `extern Obj86ED0Methods *func_80051A4C(void);` in the shared header;
     bravo DEFINES `func_80051A4C` returning its own `Class86ED0Methods *`.
     That is `conflicting types for 'func_80051A4C'` — a hard compile error,
     not a warning, and it is what the merge actually died on.
   - **against a PRE-EXISTING canonical declaration in another header, which
     is the nasty one.** delta put
     `extern BaseMethods87034_3bb8c_l *func_8004A4B8(void);` in
     `class_3bb8c.h`, colliding with the project's long-standing
     `extern Class86668Methods *func_8004A4B8(void);` in `class_39e08.h`.
     **Nothing showed up when delta's own work was verified, and nothing
     could have** — `src/class_3bb8c_l.c` includes only `class_3bb8c.h`. It
     surfaced two merges later in `class_3bb8c_k`, the first unit to include
     both headers. So the runner who wrote it sees a green build, and the
     runner who breaks is one who never touched the declaration.

   The fix is always the same and always cheap: **move the declaration into
   the calling unit's own `.c`.** Same for an `extern <local type> D_XXXX;`
   for a data symbol another unit declares differently. A unit-local view
   belongs in the unit — which is exactly what round 15's bravo did
   deliberately with `Class86ED0`, writing "LOCAL to this unit (not added to
   the shared header)" in its own source. That instinct was right and it is
   the one to copy.

   Head's part, at merge time: after resolving any shared-header conflict,
   **grep the whole tree for every symbol the resolution declares** before
   trusting a green build, because the unit that breaks may not be in this
   merge:

   ```sh
   grep -rn '<symbol>' src/ include/ | grep -v '<the unit you just merged>'
   ```

   And a staffing lesson: this round put all five runners on adjacent slices
   of ONE class block, so all five edited one header and four of the five
   merges conflicted. The conflicts were all complementary and none cost
   much, but **if you can spread runners across unrelated blocks, header
   contention drops to zero.** Concentrating them is a real (and sometimes
   worthwhile) trade, not a free one — make it deliberately.

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

   **And capture the exit code on the VERY NEXT COMMAND, or the recorded
   answer is the shell's and not the search's (round 32).** The 124-vs-137
   test above only works if you actually read `timeout`'s status:

   ```sh
   timeout 600 <search>; rc=$?; echo "permuter rc=$rc"     # right
   timeout 600 <search>; echo "search finished"            # WRONG -- rc lost
   ```

   Round 32's alpha wrapped its search in `timeout` exactly as instructed,
   appended an unconditional `echo` after it, and the background job's own
   completion signal then reported the ECHO's exit status (0) rather than
   `timeout`'s. Its report says plainly that it could not distinguish
   "wall-clock bound fired" from "the search ended on its own" — which is
   the honest disposition, and is also the whole question this idiom exists
   to answer. The runner did nothing wrong; the instruction told it to wrap
   in `timeout` without telling it that anything appended with `;` destroys
   the status.

   Alpha recovered a partial answer the right way — `ps` showed no surviving
   search process, so nothing was left consuming the host — but that
   establishes the run ENDED, not WHO ended it, and those are the two claims
   the 124/137 test separates.

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
   runner prompt at the end of this file. Run `tools/broadcast.sh clear`
   first, so no runner reads last round's levers as this round's. **Write down the agent-ID-to-runner
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
   - **Count the runner's matches from the COMMITS, not from its summary
     table.** Round 11's alpha reported "8/8 attempted, 0 stalls" and listed
     "4 functions untouched"; the branch held 7 commits, 7 reports and 5
     remaining `INCLUDE_ASM`. The work was entirely sound — all 7 verified
     byte-exact — but the count was wrong in both directions at once, and the
     tell was a duplicated row in its own table. Nothing about a self-reported
     count is load-bearing; the branch is:

     ```sh
     git log --oneline main..runner/<name>
     git show runner/<name>:src/<unit>.c | grep -c '^INCLUDE_ASM'
     ```

     **Anchor that second grep — `^INCLUDE_ASM`, not bare `INCLUDE_ASM`.**
     It was unanchored here for fifteen rounds and round 15's head hit the
     consequence immediately: bravo's unit read as 9 remaining against a
     reported 8, and the extra hit was the literal string `INCLUDE_ASM`
     inside one of bravo's own explanatory comments. Runners write good
     comments, and good comments about a unit's remaining work mention
     `INCLUDE_ASM` by name.

     The failure mode is worth naming because it inverts this section's
     whole point. This grep exists so the head does not trust a runner's
     self-reported count — so when it disagrees with the summary, the
     reflex is "the runner miscounted". An over-count here manufactures a
     phantom remaining function and impugns an accurate summary, which is
     the opposite of the error the check was added to catch.

     This is cheap and it matters beyond bookkeeping: an over-counted "matched"
     figure inflates the round's headline, and an under-counted "remaining"
     list silently drops a function from the next round's queue. Alpha's
     omitted function (`func_8003DAD4`) would have gone unstaffed on the
     strength of a summary nobody checked.
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

2c. **A runner can get stuck in a permuter WAIT-LOOP, and it looks exactly
   like a runner that is still working.** Round 17's charlie ended three
   consecutive turns on "I'll wait for the permuter to report back". The
   mechanism is mechanical, not a judgement failure: `--stop-on-zero` means
   a search that never reaches zero **never terminates**, so "wait for it to
   finish" has no end state. Its first two runs were still alive, 34 and 20
   minutes in, holding twelve cores, when it reported them killed.

   Three consequences, all the head's job:

   - **Put the bound in the ASSIGNMENT, not in a correction.** Tell runners
     to wrap every long search in `timeout N` up front. GNU `timeout`
     returns **124** when it stopped the child itself versus **137** for an
     outside SIGKILL, which is what turns "did my limit fire or did someone
     shoot it?" into a recorded answer (§2b). A runner told this at spawn
     time does not need rescuing later.
   - **"Do not start anything new" is not a sufficient instruction.** The
     head sent exactly that and charlie launched a third search anyway —
     bounded, which was the improvement that mattered. Prohibitions are
     weaker than bounds: say "give it N minutes, then land the report
     whatever the score is", not "stop".
   - **Verify the kill yourself before teardown.** `pgrep -af permuter` plus
     `readlink /proc/<pid>/cwd` attributes each survivor to a worktree; kill
     those by PID. Do NOT `pkill -f decomp-permuter` — that is the round-10
     cross-runner kill this document already forbids, and the head is no
     more exempt from it than a runner.

     **ROUND 26: this check is not a formality, and "I killed it by PID" from
     a runner is not the same as the search being gone.** Charlie captured its
     permuter's root PID, killed it at its bound, and reported it "confirmed
     gone" — accurately, for that PID. But `permuter.py -j 6` runs a
     `multiprocessing` forkserver, and killing the root leaves **six worker
     processes reparented to init** (`ppid=1`), still holding six cores, still
     `cwd`-ed into that worktree. The runner's report was true and the machine
     was still loaded.

     So the head's sweep is what actually ends a search, and the attribution
     step is what makes it safe:

     ```sh
     for p in $(pgrep -f 'decomp-permuter|permuter.py'); do
         printf '%-8s %s\n' "$p" "$(readlink /proc/$p/cwd 2>/dev/null)"
     done
     ```

     Kill only the PIDs whose `cwd` is the worktree you mean — guard the kill
     on the `cwd` rather than trusting the list you printed a moment ago, since
     PIDs are reused. Expect orphans to outlive the runner that started them,
     and sweep before `git worktree remove --force`: those processes are
     `cwd`-ed into the directory you are about to delete.

     **ROUND 27: THAT SWEEP SELF-MATCHES, AND IT REPORTS THE HEAD'S OWN SHELL
     AS AN ORPHANED PERMUTER.** `pgrep -f` matches the whole command line, so
     any process whose command line *mentions* the tool matches — including the
     shell running the sweep itself, whose command line necessarily contains
     the pattern. Measured here: with exactly one real permuter alive, the
     pattern above returned **2** PIDs. The extra was the sweeping shell.

     That is worse than a cosmetic false positive, because the next line tells
     you to kill off this list guarded on `cwd` — and the self-match's `cwd` is
     wherever you ran the sweep. Run it from the main checkout and the `cwd`
     guard saves you by accident; run it after `cd`-ing into the worktree you
     are cleaning and **the guard matches your own shell**.

     **No cleverer pattern fixes it.** Bracketing one alternative
     (`permuter[.]py`) still leaves the other (`decomp-permuter`) matching
     literally, and bracketing both still matches any command line that quotes
     the pattern — which the sweep's own does. The fix is to stop matching
     substrings of a command line and instead (a) require an ARGUMENT that *is*
     a permuter script, and (b) exclude your own process ancestry:

     ```sh
     self=$$; anc=""; p=$self
     while [ -n "$p" ] && [ "$p" != 0 ] && [ "$p" != 1 ]; do
         anc="$anc $p"; p=$(awk '{print $4}' /proc/$p/stat 2>/dev/null); done
     for d in /proc/[0-9]*; do pid=${d#/proc/}
         case " $anc " in *" $pid "*) continue;; esac
         tr '\0' '\n' < "$d/cmdline" 2>/dev/null \
           | grep -qE '(^|/)permuter\.py$|(^|/)decomp-permuter/.*\.py$' || continue
         printf '%-8s %s\n' "$pid" "$(readlink "$d/cwd" 2>/dev/null)"
     done
     ```

     Verified both directions with a real long-running process: this form
     returns exactly the one live permuter and nothing else, and returns
     nothing once it is killed.

     The general shape is the one §2b already names for `pkill`: **a process
     addressed by a global name rather than a path reaches things you did not
     mean.** §2b caught it pointing outward at other runners; this is the same
     error pointing inward at yourself.

   The round-10 rule says a runner's negative permuter result may be
   fabricated by another runner's `pkill`. This is its sibling: a runner's
   permuter result may never arrive at all, and the tell is a summary that
   is a status line rather than a finding.

   **ROUND 18: the bound works, and it only fixes HALF of 2c. Budget for the
   other half.** Every runner was given `timeout` in its assignment, and every
   search terminated on its own — the termination half is solved and stayed
   solved. But four of five runners still ended turns waiting on those bounded
   timers, roughly a dozen times between them, with summaries that were status
   lines ("waiting for the monitor notification") rather than findings. The
   cost was not lost searches; it was that two runners reached ~20 minutes with
   **zero commits**, one of them holding an entire seven-function
   characterisation that existed only in its context window.

   So 2c has two independent failure modes and they need different fixes:

   | failure | fix | status |
   | --- | --- | --- |
   | the search never ends | `timeout` in the ASSIGNMENT | solved, round 17 |
   | the RUNNER stops working while it runs | an ordered work list | round 18 |

   **A prohibition does not fix the second one.** "Do not end your turn to
   wait" was sent to four runners and produced one more turn-ending stop each.
   What worked, immediately and in every case, was an explicit numbered order
   with hand work in it:

   > 1. `git commit` what you have now. 2. Collect the finished search, record
   > its exit code, commit. 3. Launch ONE new search in the background. 4.
   > **While it runs**, read the asm for X and Y, prepare their seeds, update
   > their reports. 5. Only then collect and repeat.

   The generalisable sentence to put in the assignment: **a bounded search is
   BACKGROUND work — it needs cores, not your turn.** There is always hand work
   available (reading disassembly, checking a report's claims against the `.s`,
   preparing the next seed), and none of it needs a search result.

   This is the same lesson round 17 recorded — "prohibitions are weaker than
   bounds" — arriving on a second axis. Round 17 replaced a prohibition with a
   bound; round 18 had to replace a prohibition with a WORK ORDER. In both
   cases the thing that failed was telling a runner what not to do.

   **Corollary the head must also enforce: ONE search at a time.** Three
   runners independently launched 2-3 concurrent searches, peaking the machine
   at load 62 on 32 cores, and one launched two searches on the SAME function.
   Under saturation a second concurrent search does not add throughput, it
   halves both searches' iteration rates — and because the payoff is binary
   (reach zero inside a fixed `timeout` or not), that strictly reduces the
   chance of closing *either*. Sequential beats concurrent, and it is worth
   saying so at spawn time rather than three corrections later.

   **And a contention caveat on every negative:** an iteration count collected
   at a third of a core is much weaker evidence than the same count on an idle
   box. Tell runners up front to phrase such results as "not closed in N
   iterations under load" and never to upgrade one to *permuter-exhausted* —
   that verdict removes a function from `fresh` permanently, and under
   contention it would be wrong.

2d. **A TASK NOTIFICATION MEANS THE AGENT STOPPED ITS TURN, NOT THAT IT DIED.
   Never infer death from one, and never act destructively on that inference.**
   Round 31's head did, and it is the clearest self-inflicted error in this
   document.

   What happened: three runners ended a turn waiting on a bounded permuter
   search — the §2c wait-loop, exactly as predicted. The orchestration
   reported each as a **completed task**. Their worktrees held modified
   reports and zero commits. The head read "task completed, work
   uncommitted" as "runner died", applied the §4c salvage rule, and
   `kill -9`-ed every permuter process in all three worktrees to free cores.

   **All three then resumed on their own and reported normally.** One of them
   (delta) even re-derived the head's salvage measurements independently and
   corrected the head's narrative — its session had never ended.

   The cost was one runner's permuter negative losing its exit attribution
   permanently: charlie had reasoned from elapsed time and a leaked-semaphore
   warning that its `timeout` bound fired rather than an external kill, and
   the head's own sweep had seen those processes **alive eleven seconds
   before the head killed them**. The exit code was never captured, so the
   124-vs-137 question §2b exists to make answerable now has no answer for
   that search. §2b's whole point is that a fabricated negative is expensive;
   this one was fabricated by the head.

   **The kill was correctly `cwd`-guarded and that did not help.** It hit only
   the three worktrees it was aimed at. A `cwd` guard protects against the
   wrong TARGET; it cannot protect against a wrong PREMISE. Round 27 hardened
   the sweep's pattern-matching and round 26 hardened its orphan-finding, and
   both of those improvements were working perfectly while the head killed
   live runners' searches.

   **The discriminator is TIME, not a single observation: a paused runner's
   worktree keeps changing; a dead one's does not.** Before concluding a
   runner is gone, sample twice, minutes apart:

   ```sh
   git -C <wt> status --porcelain | wc -l          # and again, later
   find <wt>/permuter-work -newermt '-3 minutes' | head   # is a search still writing?
   ```

   A worktree whose files are still being written, or a permuter directory
   with fresh mtimes, belongs to a runner that is alive whatever the
   orchestration said. Round 31's evidence was sitting right there: charlie's
   search directory had an mtime in the same minute as the kill.

   **And the salvage rule is not the dangerous half — the KILL is.** §4c
   salvage is non-destructive: reading a worktree, scoring a body in `main`,
   and writing a report costs nothing if the runner turns out to be alive
   (round 31's salvage of `func_80031A44` was useful and delta endorsed it).
   Killing processes is irreversible. So when the two are triggered by the
   same inference, **do the salvage and DEFER the kill** — the cores are worth
   less than the evidence.

   **Corollary: the head may not be ABLE to ask.** Round 31 ran in a session
   where `SendMessage` was unavailable, so the head could neither wake a
   paused runner nor ask it whether it was alive. That is what made "paused"
   and "dead" indistinguishable by the cheap method. If you cannot message
   runners, say so in the PROGRESS entry, and treat every "dead runner"
   judgement as provisional — because the move that would have settled it in
   one message is not available to you.

   One genuine finding did come out of the sweep, and it is worth keeping:
   alpha's searches had **completed normally** on their own 600s bounds ten
   minutes earlier, and nine worker processes were still alive in its
   worktree. That independently confirms round 26's forkserver-orphan
   behaviour and extends it — workers outlive a NORMALLY COMPLETED run, not
   only a killed one. Sweeping for orphans is still right. Sweeping for
   orphans on the assumption that their owner is dead is not.

3c. **Send an early finisher back into its OWN unit** (message the same agent —
   same worktree, same branch, context intact) rather than letting it idle or
   spawning a cold replacement.

   > **Post-round 43 (2026-09-15): `SendMessage` is not exposed in this
   > environment at all, and it will not appear next round. It went missing
   > in rounds 31, 32, 33 and 43 and the four write-ups each reported it as
   > an anomaly. It is the environment. The BROADCAST half of what it did is
   > now `tools/broadcast.sh` — a dated, append-only file in `$MAIN/.round`,
   > symlinked into every worktree by `setup-worktree.sh`, which runners
   > `read` before each function and `post --from <name>` to the moment they
   > have a lever, a negative or a question. The WAKE half (re-sending an
   > idle agent into its own context) has no substitute; the paragraph below
   > is still the remedy for that.**

   **IF `SendMessage` IS UNAVAILABLE, DO NOT SKIP THE RE-SEND — SUBSTITUTE
   FOR IT. Merge the finished runner, fast-forward its worktree onto `main`,
   and spawn a FRESH agent into that same worktree on a DIFFERENT unit.**
   Rounds 31 and 32 both found the channel missing; round 31 concluded it had
   lost the lever and took no closes from it, round 32 substituted and got
   **2 of its 6 matches** from two re-staffed sessions.

   What you lose is the agent's accumulated context, which is real — so this
   is a worse re-send, not an equal one, and it argues for sending the
   replacement to FRESH ground rather than back into the wall the first pass
   hit. What you keep is the whole point of §3c: a provisioned, byte-verified
   worktree and a runner slot that would otherwise sit idle.

   **A THIRD ROUND WITHOUT THE CHANNEL (33), AND A DIFFERENT FAILURE THAT THE
   SAME ABSENCE MAKES UNFIXABLE: A RUNNER THAT STALLS OUT RATHER THAN
   FINISHING.** One of round 33's three runners ended its session mid-work
   having committed NOTHING, waiting for a "monitor" that did not exist. It
   held four modified reports. The §3c substitute does not cover this: there
   is no early finisher to re-send, and spawning a fresh agent into the
   worktree would have DESTROYED the uncommitted work rather than continuing
   it.

   **BUT THE HEAD ALSO MISDIAGNOSED A SECOND RUNNER AS STALLED WHEN IT WAS
   MERELY WAITING, AND THAT ERROR IS THE MORE USEFUL HALF.** Bravo's
   notification arrived with five modified files and zero commits, reading
   exactly like charlie's. It was not dead: it was waiting on a permuter it
   had bounded correctly with `timeout 400`, and when that search ended it
   resumed on its own, wrote its final report and committed. The head had by
   then already committed bravo's in-progress work on its branch — a write
   underneath a LIVE agent. It happened to be harmless (bravo noticed, said
   so in its summary, and carried the head's own round-label correction into
   its last commit), but nothing about the situation guaranteed that.

   **So a task notification is NOT evidence that a runner is finished.** It
   fires whenever an agent stops with no live background children, and an
   agent can resume afterwards. Zero commits plus a notification is therefore
   consistent with BOTH a dead runner and a live one mid-wait, and the two
   call for opposite actions. The discriminator is cheap and structural:

   ```sh
   ps -eo pid=,cmd= | grep -F decomp-permuter | grep -F '<func the runner named>'
   ```

   A bounded job of the runner's own still running means the runner is
   probably alive and will come back — **wait for it.** Nothing running, and a
   summary that names no pending work, means recover. When in doubt, wait:
   recovery is only forced by teardown, and teardown is the head's own choice
   of moment.

   **AND THE CONSEQUENCE FOR §3c ITSELF, WHICH ROUND 33 GOT WRONG: DO NOT
   SPAWN THE REPLACEMENT INTO THE OLD AGENT'S WORKTREE. GIVE IT A FRESH
   WORKTREE NAME.** The substitute written above says to fast-forward the
   finished runner's worktree and spawn a fresh agent into it. That is safe
   only if the previous agent can never resume, and **this harness gives no
   such guarantee** — a notification is not a death certificate, and round 33
   had two agents live in `…-wt-charlie` at the same time, which is the exact
   collision the whole worktree discipline exists to prevent.

   What it cost, and what it did not: the two agents held DIFFERENT units
   (`code_179d8_g` and `class_3bb8c_b`), so they touched disjoint files, all
   three commits landed on one branch and nothing was lost. **That was the
   assignment being disjoint, not the procedure being safe.** Two agents on
   one unit would have interleaved edits to one `.c` under one branch with no
   conflict marker anywhere — a corrupted unit that builds.

   It also cost a real measurement. The head saw permuter processes on
   `func_8002A75C`, reasoned that the session owning that function had been
   merged half an hour earlier, and killed them as orphans. They belonged to
   the still-live original runner, and the kill truncated a 1800s search at
   roughly 1342s. The report's "full 30 minutes" claim had to be corrected in
   place, because a permuter negative's weight IS the extent of its search.

   **So the ordering is:** recover the worktree's uncommitted work only when
   the checks above say the agent is really done; then, if you want a
   replacement, `tools/setup-worktree.sh <a fresh name>` — the grant covers
   five names (alpha, bravo, charlie, delta, echo) and a round using three
   has two spare. Leave the old worktree alone until teardown. A worktree
   costs a second to provision and the whole point of it is that nobody else
   is in it.

   **So the head's move is to recover the worktree itself, BEFORE any
   re-staffing decision:**

   ```sh
   git -C ../<checkout>-wt-<name> status --porcelain      # what is unsaved
   grep -c '^INCLUDE_ASM' ../<checkout>-wt-<name>/src/<unit>.c   # vs the start count
   cd ../<checkout>-wt-<name> && ./build-and-verify.sh; echo "build exit=$?"
   ```

   If `src/` is byte-correct and the build is green, nothing is at risk except
   the reports — commit them on the runner's branch yourself, attribute the
   work to the runner and the COMMIT to the head, and merge normally. Round 33
   did this twice and both branches merged clean. **Teardown is what destroys
   the work, so recover before you tear down, not after** — and that ordering
   is the whole reason this is written next to the re-send rather than in a
   troubleshooting appendix.

   Two things worth fixing in the runner prompt while you are here, both
   measured this round: tell the runner explicitly that **nothing runs on its
   behalf and no notification is coming to it**, and that it must **commit as
   it goes rather than at the end**. Both stalls were a runner waiting
   politely for an event that the harness was never going to deliver.

   **And do not write the wait loop the obvious way.** The head's own attempt
   to wait out one of those searches was
   `until ! pgrep -f "…-wt-bravo.*permuter.py"; do sleep 5; done` — which
   never terminates, because the polling shell's OWN command line contains the
   pattern and `pgrep` matches it. This is the sibling of the existing
   "never `pkill -f` a tool name" rule: **a `pgrep`/`pkill` pattern matches
   every process whose command line contains it, and that includes the
   process doing the matching.** Match on something structural instead — the
   permuter's own `permuter-work/<func>` argument, or a PID you captured — and
   note that the same check is what distinguishes a live search from an
   ORPHAN left by a dead agent. Round 33's really-running processes turned out
   to belong to a session that had been merged half an hour earlier, on a
   function in a different unit than the one the live runner held.

   **And killing a permuter's PARENT orphans its `-j N` workers.** Round 33
   killed the `permuter.py` process and its forkserver by PID; five worker
   processes re-parented to init and kept running for another four minutes,
   on six cores, invisible to a `pgrep` for the parent's own command line.
   The machine's 15-minute load average was **8.76** against a 1-minute
   figure of **0.91** once they were gone — which is the measurement that
   found them, and is worth more than any process listing. So after killing a
   search, confirm the WORKERS are gone (`ps -eo pid=,cmd= | grep -F
   decomp-permuter`, which matches structurally rather than by your own
   pattern), and check `uptime` if a round feels slow: a runner whose builds
   are competing with a dead session's orphans pays for it in wall clock
   without anything in its own transcript explaining why.

   **Carry the round's findings forward IN THE PROMPT — that is what replaces
   the broadcast.** Round 32's replacement runners were handed the three
   mechanisms found earlier that day, and one reported that a finding in its
   prompt drove both of its matches. Ask explicitly for the negative too:
   the same runner reported that the `volatile` lever did NOT apply to its
   unit, which is what stops the next round re-litigating it. This is the highest-yield *structural* move
   available: every derivation is already in that agent's context, and a
   zero-match first pass is not a wasted pass — its reports are what prove a
   stall class has several instances. Hand it the lever explicitly, name the
   exact remaining functions in a cheapest-first order, say which residues are
   expected to survive, and tell it to UPDATE existing reports rather than
   replace them. Merge the first pass anyway; merging twice is free.

   **ROUND 19 MEASURED THIS AND IT IS THE ROUND'S BIGGEST STRUCTURAL RESULT:
   9 of 17 matches came from second, third and fourth passes.** Delta alone ran
   **four** — 5 matches, three overturned verdicts, one mechanically-explained
   negative on the hardest cluster in the project, and it closed the last
   `fresh` function in the executable. Re-sending beat every other lever
   available to the head, including the choice of who to staff in the first
   place.

   Three things that made re-sends pay, all of them the head's work:

   - **Send it somewhere DIFFERENT when the first pass says the ground is
     hard.** Charlie got zero matches on a register-shaped set; its second
     assignment was deliberately non-register-shaped ground in the same units
     and produced 58/196 -> 171/196. Re-sending into the same wall is not the
     move.
   - **A runner whose own units are exhausted can take an UNOWNED unit.** Check
     `headercontention.py` first — delta's third and fourth passes were in unit
     families no live runner held, so they stayed conflict-free. This is also
     how a deliberately-unstaffed hard cluster gets a cheap attempt late in the
     round, once its opportunity cost has dropped to zero.
   - **Say what a good negative looks like, explicitly.** Delta was told before
     starting `code_8220_c` that "the aggregate-assignment axis is a clean
     negative here, and here is why" would be a genuine result. It returned a
     mechanism rather than a shrug. Runners given only a match target report
     status lines when they miss.

   **Reconcile every re-send from the branch, not the summary.** Round 19's
   alpha reported eight matches accurately but silently omitted two assigned
   functions from its disposition — one of them the cheapest open function in
   the corpus. Ask explicitly for the disposition of *every* assigned function
   including "did not reach"; an unlisted function drops out of the next
   round's queue.

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

7. **Commit everything, then PUSH `main`.** Rule 4 says a runner never pushes
   and the head merges; it did not say who pushes, and for twelve rounds
   nothing here did. The head pushes, at the end of the round, after the
   final `./build-and-verify.sh` is green and `git status --porcelain` is
   empty:

   ```sh
   git status --porcelain          # MUST be empty
   ./build-and-verify.sh; echo "build exit=$?"
   git log --oneline origin/main..main | wc -l    # what you are about to push
   git push origin main
   ```

   Do it AFTER teardown's four preconditions, not before — a worktree that
   still holds uncommitted matches (§4c) is work that is not in any commit
   and therefore not in the push. And re-run
   `git reflog show origin/main` first if §4a ever gave you a reason to
   suspect a second head: pushing is the one action in this protocol that
   another session cannot undo for you.

Escalate immediately if: a branch fails to verify after a runner claimed
matches; a runner edited protected files; two runners produced contradictory
learnings.

## Decision gates — what the head does ITSELF

Run these IN ORDER at the start of every round. Never spawn runners into a
state a gate should have fixed first. These are the head's jobs, not
escalations.

**Gate 0 — the checkout must VERIFY and re-extract before you triage.**
Cheap, unconditional, and round 13 found it the hard way: a `git pull` that
brings in another head's carves does **not** re-extract `asm/`. `asm/` is
gitignored, so it is per-checkout state that nothing in git describes, and the
newly carved units have no `asm/nonmatchings/<unit>/` at all. Two things then
happen, and only the first is loud:

- `./build-and-verify.sh` goes RED with
  `can't open asm/nonmatchings/<unit>/<func>.s for reading`. Reads exactly like
  somebody broke the tree, and is not attributable to any commit.
- **`progress.py` silently UNDER-REPORTS.** It prints a stale-asm WARNING
  naming the leftover monolithic `asm/*.s` files, and then prints a table
  computed as if the un-extracted units did not exist. Round 13 measured
  **581** uncarved game functions before re-extracting and **742** after, with
  the matched percentages wrong in the same direction. Every triage decision
  reads off that table.

The warning is the tell, and it prints ABOVE the table — so it is exactly what
scrolls away if you pipe `progress.py` through `tail`. Do this first, every
round, before believing any number:

```sh
python3 tools/progress.py            # read the TOP of the output, not just the table
rm -f asm/<each stale monolith it named>.s
make extract && ./build-and-verify.sh ; echo "build exit=$?"
```

`make extract` is one of the four targets the build hook permits, and a correct
re-extraction changes zero committed bytes. If the build is not green after
this, stop and diagnose — do not triage, do not carve, do not spawn.

**The stale monolith is PER-WORKTREE, so cleaning `main` does not clean the
runners.** `asm/` is gitignored and each worktree extracts its own. When a
runner fast-forwards onto a carve and runs `make extract`, splat leaves the
old top-level `asm/<segment>.s` behind in THAT worktree, and `progress.py`
warns there even though `main` is clean — round 13's delta reported exactly
this and was right about its own tree while `main` had already been fixed.
It is harmless to the counts and to the build, so the correct handling is to
tell the runner it is expected rather than to send it into `asm/`, which is
off-limits to runners anyway.

This is the same decay shape as the `settings.local.json` lesson above: **a
doc or a tool claim about PER-CHECKOUT state is nobody's job to re-measure**,
so it survives being wrong. The generalisation for the gates: `progress.py` is
an oracle about the REPOSITORY, not about your working tree, and it will
answer confidently either way.

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
- every report whose stall verdict has since been INVALIDATED — most often
  because the blocker it blamed was resolved — MUST say **`REOPENED --
  ASSIGNABLE`** at the start of a line. `progress.py` keys on that exact phrase
  (round 22) to stop counting the function as a documented stall and return it
  to `fresh`, without deleting a derivation that is still worth reading.

  **This is the third honesty mechanism and it closes the one direction the
  other two could not.** A report is written while a blocker is live; the
  blocker gets resolved; the report outlives it and keeps the function out of
  `fresh` forever, because a function everybody believes is blocked is one
  nobody re-measures. That is the *expensive* direction of error this document
  already names ("a false blocker silently deletes matchable ground from every
  future round") — it just had no way to be undone. Round 21 resolved
  `addiu_at` and annotated five live reports with "BLOCKER RESOLVED — THIS
  FUNCTION IS NOW ASSIGNABLE" in prose no tool could read; all five were
  invisible to `fresh` until round 22 marked them.

  So when you resolve or retire a blocker, marking its reports is part of the
  same job, not a follow-up. `gp_rel` alone currently has 82 of them.

  **ROUND 23 MEASURED THE MECHANISM AND IT WORKS — 3 of the 5 marked
  functions matched, all three by the head, in one sitting.** `func_8003C48C`
  (36 words), `func_8003C63C` (100 words) and `func_80049EB4` (107 words) were
  all filed as `addiu_at`-blocked and never attempted; all three closed
  byte-exact. A fourth (`func_8004109C`) went from "NOT ATTEMPTED, predicted
  register saturation" to a 42/56 with its body fully derived, and the fifth
  (`func_80018464`, 954 words) was left as too large for a head sitting. So the
  marker returned real ground, not paperwork: **the reopened set is the
  cheapest queue in the corpus and it should be worked before cold ground.**

  **BUT MARKING IS NOT MECHANICAL — RE-SCREEN AGAINST THE ASM BEFORE YOU
  REOPEN A REPORT.** Round 23 found `func_8005D864` (`Entity`) filed as
  blocked with the cause attributed **entirely** to `addiu_at`. On the report's
  text it reads as newly assignable. It is not: re-screened, it hits
  `nop_mflo_mfhi` (`mflo $v0` / `nop` / `div $zero, $a0, $v0`), which is still
  open. Its VERDICT was right and its CAUSE was wrong, and a mechanical sweep
  of "reports naming a resolved blocker" would have staffed a runner into a
  real wall.

  This is CLAUDE.md's wrong-CAUSE hazard arriving from the other direction. The
  usual failure is a report blaming a blocker for a residue that is really
  ordinary code; this one blames the WRONG blocker while being genuinely
  blocked. Both are fixed the same way: **run `python3 tools/nearmiss.py --all`
  and read the tag.** It already prints
  `[addiu_at(RESOLVED-not-a-blocker),nop_mflo_mfhi]` for exactly this function
  — the marker exists so you can tell "this report's verdict predates the fix"
  from "this is really blocked", and the two are not the same claim.

  A runner will inherit the error rather than make it: round 23's charlie,
  which matched all five of its assigned functions, signed off calling this
  "the still-`addiu_at`-blocked function" — read off the report, not measured.
  Correcting the report is the head's job, and it is worth doing at once,
  because the next reader has no way to know the text is stale.

  **AND SWEEP THE OTHER DIRECTION TOO, because round 22's sweep missed a
  99/100 near-miss.** Round 22 marked five `addiu_at` reports and stopped
  there — they were all stubs. `func_8005CBC8` (`code_4cd08`) was missed, and
  it is the most valuable function in the queue: with the blocker gone it
  measures **99 of 100 words**, the closest open near-miss in the corpus.
  It was missed because it does not LOOK like a stub. It is a long,
  twice-corrected report with a real attempt history whose verdict sentence
  — "cannot be matched as C under the pinned toolchain regardless of source
  shape" — reads as a considered plateau rather than as a blocker citation.

  **A blocker's death invalidates every report that RELIED on it, not only the
  ones that look like stubs.** And there is a mechanical detector, so this does
  not need judgement: **`nearmiss.py` screens from the ASM, so a function it
  lists as blocker-CLEAN whose report calls it blocked is a contradiction, and
  the report is the wrong half.** Run this at Gate 1 whenever a blocker has
  been retired:

  ```sh
  python3 tools/nearmiss.py | awk '/ASSIGN FROM HERE/,0' \
    | grep -oP '^\s+\d+w\s+\S+\s+\K\S+' > /tmp/clean.txt
  while read fn; do f=docs/match-reports/$fn.md; [ -f "$f" ] || continue
      grep -q 'REOPENED -- ASSIGNABLE' "$f" && continue
      grep -qiE 'addiu_at|addiu-\$at' "$f" && grep -qiE 'BLOCKED|blocker' "$f" \
        && echo "$fn"
  done < /tmp/clean.txt
  ```

  It over-reports — a report saying "blocker screen clean, no `addiu_at`"
  matches too — so read each hit's TITLE/verdict line, which is the only safe
  place to read a figure from anyway. Round 23 got 24 hits and exactly one was
  a genuine stale verdict; the other 23 mention the screen in passing. **24
  titles to read is a cheap price for a 99/100 function.**

  **ROUND 24: THE DETECTOR UNDER-REPORTS, AND WHAT ACTUALLY FOUND THE CHEAPEST
  GROUND WAS A SHARED PREAMBLE ACROSS A CLASS FAMILY.** Round 24 ran this
  contradiction sweep and got 7 hits. One of them, `Entity__GetEventVideo`
  (**9 words** -- the smallest function in the queue), was a pure `addiu_at`
  residue and matched first attempt with the C its own report had derived.
  **Its two siblings, `Entity__GetUnlockEffect` (14w) and
  `Entity__GetLinkStage` (15w), did NOT appear in the sweep's output at all**
  -- their titles do not match the `^#.*BLOCK|Status:.*BLOCK` shape the grep
  keys on. They were found by noticing that all three reports open with the
  *same* **HEAD ADJUDICATION** box, and checking the whole family. All three
  matched, 38 words for one sitting.

  So: **when a contradiction hit turns out to be real, read every report that
  shares its preamble, its unit, or its adjudicating round.** Blocked-function
  reports are written in families -- one runner, one sitting, one shared
  cause -- and a detector keyed on title text will find some members and miss
  others. The family is the unit of staleness, not the report.

  **And the hardest-to-see case is a report that is TOO well argued.** These
  three survived round 22's sweep and round 23's follow-up because they open
  by certifying that the head reproduced the diagnosis independently from
  scratch and wrote it up project-wide with a 502-of-502 corpus census.
  Nothing reads less like stale ground. Every word of it was also true: the
  adjudication verified the MECHANISM, which never changed. What expired was
  the premise that the mechanism was unfixable -- a premise the report states
  explicitly ("maspsx exposes no `--addiu-at` flag"), and which round 21
  falsified by adding exactly that flag.

  **A blocker's death invalidates the strongest reports as thoroughly as the
  weakest, and does so without touching a word of what they say.** Rank from
  `nearmiss.py` and treat any contradiction with a report as the report being
  the wrong half -- however good the argument, and *especially* when it is
  good enough that nobody re-reads it.

  One more round-24 refinement, on the other side: a report can have a dead
  CAUSE and a live CONCLUSION, and the honest annotation says which half it
  is correcting. `func_80059814`'s blocking residue was `addiu_at`; retiring
  it moved the function 8/53 -> 14/53 and left it 4 words short on an
  unrelated branch. Its companion `func_800598E8` defers its whole analysis
  to that report, so its cause is equally dead -- but it was deliberately NOT
  marked `REOPENED -- ASSIGNABLE`, because the sibling had just demonstrated
  that the blocker was worth 6 words of ~45. Marking it reopened would send a
  cold runner in expecting a free function.

  Note the correct disposition for such a function is a CORRECTED verdict, not
  a `REOPENED` marker: it is genuinely worked ground with a characterised
  residue, and `progress.py` should keep counting it as a documented stall
  rather than returning it to `fresh` for a cold runner to re-derive.

  **Also sweep `src/*.c` HEADER COMMENTS, which no tool can see and every
  runner assigned to that unit reads.** Round 23 matched `func_800545FC`
  (`class_3bb8c_m`, 25/25, first attempt) whose carve-time unit comment listed
  it as `addiu-$at` blocked and ended *"All four have stub reports; do not
  attempt them."* Three of that comment's four lines were still correct, which
  is exactly why nobody re-read it — and a stale line in a comment is not
  merely wrong, it is a DIRECTIVE. `grep -rn 'addiu' src/*.c` and read every
  hit framed as a live blocker; round 23 found and fixed four such units
  (`class_3bb8c_m`, `code_179d8_c`, plus `code_179d8_i` and `code_179d8_j`
  whose comments still claim 9 and 13 `addiu_at`-blocked functions
  respectively — those two are the NEXT round's first job, and the functions
  they name may be free).

  Record the method next to the verdict when you write one, or the verdict
  outlives its method: the corrected comments now name the screen (two greps
  as of round 21), the date, and `tools/nearmiss.py`.

> **Round 42 (2026-09-15): there are ZERO blocker greps.** `gp_rel` and
> `nop_mflo_mfhi` are RESOLVED the way `addiu_at` was — maspsx flags
> `--gp-symbols` and `--no-nop-mflo-mfhi` in `tools/patches/maspsx-lsd-flags.patch`,
> byte-exact whole image, blocked functions matching on the first build (see
> CLAUDE.md "Open toolchain blockers"). `nearmiss.py` reports all three
> constructs tagged `(RESOLVED-not-a-blocker)` and counts none. The 89 stall
> reports that blamed them carry a `REOPENED -- ASSIGNABLE` box and are
> `fresh` in `progress.py`; their derivations are often right and their
> verdicts are not. The paragraph below is kept for the FORWARD/backward
> lesson, which still applies to reading the tag.

**Screen the queue with TWO greps as of round 21 — it used to be three.**
`addiu $at, $at, %lo` is NO LONGER A BLOCKER (resolved round 21; see
`docs/research/addiu-at-blocker.md`), so screening for it now invents
blockers, which is the strictly worse failure described below. The two live
ones are `gp_rel` and `nop_mflo_mfhi`. **Prefer `python3 tools/nearmiss.py`,
which runs both correctly and reports the resolved construct without counting
it.** The `nop_mflo_mfhi` form, which round 13's head missed at Gate 1 and
assigned a blocked function because of it:

```sh
for f in $(grep -oP 'INCLUDE_ASM\("[^"]*", \K\w+' src/<unit>.c); do
    grep -A2 -nE '\b(mflo|mfhi)\b' asm/nonmatchings/<unit>/$f.s \
      | grep -qE '\b(mult|multu|div|divu)\b' && echo "BLOCKED (nop_mflo_mfhi): $f"
done
```

An `mflo`/`mfhi` **FOLLOWED WITHIN TWO INSTRUCTIONS BY** a
`mult`/`multu`/`div`/`divu` is blocked exactly like `addiu_at`: the pinned
pipeline inserts `nop`s retail does not have.

**The insertion is exactly TWO nops, and that number is what makes the
two-instruction window right rather than approximate.** Measured round 24
through the pinned pipeline, which is the only way this should ever be
settled:

```c
int probe(int a, int b, int c) { int q = a / b; return q * c; }
```

comes out as `mflo $a0` / `nop` / `nop` / `mult $a0, $a2`. So retail needs
**two** filler slots between the result read and the next multiply to be
reproducible, and a `mult`/`div` landing within two instructions of the
`mflo`/`mfhi` is precisely the case where it has fewer.

**This sentence used to end "with no `nop` between them in retail's own
bytes", and that qualifier is FALSE — it cost round 24's head a wrong
conclusion it nearly acted on.** Reading it literally, a retail sequence of
`mflo` / `nop` / `mult` has "a nop between them" and so reads as NOT blocked;
in fact the pipeline emits TWO nops there, so retail's single nop is still one
word short and the function is blocked like any other. The head "refined" the
canonical grep with that nop test, and it declared two genuinely-blocked
functions assignable — including `func_8005D864`, the function round 23 had
just carefully re-adjudicated onto this very blocker. One reproducer settled
it in under a second and the refinement was discarded.

Two lessons, and the second is the general one:

- **The canonical grep is not a cheap pre-filter for a subtler test. It IS
  the discriminator**, and adding a condition to it makes it wrong. This is
  the fourth recorded attempt to improve this screen and the fourth to break
  it — the previous three inverted the window direction (rounds 15, 16), this
  one added a false qualifier. The screen has now failed in both directions
  it can fail in.
- The error direction differs from the classic one and is worth naming: a
  false BLOCKER deletes matchable ground permanently, while this was a false
  CLEARANCE, which staffs a runner into a real wall and burns an attempt
  budget. Cheaper, but still paid by somebody who did nothing wrong. **Prose
  in a doc is not a specification of a toolchain behaviour; the reproducer
  is.** When a rule and a mechanism seem to disagree, run the pipeline
  (§"Escalate, do not experiment" in CLAUDE.md) rather than reasoning from
  the wording.

**RUN THAT COMMAND. DO NOT RE-IMPLEMENT IT.** The direction is load-bearing
and the shell form is right *by construction*, because `grep -A2` prints the
two FOLLOWING lines. Every reimplementation so far has inverted it:

- Round 15's head rewrote it in Python as "an `mflo`/`mfhi` within two
  instructions **after** a `mult`/`div`" and got four false blockers,
  including a 252/258 near-miss.
- **Round 16's head did it again**, in a Python window census at carve time —
  having read that entry — and wrote two stub reports for functions that were
  never blocked. Measured over the 239 functions of the old `code_179d8`
  monolith: **4 correct forward hits versus 14 inverted, of which 4 are pure
  false blockers.** One of the two wrongly-stubbed functions was matched at
  **65/65** by a runner within the hour of the correction.

The two directions are opposite in effect, reproduced in isolation (see
`docs/DECOMPILATION_LEARNINGS.md`, "the `nop_mflo_mfhi` screen runs FORWARD"):
`mult` -> `sra` -> `mfhi` is retail's **signed-divide-by-constant idiom** and
reproduces exactly with no `nop` inserted, so it is ordinary matchable code
however tight the gap looks. Only the result-read-then-multiply direction is
blocked.

**Why this specific error keeps recurring, and what to do about it.** The
inverted form is not obviously wrong when you read it — it looks like a
hazard-slot check, which is a real thing on MIPS, just not this thing. And it
fails in the expensive direction: it *invents* blockers, and a false blocker
becomes a stub report, which `progress.py` counts as a documented stall, which
removes the function from `fresh` **permanently**. Nobody re-measures a
function everyone believes is blocked. So: paste the shell form, and if you
must screen in bulk, shell out to it rather than re-expressing the window.

A false blocker is strictly worse than a missed one — a missed blocker costs a
runner some attempts and produces a report; a false blocker silently deletes
matchable ground from every future round.

**This grep belongs to the HEAD and not to the runners, deliberately.**
`addiu-at-blocker.md` measured the scope twice and both times declined to
promote it to a per-runner standing screen --- at 4 of 156 queued functions it
does not earn every runner grepping every function. Run once per round over the
queue it is seconds, and it catches the same thing. Do not "helpfully" move it
into CLAUDE.md's per-function screen; that placement was considered and
rejected on measured grounds, and the reasons are in that document.

Why it matters more than 4-of-156 suggests: round 11 assumed the 30-attempt cap
bounds the cost of walking into this blocker cheaply. It does not. Round 13's
bravo spent a full derivation on `func_8001CEB4` --- field reads, a dispatch
structure, and a `/360` division verified against the pinned `cc1` --- before
the construct surfaced at all. A blocker that only shows up after you have
understood the function is not bounded by an attempt counter.

`fresh` is a ceiling, not a work order: it cannot see "large body, deep
reconstruction, low cold-runner yield". Before believing a number, size the
queue (`wc -l asm/nonmatchings/<unit>/*.s | sort -rn`) and read the match
reports for the functions you intend to assign — see "Unit and segment state"
in MATCHING-GUIDE.

**Then price the assignment's header contention, before provisioning:**

```sh
python3 tools/headercontention.py <the units you intend to assign>
```

`fresh` is blind to this the same way it is blind to toolchain blockers. Two
units can both be full of clean fresh ground and still be a poor pair to staff
together, because their runners will both edit one header and the head pays at
merge time. Collision rule 1 has the measured three-round history and the
mitigations; the point here is that **this is a Gate 1 decision, not a
merge-time surprise** — it is the cheapest possible moment to act on it, and
acting on it costs one command.

**Gate 2 — carve to refill.** If fresh-assignable functions are fewer than
roughly (runners × per-runner target), carve new units BEFORE provisioning.

**"Most of the game is still uncarved, so this gate fires early and often" was
true for twenty-odd rounds and IS NO LONGER TRUE. Measure before you believe
it — the tool is `tools/uncarved.py` and it exists because this gate's own
one-liner answers the wrong question:**

```sh
python3 tools/uncarved.py                 # per-segment, screened, best first
python3 tools/uncarved.py --functions     # every function with its screens
python3 tools/uncarved.py --windows 20    # clean-density windows, for boundaries
```

`grep -c '^glabel'` counts FUNCTIONS. What Gate 2 needs is WORKABLE functions,
and on this corpus those diverged a long time ago: `class_3bb8c_h` counted 17
and held 4. `uncarved.py` runs all FOUR screens per function (`gp_rel`,
`nop_mflo_mfhi` via the canonical FORWARD grep, BIOS trampolines, and
`addiu_at` reported-but-not-counted) and ranks segments by clean yield. It is
the Gate 2 companion to `nearmiss.py` and was built the same way, cross-checked
against a hand census that agreed exactly.

> **Round 42 (2026-09-15) REVERSES the paragraph below.** The toolchain change
> it said would be needed has happened: `gp_rel` no longer blocks, and
> `uncarved.py` measures every uncarved game function as blocker-clean. Gate 2
> is a live source of work again; price a carve on its own merits.

> **ROUND 43 (2026-09-15) REVERSES ROUND 27's STANDING VERDICT BELOW.** That
> paragraph names its own escape clause -- *"the SHAPE, which will not reverse
> without a toolchain change"* -- and round 42 was exactly that toolchain
> change. Re-measured on 2026-09-15 with `uncarved.py`: **66 uncarved game
> functions in 4 segments, 66 of 66 blocker-clean, and ZERO BIOS trampolines
> in any of them.** The front 20-function window of BOTH large segments is
> 20/20 clean (`code_179d8` 840w, `class_3bb8c_n` 1112w) -- where round 27
> measured 10 clean out of 79 and the old window table had `code_179d8`'s
> front at 6/20.
>
> So the two specific warnings below are now FALSE and must not be acted on:
> "expect a carve to yield single-digit functions" (it yields ~20) and "the
> clean remainder is scattered a few functions at a time across segments that
> are otherwise blocked" (it is contiguous and complete). `gp_rel` was the
> binding constraint on carveable ground and it is gone.
>
> What still stands from round 27 is the part that was never about blockers:
> **price the carve against Gate 1b's near-miss corpus rather than carving
> reflexively**, and **prefer a carve that also corrects something**. Round 43
> itself declined to carve for that reason -- `fresh` was 86, which already
> exceeded five runners' capacity, so carving would have bought ground nobody
> could staff.

> **ROUND 45: WHEN A BLOCKER DIES, SWEEP THE CARVE NOTES TOO -- THE REPORT
> SWEEP DOES NOT REACH THEM.** Gate 1's honesty mechanisms
> (`REOPENED -- ASSIGNABLE` and the contradiction detector) operate on match
> REPORTS. A yaml carve note is neither a report nor a `src/` header comment,
> so nothing was checking it, and round 45 found both remaining uncarved
> segments priced under a blocker that died in round 42:
>
> | segment | the note | measured 2026-09-15 |
> | --- | --- | --- |
> | `code_179d8` | 33 of 39 `gp_rel`-blocked; *"budget 18 STUB REPORTS"* | 39 of 39 clean, stub debt **zero** |
> | `class_3bb8c_n` | *"the gp_rel-densest ground in the executable"*, 3 of 23 clean, **"Not worth a runner until the gp-relative blocker moves"** | **23 of 23 clean** |
>
> `class_3bb8c_n`'s is the one to learn from, because **it names its own expiry
> condition and was still obeyed three rounds after the condition was met.**
> Its round-26 census was honest, careful, and about a world that no longer
> exists -- and a segment everybody believes is blocked is one nobody
> re-measures, which is this document's own most expensive failure mode
> arriving at Gate 2 instead of Gate 1.
>
> Two consequences, both cheap:
>
> - **`uncarved.py` is the authority on a segment's blocker state, not the
>   yaml comment next to it.** Run it before reading any carve note. The note
>   is provenance; the tool is the measurement.
> - **A carve note that ends in a DIRECTIVE ("not worth a runner until X")
>   must be re-read whenever X changes, and the cheapest way to retire one is
>   to CARVE the segment** -- which converts a standing instruction nobody
>   re-reads into a unit header a runner will actually be handed. Round 45
>   carved `class_3bb8c_n` and banked it for exactly that reason.

**Round 27's measurement, and it is a STANDING change to what this gate can
do, not a snapshot: carving can no longer refill the queue at scale.** The
whole uncarved remainder came to 79 functions holding **10 blocker-clean** ones
— 55 `gp_rel`, 13 BIOS trampolines that no C compiles to, 1 `nop_mflo_mfhi`.
Do not read those figures as current; re-run the tool. Read the SHAPE, which
will not reverse without a toolchain change: **`gp_rel` is now the binding
constraint on carveable ground, and the clean remainder is scattered a few
functions at a time across segments that are otherwise blocked.** So:

- **A thin `fresh` queue is no longer sufficient reason to carve.** Price the
  carve against Gate 1b's near-miss corpus, which is far larger and already
  screened. Round 27 had `fresh` at 7 and correctly ran runners rather than
  carving, because the queue that mattered was 135 blocker-clean near-misses.
- **Expect a carve to yield single-digit functions and to cost stub reports for
  the blocked majority.** `code_179d8`'s worked-out next slice is 4 clean
  functions against 18 mandatory stubs. That can still be worth taking — but it
  is a different trade from the ~20-clean-function carves of rounds 14-16, and
  budgeting for the old one will overrun.
- **Prefer a carve that also corrects something.** Round 27's took
  `class_3bb8c_h` — 4 functions — specifically because its round-17 "avoid this"
  verdict had gone stale when `addiu_at` was resolved, so the carve retired a
  false directive as well as adding ground.

The `psyq_*` segments are Sony SDK code: excluded from the game-code
denominator, and left for last. Matching them proves nothing about this game.
`uncarved.py` excludes them for you.

If you still want the raw function count (it is the right tool for "what
segments exist right now", which is a different question):

```sh
for f in asm/*.s; do b=$(basename "$f" .s); case "$b" in psyq_*|header) continue;; esac
    printf '%6d %s\n' "$(grep -c '^glabel' "$f")" "$b"; done | sort -rn
```

1. Pick a contiguous run of uncarved functions, ~20 per unit — but pick it by
   **blocker density, not by "next"**. Blockers CLUSTER, so the aggregate rate
   for a monolith is a bad guide to any particular slice of it, and the front
   of a monolith is not a neutral place to cut. Measured 2026-09-04 over the
   two remaining uncarved game monoliths, in non-overlapping 20-function
   windows:

   ```
   code_179d8 (267 funcs, 44% blocked overall)
     [  0.. 19]  6/20 clean      <- carving "the next 20" lands HERE
     [ 20.. 39]  2/20 clean
     [ 40.. 59] 15/20 clean
     [ 60.. 79] 16/20 clean
     [100..119] 20/20 clean      <- and this exists in the same segment
     [140..159]  5/20 clean
   class_3bb8c_n (113 funcs, 27% blocked overall)
     [  0.. 19]  1/20 clean      <- carving "the next 20" lands HERE
     [ 60.. 79] 19/20 clean
     [ 80.. 99] 18/20 clean
   ```

   A 1-of-20 slice is a unit that cannot staff a runner, and it would have
   been chosen by following the old wording literally. **`code_179d8`'s "43%
   blocked" note in the splat yaml is true in aggregate and misleading as a
   carve decision** — its clean windows are as good as anything round 15
   carved. Run the three-grep screen across windows before choosing the
   boundary; it is seconds, and it is the difference between a unit worth a
   runner and one worth none.

   **STALE AS OF ROUND 16 — the `code_179d8` half of that table no longer
   addresses anything, and this is the doc's own "do not work from a written
   list" lesson landing on the doc itself.** Round 16 carved three units out
   of that monolith (`code_179d8_b`, `_c`, `_d`), so it is now five segments
   and those window indices index nothing. Two further reasons not to
   reconcile against it: the table counts **267** functions where a round-16
   `grep -c '^glabel'` counted **274**, so the windows were never aligned
   with anyone else's; and a re-census cannot reproduce it now, because 35 of
   those functions have been matched and no longer have a `.s` at all.

   The per-slice censuses that ARE current live in the splat yaml next to each
   carve, with their method and their caveats attached. Re-derive from the
   live segment list rather than from any table, here or there — which is
   what the paragraph above this one already tells you to do.

   The transferable point, since this is the third written list in this
   document to go stale by being acted on: **a census is evidence about a
   segmentation, and carving changes the segmentation.** Record a census
   where the thing it describes lives (the yaml entry for that carve), not in
   a guide that outlives it.

   Carving from the MIDDLE is fine and costs one extra segment: split the
   monolith into `[.. before] asm`, `[the slice] c`, `[after ..] asm`. Name
   the two remainders so the next head can tell them apart.
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

     **And run it BEFORE you believe the blocker screen, because the screen
     is blind to this class and fails in the flattering direction.** A
     trampoline has no `gp_rel`, no `addiu $at`, and no `mflo`/`mfhi`, so
     the three greps all pass it — and a trampoline-dense segment therefore
     reads as the CLEANEST ground left in the executable while being the
     least matchable. Measured round 17, at the point where every good
     window had been carved out: `class_3bb8c_h` screened **15 of 17 clean**,
     the best figure of any remaining segment, and was in fact **13 BIOS
     trampolines plus two addiu-$at functions — two workable functions out
     of seventeen.** A head triaging on the screen alone would have carved
     it first and staffed a runner into a wall.

     So the trampoline grep is not only a carve-time disposition chore, it
     is the fourth screen, and it belongs next to the other three whenever
     you are ranking segments rather than preparing one:

     ```sh
     printf 'trampolines: %s of %s\n' \
       "$(grep -c 'jr *\$t2' asm/<segment>.s)" \
       "$(grep -c '^glabel' asm/<segment>.s)"
     ```

     The general shape, which is the same one this document keeps
     rediscovering: **a screen measures the obstruction it was built for,
     and says nothing about the ones it was not.** The three-grep screen
     answers "can the pinned pipeline emit these bytes from C"; it does not
     answer "is there any C here at all".
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

   **Do NOT let the unit's header comment imply one class per unit.** A
   ~20-function slice cut at ROM-address boundaries has no reason to align
   with class boundaries, and round 15 measured this: **three of five units
   spanned two or more vtables**, and all three runners reported it as an
   anomaly because the carve comment had led them to expect one.

   - `class_3bb8c_j`: `Obj866E8` for the first functions, then a separate
     `BasicClass`-derived sibling from `func_80051A4C` on.
   - `class_3bb8c_k`: `D_80086F88` (its primary class) plus the ctor/dtor
     and first methods of `D_80087034` landing in its tail.
   - `class_3bb8c_l` / `class_3bb8c_m`: both reached `D_80087034`
     independently, from opposite ends.

   Say "expect this slice to span more than one class; identify each with
   `tools/classtable.py` rather than assuming the unit has one" in the
   carve comment. It costs a line and it converts three "anomalous" reports
   into ordinary expected work. A class that spans a carve boundary is also
   the normal reason two runners independently name the same table — see
   the shared-header rule above.
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

     **`0xFD8` (`code_179d8`)** is a standalone slot that genuinely does hold
     text pointers and will need attaching when its segment is carved: of its
     483 words, 179 are vram addresses inside `code_179d8`'s own text (the
     first at `0x1010 -> 0x80027A8C`) and 283 are ASCII.

     **`0xA8C` (`code_8220`) is NOT such a slot, and this document previously
     said it was** — listing it alongside `0xFD8` as "confirmed against the
     data". Read out of the executable, all 12 of its words are ASCII and NONE
     is a vram address: the slot is one printf format string,
     `bMemPMgr = %p, poolSize = %ld in BMemPMgrInit`. `code_8220` was carved in
     round 11 with the slot left standalone and the link came up green, which
     is the check that settles it. A string is referenced by SYMBOL, and a
     standalone rodata object resolves that fine; only `.L` labels local to a
     unit's function `.s` files force an attach.

     The general point, since the wrong entry cost nothing only because it was
     re-measured: **"confirmed against the data" in a doc is a claim, not the
     data.** Both slots take one script to survey — count how many words fall
     in `0x80010000..0x8008B800` — so survey the slot at carve time rather
     than inheriting a verdict about it.
   - **A rodata slot can be SHARED between segments, and attaching it whole
     breaks the other one.** This is the sub-case that turns the two failures
     above into a three-step dance, found in round 14 carving `code_2cc8c_e`:

     1. Left standalone, the link fails with
        `undefined reference to '.L8003FB98'` — the slot's words are `.L`
        labels local to a function's own `.s`, so you attach it:
        `- [0x1908, .rodata, code_2cc8c_e]`.
     2. That fixes the labels and immediately breaks
        `undefined reference to 'D_800111B4'` instead — because the slot
        ALSO held two symbols referenced only from `asm/psyq_memset.s`, a
        different segment. With `migrate_rodata_to_functions: True`, an
        attached slot's symbols migrate into the OWNING unit's functions, and
        a symbol referenced only from elsewhere has nowhere to migrate to and
        drops out of the link entirely.
     3. So SPLIT the slot at the ownership boundary — one entry attached, one
        left standalone:

        ```yaml
        - [0x1908, .rodata, code_2cc8c_e]
        - [0x19B4, rodata]   # -> psyq_memset, NOT ours
        ```

     **Find the boundary by asking who references each symbol**, which is one
     grep per symbol and settles it:

     ```sh
     grep -n 'dlabel\|jtbl' asm/data/<slot>.rodata.s          # what's in there
     grep -rl '<symbol>' asm/*.s asm/nonmatchings/*/*.s src/*.c | grep -v asm/data
     ```

     Two things about this that are easy to get backwards:

     - **The attach is required even when the function owning the jump table
       stays `INCLUDE_ASM`.** Being `INCLUDE_ASM` assembles the text; it does
       not embed the table. Round 13's `func_8005CBC8` note reads as though
       reverting to `INCLUDE_ASM` keeps the table with the function — it does
       not, and `code_4cd08` needed its `0x206C` attach for exactly this
       reason.
     - **The error moving from `.L…` to `D_…` is progress, not a new
       problem.** It means step 1 was right and the slot simply has two
       owners. Do not undo the attach.

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

**Gate 1b — the near-miss corpus is a QUEUE, and it is screened from the
ASM, not from the report text.** **Round 20 turned this gate into a tool —
run it instead of rebuilding it by hand:**

```sh
python3 tools/nearmiss.py            # blocker-clean queue, smallest first
python3 tools/nearmiss.py --all      # plus the blocked ones, with their class
```

It walks the live `INCLUDE_ASM` symbols, runs all FOUR screens, and prints
each function's report **title/verdict region verbatim**. It exists because
this gate has two documented recurring failure modes and both are mechanical:
heads re-implement the `nop_mflo_mfhi` window backwards (rounds 15 and 16), and
heads parse a score out of a report BODY (rounds 18, 19 and 20). The tool
shells out to the canonical grep forms rather than re-expressing them, and it
never extracts a figure — it hands you the text and leaves the ranking to you.

Round 20 built the same census by hand first and the two agreed exactly, which
is the check that says the tool is faithful rather than merely convenient.
Re-run that comparison if you change it.
 `progress.py` counts every documented stall
identically, so the difference between a 104/105 near-miss and a
gp_rel-blocked function is invisible in its table. Build the list yourself:
walk the live `INCLUDE_ASM` symbols, read each one's report, drop the ones
whose asm hits a blocker grep, and rank what is left by best recorded score.
Round 17 found **72** non-blocker scored near-misses this way, only 4 of them
already permuter-exhausted — a bigger and better-posed queue than the cold
ground remaining in the whole executable.

**AND SCREEN THEM FOR SONY OWNERSHIP, which no blocker grep can see (round
32).** Fourteen functions in this queue — 1669 words, ~4100 lines of
accumulated derivation between them — lay FULLY inside Psy-Q objects already
placed and verified against retail. They are not blocked; they are not game
code. No source shape reaches a match, so every attempt on one is spent for
certain.

```sh
python3 tools/sdkstalls.py           # stall queue crossed against placed objects
```

`nearmiss.py` now runs this itself and excludes the hits from `ASSIGN FROM
HERE`, so an assignment taken straight off that list is safe. Run the tool
directly when you CARVE, or when you are adjudicating one report's verdict
rather than picking from the ranked list.

**Why this class is worth its own screen: it passes all the others, and in the
flattering direction.** An SDK-owned function has no `gp_rel`, no `mflo`/`mfhi`
hazard and no `jr $t2` trampoline, so it reads as the cleanest ground in the
queue. That is precisely the BIOS-trampoline finding from Gate 2 (`class_3bb8c_h`
screening 15-of-17 clean while holding two workable functions), arriving in
Gate 1b — and the general shape this document keeps rediscovering: **a screen
measures the obstruction it was built for, and says nothing about the ones it
was not.**

The rule itself was never missing. CLAUDE.md has said "never write C for a
function a Sony object owns" since round 28 and named the command;
`psyq_sdk.py coverage` has been printing the overlap under a heading that
literally reads "SDK code miscounted as game". What was missing was that
checking meant reading a 60-line object list against a 200-line queue by hand,
per round, and noticing an overlap in hex — so it degraded to a judgement call
about whether a function "smells like a library". One of the fourteen is a 3×3
matrix transpose and one is `atoi`.

**Screen those candidates against the ASM before assigning them, even though
they already have reports.** Filtering on the report's own prose is not the
same check and it is weaker: round 17's first pass keyed on words like
`gp_rel`/`addiu_at` appearing in the report and let `func_8003C63C` (15/16)
through, because its report says only "toolchain blocked" without naming the
class. One `grep` over its `.s` settles it in a second. The rule from
CLAUDE.md applies unchanged here — **a blocker's scope is measured, not
read** — and a report is prose about the bytes, not the bytes.

**RANK FROM TITLE/VERDICT LINES ONLY. Never parse a figure out of a report
BODY — and this has now caught three consecutive heads, so treat it as a
mechanical rule rather than a caution.** Round 18's head pulled a "25/25" from
a *correction preamble* describing the number being retracted, and conflated a
callee's score with an already-matched **caller's**. Round 18 wrote the fix
down as "treat any figure near `correction`/`earlier version`/`superseded` as
retracted". **Round 19's head read that and still got it wrong**, pulling
`func_80032BB8`'s "0/14" out of an ordinary attempt narrative comparing a
*rejected* variant against the kept one — no warning keyword anywhere near it.
The real residue is 7/14.

So the keyword heuristic is necessary and **not sufficient**: a report body is
full of numbers describing variants that were tried and thrown away, and
nothing distinguishes them lexically from the one that stands. Only the
title/verdict line is safe.

**And a title line is only as good as the last person who rebuilt it.** Round
19 found `func_8004042C`'s title claiming "closed to ONE isolated word" when
the body measures 22/25 — the "1" was the permuter's *weighted penalty score*
read as a word count. That figure propagated title -> PROGRESS -> staffing ->
back to the runner as an instruction to close a word that was three words.
**So: rank from titles, but rebuild any figure before you put it in an
assignment**, and see DECOMPILATION_LEARNINGS' "A permuter number is in
PERMUTER units, not retail words".

**Screen for a fourth thing while you are here: preserved-body DRIFT.** Round
19 measured roughly one inherited body in six carrying a false "clean /
drift-free" claim (5 of ~30, found independently by three runners). A drifted
body's in-range score is meaningless, so it is mis-ranked by construction. You
cannot check thirty bodies at Gate 1, but you can tell runners to check the
ones they actually resume — which is what that round did, and it is how the
three were found.

**Round 33 found a WORSE variant of this, and it needs a different check: a
preserved body whose recorded score was NEVER MEASURABLE.**
`func_8002CF18`'s inherited body called `func_800375E8`, a symbol that does
not exist under that name (it is `SpuSetNoiseVoice`). That body could never
have linked, so nobody ever built it, so its figure measured nothing.

Drift is a mismatch between two things that both exist, and you catch it by
reading the report against `src/`. **A never-linked body is self-consistent
everywhere it is written down** — the report is internally coherent, the C
looks right, and no amount of static reading exposes it. The only thing that
does is putting it through the compiler.

So the instruction to runners is now BUILD the inherited body once before
trusting its score, not merely read it. It costs one iteration and it is the
only way to separate "measured, and I can reproduce it" from "never measured".
Once corrected, `func_8002CF18` measures 163/167 and that figure is real.

**AND SCREEN FOR A FIFTH: ATTEMPT HISTORY. THE SCORE YOU RANK ON IS ALSO THE
REASON THE FUNCTION IS EXHAUSTED (round 33).** Round 33's alpha was staffed
onto `code_55dd4` because its titles carried 252/258 and 33/40 — by the
ranking this gate prescribes, the best odds on the board. It produced five
confirmatory negatives and no match. Every one of those five already had five
or six rounds behind it, and two had 190,000+ permuter iterations against
scaffolds that had been sanity-checked. A whole runner session bought
re-confirmation of what the reports already said.

**The selection effect is the point, and it is structural rather than bad
luck.** A function reaches 252/258 by being worked, repeatedly, by people who
did not close it. So the very figure that makes it rank first is evidence that
the cheap levers are already spent — and a title line carries length,
word-match and first diff *by design* (round 23) and carries nothing about
cost. The two highest-ranked functions in a unit are, other things equal, the
two most likely to be exhausted.

This does not invert the ranking; a 252/258 is still worth more than cold
ground, and round 32 closed a 267/270. It changes what you do before
assigning:

- **Skim each candidate report for prior attempt counts and permuter
  iteration totals**, and prefer the smallest gap with the SHALLOWEST history
  over the smallest gap outright.
- **Do not staff a whole unit whose every entry is deeply worked.** Mix: a
  couple of well-worked near-misses plus something with a short report.
- **Tell the runner to skip and say so**, rather than re-running an exhausted
  search. Round 33 put that in the runner prompt and asked for the skips in
  the summary, so the next round can rank on cost rather than re-deriving it.

Note this does NOT make a long report a reason to write a function off — see
DECOMPILATION_LEARNINGS, round 33: a lever's negative is scoped to the state
it was tested under, and a round-19 "this does nothing" closed three words in
round 33 once an unrelated fix had moved the residue. A report full of
ruled-out axes is not an exhausted function. It is a function whose next
attempt should start from a CHANGED state, which is exactly what a cold runner
re-deriving the same ground does not do.

**AND SCREEN FOR A SIXTH, WHICH IS THE ONE MECHANICALLY-READABLE COST SIGNAL:
HAS THIS FUNCTION EVER BEEN PERMUTER-SEARCHED (round 36).** The fifth screen
above says to prefer the shallowest history and gives no way to measure it.
There is one, it is a grep, and round 36 measured that it is the discriminator
that actually paid:

**ROUND 37 CORRECTED THIS GREP. THE ORIGINAL FORM IS BELOW AND IS WRONG --
DO NOT USE IT.** It tested whether the WORD "permuter" appears in the report,
not whether a SEARCH WAS RUN, so a report saying *"this is exactly the kind
of residue the permuter is for"*, *"worth a permuter run before the next hand
attempt"*, or even *"never permuter-searched"* matched it and was classified
as ALREADY SEARCHED. It therefore inverted on precisely the functions the
screen exists to find, and failed in the EXPENSIVE direction -- silently
deleting the best ground from the queue, exactly like a false blocker.

```sh
# WRONG -- round 36's form, kept only so it is recognisable. Do not run it.
#     [ -f "$f" ] && grep -qi permuter "$f" || echo "$fn"

# RIGHT -- key on EVIDENCE OF AN ACTUAL RUN, not on the word.
python3 tools/nearmiss.py | awk '/ASSIGN FROM HERE/,0' \
  | grep -oP '^\s+\d+w\s+\S+\s+\K\S+' | while read fn; do
      f=docs/match-reports/$fn.md
      [ -f "$f" ] || { echo "$fn"; continue; }
      grep -qiE '[0-9]{3,}[, ]*(iteration|iters)|rc=(124|137|0)|base score|permuter-exhausted|score=0|--stop-on-zero' \
        "$f" || echo "$fn"
  done
```

**Measured, both directions, round 37.** Over the same 110-function
blocker-clean queue the loose form reports **18** never-searched and the
corrected form reports **39**. Six of the disagreements were opened by hand
and *all six* were recommendations rather than runs (`func_80031CF0` "a
permuter target"; `func_8004B030` "needs a real permuter pass";
`func_80028920` "still flagged for permuter per the existing report's own
suggestion"; and so on). **Two of them were found the expensive way, by
runners who had been staffed as though the ground were spent:** charlie's
`func_800598E8` (which carried only *"worth a permuter run"*) and echo's
`func_800299BC` (*"exactly the kind of residue the permuter is for"*). Both
ran the first-ever search on ground the head had written off, and echo spent
its remaining budget there instead of on two genuinely-searched functions --
the right trade, made in spite of the brief.

So round 36's "38 never searched" was itself an undercount, and any figure
derived from the loose grep is too small. Re-derive with the corrected form.

The general shape is one this document already names for blocker screens and
is here again: **a screen measures the obstruction it was built for.** This
one was built to measure *cost* and accidentally measured *vocabulary* --
and a report's vocabulary is richest exactly when its author was recommending
the lever that was never pulled.

Round 36 ran four runners over fifteen inherited bodies and closed nothing. It
produced exactly two forward movements — `func_8002AEE0` 153/174 -> 165/174 and
`func_8002B198` 45/91 -> 49/91 — and **both came from fresh permuter searches,
one of them on a function that had never been searched at all.** Every other
function that round either reproduced its recorded figure exactly or, where a
search was re-run on already-searched ground, returned nothing. At the time of
writing the split over the blocker-clean queue was reported as 73 searched
against 38 never searched -- **both figures came from the loose grep above and
are wrong; round 37 re-measured 39 never-searched out of 110 AFTER a round of
searching.** Re-derive with the corrected form, never with those numbers.

This does not make an unsearched function easy, and it is not a ranking by
promise — it is a ranking by whether the cheapest available lever has been
pulled yet. Read it together with the fifth screen: prefer the smallest gap
with NO permuter history, and treat a deeply-searched near-miss as needing a
CHANGED state rather than another attempt.

**And do the screening at ASSIGNMENT time, not in the runner.** Round 36's
alpha was staffed onto an eight-function family, correctly determined it was
permuter-exhausted, and spent zero closing attempts on any of it. That was a
sound reading of cost and a wasted runner: the unit should not have been
staffed. A whole runner reporting "this was exhausted before I arrived" is a
Gate 1b failure, not a runner failure.

**AND A SEVENTH, BECAUSE A CROSS-REFERENCE DECAYS TOO AND IT DECAYS SILENTLY
(round 38).** Round 37's head owned this as an anecdote about one function —
`func_800407F8`, carried as a "pending class member" since round 18 and matched
in round 19, still being counted by the reports pointing at it. It is not an
anecdote. It is a STANDING, MECHANICALLY-DETECTABLE class, and round 38
measured it across the whole corpus.

**The mechanism:** the round that closes a function updates THAT function's own
report. It does not update the reports pointing at it. So every "same class as
`func_X`'s stall", "analogous to `func_X`", "what made `func_X` a good permuter
candidate" keeps asserting a live stall that is gone — and the next runner
reads it as a live wall or a live precedent.

**The detector is a set difference.** A function with a match report that is no
longer a live `INCLUDE_ASM` has LEFT the queue; cross that against
stall-language lines in the reports of functions still IN the queue:

```sh
grep -hoP '^INCLUDE_ASM\("[^"]*", \K[^,)]*' src/*.c | sort -u > /tmp/live.txt
ls docs/match-reports/ | sed 's/\.md$//' | sort -u > /tmp/reported.txt
comm -13 /tmp/live.txt /tmp/reported.txt > /tmp/closed.txt     # left the queue
while read fn; do f=docs/match-reports/$fn.md; [ -f "$f" ] || continue
    grep -no "func_[0-9A-F]\{8\}" "$f" | while IFS=: read -r ln ref; do
        [ "$ref" = "$fn" ] && continue
        grep -qx "$ref" /tmp/closed.txt && \
          sed -n "${ln}p" "$f" | grep -qiE 'stall|blocked|analogous|same class|permuter' \
          && echo "$fn -> $ref"
    done
done < /tmp/live.txt | sort -u
```

**BUT THE SET DIFFERENCE HAS THREE EXITS, NOT ONE, AND THEY MEAN OPPOSITE
THINGS. This is the part that matters and the part the naive detector gets
wrong.** "Has a report, is not a live `INCLUDE_ASM`" resolves three ways, and
round 38 measured all three over 1011 closed functions:

| exit | how to tell | what a citation of it is worth |
| --- | --- | --- |
| **matched as game C** | defined in `src/*.c` | precedent is REAL and complete — go read the matched C |
| **reclassified as a Sony SDK object** | no definition in `src/`, **and no symbol in `build/lsdde.map`** | precedent **NEVER EXISTED** |
| **renamed** | no `func_XXXXXXXX` definition, but the symbol IS in the map | precedent is real, findable only under the new name |

Splitting them is one more command — a function that left the queue into a
Sony object keeps neither a C definition nor a map symbol, because it now
lives under Sony's own name:

```sh
while read f; do grep -qE "(^|[ *])${f}\(" src/*.c && continue
    grep -q "\b$f\b" build/lsdde.map && echo "RENAMED  $f" || echo "SDK      $f"
done < /tmp/closed.txt
```

**Round 38's census: 89 of 1011 closed functions have a report but no C
definition — 86 SDK-reclassified, 3 renamed — and 31 live stall reports cite at
least one of them, 10 of those as an explicit PRECEDENT claim.** Do not read
those figures as current; re-run the commands.

**The SDK exit is the expensive one, and it is expensive in a specific way: it
invalidates a precedent RETROACTIVELY AND TOTALLY.** A closed-by-matching
sibling is a *better* lead than the report claims — the C exists, go read it. A
sibling that turned out to be Sony's was never game code, so **no source shape
ever reached those bytes**, and any residue class, permuter-suitability verdict
or HARD-RULE adjudication derived from it is derived from nothing. The worked
example: `func_8002C278`'s report cites `func_8002C048`/`func_8002C0AC` twice —
once as *"the same class of residue ... in this unit"* and once as *"the 1-2
instruction swaps that made them good permuter candidates"*. They are `strcmp`
and `strncmp` (`libc2/strcmp.o`, `libc2/strncmp.o`), reclassified in round 34,
and `src/code_179d8_d.c`'s own header comment records that they *"had been
matched as C — they were Sony's the whole time"*. So the citing report holds up
as precedent a pair of stalls that were never game stalls, in support of a
permuter recommendation for a third function.

**ROUND 43: THE SCREEN ONLY EVER LOOKED AT REPORTS, AND THE SAME STALE
PRECEDENTS LIVE IN THE SHARED LEARNINGS DOC -- WHICH IS THE ONE FILE EVERY
RUNNER IS TOLD TO READ.** The detector above crosses closed functions against
other match REPORTS. `docs/DECOMPILATION_LEARNINGS.md` cites roughly thirty
SDK-exit function names and nothing was checking it, so a precedent corrected
in a report kept standing, uncorrected, in the document that taught it.

Round 43 measured the damage and it is not cosmetic. An entire named learning
-- *"the DCE-eliminated always-true check"* -- rested on **"two confirmed
instances in sibling CD functions"**, and BOTH are Sony's (`func_8002B94C` is
`CD_newmedia`, `func_8002BCEC` is `CD_cachefile`, `lib/libcd/iso9660.o`,
reclassified round 34). The class had **zero game-code instances**. Round 40
had already adjudicated exactly this in `func_80029C40.md` -- *"there is no
'documented `func_8002B94C` cause' to match against"* -- and the learnings doc
went on asserting the cause as a general category for three more rounds.

**And the invalidation is SHARPER for a toolchain claim than round 38 stated.**
Round 38's rule is that an SDK exit voids a precedent because no source shape
ever reached those bytes. For a claim of the form *"retail does X here and our
compiler does Y"*, there is a second, independent reason: those bytes came out
of **Sony's own ASPSX build**, not our pinned pipeline. So the comparison is
our compiler against a different assembler's output, and a negative result
rules out nothing about GCC 2.6.3 while a positive one would prove nothing
either. That distinction is what separates the half to keep from the half to
withdraw: **a claim verified with a standalone reproducer through the pinned
pipeline survives, because the reproducer does not care who wrote retail's
bytes. An instance-level verdict does not.**

Run it over the shared docs, not just the reports:

```sh
python3 - <<'EOF'
import re
sdk=set(open('/tmp/sdk.txt').read().split())        # the SDK column of the exits census
for doc in ['docs/DECOMPILATION_LEARNINGS.md','docs/MATCHING-GUIDE.md','CLAUDE.md']:
    lines=open(doc).read().split('\n')
    for i,l in enumerate(lines):
        for f in re.findall(r'func_[0-9A-F]{8}',l):
            if f not in sdk: continue
            ctx='\n'.join(lines[max(0,i-12):i+13])
            if re.search(r"Sony|SDK|libc2|libgs|libgte|libcd|libapi|reclassif|NOT GAME CODE",ctx,re.I):
                continue
            print(f"{doc}:{i+1}  {f}\n    {l.strip()[:150]}")
EOF
```

**It over-reports badly and that is the correct trade -- do not tighten it.**
A twelve-line context window cannot tell a live precedent from a citation that
is already corrected two paragraphs down, and every attempt in this document's
history to make a screen more precise has broken it in the expensive
direction. Read the hits; most are fine. The ones that matter are citations
that carry a COUNT of instances (*"two confirmed instances"*, *"the same class
as"*) or that name a function as the worked example FOR a category, because
those are the ones a runner will act on.

**One report already handles this correctly, which is the model to copy.**
`func_80030E90.md` line 532 writes *"a MATCHED sibling, now Sony's object"* —
it names both the exit and the reclassification in five words. That is all a
corrected cross-reference needs.

**And note which screen this is NOT.** `sdkstalls.py` answers "is the function
I am about to be ASSIGNED owned by Sony?" and `nearmiss.py` excludes its hits
from `ASSIGN FROM HERE`, so assignment is already safe. This screen answers a
different question — "is the EVIDENCE this report reasons from still valid?" —
and nothing was checking it. Same shape this document keeps rediscovering: **a
screen measures the obstruction it was built for, and says nothing about the
ones it was not.**

**Whose job:** correcting a cross-reference is the HEAD's, at Gate 1b or at
consolidation, because the runner who would inherit the error has no way to
know the text is stale — round 23's charlie matched all five of its functions
and still signed off calling one "the still-`addiu_at`-blocked function",
read off a report rather than measured. When you close a function, grep for
who points at it before you move on; that is the cheap half of this, and it is
the half that stops the class regenerating.

**AND AN EIGHTH (round 39): A FUNCTION CAN BE SONY'S WITH NO OBJECT ANYWHERE
TO PROVE IT, AND `sdkstalls.py` CANNOT SEE THAT CASE BY CONSTRUCTION.**

`sdkstalls.py` crosses the stall queue against **PLACED** objects, so it
answers *"is this owned by an object we HAVE"*. It cannot answer *"is this
Sony's"*, and `progress.py` reports hundreds of SDK functions with no object
on any disc in `sdk/`. A function in that gap passes every screen the project
owns — no `gp_rel`, no `nop_mflo_mfhi`, not a trampoline, no placed-object
overlap — while being unmatchable by construction.

Round 39 found one, and it was **the smallest function in the whole queue**
(`func_80050B28`, 12 words), therefore first in `nearmiss.py`'s `ASSIGN FROM
HERE` for four rounds, carrying 634 lines of derivation across four rounds.
It is Psy-Q **libcard**.

**Two mechanical detectors, both seconds to run. Neither needs judgement.**

*Detector 1 — segment topology.* A `c` segment bracketed by `o` segments of
ONE library, small enough to be a single module, is probably that library's:

```sh
python3 - <<'EOF'
import re
segs=[]
for ln in open('config/splat.slps01556.lsdde.yaml'):
    m=re.match(r'\s*-\s*\[\s*(0x[0-9A-Fa-f]+)\s*,\s*(\w+)\s*,\s*([\w/\.]+)\s*\]',ln)
    if m: segs.append((int(m.group(1),16),m.group(2),m.group(3)))
segs.sort()
for i,(off,ty,name) in enumerate(segs):
    if ty!='c' or i==0 or i+1>=len(segs): continue
    p,n=segs[i-1],segs[i+1]
    if p[1]=='o' and n[1]=='o':
        print(f"0x{off:06X} {name:<20} {n[0]-off:5d}B  after {p[2]:<20} before {n[2]}")
EOF
```

Ten hits when round 39 ran it; **only one had a live `INCLUDE_ASM`**, and that
one was the libcard function. The large multi-function units in the list are
ordinary game code that merely happens to abut library blocks — size is what
discriminates, so read the byte count, not the bracketing alone.

*Detector 2 — the assembler fingerprint, where the library is a "pure" one.*
Sony's libraries were built with different ASPSX versions, and the `li`
expansion for a positive 16-bit constant differs between them: `addiu
$rX,$zero,K` versus `ori $rX,$zero,K`. Our pinned pipeline
(`--aspsx-version=2.34`) emits **only** `ori` for `0 < K < 0x10000`
(`maspsx.expand_load_immediate()`), so any `addiu` form in retail came from
a different assembler:

```sh
grep -rcE '\b(addiu|ori) +\$[a-z0-9]+, \$zero, 0x' asm/     # game disassembly
for o in lib/*/*.o; do tools/binutils/bin/mipsel-linux-gnu-objdump -d "$o"; done \
  | grep -P '\t(24|34)[0-9a-f]{6} \tli\t' | grep -oP '\t\K(24|34)' | sort | uniq -c
```

Round 39's census: **1 `addiu` against 1089 `ori` in the entire game
disassembly** — the one being that function's word 5 — while Sony's own 178
objects hold 226 `addiu` and 386 `ori`, with **`libapi` and `libcard` at 100%
`addiu`**. Note `objdump` prints BOTH encodings as `li`, and so does
`asm-differ`; only the raw word distinguishes them, which is why four rounds
of instruction-text analysis never saw it.

**ROUND 40: THE VERDICT IS NOW MECHANICAL. `NOT GAME CODE` IS THE FOURTH
HONESTY MARKER.** Round 39 proved the function was libcard, wrote it in the
report title -- and `nearmiss.py` went on ranking it FIRST in `ASSIGN FROM
HERE`, because nothing read the title. A verdict no tool can read is a verdict
the next round pays for again, which is the exact failure `DELIBERATELY
UNWORKED` and `REOPENED -- ASSIGNABLE` were each added to stop.

So: **put the exact phrase `NOT GAME CODE` in the report's TITLE/VERDICT
REGION** (the first 8 lines -- the same window `verdict()` reads, and for the
same reason: only the title is safe to key on). `nearmiss.py` then partitions
it out of `ASSIGN FROM HERE` and prints it under its own heading, alongside
the placed-object partition it cannot otherwise join.

Three things this deliberately does NOT do:

- It does not touch `progress.py`. The function keeps its report and stays a
  documented STALL, which is correct -- it IS documented. The error was only
  ever paid at assignment, so that is the only place it is fixed.
- It does not delete the derivation. 634 lines of it survive, as a record of
  how the ownership question was eventually settled.
- It is not a runner's call. Marking one is a HEAD decision backed by the two
  mechanical detectors above (segment topology, and the `addiu`/`ori`
  assembler fingerprint), never a judgement about whether something "smells
  like a library" -- that judgement is what round 32 cost the project.

**The generalisable rule, and it is the one that actually failed here: A
REPRODUCED TOOLCHAIN MECHANISM IS NOT A BLOCKER UNTIL ITS CORPUS FREQUENCY IS
MEASURED.** Round 32 traced this residue to the exact lines of maspsx that
produce it, in isolation, through the pinned pipeline — and concluded the
function was toolchain-blocked from a sample of one. CLAUDE.md's *"a blocker's
SCOPE is measured, not reasoned"* was applied to the mechanism and not to the
verdict. A rule that is wrong for 1 instruction in 1090 is not wrong.

The error direction is the expensive one named throughout this document, with
a twist: the CAUSE was right and the OWNERSHIP was wrong. That produces a
clean, internally consistent, well-argued report recommending permuter budget
on Sony's code — and nothing in it reads as stale.

**Gate 3 — permuter round instead.** If the fresh queue is dry and carving is
blocked, or the stall residue is worth more than cold ground (near-misses like
88/90), run a permuter round. A zero is a LEAD, not an answer: translate it to
idiomatic C and re-verify with funcdiff.

**The discriminator is WHOSE ORACLE BELIEVED IT, not what the C looks like
(round 41).** This paragraph used to end *"if only UB or duplicate-arm forms
reach zero, mark the class permuter-exhausted in the report and move on"*, and
that sentence was wrong in a way that forbade work the project has now adopted
**twice, deliberately, with the whole-image SHA1 green both times** — round
39's `func_8002AEE0` and round 41's `func_8001E110`. A rule nobody can follow
is worse than no rule, because the runner who obeys it throws away a match and
the runner who does not feels obliged to apologise for one.

Two errors were packed into that sentence:

- **It brackets "UB or duplicate-arm" as one class. They are not.** UB is
  disqualifying on its own terms: a candidate that branches on a value read
  before its first assignment is exploiting the scorer, and round 41's
  `func_8001DA28` search produced several. A duplicate-arm form is merely
  *ugly* — `if (mid.y) return 0; else return 0;` is semantically identical to
  the `return 0;` it replaces, and round 41's bravo checked explicitly that
  `mid.y` was already computed, so it is a redundant TEST and not an
  uninitialized read.
- **It stops at the permuter's own scorer.** That is the real question, and
  the old wording never names it: **a scorer zero is a LEAD; an
  `OK: build matches retail` is an ANSWER.** A form that survives translation
  into the project's C, rebuilds through the real pipeline, and turns the
  whole-image SHA1 green has achieved the project's stated goal — byte-exact
  bytes — whatever it looks like on the page.

So: reject UB. Reject anything only the permuter's scorer believes. **Adopt a
form the real oracle verifies**, and comment it AT THE SITE saying it is inert
and why it is there — both adopted instances do exactly that. Mark a class
permuter-exhausted when nothing survives translation, not when what survives
is unattractive.

**And translation is not optional in either direction (round 41, delta).**
Round 40 established that a sub-base candidate is worth translating even with
no zero, and it was worth twelve words there. Delta translated exactly such a
candidate — score 2735 against a base of 3735, a large early improvement — and
rebuilding it through the real pipeline gave **14/164 with drift**, far worse
than the 65/164 it started from. Both results stand, and together they say:
**translate AND measure.** A permuter score is in permuter units and is not a
proxy for a funcdiff word-match, in either direction.

**Cheapest triage available before you spend a search at all (round 41,
bravo):** the `--debug --stack-diffs` insertion/deletion count predicts
whether a bounded search is worth running. Two searches that round were a
controlled comparison — `func_8001E110` scored **0 insertions / 0 deletions**
and found a zero at iteration 2642; `func_8001DA28` scored **23/44** and
burned 73273 iterations (rc=124) on candidates that were all noise or UB.
Round 40 introduced the scaffold check as a CORRECTNESS test (is this search
capable of succeeding); this makes it a COST test (is it worth starting), and
it composes with the sixth screen — prefer a never-searched function whose
scaffold is near 0/0.

> **ROUND 45 BROKE THE COST TEST IN BOTH DIRECTIONS AND ADDS A THIRD CHECK.**
> Charlie ran two bounded searches on `code_179d8_l`. `func_8002CD08` scored
> **0 insertions / 0 deletions** — the cheap-search prediction — and burned
> ~63000 iterations **without once beating its own seed**, because the residue
> is pure register identity and lies outside what a source-mutation search can
> reach at all. So **0/0 is NECESSARY, NOT SUFFICIENT**: the scaffold check
> measures frame-layout fidelity, not whether the residue is in the search
> space.
>
> The other search found something worse and it is a NEW check rather than a
> refinement: **the permuter's isolated single-function scaffold can produce
> materially different REGISTER ALLOCATION than the real translation unit, for
> the identical source.** `func_8002D8E0`'s base score turned out to be an
> artifact of that — so every candidate was scored against a program nobody
> is building, and "51435 iterations, nothing found" is evidence about the
> scaffold rather than about the function. Charlie caught it by rebuilding the
> same body in-tree and diffing against retail directly.
>
> **So check three things before spending a search, cheapest last:**
> 1. *(round 40)* does the scaffold compile and score — CORRECTNESS;
> 2. *(round 41)* insertion/deletion penalties — COST, necessary not sufficient;
> 3. **(round 45) does the scaffold's BASE SCORE agree with the same body's
>    score in the REAL build?** One rebuild answers it, and a disagreement
>    means stop.
>
> **And this changes how to read a recorded negative at Gate 1b.** "Permuter
> tried, negative" is not evidence about the function unless check 3 passed.
> Round 37 found the permuter-history screen over-counting searches because it
> keyed on the WORD rather than on evidence of a run; this is the same error
> one level deeper — a search that genuinely ran, and still measured nothing.
> Ask a runner to record WHICH of the three checks it ran, not just the
> iteration count and rc.

**Never authorized — always an operator escalation, evidence attached:**
toolchain or flag changes of any kind; editing the protected verification files;
bypassing build-and-verify.

Failing a gate is allowed. If one cannot be satisfied confidently — ambiguous
carve boundary, contradictory unit state — report the specifics and stop, rather
than burning a runner round on a bad premise.

## A runner that stops "waiting on background work" MAY RESUME ITSELF — do not hand-recover until you have confirmed it is dead (round 40)

Round 40's bravo and charlie both ended a turn waiting for a notification,
which this document has called the round-33 failure since round 33. The head
applied the documented remedy — recover the worktree by hand before teardown —
and **both runners then woke up and finished their own work.**

**The mechanism, which nobody had written down:** a runner's completion
notification fires each time it stops *with no live background children of its
own*. A runner that backgrounds a permuter and then stops therefore notifies
the head **while still being resumable**, and when its search finishes it can
be re-invoked and continue. "Notified" is not "finished", and it is certainly
not "dead".

**What the premature recovery cost, measured:**

- charlie's live `src/` was reverted under it; it detected this afterwards via
  `git reflog`, reconciled the duplicate report sections the head's commit had
  created, and reported the head's own intervention as an anomaly — correctly.
- bravo's report was written twice, by the head and then by bravo, producing
  the round's **only merge conflict**.
- The head published a verdict (`func_8004BE54`'s lever is "UNTESTED") that
  bravo falsified within the hour by testing it (142/150). That verdict had to
  be marked SUPERSEDED in the report.

None of it lost work, and the duplicated effort did buy one real thing — an
**independent reproduction** of `func_8004B700`'s 137/140 from the same saved
candidate, arrived at without access to bravo's reasoning. That is worth
having, but it is not worth engineering on purpose.

**So, before recovering a stalled runner by hand:**

```sh
pgrep -af "wt-<name>.*decomp-permuter"     # its OWN worktree path, never a bare tool name
git -C ../<checkout>-wt-<name> status --porcelain
git -C ../<checkout>-wt-<name> log --oneline <merge-base>..runner/<name>
```

**If any of its processes are alive, it is not stalled — it is waiting, and it
will come back. Leave it.** Recover only when the process table is clear AND
the tree still holds uncommitted work AND you are about to tear down. Teardown
is the real deadline; a runner between turns is not.

**ROUND 43: AN EMPTY PROCESS TABLE IS NOT EVIDENCE THE RUNNER IS DONE, AND
THE HEAD PROVED IT ON ITSELF.** Round 40 gives a sufficient condition for
ALIVE -- processes running means leave it alone. The converse does not follow,
and round 43's head assumed it did.

What happened: bravo hit the §2c wait-loop exactly as predicted, notifying
with a status line (*"I'll wait for the monitor notification"*) and 11 matches
committed. The head checked the process table, found bravo's permuter workers
**gone**, and correctly concluded the SEARCH had ended. It then salvaged: read
the leftover `permuter-work/` candidates, spliced bravo's base scaffold into
`main`, and built it to recover title figures.

**bravo then resumed on its own and wrote a better report than the salvage
could have** -- 150582 iterations, base score 1643 -> 900, three-figure title,
and the same UB verdict on the near-best candidates that the head had reached
independently. The salvage bought nothing.

**The distinction the head missed: the process table tells you about the
SEARCH, not about the RUNNER.** Round 40's own mechanism says why -- a runner
that backgrounds a search notifies while still resumable, and *"when its search
finishes it can be re-invoked and continue"*. So the search ENDING is precisely
the moment the runner becomes most likely to come back, which is the worst
possible moment to conclude it is gone.

**Cost, and why it is worth writing down even though it was cheap.** The
salvage was non-destructive and was reverted, so no work was lost -- round
31's "do the salvage and DEFER the kill" held, and nothing was killed. But it
left a non-matching body live in `main` while the head measured it, and a red
whole-image build in the shared checkout is the round-20 attribution hazard
pointed at everyone else: any other reader of `main` in that window sees a red
oracle with no compile error and no obvious cause. **If you must measure a
salvaged body, do it in a worktree, never in `main`** -- the main checkout is
single-occupancy for the head's MERGES, which is not the same as a licence to
run experiments in it.

**The operational rule: teardown is the deadline, and nothing else is.** Do
not salvage on "its processes are gone", do not salvage on a completion
notification, and do not salvage on both together -- round 43 had both and was
still wrong. Salvage when you are about to run `git worktree remove`, and not
before.

**The old rule was not wrong, it was under-specified**, and this is the same
decay shape this document keeps naming: the round-33 remedy was written from a
case where the runner genuinely never came back, and it was applied to a case
that looked identical from the outside. The distinguishing evidence is the
process table, and checking it costs one command.

**What still holds unchanged, and is the half that did the work:** bravo had
committed everything before it waited, so its stall cost nothing and its match
was never at risk. charlie had not, so its stall left a red build and two
non-matching bodies live in `src/`. **"COMMIT BEFORE you wait" is the rule
that made the difference**, and it is the one to keep pressing on runners.

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

**And spread runners across units with DISJOINT HEADER SETS where you can.**
This is the one sizing lever that reduces head attention rather than spending
it: `python3 tools/headercontention.py <units>` before provisioning, details
and the measured history in collision rule 1. It also changes what carving is
FOR — when Gate 2 fires and you get to choose which monolith to carve into,
carving across two different blocks buys a conflict-free round, while carving
four more slices of the block you are already in guarantees another
all-conflicting one. Round 15 carved four slices of a single block and got six
conflicting merges out of seven; that was a defensible choice given where the
fresh ground was, but it was a choice, and it was not free.

**A concentrated round should be SMALLER than a spread one.** If the fresh
ground genuinely lives in one block, four or five contending runners will
saturate the head before the runners saturate the machine — round 15's evidence
is that three of five finished early with ground left and could not be re-sent.
Three contending runners plus re-sends is likely to beat five contending
runners without them.

**Mid-round broadcasts are worth their cost.** A lever found at hour one is
worth several times more applied across four units than banked for the
write-up. Push it to every live runner immediately, and **ask explicitly for the
negative answer** — "does this apply to your unit?" A runner reporting that it
does NOT apply is cheap, and it stops the next head re-litigating the question.

**The mechanism is `tools/broadcast.sh`, not `SendMessage`** (which this
environment does not expose — see §3c). `tools/broadcast.sh post "..."` from
the head or `post --from <name> "..."` from a runner appends a dated entry to
`.round/BROADCAST.md`, shared by every worktree; runners `read` it before each
function. Round 43 lost two broadcasts for want of this: alpha's lever never
reached the other four, and bravo ran a search the head already knew was
invalid.

## Runner subagent prompt (head fills in `<>`)

> You are a matching runner for the LSD: Dream Emulator (PSX) decomp. **This is
> round `<N>`** — use that number in every report, title and commit message you
> write. (Two of round 33's five runner sessions guessed it wrong, one high and
> one low, and the head had to relabel three files; the prompt had simply never
> said.) Work ONLY
> in the worktree `<path>` (cd there first) on branch `runner/<name>`. Read
> CLAUDE.md, docs/MATCHING-GUIDE.md and docs/DECOMPILATION_LEARNINGS.md, then
> decompile up to `<N>` functions from the `INCLUDE_ASM` entries in
> `src/<unit>.c` ONLY.
>
> **The broadcast channel is a file, not a tool.** Nobody can message you
> mid-round and you cannot message anyone: `SendMessage` is not exposed in
> this environment. Instead, **before you start each function, run
> `tools/broadcast.sh read`** and act on anything there — a lever another
> runner found, a warning from the head, a correction to your brief. And the
> moment YOU have something the others should know — a lever that closed
> real residue, a negative ("the X lever does not apply to my unit"), a
> toolchain smell, a question for the head — **post it at once with
> `tools/broadcast.sh post --from <name> "..."`**, not in your final summary.
> A lever banked for the write-up reaches nobody until the next round.
>
> **The oracle.** `./build-and-verify.sh` plus `tools/funcdiff.py`. Chain them
> so you cannot read a score from a failed build:
> `./build-and-verify.sh > /tmp/<name>_b.log 2>&1; echo "build exit=$?"; grep -nE 'error:|parse error|undefined reference|\*\*\* \[[^]]*\.o\]' /tmp/<name>_b.log | head -8; .venv/bin/python3 tools/funcdiff.py <fn>`
>
> **The `*** [….o]` alternative is load-bearing and was added in round 21 —
> do not trim it back to the three text patterns.** GCC 2.6.3 predates the
> `error:` prefix, so `conflicting types`, `redefinition`, `undeclared`,
> `too many arguments`, `incompatible types in return` and `duplicate
> member` are ALL fatal and ALL produce zero hits on the text patterns. It
> matches make's failure on a compile target
> (`*** [Makefile:113: build/src/<unit>.c.o] Error 33`) and deliberately
> NOT on the SHA1 check's own line
> (`*** [Makefile:74: check] Error 1`), which is what round 18 removed
> `Error [0-9]` for. Measured table in CLAUDE.md, step 4.
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
>
> **Bound every long search with `timeout`, and capture its status on the
> very next command:** `timeout 600 <search>; rc=$?; echo "permuter rc=$rc"`.
> `124` means your own bound fired; `137` means something killed it from
> outside. Anything appended with `;` before you read `$?` reports ITS exit
> status instead, which is how round 32's alpha lost the answer it had been
> asked for.
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
> **A STALL report's TITLE must carry THREE figures, not one** (round 23; the
> next round ranks from title lines and from nothing else):
>
> 1. **LENGTH** — exact, or N words short/long.
> 2. **RAW WORD-MATCH** — M/N.
> 3. **WHERE THE FIRST REAL DIFF IS**, read off `tools/asm-differ/diff.py`
>    (which realigns) — never inferred.
>
> "N words short" and "M/N words match" are DIFFERENT measurements that read
> identically, and a single figure hides which state you are in. A missing word
> SHIFTS everything after it, so a low word-match can be almost entirely
> ripple from a one-word length gap — or genuine independent residue — and
> those two send the next round to opposite places. Round 23 filed
> `func_80031A44` as "51/88 ... one missing nop" (contradictory), and the
> corrected title reads *"length 1 word SHORT at 87/88; 51/88 raw word-match,
> but that 37-word gap is almost entirely the SHIFT from the one missing word"*
> — a one-word near-miss and a prime permuter target that "51/88" buried
> completely.
>
> **COMMIT EVERYTHING YOU PRODUCE, INCLUDING STALLS. COMMIT AS YOU GO, NOT AT
> THE END.** One commit per MATCHED function (do not batch matches); stalls
> may be batched into a single "stall reports" commit. Read that carefully —
> it is NOT "commit only on a match". An uncommitted file does not exist:
> `git status --porcelain` must be empty when you report, and the worktree is
> DESTROYED at the end of the round. Never push.
>
> **NOTHING RUNS ON YOUR BEHALF AND NO NOTIFICATION IS COMING TO YOU. Do not
> end your turn waiting for one.** A round-33 runner did exactly that, waiting
> on a "monitor" that never existed, and ended its session with four modified
> reports and ZERO commits; they survived only because the head recovered the
> worktree by hand before teardown. A second runner that same round ended a
> turn waiting on a permuter it HAD bounded correctly — it did resume and
> finish, but in the meantime it looked identical to the dead one, and the
> head committed underneath it. **Both problems are solved by the same
> habit: if you started something in the background, poll it yourself and
> then finish; and COMMIT BEFORE you wait, not after.** An uncommitted file
> is indistinguishable from a lost one from the outside.
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
> **And this is not limited to RETYPES — round 13 widened it.** Any edit to a
> struct that another already-matched function reads can break that function,
> including an ordinary field INSERTION where you forgot the leading
> `u8 padNN[...]`. Round 13's delta did exactly that, shifted every later
> field, and broke an already-matched function in a different unit by one
> byte with a clean compile. So run the same check for every struct edit, not
> just slot retypes. If the build goes red with no compile error, localize it:
> `cmp -l build/SLPS_015.56 disk/SLPS_015.56 | head`, convert the position
> (**1-based**) with `vram = (N - 1) - 0x800 + 0x80010000`, and look that up
> in `build/lsdde.map`.
>
> **A prototype for a function ANOTHER unit defines goes in YOUR `.c`, never
> in a shared header.** Same for an `extern <your local type> D_XXXX;`. This
> is the one shared-header mistake that git does not mark and your own green
> build does not catch: the collision only appears in some OTHER unit that
> includes both your header and a second one declaring the same name. Round 15
> hit it four times; the worst instance stayed latent for two merges and then
> broke a unit that had never touched the declaration. A unit-local view
> belongs in the unit — if you name a type for your own reading of a class,
> keep the type AND the declarations that use it next to your code. Put in the
> shared header only what a sibling unit would genuinely reuse unchanged.
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
>
> **`SendMessage` is not exposed in this environment — do not report its
> absence as an anomaly, and do not lose the broadcast because of it.** The
> channel is `tools/broadcast.sh`: `clear` it before spawning, `post` every
> lever, warning or brief correction the moment you have it, and `read` it at
> every triage for the runners' own posts and questions. Runners are told to
> read it before each function and to post levers and negatives immediately.
