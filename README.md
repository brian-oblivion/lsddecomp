# lsddecomp

A matching decompilation of **LSD: Dream Emulator** (PlayStation, 1998,
Asmik Ace / OutSide Directors Company, SLPS-01556).

"Matching" means the C in `src/` compiles, with the same Psy-Q GCC 2.6.3
toolchain the game was built with, into an executable that is
**byte-for-byte identical** to the one on the retail disc. A paraphrase of a
program can be subtly wrong without anyone noticing. A byte-exact rebuild
can't be.

## What is and is not in this repository

**This repository contains no game data and no Sony files.** There is no
executable, disc image, asset, SDK library or SDK header in it, and there
never will be. You bring your own copies, and the build reads them from
directories git ignores:

| you provide | where | what the build does with it |
| --- | --- | --- |
| the game executable `SLPS_015.56` (or the disc image it comes from) | `disk/` | disassembles it (`asm/`, generated), and checks the rebuilt image against it |
| Sony's Psy-Q "Runtime Library" SDK discs | `sdk/` | converts Sony's library objects into `lib/` and copies Sony's headers into `include/psyq/`, both generated and ignored |

What *is* committed is the reconstruction: the C source in `src/`, the
headers in `include/` that describe the game's structures and classes, the
splat configuration and symbol names in `config/`, and the tools. The C
covers the game's own code, plus a few Sony library functions that no
shipped SDK object matches (`src/psyq/`: C where it was rebuilt from the
disassembly, and elsewhere an `INCLUDE_ASM` line that splices in the
disassembly generated from your executable at build time).

## Progress

**The rebuilt executable matches retail byte for byte.** 1443 of the game's
1446 functions are C. The other three, `MoviePlayer__Advance`,
`MoviePlayer__DecodeFrame` and `New_GameApplication`, have readable C under
`#ifdef NON_MATCHING` that doesn't yet compile to the same bytes, so the
build takes them from the disassembly (`make nonmatching` builds the C).
Sony's library code is linked from Sony's own objects (below) rather than
decompiled. Every game header is
documented, and every class has one definition.

What's left is making the code read like the game's own source rather than a
decompilation: data tables written as C, comments in game terms, code shaped
by the compiler rewritten, one class per file. That's
[`docs/CLEANUP.md`](docs/CLEANUP.md).

To measure it yourself:

```sh
python3 tools/progress.py      # matched functions and bytes, per file
python3 tools/readability.py   # remaining readability debt: magic numbers, raw offsets, unk fields
python3 tools/plan.py          # the cleanup tracks and the ready jobs
```

## Building it

You need Linux, `python3`, `git`, `curl`, `make`, `sha1sum` and a
host C compiler, plus:

- **The game.** The Japanese retail release, SLPS-01556. Put the redump
  `.bin`/`.cue` in the repository root, or extract the executable yourself
  into `disk/`:

  ```sh
  python3 tools/extract_exe.py "LSD - Dream Emulator (Japan).bin"   # writes disk/SLPS_015.56
  ```

  The expected file is `SLPS_015.56`, 505856 bytes, SHA1
  `76322eeade5ebb22dca57fdeac7d68c30f06308d`. No other revision is
  supported. A different hash is a different build, and every address in
  `config/` would be wrong for it.

- **The Psy-Q SDK.** The "Programmer Tool - Runtime Library" discs, as
  redump zips (don't unpack them), in `sdk/`. They are on archive.org
  (<https://archive.org/download/ps1_sdks>):

  ```
  Programmer Tool - Runtime Library Version 3.3 (Japan)_DTL-S2190_redump.zip
  Programmer Tool - Runtime Library Version 3.5 (Japan) (En,Ja)_DTL-S2300_redump.zip
  Programmer Tool - Runtime Library Version 3.6 (Japan)_DTL-S2310_redump.zip
  ```

  The version is read from the file name, so keep the original names.
  `python3 tools/psyq_sdk.py install` names any disc that is missing.

Then:

```sh
./tools/setup.sh          # toolchain, venv, SDK conversion, split, and a verified first build
./build-and-verify.sh     # build, then compare the whole image with retail
```

`setup.sh` fetches the prebuilt GCC 2.6.3, builds `mipsel-linux-gnu`
binutils, installs splat into a venv, and clones maspsx, m2c, asm-differ and
decomp-permuter. It converts the SDK discs, runs `make extract`, and stops unless the result
rebuilds byte for byte. After that, `./build-and-verify.sh` is the only build
command. It ends `OK: build matches retail SLPS_015.56` when the image is
exact. A bare `make` produces bytes but doesn't tell you whether they are
right.

### Why the SDK discs

The game links Sony's Psy-Q libraries (libapi, libc2, libcard, libcd, libetc,
libgs, libgte, libpress, libsnd, libspu). The build doesn't re-derive that
code as C. It links **Sony's own objects**, placed exactly where the game put
them. The game mixed library builds from several SDK releases, so which disc
supplies each object was measured against retail and recorded in
`config/psyq-objects.txt`. `config/psyq-headers.txt` lists the headers the
same way, each with the hash of the generated file. The handful of Sony
functions no shipped object matches live in `src/psyq/`.

## What the code is

The game is one plain PS-X executable: no overlays, no compression, and all
of its code in one file. Its own code is C89, written on top of Sony's
libraries around a **hand-rolled class framework**.

**It looks like C++, but it isn't.** Every object starts with a pointer to
its method table, which is a flat array of function pointers stored as
ordinary data. Methods are plain C functions named `Class__Method` that
take the object as an explicit first parameter, `self`. Calls go through the
table: `self->methods->addChild(self, child)`. A class's constructor sits in
table slot +0x008, and `New_Class` allocates an object and calls its
constructor through the table. Word +0x000 of each table is a nibble-path
class id. For example, TextRow `0x11144` sits under CharSprite `0x1144`,
which sits under ScreenSprite `0x144`, then Sprite `0x44`, then SceneNode
`0x4`, then BasicClass `0x0`.

Each class has one header, `include/<class>.h`, which holds its object
struct, its method-table struct and its prototypes, all documented for
Doxygen (`doxygen Doxyfile`, output in `build/doxygen/`).
`include/basic_class.h`, the root class, is the place to start. Each `.c`
file opens with a banner saying what it holds.

| directory | what it holds |
| --- | --- |
| `src/main.c` | the entry point: memory pool, application, screen, pad, main loop |
| `src/app/` | the application shell and the object framework: the `BMemPMgr` pool allocator and `BasicClass`, `Application`/`GameApplication`, `FileResource`, the task classes, `Viewport`, `Pad` |
| `src/cd/` | CD access: `CdDriver`, `CdStream` streaming, `LbdFile`, the table of the game's files |
| `src/graphics/` | the screen, scene graph and rendering: `DrawSystem`, `SceneNode`, sprites, lights, `TmdModel` and the TMD renderer, the TIM/TMD/TOD loaders, tile maps, the FMV player |
| `src/world/` | the dream itself: `DreamSys` (dream clock, movement, the mood record, the "link" that moves you between stages), the stage grid, `DayTask`, `StageMap`, `Actor`, `Entity`, `ObjM`, the mood graph screen |
| `src/sound/` | `WBgm` background music, VAB banks, sound cues |
| `src/ui/` | the title menu and memory-card saves, text entry, list and box widgets |
| `src/psyq/` | Sony library functions that no SDK object matches, under Sony's names |

To go from a function to its class and callers:

```sh
grep -rn 'TaskObjF__SetState' src/ include/         # definition, prototype, direct calls
.venv/bin/python3 tools/classtable.py gTaskObjFMethods   # the method-table slot it fills
```

## Changing the code and keeping it matching

Every change is checked the same way: edit, build, verify.

```sh
./build-and-verify.sh > /tmp/build.log 2>&1; echo "build exit=$?"
grep -nE 'error:|parse error|undefined reference|\*\*\* \[[^]]*\.o\]' /tmp/build.log | head
.venv/bin/python3 tools/funcdiff.py <func>          # how many words match, for one function
.venv/bin/python3 tools/asm-differ/diff.py <func>   # side-by-side diff against retail
```

- **exit 1**: `disk/SLPS_015.56` is missing or is the wrong dump.
- **exit 2 with a grep hit**: the C didn't compile. GCC 2.6.3 predates the
  `error:` prefix, so the `*** [...o]` pattern, which matches make failing
  on an object file, is the reliable signal.
- **exit 2 and no hit**: it compiled, but the image differs from retail.
  `cmp -l build/SLPS_015.56 disk/SLPS_015.56 | head` gives 1-based file
  offsets; the address is `(offset - 1) - 0x800 + 0x80010000`, and
  `build/lsdde.map` names the function at that address.

The rules that break the image if you ignore them:

- **Functions stay in ROM-address order within a file.** A function out of
  order still links, but the image comes out wrong in functions you didn't
  touch.
- **C89, as GCC 2.6.3 reads it.** Declarations go at the top of a block.
  Only `/* */` comments. `char` is unsigned, so a signed byte is `s8`.
- **A rodata string is a symbol, not a literal.** The disassembly already
  contains every rodata string, so writing the literal in C emits a second
  copy and shifts the image. Declare the existing symbol instead
  (`extern const char sTitleTimPath[];`).
- **A struct edit is never local.** A field inserted without shrinking the
  padding beside it moves every later offset, and breaks functions in other
  files. Rebuild the whole image after any header change.
- **An odd spelling that exists to match gets one `MATCHING:` comment**
  beside it in the `.c` file.
- **The toolchain is pinned.** The compiler, its flags and `maspsx` (which
  reproduces Sony's assembler's macro expansions) are part of the retail
  bytes, not knobs to turn.

Rename through the tools, which update every reference and re-verify:

```sh
python3 tools/rename.py OLD NEW          # a function or global
python3 tools/renametype.py OLD NEW      # a type or class family
python3 tools/unitfile.py rename OLD NEW # a source file
```

Never edit `asm/` (it is regenerated) or `check.sha1` (it is the retail
hash). Run `tools/lint.sh` before sending a change.

## Continuous integration

- **`lint`** runs on every push and pull request: formatting (clang-format
  22), the API documentation check, the readability counters, one
  declaration per name, snake_case file names, and a Doxygen build that fails
  on any warning. The two checks that preprocess the source need Sony's
  headers, so they run only where the headers are available.
- **`build`** rebuilds the executable, checks it against `check.sha1`, and
  publishes the objdiff report [decomp.dev](https://decomp.dev) reads. The
  disc and the SDK never enter this repository, so the job reads them from a
  private repository the owner keeps (`disk/SLPS_015.56`, `lib/` and
  `include/psyq/`), named in the `LSD_DEPS_REPO` variable and read with the
  `LSD_DEPS_TOKEN` secret. Without the secret the job skips.

## An experiment in AI-driven decompilation

This project was an experiment to find out how far a matching decompilation
could get if **the AI did all of the decompilation**. Every function in
`src/`, every header, every name and comment, and nearly all of the tooling
was written by Claude agents (Anthropic's Claude Code). The human operator
supplied the disc and the SDK, set the goal, pasted the round prompt, and
made the calls that were theirs to make: the licence, what to publish, and
the plan's direction. The operator didn't write the C.

It went from an empty splat split on 2026-08-28 to every game function
matched, named and documented on 2026-09-29, in 106 rounds and about 6800
commits. A round was one **head agent** reading a written plan, measuring
the project with its own tools, and handing units of work to up to five
**runner agents**, each in its own git worktree on one source file. The head
then checked every claimed match against the whole-image SHA1 before
merging anything. Cheaper models took mechanical work, stronger models took
matching and naming, and the strongest one revised the plan and the rules
between rounds.

What made it work, and what the experiment taught:

- **A byte-exact oracle.** An agent can't talk its way past a SHA1. Every
  claim was checked by `./build-and-verify.sh` rather than by the agent that
  made it. Hooks block the obvious ways around it: editing the hash files,
  editing generated disassembly, or running a bare `make`.
- **Measure, don't transcribe.** Project state lived in tools that measure
  the tree (`progress.py`, `readability.py`, `plan.py`), never in prose.
  Numbers written into documents went stale within a round and sent later
  agents to redo finished work.
- **Wrong causes cost more than wrong scores.** A wrong score gets corrected
  the next time anyone measures. A wrong diagnosis (a stall blamed on the
  compiler, a function blamed on the game that was really Sony's) is what
  the next round acts on. Several stood for many rounds before a screen
  caught them.
- **The rules grew with the agents.** The working documents grew to
  thousands of lines as each round wrote down what the last one got wrong.
  They were distilled repeatedly, and at the end they moved off `main`.

The whole process record is on the **`archive/process`** branch: the
finishing plan, the parallel-run protocol, the round-by-round log, a report
for every function saying how it matched and why it has its name, and the
research notes behind the build and the class model.

## Licence

The original work in this repository (the C source, the project headers, the
configuration and the tools) is dedicated to the public domain under
[CC0 1.0](LICENSE). That covers only this project's own contribution. See
`LICENSE` for what it doesn't cover: the game itself, Sony's SDK, and the
maspsx patches in `tools/patches/`.

## Credits

- **[FirecatFG/lsddecomp](https://github.com/FirecatFG/lsddecomp)**, the
  first decompilation attempt on this game. This project started from its
  splat segmentation, `$gp` value and symbol names: 146 names, which were
  hand-written or inferred by `ghidra_psx_ldr` and never the developers' own,
  so every one was treated as a hypothesis. The C and the headers here were
  written independently. lsddecomp's own research notes live in its
  repository.
- **[parasite-eve-2-decomp](https://github.com/GabeRealB/parasite-eve-2-decomp)** (CC0), for linking Sony's
  SDK objects instead of decompiling them, and for `include/gte_macros.inc`.
- **The decomp toolchain:** [splat](https://github.com/ethteck/splat),
  [maspsx](https://github.com/mkst/maspsx),
  [m2c](https://github.com/matt-kempster/m2c),
  [asm-differ](https://github.com/simonlindholm/asm-differ),
  [decomp-permuter](https://github.com/simonlindholm/decomp-permuter),
  [objdiff](https://github.com/encounter/objdiff) and pcsx-redux's
  psyq-obj-parser, and the prebuilt GCC 2.6.3 from
  [decompme/compilers](https://github.com/decompme/compilers).
- **The [compu-lsd wiki](https://compu-lsd.com)**, for what the game does.
