# LockActiveDataSource

> Renamed from `func_80026E0C` on 2026-09-18 (tools/rename.py). Address 0x80026e0c.

**Unit:** GameApplicationFileResource · **Size:** 11 words · **Status:** MATCHED, round 43
(2026-09-15, runner bravo). 11/11 words, byte-exact whole-image build.

## History

Round 2026-08-29-a stalled this as class TOOLCHOOL (`gp_rel`): the obvious C
compiled `gActiveDataSource` as a two-instruction absolute load
(`lui`/`lw`) instead of retail's one-instruction `lw $v1, %gp_rel(gActiveDataSource)($gp)`,
an unrecoverable size mismatch. Round 42 (2026-09-15) resolved that blocker
project-wide with two maspsx flags (`--gp-symbols`, `--no-nop-mflo-mfhi`,
`tools/patches/maspsx-lsd-flags.patch`), now passed unconditionally by the
Makefile. Round 43 rebuilt the PRESERVED body from the round-2026-08-29-a
report verbatim (no source change needed) and it matched on the first build.

## What it does

`if (gActiveDataSource == 0x13) { LockCd(); }` — a region/mode-gated
forward. `gActiveDataSource` lives in the real `.sdata` section (file offset
`0x7b008`, per `config/splat.slps01556.lsdde.yaml`) and is initialized to
`0x13` in the retail image. `LockCd` is a still-uncarved function in
`asm/code_179d8.s`.

## Final body

```c
extern s32 gActiveDataSource;
extern s32 LockCd(void);

void LockActiveDataSource(void) {
    if (gActiveDataSource == 0x13) {
        LockCd();
    }
}
```

## Proposed learning

This is one of a six-function family in this unit (`LockActiveDataSource`,
`UnlockActiveDataSource`, `IsActiveDataSourceBusy`, `IsActiveDataSourceIdle`, `GetActiveDataSourceOperation`,
`GetActiveDataSourceState`) sharing the exact shape `if (gActiveDataSource == 0x13) { ...forward
to a still-uncarved func_800280xx/func_80027Exx... }`, differing only in the
callee and (for four of the six) whether the callee's return value is
propagated (`return func();`) or a constant is returned on the false path.
All six matched on the first build once `gActiveDataSource` was declared as a plain
`extern s32` — no source-shape change was needed at all, confirming the
round-2026-08-29-a "toolchain, not source" diagnosis was correct. See
`GetActiveDataSourceMethods.md` for a second, `if`/`else` variant of the same family (nine
functions total share the `gActiveDataSource == 0x13` gate).

## Naming

Round 52 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_80026E0C` | `LockActiveDataSource` | B |

**Evidence.** `if (gActiveDataSource == DATASOURCE_CD) LockCd();` -- forwards
to the CD driver's own `LockCd` only when it is the active source, no-op
otherwise. First of a six-function family sharing this shape (see
`GetActiveDataSourceMethods.md` and the unit header comment); named
uniformly as `...ActiveDataSource...` across all of them.
