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

A name of the form `Class<hex>` is the address of the class's method table:
its mechanics are documented in its header, but no name for what it is has
been established (`python3 tools/plan.py` lists any such class as a track 6
job).

### The main subsystems

`src/` is grouped by subsystem. Each file's banner says what it holds, and
`python3 tools/srcpath.py` lists the directories and how many files each has.

| directory | what it holds |
| --- | --- |
| `src/main.c` | the entry point: builds the pool, the application, the screen and the pad, and runs the main loop |
| `src/app/` | the application shell and the object framework: the `BMemPMgr` pool allocator and `BasicClass`, `Application` and `GameApplication`, `FileResource`, the task classes (`TaskCore`, `IntermediateBase`, `StreamTask`) with `Viewport`, and `Pad` |
| `src/cd/` | CD access and the game's files: `CdDriver`, `CdStream` streaming, `LbdFile` and the table of file names |
| `src/graphics/` | the screen, the scene graph and rendering: `DrawSystem`, `SceneNode`, the sprites, the lights, `TmdModel` and the TMD renderer, the Viewport's draw pass, and the loaders that turn TIM, TMD and TOD files into graphics objects (with the tile-map layer and the FMV player) |
| `src/world/` | the dream world and its actors: `DreamSys`, `DayTask` and `StageMap`, the stage grid, `DreamAux`'s triggers, `Actor`, `TodActor` and `Entity`, and `ObjM` with the style layer and `GraphRoom` |
| `src/sound/` | the game's sound: `WBgm` background music and the VAB backend (`VabDriver`, `VabStreamObj`, sound cues), with the map chunks' `PlacementGrid` at the head of its file |
| `src/ui/` | menus and 2D widgets: `TitleMenu` and the `TaskObjF` memory-card saves, `TextEntry` and `ItemList`, `FadeBox`, `BoxFill` and `TextRow` |
| `src/psyq/` | Sony library modules not linked from `lib/`, carried in `src/` as C or `INCLUDE_ASM` instead (`libsnd_*`, `libcd_bios`, `libgs_*`, `libspu_s_ih`, `libcard_card`), each file named for its Sony module; `libsnd_vmanager.c` opens with one game function, `ServiceSoundCueSet` |

Read each named class's header first; its banner points to the units.

- **Boot and the main loop.** `src/main.c` sets up the `BMemPMgr` pool
  allocator (`src/app/BMemPMgr.c`), the `DrawSystem` screen singleton and a
  `Pad`, then runs the root object, `GameApplication` (`src/app/GameApplicationFileResource.c`, a
  subclass of `Application`, `src/app/Application.c`). Its loop shows the intro logos, plays an opening
  movie, runs the title menu, runs a day, and plays the ending movie when a
  year has gone by. The game's file paths (sound banks, each stage's
  textures, music and map chunks, the movies) are one table,
  `src/cd/GameFiles.c`.
- **Scene objects.** `SceneNode` is the positioned 3D object, wrapping a
  libgs `GsDOBJ2` and its coordinate system (`src/graphics/SceneNode.c`); `Actor`
  adds movement (`src/world/ObjMStyleActor.c`). `LinkResource`, `TmdModel`, `ModelData` and `Tod`/`TodSet`
  load models and TOD animations. `Viewport` renders a scene through libgs;
  `FrameClock` is the per-frame tick objects listen to; `Pad` (`src/app/Pad.c`)
  turns the controller into button events; `DrawSystem` (`src/graphics/DrawSystem.c`)
  owns the screen and the VSync loop.
- **The dream.** `DreamSys` (`src/world/DreamSys.c`) is the dream in progress: the
  dream clock, the player's movement, the mood record that picks the next
  day's dream, and the "link" teleport that ends one stage and starts
  another. `Entity` (`src/world/Entity.c`, over `TodActor`, `src/world/TodActor.c`) is a TOD-animated
  actor driven by per-mood tables. `StageGrid` maps mood-graph values to
  stage chunks (`src/world/StageGrid.c`); `DreamAux` (`src/world/DreamAux.c`) spawns the
  dream's trigger entities;
  `StageMap` keeps the seven map chunks around its target loaded, each
  chunk's `PlacementGrid` linked into a lattice of `GridCell`s. `DayTask` runs one day around the
  DreamSys's startDay/endDay (both classes in `src/world/DayTaskStageMap.c`) and
  builds an `ObjM` per stage (`src/world/ObjMStyleActor.c`, with the style layer
  and `StyleEffect`).
- **Screens and menus.** `IntermediateBase` runs one attached job to a
  result. `TaskCore` (`src/app/Task.c`, with `StreamTask`, `IntermediateBase` and
  `Viewport`) is the base of the menu and screen
  tasks: `StreamTask` (plays one movie), `GraphRoom` (the mood graph) and
  `TitleMenu`, the START/FLASHBACK/SAVE/LOAD/GRAPH/SHAKE menu. The 2D pieces are `Sprite` and its subclasses (down to
  `TextRow`), `BoxFill` and `FadeBox` (`src/ui/ScreenWidgets.c`) and `TextEntry`
  (`src/ui/TextEntryItemList.c`).
- **Memory-card saves.** `TitleMenu` owns a `TaskObjF`, the save/load
  controller (both in `src/ui/TitleMenuTaskObjF.c`): a state machine over the
  BIOS memory-card calls, which shows its choices in an `ItemList` scrolling
  list (`src/ui/TextEntryItemList.c` and `src/world/ObjMStyleActor.c`).
- **CD and data sources.** `FileResource` is the base of every class loaded
  from a file; its file interface is bound at run time to the active driver: `CdDriver`
  (`src/cd/CdDriver.c`: blocking file access, the CD request queue, its state
  machines and the file table)
  or `VabDriver`. `TimImage` (`src/graphics/TimImage.c`), `TimArraySrc`, `TimBlockSrc`, `TileMap` and
  `TileAtlas` load and build textures.
- **Movies.** `MoviePlayer` (`src/graphics/GraphicsResources.c`) decodes MDEC FMV from a
  `CdStream` (`src/cd/CdStream.c`, over libcd streaming).
- **Sound.** `VabStreamObj` loads VAB banks, `WBgm` plays background music
  (`src/sound/WBgm.c`), `src/sound/PlacementGridVabSound.c` holds `VabDriver`, `VabStreamObj` and
  the sound-cue set beside the map chunks' `PlacementGrid`, and
  `src/psyq/libsnd_vmanager.c` is Sony's SPU voice manager written as C.

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
| `src/` | the C units, one directory per subsystem (the table under "The main subsystems") |
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
