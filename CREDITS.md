# Credits

## FirecatFG's `lsddecomp`

This project's head start comes almost entirely from
[**FirecatFG/lsddecomp**](https://github.com/FirecatFG/lsddecomp), the first
and — as far as we could find — only prior decompilation attempt on *LSD: Dream
Emulator*. What we took from it, and what we deliberately did not:

**Taken (facts about the binary):**

- The **splat segmentation** in `config/splat.slps01556.lsdde.yaml`: where the
  Psy-Q library blocks sit, where each rodata slot pairs to, where the game's
  own code begins, and the `$gp` value. Finding these is slow, careful work and
  the result is a set of measurements, not authorship.
- The **symbol names** in `config/symbols.slps01556.lsdde.txt` — 146 of them,
  covering the `DreamSys` and `StageGrid` subsystems, the Psy-Q entry points
  and the class constructors.
- The **project headers** `types.h`, `common.h`, `StageGrid.h` and
  `DreamSys.h`, which encode struct layouts derived from the disassembly.
- `docs/research/DreamTimer.md`, their write-up of the dream timer mechanics.

**Not taken:** their C source. `src/DreamSys.c` and `src/StageGrid.c` here are
splat-generated `INCLUDE_ASM` stubs, and every function is being re-derived from
the disassembly in this repo. That is a provenance choice rather than a
criticism — lsddecomp carries no license file, so the safe reading is that its
authored source is theirs. Facts about a binary that anyone re-measuring would
arrive at are a different thing from a person's written code.

**A warning that came with the names, and it is worth repeating.** From
lsddecomp's own README: there has never been a symbol file leak for this game,
so every name in `config/symbols.slps01556.lsdde.txt` was either hand-written by
FirecatFG or inferred by `ghidra_psx_ldr`. **None of it is the developers'
naming.** Treat a symbol name as a hypothesis the disassembly can overturn,
exactly as you would treat a guessed struct layout.

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
