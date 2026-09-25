# FINISHING-PLAN tracks 1, 1b and 2, as they stood at revision 16

Moved here verbatim on 2026-09-25 (revision 17, round 80) when all three were
done, to make room for track 4's recipe inside the plan's word budget. They
are the rules to follow if `plan.py` ever reopens one of these tracks (a new
fresh function, a stall, an unnamed SDK callee); the section numbers below are
revision 16's.

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

**Revisit rule.** Every stall gets ONE revisit, and a second once every stall
has had its first (revision 14: revisits paid 40/60 against the band's 1/13,
so the re-read, not the band, is what track 1 now is; `plan.py` labels those
jobs `REVISIT-2`). REVISIT-2 is the LAST pass: with neither pass left and no
fresh ground, `plan.py` marks track 1 done (revision 15). A revisit is a fresh Opus re-read
in cost order, with the preserved body rebuilt first so `funcdiff.py`'s
`insertions / deletions` line is recorded before anything else. Read it as
a pointer, not a verdict: a title claiming register identity with a nonzero
ins/del deserves a re-read, but at equal length an N/N figure can be a false
alignment on a loop nest's repeating instruction skeleton (round 63 measured
26/26 on a zero-insertion input), so funcdiff also prints positional skeleton
diffs and reports 0/0 when those are zero. `plan.py` lists every stall whose report has no
`REVISITED` line; the runner writes `REVISITED, round N: <outcome>;
names/types <used | not relevant>` and that retires it. This is what remains
of track 1 after the stop rule parks the band: revisit jobs in the
round-robin until every stall has had its two. The rule was first written to test whether
a unit's new names unlock old stalls, then gated on stale titles; three
consecutive revisits recorded names as not relevant while the revisits paid
3 matches in 7 attempts against the band's 1 in 13, so the trigger is gone and
the re-read stays. `plan.py` prints the running revisit yield; if it falls to
the band's rate over ten or more attempts, the operator decides whether the
remaining revisits are worth their tokens.

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
 * the second loop (docs/match-reports/Foo__Example.md). Hand-derived. */
void Foo__Example(Foo *this, s32 arg1)
{
    ...
}
#else
INCLUDE_ASM("asm/nonmatchings/<unit>", Foo__Example);
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
- Written for the reader. The verified build never compiles this body, so a
  construct whose only purpose was bytes (`do { return; } while (0);`, a
  redundant copy, an inverted arm) buys nothing here: write the plain form and
  leave the byte-shaped one in the report (round 66).
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
segment, and every Sony function inside a game segment, carries Sony's name.
`plan.py --json` lists the unnamed ones under `tracks.2.unnamed_list`. Nothing
else in the SDK is a goal.

**Sony code in game segments.** A segment's name is not evidence at function
grain: an object whose build differs in one function never places, and its
other functions sit in game units as C or as stalls (round 69 found
`code_179d8_*` holding libsnd, libcd and libapi). `progress.py` counts a
game-segment function as library when any of three records says so: an exact
fingerprint in `config/sdk-in-game.txt` (generated: `sdkname.py --game
--write`, rerun when `sdk/` changes; `--check` says whether it is current), a
`config/psyq-objects.ld` pin (a linked Sony object calls that address by
name), or an `identified` comment on its symbols entry (position plus header
or strings, recorded by the evidence rule below). Such a function leaves the
game counts and every track 1, 1b and 3 queue, and a game-style name on one
(a game-worded name for Sony's) counts as unnamed here. Renaming onto
a pinned name is byte-identical and keeps the ld fragment current (measured).
The generated file also carries LEAD lines: no exact fingerprint, but shape
>= 0.90 at >= 40 words, the cliff below which only unrelated stubs score
(revision 14; every lead above it was libsnd), and ADJACENCY leads: a body of
4+ words with a too-common exact fingerprint that touches Sony code, with a
candidate from that side's library (revision 16; it finds round 78's five
hand-found wrappers and no game stub). A lead counts as library until
the runner either identifies it (comment line with `identified`, rename; any
Sony name the evidence settles on closes it; `Rename pending` in that comment
lists it for track 2 until the rename drops the words) or rejects it with a
`// not SDK: <reason>` comment line above its symbols entry.

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
comment LINE above the entry (splat rejects `key: value` in a trailing
comment, round 69): `// identified: fingerprint 0.96 vs libsnd/ss_xxx.o (3.3), header LIBSND.H`.
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


## Prompts for these tracks

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

