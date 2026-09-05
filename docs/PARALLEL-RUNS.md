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

3c. **Send an early finisher back into its OWN unit** (message the same agent —
   same worktree, same branch, context intact) rather than letting it idle or
   spawning a cold replacement. This is the highest-yield *structural* move
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

**Screen the queue with THREE greps, not two.** CLAUDE.md names two ---
`gp_rel` and `addiu $at, $at, %lo`. There is a third, published inside
`docs/research/addiu-at-blocker.md` rather than in CLAUDE.md, and round 13's
head missed it at Gate 1 and assigned a blocked function because of that:

```sh
for f in $(grep -oP 'INCLUDE_ASM\("[^"]*", \K\w+' src/<unit>.c); do
    grep -A2 -nE '\b(mflo|mfhi)\b' asm/nonmatchings/<unit>/$f.s \
      | grep -qE '\b(mult|multu|div|divu)\b' && echo "BLOCKED (nop_mflo_mfhi): $f"
done
```

An `mflo`/`mfhi` **FOLLOWED WITHIN TWO INSTRUCTIONS BY** a
`mult`/`multu`/`div`/`divu`, with no `nop` between them in retail's own bytes,
is blocked exactly like `addiu_at`: the pinned pipeline inserts `nop`s retail
does not have.

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

## Runner subagent prompt (head fills in `<>`)

> You are a matching runner for the LSD: Dream Emulator (PSX) decomp. Work ONLY
> in the worktree `<path>` (cd there first) on branch `runner/<name>`. Read
> CLAUDE.md, docs/MATCHING-GUIDE.md and docs/DECOMPILATION_LEARNINGS.md, then
> decompile up to `<N>` functions from the `INCLUDE_ASM` entries in
> `src/<unit>.c` ONLY.
>
> **The oracle.** `./build-and-verify.sh` plus `tools/funcdiff.py`. Chain them
> so you cannot read a score from a failed build:
> `./build-and-verify.sh > /tmp/<name>_b.log 2>&1; echo "build exit=$?"; grep -nE 'error:|parse error|undefined reference' /tmp/<name>_b.log | head -8; .venv/bin/python3 tools/funcdiff.py <fn>`
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
