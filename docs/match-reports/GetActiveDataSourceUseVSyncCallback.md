> Renamed from `func_80026FE8` on 2026-09-18 (tools/rename.py). Address 0x80026fe8.

# GetActiveDataSourceUseVSyncCallback

**Unit:** code_171e0 · **Size:** 15 words · **Status:** MATCHED, round 43
(2026-09-15, runner bravo). 15/15 words, byte-exact whole-image build.

## History

Never attempted (round-2026-08-29-a/30-a stub, `gp_rel`-blocked before any
derivation). Round 42 resolved `gp_rel`. Round 43 derived this fresh from
`asm/nonmatchings/code_171e0/GetActiveDataSourceUseVSyncCallback.s`; matched on the first build.

## What it does

Same `if`/`else` tail-call shape as `GetActiveDataSourceMethods`/`GetActiveDataSourceDriverMode`, mode
`0x13`, different callees:

```
beq $v1, $v0(0x13), .L8002700C   # equal -> func_80028B6C
  jal func_8002C478                # fallthrough (not equal)
  j .L80027014
.L8002700C:
  jal func_80028B6C
.L80027014:
```

`func_8002C478` is independently confirmed non-void elsewhere in the repo:
`src/code_179d8_e.c` has `s32 func_8002C478(void) { return 0; }`. This is
the direct positive evidence CLAUDE.md asks for -- the else-arm really does
return `s32`, so the whole function (and, by the same shape, its two
siblings `GetActiveDataSourceMethods`/`GetActiveDataSourceDriverMode`) is correctly typed non-void, not
merely defaulted to it. `func_80028B6C` is still uncarved
(`asm/nonmatchings/code_179d8_h/func_80028B6C.s`).

## Final body

```c
extern s32 func_80028B6C(void);
extern s32 func_8002C478(void);

s32 GetActiveDataSourceUseVSyncCallback(void) {
    if (gActiveDataSource == 0x13) {
        return func_80028B6C();
    } else {
        return func_8002C478();
    }
}
```

## Proposed learning

See `GetActiveDataSourceMethods.md`. This is the instance that upgrades the family's
non-void typing from "CLAUDE.md-default assumption" to "independently
confirmed": `func_8002C478`'s real definition elsewhere in the tree already
declares `s32`.
