# Finishing plan: tracks 6 to 9 (phase 2), archived 2026-09-28

Archived verbatim from docs/FINISHING-PLAN.md at plan revision 40, when
round 102 closed every phase-2 track and revision 41 opened phase 3. The
phase 2 RULES (zero bytes, renames replay, comments, style, measure) stay
live in the plan; these are the per-track procedures and their prompts.

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
(a local `GsIMAGE`, a local `PadInit` prototype, or `struct GsIMAGE` for
Sony's untagged typedef) cannot sit beside these;
`python3 tools/sonyheaders.py` lists them, and `plan.py` attaches each to its
job: a header's to track 6, a unit's to its polish pass. The fix is always
Sony's declaration, never a rename of Sony's.

**Jobs**, from `plan.py`, root class first: one per class whose type or table
name is a placeholder, carrying the placeholder types its header defines, and
one per other file that defines placeholder types. The head adds a name the
patterns cannot see (an opaque one such as `ObjM`, or a local view of a type
another file defines) with `plan.py flag-type`; its file's job then ranks
first, since other jobs' debt usually waits on it (round 92: SoundCueSet).
A flag whose fix also edits other units (retiring their views of the type)
names them with `--units`, and they join the job's edit set, so it defers
behind a job holding them (round 97: ResourceRequest behind SceneNode.h's).
Ahead even of that, a header that re-declares a Sony name gets a job of its
own, because none of its includers can take Sony's headers until it does
(round 94: `ViewportOt` is Sony's anonymous `GsOT`, and `Viewport.h` could
not include `<libgs.h>` past four such headers). Its edit set is the header
and every unit including it, directly or through another header, since each
takes Sony's headers in the same fix; headers sharing an includer are one
job (round 95). A flagged fix that needs
Sony's headers where those collisions sit is flagged `--after <files>`, and
`plan.py` lists it as WAITING until each has left `tools/sonyheaders.py`,
and ranks the polish pass of a unit it waits on first; once ready, a flagged
header's job takes the same edit set and joins a collision header's job it
shares an includer with (round 96: `Viewport.h` with `Sprite.h`).
A class job's edit set is its header plus the units holding the class's own
methods (their banners and field accessors), so a polish pass on one of those
units defers behind it (round 93: class_3bb8c behind StageMap's job).

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
`include/` (never `include/psyq/`, nor `include/include_asm.h`, which splat
rewrites on every extract). A multi-line `#define` and a K&R
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
   masks, flags and addresses, for EVERY literal, named or not: "a tuning
   value with no shared meaning" excuses a name, never the base (round 92
   sent two passes back for leaving `moodTimer == 0xC8`). A literal stays
   when a name would only restate it. A helper macro (fixed point, `ARRAY_COUNT`) lives in
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
its Sony object (`libsnd_vmanager.c`). The banner says what the file holds; what
decided its edges is history, for its first function's report. A files runner may edit its region's yaml lines,
through `unitfile.py` only (the one exception to PARALLEL-RUNS §2's yaml
rule). **Head at merge:** `make extract`, then delete `build/src` and rebuild,
because unit names changed. **Park rule:** a region the evidence and content
cannot settle, or that content alone would split (no tool splits a unit),
keeps its carve edges, content-named, with the reason in each banner.
A placeholder-named header whose unit a merge absorbed belongs to no region;
`plan.py` lists it as a job of its own (its edit set: it and every unit
including it), done by moving each section into the header that owns its
subject, or by a rename for what it holds (round 102: `class_3bb8c.h`).

### Track 9: close-out

Opens when tracks 6 to 8 are done, and stays open once an item is ticked
even if one of them reopens. Items, ticked with `plan.py check --item`:
`layout` (subsystem directories under `src/`, chosen from what the files hold:
`unitfile.py rename <unit> <dir>/<unit>`; README's code map says what lives
where), `readme`, `comments` (readability history 0, headers too; a
banner's edge evidence is history), `globals` (no game global named
UPPER_SNAKE: `readability.py --globals`), `style` (`make format` leaves no
diff), `docs-budget`, `nonmatching-clean`. An item already true when
measured is ticked by the head with the measurement in PROGRESS.md; the
rest are runner jobs (§2), prompt §4.7, one item each.


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
> **Your shell's working directory resets between commands:** start EVERY
> command with `cd <path> &&`, and trust the oracle's `OK:` line only if it
> ends `(tree: <path>)` (round 97: a runner built the main checkout and
> reported four commits byte-exact that never compiled).
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
> **Your shell's working directory resets between commands:** start EVERY
> command with `cd <path> &&`, and trust the oracle's `OK:` line only if it
> ends `(tree: <path>)` (round 97: a runner built the main checkout and
> reported four commits byte-exact that never compiled).
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
> **Your shell's working directory resets between commands:** start EVERY
> command with `cd <path> &&`, and trust the oracle's `OK:` line only if it
> ends `(tree: <path>)` (round 97: a runner built the main checkout and
> reported four commits byte-exact that never compiled).
>
> Final summary: per region, the files before and after, the evidence that
> decided each edge (quote `tuboundary.py`), what content decided, and
> anything parked.

