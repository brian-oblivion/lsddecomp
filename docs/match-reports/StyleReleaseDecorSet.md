# StyleReleaseDecorSet -- MATCHED, round 46 (2026-09-15)

> Renamed from `func_80054B50` on 2026-09-23 (tools/rename.py). Address 0x80054b50.

Unit `ObjMStyleActor`. **13/13 words, byte-exact.** First build.

## What it was

Fresh ground, carved round 45, never attempted. No blockers.

## Derivation

```
/* 45350 80054B50 4803828F */  lw    $v0, %gp_rel(gStyleDecorVariant)($gp)
/* 45358 80054B58 06004010 */  beqz  $v0, .L80054B74
/* 45360 80054B60 0980043C */  lui   $a0, %hi(gStyleDecorSlots)
/* 45364 80054B64 0CE18424 */  addiu $a0, $a0, %lo(gStyleDecorSlots)
/* 45368 80054B68 F760000C */  jal   ReleaseBasicClassArray
/* 4536C 80054B6C 12000534 */   ori  $a1, $zero, 0x12
/* 45370 80054B70 480380AF */  sw    $zero, %gp_rel(gStyleDecorVariant)($gp)
.L80054B74:
...
jr $ra
```

Same one-shot-flag shape as `StyleFlushDecoration`: test `gStyleDecorVariant`, act, then
clear the flag. `ReleaseBasicClassArray` is already established across the codebase
(`src/graphics/TmdRenderer.c`, `src/class_3bb8c_o.c`, `src/ObjMStyleActor.c`) as
`void ReleaseBasicClassArray(void **array, s32 count)`. `gStyleDecorSlots` is plain `.bss`
(no `.sdata`/`.sbss` dlabel anywhere; resolved via
`config/undefined_syms_auto.slps01556.lsdde.txt`, confirmed in
`build/lsdde.ld`/the map) -- which is exactly why retail addresses it with
absolute `lui`/`addiu` rather than `%gp_rel`: the maspsx `--gp-symbols`
table is built only from sdata/sbss segment members (CLAUDE.md, "Open
toolchain blockers"), and this symbol isn't one.

```c
extern void ReleaseBasicClassArray(void **array, s32 count);
extern s32 gStyleDecorVariant;
extern void *gStyleDecorSlots[];

void StyleReleaseDecorSet(void) {
    if (gStyleDecorVariant != 0) {
        ReleaseBasicClassArray(gStyleDecorSlots, 0x12);
        gStyleDecorVariant = 0;
    }
}
```

### Proposed learning

**A `.bss` symbol with no dlabel entry in any `asm/data/*.s` file is not
missing -- it lives in `config/undefined_syms_auto.slps01556.lsdde.txt`,
generated for pure runtime-zeroed storage past the end of the ROM image.**
`grep -n <sym> config/undefined_syms_auto.slps01556.lsdde.txt` (or
`build/lsdde.map`) confirms it resolves before assuming a carve gap. It also
explains, mechanically, why such a symbol takes the absolute `lui`/`addiu`
form even when numerically within gp-relative range of `$gp`: the
`--gp-symbols` table is populated only from `.sdata`/`.sbss` segment
members, and a `.bss` symbol is neither.

## Naming

**`StyleReleaseDecorSet`, tier B.**

Guarded by `gStyleDecorVariant`; calls `ReleaseBasicClassArray(gStyleDecorSlots,
0x12)` and clears the guard -- the release half of the `StyleBuildDecorSet`/
`StyleUpdateDecorSet`/`StyleReleaseDecorSet` triad, called from
`StyleTeardown`. MATCHED, 13/13, first build.

## Round 93 polish (delta, track 7)

### Naming

`0x12` is `ARRAY_COUNT(gStyleDecorSlots)` (`STYLE_DECOR_BANDS`), round 93.
