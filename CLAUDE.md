# LSD: Dream Emulator (PSX) — matching decompilation

Matching decompilation of *LSD: Dream Emulator* (PlayStation, 1998, Asmik Ace /
OutSide Directors Company, SLPS-01556). The goal is C source that compiles
byte-for-byte to the retail `SLPS_015.56` executable.

## HARD RULES — read before doing anything

1. **NEVER edit `check.sha1` or `build.sha1`.** They hold the ground-truth
   SHA1 `76322eeade5ebb22dca57fdeac7d68c30f06308d`, which is what the whole
   project verifies against. Editing one to make a build "pass" is not a
   match, it is a deleted oracle — and it is nearly invisible in review,
   because one hex string changes and everything goes green. A hook blocks it.
2. **All builds go through `./build-and-verify.sh`.** A bare `make` produces
   bytes and says nothing about whether they are the right ones; every score
   read after one is unanchored. A hook blocks in-repo `make`, in the main
   checkout AND in every worktree of it (round 58 measured that it had never
   fired in a worktree), for anything but
   `extract`, `progress`, `format`, `clean` and `nonmatching` (the last
   compiles the readable `#ifdef NON_MATCHING` bodies into `build/nonmatching/`
   and links nothing; drive it through `tools/check-nonmatching.sh`).
3. **NEVER commit the executable or a disc image.** `disk/SLPS_015.56` and any
   `.bin`/`.cue`/`.iso` are gitignored and stay that way. Bring-your-own-disc.
   The same goes for the Psy-Q SDK: `sdk/` (the user's SDK discs) and `lib/`
   (Sony's library objects converted from them) are gitignored. What IS
   committed is `config/psyq-objects.txt`, the manifest that regenerates
   `lib/` from `sdk/`.
4. **NEVER edit anything under `asm/`.** It is generated from the executable by
   splat and rewritten on every `make extract` — `make extract` deletes
   `asm/nonmatchings/` outright. To rename a symbol, edit
   `config/symbols.slps01556.lsdde.txt`; to change segmentation, edit
   `config/splat.slps01556.lsdde.yaml`; then re-extract. A hook blocks it.
   (If a future segment holds assembly that was *never* compiled from C, it
   becomes an `hasm` segment: authored source, committed, editable. There are
   none yet, and adding one means updating the yaml, `.gitignore` and
   `.claude/hooks/block-asm-edits.sh` together.)
5. **The toolchain is PINNED.** Compiler, binutils, flags, `maspsx` version —
   none of it is a knob to turn while matching. A suspected toolchain problem
   is something to *report with a reproducer*, never something to experiment
   with mid-round. See "Escalate, do not experiment" below.
6. **Never fix a register mismatch with `register T v asm("$N")` or an
   extended-asm operand constraint.** Both are banned project rules, not
   judgement calls. A bare `__asm__("")` scheduling barrier is allowed. The
   test: if removing it changes WHICH REGISTER holds a value, it is banned; if
   it only changes instruction ORDER, it is allowed. A register-identity
   mismatch is a STALL — write the report.

   **The ban is on fixing REGISTER IDENTITY, and it is scoped to code that
   has a C form at all.** It does NOT forbid an operand constraint that is
   the only way to name a value from an instruction C cannot express. The
   PS1 GTE is the case that matters: `swc2`/`lwc2` move data to and from
   COP2 registers, which are a numbering disjoint from the GPRs, and there
   is no C that emits them. Handing such a block a pointer with
   `: : "r" (dst) : "memory"` is not register pinning, it is the only way to
   reference the pointer. `src/code_8220_b.c` has carried eight such
   constraints, byte-verified, since before this rule was written down —
   `StoreSxyPolyF3` and its five siblings — and round 13 matched three more
   functions the same way. Those constraints now live inside the `gte_*`
   macros in `include/gte.h`, which is where the exception is spelled out
   once instead of at every call site.

   **This paragraph exists because the categorical wording above was, on its
   own, false about this repository**, and "not judgement calls" forecloses
   the reading that would have rescued it. A runner obeying the literal rule
   would either refuse a legitimate match or believe it had violated a HARD
   RULE by achieving one. Round 13's charlie did the work correctly and
   flagged the tension; the head confirmed the precedent with
   `grep -rcE '"[rm]" *\(' src/` rather than by reasoning about intent.

   **The exception is scoped to "no C form EXISTS", not to "hard to type in
   C", and the difference is not a judgement call either.** GTE
   `rtpt`/`nclip`/`cfc2` and COP2 `swc2`/`lwc2` have no C spelling; an
   awkward unaligned struct copy has one. Round 13 matched
   `func_8001A3EC` (now `CopyPolyVtx3`) with a whole-function raw-register `__asm__` on the
   reasoning that its body is straight-line and frameless, citing the GTE
   function above as precedent. The head reworked it to six lines of
   ordinary C, byte-exact, using an idiom that was already written down and
   already confirmed three times (all-`s8`/`s16` struct -> alignment 2 ->
   whole-struct assignment compiles to `lwl`/`lwr` + `swl`/`swr`). Left
   standing it would have become the precedent for transcribing any
   hard-to-type function, which is what `INCLUDE_ASM` already does, more
   honestly and without pretending to be C.

   So: **"there is no C form" is a claim, and it earns the same standard as
   a toolchain lead — try the documented idiom, fail, and say so.** If you
   do write a whole-function `__asm__`, name the instruction that has no C
   spelling — **and look it up in `include/gte.h` first, because if the SDK
   ships a `gte_*` macro for that instruction, the macro IS the C form and
   the instruction does not qualify.** `TransformAndCullPoly` was carried from
   round 13 to 2026-09-14 as a whole-function `__asm__` with a hand-managed
   `noreorder` bracket, on the grounds that `rtpt`/`nclip`/`avsz3`/`cfc2`
   have no C spelling. Every one of them is a Psy-Q macro, and the body is
   ordinary branching C over seven of them, byte-exact on the first build.
   If you cannot name an instruction that has neither a C spelling nor a
   macro, it is not this exception.

   Two traps inside the exception, both measured in round 13, both cheap to
   hit and expensive to diagnose:

   - **Name the right clobbers.** A GPR clobber on an `lwc2`/`swc2` block
     that names `$2`-`$5` when the instructions target COP2 data registers
     forces spurious evictions that cascade through the whole surrounding
     function's allocation. It looks exactly like an unrelated
     register-identity residue somewhere else entirely.
   - **Bracket real branches.** An `__asm__` block containing an actual
     branch or jump mnemonic (`beq`/`bne`/`blez`/`j`/`jal` — not the
     `beqz`/`bnez` pseudo-forms) needs an explicit tab-delimited
     `".set\tnoreorder\n\t"` … `".set\treorder\n\t"` bracket, or maspsx's
     defensive nop-after-branch insertion corrupts the delay-slot semantics
     and, left unclosed, eats the nop off the NEXT generated code. The
     bracket is a source construct and yours to use; maspsx's underlying
     behaviour is an open operator escalation, not something to adjust.

## Where the project is

**Measure it, do not read it here.** Counts, percentages and per-unit queues
change every round, so this file deliberately quotes none of them:

```sh
python3 tools/progress.py          # matched / queued / stalled / fresh, per unit
python3 tools/plan.py              # the finishing plan: track status and the ranked ready-jobs list
```

**The plan for finishing the project is `docs/FINISHING-PLAN.md`**: what
done means, the tracks, which model runs what, and the one head prompt the
operator pastes. `plan.py` is its measurement. Read it before deciding what a
round should do.

**No number about project state belongs in this file.** A count written here is
correct for one round and quietly wrong for every round after, and the reader
cannot tell which they are in. Worse than the numbers are the *conclusions*
drawn from them — "the next round must carve", "only one unit has fresh
ground", a list of the biggest uncarved blocks. Those read as standing
instructions, so a stale one sends a whole round to redo finished work or to
carve a segment that no longer exists under that name. If you catch yourself
adding a figure below, put it in `docs/PROGRESS.md` (a dated log, where being a
snapshot is the point) or derive it from a command.

How to read `progress.py` correctly:

- **The library split is derived, not hardcoded.** Any subsegment named
  `psyq_*` in the splat config counts as Psy-Q SDK everywhere, and is excluded
  from the game-code percentage. So does every subsegment of type `o`: those
  are Sony's own objects linked from the SDK, they have no asm and no C, and
  `progress.py` reports them on a separate "linked from SDK objects" line
  rather than in any function count.
- **Not every matched function was work.** Some bodies are just `jr $ra; nop`
  and splat generated them itself.
- **The `fresh` column cannot see toolchain blockers** — screen candidates
  yourself, below.
- **A unit has exactly one owner**, so the number of carved units is the
  ceiling on parallel runners. The `unit` table is therefore the staffing plan:
  if only one unit has `fresh` left, the next round has to carve before it can
  run more than one runner.

The build verifies, and a clean `./build-and-verify.sh` takes under a second —
which is what makes many parallel runners cheap here.

## Open toolchain blockers

**There are none. All four are RESOLVED, each by a maspsx flag that sets one
behaviour and nothing else, each proven inert by a byte-exact whole-image
rebuild before it was adopted.** Do NOT screen for them, do NOT file a stall
against them, and do NOT trust a report whose verdict predates the fix.

| construct | resolved | flag (all passed by the Makefile) | evidence |
| --- | --- | --- | --- |
| `addiu_at` (indexed global load) | round 21, 2026-09-06 | `--addiu-at` | `docs/research/addiu-at-blocker.md` |
| `gp_rel` (small-data global) | round 42, 2026-09-15 | `--gp-symbols=config/gp-symbols.txt` | `docs/research/gp-relative-blocker.md`, "RESOLVED" |
| `nop_mflo_mfhi` (mflo/mfhi then mult/div) | round 42, 2026-09-15 | `--no-nop-mflo-mfhi` | `docs/research/addiu-at-blocker.md`, "RESOLVED" addendum |
| `nop_at_expansion` (indexed load, then store-to-symbol of the loaded register) | round 63, 2026-09-21 | `--nop-at-expansion` | `docs/research/load-delay-nop-blocker.md` |

The flags live in `tools/patches/maspsx-addiu-at.patch` and
`tools/patches/maspsx-lsd-flags.patch`, applied to the maspsx clone by
`tools/setup.sh`. `config/gp-symbols.txt` is GENERATED by
`python3 tools/gpsyms.py` from the sdata/sbss segments of the splat yaml —
regenerate it after any carve that touches those segments, and
`python3 tools/gpsyms.py --check` says whether it is current.

**How `gp_rel` was really blocked, because the wrong diagnosis stood for
fourteen rounds.** It was never `-G`. A non-zero `-G` makes cc1 emit a size
hint for EVERY small-typed extern, and this project declares splat data with
whatever scalar type fits the access, so `-G8` gp-relativised hundreds of
symbols retail reaches absolutely (the 2026-08-29 experiment's 19148 damaged
bytes; by round 42 it no longer even linked — `relocation truncated to fit:
R_MIPS_GPREL16` against `.data` symbols 0x1C000 from `$gp`). Retail's rule is
maspsx 2.34's own rule: loads and stores of a symbol DEFINED in `.sdata`/`.sbss`
go through `$gp`, `la` of the same symbol stays absolute (measured: every one
of the 113 `%gp_rel` targets is defined in a sdata/sbss segment, and every
absolute `%hi` into that window is a `lui`/`addiu` address-take). maspsx
already implemented exactly that, gated on a table it could only fill from
same-file `.sdata`/`.comm` directives — which splat-owned data never produces.
`--gp-symbols` fills the table from a list instead. Nothing about `-G` moved.

**The lessons those blockers taught are not retired with them:**

- **Run the screen before you accept a stall's CAUSE, not only before you
  assign work.** Round 13 found `CheckDreamAuxTriggerCondition` filed as a one-word near-miss
  attributed to instruction selection while an `addiu $at, $at, %lo(jtbl_*)`
  hit sat unexamined in its own `.s`. A wrong SCORE is corrected the next time
  anyone measures; a wrong CAUSE is what the next round acts on.
- **A blocker's SCOPE is measured, not reasoned.** Round 10's head "corrected"
  the `addiu_at` grep to exclude `%lo(jtbl_*)` because a jump table loads a
  CODE address and an indexed global loads DATA. At the layer that decides —
  maspsx, below cc1 — they are one construct, and the exclusion was retracted
  mid-round.
- **The right MECHANISM at the wrong GRANULARITY is the recurring shape.** All
  three fixes were a flag that set one boolean maspsx already had, decoupled
  from the version bump that dragged three others along with it. The rejected
  experiments (`--aspsx-version=2.29`, `-G8`) were rejected for turning the
  right knob together with wrong ones.
- **A minimal reproducer that FAILS to reproduce is a result.** The `-G8`
  reproducer succeeded on a five-line file and failed on the image, and the
  difference between the two was the whole diagnosis.

`tools/nearmiss.py` and `tools/uncarved.py` still report these constructs,
tagged `(RESOLVED-not-a-blocker)`, without counting them: older reports still
blame them, and the tag is how you tell "this verdict predates the fix" from
"this is really blocked". A function that hits one of these screens is
ordinary matching work with a mechanism-correct partial derivation already on
file.

## Carving new ground

Uncarved code sits in monolithic top-level `asm/*.s` segments. Rank them by
what they actually hold — the names change as carving proceeds, so derive them
rather than trusting any list:

```sh
python3 tools/uncarved.py            # screened per function, best yield first
python3 tools/uncarved.py --windows 20   # clean-density windows, for boundaries
```

**Rank by WORKABLE functions, not by function count.** Those two diverged a
long time ago on this corpus: `class_3bb8c_h` counted 17 `glabel`s and held 4
matchable functions, the other 13 being BIOS trampolines no C compiles to.
`uncarved.py` runs all four screens per function and excludes the `psyq_*` SDK
segments; the raw count below answers "what segments exist right now", which is
a different question:

```sh
for f in asm/*.s; do b=$(basename "$f" .s); case "$b" in psyq_*|header) continue;; esac
    printf '%6d %s\n' "$(grep -c '^glabel' "$f")" "$b"; done | sort -rn
```

Carving a unit out of one of those is Gate 2 in `docs/PARALLEL-RUNS.md`. Do NOT
budget time for under-split hunting — it was predicted, measured, and does not
happen (894 of 894 table-dispatched entry points already have symbols). Do
budget for the two carve failures that ARE routine: an orphaned rodata
jump-table slot, and a segment whose tail is data. Both are documented in
Gate 2.

## Key technical facts (derived from the binary)

- **Plain PS-X EXE, no overlays, no compression.** 505856 bytes = a 0x800-byte
  header plus one contiguous image loaded at vram `0x80010000`. Every byte of
  code in the game is in this one file, which makes it dramatically simpler
  than a cartridge project — there is no bank switching, no demand paging and
  no asset compression to reverse first.
- **`file offset = vram - 0x80010000 + 0x800`.** splat's instruction comments
  carry the file offset directly: `/* 39CD8 800494D8 E8FFBD27 */` is
  `FILEOFS VRAM WORD`.
- **Little-endian MIPS R3000, no FPU.** Software floating point (`-msoft-float`
  is cc1's default here). `$gp` is `0x8008A808`, and `%gp_rel` addressing shows
  up constantly — a `lw $v0, %gp_rel(sym)($gp)` is an ordinary global access,
  not something exotic.
- **The compiler is GCC 2.6.3, Psy-Q patched.** cc1's own banner says
  `GNU C 2.6.3 [AL 1.1, MM 40] Sony Playstation compiled by GNU C`. Flags:
  `-mips1 -mcpu=3000 -O2 -G0 -funsigned-char -fno-builtin -mno-abicalls`.
- **`maspsx` sits between cc1 and gas, and it is load-bearing.** Sony's ASPSX
  assembler expanded macros (`div`, `li`, …) differently from GNU as, and those
  expansions are part of the retail bytes. Without maspsx the executable does
  not even assemble.
- **It is C89, and 2.6.3's `cpp` is strict about it.** `//` comments are a
  parse error — every header in `include/` uses `/* */`. `cpp` also rejects
  `-fno-builtin`, which is a cc1-only flag; that asymmetry is why the Makefile
  has separate `CPP_FLAGS` and `CC_FLAGS`.
- **`char` is unsigned** (`-funsigned-char`). A signed byte load is `s8`.
- **It is plain C with a HAND-ROLLED class framework — not C++.** Proven, with
  a reproducer, in `docs/research/class-framework.md`: constructors are called
  *through* the method table (impossible for a C++ compiler), entries are 4
  bytes where GCC 2.6.3's own C++ emits 8, and a scan of the whole executable
  finds **zero** compiler-generated vtables against **128** flat pointer tables.
  Do not reach for `cc1plus`. Methods are ordinary C functions with an explicit
  `this` first parameter, the table pointer sits at object offset 0, and the
  tables are data. Resolve a slot with `tools/classtable.py`, never by counting.
  Word +0x000 of a table is a nibble-path class id (the parent of 0x1F234 is
  0xF234, then 0x234, 0x34, 0x4, 0x0), so the whole hierarchy is mechanical:
  `python3 tools/typeviews.py --tree`.
  **The suggestive symbol names are FirecatFG's hypotheses, not evidence** —
  they look like C++ because someone who suspected C++ chose them.
- **The Psy-Q SDK is LINKED, not decompiled.** The libraries shipped on the
  SDK discs as `.LIB` archives of `.OBJ` files with full symbol names, so the
  build links those objects themselves through splat `o` segments, exactly as
  the game did, and byte-exactly (proven 2026-09-11 with eight `libgte`
  objects; `docs/research/psyq-sdk-objects.md`). Consequences:
  - **Never write C for a function a Sony object owns.** Round 20 matched
    `func_8003FC70` as game code; it is `libgs/gs_108.o`.

    **Do not screen for this by smell — run the tool.** This rule used to say
    to check "any game-segment function that smells like a library (string
    ops, sound driver internals, BIOS stubs, GTE helpers)", which is a
    judgement call made before you know what the function does, and round 32
    measured what it costs: **14 live stalled functions, 1669 words, ~4100
    lines of accumulated derivation**, every one lying FULLY inside an object
    already placed and verified against retail. One of them was a 3×3 matrix
    transpose (`libgte/fgo_00.o`) and one was `atoi`.

    ```sh
    python3 tools/sdkstalls.py     # the stall queue crossed against placed objects
    ```

    `nearmiss.py` runs it for you and excludes the hits from `ASSIGN FROM
    HERE`, so a Gate 1b assignment is safe without thinking about it.
    **An object that never placed can still own functions one at a time**:
    retail's `libsnd/seqread` is the 3.3 build with one function changed, so
    it never placed, and 16 of its 18 functions were matched or stalled as
    game C. `progress.py` counts those as library by address
    (`config/sdk-in-game.txt` from `sdkname.py --game --write`, the
    `psyq-objects.ld` pins, `identified` symbols entries), and every queue
    follows (FINISHING-PLAN revision 13). Run
    `sdkstalls.py` directly when you carve, or before believing any single
    report's verdict.

    **These pass every blocker screen** — no `gp_rel`, no `mflo`/`mfhi`
    hazard, not a trampoline — so they read as the CLEANEST ground in the
    queue while being unmatchable by construction. That is the BIOS-trampoline
    lesson from Gate 2 arriving in Gate 1b: a screen measures the obstruction
    it was built for and says nothing about the ones it was not.

    `.venv/bin/python3 tools/psyq_sdk.py coverage` remains the underlying
    source of truth and prints the same overlap under the heading "Placed
    objects that fall inside GAME-code segments (SDK code miscounted as
    game)". It had been printing it correctly the whole time; nobody crossed
    it against the queue.
  - **The game mixed library builds.** `libetc` is the build on the 3.5 disc;
    `libgpu`/`libcd` carry RCS ids from December 1995 and match neither the
    3.5 nor the 3.6 disc. Which disc owns which object is MEASURED by
    `tools/psyq_sdk.py match` against retail (relocation-masked exact match),
    never inferred from a version number.
  - An object goes into the yaml as `- [0xOFF, o, <lib>/<module>]` AND into
    `config/psyq-objects.txt` with the same offset; `tools/psyq_sdk.py check`
    verifies the two agree, `lib/` is complete and the generated
    `config/psyq-objects.ld` is current. `.data`/`.rdata`/`.sdata` are placed
    in the yaml too (`psyq_sdk.py place` derives the lines). **bss is never
    placed in the yaml**: Sony's linker scattered library variables across
    objects, so `psyq_sdk.py ldfrag` pins them by name in that fragment.
    The full recipe is `docs/SDK-OBJECTS-GUIDE.md`; it is mechanical and is
    runner work, not head work.

## The decompilation loop

1. Pick an `INCLUDE_ASM(...)` in a `src/*.c` unit. (Or carve new ground first —
   see Gate 2 in docs/PARALLEL-RUNS.md.)
2. Read `asm/nonmatchings/<unit>/<func>.s`. Optionally seed with m2c:

   ```sh
   .venv/bin/python3 tools/m2ctx.py <unit> --sig 'void <func>(Foo *a, s32 b)' --run
   ```

   **Always pass `--sig`.** m2c types arguments from a declaration of the
   target, and `INCLUDE_ASM` leaves none, so without it every argument comes
   out `void *arg0` and every field `->unk8` no matter how good your context
   is. A `padNN[0x..]` access in the output means the struct you guessed is
   missing a field at that offset — a wrong guess is itself a measurement.
3. Write C in place of the `INCLUDE_ASM`. Keep declarations at block top (C89),
   real struct fields rather than pointer arithmetic, and **keep every function
   in the unit in strict ROM-address order** — writing one out of order
   miscompiles the whole image, the link succeeds, and previously-matched
   functions appear to regress with diffs that look unrelated to your change.
4. Verify. Chain the two so you cannot read a score from a failed build:

   ```sh
   ./build-and-verify.sh > /tmp/b.log 2>&1; echo "build exit=$?"; \
   grep -nE 'error:|parse error|undefined reference|\*\*\* \[[^]]*\.o\]' /tmp/b.log | head -8; \
   .venv/bin/python3 tools/funcdiff.py <func>
   ```

   Working in parallel? Use `/tmp/<name>_b.log`, not the shared `/tmp/b.log`
   — see docs/PARALLEL-RUNS.md.

   **`build exit=` is necessary but NOT sufficient, and the reason is
   measured, not reasoned.** `build-and-verify.sh` exits 1 only when
   `disk/SLPS_015.56` itself is missing or wrong; everything else runs under
   `set -e` and propagates make's exit code, which is **2**. So a genuine
   compile error and an ordinary clean-compile-but-SHA1-mismatch BOTH exit 2 —
   and the second is the normal state every time you iterate on a function
   that does not match yet. The exit code alone cannot tell them apart.

   The grep is what separates them, which is why `Error [0-9]` was REMOVED
   from it (round 18). That pattern matches make's own summary line
   `make: *** [Makefile:74: check] Error 1`, which make prints when the SHA1
   check fails — i.e. it fires on a perfectly good build.

   **But the compiler-only patterns round 18 left behind MISS most real
   compile errors, and they miss them in the direction that produces false
   confidence (round 21).** GCC 2.6.3 predates the `error:` prefix
   convention: it writes `/tmp/t.c:1: conflicting types for 'T'`, with no
   `error:` anywhere. `parse error` catches syntax errors and `undefined
   reference` catches the linker, so between them they cover the two loudest
   failures — and every SEMANTIC error in between produces **zero hits**.
   Measured through the pinned pipeline, one construct per row, all seven
   fatal (`cc1` exits 33):

   | construct | cc1 message | `error:`/`parse error` hit? |
   | --- | --- | --- |
   | `parse error` | ``parse error before `)' `` | yes |
   | conflicting types | ``conflicting types for `T' `` | **no** |
   | redefinition | ``redefinition of `f' `` | **no** |
   | undeclared variable | ``` `zzz' undeclared ``` | **no** |
   | too many arguments | ``too many arguments to function `g' `` | **no** |
   | incompatible return | `incompatible types in return` | **no** |
   | duplicate member | ``duplicate member `a' `` | **no** |

   So the fix is to match make's failure on a COMPILE target rather than the
   message text, which is what `\*\*\* \[[^]]*\.o\]` does — it fires on
   `make: *** [Makefile:113: build/src/<unit>.c.o] Error 33` and not on
   `make: *** [Makefile:74: check] Error 1`. That keeps round 18's result
   (the SHA1-check line must not fire) while closing the gap it opened.
   Measured in an idle worktree, both cases induced deliberately:

   | case | `build exit=` | round-18 grep | with `*** [….o]` |
   | --- | --- | --- | --- |
   | genuine compile error (`conflicting types`) | 2 | **0 hits** | **1 hit** |
   | clean compile, SHA1 mismatch | 2 | 0 hits | **0 hits** |

   With that pattern added the signal is clean: **any hit means your C did not
   build, so any funcdiff number is from the previous build.** No hits plus
   exit 2 means the build is fresh and simply does not match yet, which is
   what iterating looks like.

   **One mechanism defeats that last sentence, and it is silent (round 56).**
   The compile rule is a pipeline (`cpp | cc1 | maspsx | as -o $@`), so when
   cc1 fails, `as` still CREATES `$@`; make judges the recipe by `as`'s exit
   status and leaves the object in place, NEWER than the header that broke it.
   The next `make` then treats that object as up to date, SKIPS the unit, and
   links the stale one. The unit you actually broke vanishes from every
   subsequent log, and the only symptom is a red whole-image SHA1 with no
   compile error anywhere. The `*** [….o]` pattern catches the FIRST such
   build and cannot catch the second, because by then the unit has no failing
   target at all. So: **a red build with no compile error means suspect a
   skipped unit, not a mysterious byte.** `rm -f build/src/<unit>.c.o` and
   rebuild, or `make clean` followed by `make extract` (clean wipes `asm/`).
   Reproducer and the escalated Makefile fixes: `docs/PROGRESS.md`, round 56.

   **Scope, measured, because the paragraph above over-warns without it:**
   the trap needs the triggering change to be a HEADER (or other prerequisite)
   with the unit's own `.c` UNTOUCHED. Editing the `.c` makes the source newer
   than the stale object and make rebuilds it regardless — verified directly.
   So the ordinary matching loop, where you edit your unit every iteration, is
   NOT exposed, and a red build there is the usual "does not match yet". What
   IS exposed: a struct or header edit, a `rename.py` run, a merge, a rebase —
   anything that changes a unit's inputs without touching the unit. Runner
   bravo reasoned this out from its own session in round 56 and asked for the
   check rather than assuming its negatives were safe; they were.

   **How this was found matters more than the patch, because it is the loop
   auditing itself.** The head hit it running an ordinary experiment: a
   duplicate `typedef` gave `build exit=2`, zero grep hits, and a funcdiff
   score — the exact signature of "fresh build, does not match yet". The only
   thing that caught it was `funcdiff.py`'s own mtime staleness guard, which
   is documented as a BACKSTOP. When the backstop is the sole detector, the
   primary check has failed silently, and "the guard fired" is the wrong
   lesson to draw — the right one is to go measure why the primary did not.

   To *read* a diff rather than score it, use asm-differ:
   `.venv/bin/python3 tools/asm-differ/diff.py <func>`.
5. Iterate. On a stall, restore the `INCLUDE_ASM` and write the match report
   (see below). **No score short of byte-exact justifies leaving C in `src/`
   as live code** — it fails the whole-file SHA1 the moment it is merged.

   The one sanctioned home for a near-miss body in `src/` is an
   `#ifdef NON_MATCHING` block with the `INCLUDE_ASM` in its `#else`
   (`docs/FINISHING-PLAN.md`, track 1b, which also says when a stall
   qualifies). The verified build never compiles that block, `progress.py`
   strips it before counting, and `tools/check-nonmatching.sh` proves it
   builds. It exists so the code is READABLE, not so the score looks better.
6. On a match: keep the C idiomatic, name things sensibly, add new struct
   knowledge to `include/`, write the report, commit.

   **"To `include/`" has one exception, and it is the mistake git does not
   mark.** A prototype for a function ANOTHER unit defines — or an
   `extern <your local type> D_XXXX;` — belongs in your own `.c`, not in a
   header a sibling unit includes. Your build stays green either way; the
   collision appears in whichever OTHER unit includes both your header and a
   second one declaring the same name, as `conflicting types`. Round 15 hit
   this four times and the worst instance stayed latent through two merges
   before breaking a unit that had never touched the declaration.

   The test is reuse, not tidiness: put in a shared header what a sibling
   would use UNCHANGED, and keep next to your code anything that encodes
   *your* reading of a class. The project's multiple-independent-local-views
   convention is what makes two readings legitimate — it is placing them in
   one shared header that breaks. **The convention ends, class by class, in
   track 4:** a class `plan.py classes` lists as UNIFIED has exactly one
   definition, `include/<Class>.h`, and no unit declares its own view of it
   again (FINISHING-PLAN track 4; `include/BasicClass.h` is the model). `python3 tools/headercontention.py` shows
   which units share a header and would therefore see each other's
   declarations.

## The four ways a score lies

`funcdiff.py` guards two of these mechanically and will exit 2 rather than
hand you a number it does not trust. Know all four anyway — the guards are a
backstop, not a substitute for reading `build exit=`.

1. **Stale build.** funcdiff reads the BUILT file. A failed compile *or a
   failed link* leaves the previous build in place, so the score comes from the
   last build that succeeded — and if that build had the function as
   `INCLUDE_ASM`, it reports a FULL MATCH with no diagnostic anywhere. The
   reason knowing this is not enough: **a stale number is often plausible and
   self-consistent**, equal to a real score you measured minutes earlier.
   funcdiff compares mtimes and warns.
2. **Still `INCLUDE_ASM`.** The build succeeds, the output is fresh, and the
   score is still a full match — because an `INCLUDE_ASM` function contributes
   retail's own assembled bytes, so this compares retail against retail.
   Nothing fails and there is nothing to grep for. It fires most often when
   spot-checking someone else's stall claim *after a merge*, because merging
   restores the `INCLUDE_ASM`. funcdiff warns.
3. **Address drift.** If your C is a different length, everything after it
   shifts and the per-function window no longer means what it says. funcdiff
   reports how many bytes differ OUTSIDE the range; a non-zero count there
   makes the in-range score untrustworthy. `build-and-verify.sh` is the oracle.

   **The drift need not come from the function you are editing, and that is an
   ATTRIBUTION hazard rather than a fifth way (round 20).** Runner alpha left
   an earlier experiment's C live in its unit — `Class6B5CC__TryAttachNearby` without its
   `#if 0`/`INCLUDE_ASM` wrapper — and every LATER function in the unit then
   measured against a shifted window, including the one alpha was actually
   working on. The guard fired correctly: the out-of-range byte count was
   there in the same command's output. What fails here is not the oracle but
   the reading, because the instinct on seeing drift is to blame the function
   under the cursor, not an unrelated sibling a hundred lines up.

   So when a diff looks structurally wrong rather than like a plausible
   near-miss — a completely different instruction where you expected a
   register swap — **check `build/lsdde.map` for the function's actual linked
   address against its retail one before reading anything else**, and check
   that every other function in your unit is still wrapped:

   ```sh
   grep -c '^INCLUDE_ASM' src/<unit>.c    # against the count you started with
   ```

   Alpha proposed this as a new entry in this list. It is not one, for the
   same reason round 13's proposal was not (see below): the oracle went red
   and the guard fired. Filing it as a fifth way would teach the next runner
   that drift detection is unreliable in exactly the case where it worked.
4. **A conflicted merge.** This one bites the HEAD, not a runner, which is
   why it went unwritten for ten rounds. `git merge` exits non-zero and leaves
   conflict markers in a `src/` file — and `./build-and-verify.sh` run
   immediately afterwards can still print `build exit=0` and
   `OK: build matches retail`, because the object for the conflicted unit was
   not rebuilt. Both halves of the usual discipline pass: the exit status is
   0 and funcdiff raises no staleness warning. Nothing in the oracle chain
   knows a merge is half-finished.

   **So `build exit=` is necessary but not sufficient during a merge.** Check
   the merge itself:

   ```sh
   git merge --no-ff runner/<name> ...; echo "merge exit=$?"
   git rev-parse -q --verify MERGE_HEAD >/dev/null && echo "MERGE IN PROGRESS"
   ```

   A green build on top of `MERGE IN PROGRESS` means nothing at all. Resolve
   first, then verify. Round 10 hit this merging a runner whose unit file the
   head had also edited on `main` mid-round — see the note below on why that
   edit should not have happened.

**There is no fifth, and one was proposed and rejected — read this before
adding one.** Round 13's delta inserted a new field into a shared vtable
struct and forgot its leading `u8 padNN[...]`. Everything after it shifted,
which broke an ALREADY-MATCHED function in a DIFFERENT unit
(`Class6B5CC__DetachAttachedChildren`, `code_d294.c`) by exactly one byte. It compiled clean, and
nothing in delta's own function's output showed anything wrong. Delta
proposed it as a fifth way a score lies.

**It is not one, and the distinction matters.** The four above are cases
where the oracle hands you a wrong GREEN or a wrong NUMBER. Here the oracle
went RED and was right to: `build exit` was non-zero and the whole-image
SHA1 failed. Nothing lied. Filing it as a fifth would teach the next runner
that the oracle cannot be trusted in precisely the case where it can, which
is a worse error than the one it documents. Delta's own write-up reaches the
same conclusion in its last sentence — *"the signal only ever shows up in
the whole-image byte count, which is why the loop insists on re-running the
full oracle after every source change"* — the discipline already covers it.

What delta actually found is a **DIAGNOSTIC** gap: a red build with no
compile error and no diff in the function you were editing. Two things
follow, and both are worth having.

- **The shared-struct hazard is broader than the "shared vtable slot
  retype" warning states.** That warning (in the runner prompt in
  `docs/PARALLEL-RUNS.md`) is about RETYPING a slot. The real risk class is
  *any* edit to a struct another already-matched function reads — and an
  ordinary field INSERTION with a forgotten pad is easier to trigger than a
  retype and has identical consequences. Treat every struct edit as
  potentially non-local, not just retypes.

  **This is a STANDING pattern, not an anecdote: four live instances across
  two rounds, all self-caught.** Round 13 alone produced three — a `slotA8`
  that silently slid from `+0xA8` to `+0x9C` and broke an
  already-matched function in the same round, a `GenericObj::unkC`
  insertion that left a 4-byte gap and broke another, and the
  `GenericMethods_d294` case that found the class. Every one compiled clean
  and every one presented only as a whole-image SHA1 failure. So the check
  below is not a debugging tip to reach for when puzzled; **run it after any
  struct edit, before trusting any score.**
- **Localizing it: `cmp -l`, then the map.** When `build-and-verify.sh`
  fails with no compile error, find the differing byte and turn it into a
  function name:

  ```sh
  cmp -l build/SLPS_015.56 disk/SLPS_015.56 | head
  # cmp -l positions are 1-BASED. vram = (N - 1) - 0x800 + 0x80010000
  python3 -c 'print(hex((0xD561 - 1) - 0x800 + 0x80010000))'   # -> 0x8001cd60
  grep -n '0x8001cd60' build/lsdde.map
  ```

- **A second construct produces the identical signature: a duplicated rodata
  string.** A `D_XXXXXXXX` in rodata that holds a string is a SYMBOL to
  reference, not a string to retype — splat has already emitted those bytes,
  and writing the literal in C emits a second copy. Because `section_order`
  puts `.rodata` first, the duplicate shifts the whole image and the first
  differing byte lands in rodata, thousands of bytes AHEAD of the code you
  edited. Round 20 closed `func_8003FC70` (35/35) on this after round 14 had
  measured 339541 bytes off and attributed it to a rodata alignment
  constraint that does not exist.

  **That worked example is a function round 34 reclassified as Sony's
  `libgs/gs_108.o` — and it is the ONE citation in this corpus where that
  does not invalidate anything.** Round 38's rule is that an SDK exit voids a
  precedent because no source shape ever reached those bytes. Measured round
  45 across all 85 SDK-exit functions that have reports: 84 of them never
  matched, and `func_8003FC70` is the single one that DID reach byte-exact as
  C before it was converted. So the whole-image oracle went green on this
  mechanism, with real bytes, and the finding stands on that rather than on
  who wrote retail. Cite it knowing which half is load-bearing: the
  **mechanism** (splat already emitted those bytes; a literal emits a second
  copy) is a fact about this build system, checkable without retail at all. Before writing any string literal, grep
  `asm/data/*.rodata.s` for the symbol; if it is there,
  `extern const char D_XXXXXXXX[];` is the only correct spelling. Details in
  `docs/DECOMPILATION_LEARNINGS.md`.

  **The 1-based part is not a nitpick and delta's version of this recipe had
  it wrong**, giving `file_offset - 0x800 + 0x80010000` applied straight to
  `cmp -l`'s output. Verified here: `cmp -l` on two 4-byte files differing at
  0-based index 2 reports `3`. Off by one usually still lands inside the same
  function, which is exactly why the error survives being used.

## Standing checks

```sh
python3 tools/progress.py          # counts, and warns about stale asm/ files
python3 tools/srcpath.py           # unit names unique, layout sane
./build-and-verify.sh              # THE oracle
```

## Match reports

`docs/match-reports/<func>.md`, **one file per function you touch — matched
ones included, not just stalls.** The file is keyed by the function's NAME,
so renaming a function means renaming its report: `tools/rename.py` does
both, plus the symbols file and every reference, and re-verifies. Never
rename by hand. `tools/progress.py` decides whether a queued
function is a documented STALL or untouched FRESH ground purely by whether that
file exists. A stall with no report is counted as unworked, and the next round
staffs someone straight back onto it to re-derive what you already established.

A report is a file, not a `.c` comment and not a final chat message — those die
with the session. If you preserved a near-miss body, **inline it in the report
as literal source, with every declaration it needs**, positioned where it would
compile. A body that lives only as a path is a body that exists in exactly one
checkout and travels to no worktree, machine or clone.

Preserve a stalled body in `#if 0 ... #endif`, **never in a `/* */` block
comment**: inside one, a line whose first non-space character is `*` is
ambiguous between the comment's continuation marker and a C dereference, and
nothing local tells them apart.

## Escalate, do not experiment

Toolchain or flag changes of any kind are an operator escalation with evidence
attached, never something to try mid-round. And **never escalate a toolchain
lead you have not tried and failed to reproduce in isolation** — extract the
construct into a self-contained `.c` and run it through the pinned pipeline:

```sh
# Keep the $(sed ...) INLINE. An unquoted $(...) word-splits in bash AND zsh;
# a $VAR holding the same flags does NOT in zsh (measured 2026-09-22), and then
# maspsx gets one argv entry, dies mid-pipe, the exit status still reads 0 and
# objdump prints its "file format" line and no function at all.
tools/gcc263/cpp -Iinclude -Iinclude/psyq -undef -lang-c -nostdinc -Dmips -D__GNUC__=2 /tmp/t.c \
  | tools/gcc263/cc1 -mips1 -mcpu=3000 -quiet -G0 -O2 \
  | .venv/bin/python3 tools/maspsx/maspsx.py $(sed -n 's/^MASPSX_FLAGS *:= *//p' Makefile) \
  | tools/binutils/bin/mipsel-linux-gnu-as -march=r3000 -EL -no-pad-sections -G0 -o /tmp/t.o
tools/binutils/bin/mipsel-linux-gnu-objdump -d /tmp/t.o
```

Under a second per variant. **The maspsx flags are read from the Makefile on
purpose — never retype them.** Round 43 found `tools/setup-permuter.sh` carrying
its own copy of the list, two flags behind the Makefile since round 42, so
every permuter base for a `gp_rel`- or `mflo`-touching function was scored
against a pipeline that was not the pinned one. It now reads the Makefile too.
A reproducer or scaffold that spells the flags itself is not the pinned
pipeline, whatever it says in the comment above it.

**A minimal reproducer that FAILS to reproduce is
itself the result** — it proves the toolchain innocent and the trigger
contextual, which is usually the more useful finding.

Note the permuter mutates C source under the pinned toolchain; that is not a
toolchain change.

## Commands

```sh
./tools/setup.sh                   # one-command bootstrap for a fresh clone
./build-and-verify.sh              # THE canonical build + verify
make extract                       # regenerate asm/ from the executable
python3 tools/progress.py          # where the project is
python3 tools/plan.py              # the finishing plan, measured: tracks, ready jobs, models
python3 tools/rename.py OLD NEW    # rename a symbol everywhere, re-extract, re-verify
tools/check-nonmatching.sh         # the #ifdef NON_MATCHING bodies still compile and link-resolve
python3 tools/funcdiff.py <func>   # per-function score
python3 tools/uncarved.py          # uncarved ground, screened (Gate 2)
python3 tools/nearmiss.py          # the near-miss queue, screened (Gate 1b)
python3 tools/classtable.py --scan # the 60 class method tables
python3 tools/classtable.py <t> --vs <base>   # what a subclass overrides
python3 tools/headercontention.py  # which units would fight over a header
python3 tools/plan.py classes      # track 4: every class, unified or not, and where its views live
python3 tools/typeviews.py --census   # struct layouts from cc1's own stabs: per-class views (also --merge, --warnings, --tree)
tools/setup-worktree.sh <name>     # provision a parallel runner
.venv/bin/python3 tools/psyq_sdk.py install    # sdk/ discs -> lib/ objects (setup.sh runs it)
.venv/bin/python3 tools/psyq_sdk.py match      # place every SDK object in retail
.venv/bin/python3 tools/psyq_sdk.py coverage   # which SDK functions are already owned by an object
python3 tools/sdkstalls.py         # stalled functions that are really Sony library code
python3 tools/stalesyms.py         # preserved bodies calling symbols since renamed
.venv/bin/python3 tools/psyq_sdk.py check      # manifest, yaml `o` segments and lib/ agree
```

## Layout

| path | what |
| --- | --- |
| `disk/SLPS_015.56` | the retail executable. Yours, never committed. |
| `sdk/` | your Psy-Q SDK disc image(s). Never committed; see `sdk/README.md`. |
| `lib/` | Sony's library objects converted from `sdk/`, linked as splat `o` segments. Generated, never committed. |
| `config/psyq-objects.txt` | the manifest: which object, from which disc, at which offset |
| `config/psyq-objects.ld` | GENERATED by `psyq_sdk.py ldfrag`: bss placement and symbol pins for the linked objects |
| `config/` | splat segmentation + symbol names |
| `include/` | project headers and the Psy-Q SDK headers |
| `src/` | carved C units — where the work happens |
| `asm/` | generated disassembly. Never edit, never commit. |
| `tools/` | the toolchain and the workflow tools |
| `docs/` | the guides; `docs/match-reports/` is the durable record; `docs/archive/` holds the full-length history the lean guides were distilled from |
| `config/plan-state.json` | the finishing plan's ledger of decisions, written only through `tools/plan.py` |

## Workflow references

- `docs/FINISHING-PLAN.md` — **what to work on, in what order, on which
  model, and the one head prompt.** Start here for any round.
- `docs/PARALLEL-RUNS.md` — running several sessions at once under a head
  agent: worktrees, collision rules, gates, merging, liveness. Read this
  before spawning anything. The full history it was distilled from is
  `docs/archive/PARALLEL-RUNS-full-2026-09-16.md`.
- `docs/MATCHING-GUIDE.md` — the per-function loop in detail, and how to read
  a unit's real state instead of a transcribed one.
- `docs/DECOMPILATION_LEARNINGS.md` — source-shape idioms, one entry each,
  with the discriminator and a pointer to the round. The full narrative is
  `docs/archive/DECOMPILATION_LEARNINGS-full-2026-09-16.md`.
- `docs/PROGRESS.md` — the running session log.
- `docs/SDK-OBJECTS-RUNS.md` — running SDK-object conversion rounds: the
  head/runner split, collision rules for the shared config files, order and
  estimate, and the two paste-able prompts.
- `docs/SDK-OBJECTS-GUIDE.md` — the step-by-step recipe for converting a
  `psyq_*` block to linked SDK objects. Runner work.
- `docs/research/psyq-sdk-objects.md` — how the SDK objects were placed, what
  each disc covers, and what is still carried as disassembly.
- `CREDITS.md` — this project stands on FirecatFG's lsddecomp for its
  segmentation and symbol names, and on parasite-eve-2-decomp for the
  linked-SDK approach. Read it.
