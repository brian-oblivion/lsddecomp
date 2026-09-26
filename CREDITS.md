# Credits

## FirecatFG's `lsddecomp`

This project's head start comes almost entirely from
[**FirecatFG/lsddecomp**](https://github.com/FirecatFG/lsddecomp), the first
and — as far as we could find — only prior decompilation attempt on *LSD: Dream
Emulator*. What we took from it, and what we deliberately did not:

**Taken at bootstrap (2026-08-28, commit `50e62526`), from lsddecomp as it
stood at `c677d8f4e5df` (2024-09-13):**

- The **splat segmentation** in `config/splat.slps01556.lsdde.yaml`: where the
  Psy-Q library blocks sit, where each rodata slot pairs to, where the game's
  own code begins, and the `$gp` value (`0x8008A808`). Finding these is slow,
  careful work and the result is a set of measurements, not authorship.
  lsddecomp's yaml has 55 subsegment boundaries; all 55 were taken, and 53 of
  them are still boundaries in today's yaml, among the many added by carving
  since.
- **146 symbol names**, merged from lsddecomp's two symbol files
  (`config/symbols.slps01556.lsdde.txt` and `symbols_addrs_manual.txt`) into
  ours. Three of their names were dropped in the merge because they were an
  earlier guess at an address the other file named again (`new_GameManager`,
  `GameManager__GameManager`, `new_DreamEntity`); the symbols file's own header
  records this.
- The **project headers** `types.h`, `common.h`, `StageGrid.h` and
  `DreamSys.h`, which encode struct layouts derived from the disassembly.
  `types.h` and `common.h` are byte-identical to lsddecomp's today.
  `StageGrid.h` and `DreamSys.h` had their `//` comments rewritten as `/* */`
  for GCC 2.6.3's C89 `cpp`; since then `StageGrid.h` has changed by 7 lines
  and `DreamSys.h` has grown from 358 lines to 1094 as its classes
  were decompiled (`git diff --stat 50e62526 HEAD -- include/`).
- **Files that are not lsddecomp's own authorship but came to us through it**,
  byte-identical to its copies: the 42 Sony Psy-Q SDK headers under
  `include/psyq/` (including `SYS/`), and `include/gte.inc`, a table of GTE
  opcode macros. `include/include_asm.h` and `include/macro.inc` share names
  with lsddecomp's files but were rewritten here at bootstrap.
- `docs/research/DreamTimer.md`, their write-up of the dream timer mechanics,
  byte-identical to their `research-docs/DreamTimer.md`. It cites the
  [compu-lsd wiki](https://compu-lsd.com/w/Graph#Connection_to_starting_Field)
  for how the mood graph picks the next day's starting area.

**Not taken:** their C source (`src/lsdde/DreamSys.c`, `src/lsdde/StageGrid.c`)
and their tooling (`tools/m2ctx.py` here is a separate script). Our
`src/DreamSys.c` and `src/StageGrid.c` began as splat-generated `INCLUDE_ASM`
stubs, 118 and 2 of them; every function in both has since been re-derived
from the disassembly in this repo and neither file has an `INCLUDE_ASM` left.
That is a provenance choice rather than a criticism — lsddecomp carries no
license file, so the safe reading is that its authored source is theirs. Facts
about a binary that anyone re-measuring would arrive at are a different thing
from a person's written code.

**A warning that came with the names, and it is worth repeating.** From
lsddecomp's own README: there has never been a symbol file leak for this game,
so every inherited name was either hand-written by FirecatFG or inferred by
`ghidra_psx_ldr`. **None of it is the developers' naming.** This project treats
each one as a hypothesis the disassembly can overturn (a tier-B name, in
`docs/FINISHING-PLAN.md` track 3's terms), and the names were how a C++ reading
nearly got adopted before the bytes ruled it out
(`docs/research/class-framework.md`).

### What is left of the inherited names

Measured 2026-09-26 (round 89), by name *and* address:

```sh
git show 50e62526:config/symbols.slps01556.lsdde.txt \
  | grep -oE '^[A-Za-z_][A-Za-z0-9_]* = 0x[0-9A-Fa-f]+' | sort > /tmp/inherited
grep -oE '^[A-Za-z_][A-Za-z0-9_]* = 0x[0-9A-Fa-f]+' config/symbols.slps01556.lsdde.txt \
  | sort > /tmp/current
wc -l < /tmp/inherited                    # 146 inherited
comm -12 /tmp/inherited /tmp/current      # 125 still in use
comm -23 /tmp/inherited /tmp/current      # 21 renamed (every address is still named)
```

- **21 renamed.** The 12 `BasicClass__func_*` placeholders now say what each
  method does (`BasicClass__AddChild`, `BasicClass__NotifyParents`, ...); the
  six `new_class_*` / `class_65650__*` names became `New_TmdModel`,
  `New_DrawSystem`, `New_CdDriver`, `New_Class6D3C8`, `New_Class6D940`,
  `New_Class65650` and `Class65650__Class65650`; and two `DreamSys__func_*`
  became `DreamSys__ResetSessionState` and `DreamSys__SpawnAtLink`.
- **25 of the 125 survivors are Sony's names for Sony's code**, not guesses:
  each sits in a Psy-Q object the build links (`build/lsdde.map`), in a
  `psyq_*` disassembly segment (`GsLinkObject4`, `ResetGraph`), or in
  `config/sdk-in-game.txt` (`SetRCnt`), and where the object is linked its own
  symbol table carries the same name — `_ExpAllocArea`, for instance, is the
  function at the start of `libc2`'s `malloc.o`.
- **59 are game function names**, every one with a match report under
  `docs/match-reports/`. `BasicClass__func_18350` is the one placeholder among
  them.
- **41 are data names**: lsddecomp's stage tables (`STAGE_*`, `STGnn_*`,
  `LEN_*`, `SPECIAL_*`, `SPAWN_POS_ADJUST`), `DREAMSYS_METHODS`, and the
  globals `gpNavChallengesComplete`, `gpDinamicLinkPenalty` and
  `Small__rand`. The last names the word at `0x8008AC68`, which the build
  places as `libc2/rand.o`'s `.sbss` (`config/psyq-objects.ld`), where Sony's
  object calls it `n`.

Surviving is not the same as confirmed. Whether a name was checked against
the code is recorded in that function's match report, not here.

If you want to *play* a modernised LSD rather than read it,
[LSDRevamped](https://github.com/figglewatts/LSDRevamped) is the port
lsddecomp's README points people at.

## The Psy-Q SDK: linked from Sony's objects, the Parasite Eve 2 way

The executable is roughly one third Sony Psy-Q library code. Rather than
re-derive that as C, this project links the library objects out of the SDK
discs through splat `o` segments, so the bytes are Sony's and the symbol names
are Sony's. The approach, and the idea of sliding each object over the
executable to find where the game linked it, come from
[**GabeRealB/parasite-eve-2-decomp**](https://github.com/GabeRealB/parasite-eve-2-decomp)
(CC0 1.0): its `lib/` layout, `versions.txt` and `tools/match_obj.py` are the
template for our `config/psyq-objects.txt` and `tools/match_obj.py`. Ours
requires a relocation-masked *exact* match instead of scoring by edit distance,
and does not commit the objects — `sdk/` and `lib/` are bring-your-own like
`disk/` — but the shape of the solution is theirs.

The conversion from Sony's LNK object format to ELF is
[**psyq-obj-parser**](https://github.com/grumpycoders/pcsx-redux/tree/main/tools/psyq-obj-parser)
from the grumpycoders' **PCSX-Redux** project; we use the static Linux build
that [decompme/compilers](https://github.com/decompme/compilers) publishes,
which [celophi/lom-decomp](https://github.com/celophi/lom-decomp) pointed us
to. The `.LIB` archive layout in `tools/psyqlib.py` was read off the bytes.

The other PSX projects we studied while deciding —
[sotn-decomp](https://github.com/Xeeynamo/sotn-decomp) and lom-decomp
decompile the SDK as C; [spyro-1](https://github.com/spyro-decomp/spyro-1)
and [esa](https://github.com/mkst/esa) carry it as disassembly — showed us the
three options and their costs.

## Workflow

The parallel-agent workflow — worktree-per-runner, a head agent that triages
and merges, match reports as the durable record, and the hook-enforced build
oracle — is ported from a South Park (N64) decompilation project, which in turn
follows Chris Lewis's
[Snowboard Kids 2 decomp](https://github.com/cdlewis/snowboardkids2-decomp).
Most of the specific rules in `docs/PARALLEL-RUNS.md` are scar tissue from
rounds where something went wrong; the reasons are kept next to the rules on
purpose.

## Tools

Built entirely on the console decompilation community's work:

| tool | what it does here |
| --- | --- |
| [splat](https://github.com/ethteck/splat) (ethteck) | splits the executable into asm and C units |
| [maspsx](https://github.com/mkst/maspsx) (mkst) | reproduces ASPSX macro expansion between cc1 and gas — in the build pipeline, not optional |
| [m2c](https://github.com/matt-kempster/m2c) (matt-kempster) | seeds a C body from a function's disassembly |
| [psyq-obj-parser](https://github.com/grumpycoders/pcsx-redux/tree/main/tools/psyq-obj-parser) (grumpycoders, PCSX-Redux) | converts Sony's Psy-Q `.OBJ` files to ELF objects the linker accepts |
| [pyelftools](https://github.com/eliben/pyelftools) (eliben) | reads the converted objects' text and relocations in `tools/match_obj.py` |
| [asm-differ](https://github.com/simonlindholm/asm-differ) (simonlindholm) | side-by-side diffs with register and branch tracking |
| [decomp-permuter](https://github.com/simonlindholm/decomp-permuter) (simonlindholm) | searches source variants for a match |
| [old-gcc](https://github.com/decompals/old-gcc) (decompals) | the Psy-Q-patched GCC 2.6.3 builds |
| [spimdisasm](https://github.com/Decompollaborate/spimdisasm) / [rabbitizer](https://github.com/Decompollaborate/rabbitizer) | the MIPS disassembly underneath splat |

The `include/psyq/` headers are Sony's Psy-Q SDK headers, as redistributed
across the PSX decompilation scene.

## Legal

Nobody here has any affiliation with Asmik Ace, OutSide Directors Company or
Sony. No game data is distributed: the executable is extracted from a disc you
own, and it is gitignored. This is a clean-room-ish reimplementation effort in
the same spirit as the other console decomp projects — the work is reading a
binary and writing source that reproduces it.
