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
> `OK: build matches retail`. Write build logs inside your worktree (e.g.
> `build/b.log`), never in a shared temp directory. Make one commit per
> coherent change, with a message that says what changed in game terms.
> Never touch `config/plan-state.json`. Before you finish,
> `tools/lint.sh` and `doxygen Doxyfile` must be clean and `git status
> --porcelain` must be empty. Report what you changed, what you left and why,
> and anything that needs a decision.

## Track 14: data in C

The game's tables lived in the disassembly, and the C only saw an
`extern`. A reader never saw which entity links to which stage or which
script it runs. Defined in C, a table reads like this:

```c
EntityMoodRow sEntityMoodTable[ENTITY_MOOD_ROW_COUNT] = {
    /*            unread   unlk  act  dea   aR   pR   lnk  vid  tol  thr  cue  handler */
    /*   0 */ {{  0,   2},   20,   0,   0,   0,   2,  -13,   4,   1,  10,   0, Entity__CuePaceOrLiftOffOnPink},
    ...
```

That table is done (the end of `src/world/entity.c`). splat had cut it into
six labels, wherever code read a column on its own (`sEntityLinkStageTable`
was the linkStage column, read as a flat array); they are one symbol now.

**Procedure** (proven on `sEntityMoodTable`):

1. **Find the table's extent.** Its start is its symbol; its end is the next
   label that is *not* one of its columns (`grep -n dlabel asm/data/*.s`
   around it, and the struct's size times the row count must land on it
   exactly). Only a table whose bytes all belong to one `.c` can move: the
   bytes before and after stay asm, and a run of data that several files
   own splits into one `.data` line per owner.
2. **Dump it:** `python3 tools/datatable.py <symbol> --rows N --layout
   "<the struct's fields>"`. It prints the rows with pointers as symbol
   names, and first lists every label the symbols file places inside the
   range. Those are the column cuts.
3. **Remove the column labels** from `config/symbols.slps01556.lsdde.txt`
   (not the table's own symbol), and turn each reader into a row-field
   access: `sEntityLinkStageTable[i * 16]` became
   `sEntityMoodTable[i].linkStage`, which compiles the same. If a reader
   doesn't match that way, keep the view as a documented macro over the
   table, never as a second symbol.
4. **Write the initializer** in the owning `.c`. If its rows name functions
   of the same file, put it at the end, after them, with an `extern`
   declaration at the top (sized with a `#define` in the header) so earlier
   code can use it. Give pointer fields a type the entries have, so the
   table needs no casts (the mood table's `handler` became
   `EntityMoodCueFn`, and 17 handlers declared with fewer parameters gained
   the unused one: that compiles the same). A wide table goes between
   `/* clang-format off */` and `/* clang-format on */`, one row per line,
   columns aligned under a comment header; the column meanings stay in the
   struct's field docs.
5. **Re-cut splat.** In `config/splat.slps01556.lsdde.yaml`'s data list,
   split the `data` line that holds the table: `[<start>, .data,
   <dir>/<unit>]` at the table's file offset (`vram - 0x80010000 + 0x800`),
   then a new `[<end>, data]` for the bytes after it. The dot form says the
   bytes come from the unit's own `.data` section. The linker packs input
   sections at 2 bytes (`SUBALIGN`), so the object's own 16-byte alignment
   doesn't move anything.
6. `make extract`, `./build-and-verify.sh`. Confirm the bytes come from C:
   `objdump -h build/src/<dir>/<unit>.c.o` shows a `.data` of the table's
   size, and `build/lsdde.map` puts the symbol at its old address.
7. A unit's second table in the same `.data` run needs no new yaml line if
   nothing lies between: its `.data` holds both, in definition order. A gap
   owned by another file splits the run.

**Method tables** (from `data-graphics`, 21 tables):

- The first word is the class id: write it as `<CLASS>_CLASS_ID` from the
  class's header (add the define there if it's missing).
- Check `sizeof(<Class>Methods)` against the table's extent before writing
  it. `RequestedFile`, `CdDriver` and `NullDriver` have tables one slot
  shorter than `FileResource`'s struct (no `processBuffer`), so they need a
  shorter slot list in `include/file_resource.h` first.
- An entry whose function's declared type differs from its slot's (an
  inherited base method on `BasicClass *`, an empty method declared
  `(void)`) is written `(void *)Fn`, with one comment above the run of
  tables saying so; matching entries stay bare so they're still checked.
- **House style: one slot per line, each commented with its offset and
  field name** (`/* +0x008 ctor */ Foo__Foo,`), as `item_list.c`
  and the world tables are. A reader looks a slot up by offset; a bare list
  makes them count. Every table in the tree is written this way.
- **Unlabelled strings a table points at:** `tools/rename.py` refuses a
  name nothing references yet, so write the C with the `D_` name, let the
  link fail once, then rename. A label that sits at the wrong address (a
  column or index-offset alias, like `sCardIconNames` inside
  `sCardEventSpecs`) is moved in the symbols file by hand; its reader
  indexes the true array with `[i - k]`, which compiles the same.
- A function reached only through a table may have no prototype yet: when
  the build says "undeclared", add it to its header. GCC 2.6.3 prints that
  without an `error:` prefix, so grep `^src/.*:[0-9]*: [^w]` too.

**Limits.** A unit's `.data` is one run, so a table that can't move yet
blocks every table after it in the same unit. `.sdata` variables can't
move: `tools/gpsyms.py` takes the `$gp` symbol list from the
`asm/data/*.sdata.s` labels, and a variable defined in C would drop off it
and change how every reader addresses it. `.bss`/`.sbss` hold no
initialized data, so there's nothing to write. `make extract` leaves stale
`asm/data/*.s` files behind; grep for a moved table's `dlabel` still hits
them, but they aren't linked.

What else bites, from the `world` tables:

- **A unit has one `.data` section,** so its C data must be one unbroken
  run, defined in the file in address order. When the order in memory is
  not the order the code wants, define the whole run in one block after the
  includes and drop the scattered `extern`s (`dream_sys.c`, `day_task.c`).
  A function a table names needs a prototype before the table: its header's,
  or one in the `.c` for a function only the table names.
- **GCC word-aligns every array, struct and union** it emits, so the zero
  bytes between two odd-sized arrays come for free: size each array to its
  real length, never with a pad element. A run that ends off a word does
  not: the next unit's `.data` would land 2 bytes early (SUBALIGN), and a
  2-byte `[<end>, data]` line comes out empty. Give it a `[<end>, pad]` line.
- **`const` moves data to `.rodata`.** An `extern const` over `.data` bytes
  loses the `const` when the table is defined.
- **A method table** is a `<Class>Methods` initializer, one slot per line
  (`tools/classtable.py <table>` lists them, and the size is the gap to the
  next label, so trailing NULL slots count). Comment each slot with its
  offset and field name. A slot whose function is declared for another
  class's `self` draws "initialization from incompatible pointer type": cast
  exactly those to `(void *)`.
- **A union initializes through its first member.** `MoodGraphPoint` lists
  its `axis` struct first so a point reads `{{dynamic, upper}}`; `-Wall`
  wants both brace pairs.
- **A column label** (splat's `sTurnRotationYaw` at `&sTurnRotations[0][1]`)
  goes from the symbols file, and the reader writes the row field; the
  address comes out the same.
- **Warnings only show on a recompile.** After the edit that should silence
  them, `touch` the `.c` before reading the log again.

Then each area moves the data it owns. An `extern` stays only for data
another file defines.

## Track 15: named literals, a pass

This is not a campaign to reach zero. A motion amount, a timer length or a
screen coordinate reads fine as a number. Name a literal when the name says
more than the value:

- **Per-script states.** `Entity::state` isn't one enum. Each
  mood-row handler (`Entity__Cue*`) runs its own small script, and values from 11
  up are that script's private steps (1 is the shared `ENTITY_STATE_DONE`).
  Name them locally:

  ```c
  void Entity__CueHoverOverDreamerOnBlueElseRise(Entity *self) {
      enum { HOVER_APPROACH = 11, HOVER_GROW = 12, HOVER_FOLLOW = 13 };
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
The split of `dream_scene.c` (track 19) already put each new file's
defines at its top. `tmd_renderer.c`'s
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

Names that were table positions: 107 `Entity__MoodCueNN` handlers (now
`Entity__Cue<Behaviour>`) and 56 `*__NoOpSlotNN` occupants. A handler's body says what the entity does
("rises, then faces the dreamer"), so it can be named for that now. What the
entity *is* (which character, which stage) needs track 14's table, which says
which stage row NN links to and which video it ends in. Rename them first by
behaviour, then refine when the data is readable. Rename through
`python3 tools/rename.py`.

## Track 19: one class, one file

`dream_scene.c` held half of `ItemList`, `ObjM`, the style layer,
`StyleEffect`, `Actor`, `VariantSprite` and `GraphRoom`; it is now seven
files, and ItemList's first half, which sat at the tail of the TextEntry
unit right before it, has joined the second in `src/ui/item_list.c`. The
other multi-class units went the same way (`split-rest`). A split keeps ROM
order: each new file takes a
contiguous run of functions, and a `.c` is one splat subsegment, so cutting
a class out of the middle of a unit makes three files, not two.

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
4. **Split the data.** A unit's C tables are one run in the data list
   (track 14), in the same class order as its text. Give the new file its
   own `- [<offset>, .data, <dir>/<file>]` line at its first table's file
   offset, in address order, and move its tables with its code. The old
   unit's data must stay one run too, so **cut from the unit's end
   backwards**: taking the last class leaves a prefix, while taking a
   middle class leaves the old unit's data on both sides of the new file's
   and the link cannot place it. Check `objdump -h` on each object: its
   `.data` is the gap to the next line.
5. **Split the rodata, if the new file owns any.** Find the unit's
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
6. **Move the C.** The run of functions, their section banner and the
   `#define`s and `extern`s only they use go to the new `.c`, defines at the
   top after the includes. Give the file a header comment in the style of
   its siblings (what class, which functions, in ROM order). Include only
   what it needs; the build says what is missing. Fix the header's
   `Methods in src/...` line and the old file's header comment.
7. **Rebuild from a fresh extract:** `make extract`, then
   `./build-and-verify.sh`. Wrong C, a wrong `.data` offset or a misplaced
   jump table fails the link or the SHA1. A wrong **text** offset does not:
   the objects carry the bytes, so a `c` line at the wrong offset (in
   address order still) links green. Check each new line against
   `build/lsdde.map`: the object's `.text` address, `- 0x80010000 +
   0x800`, must be the line's offset. Check the new file compiled without
   implicit-declaration warnings (`grep -A2 '<file>.c:' /tmp/b.log`).
8. **Ledger:** the head marks the new unit passed where the old one was
   (`plan.py mark-unit --unit <file> --track 3`, then `--track 7`), since
   its code already passed under the old unit. Runners leave
   `config/plan-state.json` alone.
9. `tools/lint.sh`, `doxygen Doxyfile`, and grep `src/`, `include/` and
   `README.md` for the old file name next to the moved class.

What else bites, from the `dream_scene.c` split:

- A caller moved away from its callee loses the in-file prototype:
  `declcheck` (LOCAL) refuses a prototype in a `.c` for another unit's
  function, so the callee's header must declare it (the style layer got
  `include/style_layer.h` that way).
- `apidoc.py` limits a `.c` comment block to 20 lines; a new file's header
  comment must fit.
- `config/typeviews-warnings.txt` is keyed by unit: a warning that moves
  with its function is re-keyed to the new unit.
- `tools/unitfile.py rename`, used when the old file keeps none of what its
  name says, rewrites every token of the old name, including
  `config/plan-state.json` and this file. A runner reverts both and leaves
  the ledger to the head; fix mentions that would turn wrong first.
- `unitfile.py rename` refuses when `include/<new>.h` exists beside
  `include/<old>.h`: fold what the old header declares into the class's
  header by hand and delete it first (`dream_day.h` went into
  `day_task.h`). An umbrella header two units shared goes the same way:
  its declarations to the class headers that own them, its includers to the
  class headers they use (`task.h`, whose one declaration was TaskCore's).

What else bites, from `split-rest`:

- **The first class is the one a split leaves behind.** A file named for a
  later class (`sprite.c` held Sprite third, `title_menu.c` TitleMenu
  third, `game_files.c` its record table after LbdFile) keeps its name by
  a split and a rename in one step: `git mv` the old file to the first
  class's name, rename its text, `.rodata` and `.data` lines, and create
  the named file with the later run.
- **A define or local type two new files share** goes to the header the
  sibling includes anyway, with a doc comment: the tile grid's size to
  `tile_map.h`, `CLUT_FADE_Y` to `tim_block_src.h`, `SubBlockTable` to
  `tod_set.h`, `UnprototypedCtorTable` to `file_resource.h`, playTone's
  packed index to `vab_stream_obj.h`, the save title's layout to
  `title_menu.h`.
- **An `INCLUDE_ASM` path names the unit** (`asm/nonmatchings/<dir>/<unit>`):
  a NON_MATCHING body moved to a new file takes the new unit's path
  (`unitfile.py rename` does it for a rename).
- **A Sony function carried as C at a unit's edge** becomes its own
  `src/psyq/<lib>_<obj>.c` unit, as `libgs_gs_101` and `libgs_gs_124`
  are (`GsSetProjection`, `libgs_gs_106`, was the tail of `task.c`).
- **Put a new line after the old unit's history comments** when they
  describe the old unit, not the next one: those blocks read as the old
  line's notes.
- A file's directory follows its class, not the unit it was carved from:
  `grid_cell.c` and `node_guarded_viewport.c` went to `src/world`, beside
  their only makers, out of the title-menu unit in `src/ui`.

## Not in scope yet

A launcher that runs the game in an emulator (PCSX-Redux's GDB server and
Lua), breaks on each MoodCue handler, and screenshots what triggered it. That
would name entities by what they are on screen. It's a large project of its
own, parked until the tracks above are done.
