# StyleReleaseEffectSlots -- MATCHED, round 46 (2026-09-15)

> Renamed from `func_80054CFC` on 2026-09-23 (tools/rename.py). Address 0x80054cfc.

Unit `ObjMStyleActor`. **13/13 words, byte-exact.** First build.

## What it was

Fresh ground, carved round 45, never attempted. No blockers.

## Derivation

```
/* 454FC 80054CFC 7804828F */  lw   $v0, %gp_rel(sStyleVariant)($gp)
/* 45504 80054D04 06004004 */  bltz $v0, .L80054D20
/* 4550C 80054D0C 8004858F */  lw   $a1, %gp_rel(sStyleEffectSlotCount)($gp)
/* 45510 80054D10 0980043C */  lui  $a0, %hi(sStyleEffectSlots)
/* 45514 80054D14 C8E08424 */  addiu $a0, $a0, %lo(sStyleEffectSlots)
/* 45518 80054D18 F760000C */  jal  ReleaseBasicClassArray
.L80054D20:
...
jr $ra
```

Same family as `StyleReleaseDecorSet` (also calls `ReleaseBasicClassArray`), but gated by
`sStyleVariant >= 0` rather than a nonzero flag, with no flag-clear afterward
and a variable count (`sStyleEffectSlotCount`) instead of a literal. `sStyleEffectSlots` is
the same kind of far `.bss` symbol as `sStyleDecorSlots` (no dlabel in any
`asm/data/*.s`, resolved via `config/undefined_syms_auto.slps01556.lsdde.txt`
and confirmed in `build/lsdde.map`).

```c
extern s32 sStyleVariant;
extern s32 sStyleEffectSlotCount;
extern void *sStyleEffectSlots[];

void StyleReleaseEffectSlots(void) {
    if (sStyleVariant >= 0) {
        ReleaseBasicClassArray(sStyleEffectSlots, sStyleEffectSlotCount);
    }
}
```

`sStyleVariant` is the same global already read elsewhere in this class family
(`ObjMStyleActor.c`, `class_3bb8c_r.c IsStyleVariantEven`) as a plain `s32`.

### Proposed learning

None new -- confirms the `StyleReleaseDecorSet` pattern generalises (flag test,
optional clear, `ReleaseBasicClassArray` call) rather than needing its own lever.

## Naming

**`StyleReleaseEffectSlots`, tier B.**

Guarded by `sStyleVariant >= 0`; `ReleaseBasicClassArray(sStyleEffectSlots,
sStyleEffectSlotCount)` -- the release half of the triad, called from
`StyleTeardown`. MATCHED, 13/13, first build.

## Track 4 (2026-09-26, round 88, charlie)

`sStyleEffectSlots` retyped `void *[]` -> `StyleEffect *[]` (every element is a New_StyleEffect object); ReleaseBasicClassArray takes it as `(void **)`. Image byte-identical.
