# Contributing

How to change the code without breaking the match. The README covers
getting a first build; this page assumes `./tools/setup.sh` has run.

## Why the SDK discs

The game links Sony's Psy-Q libraries (libapi, libc2, libcard, libcd, libetc,
libgs, libgte, libpress, libsnd, libspu). The build doesn't re-derive that
code as C. It links **Sony's own objects**, placed exactly where the game put
them. The game mixed library builds from several SDK releases, so which disc
supplies each object was measured against retail and recorded in
`config/psyq-objects.txt`. `config/psyq-headers.txt` lists the headers the
same way, each with the hash of the generated file. The handful of Sony
functions no shipped object matches live in `src/psyq/`.

## Measuring

```sh
python3 tools/progress.py      # matched functions and bytes, per file
python3 tools/readability.py   # remaining readability debt: magic numbers, raw offsets, unk fields
```

## Finding your way

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
- **A rodata string is defined once, by name.** Each of the game's strings
  is a `const` array in the unit that uses it, defined ahead of the unit's
  functions in the image's order (`sTitleTimPath` in `title_menu.c`). A
  literal written at a use emits a second copy and shifts the image.
- **Data stays in address order.** GCC 2.6.3 emits each section in source
  order. A unit with long `.data` tables keeps them in
  `<unit>_tables.inc` beside it and `#include`s that file at the tables'
  place, so the `.c` is mostly code and not hundreds of lines of tables
  (`dream_sys.c`).
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

## The host build

[lsd-port](https://github.com/brian-oblivion/lsd-port) compiles this C for
other platforms, against [psyz](https://github.com/Xeeynamo/psyz), with
`HOST_BUILD` defined. The PS1 build never sees that code, so the retail
hash says nothing about it; lsd-port's CI builds it. The C is shared, so:

- **Prefer C that is right on both.** `#ifdef HOST_BUILD` is only for what
  cannot be shared, and each one says why.
- **No address in an integer.** The host is 64-bit; a pointer type where an
  `s32` held an address changes no PS1 bytes.
- **A global the PS1 link places by address**
  (`config/undefined_syms_auto.*.txt`) has no definition in the C, so it
  also needs a host one: `#ifdef HOST_BUILD` beside its `extern`, sized
  from the code. Symbols the code lays one structure over become that
  structure's fields on the host (`sStyleSpawnParams` in `style_layer.c`).
- **Read-only data the game writes** is `IMAGE_CONST`: `const` on the PS1,
  writable on a host, whose `.rodata` is read-only memory.
- **A pointer into `MATRIX.t` or a `VECTOR`** is a `GteLong *`: Sony's
  headers declare those fields `long`, psyz's `int`.

## Continuous integration

- **`lint`** runs on every push and pull request: formatting (clang-format
  22), the API documentation check, the readability counters, one
  declaration per name, snake_case file names, and a Doxygen build that fails
  on any warning. The two checks that preprocess the source need Sony's
  headers, so they run only where the headers are available.
- **`build`** rebuilds the executable, checks it against `check.sha1`, and
  publishes the objdiff report [decomp.dev](https://decomp.dev) reads. The
  report, like `tools/progress.py`, counts the game's own code only; Sony's
  library code is linked from Sony's objects and isn't a matching goal. The
  disc and the SDK never enter this repository, so the job reads them from a
  private repository the owner keeps (`disk/SLPS_015.56`, `lib/` and
  `include/psyq/`), named in the `LSD_DEPS_REPO` variable and read with the
  `LSD_DEPS_TOKEN` secret. Without the secret the job skips.

