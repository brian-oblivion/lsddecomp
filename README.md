# lsddecomp2

A matching decompilation of **LSD: Dream Emulator** (PlayStation, 1998, Asmik
Ace / OutSide Directors Company, SLPS-01556).

"Matching" means the C in `src/` compiles, through the original 1997 Sony
toolchain, to an executable that is **byte-for-byte identical** to the one on
the disc. That is the whole point: a paraphrase of the game can be wrong in
ways nobody notices, and a byte-exact rebuild cannot.

**Status:** the build reproduces retail. 6 of 1356 game functions are C so far.
Run `python3 tools/progress.py` for live numbers.

## Getting started

You need your own copy of the game. Nothing here contains game data.

```sh
git clone <this repo> lsddecomp2 && cd lsddecomp2
cp ~/path/to/'LSD - Dream Emulator (Japan).bin' .   # or the extracted SLPS_015.56 into disk/
cp ~/path/to/'Programmer Tool - Runtime Library Version 3.5 (Japan)_DTL-S2300_redump.zip' sdk/
./tools/setup.sh
```

You also need the Psy-Q SDK disc(s), because the build **links Sony's own
library objects** rather than re-deriving them as C — the same objects the
game linked, placed where the game put them. `sdk/README.md` says which discs
and where to find them; `setup.sh` tells you exactly which version is missing
if one is.

`setup.sh` extracts the executable from the disc image if it finds one, builds
a `mipsel-linux-gnu` binutils, fetches the Psy-Q GCC 2.6.3, sets up a Python
venv, converts the SDK libraries in `sdk/` into `lib/`, runs the split, and
**proves the result rebuilds byte-for-byte**. If that
last step fails it stops: a funcdiff score against a toolchain that cannot
reproduce retail is meaningless, and starting anyway wastes a session.

Host requirements: `python3`, `git`, `curl`, `make`, `sha1sum`, and a C
compiler for the binutils build. Tested on Arch with Python 3.14.

Then:

```sh
./build-and-verify.sh          # the canonical build + verify — use this, not `make`
python3 tools/progress.py      # where the project is
```

## How the work is done

Read [`CLAUDE.md`](CLAUDE.md) — it is the operating manual, for people as much
as for agents. The short version:

1. Pick an `INCLUDE_ASM(...)` in a `src/` unit.
2. Read `asm/nonmatchings/<unit>/<func>.s`; seed with `tools/m2ctx.py --sig`.
3. Write C in its place.
4. `./build-and-verify.sh`, then `tools/funcdiff.py <func>` for the score and
   `tools/asm-differ/diff.py <func>` to read the diff.
5. Match, or stall and write the report. Never leave non-matching C in `src/`.

### The oracle, and the ways it lies

`./build-and-verify.sh` is the only oracle. Everything else is a hint. Three
things make a per-function score lie, and two of them report a **full match** on
a function that is nowhere near matching:

- a failed compile **or link** leaves the previous build in place;
- an `INCLUDE_ASM` function contributes retail's own bytes, so it compares
  retail against retail;
- a length change shifts everything after it.

`funcdiff.py` detects the first two and exits 2 rather than hand you a number
it does not trust. CLAUDE.md has the details. This is worth internalising before
your first session, because a stale score is usually plausible.

### Parallel runs

[`docs/PARALLEL-RUNS.md`](docs/PARALLEL-RUNS.md) covers running several matching
sessions at once — one git worktree per runner, a head agent that triages,
merges and consolidates. `tools/setup-worktree.sh <name>` provisions one and
verifies it before handing it over.

Builds here take under a second, so CPU is not the constraint; head attention
is. Four to six runners is comfortable on a many-core host, three if the head
also has work of its own.

## Guard rails

`.claude/hooks/` enforces three rules that are easy to break by accident and
expensive to notice:

- `check.sha1` / `build.sha1` cannot be modified — a build that only passes
  against an edited hash is not a match, and the edit is nearly invisible in
  review;
- `asm/` cannot be edited — it is regenerated on every `make extract`, so the
  change survives just long enough to look like it worked;
- in-repo `make` is restricted to `extract`, `progress`, `format` and `clean` —
  a bare `make` produces bytes without checking them.

## Layout

| path | what |
| --- | --- |
| `disk/SLPS_015.56` | the retail executable. Yours; never committed. |
| `config/` | splat segmentation and symbol names |
| `include/` | project headers and the Psy-Q SDK headers |
| `sdk/`, `lib/` | your Psy-Q SDK disc(s), and the Sony objects converted from them. Never committed. |
| `src/` | carved C units — where the work happens |
| `asm/` | generated disassembly. Never edit, never commit. |
| `tools/` | toolchain and workflow tools |
| `docs/` | the guides; `docs/match-reports/` is the durable record |

## Credits

This project would be starting from nothing without
[FirecatFG/lsddecomp](https://github.com/FirecatFG/lsddecomp), whose
segmentation and symbol names are inherited here. See
[`CREDITS.md`](CREDITS.md) for exactly what was taken, what was not, and why —
and for the tools the whole console-decomp scene runs on.
