# LSD: Dream Emulator decompilation

A complete, matching decompilation of **LSD: Dream Emulator** (PlayStation,
1998, Asmik Ace / OutSide Directors Company, SLPS-01556).

The C in `src/` compiles into an executable that is byte-for-byte identical
to the one on the retail disc. Every one of the game's functions is done,
named and documented.

To play it on a PC rather than rebuild the PS1 executable, see
[lsd-port](https://github.com/brian-oblivion/lsd-port), which builds this
same C as a native program.

## Building it

This repository contains no game data and no Sony files. You need your own:

- **The game**: the Japanese retail disc, SLPS-01556. Put the redump
  `.bin`/`.cue` in the repository root, or extract the executable into
  `disk/`:

  ```sh
  python3 tools/extract_exe.py "LSD - Dream Emulator (Japan).bin"
  ```

  It must be `SLPS_015.56` with SHA1
  `76322eeade5ebb22dca57fdeac7d68c30f06308d`. No other version works.

- **Sony's Psy-Q SDK**: these three redump zips from
  <https://archive.org/download/ps1_sdks>, in `sdk/`, still zipped and
  with their original names:

  ```
  Programmer Tool - Runtime Library Version 3.3 (Japan)_DTL-S2190_redump.zip
  Programmer Tool - Runtime Library Version 3.5 (Japan) (En,Ja)_DTL-S2300_redump.zip
  Programmer Tool - Runtime Library Version 3.6 (Japan)_DTL-S2310_redump.zip
  ```

On Linux, with `python3`, `git`, `curl`, `make` and a C compiler:

```sh
./tools/setup.sh          # one time: toolchain, SDK, first verified build
./build-and-verify.sh     # every build after that
```

A good build ends with `OK: build matches retail SLPS_015.56`.

## Reading the code

Start with [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md), a map of how the
game is put together, then `include/basic_class.h`. The game is C written
as objects with hand-made method tables; it looks like C++ but isn't.

| directory | what's in it |
| --- | --- |
| `src/app/` | the application, memory pool, object framework, controller |
| `src/cd/` | reading files from the disc |
| `src/graphics/` | the screen, models, sprites, textures, FMV |
| `src/world/` | the dream itself: the clock, stages, characters, the mood graph |
| `src/sound/` | music and sound effects |
| `src/ui/` | title menu, memory card, menus |
| `src/psyq/` | the few Sony library functions rebuilt from the disassembly |

To change the code and keep it matching, read
[`docs/CONTRIBUTING.md`](docs/CONTRIBUTING.md).

## A note on AI

The decompilation was heavily assisted by AI (Claude Code), and checked at
every step against the retail executable's SHA1. The history of how it was
done is on the `archive/process` branch.

## Licence

This project's own work (the C, headers, configuration and tools) is
[CC0 1.0](LICENSE). The game, Sony's SDK and the maspsx patches in
`tools/patches/` are not; see `LICENSE`.

## Credits

- [FirecatFG/lsddecomp](https://github.com/FirecatFG/lsddecomp), the first
  decompilation attempt on this game, for the starting segmentation and
  symbol names.
- [parasite-eve-2-decomp](https://github.com/GabeRealB/parasite-eve-2-decomp)
  (CC0), for linking Sony's SDK objects and `include/gte_macros.inc`.
- The decomp toolchain: [splat](https://github.com/ethteck/splat),
  [maspsx](https://github.com/mkst/maspsx),
  [m2c](https://github.com/matt-kempster/m2c),
  [asm-differ](https://github.com/simonlindholm/asm-differ),
  [decomp-permuter](https://github.com/simonlindholm/decomp-permuter),
  [objdiff](https://github.com/encounter/objdiff), pcsx-redux's
  psyq-obj-parser, and GCC 2.6.3 from
  [decompme/compilers](https://github.com/decompme/compilers).
- The [compu-lsd wiki](https://compu-lsd.com), for what the game does.
