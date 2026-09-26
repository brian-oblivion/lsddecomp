# lsddecomp2

A matching decompilation of **LSD: Dream Emulator** (PlayStation, 1998, Asmik
Ace / OutSide Directors Company, SLPS-01556).

"Matching" means the C in `src/` compiles, through the Psy-Q GCC 2.6.3
toolchain Sony shipped, to an executable that is **byte-for-byte identical**
to the one on the disc. A paraphrase can be wrong unnoticed; a byte-exact
rebuild cannot.

**Status:** every game function is C and the build reproduces retail. What
remains is readability: names, types and docs. Measure it rather than trust
this line:

```sh
python3 tools/progress.py      # matched and queued functions, per unit
python3 tools/plan.py          # the finishing plan's tracks and what is left
```

## Building it

You need your own copy of the game and of the Psy-Q SDK discs. Nothing in
this repository contains game data or Sony code.

```sh
git clone <this repo> lsddecomp2 && cd lsddecomp2
cp ~/path/to/'LSD - Dream Emulator (Japan).bin' .   # or put SLPS_015.56 in disk/
cp ~/path/to/'Programmer Tool - Runtime Library Version 3.5 (Japan)_DTL-S2300_redump.zip' sdk/
# ...plus every other Runtime Library disc the manifest needs (below)
./tools/setup.sh
```

`tools/setup.sh` checks the host tools (`python3`, `git`, `curl`, `make`,
`sha1sum`, a C compiler), extracts `SLPS_015.56` from the disc image if it
finds exactly one, verifies it against `check.sha1`, sets up a Python venv,
clones maspsx, asm-differ, m2c and decomp-permuter, fetches the prebuilt GCC
2.6.3, builds a `mipsel-linux-gnu` binutils, converts the SDK discs in `sdk/`
into `lib/`, runs the split, and **proves the result rebuilds byte-for-byte**.
If that fails it stops. `disk/README.md` covers extracting the executable by
hand and which dump is expected.

After that there is one build command:

```sh
./build-and-verify.sh          # build, then compare the whole image with retail
```

Use it instead of `make`: a bare `make` produces bytes and says nothing about
whether they are the right ones.

### Where the SDK comes from

The game linked Sony's Psy-Q runtime libraries, and so does this build.
Rather than re-derive that code as C, it links **Sony's own objects**
(libapi, libc2, libcard, libcd, libetc, libgs, libgte, libpress, libsnd,
libspu), taken from the "Programmer Tool — Runtime Library" discs and placed
exactly where the game put them, each as a splat `o` segment. The discs are
on archive.org; `sdk/README.md` has the link. The game mixed library builds,
so objects come from more than one disc version: which disc owns which object
is measured against retail and recorded in `config/psyq-objects.txt`. The
manifest currently draws on the 3.3, 3.5 and 3.6 discs; to see which you are
missing:

```sh
.venv/bin/python3 tools/psyq_sdk.py install    # names any missing disc by its exact file name
```

Sony code no disc's object matches stays as disassembly in the `psyq_*`
segments, or sits in a game unit whose header says so.

## What the code is

One plain PS-X executable: no overlays, no compression, every byte of code in
one file. The game's own code is C89 built on a **hand-rolled class
framework**, over Sony's libraries.

### The class framework

It looks like C++ and is not (the proof is
`docs/research/class-framework.md`). Every object starts with a pointer to
its method table, a flat array of function pointers that is ordinary data.
Methods are C functions named `Class__Method` taking the object explicitly as
their first parameter, `self`, and calls go through the table:
`self->methods->addChild(self, child)`. Each class has an allocator
`New_Class`, and its constructor sits in table slot +0x008.

Word +0x000 of every table is a **nibble-path class id**: each nibble above
the lowest is one more level of derivation. TextRow is `0x11144`, below
CharSprite `0x1144`, ScreenSprite `0x144`, Sprite `0x44`, SceneNode `0x4`
and BasicClass `0x0`. The id is a first guess; the constructor chain (each
ctor calls its parent's first) is the real inheritance, and
`plan.py classes` marks where the two disagree.

`include/BasicClass.h` is the worked example. BasicClass is the root: every
table starts with its slots (release, ctor, finalize, a child list, a list of
parent back-references, and `notifyParents`/`onNotify` events, which is how
objects talk to each other). Each class has exactly one header,
`include/<Class>.h`, holding its object struct, its method-table struct and
its prototypes; a class with subclasses exports `FIELDS`/`SLOTS` macros its
children expand first. Each header's banner says what the class does, which
units hold its methods and who builds it.

```sh
python3 tools/plan.py classes        # every class: id, table, parent, header
python3 tools/typeviews.py --tree    # the id tree from the method tables
python3 tools/classtable.py <table>  # a table's slots and their occupants
python3 tools/classtable.py <table> --vs <parent-table>   # what a subclass overrides
```

A name like `Class865C8` is the address of the class's method table: its
mechanics are documented in its header, but no name for what it is has been
established.

### The main subsystems

Read each named class's header first; its banner points to the units.

- **Boot and the main loop.** `src/main.c` sets up the `BMemPMgr` pool
  allocator (`src/code_8220.c`), the `DrawSystem` screen singleton and a
  `Pad`, then runs the root object, `GameApplication` (a subclass of `Application`,
  `src/code_1677c.c`). Its loop plays the intro logos and the day-of-week
  movie, polls the graph-screen tasks against the dream's status, and can
  start a cinematic.
- **Scene objects.** `SceneNode` is the positioned 3D object, wrapping a
  libgs `GsDOBJ2` and its coordinate system (`src/code_d294*.c`); `Actor`
  adds movement. `LinkResource`, `TmdModel`, `ModelData` and `Tod`/`TodSet`
  load models and TOD animations. `Viewport` renders a scene through libgs;
  `FrameClock` is the per-frame tick objects listen to; `Pad` turns the
  controller into button events.
- **The dream.** `DreamSys` (`src/DreamSys.c`) is the dream in progress: the
  dream clock, the player's movement, the mood record that picks the next
  day's dream, and the "link" teleport that ends one stage and starts
  another. `Entity` (`src/Entity*.c`, over `TodActor`) is a TOD-animated
  actor driven by per-mood tables. `StageGrid` maps mood-graph values to
  stage chunks; `code_4cd08` spawns and despawns trigger entities;
  `StageMap` keeps the seven map chunks around its target loaded, each
  chunk's `PlacementGrid` linked into a lattice of `GridCell`s (`src/class_3ac78.c`,
  `src/class_3bb8c.c`, `src/class_3bb8c_b.c`). `Class865C8` runs a day's
  loop around the DreamSys and builds an `ObjM`.
- **Screens and menus.** `IntermediateBase` runs one attached job to a
  result. `TaskCore` (`src/code_2cc8c*.c`) is the base of the menu and screen
  tasks: `StreamTask` (plays one movie), `GraphRoom` (the mood graph) and
  `TitleMenu`, the START/FLASHBACK/SAVE/LOAD/GRAPH/SHAKE menu. The 2D pieces are `Sprite` and its subclasses (down to
  `TextRow`), `BoxFill` and `TextEntry`.
- **Memory-card saves.** `TitleMenu` (`src/class_3bb8c_c.c`, `_d.c`) owns a
  `TaskObjF` (`src/class_3bb8c_d.c` to `_g.c`), the save/load controller: a
  state machine over the BIOS memory-card calls, which shows its choices in a
  `ItemList` scrolling list (`src/class_3bb8c_j.c`, `_k.c`).
- **CD and data sources.** `FileResource` is the base of every class loaded
  from a file; its file interface is bound at run time to the active driver: `CdDriver`
  (`src/code_179d8_o.c`, `_q.c`, `_s.c`: the CD request queue and file table)
  or `VabDriver`. `TimImage`, `TimArraySrc`, `TimBlockSrc`, `TileMap` and
  `TileAtlas` load and build textures.
- **Movies.** `MoviePlayer` (`src/code_33808.c`) decodes MDEC FMV from a
  `CdStream` (`src/code_3770c.c`, over libcd streaming).
- **Sound.** `VabStreamObj` loads VAB banks, `WBgm` plays background music
  (`src/code_2a0e0.c`), and the other `src/code_179d8_*.c` units hold the
  sequencer timer and SPU voice code, some of it Sony's `libsnd` written as C.

Units named `code_<offset>` or `class_<offset>` are named for where their
block starts in the file, not for what they hold; each unit's header comment
says that.

### From a function to its class and callers

```sh
grep -n 'TaskObjF__SetState' config/symbols.slps01556.lsdde.txt   # its address
grep -rn 'TaskObjF__SetState' src/ include/                        # definition, prototype, direct calls
python3 tools/classtable.py gTaskObjFMethods                       # the slot it fills
```

A method's name gives its class, and the class's header has its table
struct. Most calls go through a slot, so grep the slot name as well
(`->setState(`). Every function has a report, `docs/match-reports/<func>.md`,
with how it matched and the evidence for its name.

## Layout

| path | what |
| --- | --- |
| `src/` | the C units |
| `include/` | one header per class, subsystem headers, and the Psy-Q SDK headers in `include/psyq/` |
| `config/` | splat segmentation, symbol names, the SDK object manifest |
| `asm/` | disassembly generated by `make extract`. Never edited or committed. |
| `disk/` | your `SLPS_015.56`. Never committed. |
| `sdk/`, `lib/` | your SDK discs, and the Sony objects converted from them. Never committed. |
| `tools/` | the toolchain and the project's tools |
| `docs/` | the guides; `docs/match-reports/` is the per-function record |

## Contributing

- [`CLAUDE.md`](CLAUDE.md): the operating manual, for people as much as for
  agents, including the rules the hooks in `.claude/hooks/` enforce.
- [`docs/FINISHING-PLAN.md`](docs/FINISHING-PLAN.md): what "done" means and
  what is left.
- [`docs/MATCHING-GUIDE.md`](docs/MATCHING-GUIDE.md): the per-function loop.
- [`docs/DECOMPILATION_LEARNINGS.md`](docs/DECOMPILATION_LEARNINGS.md): the
  source idioms that reach retail's bytes.
- [`docs/PARALLEL-RUNS.md`](docs/PARALLEL-RUNS.md): how the project runs
  parallel agent sessions.

## Credits

This project stands on [FirecatFG/lsddecomp](https://github.com/FirecatFG/lsddecomp)
for its original segmentation and symbol names, and on the tools the console
decomp scene shares. [`CREDITS.md`](CREDITS.md) says exactly what was taken
and why.
