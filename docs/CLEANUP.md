# Cleanup: making the code read like the game's source

Every function matches and every header is documented. What's left is the
distance between "a careful decompilation" and "code someone would have
written": data you can't see, notes about the compiler, names that are
table indices, and a few files holding many classes. This is phase 4 of the
plan (tracks 14-19). `python3 tools/plan.py` measures it and lists the ready
jobs; tracks 1-13 and their history are on the `archive/process` branch.

Everything here has one constraint: **the image stays byte-identical.**
`./build-and-verify.sh` after every change, `tools/lint.sh` and
`doxygen Doxyfile` clean before every commit.

## Running a round

One **head** session reads this file and `python3 tools/plan.py`, then
staffs up to five **runners**. Each runner gets one item and its own
worktree:

```sh
tools/setup-worktree.sh <name>      # ../<checkout>-wt-<name> on branch runner/<name>, verified
tools/teardown-worktree.sh <name>   # refuses while anything is unmerged or uncommitted
```

**An area has one owner at a time.** Tracks 15, 16 and 17 all have a
`world`, `graphics` and `rest` item, and two runners in the same files
conflict. So a round gives each runner a different area. For example:
literals-world, comments-graphics, shape-rest, mood-cues and noop-slots.
Track 18 touches `entity.c` and the class headers, so it doesn't run beside a
`world` item.

The head merges each runner with `git merge --no-ff`, checks that no merge is
half-finished (`git rev-parse -q --verify MERGE_HEAD`), rebuilds with
`./build-and-verify.sh`, and ticks the item (`python3 tools/plan.py check
--item <id>`) only when the item's done-when holds for the whole area. It
records the round with `plan.py record-round --track <k> --round <n> ...`
and commits.

A premium session does the `*-setup` items. They write a procedure into this
file, and a runner can't follow a procedure that doesn't exist yet.

**Runner prompt** (the head fills in the angle brackets):

> You are runner `<name>` for the LSD: Dream Emulator decomp, in worktree
> `<path>` on branch `runner/<name>`. Your shell's working directory resets
> between commands: start every command with `cd <path> &&`. Read CLAUDE.md
> and docs/CLEANUP.md (your track's section). Your item: `<id>`: `<done-when
> from plan.py>`. Stay inside your area's files and the headers of the
> classes they define. After every change, `./build-and-verify.sh` must end
> `OK: build matches retail`. Make one commit per coherent change, with a
> message that says what changed in game terms. Before you finish,
> `tools/lint.sh` and `doxygen Doxyfile` must be clean and `git status
> --porcelain` must be empty. Report what you changed, what you left and why,
> and anything that needs a decision.

## Track 14: data in C

The game's tables live in the disassembly, and the C only sees
`extern EntityMoodRow sEntityMoodTable[];`. A reader never sees which entity
links to which stage or which script it runs. Defined in C, a table reads
like this (row 0, from the retail bytes):

```c
EntityMoodRow sEntityMoodTable[] = {
    [0] = { {0, 2}, 20, ENTITY_ACTIVATE_AT_ATTACH, 0, 0, 2, -13, 4, 1, 10, 0, Entity__CuePaceOrLiftOffOnPink },
    ...
```

**Why it needs a setup item.** splat cut many tables into several labels,
wherever code reads a column on its own (`sEntityLinkStageTable` is
`sEntityMoodTable`'s linkStage column, read as a flat array). The labels
have to merge back into one symbol, the data has to move from the `.data`
segment into the unit's own section without moving a byte, and every column
reader has to become a field access or keep its view through a documented
alias. `data-setup` works that out on `sEntityMoodTable`, proves it
byte-identical, and writes the steps here. Then each area moves the data it
owns. An `extern` stays only for data another file defines.

## Track 15: named literals, a pass

This is not a campaign to reach zero. A motion amount, a timer length or a
screen coordinate reads fine as a number. Name a literal when the name says
more than the value:

- **Per-script states.** `Entity::state` isn't one enum. Each
  `Entity__MoodCueNN` handler runs its own small script, and values from 11
  up are that script's private steps (1 is the shared `ENTITY_STATE_DONE`).
  Name them locally:

  ```c
  void Entity__CueHoverOverDreamerOnBlueElseRise(Entity *self) {
      enum { CUE30_APPROACH = 11, CUE30_GROW = 12, CUE30_FOLLOW = 13 };
  ```

- **Enums that already exist.** `entity.c:988` compares `getDreamColor()`
  with `1` while its neighbours use `DREAM_COLOR_*`.
- **Flags, ids and sizes** that recur, or that a Sony header already names.

## Track 16: .c comments

The headers were rewritten to describe the game; the `.c` comments weren't,
and read older. A `MATCHING:` line stays only where the spelling isn't the
natural one, and says why in plain terms ("two nested ifs: `&&` compiles to
a different compare"), without registers or tool names.

**File-private `#define`s go at the top of the `.c`**, after the includes;
shared ones go in the header. Most files already do this.
`dream_scene.c` scatters its defines because it holds several classes;
track 19 fixes that by splitting the file. `tmd_renderer.c`'s
define-before-use-then-`#undef` blocks are a legitimate local style, so
leave them.

## Track 17: code bent to match

Some constructs exist only because they reproduce retail's bytes:

- **Calls that rely on leftover argument registers.** For example,
  `((LoadFileNoArgsFn)FileResource__LoadFile)();` in `cd_driver.c` passes
  nothing and relies on `self` and `name` still sitting in the argument
  registers. Functions that fall off their end without a `return` are the
  same kind of problem. Both are undefined behaviour in C and would break a
  port. `ub-calls` tries the natural spelling first. If the natural spelling
  doesn't match, the function becomes a stall: the readable body goes in
  `#ifdef NON_MATCHING` with the `INCLUDE_ASM` in its `#else`, and the item
  says so.
- **`goto`s, `__asm__("")` barriers, single-field "box" structs, `(u32)`
  loop counters.** Rewrite each as natural C where that is byte-identical;
  otherwise it keeps one `MATCHING:` line saying why.

## Track 18: behaviour names

Names that are table positions: 107 `Entity__MoodCueNN` handlers and 56
`*__NoOpSlotNN` occupants. A handler's body says what the entity does
("rises, then faces the dreamer"), so it can be named for that now. What the
entity *is* (which character, which stage) needs track 14's table, which says
which stage row NN links to and which video it ends in. Rename them first by
behaviour, then refine when the data is readable. Rename through
`python3 tools/rename.py`.

## Track 19: one class, one file

`dream_scene.c` holds half of `ItemList`, `ObjM`, the style layer,
`StyleEffect`, `Actor` and `VariantSprite` (`GraphRoom` went to
`graph_room.c` in `split-setup`). A split keeps ROM order: each new file
takes a contiguous run of functions, and a `.c` is one splat subsegment, so
cutting a class out of the middle of a unit makes three files, not two.

**Procedure** (proven on `GraphRoom`, and on a trial cut of `ItemList`'s
half that also splits an attached jump-table slot):

1. **Pick the cut.** `python3 tools/tuboundary.py --unit <unit>` lists the
   unit's functions with the verdict at each edge. Cut only at `boundary
   possible`, normally right after a class's `Get<Class>Methods`. An
   `unlikely` edge means rodata used by one function on each side would sit
   in the wrong order; an `impossible` one can't be cut at all.
2. **Find the file offset** of the first function of the new file:
   `vram - 0x80010000 + 0x800` (`New_GraphRoom` at `0x80057F68` is
   `0x48768`).
3. **Split the text subsegment** in `config/splat.slps01556.lsdde.yaml`: add
   `- [0x<offset>, c, <dir>/<file>]` after the unit's own line, in address
   order, with a one-line comment saying what the file holds.
4. **Split the rodata, if the new file owns any.** Find the unit's
   `.rodata` line (`grep -n '<dir>/<unit>]' config/splat*.yaml`).
   - Jump tables (`jtbl_…`, attached with the dot form `.rodata`) go with the
     function that switches on them: their words are labels local to that
     function's `.s`. If both sides own tables, split the attached line at
     the first table the new file owns: `[0x1EF4, .rodata, world/a]`
     becomes `[0x1EF4, .rodata, world/a]` and `[0x1F4C, .rodata,
     world/b]`. The offset is the table's `vram - 0x80010000 + 0x800`.
   - Strings reached through an `extern const char …[]` sit in a standalone
     `rodata` line and need nothing: they resolve by symbol from any object.
     `GraphRoom`'s two paths are that case, so its split touched only text.
   - A string written as a C literal is emitted in the object that holds
     the function, so a cut between it and its neighbours' strings has to
     land where tuboundary says `possible`.
5. **Move the C.** The run of functions, their section banner and the
   `#define`s and `extern`s only they use go to the new `.c`, defines at the
   top after the includes. Give the file a header comment in the style of
   its siblings (what class, which functions, in ROM order). Include only
   what it needs; the build says what is missing. Fix the header's
   `Methods in src/...` line and the old file's header comment.
6. **Rebuild from a fresh extract:** `make extract`, then
   `./build-and-verify.sh`. A split that is wrong fails the link or the
   SHA1, so a green build is the whole proof. Check the new file compiled
   without implicit-declaration warnings (`grep -A2 '<file>.c:' /tmp/b.log`).
7. **Ledger:** the head marks the new unit passed where the old one was
   (`plan.py mark-unit --unit <file> --track 3`, then `--track 7`), since
   its code already passed under the old unit. Runners leave
   `config/plan-state.json` alone.
8. `tools/lint.sh`, `doxygen Doxyfile`, and grep `include/` and `README.md`
   for the old file name next to the moved class.

## Not in scope yet

A launcher that runs the game in an emulator (PCSX-Redux's GDB server and
Lua), breaks on each MoodCue handler, and screenshots what triggered it. That
would name entities by what they are on screen. It's a large project of its
own, parked until the tracks above are done.
