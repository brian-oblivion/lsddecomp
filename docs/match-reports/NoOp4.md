# NoOp4 -- MATCHED (2/2 words, splat matched itself)

> Renamed from `func_80028B64` on 2026-09-21 (tools/rename.py, round 64,
> runner alpha). Address 0x80028B64.

Unit: `code_179d8_h`. One of the three 2-instruction leaves the unit's own
header comment already noted as "splat matched itself" at carve time
(mid-round 17, 2026-09-04) -- no C was ever written for it, and no report
existed before this one.

## Result

```c
void NoOp4(void) {
}
```

Byte-exact: `build/lsdde.map` confirms `GetCdUseVSyncCallback - func_80028B64
= 0x8` (2 instructions, `jr $ra; nop`) both before and after this rename.

## Naming (round 64, runner alpha)

`func_80028B64` -> `NoOp4`, tier A. Same reasoning as `NoOp2.md`/`NoOp3.md`:
an empty body IS the complete mechanics, tier A by CLAUDE.md/FINISHING-
PLAN.md track 3's own definition. No caller or vtable-slot evidence found
(`grep` over `asm/data/*.s`, `tools/classtable.py --scan`: no hit).
Numbered against `NoOp`/`NoOpIgnoreArgs` (elsewhere in the tree) and this
unit's own `NoOp2`/`NoOp3`, in ROM order, purely for disambiguation.

## Naming (round 99, echo, track 7)

Measured: the executable holds no `jal` to this function and no 32-bit word
equal to its address (a scan of every aligned word of `disk/SLPS_015.56`'s
image for both encodings), so it has no caller and sits in no table. The
round 64 note that it is "referenced only from the still-uncarved
`code_179d8` remainder" no longer holds; it is unreferenced, at least by
`jal`, stored pointer, or `lui`/`addiu`/`ori` address build (no `addiu`
or `ori` anywhere in the image carries its address's low half).
