# Small data in C

The game's small globals (`.sdata`, initialized, and `.sbss`, zeroed) are
still defined in the disassembly. The C sees them only as `extern s32
sDivClipWidth;`, so a reader can't see that it starts at 320. This job moves
them into the C units that own them, one unit at a time, with the image
byte-identical after every unit.

Read CLAUDE.md first. Its hard rules and its build-and-verify loop apply to
every step here.

## How it works

The game reaches small data off `$gp`. maspsx emits a gp-relative access
only for a name listed in `config/gp-symbols.txt`, and `tools/gpsyms.py`
builds that list (it runs at the end of `make extract`) from two sources:

- the labels splat writes for the plain `sdata`/`sbss` segments in
  `config/splat.slps01556.lsdde.yaml`;
- the symbols-file entries inside every range a yaml line hands to a C
  unit: `[0x7B024, .sdata, graphics/tmd_renderer]`, up to the next yaml
  line.

In the C, `SDATA` and `SBSS` (`include/common.h`) put a definition in its
section:

```c
/* The clip area every DIVPOLYGON gets: 320 x 240, the screen. */
static s32 sDivClipWidth SDATA = 320;
static s32 sDivClipHeight SDATA = 240;

static Viewport *sStyleEffectViewport SBSS = NULL;
```

An `SBSS` definition needs its `= 0` (or `= NULL`): GCC 2.6.3 ignores a
section attribute on a definition with no initializer.

gpsyms.py checks the two sides agree. Every symbol in a C-owned range needs
a `src/` definition marked with the right macro, and every marked definition
must sit in such a range. A mismatch is fatal, so `make extract` stops on it.
Its messages say which side is missing.

`tmd_renderer.c` (five `.sdata` variables) and `style_effect.c` (four
`.sbss` pointers) are done and serve as the worked examples.

## What's left

```sh
python3 tools/smalldata.py        # one line per run: address, section, owning unit, count
python3 tools/smalldata.py -v     # every symbol in each run, with its first data word
```

A run is a stretch of consecutive labels that the same `src/` files name.
One user means that unit owns the run. The tool flags the runs to read by
hand, `SHARED` and `NO USER`, which are covered under "Cases" below.

## One unit, step by step

1. **Find the unit's range.** Look it up in `tools/smalldata.py -v`: the
   first symbol's address, and the address where the next run starts. A
   unit can own more than one run (`cd_driver`, `title_menu`, `text_entry`
   do, split by a `SHARED` or `NO USER` symbol that also belongs to them).
   Take the whole stretch.
2. **Split the yaml.** Find the `sdata`/`sbss` line whose range holds it and
   add the unit's line, then a plain line where the unit's range ends:

   ```yaml
         - [0x7B018, sdata]
         - [0x7B024, .sdata, graphics/tmd_renderer]  # what the variables are
         - [0x7B038, sdata]
   ```

   Addresses in the yaml are file offsets (`vram - 0x80010000 + 0x800`). If
   the range already ends at an existing line, don't add a plain line.
3. **Write the definitions** in the unit, in address order, all in one
   place. GCC emits initialized data in definition order, so the order in
   the file must be the order in the image. Delete the `extern`s they
   replace. Take each value from the `-v` listing (or from
   `asm/data/<off>.sdata.s` before you split it; splat writes no labels
   for the range afterwards). Write values the way the code uses them:
   `320`, not `0x140`; a `ColorRgb` as `{128, 128, 128}`; a pointer to
   another symbol as `&sName` or the array's name.
4. **`static` or not.** A symbol only this unit names is `static`. One that
   other units name keeps external linkage, and the others keep their
   `extern` (see `SHARED`).
5. **Rebuild:** `make extract`, then the verify chain from CLAUDE.md. A
   red build with no compile error usually means the order is wrong, a
   value is wrong, or the padding differs (a 3-byte `ColorRgb` followed by
   an `s32` pads to 4 by itself; a string's size is its bytes plus the NUL,
   rounded up to 4 by the next word's alignment). Use `cmp -l` as in
   CLAUDE.md to find the byte.
6. **Commit** one unit per commit, with a message in game terms: what the
   variables are and what they start as. `tools/lint.sh` and `doxygen
   Doxyfile` clean first.

## Cases

- **SHARED** (a symbol more than one unit names). The owner is the unit
  whose range the symbol sits in, not whichever uses it most: `sCdBusy`
  sits inside `cd_driver`'s run, so `cd_driver.c` defines it and
  `data_source.c` keeps `extern`. `sDefaultGridSpan`, between `day_task`
  and `title_menu`, belongs to `stage_map`, the unit between them in link
  order (the `c` lines of the yaml, top to bottom).
- **NO USER** (an unnamed `D_` label). Mostly short strings the code reaches
  through a pointer that is itself small data: `D_8008A958`, `"CDI\\"`, is
  what `sDefaultDataDirectory` (game_files) points at. Its owner is decided
  by its neighbours and by what points at it. Name it with
  `tools/rename.py` before defining it (`sDefaultDataDirectoryName` or
  similar), then define it as a `char` array: `static char sName[] SDATA
  = "CDI\\";`. The other cases are a `D_` inside `title_menu`'s run (the
  string `"7654321"`, which title_menu.c's comments mention), one inside
  `text_entry`'s run (a word, `-12`), `"SND\\SE"` after `game_files`, and
  one `.sbss` word between `style_layer` and `style_effect`.
- **Sony's data.** Some small data belongs to Sony's objects, already `o`
  segments in the yaml (`libc2/prnt`, `libc2/puts`, `libc2/itoa`), and the
  `.sbss` word at `0x7B468` is libc2/rand's seed, placed by
  `config/psyq-objects.ld`. Never define those in C. If a `D_` label turns
  out to be Sony's (`tools/sonydata.py`), leave it alone.
- **`.sbss` order.** The `.sbss` runs are not in code order (`main`,
  `tmd_model`, `style_layer`, `style_effect`, `dream_sys`, `entity`, then
  `tmd_renderer` last). Follow the addresses, never the text order.
- **Large `.sbss` objects.** `tmd_renderer`'s `sDivPolygon3`/`sDivPolygon4`
  are 0x218 bytes each. Define them with `= {0}` and check the section and
  size in the object (`objdump -h build/src/graphics/tmd_renderer.c.o`).
  If a big zero initializer won't place, leave them for last and say so.
- **A unit with a lot of it.** `title_menu` (about 30 symbols), `task_objf`
  (about 20) and `cd_driver` (about 20) are the big ones. Do a small unit
  first to get the routine down.

## Parallel runs

Units are independent, but two agents in one file conflict, and every unit
edits the same few lines of the yaml. Give each agent its own worktree
(`tools/setup-worktree.sh <name>`) and its own area (`app`+`cd`, `graphics`,
`sound`+`ui`, `world`), and merge them one at a time. Resolve a yaml
conflict by keeping both sides' lines in address order, then `make extract`
and rebuild after each merge.

## Done when

`python3 tools/smalldata.py` lists only Sony's symbols, every game `extern`
for a small-data variable is gone from `src/` (or kept by a `SHARED`
symbol's other users), the build matches retail, and lint and Doxygen are
clean. Then delete this file: the history belongs in the commits.
