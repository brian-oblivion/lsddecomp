# LSD: Dream Emulator (PSX) — matching decompilation

C source that compiles byte-for-byte to the retail `SLPS_015.56` (SLPS-01556).
All but three game functions are matched (`python3 tools/progress.py`
names them), and the readability cleanup is finished; `python3
tools/readability.py` measures what debt is left. README.md covers the
layout, the build and the class framework; read it first. `tools/research/`
holds tools no current work uses, kept for reuse.

**Keep this file short.** It holds rules and the loop, never project state,
counts or history. A number written here is right for one session and wrong
for every one after. Measure instead (`python3 tools/progress.py`,
`python3 tools/readability.py`). Lessons go in commit messages. The full
history of the project (plans, round logs, per-function match reports,
research notes) is on the `archive/process` branch; read it with
`git show archive/process:docs/<file>` when a tool comment points there.

## Hard rules

1. **Never edit `check.sha1` or `build.sha1`.** They hold the retail SHA1.
   Changing one to make a build pass deletes the oracle. A hook blocks it.
2. **Every build goes through `./build-and-verify.sh`.** A bare `make` says
   nothing about whether the bytes are right. A hook blocks in-repo `make`
   except `extract`, `progress`, `format`, `clean` and `nonmatching`.
3. **Never commit the executable, a disc image, or anything of Sony's.**
   `disk/`, `sdk/`, `lib/` and `include/psyq/` are gitignored and generated
   from the user's own files (`tools/psyq_sdk.py install`).
4. **Never edit `asm/`.** splat regenerates it. Rename symbols in
   `config/symbols.slps01556.lsdde.txt` (through `tools/rename.py`) and change
   segmentation in `config/splat.slps01556.lsdde.yaml`. A hook blocks it.
5. **The toolchain is pinned** (GCC 2.6.3, binutils, flags, maspsx). A
   suspected toolchain problem is reported to the operator with a minimal
   reproducer, never experimented on.
6. **No register pinning.** `register T v asm("$N")` and extended-asm operand
   constraints are banned as a way to fix which register holds a value. A
   bare `__asm__("")` barrier (order only) is allowed. The one exception is
   an instruction with no C spelling and no `gte_*` macro in `include/gte.h`
   (COP2 `lwc2`/`swc2`), and those constraints live inside `include/gte.h`.

## The loop

1. Edit `src/` or `include/`. Keep functions within a file in ROM-address
   order. C89 only (`/* */` comments, declarations at block top). `char` is
   unsigned, so a signed byte is `s8`.
2. Verify, chained so a failed build can't hand you a score:

   ```sh
   ./build-and-verify.sh > /tmp/b.log 2>&1; echo "build exit=$?"; \
   grep -nE 'error:|parse error|undefined reference|\*\*\* \[[^]]*\.o\]' /tmp/b.log | head -8; \
   .venv/bin/python3 tools/funcdiff.py <func>
   ```

   Exit 2 is both "did not compile" and "compiled, does not match". Only the
   grep tells them apart: any hit means the C didn't build, and any score is
   from the previous build. GCC 2.6.3 prints semantic errors without an
   `error:` prefix, which is why the `*** [….o]` pattern is there.
3. `tools/lint.sh` before committing. Renames go through `tools/rename.py`,
   `tools/renametype.py` and `tools/unitfile.py`, never by hand.
4. Code that doesn't match never stays live. A readable near-miss goes in
   `#ifdef NON_MATCHING` with the `INCLUDE_ASM` in its `#else`
   (`tools/check-nonmatching.sh`). An odd spelling kept for matching gets one
   `MATCHING:` comment in the `.c`. Headers carry no process text
   (`tools/apidoc.py` checks).

## How a score lies

- **Stale build:** a failed compile or link leaves the old image, and its
  score is plausible. `funcdiff.py` warns on mtimes.
- **Still `INCLUDE_ASM`:** compares retail with retail and reports a full
  match. Check the function is really C.
- **Address drift:** C of a different length shifts everything after it.
  `funcdiff.py` reports out-of-range differing bytes. Also check that no
  sibling function in the unit lost its wrapper.
- **Half-finished merge:** a unit with conflict markers may not rebuild, so
  the build can read green. Check `git rev-parse -q --verify MERGE_HEAD`
  first.
- **Skipped unit:** after a *header* edit breaks a unit once, the pipeline
  leaves a stale `.o` that make then skips. A red build with no compile error
  means `rm -f build/src/<dir>/<unit>.c.o` and rebuild.

To localise a red build with no compile error, run
`cmp -l build/SLPS_015.56 disk/SLPS_015.56 | head`. The offsets are 1-based,
`vram = (N - 1) - 0x800 + 0x80010000`, and `grep` that address in
`build/lsdde.map`. The usual causes are a struct edit that moved an offset
(any struct edit is non-local) or a rodata string written as a literal
(declare the existing `extern const char D_…[]` instead).

## Facts worth knowing

- Plain PS-X EXE, loaded at `0x80010000`; `file offset = vram - 0x80010000 +
  0x800`. `$gp` is `0x8008A808`. Little-endian R3000, no FPU.
- Flags: `-mips1 -mcpu=3000 -O2 -G0 -funsigned-char -fno-builtin
  -mno-abicalls`, then maspsx with the flags in the Makefile's
  `MASPSX_FLAGS`. Anything that compiles in isolation reads the flags from
  the Makefile and never retypes them.
- Plain C with a hand-rolled class framework, **not C++**. Resolve a method
  slot with `tools/classtable.py`, never by counting.
- Sony's SDK is linked from Sony's objects (`config/psyq-objects.txt`). Never
  write C for a function a Sony object owns; `tools/sdkstalls.py` and
  `tools/psyq_sdk.py coverage` say which those are.
- A prototype for a function another unit defines belongs in that unit's
  header or in your `.c`, not in a shared header of your own. Put in a shared
  header only what a sibling would use unchanged.
