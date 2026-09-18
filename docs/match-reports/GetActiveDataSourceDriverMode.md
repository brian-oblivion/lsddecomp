> Renamed from `func_80026FAC` on 2026-09-18 (tools/rename.py). Address 0x80026fac.

# GetActiveDataSourceDriverMode

**Unit:** code_171e0 · **Size:** 15 words · **Status:** MATCHED, round 43
(2026-09-15, runner bravo). 15/15 words, byte-exact whole-image build.

## History

Never attempted (round-2026-08-29-a/30-a stub, `gp_rel`-blocked before any
derivation). Round 42 resolved `gp_rel`. Round 43 derived this fresh from
`asm/nonmatchings/code_171e0/GetActiveDataSourceDriverMode.s`; matched on the first build.

## What it does

Same `if`/`else` tail-call shape as `GetActiveDataSourceMethods` (see that report for the
full residue analysis), mode `0x13` this time:

```
beq $v1, $v0(0x13), .L80026FD0   # equal -> GetCdDriverMode
  jal func_8002C448               # fallthrough (not equal)
  j .L80026FD8
.L80026FD0:
  jal GetCdDriverMode
.L80026FD8:
```

Both callees (`GetCdDriverMode`, `func_8002C448`) are still uncarved
(`asm/code_179d8.s` / `asm/nonmatchings/code_179d8_e/func_8002C448.s`).
Treated as `s32`-returning per CLAUDE.md's tail-call caution (no positive
void evidence, so default to non-void).

## Final body

```c
extern s32 GetCdDriverMode(void);
extern s32 func_8002C448(void);

s32 GetActiveDataSourceDriverMode(void) {
    if (gActiveDataSource == 0x13) {
        return GetCdDriverMode();
    } else {
        return func_8002C448();
    }
}
```

## Proposed learning

See `GetActiveDataSourceMethods.md` — third instance of the "if/else, both arms tail-call"
family in this unit.
