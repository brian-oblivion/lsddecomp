# The Psy-Q SDK is linked from Sony's objects

*2026-09-11. Decision, spike and first byte-exact result. Counts here are a
dated snapshot; measure with `tools/psyq_sdk.py match` / `coverage`.*

## The decision

Roughly a third of `SLPS_015.56` is Sony Psy-Q runtime library code. The
project carried it as `asm` segments named `psyq_*`, excluded from the game
percentage, with 703 of its 724 functions unnamed. Other PSX projects take one
of three paths:

| path | who | cost |
| --- | --- | --- |
| decompile the SDK as C | sotn-decomp, lom-decomp | hundreds of functions of matching work on code that is not the game |
| keep it as disassembly | spyro-1, esa, us until now | no names, no types, BIOS trampolines with no C form hang around |
| link the SDK's own objects | parasite-eve-2-decomp | find the right library builds; place each object's data sections |

We took the third. The libraries shipped on the SDK discs as `.LIB` archives
of `.OBJ` files **with full symbol names**, so linking them gives every SDK
function and global its real name for free, the Psy-Q manuals then document
what each one does, and the bytes are Sony's rather than a transcription.
splat supports it natively (`o` segments, since 0.41). Nothing Sony-owned is
committed: `sdk/` and `lib/` are gitignored, `config/psyq-objects.txt` is the
manifest, `tools/setup.sh` regenerates `lib/` from the user's discs.

## Which library builds the game linked

The executable embeds RCS ids from three library source files:

| in SLPS_015.56 | 3.5 disc (mastered 1996-06) | 3.6 disc (1996-10) |
| --- | --- | --- |
| `sys.c 1.116 1995/12/01` (libgpu) | `1.120 1996/05/01` | `1.126 1996/09/13` |
| `intr.c 1.73 1995/11/10` (libetc) | `1.73` **same** | `1.73` same |
| `bios.c 1.71 1995/12/01` (libcd) | `1.77 1996/05/13` | `1.80 1996/09/11` |

So `libetc` is the 3.5 build and `libgpu`/`libcd` are OLDER than the 3.5
disc. The game mixed library builds, which is normal for the period. Objects
are therefore taken from whichever disc holds the build that matches retail,
decided per object by measurement.

## Pipeline

```
sdk/<redump>.zip -> track1.bin -> PSX/LIB/*.LIB (ISO 9660 read out of raw
2352-byte sectors) -> .OBJ (tools/psyqlib.py) -> .o (psyq-obj-parser) ->
tools/match_obj.py (relocation-masked exact search over the executable)
```

`match_obj.py` masks every byte a relocation rewrites (`R_MIPS_26` low 26
bits, `HI16`/`LO16`/`GPREL16` low halfword, `R_MIPS_32` whole word) and asks
for every remaining byte to agree. A hit is therefore "this object, as
shipped, is what the game linked". Objects under 8 fully-compared bytes are
skipped; a few 16-byte BIOS stubs hit in many places and are reported as
AMBIGUOUS rather than placed.

Conversion: 3.5 disc 865 objects (6 failures), 3.6 disc 1042 (7). The
failures are `libsn/snmain` and `libsn/cache` (relocation expression types
psyq-obj-parser does not know — parasite-eve-2 keeps its `snmain` as asm for
the same reason) and loose `.OBJ` files that are not libraries.

## What the two discs place (snapshot)

Objects placed exactly, per library, 3.5 disc / 3.6 disc:

| library | 3.5 exact / total | 3.6 exact / total |
| --- | --- | --- |
| libapi | 29 / 74 | 28 / 81 |
| libc | 3 / 55 | 3 / 55 |
| libc2 | 17 / 46 | 17 / 46 |
| libcard | 7 / 14 | 7 / 17 |
| libcd | 9 / 25 | 9 / 25 |
| libetc | 4 / 7 | 4 / 7 |
| libgpu | **0 / 11** | **0 / 11** |
| libgs | 23 / 132 | 23 / 148 |
| libgte | 26 / 233 | 30 / 261 |
| libsnd | 25 / 82 | 24 / 159 |
| libspu | 17 / 65 | 11 / 105 |
| libmath, libpress, libcomb, libgun, libtap, libsio | 0 | 0 |

"Total" is every object on the disc, most of which the game never linked, so
the ratio is not a coverage figure. Coverage is measured against our
functions instead. Of the 724 functions in `psyq_*` segments, the placed
objects own **190** (3.5 and 3.6 together; 3.5 wins ties). Per segment:

| segment | owned | notes |
| --- | --- | --- |
| psyq_rcpoly* (5 segments) | 24 / 24 | **converted — the pilot** |
| psyq_PadInit | 6 / 6 | libetc `pad` |
| psyq_15d04 | 9 / 13 | rest looks like libetc |
| psyq_2258 | 69 / 104 | rest: libgte matrix/geometry, libc2 `prnt`, libapi heap |
| psyq_SpuSetMute | 48 / 114 | rest: libspu/libsnd of another build |
| psyq_GsLinkObject4 | 21 / 201 | rest: **libgpu** (89 functions), libetc, libgte |
| psyq_memset | 13 / 260 | rest: libcd (29), libgpu (19), libpress (9), 140 unattributed |
| psyq_rand | 0 / 2 | libc `rand` of another build |

The uncovered remainder was attributed by opcode-shape similarity (4-grams
of opcode/rs, immediates masked): most of it has a 3.5 object of IDENTICAL
shape and different bytes, i.e. the same source compiled in an older
library build. `libgpu/sys`, `libgte/mtx_*`, `libgte/geo*`, `libcd/sys`
score 1.00 on shape and 0 on bytes. **An older disc is needed for those:
the 3.3 (DTL-S2190) and 3.0 discs on archive.org are the candidates.**

## SDK code hiding in game segments

The placed objects also land inside segments the config calls game code —
45 objects, all `libc2` string functions, `libsnd` driver internals, `libgs`
helpers and 16-byte `libapi`/`libcard` BIOS stubs:

| where | what |
| --- | --- |
| code_171e0, code_179d8_d/_h | `strcat`, `strcpy`, `strstr`, `strcmp`, `strncmp` |
| code_179d8, _c, _f, _i, _j | 19 `libsnd` objects (`sscall`, `stop`, `adsr`, `sstable`, …) |
| code_2cc8c_e | `libgs/gs_133`, `gs_111`, `gs_113`, `gs_108`, `libgte/fgo_00`, `fog_01` |
| class_3bb8c_h, _h_b, _h_c | 13 BIOS trampolines: `libapi/a5x`, `libcard/a7x`, `c17x`, `c112` |

Two consequences. `func_8003FC70`, closed as a game match in round 20 (35/35),
is `libgs/gs_108.o` — matched correctly, but as SDK code. And the "13 BIOS
trampolines no C compiles to" that class_3bb8c_h's yaml comment agonises over
are Sony objects; the `hasm` question for them is moot once they link as `o`.
Every one of these becomes an `o` segment in due course, and the game
denominator shrinks by exactly those functions.

## The pilot: eight libgte objects, byte-exact

`0xAD64..0xD294` is `libgte/divf3a divf4a divg3a divg4a divft3a divft4a
divgt3a divgt4a` — the `RCpoly*` polygon-subdivision family, text-only, no
data sections. Replacing the five `psyq_rcpoly*` asm segments with eight `o`
segments, adding `o_path: lib/`, mirroring `lib/**/*.o` into `build/lib/` in
the Makefile and renaming the eight `func_*` callers in `code_8220_c.c` to
`RCpolyF3` … `RCpolyGT4`: `./build-and-verify.sh` -> `OK: build matches
retail`. First attempt.

One trap found on the way: `make extract` deletes `asm/nonmatchings/` but not
top-level `asm/*.s`, so the five old `psyq_rcpoly*.s` stayed behind and were
still assembled (harmlessly — the linker script no longer named them).
`progress.py` warns about exactly this; delete them by hand after a segment
type change.

## Update, same day: the 3.3 and 3.0 discs

The user supplied the 3.3 (DTL-S2190) and 3.0 (DTL-S2180) discs. 3.0 is a
**Mode 1** disc (user data at +16 in each 2352-byte sector, not +24), which
`tools/extract_exe.py` now recognises; the game discs and the other three
SDK discs are Mode 2.

| disc | mastered | libgpu `sys.c` | libetc `intr.c` | objects placed |
| --- | --- | --- | --- | --- |
| 3.0 | 1995 | `1.67 1995/03/13` | (`pad.c 1.33 1995/03`) | 53 |
| 3.3 | 1995-late | `1.107 1995/10/18` | `1.71 1995/08/29` | **183** |
| **game** | 1998 | **`1.116 1995/12/01`** | **`1.73 1995/11/10`** | |
| 3.5 | 1996-06 | `1.120 1996/05/01` | `1.73` | 160 |
| 3.6 | 1996-10 | `1.126 1996/09/13` | `1.73` | 156 |

Coverage of the 700 `psyq_*` functions rose from 190 to **273**: `psyq_2258`
is now 103/104, `psyq_15d04` and `psyq_rand` are complete, `psyq_GsLinkObject4`
66/201, `psyq_SpuSetMute` 58/114, `psyq_memset` 25/260. Objects placed inside
game segments rose from 45 to **60**. 3.0 placed nothing the other discs had
not.

**The remaining gap is one library build that is on none of the discs.** The
game's `libgpu` (`sys.c 1.116`, December 1995) and `libcd` (`bios.c 1.71`,
December 1995) sit BETWEEN the 3.3 disc (October 1995) and the 3.5 disc (May
1996). Sony shipped library updates between disc releases, and this game was
built against one of those. By shape attribution the uncovered remainder is
roughly: libgpu ~110 functions, libcd ~35, libspu ~15, libpress ~11, plus
~175 that no object resembles at the 0.5 threshold (mostly short BIOS/kernel
glue and data-heavy helpers). Unless an interim 1995-12 library archive turns
up, those stay as `psyq_*` disassembly — which costs nothing beyond names,
and the names can still be recovered: the shape match identifies the MODULE,
and a module's exported symbols and their order are stable between adjacent
builds, so the functions can be named from the 3.3/3.5 object even where the
bytes cannot be linked. That is a symbol pass, not a matching problem.

## Second conversion: a block WITH data sections, and what bss really looks like

`psyq_15d04` + `psyq_PadInit` (`0x15D04..0x16334`, `0x166AC..0x1677C`)
became nine objects: `libetc/vmode intr_dma intr_vb vsync`, `libc2/puts`,
`libetc/pad`, `libapi/a20 a21 a22`. Byte-exact after two iterations.

**Data sections place cleanly.** `psyq_sdk.py place` derives each section's
retail address from the object's own HI16/LO16, `R_MIPS_32` and `GPREL16`
relocations read against retail, and confirms with a byte search. The four
libetc `.data` sections are consecutive (`0x5DB0C`, `+4`, `+0x28`, `+0x28`)
exactly as the linker laid them; `intr_dma`/`vsync` `.rdata` are the two
strings at `0xF28`/`0xF54` that the old rodata slot held; `puts` `.sdata` is
the 7-byte `"<NULL>"` at `0x7B040`.

**bss does NOT.** `libetc/pad`'s `.bss` is 8 bytes — `pad_buf` at +0,
`PadIdentifier` at +4 — and retail has them at `0x8008B3C8` and
`0x8008E984`. `libgs` variables interleave the same way (`HWD0 0x8008E980`,
`PadIdentifier 0x8008E984`, `GsIDMATRIX 0x8008E98C`). Sony's linker
allocated uninitialised variables one at a time across all objects. So an
object's NOBITS section cannot be placed as a unit, and the yaml never tries:
`psyq_sdk.py ldfrag` generates `config/psyq-objects.ld`, linked BEFORE the
splat script, which (a) places each NOBITS section `(NOLOAD)` where its
section-relative references demand, and (b) pins every bss variable by name.
GNU ld lets a linker-script assignment override an object's own definition
— tested on `pad.o`'s WEAK `PadIdentifier` and `vsync.o`'s GLOBAL `Hcount`,
and the object's own `lui`/`sw` then use the pinned address. (c) The same
fragment pins the externals the linked objects call and nothing yet defines
(`printf`, `putchar`, `InterruptCallback`, `ResetCallback`, `ChangeClearPAD`,
`ChangeClearRCnt`), each to the address retail's code calls. A wrong pin
changes an instruction, so it cannot pass the SHA1.

**A false placement, caught by the glabels.** `libgs/gs_125` (`GsGetWorkBase`,
four instructions) "matched" at `0x15D1C` — which is `GetVideoMode`, the
second function of `libetc/vmode`, whose masked bytes are identical. The
corpus has seven such overlaps (`bcopy`/`memcmp`, `s_r`/`s_w`, `gs_111`/
`gs_112`, `libapi/c112`/`libcard/c112`, ...). `match` now prints them and
`install` refuses an overlapping manifest; the tie-break is the segment's own
function list.

**Tooling added for this**: `psyq_sdk.py place`, `symbols` (retail address of
every SDK symbol the corpus defines or references — 548 names, 4 conflicts,
which is also the raw material for the naming pass), `ldfrag`, disc
preference `3.3 > 3.5 > 3.6 > 3.0`, and `check` verifying the fragment is
current. The runner-facing recipe is `docs/SDK-OBJECTS-GUIDE.md`.

## Round 29 (2026-09-11): the corpus's overlaps are one pattern, and three placement failure classes have remedies

Head-side measurements made while the first conversion runner worked
`psyq_2258`. Each was either turned into tooling or recorded as a decision.

**Every `PARTIAL OVERLAP` was a coarse 3.3 module against its own 3.5/3.6
pieces.** Ten containers, twenty-five contained placements, zero exceptions:
`libgte/mtx_00` (0xA20 bytes) against `mtx_003 mtx_004 mtx_005 mtx_006
mtx_008 mtx_00b`; `libgte/mtx` against `mtx_05 mtx_09 mtx_10 mtx_11 mtx_12`;
`libgte/geo` against `cor_00 geo_03`; `libc2/bcopy` (bcopy+bzero+bcmp, 0xC0)
against `bzero memcmp`; `libsnd/ssplay` against `playmode ssplayb`;
`libsnd/ut_rev` against `ut_rdel ut_rdep ut_rfb`; `libsnd/pause` against
`npause`; `reg01`/`reg08`, `reg03`/`reg10 reg11`; and the one true false
positive, `libgs/gs_125` inside `libetc/vmode`. In each case the coarse
object's span tiles the range exactly and the pieces leave gaps (3.5's
`ut_rev` split starts 0x40 bytes after 3.3's `ut_rev`). `placed_objects()`
now drops any placement strictly contained in another; `runs` lists what it
dropped. Coverage is unchanged by construction (254/681), the
`GsOUT_PACKET_P` symbol conflict vanished with `gs_125`, and the queue went
from 189 objects / 52 runs to 165 / 44 with no overlap lines.

**A masked-call body can place uniquely and still be the wrong function.**
`libsnd/ssinit_c` (3.5; `ssinit_h` has identical bytes) is twelve
instructions: prologue, three `jal`s, epilogue. It placed once, at `0x18540`
in `code_179d8`. But `symbols` reported `ResetCallback` at two addresses:
`libetc/intr` DEFINES it at `0x80024D10` (and the already-linked `libetc/pad`
calls it there), while `ssinit_c`'s first `jal` resolves to `0x800280D0`,
which is a four-instruction function that stores 1 to a `$gp`-relative flag.
So the placement is a false positive by shape, and the RUNS doc's "a Sony
name the game overrides (`ResetCallback`)" was this artefact, not an
override. `runs` now prints `SUSPECT` under any run whose object's call
resolves away from another placed object's definition. `func_80027D40` and
its neighbours (`func_80028218` calls the same flag setter first) are the
game's libsnd build, which no disc has.

**An object can be right in text and wrong in one data section, and a
NOLOAD shadow fixes it byte-exactly.** `libetc/intr`: the 3.3 object's text
places at `0x15510` and its `.data` (0x109C, the callback tables) derives to
`0x5CA70`, but its `.rdata` holds `$Id: intr.c,v 1.71 1995/08/29` where
retail has `1.73 1995/11/10` — the 3.5 disc has that string and different
text. Neither object links whole. Mapping the 3.3 object's `.rdata` as a
NOLOAD section at `0x800106BC` (the address its own relocations derive)
makes its references resolve there while the image bytes come from splat's
plain rodata slot, which still holds retail's. Measured in a throwaway
worktree: first attempt three bytes off, all `+4`, because the fragment's
section had no `SUBALIGN` and the object claims alignment 8; with
`SUBALIGN(2)` zero bytes differ. `ldfrag` now emits shadows from a manifest
annotation (`3.3 libetc/intr 0x15510 shadow=.rdata`) and puts `SUBALIGN(2)`
on every fragment section. The head then landed `libetc/intr` on `main`
during consolidation (16 functions named, callers in three `code_179d8_*`
units renamed, byte-exact), leaving `0x15C24..0x15D04` as `psyq_15c24`. The
rest of `psyq_GsLinkObject4`'s last run (`0x153C0..0x15510` and
`0x15C24..0x15D04`) is ordinary runner work now.

**`SUBALIGN(2)` is also why byte gaps need `pad` lines in the yaml.** Sony's
linker aligned each object's data to (at least) 4; splat's script forces
input alignment to 2, so after a section of size 0x81 (`libc2/ctype`
`.data`), 0x2A (`libgs/2d_com1` `.rdata`) or 0x802 (`libgte/geo` `.data`)
the next object lands 2–3 bytes early. splat's `pad` subsegment emits
`. += N;`; verified on the live build at the pilot's own `0xF53..0xF54`
boundary. The guide's step 4 has the three `psyq_2258` instances.

**Three `libgs` objects need editing, or stay asm.** The fragment's plan
for `psyq_GsLinkObject4` carries three `NOTE`s: `gs_001`, `gs_002` and
`gs_003` each have a `.bss` whose SECTION-RELATIVE references (static
variables) derive to several addresses — Sony's linker scattered the
statics too, and a static cannot be pinned by name. This is exactly the
case parasite-eve-2 hand-edited objects for. Options: split each `.bss`
into one section per static (an install-time object rewrite), or leave the
three objects as asm remainders inside their runs. Operator's call; nothing
else in the four segments carries a NOTE (`psyq_2258` 0, `psyq_SpuSetMute`
0, `psyq_memset`+`psyq_rand` 0).

**`libcd/iso9660` carries a WEAK `memcpy`.** `symbols` reports `memcpy`
defined at `0x800238A8` (`libc2/memcpy`, GLOBAL) and `0x8002C014`
(`iso9660`, WEAK). Linking both as-is makes iso9660's internal calls resolve
to libc2's copy — a clean link and wrong bytes. An install-time
`objcopy -L memcpy` on iso9660 (localise the symbol) is the mechanical fix;
phase 2, since iso9660 sits in a game unit.

**Two worktree defects, both found by the worktree's own verification
build.** `find lib` does not descend into a symlinked directory, so
`LIB_FILES` was empty in every worktree and the link failed on the first
`build/lib/*.o`; `.gitignore`'s `lib/` matched only a directory, so the
symlink showed as untracked. And `setup-worktree.sh` never linked `sdk/`, so
`install`/`place`/`runs`/`symbols` saw an empty corpus there (the runner
linked the zips and `sdk/work` by hand; the script now does). Round 28 was
head-alone, so this was the first worktree since the SDK objects landed.

## Round 30 (2026-09-11): phase 1 complete

Three more sequential runner sessions converted `psyq_SpuSetMute`,
`psyq_memset`+`psyq_rand` and `psyq_GsLinkObject4`+`psyq_15c24`: 76 objects
in 16 chunks, 15 byte-exact on the first build. With round 29's 31 that is
124 linked objects; the `psyq_*` asm that remains is 436 functions, of which
427 have no object on any disc and 8 are the three `gs_00x` objects held
back. Decisions and measurements made on the way:

- **`SpuRead`/`SpuWrite` (`libspu/s_r`/`s_w`) are byte-identical objects**;
  retail holds one copy at `0x800391C8`. The placed `libsnd/vs_vtb` calls
  `SpuWrite` and its call resolves there, so `s_w` is the one the game
  linked. The `_spu_read = 0x80038900` line `symbols` prints comes from the
  wrong alternate reading retail's `jal` and is an artefact. General rule:
  when identical objects differ only in name, a placed CALLER decides.
- **`libc/*` vs `libc2/*` alternates**: the game linked libc2 (`itoa`'s
  3-byte `"%d"` is `.sdata` in libc2 and `.rdata` in libc; retail has it in
  sdata).
- **Nine libcd objects (`c_002 c_003 c_004 c_005 c_007 c_008 c_009 c_010
  cdrom`) each carry the same unreferenced 0x10-byte `.data`.** Retail has
  four copies. Only `c_003`'s is placeable (one relocation names it); the
  rest are left out of the yaml and `/DISCARD/`ed — byte-exact, because
  nothing in the objects' text reads them.
- **`libgs/gs_001 gs_002 gs_003` stay asm** (`psyq_140dc`, `psyq_14e9c`,
  `psyq_15020`) for scattered static bss; `GsSetDrawBuffClip` and
  `GsSetDrawBuffOffset` therefore stay `func_*`, pinned as externals.
- **splat merges labels when a segment shrinks.** A `jr $t2` BIOS
  trampoline has no `jr $ra`, so once its segment boundary moves splat folds
  it into the preceding function's `glabel` (`libapi/a07` in bravo's round;
  six trampolines under one `glabel func_80025424` in `psyq_15c24`). The
  linked objects recover the names; a carve that KEEPS such a range as asm
  would lose them silently.
- **The fragment scales with libcd**: 34 NOLOAD sections, 333 pinned bss
  symbols, 48 externals, 0 notes. `libcd/c_011` alone pins 16 scattered
  statics by name.
- Delta counted 26 splat `glabel`s against 25 exported functions in
  `0x1514C..0x15510` and could not localise the extra one after the range
  converted; oracle green, every Sony name applied. Recorded, not resolved.

## What is next

1. **Text-only, fully covered blocks first**: `psyq_PadInit` (libetc `pad`),
   then the covered parts of `psyq_2258`. Each needs its `.rdata`/`.data`/
   `.sbss`/`.bss` pieces placed — that is the real work and where
   parasite-eve-2 needed to hand-edit three objects' bss ordering.
2. **The 45 objects inside game segments**, which also retires the
   class_3bb8c_h `hasm` question.
3. **An older disc** for libgpu/libcd/libgte/libspu of the 1995 build.
4. Symbol renames: every `func_*` that a placed object owns takes the
   object's exported name, in `config/symbols.slps01556.lsdde.txt` and in the
   C that calls it.
