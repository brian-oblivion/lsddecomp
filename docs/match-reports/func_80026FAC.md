# func_80026FAC

**Unit:** code_171e0 · **Size:** 15 words · **Status:** MATCHED, round 43
(2026-09-15, runner bravo). 15/15 words, byte-exact whole-image build.

## History

Never attempted (round-2026-08-29-a/30-a stub, `gp_rel`-blocked before any
derivation). Round 42 resolved `gp_rel`. Round 43 derived this fresh from
`asm/nonmatchings/code_171e0/func_80026FAC.s`; matched on the first build.

## What it does

Same `if`/`else` tail-call shape as `func_80026CAC` (see that report for the
full residue analysis), mode `0x13` this time:

```
beq $v1, $v0(0x13), .L80026FD0   # equal -> func_80027EF8
  jal func_8002C448               # fallthrough (not equal)
  j .L80026FD8
.L80026FD0:
  jal func_80027EF8
.L80026FD8:
```

Both callees (`func_80027EF8`, `func_8002C448`) are still uncarved
(`asm/code_179d8.s` / `asm/nonmatchings/code_179d8_e/func_8002C448.s`).
Treated as `s32`-returning per CLAUDE.md's tail-call caution (no positive
void evidence, so default to non-void).

## Final body

```c
extern s32 func_80027EF8(void);
extern s32 func_8002C448(void);

s32 func_80026FAC(void) {
    if (D_8008A84C == 0x13) {
        return func_80027EF8();
    } else {
        return func_8002C448();
    }
}
```

## Proposed learning

See `func_80026CAC.md` — third instance of the "if/else, both arms tail-call"
family in this unit.
