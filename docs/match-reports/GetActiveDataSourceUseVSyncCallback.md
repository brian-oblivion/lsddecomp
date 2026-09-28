# GetActiveDataSourceUseVSyncCallback

> Renamed from `func_80026FE8` on 2026-09-18 (tools/rename.py). Address 0x80026fe8.

**Unit:** GameApplicationFileResource · **Size:** 15 words · **Status:** MATCHED, round 43
(2026-09-15, runner bravo). 15/15 words, byte-exact whole-image build.

## History

Never attempted (round-2026-08-29-a/30-a stub, `gp_rel`-blocked before any
derivation). Round 42 resolved `gp_rel`. Round 43 derived this fresh from
`asm/nonmatchings/code_171e0/GetActiveDataSourceUseVSyncCallback.s`; matched on the first build.

## What it does

Same `if`/`else` tail-call shape as `GetActiveDataSourceMethods`/`GetActiveDataSourceDriverMode`, mode
`0x13`, different callees:

```
beq $v1, $v0(0x13), .L8002700C   # equal -> GetCdUseVSyncCallback
  jal GetVabUseVSyncCallback                # fallthrough (not equal)
  j .L80027014
.L8002700C:
  jal GetCdUseVSyncCallback
.L80027014:
```

`GetVabUseVSyncCallback` is independently confirmed non-void elsewhere in the repo:
`src/PlacementGridVabSound.c` has `s32 GetVabUseVSyncCallback(void) { return 0; }`. This is
the direct positive evidence CLAUDE.md asks for -- the else-arm really does
return `s32`, so the whole function (and, by the same shape, its two
siblings `GetActiveDataSourceMethods`/`GetActiveDataSourceDriverMode`) is correctly typed non-void, not
merely defaulted to it. `GetCdUseVSyncCallback` is still uncarved
(`asm/nonmatchings/CdDriver/GetCdUseVSyncCallback.s`).

## Final body

```c
extern s32 GetCdUseVSyncCallback(void);
extern s32 GetVabUseVSyncCallback(void);

s32 GetActiveDataSourceUseVSyncCallback(void) {
    if (gActiveDataSource == 0x13) {
        return GetCdUseVSyncCallback();
    } else {
        return GetVabUseVSyncCallback();
    }
}
```

## Proposed learning

See `GetActiveDataSourceMethods.md`. This is the instance that upgrades the family's
non-void typing from "CLAUDE.md-default assumption" to "independently
confirmed": `GetVabUseVSyncCallback`'s real definition elsewhere in the tree already
declares `s32`.

## Naming

Round 52 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_80026FE8` | `GetActiveDataSourceUseVSyncCallback` | B |

**Evidence.** Forwards to `GetCdUseVSyncCallback` (itself just `return
gCdUseVSyncCallback;`, an already-named global) when the CD driver is
active, else `GetVabUseVSyncCallback` (always `0`). The "VSync callback" framing
comes directly from that already-established global's name, generalised to
whichever source is active.
