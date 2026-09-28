# NoOp2 -- MATCHED (2/2 words, splat matched itself)

> Renamed from `func_80028918` on 2026-09-21 (tools/rename.py, round 64,
> runner alpha). Address 0x80028918.

Unit: `CdDriver`. One of the three 2-instruction leaves the unit's own
header comment already noted as "splat matched itself" at carve time
(mid-round 17, 2026-09-04) -- no C was ever written for it, and no report
existed before this one.

## Result

```c
void NoOp2(void) {
}
```

Byte-exact: `build/lsdde.map` confirms `func_80028920 - func_80028918 = 0x8`
(2 instructions, `jr $ra; nop`) both before and after this rename.

## Naming (round 64, runner alpha)

`func_80028918` -> `NoOp2`, tier A. An empty function body IS its own
complete mechanics -- CLAUDE.md/FINISHING-PLAN.md track 3: "a pure leaf
whose mechanics ARE its purpose... is tier A by definition." No caller is
visible in carved code (referenced, if at all, only from the still-uncarved
`code_179d8` remainder or as an as-yet-unidentified vtable slot -- neither
`grep` over `asm/data/*.s` nor `tools/classtable.py --scan` finds this
address anywhere), so there is no evidence for WHICH slot or subsystem this
serves. Named `NoOp2` following the existing project-wide precedent for
multiple distinct no-op functions at different addresses (`NoOp` at
0x80026C80, `NoOpIgnoreArgs` at 0x80056DF0, both in `GameApplicationFileResource.c`/
elsewhere) -- the suffix numbers this unit's three no-op leaves in ROM
order (`NoOp2`/`NoOp3`/`NoOp4`) purely for disambiguation, not because they
share a slot or a caller; no such link is established.

## Naming (round 99, echo, track 7)

Measured: the executable holds no `jal` to this function and no 32-bit word
equal to its address (a scan of every aligned word of `disk/SLPS_015.56`'s
image for both encodings), so it has no caller and sits in no table. The
round 64 note that it is "referenced only from the still-uncarved
`code_179d8` remainder" no longer holds; it is unreferenced, at least by
`jal`, stored pointer, or `lui`/`addiu`/`ori` address build (no `addiu`
or `ori` anywhere in the image carries its address's low half).
