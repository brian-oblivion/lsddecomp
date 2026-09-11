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
