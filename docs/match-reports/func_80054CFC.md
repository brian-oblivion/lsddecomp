# func_80054CFC -- MATCHED, round 46 (2026-09-15)

Unit `class_3bb8c_n`. **13/13 words, byte-exact.** First build.

## What it was

Fresh ground, carved round 45, never attempted. No blockers.

## Derivation

```
/* 454FC 80054CFC 7804828F */  lw   $v0, %gp_rel(D_8008AC80)($gp)
/* 45504 80054D04 06004004 */  bltz $v0, .L80054D20
/* 4550C 80054D0C 8004858F */  lw   $a1, %gp_rel(D_8008AC88)($gp)
/* 45510 80054D10 0980043C */  lui  $a0, %hi(D_8008E0C8)
/* 45514 80054D14 C8E08424 */  addiu $a0, $a0, %lo(D_8008E0C8)
/* 45518 80054D18 F760000C */  jal  func_800183DC
.L80054D20:
...
jr $ra
```

Same family as `func_80054B50` (also calls `func_800183DC`), but gated by
`D_8008AC80 >= 0` rather than a nonzero flag, with no flag-clear afterward
and a variable count (`D_8008AC88`) instead of a literal. `D_8008E0C8` is
the same kind of far `.bss` symbol as `D_8008E10C` (no dlabel in any
`asm/data/*.s`, resolved via `config/undefined_syms_auto.slps01556.lsdde.txt`
and confirmed in `build/lsdde.map`).

```c
extern s32 D_8008AC80;
extern s32 D_8008AC88;
extern void *D_8008E0C8[];

void func_80054CFC(void) {
    if (D_8008AC80 >= 0) {
        func_800183DC(D_8008E0C8, D_8008AC88);
    }
}
```

`D_8008AC80` is the same global already read elsewhere in this class family
(`class_3bb8c_m.c`, `class_3bb8c_r.c func_8005630C`) as a plain `s32`.

### Proposed learning

None new -- confirms the `func_80054B50` pattern generalises (flag test,
optional clear, `func_800183DC` call) rather than needing its own lever.
