# SpuVmSeKeyOn -- MATCH (59/59 words, 2 rebuild attempts)

> Renamed from `func_800302DC` on 2026-09-23 (tools/rename.py). Address 0x800302dc.

Unit `libsnd_vmanager`, round 21 (2026-09-06). Not a class method (no
`classtable.py` hit on either global it touches) -- plain utility code.

m2c's seed (`.venv/bin/python3 tools/m2ctx.py libsnd_vmanager --sig 's32
SpuVmSeKeyOn(s32 p0, s32 p1, s32 p2, s32 p3, u16 p4, u16 p5)' --run`)
came back essentially byte-identical to the final source on the first
try; only the local variable names were changed for clarity.

**The 4th register parameter (`p3`) is completely unused by the body.**
Retail never reads `$a3` at all -- the two u16 arguments actually used
(`p4`, `p5`) arrive on the STACK at the 5th/6th ABI positions
(`0x30($sp)`/`0x34($sp)` before the callee's own `-0x20` frame
allocation), which only happens if there really are 4 register
parameters ahead of them. Per the already-documented "unused parameter"
idiom (DECOMPILATION_LEARNINGS), declare it and never reference it --
that reproduces retail's silent register waste exactly.

```c
extern s32 SpuVmKeyOn(s32 a0, s16 a1, s16 a2, u16 a3, u16 a4, u16 a5);

s32 SpuVmSeKeyOn(s32 p0, s32 p1, s32 p2, s32 p3, u16 p4, u16 p5)
{
    u16 outA;
    u16 outB;

    if (p4 == p5) {
        outB = 0x40;
        outA = p4;
    } else if (p5 < p4) {
        outA = p4;
        outB = (p5 << 6) / p4;
    } else {
        outA = p5;
        outB = 0x7F - ((p4 << 6) / p5);
    }
    return SpuVmKeyOn(0x21, (s16) p0, (s16) p1, (u16) p2, outA, outB);
}
```

`SpuVmKeyOn` is itself still `INCLUDE_ASM` elsewhere (never seen in
any other unit's source), so this extern is a per-call-site guess typed
from the registers loaded before the `jal`: a literal `0x21` first, two
`s16`-truncated args, then three `u16`s (the third truncated explicitly
at the call site since `p2`'s own entry never gets the prologue
narrowing idiom -- only `p4`/`p5`, which arrive already masked from the
stack `lhu`/`andi` pair, get that treatment).

Retail's own `p4==p5` / `p5<p4` / else three-way split with the two
runtime `div`s (maspsx-expanded, zero-check + `INT_MIN/-1` overflow
check both present) reproduced exactly with a plain three-armed
`if`/`else if`/`else` and plain `/` operators -- no manual reconstruction
of the `div`/`break 6`/`break 7` sequence needed.

### Proposed learning

None beyond what's already written down; this one just confirmed the
existing "unused 4th register argument" and "runtime `/` reproduces the
safe-division expansion" idioms transfer cleanly to a fresh function.

## Round 97 types pass (echo)

Sony's function (libsnd/vmanager); the function name is Sony's and was not
touched. Parameters renamed from m2c's `p0..p5` to `vabId, prog, note,
unused, volL, volR` and the locals `outA/outB` to `vol/pan`: the three leading
arguments are forwarded unchanged to `SpuVmKeyOn`'s VAB/program/note slots,
the fourth is never read, and the last two are reduced to one volume (the
larger) and a pan of 64 when equal, `volR*64/volL` (< 64, left) when left is
louder and `127 - volL*64/volR` (> 64, right) when right is louder. `0x40`
and `0x7F` became decimal `64`/`127` (volume/pan values). `0x21` became
`SPUVM_SE_SEQ`, local to the unit: both SE key functions pass it as
`SpuVmKeyOn`/`SpuVmKeyOff`'s first (sequence) argument, and the SE paths in
`libsnd_vm_vol_ut_key_ut_keyv.c` store the same value in `D_8008EA22` and
`_svm_voice[].unk0E`. Byte-exact unchanged.

## Unit banner history (moved from src/code_179d8_j.c, round 97)

The unit's banner carried this history; it moved here when the banner was
rewritten as documentation.

- Carved round 21 (2026-09-06) out of the middle of the old
  `code_179d8_mid_c` by blocker density: the front quarter of functions
  173..198 of the original `code_179d8` monolith, 0x20ADC..0x20FF0.
- Round 34 (2026-09-12): the slice was cut in two by the linked
  `libsnd/vm_prog.o` (Psy-Q 3.6) at 0x20FF0..0x21180, whose four functions
  (`SpuVmSetProgVol`, `SpuVmGetProgVol`, `SpuVmSetProgPan`,
  `SpuVmGetProgPan`, previously matched as `func_800307F0`, `func_80030864`,
  `func_800308B8`, `func_8003092C`) had their C deleted; everything from
  `SpuVmSetVol` on moved to `src/libsnd_vm_vol_ut_key_ut_keyv.c`. The
  `SlotE968` local view of `_svm_pg` left with them (those four were its only
  readers). All four had screened clean on every blocker, the lesson being
  that `nearmiss.py`/`sdkstalls.py` must run before any WORKABLE list is
  believed.
- The old monolith has no `jtbl_` and no `.word .L`, so no unit of this
  family owns a rodata attach.
- Round 42/43: the banner's "BLOCKED" verdicts (`gp_rel`, `nop_mflo_mfhi`,
  `addiu_at`) were retracted; all three constructs are resolved maspsx flags.
- Round 79: `KeyOnCheck` identified as vmanager's internal hook; the unit
  holds no game code.
- Round 97: the local `Entry90902E8` view (only `+0x74`/`+0x76` named, as
  `s16`) retired onto `include/SsScore.h`; the unused externs `SpuVmVSetUp`,
  `SpuVmPBVoice`, `SeAutoVol`, `SeAutoPan` (no call in any of the unit's C or
  asm) were dropped.
